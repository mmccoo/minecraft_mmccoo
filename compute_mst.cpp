
#include <compute_mst.h>

#include <fstream>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <type_traits>



#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/connected_components.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_data_structure_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_3<K>                   Triangulation;
typedef Triangulation::Point                                Point;

#define UNUSED(x) (void)(x)


// using a global for this is not good. I really should be passing it
// some other way.
double scale_z=1.0;

// these two functions are for my readvtk function.
Triangulation::Vertex_handle
add_vertex(Triangulation &T, double x, double y, double z)
{
  return T.insert(Point(x,y,z));
}

void add_edge(Triangulation &T,
               Triangulation::Vertex_handle s,
               Triangulation::Vertex_handle t)
{
  // no edges to add. CGAL does that.
}

struct VertexData {
  std::string name;
  double x,y,z;
};

struct EdgeData {
  double distance;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              VertexData,
                              boost::property<boost::edge_weight_t, double, EdgeData>
                              > MyGraphType;



// these are needed by readvtk
typedef typename boost::graph_traits<MyGraphType>::vertex_descriptor vertex_descriptor;
typedef typename boost::graph_traits<MyGraphType>::edge_descriptor   edge_descriptor;
typename boost::graph_traits<MyGraphType>::vertex_descriptor
add_vertex(MyGraphType &G, std::string& vname, double x, double y, double z)
{
  typedef typename boost::graph_traits<MyGraphType>::vertex_descriptor vertex_descriptor;
  vertex_descriptor v = add_vertex(G);
  G[v].x = x;
  G[v].y = y;
  G[v].z = z;
  return v;
}

inline
double distance(MyGraphType &G,
                typename boost::graph_traits<MyGraphType>::vertex_descriptor v1,
                typename boost::graph_traits<MyGraphType>::vertex_descriptor v2)
{
  return sqrt((G[v1].x - G[v2].x)*(G[v1].x - G[v2].x) +
              (G[v1].y - G[v2].y)*(G[v1].y - G[v2].y) +
              (G[v1].z - G[v2].z)*(G[v1].z - G[v2].z));
}


typename boost::graph_traits<MyGraphType>::edge_descriptor
add_edge(MyGraphType &G,
         typename boost::graph_traits<MyGraphType>::vertex_descriptor v1,
         typename boost::graph_traits<MyGraphType>::vertex_descriptor v2)
{
  typedef typename boost::graph_traits<MyGraphType>::edge_descriptor edge_descriptor;
  edge_descriptor e = add_edge(v1, v2, G).first;
  boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, G);
  weightmap[e] = distance(G, v1, v2);
  return e;
}


enum MSTAlgorithm { PRIM, KRUSKAL };
std::istream& operator>>(std::istream& in, MSTAlgorithm &format)
{
    std::string token;
    in >> token;
    if (token == "prim")
        format = PRIM;
    else if (token == "kruskal")
        format = KRUSKAL;
    else
        in.setstate(std::ios_base::failbit);
    return in;
}




