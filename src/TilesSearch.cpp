#include <boost/timer.hpp>

#include <iostream>
#include <fstream>

#include "search/BucketPriorityQueue.hpp"
#include "search/Constants.hpp"
#include "search/astar/AStar.hpp"
#include "search/hastar/HAStar.hpp"
#include "search/switchback/Switchback.hpp"
#include "tiles/Tiles.hpp"
#include "tiles/MacroTiles.hpp"

using namespace std;
using namespace boost;


typedef AStar<TilesInstance15, TilesNode15> TilesAStar;
typedef HAStar<TilesInstance15, TilesNode15> TilesHAStar;
typedef Switchback<TilesInstance15, TilesNode15> TilesSwitchback;

typedef AStar<MacroTilesInstance15, TilesNode15> MacroTilesAStar;
typedef HAStar<MacroTilesInstance15, TilesNode15> MacroTilesHAStar;
typedef Switchback<MacroTilesInstance15, TilesNode15> MacroTilesSwitchback;


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

  o << "OUTPUT_SEARCH_PROGRESS is "
#ifdef OUTPUT_SEARCH_PROGRESS
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "INITIAL_CLOSED_SET_SIZE is " << INITIAL_CLOSED_SET_SIZE << endl;
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
  o << "usage: " << prog_name << " DOMAIN ALGORITHM [FILE]" << endl
    << "where" << endl
    << "  DOMAIN is one of {tiles, macro_tiles}" << endl
    << "  ALGORITHM is one of {astar, hastar, switchback, hidastar}" << endl
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


TilesInstance15 * get_tiles_instance(int argc, char *argv[])
{
  TilesInstance15 *instance;
  if (argc == 4) {
    ifstream infile(argv[3]);
    instance = readTilesInstance15(infile);
  }
  else {
    instance = readTilesInstance15(cin);
  }

  if (instance == NULL) {
    cerr << "error reading instance!" << endl;
    exit(1);
  }

  cout << "######## The Instance ########" << endl;
  cout << *instance << endl;
  cout << endl;

  return instance;
}


MacroTilesInstance15 * get_macro_tiles_instance(int argc, char *argv[])
{
  return new MacroTilesInstance15(get_tiles_instance(argc, argv));
}


template <class Searcher>
void search(Searcher &searcher)
{
  cout << "######## Search Results ########" << endl;

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
  if (argc < 3 || argc > 4) {
    print_usage(cerr, argv[0]);
    return 1;
  }

  const string domain_string(argv[1]);
  const string alg_string(argv[2]);

  const bool is_tiles = domain_string == "tiles";
  const bool is_macro_tiles = domain_string == "macro_tiles";

  const bool is_astar = alg_string == "astar";
  const bool is_hastar = alg_string == "hastar";
  const bool is_hidastar = alg_string == "hidastar";
  const bool is_switchback = alg_string == "switchback";
  
  if (is_tiles && is_astar) {
    TilesInstance15 *instance = get_tiles_instance(argc, argv);
    TilesAStar &astar = *new TilesAStar(*instance);
    search(astar);
  }
  else if (is_tiles && is_hastar) {
    TilesInstance15 *instance = get_tiles_instance(argc, argv);
    TilesHAStar &hastar = *new TilesHAStar(*instance);
    search(hastar);
  }
  else if (is_tiles && is_hidastar) {
    cerr << "HIDA* unimplemented" << endl;
    exit(1);
  }
  else if (is_tiles && is_switchback) {
    TilesInstance15 *instance = get_tiles_instance(argc, argv);
    TilesSwitchback &switchback = *new TilesSwitchback(*instance);
    search(switchback);
  }
  else if (is_macro_tiles && is_astar) {
    MacroTilesInstance15 *instance = get_macro_tiles_instance(argc, argv);
    MacroTilesAStar &astar = *new MacroTilesAStar(*instance);
    search(astar);
  }
  else if (is_macro_tiles && is_hastar) {
    MacroTilesInstance15 *instance = get_macro_tiles_instance(argc, argv);
    MacroTilesHAStar &hastar = *new MacroTilesHAStar(*instance);
    search(hastar);
  }
  else if (is_macro_tiles && is_hidastar) {
    cerr << "HIDA* unimplemented" << endl;
  }
  else if (is_macro_tiles && is_switchback) {
    MacroTilesInstance15 *instance = get_macro_tiles_instance(argc, argv);
    MacroTilesSwitchback &switchback = *new MacroTilesSwitchback(*instance);
    search(switchback);
  }
  else if (!is_tiles && !is_macro_tiles) {
    cerr << "error: invalid domain specified" << endl;
    print_usage(cerr, argv[0]);
    return 1;
  }
  else if (!is_astar && !is_hastar && !is_hidastar && !is_switchback) {
    cerr << "error: invalid algorithm specified" << endl;
    print_usage(cerr, argv[0]);
    return 1;
  }

  return 0;
}
