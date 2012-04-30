#ifndef _TILES_TYPES_HPP_
#define _TILES_TYPES_HPP_


#include <boost/array.hpp>
#include <boost/integer.hpp>
#include <boost/integer_traits.hpp>
#include <cassert>
#include <iostream>


// Beware!  Using small int types doesn't seem to play well with
// overloading resolution.
typedef boost::int_t<8>::least  Tile;
typedef boost::uint_t<8>::least TileIndex;
typedef boost::array<Tile, 16>  TileArray;
typedef boost::uint_t<8>::least TileCost;

inline std::ostream & operator <<(std::ostream &o, boost::int_t<8>::least i)
{
  o << static_cast<int>(i);
  return o;
}

inline std::ostream & operator <<(std::ostream &o, boost::uint_t<8>::least i)
{
  o << static_cast<unsigned>(i);
  return o;
}

inline bool valid_tile(Tile t)
{
  return -1 <= t && t <= 15;
}

inline bool valid_tile_index(TileIndex i)
{
  return i <= 15;
}


#endif /* !_TILES_TYPES_HPP_ */
