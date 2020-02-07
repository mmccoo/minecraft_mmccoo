
#include <parse_bedrock.h>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <sstream>
#include <fstream>
#include <map>
#include <array>
#include <typeinfo>
#include <regex>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/decompress_allocator.h"

#include "BlockType.h"
#include <MinecraftWorld.h>
#include <biome.h>

#include <BiomeImage.h>

#include <polygon.h>

// this page was really useful:
// http://web.archive.org/web/20110723210920/http://www.minecraft.net/docs/NBT.txt

// The MCC ToolChest PE was also invaluable in verifying my numbers.
// http://www.mcctoolchest.com/

#define UNUSED(x) (void)(x)


int parse_nbt_tag(leveldb::Slice& slice, int offset, NBTObject &nbtobj, std::string indent);

// I could used function overloading, but I want to be sure to get the right data size.
int32_t get_intval(leveldb::Slice slice, uint32_t offset)
{
    int32_t retval = 0;
    // need to switch this to union based like the others.
    for(int i=0; i<4; i++) {
        // if I don't do the static cast, the top bit will be sign extended.
        retval |= (static_cast<uint8_t>(slice[offset+i])<<i*8);
    }
    return retval;
}

int64_t get_longval(leveldb::Slice slice, uint32_t offset)
{
    // initializing a union assigns to the first member.
    // https://en.cppreference.com/w/c/language/struct_initialization
    union {
        long lnum;
        uint8_t bytes[8];
    } retval = {0};

    for(int i=0; i<8; i++) {
        // if I don't do the static cast, the top bit will be sign extended.
        //retval |= (static_cast<uint8_t>(slice[offset+i])<<i*8);
        retval.bytes[i] = slice[offset+i];
    }
    return retval.lnum;
}

float get_floatval(leveldb::Slice slice, uint32_t offset)
{

    // https://stackoverflow.com/a/36960552/23630
    // I don't understand the issue, but this union enables me to
    // properly extract floats.
    union {
        float fnum;
        uint8_t bytes[4];
    } retval = {0};

    for(int i=0; i<4; i++) {
        // if I don't do the static cast, the top bit will be sign extended.
        //retval.num |= (static_cast<uint8_t>(slice[offset+i])<<i*8);
        retval.bytes[i] = slice[offset+i];
    }

    return retval.fnum;
}

int16_t get_shortval(leveldb::Slice slice, uint32_t offset)
{
    union {
        int16_t shortnum;
        uint8_t bytes[2];
    } retval = {0};

    for(int i=0; i<2; i++) {
        retval.bytes[i] = slice[offset+i];
    }
    return retval.shortnum;
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
    Short = 2,
    Int = 3,
    Long = 4,
    Float = 5,
    ByteArray = 7,
    String = 8,
    List = 9,
    Compound = 10,
};
std::map<int, std::string> Tag_Names = {
    {NBT_Tags::End,      "TagEnd"},
    {NBT_Tags::Byte,     "TagByte"},
    {NBT_Tags::Short,    "TagShort"},
    {NBT_Tags::Int,      "TagInt"},
    {NBT_Tags::Long,     "TagLong"},
    {NBT_Tags::Float,    "TagFloat"},
    {NBT_Tags::ByteArray, "TagByteArray"},
    {NBT_Tags::String,   "TagString"},
    {NBT_Tags::List,     "TagList"},
    {NBT_Tags::Compound, "TagCompound"},
};

