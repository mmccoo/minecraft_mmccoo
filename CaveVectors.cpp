

#include <CaveVectors.h>

#include <sstream>
#include <fstream>


//https://github.com/nlohmann/json#json-as-first-class-data-type
// this json header is copied from the above github. I don't know how to include
// a single file as a submodule and the git report itself is pretty large.
#include <nlohmann/json.hpp>

CaveVectors::CaveVectors(MinecraftWorld& world)
{

    for (auto scix : world.chunks_by_y) {
        int chunkx = scix.first;
        UNUSED(chunkx);
        for (auto sciz : scix.second) {
            int chunkz = sciz.first;
            UNUSED(chunkz);

            Grid16 &chunk_top_earthly = world.top_earthly[chunkx][chunkz];

            for (auto sciy : sciz.second) {
                int chunky = sciy.first;
                UNUSED(chunky);
                auto sc = sciy.second;



                for (auto iter=sc->begin(); iter!=sc->end(); ++iter) {
                    auto loc = *iter;
                    // The loc has real world coords.
                    int rawx = loc.x - chunkx*16;
                    int rawz = loc.z - chunkz*16;

                    if (loc.y >= chunk_top_earthly[rawx][rawz]) { continue; }

                    BlockType bt = BlockType::get_block_type_by_id(loc.type);

                    if (bt.is_air() || bt.is_liquid()) {
                        add_rectangle_to_polygon_set(polysets[loc.y/world.y_resolution], loc.x, loc.z, loc.x+1, loc.z+1);
                    }
                }
            }
        }
    }
}


void CaveVectors::write(std::string filename)
{
    nlohmann::json top;

    top["type"] = "FeatureCollection";

    PolygonHolesSet prev;
    for(auto iter = polysets.rbegin(); iter!=polysets.rend(); iter++) {
        int elevation = (*iter).first;

        // we're going from upper elevations on down. each layer, merge in the polys from above.
        PolygonHolesSet &ps = (*iter).second;

        PolygonHolesSet merged;

#if 0
        prev.insert(prev.end(), ps.begin(), ps.end());
        std::cout << "for elevation: " << elevation << " we have " << prev.size() << " going in and ";
        gtl::assign(merged, prev);
        // can't I just assign above?
        prev = merged;
        std::cout << merged.size() << " coming out\n" << std::flush;
#else
        std::cout << "for elevation: " << elevation << " we have " << ps.size() << " going in and ";
        gtl::assign(merged, ps);
        std::cout << merged.size() << " coming out\n" << std::flush;
#endif

        for (auto poly : merged) {
            std::optional<nlohmann::json> coords = polygon_to_json(poly);
            if (!coords) { continue; }

            nlohmann::json feature;

            feature["type"] = "Feature";
            feature["properties"]["elevation"] = elevation;

            feature["geometry"]["type"] = "Polygon";

            feature["geometry"]["coordinates"] = *coords;

            top["features"].push_back(feature);
        }
    }

    std::ofstream file;
    file.open(filename);
    file << top.dump(2);
    file.close();

}
