#ifndef _TILES_TYPES_HPP_
#define _TILES_TYPES_HPP_


#include <boost/array.hpp>
#include <boost/integer.hpp>
#include <boost/integer_traits.hpp>


typedef boost::int_t<8>::least  Tile;
typedef boost::uint_t<8>::least TileIndex;
typedef boost::array<Tile, 16>  TileArray;
typedef boost::uint_t<8>::least Cost;


#endif /* !_TILES_TYPES_HPP_ */
