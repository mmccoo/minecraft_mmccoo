

#ifndef MINECRAFT_WORLD_HH
#define MINECRAFT_WORLD_HH

#include <map>

#include <SubChunk.h>

// based on https://www.artificialworlds.net/blog/2017/05/11/c-iterator-example-and-an-iterable-range/

class MinecraftWorld {
  public:
    std::map<int, std::map<int, std::map<int, SubChunk> > > theworld;

    int num_chunks();

    void    set_type_at(int x, int y, int z, uint8_t type);

    // a return value of 0 means undefined. It's not in the database
    uint8_t get_type_at(int x, int y, int z);

    SubChunk &get_sub_chunk(int chunkx, int chunky, int chunkz);

};



#endif
