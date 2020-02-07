
#include <ElevationVectors.h>
#include <biome.h>

#include <sstream>
#include <fstream>

// this file is basically the same as biome vectors. They should be merged.

ElevationVectors::ElevationVectors()
{
    /* empty */
}

void ElevationVectors::add_chunk(int chunkx, int chunkz, Grid16 elevation)
{
    for(int x=0; x<16; x++) {
        for(int z=0; z<16; z++) {
            auto e = elevation[x][z];
            int xl = (chunkx*16+x);
            int zl = (chunkz*16+z);
            int xh = xl + 1;
            int zh = zl + 1;
            add_rectangle_to_polygon_set(polysets[e], xl, zl, xh, zh);
        }
    }
}


void ElevationVectors::write(std::string filename)
{
    nlohmann::json top;

    top["type"] = "FeatureCollection";

    PolygonHolesSet prev;
    for(auto iter = polysets.rbegin(); iter!=polysets.rend(); iter++) {
        //for(auto ps : polysets) {
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
            // if a poly simplifies to nothing.
#if 0
            std::optional<nlohmann::json> coords = polygon_to_json_simplified(poly);
#else
            std::optional<nlohmann::json> coords = polygon_to_json(poly);
#endif
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
