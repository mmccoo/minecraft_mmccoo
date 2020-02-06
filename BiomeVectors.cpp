
#include <BiomeVectors.h>
#include <biome.h>

#include <sstream>
#include <fstream>

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


// https://codereview.stackexchange.com/a/142907
template<typename InputIt>
std::string join(InputIt first,
    InputIt last,
    const std::string& separator = "",
    const std::string& concluder = "")
{
    if (first == last)
    {
        return concluder;
    }

    std::stringstream ss;
    ss << *first;
    ++first;

    while (first != last)
    {
        ss << separator;
        ss << *first;
        ++first;
    }

    ss << concluder;

    return ss.str();
}
void BiomeVectors::write(std::string filename)
{

    std::ofstream file;
    file.open(filename);

    int minx;
    int minz;
    int maxx;
    int maxz;

    bool firstps = true;
    for(auto ps : polysets) {
        Rect rect;
        gtl::extents(rect, ps.second);

        if (firstps) {
            minx = maxx = gtl::xl(rect);
            minz = maxz = gtl::yl(rect);
            firstps = false;
        } else {
            minx = std::min(minx, gtl::xl(rect));
            minz = std::min(minz, gtl::yl(rect));
            maxx = std::max(maxx, gtl::xh(rect));
            maxz = std::max(maxz, gtl::yh(rect));
        }
    }

    std::cout << "vector bounds: " << minx << ", " << minz << ", " << maxx << ", " << maxz << "\n";

    file << "{\n";
    file << "  \"type\": \"FeatureCollection\",\n";
    file << "  \"features\": [\n";

    firstps = true;
    for(auto ps : polysets) {
        const Biome &b = get_biome(ps.first);

        PolygonSet merged;
        gtl::assign(merged, ps.second);

        if (firstps) {
            firstps = false;
        } else {
            file << ",\n";
        }

        std::vector<std::string> polystrings;
        for (auto poly : merged) {

            std::stringstream polyss;
            polyss << "    {\n";
            polyss << "      \"type\": \"Feature\",\n";
            polyss << "      \"properties\": {\n";
            if (b.name == "") {
                std::cout << "blank biome\n";
            }
            polyss << "        \"biome\": \"" << b.name << "\"\n";
            polyss << "      },\n";

            polyss << "      \"geometry\": {\n";
            polyss << "        \"type\": \"Polygon\",\n";

            // you need two brackets. the first polygon is the outline. the ones after are holes.
            polyss << "        \"coordinates\": [\n";

            std::vector<std::string> points;
            for (auto point : poly) {
                std::stringstream ss;
                // need to flip top/bottom and shift to get all of what used to be positive back to positive.
                ss << "            [" <<  gtl::x(point) << ", " << gtl::y(point) << "]";
                points.push_back(ss.str());
            }
            points.push_back(points.front());

            polyss << "          [\n";
            // reverse order of what gtl gives.
            polyss << join(points.rbegin(), points.rend(), ",\n", "\n");
            polyss << "          ]\n";
            polyss << "        ]\n";


            // close geometry
            polyss << "      }\n";

            // close the poly
            polyss << "    }";

            polystrings.push_back(polyss.str());
        }

        file << join(polystrings.begin(), polystrings.end(), ",\n", "");
    }

    file << "\n";

    file << "  ]\n";
    file << "}\n";

    file.close();
}
