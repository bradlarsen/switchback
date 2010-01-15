#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <boost/array.hpp>
#include <boost/pool/pool.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <vector>

#include "tiles/ManhattanDistance.hpp"
#include "tiles/TilesState.hpp"
#include "tiles/TilesNode.hpp"


class TilesInstance15 : boost::noncopyable
{
public:
  static const unsigned num_abstraction_levels = 8;

private:
  typedef std::pair<Tile, TileCost> TileCostPair;
  typedef boost::array<
    boost::array<bool, 17>,
    num_abstraction_levels + 1
    > AbstractionOrder;

public:
  TilesInstance15 (const TilesState15 &start, const TilesState15 &goal);

  void print(std::ostream &o) const;

  inline bool is_goal(const TilesState15 &s) const
  {
    return s == goal;
  }

  inline TileCost get_epsilon(const TilesState15 &s) const
  {
    return 1;
  }

  /**
   * Expands the given node into the given vector for successors.
   *
   * Note that this does not assign the h values for the successor
   * nodes: that must be done by the caller.
   */
  void compute_successors(const TilesNode15 &n,
                          std::vector<TilesNode15 *> &succs);

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
  void compute_predecessors(const TilesNode15 &n,
                            std::vector<TilesNode15 *> &succs);

  /**
   * Computes and assigned the heuristic for the given child node.
   */
  void compute_heuristic(const TilesNode15 &parent,
                         TilesNode15 &child) const;

  void compute_heuristic(TilesNode15 &child) const;

  const TilesState15 & get_start_state() const;

  const TilesState15 & get_goal_state() const;


  TilesNode15 * create_node(const TilesState15 &state,
                            TileCost g,
                            TileCost h,
                            const TilesNode15 *parent);

  void free_node(TilesNode15 *n);
  
  TilesState15 abstract(unsigned level, const TilesState15 &s) const;


  static bool is_valid_level(const unsigned level);

  
private:
  TilesNode15 * child(const TilesState15 &new_state,
                      TileCost new_g,
                      const TilesNode15 &parent);

  unsigned find_tile_index(const std::vector<TileCostPair> &pairs,
                           Tile t) const;

  AbstractionOrder compute_abstraction_order(const TilesState15 &s,
                                             const ManhattanDist15 &md) const;

  void dump_abstraction_order(std::ostream &o) const;

  bool should_abstract(unsigned level, Tile t) const;

  static bool valid_level(unsigned level);


private:
  const TilesState15 start;
  const TilesState15 goal;

  const ManhattanDist15 md_heur;
  const AbstractionOrder abstraction_order;

  boost::pool<> node_pool;
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
