#ifndef _TILES_STATE_HPP_
#define _TILES_STATE_HPP_


#include <boost/functional/hash.hpp>

#include <cassert>
#include <iostream>

#include "TilesTypes.hpp"


class TilesState15
{
private:
  TileArray tiles;
#ifdef CACHE_TILES_HASH_VALUE
  std::size_t hash_value;
#endif
#ifdef CACHE_TILES_BLANK_INDEX
  TileIndex blank_index;
#endif


public:
  TilesState15()
  {
    for (TileIndex t = 0; t < 16; t += 1)
      tiles[t] = t;

#ifdef CACHE_TILES_HASH_VALUE
    hash_value = compute_hash(tiles);
#endif
#ifdef CACHE_TILES_BLANK_INDEX
    blank_index = 0;
#endif
  }

  TilesState15(const TileArray &tiles)
    : tiles(tiles)
#ifdef CACHE_TILES_HASH_VALUE
    , hash_value(compute_hash(tiles))
#endif
#ifdef CACHE_TILES_BLANK_INDEX
    , blank_index(find_blank_index(tiles))
#endif
  {
    assert(tiles[get_blank()] == 0);
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
#ifdef CACHE_TILES_HASH_VALUE
      hash_value == other.hash_value &&
#endif
#ifdef CACHE_TILES_BLANK_INDEX
      blank_index == other.blank_index &&
#endif
      tiles == other.tiles;
  }

  inline bool operator !=(const TilesState15 &other) const
  {
    return !(*this == other);
  }

  inline TileIndex get_blank() const
  {
#ifdef CACHE_TILES_BLANK_INDEX
    return blank_index;
#else
    return find_blank_index(tiles);
#endif
  }

  inline TileIndex get_blank_row() const
  {
    return get_blank() / 4;
  }

  inline TileIndex get_blank_col() const
  {
    return get_blank() % 4;
  }

  inline const TileArray & get_tiles() const
  {
    return tiles;
  }

  inline size_t get_hash_value() const
  {
#ifdef CACHE_TILES_HASH_VALUE
    return hash_value;
#else
    return compute_hash(tiles);
#endif
  }

  // Get the tile above the blank.
  Tile get_up_tile() const
  {
    assert(get_blank_row() > 0);
    return get_tile_rel_blank(-4);
  }

  // Get the tile below the blank.
  Tile get_down_tile() const
  {
    assert(get_blank_row() < 3);
    return get_tile_rel_blank(4);
  }

  // Get the tile left of the blank.
  Tile get_left_tile() const
  {
    assert(get_blank_col() > 0);
    return get_tile_rel_blank(-1);
  }

  // Get the tile right of the blank.
  Tile get_right_tile() const
  {
    assert(get_blank_col() < 3);
    return get_tile_rel_blank(1);
  }


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

  bool valid() const;


private:
  // Get the tile with the relative position to the blank.
  inline Tile get_tile_rel_blank(TileIndex offset) const
  {
    TileIndex blank_index = get_blank();
    TileIndex tile_index = blank_index + offset;
    return tiles[tile_index];
  }

  inline TilesState15 move_blank(TileIndex new_blank_offset) const
  {
    TileIndex blank_index = get_blank();
    TileIndex new_blank_index = blank_index + new_blank_offset;
    TilesState15 new_state(*this);
    std::swap(new_state.tiles[blank_index], new_state.tiles[new_blank_index]);
#ifdef CACHE_TILES_HASH_VALUE
    new_state.hash_value = compute_hash(new_state.tiles);
#endif
#ifdef CACHE_TILES_BLANK_INDEX
    new_state.blank_index = find_blank_index(new_state.tiles);
#endif

    return new_state;
  }

  static std::size_t compute_hash(const TileArray &tiles)
  {
    return boost::hash_range(tiles.begin(), tiles.end());
  }

  static TileIndex find_blank_index(const TileArray &tiles)
  {
    for (TileIndex i = 0; i < tiles.size(); i += 1)
      if (tiles[i] == 0)
        return i;

    assert(false);
    return 0;
  }
};


inline std::size_t hash_value(TilesState15 const &tiles)
{
  return tiles.get_hash_value();
}


std::ostream & operator <<(std::ostream &o, const TilesState15 &tiles);


#endif  /* !_TILES_STATE_HPP_ */
