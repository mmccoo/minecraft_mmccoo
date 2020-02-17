

#include <ResourceVectors.h>

#include <sstream>
#include <fstream>


//https://github.com/nlohmann/json#json-as-first-class-data-type
// this json header is copied from the above github. I don't know how to include
// a single file as a submodule and the git report itself is pretty large.
#include <nlohmann/json.hpp>


std::set<std::string> rare_interesting_resources = {
  "minecraft:diamond_ore",
  "minecraft:torch",
  "minecraft:emerald_ore", // not sure this is the right name.
  "minecraft:lapis_ore",
  "minecraft:bee_nest",
  "minecraft:bell",
};

ResourceVectors::ResourceVectors(MinecraftWorld& world)
{
    for (auto scix : world.chunks_by_y) {
        int chunkx = scix.first;
        UNUSED(chunkx);
        for (auto sciz : scix.second) {
            int chunkz = sciz.first;
            UNUSED(chunkz);

            auto &te = world.top_earthly[chunkx][chunkz];

            for (auto sciy : sciz.second) {
                int chunky = sciy.first;
                UNUSED(chunky);
                auto sc = sciy.second;

                for (auto iter=sc->begin(); iter!=sc->end(); ++iter) {
                    auto loc = *iter;

                    BlockType bt = BlockType::get_block_type_by_id(loc.type);
                    if (rare_interesting_resources.find(bt.get_name()) == rare_interesting_resources.end()) {
                        continue;
                    }

                    int rawx = loc.x - chunkx*16;
                    int rawz = loc.z - chunkz*16;

                    resources.push_back({loc.x, loc.y, loc.z, bt, loc.y > te[rawx][rawz]});
                }
            }
        }
    }
}


void ResourceVectors::write(std::string filename)
{
    nlohmann::json top;

    top["type"] = "FeatureCollection";

    for(auto res : resources) {
        nlohmann::json feature;
        feature["type"] = "Feature";
        feature["properties"]["coords"] = { res.x, res.y, res.z };
        feature["properties"]["name"] = res.bt.get_name();
        feature["properties"]["surface"] = res.isSurface;

        feature["geometry"]["type"] = "Point";

        feature["geometry"]["coordinates"] = { res.x, res.z };

        top["features"].push_back(feature);
    }

    std::ofstream file;
    file.open(filename);
    file << top.dump(2);
    file.close();

}
