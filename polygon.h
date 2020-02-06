

#ifndef POLYGON_HH
#define POLYGON_HH


#include <boost/polygon/polygon.hpp>
#include <vector>
#include <cassert>

namespace gtl = boost::polygon;
typedef gtl::polygon_90_data<int>           Polygon_Holes;
typedef std::vector<Polygon_Holes>                     PolygonSet;
typedef gtl::polygon_traits<Polygon_Holes>::point_type Point;
typedef gtl::rectangle_data<int>                       Rect;

void add_rectangle_to_polygon_set(PolygonSet &theset, int xl, int zl, int xh, int zh);


#endif
