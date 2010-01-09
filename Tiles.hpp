#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <vector>
#include <iostream>

#include "TilesState.hpp"


class TilesInstance15 {
public:
  TilesInstance15(const TilesState15 &start,
                  const TilesState15 &goal);
  ~TilesInstance15();

  void print(std::ostream &o) const;

  bool isGoal(const TilesState15 &s) const;

  // class ManhattanDist : public Heuristic {
  // public:
  //   ManhattanDist(const SearchDomain *d);
  //   ~ManhattanDist();
  //   Cost compute(State *s) const;
  // protected:
  //   void init(const SearchDomain *d);
  //   fp_type compute_full(TilesState *s) const;
  //   fp_type compute_incr(TilesState *s,
  //                        TilesState *p) const;
  //   int lookup_dist(Tile num, unsigned int pos) const;
    
  //   vector<unsigned int> table;
  // };

private:
  bool invariants_satisfied() const;

  const TilesState15 start;
  const TilesState15 goal;
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
