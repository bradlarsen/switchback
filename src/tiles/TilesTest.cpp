#include <iostream>
#include <boost/scoped_ptr.hpp>

#include "tiles/Tiles.hpp"
#include "search/BucketPriorityQueue.hpp"

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
  scoped_ptr<TilesNode15> start_node(instance->create_start_node());

  BucketPriorityQueue<TilesNode15> pqueue;

  cout << "######## The Successors of Start ########" << endl;
  vector<TilesNode15 *> succs;
  instance->compute_successors(*start_node, succs);
  for (unsigned i = 0; i < succs.size(); i += 1) {
    instance->compute_heuristic(*start_node, *succs[i]);
    cout << "######## Successor " << i+1 << " ########" << endl;
    cout << *succs[i] << endl;
    pqueue.push(succs[i]);
  }

  cout << "######## The nodes, in order of ascending f-value ########" << endl;
  while (!pqueue.empty()) {
    cout << *pqueue.top() << endl;
    delete pqueue.top();
    pqueue.pop();
  }

  return 0;
}