void compute_mst(std::vector<std::tuple<int, int, int> > locs, std::vector<std::set<int> > &clusters, float maxdist)
{

    Triangulation T;

    for(auto loc : locs) {
        int x,y,z;
        std::tie(x,y,z) = loc;
        add_vertex(T, x, y, z);
    }

    assert( T.is_valid() ); // checking validity of T

    // at this point, the points have been loaded and the triangulation is done.

    MyGraphType G;

    std::map<Point, MyGraphType::vertex_descriptor> vertex_map;

    Triangulation::Finite_vertices_iterator viter;
    for (viter =  T.finite_vertices_begin();
         viter != T.finite_vertices_end();
         viter++) {
        Triangulation::Triangulation_data_structure::Vertex v = *viter;
        Point p = v.point();
        double x = CGAL::to_double(p.x());
        double y = CGAL::to_double(p.y());
        double z = CGAL::to_double(p.z());
        std::string d("");
        auto boost_vertex = vertex_map[v.point()] = add_vertex(G, d, x,y,z);
        UNUSED(boost_vertex);
    }

    Triangulation::Finite_edges_iterator iter;
    for(iter =  T.finite_edges_begin();
        iter != T.finite_edges_end();
        iter++) {
        // edges are not represented as edges in CGAL triangulation graphs.
        // Instead, they are stored in faces/cells.

        Triangulation::Triangulation_data_structure::Edge e = *iter;
        Triangulation::Triangulation_data_structure::Cell_handle c = e.first;
        int i = e.second;
        int j = e.third;
        auto boost_edge = add_edge(G, vertex_map[c->vertex(i)->point()], vertex_map[c->vertex(j)->point()]);
        UNUSED(boost_edge);
    }



    boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> CGraph;


    std::vector<vertex_descriptor> mst_prim(num_vertices(G));

    // the not particularly helpful doc for iterator_property_map:
    // http://www.boost.org/doc/libs/1_64_0/libs/property_map/doc/iterator_property_map.html
    // iterator_property_map<RandomAccessIterator, OffsetMap, T, R>
    //
    typedef boost::property_map<MyGraphType, boost::vertex_index_t>::type IdMap;
    boost::iterator_property_map<std::vector<vertex_descriptor>::iterator,
                                 IdMap,
                                 vertex_descriptor,
                                 vertex_descriptor&>
        predmap(mst_prim.begin(), get(boost::vertex_index, G));

    boost::prim_minimum_spanning_tree(G, predmap);

    boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, G);

    // I'm not merging this for loop with the one right after because
    // I don't want to double add vertices. adding an edge implicitly adds a vertex.
    // if there are blocks that are loners not in clusters (like spawners)
    // adding vertices here will ensure they are represented in the connected
    // components stuff.
    for(long unsigned int i=0; i<num_vertices(G); i++) {
        boost::add_vertex(CGraph);
    }

    for(long unsigned int i=0; i<num_vertices(G); i++) {

        if (i == mst_prim[i]) {
            //std::cerr << "skipping " << i << std::endl;
            continue;
        }
        auto theedge = edge(i, mst_prim[i], G);
        if (theedge.second) {
            //std::cout << "length " << weightmap[theedge.first] << "\n";
            if (weightmap[theedge.first] > maxdist) {
                continue;
            }
        } else {
            std::cout << "unexpected edge missing\n";
        }

        boost::add_edge(i, mst_prim[i], CGraph);
    }

    std::vector<int> component(num_vertices(CGraph));
    int num = connected_components(CGraph, &component[0]);

    //std::cout << "Total number of clusters: " << num << std::endl;

    clusters.resize(0);
    clusters.resize(num);
    std::map<int, int> compsize;
    for(std::vector<int>::size_type i=0; i<component.size(); i++) {
        clusters[component[i]].insert(i);
    }
}


