

#pragma once


#include <MinecraftWorld.h>
#include <polygon.h>

class CaveVectors {

  public:
    CaveVectors(MinecraftWorld& world);
    void write(std::string filename);

  private:
    std::map<int, PolygonHolesSet> polysets;
};
