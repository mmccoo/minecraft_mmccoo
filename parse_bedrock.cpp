
#include <parse_bedrock.h>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <sstream>
#include <map>

#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/decompress_allocator.h"

#include "BlockType.h"
#include <MinecraftWorld.h>

int32_t get_intval(leveldb::Slice slice, uint32_t offset)
{
    int32_t retval = 0;
    for(int i=0; i<4; i++) {
        // if I don't do the static cast, the top bit will be sign extended.
        retval |= (static_cast<uint8_t>(slice[offset+i])<<i*8);
    }
    return retval;
}

std::string get_strval(leveldb::Slice slice, uint32_t offset, uint32_t length)
{
    std::string retval;
    retval.reserve(length);

    for(uint32_t i=0; i<length; i++) {
        retval += slice[offset+i];
    }

    return retval;
}



enum NBT_Tags {
    End = 0,
    Byte = 1,
    Int = 3,
    String = 8,
    Compound = 10,
};

int parse_nbt_tag(leveldb::Slice& slice, int offset, BlockType &bt) {
    int curoffset = offset;

    // first byte is the nbt "node number". tells you what the next tag is.
    int id = slice[curoffset];
    curoffset += 1;

    // all nbt tags can have a name.
    int len = (slice[curoffset]) | (slice[curoffset+1]<<8);
    curoffset += 2;

    std::string tagname = get_strval(slice, curoffset, len);
    curoffset += len;

    switch (id) {
    case NBT_Tags::Compound: {
        while(slice[curoffset] != NBT_Tags::End) {
            curoffset = parse_nbt_tag(slice, curoffset, bt);
        }

        // this is to skip over the NBT_Tags::End.
        curoffset += 1;

        return curoffset;
        break;
    }

    case NBT_Tags::String: {
        // for strings, there's the nbt tag name and the value of the string itself.

        // now comes the length of the string itself
        int strlength = (slice[curoffset]) | (slice[curoffset+1]<<8);
        curoffset += 2;

        std::string strvalue = get_strval(slice, curoffset, strlength);
        curoffset += strlength;

        // this is not a generic nbt parser. this is a minecraft
        // parser. If the current string tag has an nbt name "name",
        // it's the name of the palette entry.
        if (tagname == "name") {
            bt.name = strvalue;
        } else {
            bt.string_properties[tagname] = strvalue;
        }

        return curoffset;
        break;
    }

    case NBT_Tags::Byte: {
        int8_t byteval = (int8_t) slice[curoffset];
        curoffset += 1;

        bt.byte_properties[tagname] = byteval;

        return curoffset;
        break;
    }

    case NBT_Tags::Int: {
        int32_t intval = get_intval(slice, curoffset);
        curoffset += 4;

        bt.int_properties[tagname] = intval;

        return curoffset;
        break;
    }
    default:
        std::cout << "other id " << id << "\n";
        break;
    }

    // bad. need exception or something.
    assert(0);
    return -1;
}

void parse_bedrock(std::string dbpath, MinecraftWorld &world) {

    class NullLogger : public leveldb::Logger {
    public:
        void Logv(const char* format, va_list va) override {
            //char p[1024];
            //vsnprintf(p, sizeof(p)/sizeof(char), format, va);
            //std::cout << p << "\n";
        }
    };

    // This options and compressor related stuff comes from
    // mcpe_sample_setup.cpp in leveldb-mcpe
    leveldb::Options options;

    //create a bloom filter to quickly tell if a key is in the database or not
    options.filter_policy = leveldb::NewBloomFilterPolicy(10);

    //create a 40 mb cache (we use this on ~1gb devices)
    options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

    //create a 4mb write buffer, to improve compression and touch the disk less
    options.write_buffer_size = 4 * 1024 * 1024;

    //disable internal logging. The default logger will still print out things to a file
    options.info_log = new NullLogger();

    //use the new raw-zip compressor to write (and read)
    options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);

    //also setup the old, slower compressor for backwards compatibility. This will only be used to read old compressed blocks.
    options.compressors[1] = new leveldb::ZlibCompressor();


    //create a reusable memory space for decompression so it allocates less
    leveldb::ReadOptions readOptions;
    readOptions.decompress_allocator = new leveldb::DecompressAllocator();

    // the basics of iteration are described in leveldb-mcpe/doc/index.md

    leveldb::DB *db;
    assert(leveldb::DB::Open(options, dbpath, &db).ok());

    leveldb::Iterator *iter = db->NewIterator(readOptions);

    uint32_t total_blocks = 0;

    int num = 0;
    for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
        num++;

        auto k = iter->key();
        auto v = iter->value();

        // overworld subchunk keys are 10 long
        int chunkidx = -1;
        if ((k.size() == 10) && (k[8] == 0x2f)) {
            chunkidx = k[9];
        } else if ((k.size() == 14) && (k[12] == 0x2f)) {
            // i'm skipping nether and end for the moment.
            continue;

            chunkidx = k[11];
        } else {
#if 0
            // these are the non-subchunk entries
            for(size_t i=0; i<iter->key().size(); i++) {
                std::cout << std::hex << (unsigned int) iter->key()[i] << std::dec << " ";
            }
            std::cout << "\n";
#endif
            continue;
        }

        int chunkx =  get_intval(k,0);
        int chunkz =  get_intval(k,4);

