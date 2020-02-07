

#ifndef POLYGON_HH
#define POLYGON_HH


#include <boost/polygon/polygon.hpp>
#include <vector>
#include <cassert>
#include <optional>

//https://github.com/nlohmann/json#json-as-first-class-data-type
// this json header is copied from the above github. I don't know how to include
// a single file as a submodule and the git report itself is pretty large.
#include <nlohmann/json.hpp>

namespace gtl = boost::polygon;
typedef gtl::polygon_90_data<int>                      Polygon;
typedef gtl::polygon_90_with_holes_data<int>           Polygon_Holes;
typedef std::vector<Polygon>                           PolygonSet;
typedef std::vector<Polygon_Holes>                     PolygonHolesSet;
//typedef gtl::polygon_traits<Polygon_Holes>::point_type Point;
typedef gtl::rectangle_data<int>                       Rect;

void add_rectangle_to_polygon_set(PolygonSet      &theset, int xl, int zl, int xh, int zh);
void add_rectangle_to_polygon_set(PolygonHolesSet &theset, int xl, int zl, int xh, int zh);


nlohmann::json polygon_to_json(Polygon &polygon);
nlohmann::json polygon_to_json(Polygon_Holes &polygon);
std::optional<nlohmann::json> polygon_to_json_simplified(Polygon_Holes &polygon);
#endif
