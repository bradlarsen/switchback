#include <algorithm>
#include <cassert>
#include <iomanip>

#include "tiles/Tiles.hpp"

TilesInstance15::TilesInstance15 (const TilesState15 &start,
                                  const TilesState15 &goal)
    : start(start)
    , goal(goal)
    , md_heur(goal)
    , abstraction_order(get_custom_abstraction(start, md_heur))
{
  assert(is_goal(goal));
}


void TilesInstance15::print(std::ostream &o) const
{
  o << "Initial state:" << std::endl
    << start << std::endl;

  o << std::endl << "Goal state:" << std::endl
    << goal << std::endl;

  o << std::endl << "Initial Manhattan distance heuristic estimate: "
    << md_heur.compute_full(start)
    << std::endl;
}


TilesNode15 * TilesInstance15::child(const TilesState15 &new_state,
                                     TileCost new_g,
                                     const TilesNode15 &parent,
                                     boost::pool<> &node_pool)
{
  assert(new_state != parent.get_state());
  assert(parent.get_parent() == NULL ||
         new_state != parent.get_parent()->get_state());
  TilesNode15 *child_node =
    new (node_pool.malloc()) TilesNode15(new_state,
                                         new_g,
                                         0,
                                         &parent);
  return child_node;
}


void TilesInstance15::compute_heuristic(const TilesNode15 &parent,
                                        TilesNode15 &child) const
{
  TileCost new_h = md_heur.compute_incr(child.get_state(), parent);
  if (!is_goal(child.get_state()))
    new_h = 1 > new_h ? 1 : new_h;
  child.set_h(new_h);
}


void TilesInstance15::compute_heuristic(TilesNode15 &child) const
{
  TileCost new_h = md_heur.compute_full(child.get_state());
  if (!is_goal(child.get_state()))
    new_h = 1 > new_h ? 1 : new_h;
  child.set_h(new_h);
}


