
#include <polygon.h>


void add_rectangle_to_polygon_set(PolygonSet &theset, int xl, int zl, int xh, int zh)
{
    // we extend the center of the line, giving up a pointy diamond.
    Point pts[] = { Point{xl, zl},
                    Point{xh, zl},
                    Point{xh, zh},
                    Point{xl, zh}};

    Polygon_Holes rect;
    gtl::set_points(rect, pts, pts+sizeof(pts)/sizeof(Point));

    theset.push_back(rect);
}
