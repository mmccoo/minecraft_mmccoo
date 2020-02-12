

#include <biome.h>

//https://github.com/nlohmann/json#json-as-first-class-data-type
// this json header is copied from the above github. I don't know how to include
// a single file as a submodule and the git report itself is pretty large.
#include <nlohmann/json.hpp>

#include <map>
#include <iostream>
#include <fstream>

// if you change this, make sure to update biomeproperties.js in web_stuff/map
Biome Biomes[] = {
    {"Ocean", 0,  {2,0,107}},
    {"Plains", 1, {140,178,100}},
    {"Desert", 2, {247,146,42}},
    {"Mountains", 3, {95,95,95}},
    {"Forest", 4, {21,101,41}},
    {"Taiga", 5, {24,101,89}},
    {"Swamp", 6, {46,249,181}},
    {"River", 7, {0,0,250}},
    {"Nether", 8, {252,0,1}},
    {"TheEnd", 9, {125,121,251}},
    {"FrozenOcean", 0xa, {141,141,156}},
    {"FrozenRiver", 0xb, {158,155,252}},
    {"SnowyTundra", 0xc, {254,254,254}},
    {"SnowyMountains", 0xd, {156,157,156}},
    {"MushroomFields", 0xe, {251,0,249}},
    {"MushroomFieldShore", 0xf, {156,0,249}},

    {"Beach", 0x10, {249,222,93}},
    {"DesertHills", 0x11, {207,94,31}},
    {"WoodedHills", 0x12, {39,85,35}},
    {"TaigaHills",  0x13, {28,58,53}},
    {"MountainEdge", 0x14, {122,127,157}},
    {"Jungle", 0x15, {84,122,30}},
    {"JungleHills", 0x16, {48,68,19}},
    {"JungleEdge", 0x17, {98,138,40}},
    {"DeepOcean", 0x18, {3,2,49}},
    {"StoneShore", 0x19, {158,159,130}},
    {"SnowyBeach", 0x1a, {249,239,191}},
    {"BirchForest", 0x1b, {52,115,70}},
    {"BirchForestHills", 0x1c, {39,96,56}},
    {"DarkForest", 0x1d, {67,83,36}},
    {"SnowyTaiga", 0x1e, {52,85,74}},
    {"SnowyTaigaHills", 0x1f, {41,65,57}},
    {"GiantTreeTaiga", 0x20, {88,101,81}},
    {"GiantTreeTaigaHills", 0x21, {84,94,64}},
    {"WoodedMountains", 0x22, {80,111,81}},
    {"Savanna", 0x23, {187,178,104}},
    {"SavannaPlateau", 0x24, {164,155,101}},
    {"Badlands", 0x25, {213,67,29}},
    {"WoodedBadlandsPlateau", 0x26, {174,150,103}},
    {"BadlandsPlateau", 0x27, {198,137,101}},
    {"Unknown28", 0x28, {0,0,0}},
    {"Unknown2a", 0x2a, {0,0,0}},
    {"Unknown2b", 0x2b, {0,0,0}},
    {"WarmOcean", 0x2c, {76,35,183}},
    {"LukewarmOcean", 0x2d, {87,58,168}},
    {"ColdOcean", 0x2e, {69,116,214}},
    {"DeepWarmOcean", 0x2f, {100,53,227}},
    {"DeepLukewarmOcean", 0x30, {105,55,238}},
    {"DeepColdOcean", 0x31, {29,72,164}},
    {"DeepFrozenOcean", 0x32, {158,184,237}},
    {"Void", 0x7f, {3,3,3}},
    {"SunflowerPlains", 0x81, {181,220,140}},
    {"DesertLakes", 0x82, {253,186,75}},
    {"GravellyMountains", 0x83, {133,133,133}},
    {"FlowerForest", 0x84, {105,115,46}},
    {"TaigaMountains", 0x85, {88,101,81}},
    {"SwampHills", 0x86, {65,254,218}},
    {"IceSpikes", 0x8c, {178,218,218}},
    {"ModifiedJungle", 0x95, {122,162,59}},
    {"ModifiedJungleEdge", 0x97, {138,178,71}},
    {"TallBirchForest", 0x9b, {89,154,108}},
    {"TallBirchHills", 0x9c, {79,137,96}},
    {"DarkForestHills", 0x9d, {104,121,71}},
    {"SnowyTaigaMountains", 0x9e, {89,123,112}},
    {"GiantSpruceTaiga", 0xa0, {106,95,78}},
    {"GiantSprucetaigaHills", 0xa1, {106,116,100}},
    {"GravellyMountainsPlus", 0xa2, {119,150,119}},
    {"ShatteredSavanna", 0xa3, {227,217,136}},
    {"ShatteredsavannaPlateau", 0xa4, {204,195,140}},
    {"ErodedBadlands", 0xa5, {251,107,65}},
    {"ModifiedWoodedBadlandsPlateau", 0xa6, {213,189,141}},
    {"ModifiedBadlandsPlateau", 0xa7, {239,176,139}},
    {"BambooJungle", 0xa8, {113,230,52}},
    {"BambooJungleHills", 0xa9, {96,197,45}},

    {"UnknownBiome", 0xff, {255, 0, 0}}
};


const Biome &get_biome(int id) {
    static bool init=false;
    static std::map<int, int> biome_map;
    if (!init) {
        for(long unsigned int i=0; i<sizeof(Biomes)/sizeof(Biomes[0]); i++) {
            biome_map[Biomes[i].id] = i;
        }
        init = true;
    }

    if (biome_map.find(id) == biome_map.end()) {
        std::cout << "don't know about biome " << id << "\n";
        return Biomes[biome_map[0xff]];
    }

    return Biomes[biome_map[id]];
}


void write_biome_properties(std::string filename)
{
    nlohmann::json top;
    for(Biome &b : Biomes) {
        top["biome_colors"][b.name].push_back(b.color.r);
        top["biome_colors"][b.name].push_back(b.color.g);
        top["biome_colors"][b.name].push_back(b.color.b);
    }

    std::ofstream file;
    file.open(filename);
    file << top.dump(2);
    file.close();
}
