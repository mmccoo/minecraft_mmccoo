
#pragma once

#include <string>
#include <array>

typedef std::array<std::array<int, 16>, 16> Grid16;

struct Biome {
    std::string name;
    int id;
    struct {
        int r;
        int g;
        int b;
    } color;
};

const Biome &get_biome(int id);

void write_biome_properties(std::string filename);
