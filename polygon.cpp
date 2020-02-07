
#include <polygon.h>

// these are to simplify the polygons.
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>


void add_rectangle_to_polygon_set(PolygonSet &theset, int xl, int zl, int xh, int zh)
{
    typedef gtl::polygon_traits<Polygon>::point_type Point;
    // we extend the center of the line, giving up a pointy diamond.
    Point pts[] = { Point{xl, zl},
                    Point{xh, zl},
                    Point{xh, zh},
                    Point{xl, zh}};

    Polygon rect;
    gtl::set_points(rect, pts, pts+sizeof(pts)/sizeof(Point));

    theset.push_back(rect);
}

void add_rectangle_to_polygon_set(PolygonHolesSet &theset, int xl, int zl, int xh, int zh)
{
    typedef gtl::polygon_traits<Polygon_Holes>::point_type Point;
    // we extend the center of the line, giving up a pointy diamond.
    Point pts[] = { Point{xl, zl},
                    Point{xh, zl},
                    Point{xh, zh},
                    Point{xl, zh}};

    Polygon_Holes rect;
    gtl::set_points(rect, pts, pts+sizeof(pts)/sizeof(Point));

    theset.push_back(rect);
}

nlohmann::json polygon_to_json(Polygon &polygon)
{
    nlohmann::json boundary;

    nlohmann::json ring;
    std::vector<std::string> points;
    for (auto point : polygon) {
        ring.push_back({gtl::x(point), gtl::y(point)});
    }
    ring.push_back(ring.front());

    // I'm using push_back because coordinates is a list of rings/boundaries.
    // The first is the outer bounds. After that come the holes.
    boundary.push_back(ring);

    return boundary;
}

nlohmann::json polygon_to_json(Polygon_Holes &polygon)
{
    nlohmann::json boundary;

    nlohmann::json ring;
    for (auto point : polygon) {
        ring.push_back({gtl::x(point), gtl::y(point)});
    }
    ring.push_back(ring.front());

    // I'm using push_back because coordinates is a list of rings/boundaries.
    // The first is the outer bounds. After that come the holes.
    boundary.push_back(ring);

    for(auto hiter = gtl::begin_holes(polygon); hiter!= gtl::end_holes(polygon); hiter++) {
        nlohmann::json ring;
        for (auto point : *hiter) {
            ring.push_back({gtl::x(point), gtl::y(point)});
        }
        ring.push_back(ring.front());

        boundary.push_back(ring);
    }

    return boundary;
}

std::optional<nlohmann::json> polygon_to_json_simplified(Polygon_Holes &polygon)
{

    namespace bg = boost::geometry;
    typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
    typedef bg::model::polygon<point_t> polygon_t;

    polygon_t poly1;

    for (auto point : polygon) {
        bg::append(poly1.outer(), point_t(gtl::x(point), gtl::y(point)));
    }
    //bg::append(poly1.outer(), point_t(gtl::x(*polygon.begin()), gtl::y(*polygon.begin())));

    poly1.inners().resize(polygon.size_holes());
    int hole_num=0;
    for(auto hiter = gtl::begin_holes(polygon); hiter!= gtl::end_holes(polygon); hiter++, hole_num++) {
        auto &hole = *hiter;
        nlohmann::json ring;
        for (auto point : hole) {
            bg::append(poly1.inners()[hole_num], point_t(gtl::x(point), gtl::y(point)));
        }
        //bg::append(poly1.inners()[hole_num], point_t(gtl::x(*hole.begin()), gtl::y(*hole.begin())));
    }

    polygon_t poly_simple;
    boost::geometry::simplify(poly1, poly_simple, .5);

    if (poly_simple.outer().size() == 0) {
        return std::optional<nlohmann::json>();
    }

    nlohmann::json boundary;

    nlohmann::json ring;
    for(auto pt : poly_simple.outer()) {
        ring.push_back({int(bg::get<0>(pt)), int(bg::get<1>(pt))});
    }
    ring.push_back(ring.front());
    boundary.push_back(ring);

    for(auto &out : poly_simple.inners()) {
        nlohmann::json ring;
        for (auto point : out) {
            ring.push_back({int(bg::get<0>(point)), int(bg::get<1>(point))});
        }
        ring.push_back(ring.front());

        boundary.push_back(ring);
    }

    return boundary;
}
