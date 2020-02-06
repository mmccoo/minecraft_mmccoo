

#pragma once


#include <map>
#include <string>
#include <array>

#include <Magick++.h>

typedef std::array<std::array<int, 16>, 16> Grid16;

class BiomeImage {
public:
    BiomeImage();

    void add_chunk(int chunkx, int chunkz, Grid16);

    void write(std::string filename);

    static const int pixels_per_block = 4;
    static const int tile_size = pixels_per_block*16;

private:

    std::map<int, std::map<int, Grid16 > > chunks;
    std::map<int, std::map<int, Magick::Image > > images;
};
