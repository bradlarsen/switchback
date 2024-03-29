#include <boost/timer.hpp>

#include <iostream>
#include <fstream>

#include <sys/resource.h>
#include <cstring>

#include "search/Node.hpp"
#include "search/BucketPriorityQueue.hpp"
#include "search/Constants.hpp"
#include "search/astar/AStar.hpp"
#include "search/hastar/HAStar.hpp"
#include "search/hidastar/HIDAStar.hpp"
#include "search/idastar/IDAStar.hpp"
#include "search/switchback/Switchback.hpp"
#include "tiles/Tiles.hpp"
#include "tiles/MacroTiles.hpp"
#include "tiles/GluedTiles.hpp"
#include "pancake/PancakeInstance.hpp"

using namespace std;
using namespace boost;


typedef AStar<TilesInstance15, TilesNode15> TilesAStar;
typedef IDAStar<TilesInstance15, TilesNode15> TilesIDAStar;
typedef HAStar<TilesInstance15, TilesNode15> TilesHAStar;
typedef HIDAStar<TilesInstance15, TilesNode15> TilesHIDAStar;
typedef Switchback<TilesInstance15, TilesNode15> TilesSwitchback;

typedef AStar<MacroTilesInstance15, TilesNode15> MacroTilesAStar;
typedef IDAStar<MacroTilesInstance15, TilesNode15> MacroTilesIDAStar;
typedef HAStar<MacroTilesInstance15, TilesNode15> MacroTilesHAStar;
typedef HIDAStar<MacroTilesInstance15, TilesNode15> MacroTilesHIDAStar;
typedef Switchback<MacroTilesInstance15, TilesNode15> MacroTilesSwitchback;

typedef AStar<GluedTilesInstance15, TilesNode15> GluedTilesAStar;
typedef IDAStar<GluedTilesInstance15, TilesNode15> GluedTilesIDAStar;
typedef HAStar<GluedTilesInstance15, TilesNode15> GluedTilesHAStar;
typedef HIDAStar<GluedTilesInstance15, TilesNode15> GluedTilesHIDAStar;
typedef Switchback<GluedTilesInstance15, TilesNode15> GluedTilesSwitchback;

typedef AStar<PancakeInstance14, PancakeNode14> PancakeAStar;
typedef HAStar<PancakeInstance14, PancakeNode14> PancakeHAStar;
typedef HIDAStar<PancakeInstance14, PancakeNode14> PancakeHIDAStar;
typedef IDAStar<PancakeInstance14, PancakeNode14> PancakeIDAStar;
typedef Switchback<PancakeInstance14, PancakeNode14> PancakeSwitchback;


static void print_build_info(ostream &o)
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


  o << endl;


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

  o << "Hierarchical A*'s reexpansion counting is "
#ifdef HIERARCHICAL_A_STAR_REEXPANSION_COUNTING
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif


  o << endl;


  o << "HIDA* Cycle Checking is "
#ifdef HIDA_STAR_CYCLE_CHECKING
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "HIDA* Duplicate Detection is "
#ifdef HIDA_STAR_DUPLICATE_DETECTION
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "HIDA* Reexpansion Counting is "
#ifdef HIDA_STAR_REEXPANSION_COUNTING
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif


  o << endl;


  o << "IDA* Cycle Checking is "
#ifdef IDA_STAR_CYCLE_CHECKING
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif


  o << endl;


  o << "OUTPUT_SEARCH_PROGRESS is "
#ifdef OUTPUT_SEARCH_PROGRESS
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif

  o << "INITIAL_CLOSED_SET_SIZE is " << INITIAL_CLOSED_SET_SIZE << endl;


  o << endl;


  o << "NDEBUG is "
#ifdef NDEBUG
    << "enabled" << endl;
#else
    << "disabled" << endl;
#endif
}


static void print_size_info(ostream &o)
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
    << sizeof(BucketPriorityQueue<TilesNode15>::ItemPointer) << endl
    << "sizeof(Pancake) is " << sizeof(Pancake) << endl
    << "sizeof(boost::array<Pancake, 14>) is "
    << sizeof(boost::array<Pancake,14>) << endl
    << "sizeof(PancakeCost) is " << sizeof(PancakeCost) << endl
    << "sizeof(PancakeState14) is " << sizeof(PancakeState14) << endl
    << "sizeof(PancakeNode14) is " << sizeof(PancakeNode14) << endl
    << "sizeof(BucketPriorityQueue<PancakeNode14>::ItemPointer) is "
    << sizeof(BucketPriorityQueue<PancakeNode14>::ItemPointer) << endl;
}