// I have this function because of lists. with lists, you have a bunch of the same type
// but the type and tagname is only represented once for the list
int parse_nbt_payload(leveldb::Slice& slice,
                      int curoffset,
                      std::string &tagname,
                      int id,
                      NBTObject &nbtobj,
                      std::string indent="")
{
    NBTObject::indent = indent;

    switch (id) {
    case NBT_Tags::Compound: {
        NBTObject *obj_to_pass = &nbtobj;
        if (tagname == "ItemInHand") {
            assert(typeid(nbtobj) == typeid(EntityType));
            obj_to_pass = &(dynamic_cast<EntityType&>(nbtobj).get_item_in_hand());
        } else if ((tagname == "states") && (typeid(nbtobj) == typeid(BlockType))) {
            // this is here to suppress the info message.
        } else if ((tagname == "tag") && (typeid(nbtobj) == typeid(EquipmentType))) {
            // this is here to suppress the info message.
            // equipment has Damage inside a tag.
        } else if (tagname == "") {
            // no sense in reporting an no-name compound
        } else {
            std::cout << indent << "compound(" << tagname << ")\n";
        }
        while(slice[curoffset] != NBT_Tags::End) {
            curoffset = parse_nbt_tag(slice, curoffset, *obj_to_pass, indent + "  ");
        }

        // this is to skip over the NBT_Tags::End.
        curoffset += 1;

        return curoffset;
        break;
    }

    case NBT_Tags::String: {
        // for strings, there's the nbt tag name and the value of the string itself.

        // now comes the length of the string itself
        int16_t strlength = get_shortval(slice, curoffset);
        curoffset += 2;

        std::string strvalue = get_strval(slice, curoffset, strlength);
        curoffset += strlength;

        nbtobj.add_string(tagname, strvalue);

        return curoffset;
        break;
    }

    case NBT_Tags::Byte: {
        int8_t byteval = (int8_t) slice[curoffset];
        UNUSED(byteval);
        curoffset += 1;

        nbtobj.add_byte(tagname, byteval);

        return curoffset;
        break;
    }

    case NBT_Tags::Short: {
        int16_t s = get_shortval(slice,curoffset);
        curoffset += 2;

        nbtobj.add_short(tagname, s);

        return curoffset;
        break;
    }

    case NBT_Tags::Int: {
        int32_t intval = get_intval(slice, curoffset);
        UNUSED(intval);
        curoffset += 4;


        nbtobj.add_int(tagname, intval);

        return curoffset;
        break;
    }

    case NBT_Tags::Long: {
        long longval = get_longval(slice, curoffset);
        UNUSED(longval);
        curoffset += 8;

        nbtobj.add_long(tagname, longval);

        return curoffset;
        break;
    }
    case NBT_Tags::Float: {
        float floatval = get_floatval(slice, curoffset);
        UNUSED(floatval);
        curoffset += 4;

        if (typeid(nbtobj) == typeid(RotationType)) {
            RotationType &rot = dynamic_cast<RotationType&>(nbtobj);
            rot.add_value(floatval);
        } else if (typeid(nbtobj) == typeid(PositionType)) {
            PositionType &pos = dynamic_cast<PositionType&>(nbtobj);
            pos.add_value(floatval);
        } else if (typeid(nbtobj) == typeid(MotionType)) {
            MotionType &motion = dynamic_cast<MotionType&>(nbtobj);
            motion.add_value(floatval);
        } else if (typeid(nbtobj) == typeid(Attribute)) {
            Attribute &attr = dynamic_cast<Attribute&>(nbtobj);
            attr.add_float(tagname, floatval);
        } else {
            std::cout << indent << "float " << tagname << "(" << floatval << std::dec << ")\n";
        }

        return curoffset;
        break;
    }

    case NBT_Tags::ByteArray: {
        // TYPE: 7  NAME: TAG_Byte_Array
        // Payload: TAG_Int length
        // An array of bytes of unspecified format. The length of this array is <length> bytes
        int32_t length = get_intval(slice, curoffset);
        curoffset += 4;
        std::cout << "got byte array of length " << length << " bytes\n";

        curoffset += length;

        return curoffset;
        break;
    }

    case NBT_Tags::List: {
        int8_t tagid = (int8_t) slice[curoffset];
        curoffset += 1;

        int32_t length = get_intval(slice, curoffset);
        curoffset += 4;

#if 0
        for(size_t i=curoffset; i<(size_t)(curoffset+20); i++) {
            std::cout << indent << std::hex << (unsigned int) slice[i] << std::dec << " ";
        }
        std::cout << "\n";
        for(size_t i=curoffset; i<(size_t)(curoffset+100); i++) {
            std::cout << indent <<  slice[i] << std::dec;
        }
        std::cout << "\n";
#endif

        std::string listname{""};

        if (typeid(nbtobj) == typeid(EntityType)) {
            EntityType &entity = dynamic_cast<EntityType&>(nbtobj);

            if (tagname == "Armor") {
                assert(length == 4);
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_armor(i), indent + "  ");
                }
            } else if (tagname == "Attributes") {
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_attr(i), indent + "  ");
                }
            } else if (tagname == "Pos") {
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_position(), indent + "  ");
                }
            } else if (tagname == "Rotation") {
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_rotation(), indent + "  ");
                }
            } else if (tagname == "Motion") {
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_motion(), indent + "  ");
                }
            } else if (tagname == "Mainhand") {
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_main_hand(), indent + "  ");
                }
            } else if (tagname == "Offhand") {
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_off_hand(), indent + "  ");
                }
            } else if (tagname == "definitions") {
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid,
                                                  entity.get_definitions(), indent + "  ");
                }
            } else {
                std::cout << "other entity list " << tagname << " of length " << length << "\n";
                for(int i=0; i<length; i++) {
                    curoffset = parse_nbt_payload(slice, curoffset, listname, tagid, nbtobj, indent + "  ");
                }
            }
        } else {
            std::cout << "other list " << tagname << " of length " << length << "\n";
            for(int i=0; i<length; i++) {
                curoffset = parse_nbt_payload(slice, curoffset, listname, tagid, nbtobj, indent + "  ");
            }
        }


        return curoffset;
        break;
    }
    default:
        std::cout << indent << "other id " << id << "\n";
        break;
    }

    // bad. need exception or something.
    assert(0);
    return -1;

}

