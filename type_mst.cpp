

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <map>

#include <MinecraftWorld.h>
#include <parse_bedrock.h>

#include <compute_mst.h>


int
main(int argc,char* argv[])
{

  MinecraftWorld world;

  //std::string dbpath = "/bubba/electronicsDS/minecraft/sampledata/2019.11.26.04.00.53/worlds/Bedrock level/db";
  std::string dbpath = "/bubba/electronicsDS/minecraft/gfhtx2/gfHTXZk9AQA/db";

  parse_bedrock(dbpath, world);

  std::map<std::string, std::string> layers = {
      {"diamond.vtk", "minecraft:diamond_ore ()"},
      {"lapis.vtk", "minecraft:lapis_ore ()"},
      {"iron.vtk",  "minecraft:iron_ore ()"},
      {"redstone.vtk", "minecraft:redstone_ore ()"},
      {"spawner.vtk", "minecraft:mob_spawner ()"},
  };


  for(auto layer : layers) {
      int ore_id = BlockType::get_block_type_id_by_name(layer.second);
      std::cout << "filtering by " << ore_id << "\n";

      std::vector<std::tuple<int, int, int> > ores;

      for (auto scix : world.theworld) {
          //int chunkx = scix.first;
          for (auto sciy : scix.second) {
              //int chunky = sciy.first;
              for (auto sciz : sciy.second) {
                  //int chunkz = sciz.first;
                  auto sc = sciz.second;

                  for (auto iter=sc->begin(ore_id); iter!=sc->end(); ++iter) {
                      auto loc = *iter;
                      ores.push_back({loc.x, loc.y, loc.z});
                  }
              }
          }
      }

      compute_mst(ores, 4, layer.first);

  }

}
