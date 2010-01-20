/**
 * \file GluedTiles.hpp
 *
 *
 *
 * \author eaburns
 * \date 19-01-2010
 */

#if !defined(_GLUED_TILES_INSTANCE_H_)
#define _GLUED_TILES_INSTANCE_H_

#include <boost/array.hpp>
#include <boost/pool/pool.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <vector>

#include "tiles/ManhattanDistance.hpp"
#include "tiles/Tiles.hpp"
#include "tiles/TilesState.hpp"
#include "tiles/TilesNode.hpp"

class GluedTilesInstance15 : boost::noncopyable
{
public:
	const Tile glued;

private:
	TilesInstance15 *tiles_instance;

public:
	static const unsigned num_abstraction_levels = 7;

	GluedTilesInstance15 (TilesInstance15 *tiles_instance, Tile glued);

	~GluedTilesInstance15 () {
		delete tiles_instance;
	}

	void print(std::ostream &o) const {
		tiles_instance->print(o);
	}

	bool is_goal(const TilesState15 &s) const {
		return tiles_instance->is_goal(s);
	}

	TileCost get_epsilon(const TilesState15 &s) const {
		return 1;
	}

	void compute_successors(const TilesNode15 &n,
				std::vector<TilesNode15 *> &succs,
				boost::pool<> &node_pool) {
		tiles_instance->compute_glued_successors(n, succs, glued,
							 node_pool);
	}

	void compute_predecessors(const TilesNode15 &n,
				  std::vector<TilesNode15 *> &succs,
				  boost::pool<> &node_pool) {
		compute_successors(n, succs, node_pool);
	}

	void compute_heuristic(const TilesNode15 &parent,
			       TilesNode15 &child) const {
		tiles_instance->compute_heuristic(parent, child);
		child.set_h(child.get_h() / 3);
	}

	void compute_heuristic(TilesNode15 &child) const {
		tiles_instance->compute_heuristic(child);
		child.set_h(child.get_h() / 3);
	}

	const TilesState15 & get_start_state() const {
		return tiles_instance->get_start_state();
	}

	const TilesState15 & get_goal_state() const {
		return tiles_instance->get_goal_state();
	}


	TilesState15 abstract(unsigned level, const TilesState15 &s) const {
		return tiles_instance->abstract(level, s);
	}


	static bool is_valid_level(const unsigned level) {
		return TilesInstance15::is_valid_level(level);
	}

private:
	TilesInstance15::AbstractionOrder
	compute_abstraction_order(const TilesState15 &s,
				  const ManhattanDist15 &md,
				  Tile glued);


	void dump_abstraction_order(std::ostream &o) const;
};

inline std::ostream & operator << (std::ostream &o,
				   const GluedTilesInstance15 &t)
{
	t.print(o);
	std::cout << "Tile " << t.glued << " is glued down" << std::endl;
	return o;
}

GluedTilesInstance15 * readGluedTilesInstance15 (std::istream &in);

#endif // !_GLUED_TILES_INSTANCE_H_