static void print_usage(ostream &o, const char *prog_name)
{
  o << "usage: " << prog_name << " DOMAIN ALGORITHM [FILE]" << endl
    << "where" << endl
    << "  DOMAIN is one of {tiles, tiles_static_abstraction, macro_tiles, glued_tiles, pancake}" << endl
    << "  ALGORITHM is one of {astar, hastar, idastar, hidastar, switchback}" << endl
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


static TilesInstance15 * get_tiles_instance(int argc, char *argv[])
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

  return instance;
}


static MacroTilesInstance15 * get_macro_tiles_instance(int argc, char *argv[])
{
  return new MacroTilesInstance15(get_tiles_instance(argc, argv));
}


static GluedTilesInstance15 * get_glued_tiles_instance(int argc, char *argv[])
{
  GluedTilesInstance15 *instance;
  if (argc == 4) {
    ifstream infile(argv[3]);
    instance = readGluedTilesInstance15(infile);
  }
  else {
    instance = readGluedTilesInstance15(cin);
  }

  if (instance == NULL) {
    cerr << "error reading instance!" << endl;
    exit(1);
  }

  return instance;
}

static PancakeInstance14 * get_pancake_instance(int argc, char *argv[])
{
  PancakeInstance14 *instance;
  if (argc == 4) {
    ifstream infile(argv[3]);
    instance = PancakeInstance14::read(infile);
  }
  else {
    instance = PancakeInstance14::read(cin);
  }

  if (instance == NULL) {
    cerr << "error reading instance!" << endl;
    exit(1);
  }

  return instance;
}

static long get_max_mem_used_in_mb ()
{
  struct rusage usage;
  int ret = getrusage (RUSAGE_SELF, &usage);
  if (ret == -1)
    return 0;
  else
#ifdef __APPLE__
    // ru_maxrss represents *bytes*, not kilobytes, on Mac OS X
    return usage.ru_maxrss / (1024L * 1024L);
#else
    return usage.ru_maxrss / 1024L;
#endif
}

