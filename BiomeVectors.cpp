
#include <BiomeVectors.h>
#include <biome.h>

#include <sstream>
#include <fstream>

//https://github.com/nlohmann/json#json-as-first-class-data-type
// this json header is copied from the above github. I don't know how to include
// a single file as a submodule and the git report itself is pretty large.
#include <nlohmann/json.hpp>

BiomeVectors::BiomeVectors()
{
    /* empty */
}

void BiomeVectors::add_chunk(int chunkx, int chunkz, Grid16 biomes)
{
    for(int x=0; x<16; x++) {
        for(int z=0; z<16; z++) {
            auto b = get_biome(biomes[x][z]);
            int xl = (chunkx*16+x);
            int zl = (chunkz*16+z);
            int xh = xl + 1;
            int zh = zl + 1;
            if (b.name == "") {
                std::cout << "adding blank biome\n";
            }
            add_rectangle_to_polygon_set(polysets[b.id], xl, zl, xh, zh);
        }
    }
}

void BiomeVectors::write(std::string filename)
{
    nlohmann::json top;

    top["type"] = "FeatureCollection";

    for(auto ps : polysets) {
        const Biome &b = get_biome(ps.first);

        PolygonHolesSet merged;
        gtl::assign(merged, ps.second);

        for (auto poly : merged) {

            nlohmann::json feature;

            feature["type"] = "Feature";
            feature["properties"]["biome"] = b.name;

            feature["geometry"]["type"] = "Polygon";

            feature["geometry"]["coordinates"] = polygon_to_json(poly);

            top["features"].push_back(feature);
        }
    }

    std::ofstream file;
    file.open(filename);
    file << top.dump(2);
    file.close();
}
