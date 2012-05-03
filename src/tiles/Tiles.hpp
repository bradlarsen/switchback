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

  typedef std::pair<Tile, TileCost> TileCostPair;
  /*! \brief An AbstractionOrder indicates which tiles should be obscured
      at each level in an abstraction hierarchy.

      Given an AbstractionOrder `order`, A tile `t` should be abstracted at
      level `i` if and only if `order[i][t + 1]` is true.  Why `t + 1`?
      Because the value -1 is used to represent an obscured tile, and
      boost::array is indexed from 0.
   */
  typedef boost::array<
    boost::array<bool, 17>,
    num_abstraction_levels + 1
    > AbstractionOrder;

  /*! \brief Builds a static, domain-specific abstraction for the given board.

      This static abstraction covers the following tiles at each level:

          Level 1: 1 2 3 4 5 6 7
          Level 2: 1 2 3 4 5 6 7 8
          Level 3: 1 2 3 4 5 6 7 8 9
          Level 4: 1 2 3 4 5 6 7 8 9 10
          Level 5: 1 2 3 4 5 6 7 8 9 10 11
          Level 6: 1 2 3 4 5 6 7 8 9 10 11 12
          Level 7: 1 2 3 4 5 6 7 8 9 10 11 12 13
          Level 8: 1 2 3 4 5 6 7 8 9 10 11 12 13 14
   */
  static const AbstractionOrder static_abstraction_order;

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
                          std::vector<TilesNode15 *> &succs,
                          boost::pool<> &node_pool);

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
                            std::vector<TilesNode15 *> &succs,
                            boost::pool<> &node_pool);



  void compute_macro_successors(const TilesNode15 &n,
                                std::vector<TilesNode15 *> &succs,
                                boost::pool<> &node_pool);

  void compute_macro_predecessors(const TilesNode15 &n,
                                  std::vector<TilesNode15 *> &succs,
                                  boost::pool<> &node_pool);


  void compute_glued_successors(const TilesNode15 &n,
				std::vector<TilesNode15 *> &succs,
				Tile glued,
				boost::pool<> &node_pool);
  /**
   * Computes and assigns the heuristic for the given child node.
   */
  void compute_heuristic(const TilesNode15 &parent,
                         TilesNode15 &child) const;

  void compute_heuristic(TilesNode15 &child) const;

  const TilesState15 & get_start_state() const;

  const TilesState15 & get_goal_state() const;


  TilesState15 abstract(unsigned level, const TilesState15 &s) const;


  static bool is_valid_level(const unsigned level);


  // Set the abstraction order
  void set_abstraction_order(const AbstractionOrder ord) {
    abstraction_order = ord;
  }

  // Get the Manhattan distance heuristic.
  const ManhattanDist15 get_md(void) {
    return md_heur;
  }

  unsigned find_tile_index(const std::vector<TileCostPair> &pairs,
                           Tile t) const;

  bool should_abstract(unsigned level, Tile t) const;

  void dump_abstraction_order(std::ostream &o) const;

private:
  TilesNode15 * child(const TilesState15 &new_state,
                      TileCost new_g,
                      const TilesNode15 &parent,
                      boost::pool<> &node_pool);

  AbstractionOrder get_custom_abstraction(const TilesState15 &s,
                                          const ManhattanDist15 &md) const;

  static bool valid_level(unsigned level);


private:
  const TilesState15 start;
  const TilesState15 goal;

  const ManhattanDist15 md_heur;
  AbstractionOrder abstraction_order;
};


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t);
TilesInstance15 * readTilesInstance15 (std::istream &in);

#endif	/* !_TILES_HPP_ */
