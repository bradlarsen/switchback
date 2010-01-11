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
                             std::vector<TilesNode15 *> &succs)
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


TilesNode15 * TilesInstance15::create_start_node()
{
  Cost h = md_heur.compute_full(start);
  return new (node_pool.malloc())
    TilesNode15(start, 0, h, static_cast<const TilesNode15 *>(NULL));
}


unsigned TilesInstance15::get_num_buckets() const
{
  return 100;
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
