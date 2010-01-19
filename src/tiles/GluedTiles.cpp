/**
 * \file GluedTiles.cpp
 *
 *
 *
 * \author eaburns
 * \date 19-01-2010
 */

#include "tiles/GluedTiles.hpp"

/*
GluedTilesInstance15::AbstractionOrder
GluedTilesInstance15::compute_abstraction_order(const TilesState15 &s,
						const ManhattanDist15 &md,
						Tile glued) const
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

*/

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
	if (glued < 0 || glued > 15) {
		std::cerr << glued
			  << " is an invalid glued tile"
			  << std::endl;
		return NULL;
	}

	return new GluedTilesInstance15(tiles, glued);
}
