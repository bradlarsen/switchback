#include <boost/timer.hpp>

#include <iostream>
#include <fstream>

#include "search/BucketPriorityQueue.hpp"
#include "search/astar/AStar.hpp"
#include "search/hastar/HAStar.hpp"
#include "search/switchback/Switchback.hpp"
#include "tiles/Tiles.hpp"

using namespace std;
using namespace boost;


typedef AStar<TilesInstance15, TilesNode15> TilesAStar;
typedef HAStar<TilesInstance15, TilesNode15> TilesHAStar;
typedef Switchback<TilesInstance15, TilesNode15> TilesSwitchback;


void print_build_info(ostream &o)
{
  o << "tiles state hash caching is "
#ifdef CACHE_TILES_HASH_VALUE
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "tiles state blank index caching is "
#ifdef CACHE_TILES_BLANK_INDEX
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "node f-value caching is "
#ifdef CACHE_NODE_F_VALUE
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "Hierarchical A*'s H* caching is "
#ifdef HIERARCHICAL_A_STAR_CACHE_H_STAR
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "Hierarchical A*'s P-g caching is "
#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "Hierarchical A*'s optimal path caching is "
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "NDEBUG is "
#ifdef NDEBUG
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif
} 


void print_size_info(ostream &o)
{
  o << "sizeof(unsigned) is " << sizeof(unsigned) << endl
    << "sizeof(Tile) is " << sizeof(Tile) << endl
    << "sizeof(TileIndex) is " << sizeof(TileIndex) << endl
    << "sizeof(TileArray) is " << sizeof(TileArray) << endl
    << "sizeof(TileCost) is " << sizeof(TileCost) << endl
    << "sizeof(TilesState15) is " << sizeof(TilesState15) << endl
    << "sizeof(TilesNode15) is " << sizeof(TilesNode15) << endl
    << "sizeof(size_t) is " << sizeof(size_t) << endl
    << "sizeof(BucketPriorityQueue<TilesNode15>::ItemPointer) is "
    << sizeof(BucketPriorityQueue<TilesNode15>::ItemPointer) << endl;
}


void print_usage(ostream &o, const char *prog_name)
{
  o << "usage: " << prog_name << " ALGORITHM [FILE]" << endl
    << "where" << endl
    << "  ALGORITHM is one of {astar, hastar, switchback}" << endl
    << "  FILE is the optional instance file to read from" << endl
    << endl
    << "If no file is specified, the instance is read from stdin." << endl;

  o << endl << endl;

  o << "Build Information:" << endl;
  print_build_info(o);
  o << endl;
  print_size_info(o);
  o << endl;
}


template <class Searcher>
void search(Searcher &searcher)
{
  timer search_timer;
  searcher.search();

  const TilesNode15 *goal = searcher.get_goal();
  if (goal == NULL) {
    cout << "no solution found!" << endl;
  }
  else {
    cout << "found a solution:" << endl;
    cout << *goal << endl;
    assert(goal->num_nodes_to_start() == goal->get_g() + 1u);
  }

  const double seconds_elapsed = search_timer.elapsed();
  const unsigned exp_per_second = searcher.get_num_expanded() / seconds_elapsed;
  const unsigned gen_per_second = searcher.get_num_expanded() / seconds_elapsed;

  cout << searcher.get_num_expanded() << " expanded ("
       << exp_per_second << "/s)" << endl
       << searcher.get_num_generated() << " generated ("
       << gen_per_second << "/s)" << endl
       << search_timer.elapsed() << "s" << endl;
}


int main(int argc, char * argv[])
{
  if (argc < 2 || argc > 3) {
    print_usage(cerr, argv[0]);
    return 1;
  }

  TilesInstance15 *instance;
  if (argc == 3) {
    ifstream infile(argv[2]);
    instance = readTilesInstance15(infile);
  }
  else {
    instance = readTilesInstance15(cin);
  }

  if (instance == NULL) {
    cerr << "error reading instance!" << endl;
    return 1;
  }

  cout << "######## The Instance ########" << endl;
  cout << *instance << endl;
  cout << endl;

  cout << "######## Search Results ########" << endl;

  const string alg_string(argv[1]);
  if (alg_string == "astar") {
    TilesAStar &astar = *new TilesAStar(*instance);
    search(astar);
  }
  else if (alg_string == "hastar") {
    TilesHAStar &hastar = *new TilesHAStar(*instance);
    search(hastar);
  }
  else if (alg_string == "switchback") {
    TilesSwitchback &switchback = *new TilesSwitchback(*instance);
    search(switchback);
  }
  else {
    cerr << "error: invalid algorithm specified" << endl;
    print_usage(cerr, argv[0]);
    return 1;
  }

  return 0;
}