#if 0
        std::cout << "\n(" << chunkx << "," << chunkz << ") id: " << chunkidx;
        std::cout << " vsize: " << iter->value().size();
        std::cout << "\n";
#endif

        SubChunk &curchunk = world.get_sub_chunk(chunkx, chunkidx, chunkz);

        // This format is basically documented in https://minecraft.gamepedia.com/Bedrock_Edition_level_format
        // in the SubChunkPrefix section.

        int curoffset = 0;

        //  version is 8      num storage blocks is 1
        int version = v[curoffset];
        curoffset++;
        if (version != 0x08) {
            std::cout << "unexpected version " << version << "\n";
        }

        int num_storage_blocks = v[curoffset];
        curoffset++;

        for(int blocknum=0; blocknum<num_storage_blocks; blocknum++) {
            int storageVersion = v[curoffset];
            curoffset++;

            int bitsPerBlock = storageVersion >> 1;
            int blocksPerWord = std::floor(32.0 / bitsPerBlock);
            int numints = ceil(4096.0 / blocksPerWord);

            // the first byte of block storage is at:
            // 0 - Version (always 8?)
            // 1 - Storage count (always 1?)
            // 2 - first byte of first storage
            //   2 - storage version + bitsperblock (lsb/version should always be 0)
            //   3->3+(4*numints)-1 These are the block states
            // 2 + 4*numints is the first byte of palette storage
            //   (assuming only one block storage)

            int block_data_offset = curoffset;

            int paletteoffset = block_data_offset+4*numints;

            int psize = get_intval(v,paletteoffset);
            paletteoffset += 4;

#if 0
            std::cout << "palette: (";
            std::cout << psize << ", " << v.size()-paletteoffset << ") ";
            if (psize>19) {
                std::cout << "large palette " << psize;
            }
            std::cout << "\n";
#endif

            std::vector<int> block_types;
            for(int i=0; i<psize; i++) {
                BlockType block_type;
                paletteoffset = parse_nbt_tag(v, paletteoffset, block_type);

                BlockType::add_block_type(block_type);
                int id = BlockType::get_block_type_id(block_type);
                block_types.push_back(id);
            }

            // this is important. there's usually only one block, but sometimes more.
            curoffset = paletteoffset;

            // Blocks are stored in XZY order (i.e. incrementing Y first)
            for (int i=0; i<4096; i++) {
                int32_t maskval = get_intval(v, block_data_offset+(i/blocksPerWord)*4);
                long unsigned int block_val = (maskval >> ((i%blocksPerWord)*bitsPerBlock)) & ((1<<bitsPerBlock)-1);
                if (block_val >= block_types.size()) {
                    std::cout << "oob\n";
                }

                int block_type_id = block_types[block_val];
#if 0
                int x = chunkx*16   + ((i>>8) & 0xf);
                int z = chunkz*16   + ((i>>4) & 0xf);
                int y = chunkidx*16 + ((i>>0) & 0xf);
                // doing it this way is a bit slow since the subchunk has to be looked up every time.
                // but we're loading data chunk by chunk.
                world.set_type_at(x,y,z, block_types[block_val]);
#else

                int x = ((i>>8) & 0xf);
                int z = ((i>>4) & 0xf);
                int y = ((i>>0) & 0xf);

                curchunk.set_type_at(x,y,z, block_type_id);
                BlockType::get_block_type_by_id(block_type_id).count++;
#endif

                total_blocks++;
#if 0
                if (block_types[block_val] == 19) {
                    std::cout << "at (" << x << ", " << y << ", " << z

                              << ") idx " << block_val << "\n";
                }
#endif

            }
        }
    }
    std::cout << "total blocks " << total_blocks << "\n";

    BlockType::print_block_types();

    delete iter;
    delete db;
}
