
#include <SubChunk.h>
#include <iostream>
#include <cassert>

SubChunk::SubChunk()
{
    memset(block_type_ids, 0, 16*16*16);
    chunkx=-1;
    chunky=-1;
    chunkz=-1;
}


void SubChunk::set_type_at(int x, int y, int z, uint8_t type)
{
    assert(x<16);
    assert(y<16);
    assert(z<16);
    assert(x>=0);
    assert(y>=0);
    assert(z>=0);

    assert(chunky>=0);
    block_type_ids[x][y][z] = type;
}

uint8_t SubChunk::get_type_at(int x, int y, int z)
{
    assert(x<16);
    assert(y<16);
    assert(z<16);
    assert(x>=0);
    assert(y>=0);
    assert(z>=0);

    return block_type_ids[x][y][z];
}
