#include "Tiles.hpp"


TilesInstance15::TilesInstance15 (const TilesState15 &start,
                                  const TilesState15 &goal)
  : start(start)
  , goal(goal)
  , md_heur(ManhattanDist15(goal))
{
}


void TilesInstance15::print(std::ostream &o) const
{
  o << "Initial state:" << std::endl
    << start << std::endl;

  o << std::endl << "Goal state:" << std::endl
    << goal << std::endl;

  o << std::endl << "Initial heuristic estimate: "
    << static_cast<unsigned>(md_heur.compute_full(start))
    << std::endl;
}


bool TilesInstance15::isGoal(const TilesState15 &s) const
{
  return s == goal;
}


std::ostream & operator << (std::ostream &o, const TilesInstance15 &t)
{
  t.print(o);
  return o;
}



namespace
{
  bool readTiles (std::istream &in, TileArray &tiles)
  {
    unsigned pos;
    unsigned i;

    for (i = 0; i < 16; ++i) {
      in >> pos;
      if (pos < 0 || pos > 15) {
        return true;
      }
      else
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
    err = readTiles(in, startTiles);
    startBlankIndex = startTiles[0];
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
    goalBlankIndex = goalTiles[0];
  }

  if (!err) {
    TilesState15 start(startTiles, startBlankIndex);
    TilesState15 goal(goalTiles, goalBlankIndex);

    if (start.valid() && goal.valid())
      return new TilesInstance15(start, goal);
  }

  return NULL;
}
