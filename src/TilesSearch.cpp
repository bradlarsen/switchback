#include <boost/timer.hpp>

#include <iostream>
#include <fstream>

#include "search/BucketPriorityQueue.hpp"
#include "search/astar/AStar.hpp"
#include "search/switchback/Switchback.hpp"
#include "tiles/Tiles.hpp"

using namespace std;
using namespace boost;


typedef AStar<TilesInstance15, TilesNode15> TilesAStar;
typedef Switchback<TilesInstance15, TilesNode15> TilesSwitchback;


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
    << "  ALGORITHM is one of {astar, switchback}" << endl
    << "  FILE is the optional instance file to read from" << endl
    << endl
    << "If no file is specified, the instance is read from stdin." << endl;
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

  cout << "######## Type Size Information ########" << endl;
  print_size_info(cout);
  cout << endl;

  cout << "######## The Instance ########" << endl;
  cout << *instance << endl;

  const string alg_string(argv[1]);
  if (alg_string == "astar") {
    TilesAStar &astar = *new TilesAStar(*instance);
    search(astar);
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
