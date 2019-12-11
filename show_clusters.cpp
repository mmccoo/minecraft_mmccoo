

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

bool
adjacent_to_type(MinecraftWorld &world,
                std::vector<std::tuple<int, int, int> > &ores,
                 std::set<int> &cluster,
                 int othertype
    )
{
    for(int id : cluster) {
        int x,y,z;
        std::tie(x,y,z) = ores[id];

        std::vector<std::tuple<int, int, int> > offsets = {
            {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1},{0,0,-1}
        };

        for(auto offset : offsets) {
            int type = world.get_type_at(x + std::get<0>(offset),
                                         y + std::get<1>(offset),
                                         z + std::get<2>(offset));

            if (type == othertype) {
                return true;
            }

        }
    }

    return false;
}

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

  std::map<std::string, std::string> layers = {
      {"diamond_cluster.vtk", "minecraft:diamond_ore ()"},
      {"lapis_cluster.vtk", "minecraft:lapis_ore ()"},
      {"iron_cluster.vtk",  "minecraft:iron_ore ()"},
      {"redstone_cluster.vtk", "minecraft:redstone_ore ()"},
      {"spawner_cluster.vtk", "minecraft:mob_spawner ()"},
  };

  int air_id = BlockType::get_block_type_id_by_name("minecraft:air ()");

  std::map<std::string, std::map<int, int> > cluster_stats;
  int max_cluster_size = 0;

  std::map<std::string, std::map<int, int> > level_stats;
  std::map<std::string, int> totals;
  int max_level = 0;

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

                  for (auto iter=sc.begin(ore_id); iter!=sc.end(); ++iter) {
                      auto loc = *iter;
                      ores.push_back({loc.x, loc.y, loc.z});
                      level_stats[layer.second][loc.y]++;
                      totals[layer.second]++;
                      max_level = std::max(max_level, loc.y);
                  }
              }
          }
      }

      std::vector<std::set<int> > clusters;
      compute_mst(ores, clusters, 2);
      std::cout << "Total number ores: " << ores.size() << " in " << clusters.size() << " clusters\n";

      write_cluster_vtk(clusters, ores, layer.first);


      int nexttoair=0;
      int ores_represented=0;
      std::map<int, int> sizehist;

      // found_via_tunnel[l][p]
      // where l is the layer of your feet.
      // p is the pitch/spacing of the tunnels. p=3 means you go down x=0,2,5,8...
      std::map<int, std::map<int, int> > found_via_tunnel;


      for (auto cluster : clusters) {
          sizehist[cluster.size()]++;
          if (adjacent_to_type(world, ores, cluster, air_id)) {
              nexttoair++;
              ores_represented += cluster.size();

              // for(auto id : cluster) {
              //     int x,y,z;
              //     std::tie(x,y,z) = ores[id];
              //     //std::cout << "   (" << x << ", " << y << ", " << z << "\n";
              //}
          } else {
              // std::cout << "not adjacent\n";
              // for(auto id : cluster) {
              //     int x,y,z;
              //     std::tie(x,y,z) = ores[id];
              //     std::cout << "   (" << x << ", " << y << ", " << z << "\n";
              //}
          }
      }


      std::vector<int> levels = {2,3,4,5,6,7,8,9,10,11,12,13,14,15};

      // 0 pitch means you tunnel every row.   100% digging
      // 1 pitch means you tunnel every other row. 50& digging
      // 2 pitch means you skip two inbetween each. This seems to be the most commonly mentioned one.

      // let's say there's a diamond at location 123 and your tunnel pitch is 3...
      //           _XXX_XXX_XXX_XXX_XXX_X
      //                     1         2
      //           0123456789012345678901
      //
      // x%(3+1)   0123012301230123012301
      //
      // so x%(pitch+1) will be == 0 for x's 0,4,8,12,16 --> the diamond with be in front of you.
      //    x%(pitch+1) will be == 1 for x's 1,5,9,13,13,17,21 -> the diamond is to the right of a tunnel
      //    x%(pitch+1) will be == 3(AKA pitch) for x's 3,7,11,15,19 -> the diamond is the left of a tunnel.
      // other values will be missed.

      std::vector<int> pitches = {0,1,2,3,4,5,6,7,8,9,10};


      for(int level : levels) {
          for(int pitch : pitches) {
              for (auto cluster : clusters) {
                  bool would_be_found=false;
                  for(int ore : cluster) {
                      int x,y,z;
                      std::tie(x,y,z) = ores[ore];

                      // is the ore to the left or right?
                      int loc_in_pitch = x%(pitch+1);
                      if ((level == y) &&
                          ((loc_in_pitch == 0) ||  // the ore will be in front of you
                           (loc_in_pitch == 1) ||  // the ore will be to the right
                           (loc_in_pitch == (pitch-1)))) { // the ore with be to the left.
                          would_be_found = true;
                          break;
                      }

                      // the diamond needs in your tunnel row and somewhere between one below your feet upto
                      // two above your feet.
                      if ((loc_in_pitch == 0) &&
                          (y >= (level-1)) &&
                          (y <= (level+2))) {
                          would_be_found = true;
                          break;
                      }

                  } // cluster
                  if (would_be_found) {
                      found_via_tunnel[level][pitch] += cluster.size();
                  }
              } // clusters

          } // pitches
      } // levels
      std::cout << std::setprecision(4);


      std::cout << "               % of the two layers you have to dig\n";

      std::cout << "pitch/level, ";
      for(int pitch : pitches) {
          std::cout << std::setw(6) << pitch << "/" << std::setw(6) << 1.0 / (pitch+1.0) * 100.0 << ",";
      }
      std::cout << "\n";


      for(int level : levels) {
          std::cout << "level " << std::setw(6) << level << ",";
          for(int pitch : pitches) {
              double percent_found = (double) found_via_tunnel[level][pitch] / ores.size();

              // let's say there is a total of D diamonds and a total of B blocks per level.
              // when tunnelling you have to tunnel 2 levels.
              // so the number of blocks dug is 2B/(pitch+1.0)
              // The number of diamonds found is D*%found
              // so work/diamond is: 2B/(pitch+1.0) * 1/(D*%found)
              // or          2B
              //    ---------------------
              //    (pitch+1.0)*D*%found
              // We'll drop 2B and D

              // D is equal to ores.size()
              // B is equal to num_chunks*16*16
              double work_per_diamond = (2.0*num_chunks*16*16) / (pitch+1.0) / (percent_found * ores.size());

              std::cout << std::setw(6) << 100.0*percent_found << "/";
              std::cout << std::setw(6) << work_per_diamond << ",";
          }
          std::cout << "\n";
      }
      std::cout << "\n";

      std::cout << "Cluster distribution:\n";
      for(auto h : sizehist) {
          std::cout << "  " << h.first << ": " << h.second << "\n";
          cluster_stats[layer.second][h.first] = h.second;
          max_cluster_size = std::max(max_cluster_size, h.first);
      }

      std::cout << "out of " << clusters.size() << " clusters, " << nexttoair <<
          " are next to air representing " << ores_represented << " ores\n";


      show_ore_stats(world, ores, ore_id);
  }

  for(auto iter : cluster_stats) {
      std::cout << iter.first << ", ";
  }
  std::cout << "\n";

  for(int i=1; i <= max_cluster_size; i++) {
      std::cout << i;
      for(auto iter : cluster_stats) {
          std::cout << ", " << iter.second[i];
      }
      std::cout << "\n";
  }

  for(auto iter : level_stats) {
      std::cout << iter.first << "(" << totals[iter.first] << "),";
  }
  std::cout << "\n";

  for(int i=0; i<max_level; i++) {
      std::cout << i;
      for (auto iter : level_stats) {
          std::cout << ", " << iter.second[i] << "(" << 100.0*(double)iter.second[i]/totals[iter.first] << "%)";
      }
      std::cout << "\n";
  }
}
