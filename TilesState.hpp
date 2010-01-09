#ifndef _TILES_STATE_HPP_
#define _TILES_STATE_HPP_


#include <iostream>
#include <boost/functional/hash.hpp>
#include "TilesTypes.hpp"


class TilesState15 {
public:
  const TileIndex blank_index;
  const TileArray tiles;
  const std::size_t hash_value;

  TilesState15(const TileArray &tiles,
               TileIndex blank_index)
    : blank_index(blank_index)
    , tiles(tiles)
    , hash_value(boost::hash_range(tiles.begin(), tiles.end()))
  {
  }

  ~TilesState15()
  {
  }

  Tile operator ()(unsigned i, unsigned j) const
  {
    assert(i < 4);
    assert(j < 4);
    return tiles[i * 4 + j];
  }

  bool operator ==(const TilesState15 &other) const
  {
    return
      hash_value == other.hash_value &&
      blank_index == other.blank_index &&
      tiles == other.tiles;
  }

  bool valid() const
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

private:
  TilesState15 & operator =(const TilesState15 &other);
};


std::size_t hash_value(TilesState15 const &tiles);
std::ostream & operator <<(std::ostream &o, const TilesState15 &tiles);


#endif  /* !_TILES_STATE_HPP_ */
