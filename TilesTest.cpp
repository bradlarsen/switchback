#include <iostream>
#include <boost/scoped_ptr.hpp>
#include "Tiles.hpp"


int main()
{
  boost::scoped_ptr<TilesInstance15> instance(readTilesInstance15(std::cin));

  if (instance == NULL) {
    std::cout << "error reading instance!" << std::endl;
    return 1;
  }
  else {
    std::cout << *instance << std::endl;
    return 0;
  }

  return 0;
}
