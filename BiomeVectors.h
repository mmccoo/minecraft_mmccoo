
#pragma once


#include <polygon.h>
#include <biome.h>

class BiomeVectors {
public:
    BiomeVectors();

    void add_chunk(int chunkx, int chunkz, Grid16);

    void write(std::string filename);

private:
    std::map<int, PolygonHolesSet> polysets;

};
