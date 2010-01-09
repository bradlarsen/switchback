#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <vector>
#include <iostream>

#include "ManhattanDistance.hpp"
#include "TilesState.hpp"
#include "TilesNode.hpp"


class TilesInstance15 {
public:
  TilesInstance15(const TilesState15 &start,
                  const TilesState15 &goal);

  void print(std::ostream &o) const;

  bool isGoal(const TilesState15 &s) const;

  std::vector<TilesNode15 *> * expand(const TilesNode15 &n) const;

  const TilesState15 & get_initial_state() const;

  /**
   * Creates the start node, for the start state.  It is the caller's
   * responsibility to dispose of the returned result.
   */
  TilesNode15 * create_start_node() const;

private:
  const TilesState15 start;
  const TilesState15 goal;

  const ManhattanDist15 md_heur;
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
