#include <iostream>
#include <boost/scoped_ptr.hpp>

#include "AStar.hpp"
#include "Tiles.hpp"
#include "BucketPriorityQueue.hpp"

using namespace std;
using namespace boost;

int main()
{
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
