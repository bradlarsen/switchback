#ifndef _TILES_TYPES_HPP_
#define _TILES_TYPES_HPP_


#include <boost/array.hpp>

#ifndef BLOATED_TILES_REPRESENTATION
#include <boost/integer.hpp>
#include <boost/integer_traits.hpp>
#endif

#include <cassert>


#ifdef BLOATED_TILES_REPRESENTATION
typedef int Tile;
typedef unsigned TileIndex;
typedef boost::array<Tile, 16>  TileArray;
typedef unsigned Cost;
#else
// Beware!  Using small int types doesn't seem to play well with
// overloading resolution.
typedef boost::int_t<8>::least  Tile;
typedef boost::uint_t<8>::least TileIndex;
typedef boost::array<Tile, 16>  TileArray;
typedef boost::uint_t<8>::least Cost;


#include <iostream>

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

#endif /* !COMPACT_TILES_REPRESENTATION */


inline bool valid_tile(Tile t)
{
  return -1 <= t && t <= 15;
}

inline bool valid_tile_index(TileIndex i)
{
  return 0 <= i && i <= 15;
}


#endif /* !_TILES_TYPES_HPP_ */
