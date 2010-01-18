#ifndef _MACRO_TILES_HPP_
#define _MACRO_TILES_HPP_

#include <boost/array.hpp>
#include <boost/pool/pool.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <vector>

#include "tiles/ManhattanDistance.hpp"
#include "tiles/Tiles.hpp"
#include "tiles/TilesState.hpp"
#include "tiles/TilesNode.hpp"


class MacroTilesInstance15 : boost::noncopyable
{
private:
  TilesInstance15 *tiles_instance;

public:
  static const unsigned num_abstraction_levels = TilesInstance15::num_abstraction_levels;

  
  MacroTilesInstance15 (TilesInstance15 *tiles_instance)
    : tiles_instance(tiles_instance)
  {
  }

  ~MacroTilesInstance15 ()
  {
    delete tiles_instance;
  }

  void print(std::ostream &o) const
  {
    tiles_instance->print(o);
  }

  bool is_goal(const TilesState15 &s) const
  {
    return tiles_instance->is_goal(s);
  }

  TileCost get_epsilon(const TilesState15 &s) const
  {
    return 1;
  }

  void compute_successors(const TilesNode15 &n,
                          std::vector<TilesNode15 *> &succs,
                          boost::pool<> &node_pool)
  {
    tiles_instance->compute_macro_successors(n, succs, node_pool);
  }

  void compute_predecessors(const TilesNode15 &n,
                            std::vector<TilesNode15 *> &succs,
                            boost::pool<> &node_pool)
  {
    tiles_instance->compute_macro_predecessors(n, succs, node_pool);
  }

  void compute_heuristic(const TilesNode15 &parent,
                         TilesNode15 &child) const
  {
    tiles_instance->compute_heuristic(parent, child);
    child.set_h(child.get_h() / 3);
  }

  void compute_heuristic(TilesNode15 &child) const
  {
    tiles_instance->compute_heuristic(child);
    child.set_h(child.get_h() / 3);
  }

  const TilesState15 & get_start_state() const
  {
    return tiles_instance->get_start_state();
  }

  const TilesState15 & get_goal_state() const
  {
    return tiles_instance->get_goal_state();
  }


  TilesState15 abstract(unsigned level, const TilesState15 &s) const
  {
    return tiles_instance->abstract(level, s);
  }


  static bool is_valid_level(const unsigned level)
  {
    return TilesInstance15::is_valid_level(level);
  }
};


inline std::ostream & operator << (std::ostream &o, const MacroTilesInstance15 &t)
{
  t.print(o);
  return o;
}


#endif	/* !_MACRO_TILES_HPP_ */
