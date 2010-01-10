#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <vector>
#include <iostream>

#include "ManhattanDistance.hpp"
#include "TilesState.hpp"
#include "TilesNode.hpp"


class TilesInstance15 {
public:
  TilesInstance15 (const TilesState15 &start,
                   const TilesState15 &goal)
    : start(start)
    , goal(goal)
    , md_heur(ManhattanDist15(goal))
  {
    assert(is_goal(goal));
  }



  void print(std::ostream &o) const;

  inline bool is_goal(const TilesState15 &s) const
  {
    return s == goal;
  }

  std::vector<TilesNode15 *> * expand(const TilesNode15 &n) const;

  const TilesState15 & get_start_state() const;
  const TilesState15 & get_goal_state() const;

  /**
   * Creates the start node, for the start state.  It is the caller's
   * responsibility to dispose of the returned result.
   */
  TilesNode15 * create_start_node() const;

  /**
   * Indicates how many buckets will be needed in a bucket-based
   * priority queue data structure.
   *
   * For the 15-puzzle, I believe the hardest instances have an
   * f-value around 70.
   */
  unsigned get_num_buckets() const;

private:
  const TilesState15 start;
  const TilesState15 goal;

  const ManhattanDist15 md_heur;
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
