#include "TilesState.hpp"


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


bool TilesState15::valid() const
{
  bool hasBlank = false;

  for (unsigned i = 0; i < 4; ++i) {
    for (unsigned j = 0; j < 4; ++j) {
      if ((*this)(i, j) == 0)
        hasBlank = true;
      if ((*this)(i, j) < -1 || (*this)(i, j) > 15)
        return false;
    }
  }

  for (Tile t = 1; t < 16; ++t) {
    unsigned t_count = 0;
    for (unsigned i = 0; i < 4; ++i) {
      for (unsigned j = 0; j < 4; ++j) {
        if ((*this)(i, j) == t)
          ++t_count;
      }
    }

    if (t_count > 1) return false;
  }

  return hasBlank;
}
