/**
 * \file GluedTiles.cpp
 *
 *
 *
 * \author eaburns
 * \date 19-01-2010
 */

#include "tiles/GluedTiles.hpp"
#include "tiles/TilesTypes.hpp"

#include <iomanip>

GluedTilesInstance15::GluedTilesInstance15 (TilesInstance15 *tiles_instance,
					    Tile glued)
	: glued(glued), tiles_instance(tiles_instance)
{
	// Over-ride the abstraction order so we don't abstract out
	// the glued tile.
	TilesInstance15::AbstractionOrder ord;
	ord = compute_abstraction_order(tiles_instance->get_start_state(),
					tiles_instance->get_md(),
					glued);
	tiles_instance->set_abstraction_order(ord);

#ifndef NDEBUG
	dump_abstraction_order(std::cerr);
#endif
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


void GluedTilesInstance15::dump_abstraction_order(std::ostream &o) const
{
  o << "The following abstraction schedule will really be used:" << std::endl;
  for (unsigned level = 0; level <= num_abstraction_levels; level += 1) {
    o << "  " << level << ": ";
    for (Tile t = -1; t <= 15; t += 1)
      if (tiles_instance->should_abstract(level, t))
        o << std::setw(4) << t;
      else
        o << "    ";
    o << std::endl;
  }
}

TilesInstance15::AbstractionOrder
GluedTilesInstance15::compute_abstraction_order(const TilesState15 &s,
						const ManhattanDist15 &md,
						Tile glued)
{
	std::vector<TilesInstance15::TileCostPair> pairs;

	for (TileIndex i = 0; i < 4; i += 1)
		for (TileIndex j = 0; j < 4; j += 1) {
			Tile tile = s(i, j);
			if (tile == 0 || tile == glued)
				continue;
			TileCost cost = md.lookup_dist(tile, i * 4 + j);
			pairs.push_back(std::make_pair(tile, cost));
		}

	std::sort(pairs.begin(), pairs.end(),
		  lt_snd<TilesInstance15::TileCostPair>());
	assert(pairs.size() == 14);

	TilesInstance15::AbstractionOrder order;
	order[0].assign(false);
	for (unsigned level = 1; level <= num_abstraction_levels; level += 1) {
		order[level][0] = false;
		order[level][1] = false;
		order[level][glued + 1] = false;
		for (Tile t = 1; t < 16; t += 1) {
			if (t == glued)
				continue;
			order[level][t + 1] =
				tiles_instance->find_tile_index(pairs, t)
				< level + (14 - num_abstraction_levels);
		}
	}

	return order;
}


GluedTilesInstance15 * readGluedTilesInstance15 (std::istream &in)
{
	unsigned int glued = 0;
	std::string inputWord;

	TilesInstance15 *tiles = readTilesInstance15(in);
	if (!tiles) {
		std::cerr << "Failed to load tiles instance." << std::endl;
		return NULL;
	}

	in >> inputWord;
	if (inputWord != "glued:") {
		std::cerr << "Expected keyword 'glued', got "
			  << inputWord << std::endl;
		return NULL;
	}

	in >> glued;
	if (glued > 15) {
		std::cerr << glued
			  << " is an invalid glued tile"
			  << std::endl;
		return NULL;
	}

	return new GluedTilesInstance15(tiles, glued);
}
