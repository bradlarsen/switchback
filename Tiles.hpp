#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <boost/pool/pool.hpp>
#include <iostream>
#include <vector>

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
    , node_pool(sizeof(TilesNode15))
  {
    assert(is_goal(goal));
  }


  ~TilesInstance15()
  {
  }



  void print(std::ostream &o) const;

  inline bool is_goal(const TilesState15 &s) const
  {
    return s == goal;
  }

  std::vector<TilesNode15 *> * expand(const TilesNode15 &n);

  const TilesState15 & get_start_state() const;
  const TilesState15 & get_goal_state() const;

  /**
   * Creates the start node, for the start state.  It is the caller's
   * responsibility to dispose of the returned result.
   */
  TilesNode15 * create_start_node();

  /**
   * Indicates how many buckets will be needed in a bucket-based
   * priority queue data structure.
   *
   * For the 15-puzzle, I believe the hardest instances have an
   * f-value around 70.
   */
  unsigned get_num_buckets() const;

private:
  TilesNode15 * child(const TilesState15 &new_state,
                      Cost new_g,
                      const TilesNode15 &parent)
  {
    assert(new_state != parent.get_state());
    assert(parent.get_parent() == NULL ||
           new_state != parent.get_parent()->get_state());
    Cost new_h = md_heur.compute_incr(new_state, parent);
    if (!is_goal(new_state))
      new_h = 1 > new_h ? 1 : new_h;

    TilesNode15 *child_node = new (node_pool.malloc()) TilesNode15(new_state,
                                                                   new_g,
                                                                   new_h,
                                                                   &parent);
    assert(parent.get_f() <= child_node->get_f());
    assert(child_node->get_f() <= 80);

    return child_node;
  }


  const TilesState15 start;
  const TilesState15 goal;

  const ManhattanDist15 md_heur;

  boost::pool<> node_pool;

private:
  TilesInstance15(const TilesInstance15 &other);
  TilesInstance15 & operator =(const TilesInstance15 &other);
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
