#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <boost/array.hpp>
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
  static const unsigned num_abstraction_levels = 8;

private:
  typedef std::pair<Tile, Cost> TileCostPair;
  typedef boost::array<
    boost::array<bool, 17>,
    num_abstraction_levels
    > AbstractionOrder;

public:
  TilesInstance15 (const TilesState15 &start,
                   const TilesState15 &goal)
    : start(start)
    , goal(goal)
    , md_heur(goal)
    , abstraction_order(compute_abstraction_order(start, md_heur))
  {
#ifndef NDEBUG
    dump_abstraction_order(std::cerr);

    std::cerr << "the 9 abstractions of the start node:" << std::endl
              << "base level:" <<std::endl
              << start << std::endl;
    for (unsigned level = 0; level < num_abstraction_levels; level += 1) {
      TilesState15 abs_start = abstract(start, level);
      std::cerr << "abstraction " << level+1 << ":" << std::endl
                << abs_start << std::endl;
    }
#endif
    assert(is_goal(goal));
  }


  void print(std::ostream &o) const;

  inline bool is_goal(const TilesState15 &s) const
  {
    return s == goal;
  }

  /**
   * Expands the given node into the given vector for successors.
   *
   * Note that this does not assign the h values for the successor
   * nodes: that must be done by the caller.
   */
  void compute_successors(const TilesNode15 &n, std::vector<TilesNode15 *> &succs) const;

  /**
   * Expands the given node into the given vector for predecessors.
   *
   * For tiles, this method merely calls compute_successors.  With
   * tiles, the moves are all reversible.
   *
   * Note that this does not assign the h values for the predecessor
   * nodes: that must be done by the caller.
   *
   */
  void compute_predecessors(const TilesNode15 &n, std::vector<TilesNode15 *> &succs) const;

  /**
   * Computes and assigned the heuristic for the given child node.
   */
  void compute_heuristic(const TilesNode15 &parent, TilesNode15 &child) const;

  const TilesState15 & get_start_state() const;
  const TilesState15 & get_goal_state() const;

  /**
   * Creates the start node, for the start state.
   */
  TilesNode15 * create_start_node() const;

  void free_node(TilesNode15 *n) const;
  
  TilesState15 abstract(const TilesState15 &s, unsigned level) const;

  
private:
  TilesNode15 * child(const TilesState15 &new_state,
                      Cost new_g,
                      const TilesNode15 &parent) const;

  unsigned find_tile_index(const std::vector<TileCostPair> &pairs,
                           Tile t) const;

  AbstractionOrder compute_abstraction_order(const TilesState15 &s,
                                             const ManhattanDist15 &md) const;

  void dump_abstraction_order(std::ostream &o) const;

  bool should_abstract(Tile t, unsigned level) const;


  const TilesState15 start;
  const TilesState15 goal;

  const ManhattanDist15 md_heur;

  const AbstractionOrder abstraction_order;

private:
  TilesInstance15(const TilesInstance15 &other);
  TilesInstance15 & operator =(const TilesInstance15 &other);
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
