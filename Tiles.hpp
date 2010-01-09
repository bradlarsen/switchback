#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <vector>
#include <iostream>

#include "ManhattanDistance.hpp"
#include "TilesState.hpp"


class TilesInstance15 {
public:
  TilesInstance15(const TilesState15 &start,
                  const TilesState15 &goal);

  void print(std::ostream &o) const;

  bool isGoal(const TilesState15 &s) const;

private:
  const TilesState15 start;
  const TilesState15 goal;

  const ManhattanDist15 md_heur;
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
