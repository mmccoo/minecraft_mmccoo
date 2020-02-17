
#pragma once

#include <MinecraftWorld.h>
#include <polygon.h>
#include <biome.h>

class ElevationVectors {
public:
    ElevationVectors(MinecraftWorld &world);

    void add_chunk(int chunkx, int chunkz, Grid16);

    void write(std::string filename);

private:
    std::map<int, PolygonHolesSet> polysets;
    int y_resolution;
};
