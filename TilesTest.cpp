#include <iostream>
#include <boost/scoped_ptr.hpp>

#include "Tiles.hpp"

using namespace std;
using namespace boost;

int main()
{
  scoped_ptr<TilesInstance15> instance(readTilesInstance15(cin));

  if (instance == NULL) {
    cout << "error reading instance!" << endl;
    return 1;
  }
  else {
    cout << "######## The Instance ########" << endl;
    cout << *instance << endl;
    scoped_ptr<TilesNode15> start_node(instance->create_start_node());

    cout << "######## The Successors of Start ########" << endl;
    scoped_ptr< vector<TilesNode15 *> > succs(instance->expand(*start_node));
    for (unsigned i = 0; i < succs->size(); i += 1) {
      cout << "######## Successor " << i+1 << " ########" << endl;
      cout << *(*succs)[i] << endl;
      delete (*succs)[i];
    }

    return 0;
  }

  return 0;
}
