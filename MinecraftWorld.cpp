

#include <MinecraftWorld.h>
#include <cassert>
#include <set>
#include <fstream>
#include <cmath>

void MinecraftWorld::write_world_js(std::string filename)
{

    int xcl, xch, zcl, zch;
    bool first = true;

    // I'm iterating on chunk biomes because world.js was originally used to
    // bound biome image tiles display
    for(auto it1 : chunk_biomes) {
        int chunkx = it1.first;
        for(auto it2 : it1.second) {
            int chunkz = it2.first;
            if (first) {
                xcl = xch = chunkx;
                zcl = zch = chunkz;
                first = false;
            } else {
                xcl = std::min(xcl, chunkx);
                xch = std::max(xch, chunkx);
                zcl = std::min(zcl, chunkz);
                zch = std::max(zch, chunkz);
            }
        }
    }

    const int max_tile_level = std::ceil(log(std::max(xch-xcl, zch-zcl))/log(2));

    std::ofstream file;
    file.open(filename);
    file << "const world_info = {\n";
    file << "  chunk_xl: " << xcl << ",\n";
    file << "  chunk_zl: " << zcl << ",\n";
    file << "  chunk_xh: " << xch << ",\n";
    file << "  chunk_zh: " << zch << ",\n";
    file << "  max_tile_level: " << max_tile_level << ",\n";
    file << "  tile_0_size: " << pow(2, max_tile_level)*16 << ",\n";

    file << "  eojson: 1\n";
    file << "}\n";
    file << "exports.info = world_info;\n";

    file.close();

}

void real_to_chunk(int realcoord, int &chunkcoord, int &offset)
{
    chunkcoord = realcoord/16;
    offset = realcoord - chunkcoord*16;
    if (offset < 0) {
        chunkcoord--;
        offset = realcoord - chunkcoord*16;
    }
    assert(offset<16 && offset>=0);
}

int MinecraftWorld::num_chunks()
{
    std::set<std::pair<int, int> > setofchunks;

    for (auto scix : theworld) {
        int chunkx = scix.first;
        for (auto sciy : scix.second) {
            //int chunky = sciy.first;
            for (auto sciz : sciy.second) {
                int chunkz = sciz.first;
                //auto sc = sciz.second;

                setofchunks.insert(std::make_pair(chunkx, chunkz));
            }
        }
    }

    // I have to do it this way because not all subchunks are stores.
    return setofchunks.size();
}


void
MinecraftWorld::set_type_at(int realx, int realy, int realz, uint8_t type)
{
    int chunkx, x;
    real_to_chunk(realx, chunkx, x);
    int chunky, y;
    real_to_chunk(realy, chunky, y);
    int chunkz, z;
    real_to_chunk(realz, chunkz, z);

    SubChunk &subchunk = theworld[chunkx][chunky][chunkz];
    subchunk.chunkx = chunkx;
    subchunk.chunky = chunky;
    subchunk.chunkz = chunkz;

    subchunk.set_type_at(x,y,z, type);
}

uint8_t
MinecraftWorld::get_type_at(int realx, int realy, int realz)
{

    int chunkx, x;
    real_to_chunk(realx, chunkx, x);
    int chunky, y;
    real_to_chunk(realy, chunky, y);
    int chunkz, z;
    real_to_chunk(realz, chunkz, z);

    if (theworld.find(chunkx) == theworld.end()) {
        return 0;
    }
    if (theworld[chunkx].find(chunky) == theworld[chunkx].end()) {
        return 0;
    }

    if (theworld[chunkx][chunky].find(chunkz) == theworld[chunkx][chunky].end()) {
        return 0;
    }

    // a return value of 0 means undefined. It's not in the database
    return theworld[chunkx][chunky][chunkz].get_type_at(x,y,z);

}


SubChunk &
MinecraftWorld::get_sub_chunk(int chunkx, int chunky, int chunkz)
{
    SubChunk &subchunk = theworld[chunkx][chunky][chunkz];
    subchunk.chunkx = chunkx;
    subchunk.chunky = chunky;
    subchunk.chunkz = chunkz;
    return subchunk;
}


void MinecraftWorld::populate_entity_table()
{
    for(auto it1 : chunk_entities) {
        for(auto it2 : it1.second) {
            for(auto entity : it2.second) {
                entities_by_id[entity->get_unique_id()] = entity;
            }
        }
    }
}
