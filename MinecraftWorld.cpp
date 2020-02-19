

#include <MinecraftWorld.h>
#include <cassert>
#include <set>
#include <fstream>
#include <cmath>
//#include <ctime>
#include <chrono>

//https://github.com/nlohmann/json#json-as-first-class-data-type
// this json header is copied from the above github. I don't know how to include
// a single file as a submodule and the git report itself is pretty large.
#include <nlohmann/json.hpp>

void MinecraftWorld::populate_extra()
{
    for (auto scix : theworld) {
        int chunkx = scix.first;
        UNUSED(chunkx);
        for (auto sciy : scix.second) {
            int chunky = sciy.first;
            UNUSED(chunky);
            for (auto sciz : sciy.second) {
                int chunkz = sciz.first;
                UNUSED(chunkz);
                auto sc = sciz.second;

                chunks_by_y[chunkx][chunkz][chunky] = sc;
            }
        }
    }


    for (auto scix : chunks_by_y) {
        int chunkx = scix.first;
        UNUSED(chunkx);
        for (auto sciz : scix.second) {
            int chunkz = sciz.first;
            UNUSED(chunkz);


            // This is the highed elevation of a stone or dirt block at each location.
            Grid16 &chunk_top_earthly = top_earthly[chunkx][chunkz];

            for (auto sciy : sciz.second) {
                int chunky = sciy.first;
                UNUSED(chunky);
                auto sc = sciy.second;
                for (auto iter=sc->begin(); iter!=sc->end(); ++iter) {
                    auto loc = *iter;

                    BlockType bt = BlockType::get_block_type_by_id(loc.type);

                    // The loc has real world coords.
                    int rawx = loc.x - chunkx*16;
                    int rawz = loc.z - chunkz*16;

                    if (bt.is_earthly()) {
                        chunk_top_earthly[rawx][rawz] = std::max(chunk_top_earthly[rawx][rawz], loc.y);
                    }
                }
            }

#if 0
            std::cout << "for " << chunkx << ", " << chunkz << "\n";
            for(int x=0; x<16; x++) {
                std::cout << "x";
                for(int z=0; z<16; z++) {
                    std::cout << " " << chunk_top_earthly[x][z];
                }
                std::cout << "\n";
            }
#endif
        }
    }

}

void MinecraftWorld::write_world_json(std::string filename)
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

    nlohmann::json top;

    const int max_tile_level = std::ceil(log(std::max(xch-xcl, zch-zcl))/log(2));

    top["chunk_xl"] = xcl;
    top["chunk_zl"] = zcl;
    top["chunk_xh"] = xch;
    top["chunk_zh"] = zch;
    top["max_tile_level"] = max_tile_level;
    top["tile_0_size"] = pow(2, max_tile_level)*16;
    top["y_resolution"] = y_resolution;

    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    top["update_time"] = std::ctime(&end_time);


    std::ofstream file;
    file.open(filename);
    file << top.dump(2);
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

    SubChunk *subchunk = theworld[chunkx][chunky][chunkz];
    subchunk->chunkx = chunkx;
    subchunk->chunky = chunky;
    subchunk->chunkz = chunkz;

    subchunk->set_type_at(x,y,z, type);
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
    return theworld[chunkx][chunky][chunkz]->get_type_at(x,y,z);

}


SubChunk *
MinecraftWorld::get_sub_chunk(int chunkx, int chunky, int chunkz)
{
    if (theworld[chunkx][chunky].find(chunkz) == theworld[chunkx][chunky].end()) {
        theworld[chunkx][chunky][chunkz] = new SubChunk();
    }

    SubChunk *subchunk = theworld[chunkx][chunky][chunkz];
    subchunk->chunkx = chunkx;
    subchunk->chunky = chunky;
    subchunk->chunkz = chunkz;
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