int parse_nbt_tag(leveldb::Slice& slice, int offset, NBTObject &nbtobj, std::string indent="")
{
    NBTObject::indent = indent;

    int curoffset = offset;

    // first byte is the nbt "node number". tells you what the next tag is.
    int id = slice[curoffset];
    curoffset += 1;

    // all nbt tags can have a name.
    int len = (slice[curoffset]) | (slice[curoffset+1]<<8);
    curoffset += 2;

    std::string tagname = get_strval(slice, curoffset, len);
    curoffset += len;

    curoffset = parse_nbt_payload(slice, curoffset, tagname, id, nbtobj, indent);

    return curoffset;
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

    //BiomeImage bimage;

    const std::regex village_dweller_regex("VILLAGE_[0-9a-f\\-]+_DWELLERS");
    const std::regex village_info_regex("VILLAGE_[0-9a-f\\-]+_INFO");
    const std::regex village_player_regex("VILLAGE_[0-9a-f\\-]+_PLAYERS");
    const std::regex village_poi_regex("VILLAGE_[0-9a-f\\-]+_POI");
    const std::regex map_regex("map_\\-[0-9]+");

    int num = 0;
    for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
        num++;

        auto k = iter->key();
        auto v = iter->value();

        if ((k.ToString() == "Nether") || (k.ToString() == "portals")) {
            continue;
        }

        // the first eight bytes are chunk x and z
        int chunkx = k.size()>4 ? get_intval(k,0) : 0;
        int chunkz = k.size()>8 ? get_intval(k,4) : 0;

        // overworld subchunk keys are 10 long
        int chunky = -1;
        if ((k.size() == 10) && (k[8] == 0x2f)) {
            chunky = k[9];
        } else if ((k.size() == 14) && (k[12] == 0x2f)) {
            // i'm skipping nether and end for the moment.
            continue;
            // neo is nether(1), end(2), overworld(omitted).
            int neo = get_intval(k,8);
            UNUSED(neo);
            chunky = k[11];
        } else if ((k.size() == 13) && (k[12] == 0x2d)) {
            int neo = get_intval(k,8);

            std::cout << "biomes and elevation for " << chunkx << ", " << chunkz << ", " << (neo==1 ? "nether" : "end") << "\n";

            continue;
        } else if ((k.size() == 9) && (k[8] == 0x2d)) {

            std::cout << "biomes and elevation for " << chunkx << ", " << chunkz << ", overworld" << "\n";

            Grid16 elevations;
            for(int i=0; i<256; i++) {
                int16_t height = get_shortval(v, i*2);
                elevations[i%16][i/16] = height;
            }
            world.chunk_elevation[chunkx][chunkz] = elevations;

            Grid16 biomes;
            for(int i=0; i<256; i++) {
                // the biomes are stored after the elevations;
                uint8_t biome = v[(256*2)+i];
                biomes[i%16][i/16] = biome;
            }
            world.chunk_biomes[chunkx][chunkz] = biomes;
            //std::cout << "\n";

            //bimage.add_chunk(chunkx, chunkz, biomes);
            continue;

        } else if ((k.size() == 13) && (k[12] == 0x32)) {
            int neo = get_intval(k,8);
            std::cout << "entities for " << chunkx << ", " << chunkz << (neo==1 ? "nether" : "end") << "\n";
            continue;

        } else if ((k.size() == 9) && (k[8] == 0x32)) {
            std::cout << "entities for " << chunkx << ", " << chunkz << ", overworld" << "\n";

            auto v = iter->value();
            size_t curoffset = 0;
            int num = 0;
            while (curoffset < v.size()) {
                num++;
                std::cout << "num " << num << "\n";
                EntityType *entity = new EntityType();
                curoffset = parse_nbt_tag(v,curoffset, *entity);
                std::cout << "got entity of type: " << entity->get_type() << " at ";
                std::cout << entity->get_position().get_string() << " and rot: ";
                std::cout << entity->get_rotation().get_string();
                std::cout << " uid: " << entity->get_unique_id();

                for(auto def : entity->get_definitions().get_defs()) {
                    std::cout << " " << def;
                }

                std::cout << "\n";;

                if (entity->get_type() == "minecraft:item") {
                    std::cout << "more info " << entity->get_string() << "\n";
                }
                world.chunk_entities[chunkx][chunkz].push_back(entity);
            }

            std::cout << "vsize " << v.size() << " offset " << curoffset << "\n";
            if (v.size() != curoffset) { std::cout << "different\n"; }

            continue;
        } else if ((k.size() == 13) && (k[12] == 0x31)) {
            int neo = get_intval(k,8);
            std::cout << "block entities for " << chunkx << ", " << chunkz << (neo==1 ? "nether" : "end") << "\n";
            continue;


        } else if ((k.size() == 9) && (k[8] == 0x31)) {
            std::cout << "block entities for " << chunkx << ", " << chunkz << ", overworld" << "\n";

#if 0
            for(size_t i=0; i<30; i++) {
                std::cout << std::hex << (unsigned int) iter->value()[i] << std::dec << " ";
            }
            std::cout << "\n";
#endif

            auto v = iter->value();
            size_t curoffset = 0;
            // seems like nbt would tell you how many to expect.
            while (curoffset < v.size()) {
                BlockEntityType blockentity;
                curoffset = parse_nbt_tag(v,curoffset, blockentity);
                std::cout << "got block entity of type " << blockentity.get_type() << " at " << blockentity.get_position_string() << "\n";;
                world.chunk_block_entities[chunkx][chunkz].push_back(blockentity);
            }

            if (v.size() != curoffset) {
                std::cout << "vsize " << v.size() << " offset " << curoffset << "\n";
            }

            continue;
        } else if ((k.size() == 9) && (k[8] == 0x36)) {
            //std::cout << "finalized state for overworld\n";
            continue;
        } else if ((k.size() == 13) && (k[12] == 0x36)) {
            int neo = get_intval(k,8);
            UNUSED(neo);
            //std::cout << "finalized state for << " << (neo==1 ? "nether" : "end") << "\n";
            continue;
        } else if ((k.size() == 9) && (k[8] == 0x76)) {
            //std::cout << "version for overworld\n";
            continue;
        } else if ((k.size() == 13) && (k[12] == 0x76)) {
            int neo = get_intval(k,8);
            UNUSED(neo);
            //std::cout << "version for " << (neo==1 ? "nether" : "end") << "\n";
            continue;
        } else if ((k.size() == 13) && (k[12] == 0x39)) {
            int neo = get_intval(k,8);
            UNUSED(neo);
            std::cout << "39 data " << (neo==1 ? "nether" : "end") << " length " << v.size() << "\n";
            for(size_t i=0; i<v.size(); i++) {
                std::cout << std::hex << (unsigned int) iter->value()[i] << std::dec << " ";
            }
            std::cout << "\n";
            continue;

            // doesn't seem to be nbt
            size_t curoffset = 0;
            int num = 0;
            while (curoffset < v.size()) {
                num++;
                std::cout << "num " << num << "\n";
                NBTObject nbtobj;
                curoffset = parse_nbt_tag(v,curoffset, nbtobj);
            }
            continue;
        } else if ((k.size() == 9) && (k[8] == 0x35)) {
            std::cout << "35 data overworld length " << v.size() << "\n";

            for(size_t i=0; i<v.size(); i++) {
                std::cout << std::hex << (unsigned int) iter->value()[i] << std::dec << " ";
            }
            std::cout << "\n";
            continue;
        } else if (std::regex_match(k.ToString(), village_dweller_regex) ||
                   std::regex_match(k.ToString(), village_info_regex) ||
                   std::regex_match(k.ToString(), village_player_regex) ||
                   std::regex_match(k.ToString(), village_poi_regex)) {
            std::cout << "village data: " << k.ToString() << "\n";
            // sample data name VILLAGE_52c7ed3f-9531-4563-91b8-bcfde4151e4d_PLAYERS
            // the strlens should be optimized into constants.
            std::string vname = k.ToString().substr(strlen("VILLAGE_"), strlen("52c7ed3f-9531-4563-91b8-bcfde4151e4d"));

            size_t curoffset = 0;
            auto v = iter->value();
            VillageType &village = world.get_village(vname);
            curoffset = parse_nbt_tag(v,curoffset, village);
            continue;
        } else if (std::regex_match(k.ToString(), map_regex)) {
            std::cout << "map " << k.ToString() << " length " << v.size() << "\n";
            continue;
        } else if (k.ToString() == "BiomeData") {
            std::cout << "Biome data\n";
            size_t curoffset = 0;
            auto v = iter->value();
            NBTObject nbtobj;
            curoffset = parse_nbt_tag(v,curoffset, nbtobj);
            continue;
        } else if (k.ToString() == "AutonomousEntities") {
            std::cout << "AutonomousEntities\n";
            size_t curoffset = 0;
            auto v = iter->value();
            NBTObject nbtobj;
            curoffset = parse_nbt_tag(v,curoffset, nbtobj);
            continue;
        } else if (k.ToString() == "Overworld") {
            std::cout << "Overworld\n";
            size_t curoffset = 0;
            auto v = iter->value();
            NBTObject nbtobj;
            curoffset = parse_nbt_tag(v,curoffset, nbtobj);
            continue;
        } else if (k.ToString() == "TheEnd") {
            std::cout << "TheEnd\n";
        } else {
#if 1
            std::cout << "non subchunk " << chunkx << ", " << chunkz << "\n";
            std::cout << k.ToString() << "\n";
            int neo = get_intval(k,4);
            UNUSED(neo);

            // these are the non-subchunk entries
            for(size_t i=0; i<iter->key().size(); i++) {
                std::cout << std::hex << (unsigned int) iter->key()[i] << std::dec << " ";
            }
            std::cout << "\n";

            for(size_t i=0; i<iter->value().size(); i++) {
                std::cout << std::hex << (unsigned int) iter->value()[i] << std::dec << " ";
            }
            std::cout << "\n";

#endif
            continue;
        }


#if 0
        std::cout << "\n(" << chunkx << "," << chunkz << ") id: " << chunky;
        std::cout << " vsize: " << iter->value().size();
        std::cout << "\n";
#endif

        SubChunk &curchunk = world.get_sub_chunk(chunkx, chunky, chunkz);

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
                int y = chunky*16   + ((i>>0) & 0xf);
                // doing it this way is a bit slow since the subchunk has to be looked up every time.
                // but we're loading data chunk by chunk.
                world.set_type_at(x,y,z, block_types[block_val]);
#else

                int x = ((i>>8) & 0xf);
                int z = ((i>>4) & 0xf);
                int y = ((i>>0) & 0xf);

                curchunk.set_type_at(x,y,z, block_type_id);
                BlockType::get_block_type_by_id(block_type_id).incr_count();
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


    //BlockType::print_block_types();

    delete iter;
    delete db;
}
