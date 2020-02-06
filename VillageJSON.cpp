
#include <VillageJSON.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define UNUSED(x) (void)(x)

void GenerateVillageJSON(MinecraftWorld &world, std::string filename)
{

    boost::property_tree::ptree tree;

    tree.put("type", "FeatureCollection");

    boost::property_tree::ptree features;

    for(auto it : world.villages) {
        VillageType &village = it.second;

        std::cout << "writing village json\n";
        std::cout << village.get_string() << "\n";
        std::cout << "dwellers:\n";
        std::map<std::string, int> dcount;
        for(long villagerid : village.get_dwellers()) {
            std::optional<EntityType *> dweller = world.get_entity_by_id(villagerid);
            if (!dweller) {
                std::cout << "couldn't find dweller with id: " << villagerid << "\n";
                continue;
            }

            std::cout << "  " << villagerid << " " << (*dweller)->get_type() << "\n";
            dcount[(*dweller)->get_type()]++;
        }


        boost::property_tree::ptree feature_node;

        feature_node.put("type", "Feature");
        feature_node.put("properties.name", village.get_name());

        for (auto it : dcount) {
            feature_node.put("properties.counts." + it.first, it.second);
        }


        // this seems like a goofy way to have a list.
        boost::property_tree::ptree coords;
        boost::property_tree::ptree coord;

        auto bbox = village.get_bbox();

        coords.clear();
        for(int i : {bbox.xl, bbox.yl, bbox.zl, bbox.xh, bbox.yh, bbox.zh}) {
            coord.put("", i);
            coords.push_back(std::make_pair("", coord));
        }
        feature_node.add_child("properties.bounds", coords);

        std::vector<std::pair<int,int> > rectpts = {
            { bbox.xl, bbox.zl },
            { bbox.xh, bbox.zl },
            { bbox.xh, bbox.zh },
            { bbox.xl, bbox.zh },
            { bbox.xl, bbox.zl }
        };

        boost::property_tree::ptree polygon;
        for (std::pair<int,int> c : rectpts) {
            coords.clear();
            coord.put("", c.first);
            coords.push_back(std::make_pair("", coord));
            coord.put("", c.second);
            coords.push_back(std::make_pair("", coord));

            polygon.push_back(std::make_pair("", coords));
        }

        // geojson wants an array of first the polygon followed by the holes.
        // in our case, we have no holes.
        boost::property_tree::ptree polyholepair;
        polyholepair.push_back(std::make_pair("", polygon));

        feature_node.put("geometry.type", "Polygon");
        feature_node.add_child("geometry.coordinates", polyholepair);



        features.push_back(std::make_pair("", feature_node));
    }


    tree.add_child("features", features);

    boost::property_tree::write_json(filename, tree);

    std::cout << "wrote " << world.villages.size() << " block entities\n";

}
