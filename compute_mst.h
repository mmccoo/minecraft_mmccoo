


#ifndef COMPUTE_MST_HH
#define COMPUTE_MST_HH

#include <vector>
#include <tuple>
#include <set>

void compute_mst(std::vector<std::tuple<int, int, int> > locs, float maxdist, std::string filename);
void compute_mst(std::vector<std::tuple<int, int, int> > locs,
                 std::vector<std::set<int> > &clusters,
                 float maxdist);
#endif
