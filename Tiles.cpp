#include "Tiles.hpp"


TilesInstance15::TilesInstance15 (const TilesState15 &start,
                                  const TilesState15 &goal)
  : start(start)
  , goal(goal)
{
}


TilesInstance15::~TilesInstance15 ()
{
}


void TilesInstance15::print(std::ostream &o) const
{
  o << "Initial state:" << std::endl
    << start << std::endl;

  o << std::endl << "Goal state:" << std::endl
    << goal << std::endl;
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

  std::cerr << err << " " << 0 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "4") err = true;
  }

  std::cerr << err << " " << 1 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "4") err = true;
  }

  std::cerr << err << " " << 2 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "starting") err = true;
  }

  std::cerr << err << " " << 3 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "positions") err = true;

  }

  std::cerr << err << " " << 4 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "for") err = true;
  }

  std::cerr << err << " " << 5 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "each") err = true;
  }

  std::cerr << err << " " << 6 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "tile:") err = true;
  }

  std::cerr << err << " " << 7 << std::endl;
  if (!err) {
    err = readTiles(in, startTiles);
    startBlankIndex = startTiles[0];
  }

  std::cerr << err << " " << 8 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "goal") err = true;
  }

  std::cerr << err << " " << 9 << std::endl;
  if (!err) {
    in >> inputWord;
    if (inputWord != "positions:") err = true;
  }

  std::cerr << err << " " << 10 << std::endl;
  if (!err) {
    err = readTiles(in, goalTiles);
    goalBlankIndex = goalTiles[0];
  }

  std::cerr << err << " " << 11 << std::endl;
  if (!err) {
    TilesState15 start(startTiles, startBlankIndex);
    TilesState15 goal(goalTiles, goalBlankIndex);

    std::cerr << err << " " << 12 << std::endl;    
    if (start.valid() && goal.valid())
      return new TilesInstance15(start, goal);
  }

  return NULL;
}
