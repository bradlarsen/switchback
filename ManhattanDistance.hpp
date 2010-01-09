#ifndef _MANHATTAN_DISTANCE_HPP_
#define _MANHATTAN_DISTANCE_HPP_


#include <boost/array.hpp>

#include "TilesState.hpp"
#include "TilesTypes.hpp"


// TODO: need to implement incremental heuristic calculation

class ManhattanDist15 {
public:
  ManhattanDist15(const TilesState15 &goal);

  Cost compute(const TilesState15 &s) const;
  Cost compute_full(const TilesState15 &s) const;
  Cost lookup_dist(Tile tile, TileIndex pos) const;

private:
  void init(const TilesState15 &goal);

  boost::array<Cost, 4 * 4 * 4 * 4> table;
};



#endif /* !_MANHATTAN_DISTANCE_HPP_ */
