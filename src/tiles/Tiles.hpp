#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <boost/pool/singleton_pool.hpp>
#include <iostream>
#include <vector>

#include "tiles/ManhattanDistance.hpp"
#include "tiles/TilesState.hpp"
#include "tiles/TilesNode.hpp"


// Used with boost's singleton pool.
struct TilesNode15Tag { };
typedef boost::singleton_pool<TilesNode15Tag, sizeof(TilesNode15)> NodePool;


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

  /**
   * Expands the given node into the given vector for successors.  The
   * caller need not worry about disposing of the successors.
   */
  void expand(const TilesNode15 &n, std::vector<TilesNode15 *> &succs) const;

  const TilesState15 & get_start_state() const;
  const TilesState15 & get_goal_state() const;

  /**
   * Creates the start node, for the start state.  The caller need not
   * worry about disposing of the returned node.
   */
  TilesNode15 * create_start_node() const;


private:
  TilesNode15 * child(const TilesState15 &new_state,
                      Cost new_g,
                      const TilesNode15 &parent) const
  {
    assert(new_state != parent.get_state());
    assert(parent.get_parent() == NULL ||
           new_state != parent.get_parent()->get_state());
    Cost new_h = md_heur.compute_incr(new_state, parent);
    if (!is_goal(new_state))
      new_h = 1 > new_h ? 1 : new_h;

    TilesNode15 *child_node = new (NodePool::malloc()) TilesNode15(new_state,
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

private:
  TilesInstance15(const TilesInstance15 &other);
  TilesInstance15 & operator =(const TilesInstance15 &other);
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
