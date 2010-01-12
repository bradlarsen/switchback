#ifndef _TILES_STATE_HPP_
#define _TILES_STATE_HPP_


#include <boost/functional/hash.hpp>

#include <cassert>
#include <iostream>

#include "TilesTypes.hpp"


class TilesState15 {
public:
  TilesState15(const TileArray &tiles,
               TileIndex blank_index)
    : blank_index(blank_index)
    , tiles(tiles)
    , hash_value(compute_hash())
  {
    assert(tiles[blank_index] == 0);
  }

  ~TilesState15()
  {
  }

  inline Tile operator ()(unsigned i, unsigned j) const
  {
    assert(i < 4);
    assert(j < 4);
    return tiles[i * 4 + j];
  }

  inline bool operator ==(const TilesState15 &other) const
  {
    return
      hash_value == other.hash_value &&
      blank_index == other.blank_index &&
      tiles == other.tiles;
  }

  inline bool operator !=(const TilesState15 &other) const
  {
    return !(*this == other);
  }

  inline TileIndex get_blank() const
  {
    return blank_index;
  }

  inline TileIndex get_blank_row() const
  {
    return blank_index / 4;
  }

  inline TileIndex get_blank_col() const
  {
    return blank_index % 4;
  }

  inline const TileArray & get_tiles() const
  {
    return tiles;
  }

  inline size_t get_hash_value() const
  {
    return hash_value;
  }

  bool valid() const;

  TilesState15 move_blank_up() const
  {
    assert(get_blank_row() > 0);
    return move_blank(-4);
  }

  TilesState15 move_blank_down() const
  {
    assert(get_blank_row() < 3);
    return move_blank(4);
  }

  TilesState15 move_blank_left() const
  {
    assert(get_blank_col() > 0);
    return move_blank(-1);
  }

  TilesState15 move_blank_right() const
  {
    assert(get_blank_col() < 3);
    return move_blank(1);
  }


private:
  std::size_t compute_hash() const
  {
    return boost::hash_range(tiles.begin(), tiles.end());
  }

  inline TilesState15 move_blank(TileIndex new_blank_offset) const
  {
    TileIndex new_blank_index = blank_index + new_blank_offset;
    TilesState15 new_state(*this);
    new_state.blank_index = new_blank_index;
    std::swap(new_state.tiles[blank_index], new_state.tiles[new_blank_index]);
    new_state.hash_value = new_state.compute_hash();

    return new_state;
  }

  TilesState15 & operator =(const TilesState15 &other);

  TileIndex blank_index;
  TileArray tiles;
  std::size_t hash_value;
};


inline std::size_t hash_value(TilesState15 const &tiles)
{
  return tiles.get_hash_value();
}


std::ostream & operator <<(std::ostream &o, const TilesState15 &tiles);


#endif  /* !_TILES_STATE_HPP_ */
