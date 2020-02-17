


#pragma once


#include <MinecraftWorld.h>
#include <vector>

class ResourceVectors {
  public:
    ResourceVectors(MinecraftWorld& world);
    void write(std::string filename);

  private:
    struct Resource {
        int x;
        int y;
        int z;
        BlockType bt;
        bool isSurface;
    };

    std::vector<Resource> resources;
};
