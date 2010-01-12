#include <algorithm>
#include <cassert>

#include "tiles/Tiles.hpp"


void TilesInstance15::print(std::ostream &o) const
{
  o << "Initial state:" << std::endl
    << start << std::endl;

  o << std::endl << "Goal state:" << std::endl
    << goal << std::endl;

  o << std::endl << "Initial heuristic estimate: "
    << md_heur.compute_full(start)
    << std::endl;
}


void TilesInstance15::expand(const TilesNode15 &n,
                             std::vector<TilesNode15 *> &succs) const
{
  succs.clear();
  const TilesNode15 *gp = n.get_parent();

  const unsigned blank = n.get_state().get_blank();
  const unsigned col = blank % 4;
  const unsigned row = blank / 4;

  const Cost g = n.get_g();
  const Cost new_g = g + 1;

  if (col > 0 && (gp == NULL || gp->get_state().get_blank() != blank - 1)) {
    const TilesState15 new_state = n.get_state().move_blank_left();
    TilesNode15 *child_node = child(new_state, new_g, n);
    succs.push_back(child_node);
  }
  if (col < 3 && (gp == NULL || gp->get_state().get_blank() != blank + 1)) {
    const TilesState15 new_state = n.get_state().move_blank_right();
    TilesNode15 *child_node = child(new_state, new_g, n);
    succs.push_back(child_node);
  }
  if (row > 0 && (gp == NULL || gp->get_state().get_blank() != blank - 4)) {
    const TilesState15 new_state = n.get_state().move_blank_up();
    TilesNode15 *child_node = child(new_state, new_g, n);
    succs.push_back(child_node);
  }
  if (row < 3 && (gp == NULL || gp->get_state().get_blank() != blank + 4)) {
    const TilesState15 new_state = n.get_state().move_blank_down();
    TilesNode15 *child_node = child(new_state, new_g, n);
    succs.push_back(child_node);
  }
}


const TilesState15 & TilesInstance15::get_start_state() const
{
  return start;
}


const TilesState15 & TilesInstance15::get_goal_state() const
{
  return goal;
}


TilesNode15 * TilesInstance15::create_start_node() const
{
  Cost h = md_heur.compute_full(start);
  return new (NodePool::malloc())
    TilesNode15(start, 0, h, static_cast<const TilesNode15 *>(NULL));
}


void TilesInstance15::free_node(TilesNode15 *n) const
{
  assert(NodePool::is_from(n));
  NodePool::free(n);
}


TilesState15 TilesInstance15::abstract(const TilesState15 &s, unsigned level) const
{
  assert(level < 8);

  TileArray new_tiles(s.get_tiles());

  for (TileIndex i = 0; i < 4; i += 1)
    for (TileIndex j = 0; j < 4; j += 1) {
      if ( should_abstract(s(i, j), level) )
        new_tiles[i * 4 + j] = -1;
    }

  return TilesState15(new_tiles, s.get_blank());
}



std::ostream & operator <<(std::ostream &o, const TilesInstance15 &t)
{
  t.print(o);
  return o;
}



namespace
{
  bool readTiles (std::istream &in, TileIndex &blank, TileArray &tiles)
  {
    unsigned pos;
    unsigned i;

    for (i = 0; i < 16; ++i) {
      in >> pos;
      if (pos < 0 || pos > 15) {
        return true;
      }

      if (i == 0)
        blank = pos;

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
  TileIndex startBlankIndex;

  TileArray goalTiles;
  TileIndex goalBlankIndex;

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
    err = readTiles(in, startBlankIndex, startTiles);
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
    err = readTiles(in, goalBlankIndex, goalTiles);
  }

  if (!err) {
    TilesState15 start(startTiles, startBlankIndex);
    TilesState15 goal(goalTiles, goalBlankIndex);

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


std::vector<TilesInstance15::TileCostPair>
TilesInstance15::compute_abstraction_order(const TilesState15 &s,
                                           const ManhattanDist15 &md) const
{
  std::vector<TileCostPair> pairs;

  for (TileIndex i = 0; i < 4; i += 1)
    for (TileIndex j = 0; j < 4; j += 1) {
      Tile tile = s(i, j);
      Cost cost = md.lookup_dist(tile, i * 4 + j);
      pairs.push_back(std::make_pair(tile, cost));
    }

  sort(pairs.begin(), pairs.end(), lt_snd<TileCostPair>());

  for(std::vector<TileCostPair>::iterator it = pairs.begin();
      it != pairs.end();
      ++it) {
    if (it->first == 0) {
      pairs.erase(it);
      break;
    }
  }

  assert(pairs.size() == 15);
  return pairs;
}


void TilesInstance15::dump_abstraction_order(std::ostream &o) const
{
  o << "The abstraction order:" << std::endl;
  for (unsigned i = 0; i < abstraction_order.size(); i += 1) {
    const TilesInstance15::TileCostPair &p = abstraction_order[i];
    o << "  tile " << p.first << " has cost " << p.second << std::endl;
  }

  o << "The following abstraction schedule will be used:" << std::endl;
  for (unsigned level = 0; level < 8; level += 1) {
    o << "  " << level << ": ";
    for (Tile t = 1; t <= 15; t += 1)
      if (should_abstract(t, level))
        o << t << " ";
    o << std::endl;
  }
}


bool TilesInstance15::should_abstract(Tile t, unsigned level) const
{
  assert(-1 <= t && t <= 15);
  assert(level < 8);

  if (t == -1)
    return false;

  unsigned t_idx = 0;
  for (unsigned i = 0; i < abstraction_order.size(); i += 1)
    if (abstraction_order[i].first == t) {
      t_idx = i;
      break;
    }

  return t_idx < level + 7;
}
