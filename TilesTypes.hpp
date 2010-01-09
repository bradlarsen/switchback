#ifndef _TILES_TYPES_HPP_
#define _TILES_TYPES_HPP_


#include <boost/array.hpp>
#include <boost/integer.hpp>
#include <boost/integer_traits.hpp>


// Beware!  Using small int types doesn't seem to play well with
// overloading resolution.  When using the `<<' and `>>' operators,
// the char version is selected.

typedef boost::int_t<8>::least  Tile;
typedef boost::uint_t<8>::least TileIndex;
typedef boost::array<Tile, 16>  TileArray;
typedef boost::uint_t<8>::least Cost;


#endif /* !_TILES_TYPES_HPP_ */
