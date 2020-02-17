
#include <MinecraftWorld.h>
#include <parse_bedrock.h>

#include <EntityJSON.h>
#include <BlockEntityJSON.h>
#include <VillageJSON.h>
#include <BiomeVectors.h>
#include <ElevationVectors.h>
#include <CaveVectors.h>
#include <ResourceVectors.h>

#include <fstream>
#include <iomanip>
#include <boost/filesystem.hpp>

#define UNUSED(x) (void)(x)

int main(int argc, char** argv) {

    MinecraftWorld world;

    //std::string dbpath = "/bubba/electronicsDS/minecraft/sampledata/2019.11.26.04.00.53/worlds/Bedrock level/db";
    //std::string dbpath = "/bubba/electronicsDS/minecraft/sampledata/2020.02.04.04.00.53/worlds/Bedrock level/db";
    // http://www.trulybedrock.com/downloads/
    //std::string dbpath = "/bubba/electronicsDS/minecraft/sampledata/TBSeason0/db";
    std::string dbpath = "/bubba/electronicsDS/minecraft/sampledata/myserver_feb_13/Bedrock level/db";
    parse_bedrock(dbpath, world);
    BlockType::print_block_types();

# if 0
    std::cout << "iterating locations\n";

    int diamond = BlockType::get_block_type_id_by_name("minecraft:diamond_ore ()");
    std::cout << "filtering by " << diamond << "\n";

    for (auto scix : world.theworld) {
        int chunkx = scix.first;
        UNUSED(chunkx);
        for (auto sciy : scix.second) {
            int chunky = sciy.first;
            UNUSED(chunky);
            for (auto sciz : sciy.second) {
                int chunkz = sciz.first;
                UNUSED(chunkz);
                auto sc = sciz.second;

                for (auto iter=sc.begin(diamond); iter!=sc.end(); ++iter) {
                    //for (auto loc : sc) {
                    auto loc = *iter;
                    BlockType bt = BlockType::get_block_type_by_id(loc.type);
                    std::cout << "at " << loc.x << ", " << loc.y << ", " << loc.z << ":" << bt.get_name() << "\n";
                }
            }
        }
    }
#endif
    {
        ResourceVectors resourcevectors(world);
        resourcevectors.write("map/resources.json");
    }

    {
        CaveVectors cavevectors(world);
        cavevectors.write("map/caves.json");
    }

    boost::filesystem::create_directory("map");

    world.write_world_json("map/world.json");
    write_biome_properties("map/biomeproperties.json");


    {
        BiomeVectors bvectors;
        for(auto it1 : world.chunk_biomes) {
            int chunkx = it1.first;
            for(auto it2 : it1.second) {
                int chunkz = it2.first;
                bvectors.add_chunk(chunkx, chunkz, it2.second);
            }
        }
        bvectors.write("map/biomes.json");
    }

    // elevation stuff generates really large files. 100MB+ for a not that large world.
    if (0) {
        ElevationVectors evectors(world);
        for(auto it1 : world.chunk_elevation) {
            int chunkx = it1.first;
            //if (chunkx%2) { continue; }
            for(auto it2 : it1.second) {
                int chunkz = it2.first;
                //if (chunkz%2) { continue; }
                evectors.add_chunk(chunkx, chunkz, it2.second);
#if 0
                std::cout << "elevations for " << chunkx << ", " << chunkz << "\n";
                for(int z=0; z<16; z++) {
                    for(int x=0; x<16; x++) {
                        std::cout << std::setw(4) << it2.second[x][z] << " ";
                    }
                    std::cout << "\n";
                }
                std::cout << "\n";
#endif
            }
        }
        evectors.write("map/elevations.json");
    }

    if (1) {
        std::cout << "computing elevations\n";
        ElevationVectors evectors(world);
        for(auto it1 : world.top_earthly) {
            int chunkx = it1.first;
            //if (chunkx%2) { continue; }
            for(auto it2 : it1.second) {
                int chunkz = it2.first;
                //if (chunkz%2) { continue; }
                evectors.add_chunk(chunkx, chunkz, it2.second);
            }
        }
        evectors.write("map/elevations.json");
    }

    world.populate_entity_table();

    GenerateVillageJSON(world, "map/villages.json");

    GenerateEntityJSON(world, "map/entities.json");

    GenerateBlockEntityJSON(world, "map/block_entities.json");

    return 0;
}
