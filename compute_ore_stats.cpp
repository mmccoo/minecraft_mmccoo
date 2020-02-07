

#include <compute_ore_stats.h>
#include <map>
#include <iomanip>


void
show_ore_stats(MinecraftWorld &world,
               std::vector<std::tuple<int, int, int> > &ores,
               int oretype
    )
{

    std::map<int, int> blocks_above;
    std::map<int, int> blocks_below;
    std::map<int, int> blocks_adjacent;
    std::map<int, int> level_hist;

    int lava = BlockType::get_block_type_id_by_name("minecraft:lava ()");

    const bool show_lava = false;

    for(auto ore : ores) {
        int x,y,z;
        std::tie(x,y,z) = ore;

        level_hist[y]++;

        int type = world.get_type_at(x, y + 1, z);
        blocks_above[type]++;
        if (show_lava && type == lava) {
            std::cout << "below lava: " << x << "," << y << "," << z << "\n";
        }


        if (y>0) {
            type = world.get_type_at(x, y - 1, z);
            blocks_below[type]++;
        }

        std::vector<std::tuple<int, int, int> > offsets = {
            {1,0,0}, {-1,0,0}, {0,0,1},{0,0,-1}
        };

        for(auto offset : offsets) {
            int type = world.get_type_at(x + std::get<0>(offset),
                                         y + std::get<1>(offset),
                                         z + std::get<2>(offset));
            blocks_adjacent[type]++;
            if (show_lava && type == lava) {
                std::cout << "next to lava: " << x << "," << y << "," << z << "\n";
            }
        }
    }

    std::cout << "level stats\n";
    for(auto lev : level_hist) {
        std::cout << "  " << std::setw(4) << lev.first << ": " << std::setw(10) << lev.second << " " << std::setprecision(5) << (double) lev.second / ores.size() * 100.0 << "%\n";
    }

    std::vector<std::pair<std::string, std::map<int, int>& > > cats = {
            {"blocks above",blocks_above},{"blocks below",blocks_below},{"blocks adjacent",blocks_adjacent}
    };
    for(auto t : cats) {
        std::cout << t.first << "\n";
        for(auto iter : t.second) {
            std::cout << "  " << BlockType::get_block_type_by_id(iter.first).get_name() << " " << iter.second << "\n";
        }
    }

}