template <class Searcher>
static void search(Searcher &searcher)
{
  cout << "######## Search Results ########" << endl;

  timer search_timer;
  searcher.search();

  const typename Searcher::Node *goal = searcher.get_goal();
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
  const unsigned gen_per_second = searcher.get_num_generated() / seconds_elapsed;

  cout << "expanded: " << searcher.get_num_expanded() << " ("
       << exp_per_second << "/s)" << endl
       << "generated: " << searcher.get_num_generated() << " ("
       << gen_per_second << "/s)" << endl
       << "time: " << search_timer.elapsed() << " s" << endl
       << "max memory: " << get_max_mem_used_in_mb () << " MB" << endl;

  searcher.output_statistics(cout);
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
  const bool is_tiles_static = domain_string == "tiles_static_abstraction";
  const bool is_macro_tiles = domain_string == "macro_tiles";
  const bool is_glued_tiles = domain_string == "glued_tiles";
  const bool is_pancake = domain_string == "pancake";

  const bool is_astar = alg_string == "astar";
  const bool is_hastar = alg_string == "hastar";
  const bool is_hidastar = alg_string == "hidastar";
  const bool is_idastar = alg_string == "idastar";
  const bool is_switchback = alg_string == "switchback";

  // ############################################################
  // Argument Error Checking
  // ############################################################
  if (!is_tiles && !is_tiles_static && !is_macro_tiles && !is_pancake && !is_glued_tiles) {
    cerr << "error: invalid domain specified" << endl;
    print_usage(cerr, argv[0]);
    exit (1);
  }
  if (!is_astar && !is_hastar && !is_hidastar && !is_idastar && !is_switchback) {
    cerr << "error: invalid algorithm specified" << endl;
    print_usage(cerr, argv[0]);
    exit (1);
  }

  // ############################################################
  // tiles domain with custom abstraction
  // ############################################################
  if (is_tiles) {
    TilesInstance15 *instance = get_tiles_instance(argc, argv);
    cout << "######## The Instance ########" << endl;
    cout << *instance << endl << endl;

    if (is_astar) {
      TilesAStar &astar = *new TilesAStar(*instance);
      search(astar);
    }
    else if (is_hastar) {
      TilesHAStar &hastar = *new TilesHAStar(*instance);
      search(hastar);
    }
    else if (is_hidastar) {
      TilesHIDAStar &hidastar = *new TilesHIDAStar(*instance);
      search(hidastar);
    }
    else if (is_idastar) {
      TilesIDAStar &idastar = *new TilesIDAStar(*instance);
      search(idastar);
    }
    else if (is_switchback) {
      TilesSwitchback &switchback = *new TilesSwitchback(*instance);
      search(switchback);
    }
  }

  // ############################################################
  // tiles domain with static abstraction
  // ############################################################
  if (is_tiles_static) {
    TilesInstance15 *instance = get_tiles_instance(argc, argv);
    instance->set_abstraction_order (TilesInstance15::static_abstraction_order);
    cout << "######## The Instance ########" << endl;
    cout << *instance << endl << endl;

    if (is_astar) {
      TilesAStar &astar = *new TilesAStar(*instance);
      search(astar);
    }
    else if (is_hastar) {
      TilesHAStar &hastar = *new TilesHAStar(*instance);
      search(hastar);
    }
    else if (is_hidastar) {
      TilesHIDAStar &hidastar = *new TilesHIDAStar(*instance);
      search(hidastar);
    }
    else if (is_idastar) {
      TilesIDAStar &idastar = *new TilesIDAStar(*instance);
      search(idastar);
    }
    else if (is_switchback) {
      TilesSwitchback &switchback = *new TilesSwitchback(*instance);
      search(switchback);
    }
  }

  // ############################################################
  // macro tiles domain
  // ############################################################
  else if (is_macro_tiles) {
    MacroTilesInstance15 *instance = get_macro_tiles_instance(argc, argv);
    cout << "######## The Instance ########" << endl;
    cout << *instance << endl << endl;

    if (is_astar) {
      MacroTilesAStar &astar = *new MacroTilesAStar(*instance);
      search(astar);
    }
    else if (is_hastar) {
      MacroTilesHAStar &hastar = *new MacroTilesHAStar(*instance);
      search(hastar);
    }
    else if (is_hidastar) {
      MacroTilesHIDAStar &hidastar = *new MacroTilesHIDAStar(*instance);
      search(hidastar);
    }
    else if (is_idastar) {
      MacroTilesIDAStar &idastar = *new MacroTilesIDAStar(*instance);
      search(idastar);
    }
    else if (is_switchback) {
      MacroTilesSwitchback &switchback = *new MacroTilesSwitchback(*instance);
      search(switchback);
    }
  }

  // ############################################################
  // glued tiles domain
  // ############################################################
  else if (is_glued_tiles) {
    GluedTilesInstance15 *instance = get_glued_tiles_instance(argc, argv);
    cout << "######## The Instance ########" << endl;
    cout << *instance << endl << endl;

    if (is_astar) {
      GluedTilesAStar &astar = *new GluedTilesAStar(*instance);
      search(astar);
    }
    else if (is_hastar) {
      GluedTilesHAStar &hastar = *new GluedTilesHAStar(*instance);
      search(hastar);
    }
    else if (is_hidastar) {
      GluedTilesHIDAStar &hidastar = *new GluedTilesHIDAStar(*instance);
      search(hidastar);
    }
    else if (is_idastar) {
      GluedTilesIDAStar &idastar = *new GluedTilesIDAStar(*instance);
      search(idastar);
    }
    else if (is_switchback) {
      GluedTilesSwitchback &switchback = *new GluedTilesSwitchback(*instance);
      search(switchback);
    }
  }

  // ############################################################
  // pancake puzzle domain
  // ############################################################
  else if (is_pancake) {
    PancakeInstance14 *instance = get_pancake_instance(argc, argv);
    cout << "######## The Instance ########" << endl;
    cout << *instance << endl << endl;

    if (is_astar) {
      PancakeAStar &astar = *new PancakeAStar(*instance);
      search(astar);
    }
    else if (is_hastar) {
      PancakeHAStar &hastar = *new PancakeHAStar(*instance);
      search(hastar);
    }
    else if (is_hidastar) {
      PancakeHIDAStar &hidastar = *new PancakeHIDAStar(*instance);
      search(hidastar);
    }
    else if (is_idastar) {
      PancakeIDAStar &idastar = *new PancakeIDAStar(*instance);
      search(idastar);
    }
    else if (is_switchback) {
      PancakeSwitchback &switchback = *new PancakeSwitchback(*instance);
      search(switchback);
    }
  }

  return 0;
}