void TilesInstance15::compute_successors(const TilesNode15 &n,
                                         std::vector<TilesNode15 *> &succs,
                                         boost::pool<> &node_pool)
{
  succs.clear();
  const TilesNode15 *gp = n.get_parent();

  const unsigned blank = n.get_state().get_blank();
  const unsigned col = blank % 4;
  const unsigned row = blank / 4;

  const TileCost g = n.get_g();
  const TileCost new_g = g + 1;

  if (col > 0 && (gp == NULL || gp->get_state().get_blank() != blank - 1)) {
    const TilesState15 new_state = n.get_state().move_blank_left();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
  if (col < 3 && (gp == NULL || gp->get_state().get_blank() != blank + 1)) {
    const TilesState15 new_state = n.get_state().move_blank_right();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
  if (row > 0 && (gp == NULL || gp->get_state().get_blank() != blank - 4)) {
    const TilesState15 new_state = n.get_state().move_blank_up();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
  if (row < 3 && (gp == NULL || gp->get_state().get_blank() != blank + 4)) {
    const TilesState15 new_state = n.get_state().move_blank_down();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
}

void
TilesInstance15::compute_glued_successors(const TilesNode15 &n,
					  std::vector<TilesNode15 *> &succs,
					  Tile glued,
					  boost::pool<> &node_pool)
{
  succs.clear();
  const TilesNode15 *gp = n.get_parent();

  const TilesState15 p = n.get_state();
  const unsigned blank = n.get_state().get_blank();
  const unsigned col = blank % 4;
  const unsigned row = blank / 4;

  const TileCost g = n.get_g();
  const TileCost new_g = g + 1;

  if (col > 0 && (gp == NULL || gp->get_state().get_blank() != blank - 1)
      && (p.get_left_tile() != glued)) {
    const TilesState15 new_state = n.get_state().move_blank_left();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
  if (col < 3 && (gp == NULL || gp->get_state().get_blank() != blank + 1)
      && (p.get_right_tile() != glued)) {
    const TilesState15 new_state = n.get_state().move_blank_right();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
  if (row > 0 && (gp == NULL || gp->get_state().get_blank() != blank - 4)
      && (p.get_up_tile() != glued)) {
    const TilesState15 new_state = n.get_state().move_blank_up();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
  if (row < 3 && (gp == NULL || gp->get_state().get_blank() != blank + 4)
      && (p.get_down_tile() != glued)) {
    const TilesState15 new_state = n.get_state().move_blank_down();
    TilesNode15 *child_node = child(new_state, new_g, n, node_pool);
    succs.push_back(child_node);
  }
}


void TilesInstance15::compute_predecessors(const TilesNode15 &n,
                                           std::vector<TilesNode15 *> &succs,
                                           boost::pool<> &node_pool)
{
  compute_successors(n, succs, node_pool);
}


void TilesInstance15::compute_macro_successors(const TilesNode15 &n,
                                               std::vector<TilesNode15 *> &succs,
                                               boost::pool<> &node_pool)
{
  succs.clear();
  const TilesNode15 *gp = n.get_parent();

  const unsigned blank = n.get_state().get_blank();
  const unsigned col = blank % 4;
  const unsigned row = blank / 4;

  const TileCost g = n.get_g();
  const TileCost new_g = g + 1;

  // move the blank up
  for (unsigned i = 0; i < row; i += 1) {
    const unsigned new_blank = col + 4 * i;
    if (gp == NULL || gp->get_state().get_blank() != new_blank) {
      TileArray new_tiles = n.get_state().get_tiles();
      for(unsigned idx = blank; idx >= 4 * (i + 1); idx -= 4){
        std::swap(new_tiles[idx], new_tiles[idx - 4]);
      }

      TilesNode15 *child_node = child(TilesState15(new_tiles), new_g, n, node_pool);
      succs.push_back(child_node);
    } /* end if */
  } /* end for */

  // move the blank down
  for (unsigned i = row + 1; i < 4; i += 1) {
    const unsigned new_blank = col + 4 * i;
    if (gp == NULL || gp->get_state().get_blank() != new_blank) {
      TileArray new_tiles = n.get_state().get_tiles();
      for(unsigned idx = blank + 4; idx <= new_blank; idx += 4){
        std::swap(new_tiles[idx], new_tiles[idx - 4]);
      }

      TilesNode15 *child_node = child(TilesState15(new_tiles), new_g, n, node_pool);
      succs.push_back(child_node);
    } /* end if */
  } /* end for */

  // move the blank left
  for (unsigned j = 0; j < col; j += 1) {
    const unsigned new_blank = row * 4 + j;
    if (gp == NULL || gp->get_state().get_blank() != new_blank) {
      TileArray new_tiles = n.get_state().get_tiles();
      for(unsigned idx = blank; idx > new_blank; idx -= 1){
        std::swap(new_tiles[idx], new_tiles[idx - 1]);
      }

      TilesNode15 *child_node = child(TilesState15(new_tiles), new_g, n, node_pool);
      succs.push_back(child_node);
    } /* end if */
  } /* end for */

  // move the blank right
  for (unsigned j = col + 1; j < 4; j += 1) {
    const unsigned new_blank = row * 4 + j;
    if (gp == NULL || gp->get_state().get_blank() != new_blank) {
      TileArray new_tiles = n.get_state().get_tiles();
      for(unsigned idx = blank; idx < new_blank; idx += 1){
        std::swap(new_tiles[idx], new_tiles[idx + 1]);
      }

      TilesNode15 *child_node = child(TilesState15(new_tiles), new_g, n, node_pool);
      succs.push_back(child_node);
    } /* end if */
  } /* end for */

  assert(succs.size() <= 6);
#ifndef NDEBUG
  if (gp != NULL) {
    for (unsigned i = 0; i < succs.size(); i += 1) {
      assert(succs[i]->get_state() != gp->get_state());
    }
  }
#endif
}


void TilesInstance15::compute_macro_predecessors(const TilesNode15 &n,
                                                 std::vector<TilesNode15 *> &succs,
                                                 boost::pool<> &node_pool)
{
  compute_macro_successors(n, succs, node_pool);
}



const TilesState15 & TilesInstance15::get_start_state() const
{
  return start;
}


const TilesState15 & TilesInstance15::get_goal_state() const
{
  return goal;
}


TilesState15 TilesInstance15::abstract(unsigned level,
                                       const TilesState15 &s) const
{
  assert(valid_level(level));

  TileArray new_tiles(s.get_tiles());

  for (TileIndex i = 0; i < 4; i += 1)
    for (TileIndex j = 0; j < 4; j += 1) {
      if ( should_abstract(level, s(i, j)) )
        new_tiles[i * 4 + j] = -1;
    }

  return TilesState15(new_tiles);
}


bool TilesInstance15::is_valid_level(const unsigned level)
{
  return level <= num_abstraction_levels;
}



std::ostream & operator <<(std::ostream &o, const TilesInstance15 &t)
{
  t.print(o);
  t.dump_abstraction_order(o);
  return o;
}



namespace
{
  bool readTiles (std::istream &in, TileArray &tiles)
  {
    unsigned pos;
    unsigned i;

    for (i = 0; i < tiles.size(); ++i) {
      in >> pos;
      if (pos > 15) {
        return true;
      }

      tiles[pos] = i;
    }

    return i != 16;
  }
}


/**
 * Reads a tiles instance from the given input stream.  If there is an
 * error, NULL is returned.  Otherwise, a pointer to a newly allocated
 * TilesInstance15 is returned.  The caller is responsible for
 * cleaning up a returned TilesInstance15.
 */
TilesInstance15 * readTilesInstance15 (std::istream &in)
{
  bool err = false;

  std::string inputWord;

  TileArray startTiles;

  TileArray goalTiles;

  if (!err) {
    in >> inputWord;
    if (inputWord != "4") err = true;
  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "4") err = true;
  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "starting") err = true;
  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "positions") err = true;

  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "for") err = true;
  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "each") err = true;
  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "tile:") err = true;
  }

  if (!err) {
    err = readTiles(in, startTiles);
  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "goal") err = true;
  }

  if (!err) {
    in >> inputWord;
    if (inputWord != "positions:") err = true;
  }

  if (!err) {
    err = readTiles(in, goalTiles);
  }

  if (!err) {
    TilesState15 start(startTiles);
    TilesState15 goal(goalTiles);

    if (start.valid() && goal.valid())
      return new TilesInstance15(start, goal);
  }

  return NULL;
}


