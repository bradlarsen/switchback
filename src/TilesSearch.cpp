#include <iostream>
#include <boost/scoped_ptr.hpp>

#include "search/astar/AStar.hpp"
#include "tiles/Tiles.hpp"

using namespace std;
using namespace boost;

int main()
{
  cout << "the size of a TilesNode15 is " << sizeof(TilesNode15) << endl;
  cout << "the size of a TilesState15 is " << sizeof(TilesState15) << endl;

  scoped_ptr<TilesInstance15> instance(readTilesInstance15(cin));

  if (instance == NULL) {
    cout << "error reading instance!" << endl;
    return 1;
  }

  cout << "######## The Instance ########" << endl;
  cout << *instance << endl;

  AStar<TilesInstance15, TilesNode15> astar(*instance);

  astar.search();

  const TilesNode15 *goal = astar.get_goal();
  if (goal == NULL) {
    cout << "no solution found!" << endl;
  }
  else {
    cout << "found a solution:" << endl;
    cout << *goal << endl;
  }

  cout << astar.get_num_expanded() << " expanded" << endl
       << astar.get_num_generated() << " generated" << endl;

  return 0;
}
