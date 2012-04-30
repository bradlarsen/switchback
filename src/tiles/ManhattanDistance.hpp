#ifndef _MANHATTAN_DISTANCE_HPP_
#define _MANHATTAN_DISTANCE_HPP_


#include <boost/array.hpp>

#include "tiles/TilesNode.hpp"
#include "tiles/TilesState.hpp"
#include "tiles/TilesTypes.hpp"


// TODO: make ManhattanDist15 take into account obscured tiles when
// computing heuristic values.  Presently, obscured tiles are given a
// heuristic value of 0.  If an obscured tile is in a position in the
// goal board that is /not/ obscured, then we know that the obscured
// tile needs to be moved at least to the nearest obscured tile in the
// goal board.
//
// This would complicate things, however, since then ManhattanDist15
// would need to be aware of abstraction levels.  We could do this by
// passing in the goal & all its abstractions in the constructor, and
// building a 3d table indexed by (level, tile, index).  (Currently, a
// 2d table is constructed, indexed by (tile, index).)
//
// But this is a bit of tricky work, to improve MD when using
// abstraction.  Manhattan distance sucks when using abstraction!  So
// I delay implementing this for now.

class ManhattanDist15 {
public:
  ManhattanDist15(const TilesState15 &goal);

  inline TileCost compute_full(const TilesState15 &s) const
  {
    TileCost dist = 0;

    for (unsigned i = 0; i < 16; i += 1)
      dist += lookup_dist(s.get_tiles()[i], i);

    return dist;
  }

  inline TileCost compute_incr(const TilesState15 &s,
                               const TilesNode15 &parent) const
  {
    const TilesState15 &p = parent.get_state();
    unsigned new_b = s.get_blank();
    unsigned par_b = p.get_blank();
    Tile tile = p.get_tiles()[new_b];
    TileCost ret = 0;
  
    const unsigned par_dist = lookup_dist (tile, par_b);
    const unsigned new_dist = lookup_dist (tile, new_b);
    assert (par_dist >= new_dist);
    ret = parent.get_h() + (par_dist - new_dist);
    return ret;
  }
  
  inline TileCost lookup_dist(Tile tile, TileIndex pos) const
  {
    assert(valid_tile(tile));
    assert(valid_tile_index(pos));

    return tile == -1 ? 0 : table[tile][pos];
  }


private:
  void init(const TilesState15 &goal);

  boost::array<boost::array<TileCost, 16>, 16> table;
};



#endif /* !_MANHATTAN_DISTANCE_HPP_ */
