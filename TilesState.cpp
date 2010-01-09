#include "TilesState.hpp"


std::size_t hash_value(TilesState15 const &tiles)
{
  return tiles.hash_value;
}


std::ostream & operator <<(std::ostream &o, const TilesState15 &tiles)
{
  o << "Hash: " << hash_value(tiles) << std::endl;
  for (unsigned y = 0; y < 4; y += 1) {
      unsigned x = 0;
      do {
        o << static_cast<int>(tiles(y, x)) << '\t';
        x += 1;
      } while (x < 3);
      o << static_cast<int>(tiles(y, 3)) << '\n';
  }

  return o;
}