namespace
{
  template <class T>
  struct lt_snd
  {
    bool operator ()(const T &p1, const T &p2) const
    {
      return p1.second < p2.second;
    }
  };
}


unsigned
TilesInstance15::find_tile_index(const std::vector<TileCostPair> &pairs,
                                 Tile t) const
{
  for (unsigned i = 0; i < pairs.size(); i += 1)
    if (pairs[i].first == t)
      return i;

  assert(false);
  return 0;
}


TilesInstance15::AbstractionOrder
TilesInstance15::get_custom_abstraction(const TilesState15 &s,
                                        const ManhattanDist15 &md) const
{
  std::vector<TileCostPair> pairs;

  for (TileIndex i = 0; i < 4; i += 1)
    for (TileIndex j = 0; j < 4; j += 1) {
      Tile tile = s(i, j);
      if (tile == 0)
        continue;
      TileCost cost = md.lookup_dist(tile, i * 4 + j);
      pairs.push_back(std::make_pair(tile, cost));
    }

  std::sort(pairs.begin(), pairs.end(), lt_snd<TileCostPair>());
  assert(pairs.size() == 15);

  AbstractionOrder order;
  order[0].assign(false);
  for (unsigned level = 1; level <= num_abstraction_levels; level += 1) {
    order[level][0] = false;
    order[level][1] = false;
    for (Tile t = 1; t < 16; t += 1)
      order[level][t + 1] = find_tile_index(pairs, t) < level + (14 - num_abstraction_levels);
  }

  return order;
}

const TilesInstance15::AbstractionOrder
TilesInstance15::static_abstraction_order =
  {{
    //A  B  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}},
    {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}},
  }};

void TilesInstance15::dump_abstraction_order(std::ostream &o) const
{
  o << "The following abstraction schedule will be used:" << std::endl;

  o << std::endl << "level:  tiles to be obscured" << std::endl;
  for (unsigned level = 0; level <= num_abstraction_levels; level += 1) {
    o << "  " << level << ": ";
    for (Tile t = -1; t <= 15; t += 1)
      if (should_abstract(level, t))
        o << std::setw(4) << t;
      else
        o << "    ";
    o << std::endl;
  }
}


bool TilesInstance15::should_abstract(unsigned level, Tile t) const
{
  assert(valid_tile(t));
  assert(valid_level(level));

  return abstraction_order[level][t+1];
}


bool TilesInstance15::valid_level(unsigned level)
{
  return level <= num_abstraction_levels;
}
