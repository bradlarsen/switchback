#include <cassert>

#include "tiles/ManhattanDistance.hpp"


ManhattanDist15::ManhattanDist15(const TilesState15 &goal)
{
  init(goal);
}

void ManhattanDist15::init(const TilesState15 &goal)
{
  TileIndex goal_pos;

  for (Tile tile = 1; tile < 16; tile += 1) {
    for (goal_pos = 0; goal_pos < 16; goal_pos += 1) {
      if (goal.get_tiles()[goal_pos] == tile)
        break;
    }

    assert(goal.get_tiles()[goal_pos] == tile);
    int goal_col = goal_pos % 4;
    int goal_row = goal_pos / 4;
    for (unsigned pos = 0; pos < 16; pos += 1) {
      int col = pos % 4;
      int row = pos / 4;

      table[tile][pos] = abs(goal_col - col) + abs(goal_row - row);
    }
  }

  for (unsigned i = 0; i < table.size(); ++i) {
    for (unsigned j = 0; j < table[i].size(); j += 1)
      if (i == 0)
        assert(table[i][j] == 0);
      else
        assert(table[i][j] <= 6);
  }
}
