

#ifndef COMPUTE_ORE_STATS
#define COMPUTE_ORE_STATS

#include <MinecraftWorld.h>
#include <vector>

void
show_ore_stats(MinecraftWorld &world,
               std::vector<std::tuple<int, int, int> > &ores,
               int oretype);


#endif
