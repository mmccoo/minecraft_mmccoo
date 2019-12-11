

#include <MinecraftWorld.h>
#include <cassert>
#include <set>

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
