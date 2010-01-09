#include <cassert>

#include "ManhattanDistance.hpp"


ManhattanDist15::ManhattanDist15(const TilesState15 &goal)
{
  init(goal);
}

void ManhattanDist15::init(const TilesState15 &goal)
{
  TileIndex goal_pos;

  for (Tile tile = 1; tile < 16; tile += 1) {
    for (goal_pos = 0; goal_pos < 16; goal_pos += 1) {
      if (goal.tiles[goal_pos] == tile)
        break;
    }

    assert(goal.tiles[goal_pos] == tile);
    int goal_col = goal_pos % 4;
    int goal_row = goal_pos / 4;
    for (unsigned pos = 0; pos < 16; pos += 1) {
      int col = pos % 4;
      int row = pos / 4;

      table[tile * 16 + pos] = abs(goal_col - col) + abs(goal_row - row);
    }
  }
}

Cost ManhattanDist15::compute_full(const TilesState15 &s) const
{
  Cost dist = 0;

  for (unsigned i = 0; i < 16; i += 1)
    if (s.tiles[i] != 0)
      dist += lookup_dist(s.tiles[i], i);

  return dist;
}

Cost ManhattanDist15::lookup_dist(Tile tile, TileIndex pos) const
{
  return table[tile * 16 + pos];
}



