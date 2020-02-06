
#include <BlockEntityJSON.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define UNUSED(x) (void)(x)

void GenerateBlockEntityJSON(MinecraftWorld &world, std::string filename)
{

    boost::property_tree::ptree tree;

    tree.put("type", "FeatureCollection");

    boost::property_tree::ptree features;

    int num = 0;
    for (auto i1 : world.chunk_block_entities) {
        int chunkx = i1.first;
        UNUSED(chunkx);
        for(auto i2 : i1.second) {
            int chunkz = i2.first;
            UNUSED(chunkz);
            for(auto entity : i2.second) {
                num++;

                std::cout << entity.get_string() << "\n";

                boost::property_tree::ptree feature_node;

                feature_node.put("type", "Feature");
                feature_node.put("properties.type", entity.get_type());
                if (entity.get_type() == "MobSpawner") {
                    auto spawnertype = entity.get_string_property("EntityIdentifier");
                    assert(spawnertype);
                    feature_node.put("properties.spawner_type", *spawnertype);
                }


                // this seems like a goofy way to have a list.
                boost::property_tree::ptree coords;
                boost::property_tree::ptree coord;

                // the 0 coord is x, 1 coord is y (the height), 2 coord is z.
                std::vector<int> position = entity.get_position();

                // this first list is the full coordinates for reporting.
                for (int i : {0,1,2}) {
                    coord.put("", position[i]);
                    coords.push_back(std::make_pair("", coord));
                }
                feature_node.add_child("properties.coords", coords);

                feature_node.put("geometry.type", "Point");

                // this second list of coordinates is for the geojson format.
                // 0(x) and 2(z) are the normal "xy" coordinates
                coords.clear();
                for (int i : {0,2}) {
                    coord.put("", position[i]);
                    coords.push_back(std::make_pair("", coord));
                }

                feature_node.add_child("geometry.coordinates", coords);

                features.push_back(std::make_pair("", feature_node));
            }
        }
    }

    tree.add_child("features", features);

    boost::property_tree::write_json(filename, tree);

    std::cout << "wrote " << num << " block entities\n";

}
