#ifndef _MANHATTAN_DISTANCE_HPP_
#define _MANHATTAN_DISTANCE_HPP_


#include <boost/array.hpp>

#include "TilesNode.hpp"
#include "TilesState.hpp"
#include "TilesTypes.hpp"


class ManhattanDist15 {
public:
  ManhattanDist15(const TilesState15 &goal);

  inline Cost compute_full(const TilesState15 &s) const
  {
    Cost dist = 0;

    for (unsigned i = 0; i < 16; i += 1)
      if (s.get_tiles()[i] != 0)
        dist += lookup_dist(s.get_tiles()[i], i);

    return dist;
  }

  inline Cost compute_incr(const TilesState15 &s, const TilesNode15 &parent) const
  {
    const TilesState15 &p = parent.get_state();
    unsigned new_b = s.get_blank();
    unsigned par_b = p.get_blank();
    Tile tile = p.get_tiles()[new_b];
    Cost ret = 0;
  
    ret = parent.get_h() + (lookup_dist(tile, par_b) - lookup_dist(tile, new_b));
    assert(ret >= 0);
    return ret;
  }
  
  inline Cost lookup_dist(Tile tile, TileIndex pos) const
  {
    return table[tile * 16 + pos];
  }


private:
  void init(const TilesState15 &goal);

  boost::array<Cost, 4 * 4 * 4 * 4> table;
};



#endif /* !_MANHATTAN_DISTANCE_HPP_ */
