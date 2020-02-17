

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <map>
#include <iomanip>
#include <algorithm>
#include <MinecraftWorld.h>
#include <parse_bedrock.h>

#include <compute_mst.h>
#include <compute_ore_stats.h>


void
write_cluster_vtk(std::vector<std::set<int> > &clusters,
                  std::vector<std::tuple<int, int, int> > &ores,
                  std::string filename)
{

    std::ofstream outfile;
    outfile.open(filename);

    outfile << "# vtk DataFile Version 1.0\n";
    outfile << "3D triangulation data\n";
    outfile << "ASCII\n";
    outfile << std::endl;
    outfile << "DATASET POLYDATA\n";



    std::vector<std::string> points;
    for (auto cluster : clusters) {
        float x=0;
        float y=0;
        float z=0;

        for (auto loc : cluster) {
            x += std::get<0>(ores[loc]);
            y += std::get<1>(ores[loc]);
            z += std::get<2>(ores[loc]);
        }
        std::stringstream ss;
        ss << x/cluster.size() << " " << y/cluster.size() << " " << z/cluster.size() << "\n";
        points.push_back(ss.str());
    }

    outfile << "POINTS " << points.size() << " float\n";
    for (auto str : points) {
        outfile << str;
    }


    outfile.close();
}

int
main(int argc,char* argv[])
{

  MinecraftWorld world;

  //std::string dbpath = "/bubba/electronicsDS/minecraft/sampledata/2019.11.26.04.00.53/worlds/Bedrock level/db";
  std::string dbpath = "/bubba/electronicsDS/minecraft/gfhtx2/gfHTXZk9AQA/db";

  parse_bedrock(dbpath, world);

  int num_chunks = world.num_chunks();

  // 16 subchunks/chunk and 16*16*16 per subchunk
  std::cout << "num chunks " << num_chunks << "\n";
  std::cout << "num blocks in world " << num_chunks*16*16*16*16 << "\n";

  int air_id = BlockType::get_block_type_id_by_name("minecraft:air ()");
  int stairs_id = BlockType::get_block_type_id_by_name("minecraft:mossy_cobblestone_stairs ()");
  std::map<std::pair<int, int>, int > top_height;

  for (auto scix : world.theworld) {
      //int chunkx = scix.first;
      for (auto sciy : scix.second) {
          //int chunky = sciy.first;
          for (auto sciz : sciy.second) {
              //int chunkz = sciz.first;
              auto sc = sciz.second;

              for (auto iter=sc->begin(); iter!=sc->end(); ++iter) {
                  auto loc = *iter;
                  uint8_t blocktype = loc.type;
                  if (blocktype == stairs_id) {
                      std::cout << "stairs at: " << loc.x << ", " << loc.y << ", " << loc.z << "\n";
                  }

                  if (blocktype == air_id) { continue; }

                  auto point = std::make_pair(loc.x, loc.z);
                  top_height[point] = std::max(top_height[point], loc.y);
              }
          }
      }
  }

  std::string filename = "tops.vtk";
  std::ofstream outfile;
  outfile.open(filename);

  outfile << "# vtk DataFile Version 1.0\n";
  outfile << "3D triangulation data\n";
  outfile << "ASCII\n";
  outfile << std::endl;
  outfile << "DATASET POLYDATA\n";


  outfile << "POINTS " << top_height.size() << " float\n";
  for (auto iter : top_height) {
      std::stringstream ss;
      ss << iter.first.first << " " << iter.first.second << " " << iter.second << "\n";
      outfile << ss.str();
  }


  outfile.close();

}