void compute_mst(std::vector<std::tuple<int, int, int> > locs, float maxdist, std::string filename)
{

    Triangulation T;

    for(auto loc : locs) {
        int x,y,z;
        std::tie(x,y,z) = loc;
        add_vertex(T, x, y, z);
    }

      assert( T.is_valid() ); // checking validity of T

  // at this point, the points have been loaded and the triangulation is done.

  MyGraphType G;

  std::map<Point, MyGraphType::vertex_descriptor> vertex_map;

  Triangulation::Finite_vertices_iterator viter;
  for (viter =  T.finite_vertices_begin();
       viter != T.finite_vertices_end();
       viter++) {
    Triangulation::Triangulation_data_structure::Vertex v = *viter;
    Point p = v.point();
    double x = CGAL::to_double(p.x());
    double y = CGAL::to_double(p.y());
    double z = CGAL::to_double(p.z());
    std::string d("");
    auto boost_vertex = vertex_map[v.point()] = add_vertex(G, d, x,y,z);
    UNUSED(boost_vertex);

  }

  Triangulation::Finite_edges_iterator iter;
  for(iter =  T.finite_edges_begin();
      iter != T.finite_edges_end();
      iter++) {
    // edges are not represented as edges in CGAL triangulation graphs.
    // Instead, they are stored in faces/cells.

    Triangulation::Triangulation_data_structure::Edge e = *iter;
    Triangulation::Triangulation_data_structure::Cell_handle c = e.first;
    int i = e.second;
    int j = e.third;
    auto boost_edge = add_edge(G, vertex_map[c->vertex(i)->point()], vertex_map[c->vertex(j)->point()]);
    UNUSED(boost_edge);
  }




  std::cerr << "running prim" << std::endl;
  std::vector<vertex_descriptor> mst_prim(num_vertices(G));


  std::ofstream outfile;
  outfile.open(filename);

  outfile << "# vtk DataFile Version 1.0\n";
  outfile << "3D triangulation data\n";
  outfile << "ASCII\n";
  outfile << std::endl;
  outfile << "DATASET POLYDATA\n";

  outfile << "POINTS " << num_vertices(G) << " float\n";
  for(long unsigned int i=0; i<num_vertices(G); i++) {
    outfile << G[i].x  << " " << G[i].y << " " << G[i].z << std::endl;
  }

  std::vector<std::pair<int, int> > edges_to_include;
  boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> CGraph;

  //if (mst_algorithm == PRIM) {
  if (1) {

    // the not particularly helpful doc for iterator_property_map:
    // http://www.boost.org/doc/libs/1_64_0/libs/property_map/doc/iterator_property_map.html
    // iterator_property_map<RandomAccessIterator, OffsetMap, T, R>
    //
    typedef boost::property_map<MyGraphType, boost::vertex_index_t>::type IdMap;
    boost::iterator_property_map<std::vector<vertex_descriptor>::iterator,
                                 IdMap,
                                 vertex_descriptor,
                                 vertex_descriptor&>
      predmap(mst_prim.begin(), get(boost::vertex_index, G));

    boost::prim_minimum_spanning_tree(G, predmap);

    boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, G);

    for(long unsigned int i=0; i<num_vertices(G); i++) {
      if (i == mst_prim[i]) {
        std::cerr << "skipping " << i << std::endl;
        continue;
      }
      auto theedge = edge(i, mst_prim[i], G);
      if (theedge.second) {
          //std::cout << "length " << weightmap[theedge.first] << "\n";
          if (weightmap[theedge.first] > maxdist) {
              continue;
          }
      } else {
          std::cout << "unexpected edge missing\n";
      }

      edges_to_include.push_back({i, mst_prim[i]});
      boost::add_edge(i, mst_prim[i], CGraph);
    }

  } else {
    //if (mst_algorithm == KRUSKAL) {
    std::cerr << "running kruskal" << std::endl;
    std::list<boost::graph_traits<MyGraphType>::edge_descriptor> mst_kruskal;
    boost::kruskal_minimum_spanning_tree(G, std::back_inserter(mst_kruskal));

    for(auto iter=mst_kruskal.begin();
        iter != mst_kruskal.end();
        iter++) {
      outfile << "2 " << source(*iter, G) << " " << target(*iter, G) <<std::endl;
    }
  }

  outfile << "LINES " << edges_to_include.size() << " " << edges_to_include.size()*3 << std::endl;
  for(auto line : edges_to_include) {
      outfile << "2 " << line.first << " " << line.second << std::endl;
  }


  outfile.close();


  std::vector<int> component(num_vertices(CGraph));
  int num = connected_components(CGraph, &component[0]);

  std::cout << "Total number of clusters: " << num << " representing " << num_vertices(CGraph) << "ores\n";

  std::map<int, int> compsize;
  for(std::vector<int>::size_type i=0; i<component.size(); i++) {
      compsize[component[i]]++;
  }

  std::map<int, int> sizehist;
  for(std::vector<int>::size_type i=0; i<compsize.size(); i++) {
      sizehist[compsize[i]]++;
  }

  for(auto cl : sizehist) {
      std::cout << "there are " << cl.second << " clusters of size " << cl.first << "\n";
  }
}
