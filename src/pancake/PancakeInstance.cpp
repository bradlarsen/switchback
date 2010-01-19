/**
 * \file PancakeInstance.cpp
 *
 *
 *
 * \author eaburns
 * \date 18-01-2010
 */

#include <boost/array.hpp>

#include "pancake/PancakeInstance.hpp"
#include "pancake/PancakeState.hpp"

#include <vector>

PancakeInstance14::PancakeInstance14 *PancakeInstance14::read(std::istream &in)
{
	unsigned int ncakes;

	in >> ncakes;
	if (ncakes != 14) {
		std::cerr << "Invalid number of pancakes, expected 14, "
			  << "got " << ncakes << std::endl;
		return NULL;
	}

	PancakeState14 *start = PancakeState14::read(in);
	if (!start)
		return NULL;
	PancakeState14 cgoal = PancakeState14::canonical_goal();
	PancakeState14 *goal = new PancakeState14(cgoal);

	return new PancakeInstance14(*start, *goal);
}


// Compute a simple abstraction order that blanks out one more tile
// each level.
PancakeInstance14::AbstractionOrder
PancakeInstance14::simple_abstraction_order(const PancakeState14 &s)
{
	AbstractionOrder abst;
	unsigned int ncakes = abst[0].size();

	for (unsigned int lvl = 0;
	     lvl <= PancakeInstance14::num_abstraction_levels;
	     lvl += 1) {
		for (unsigned int i = 0; i < ncakes; i += 1) {
			if (lvl == 0)
				abst[lvl][i] = false;
			else if (i < ncakes - (num_abstraction_levels - lvl))
				abst[lvl][i] = true;
			else
				abst[lvl][i] = false;
		}
	}

	return abst;
}


PancakeInstance14::PancakeInstance14(const PancakeState14 &s,
				     const PancakeState14 &g)
	: start(s),
	  goal(g),
	  abstraction_order(simple_abstraction_order(s))
{ }


PancakeNode14 *PancakeInstance14::child(const PancakeState14 &new_state,
					PancakeCost new_g,
					const PancakeNode14 &parent,
					boost::pool<> &node_pool)
{
	assert(!(parent.get_state() == new_state));
	assert((parent.get_parent() == NULL)
	       || !(new_state == parent.get_parent()->get_state()));

	PancakeNode14 *child_node =
		new (node_pool.malloc()) PancakeNode14(new_state,
						       new_g,
						       0,
						       &parent);

	return child_node;
}


void PancakeInstance14::compute_successors(const PancakeNode14 &node,
					   std::vector<PancakeNode14*> &succs,
					   boost::pool<> &node_pool)
{
	const PancakeNode14 *gp = node.get_parent();

	succs.clear();
	for (unsigned int n = 2; n <= 14; n += 1) {
		const PancakeState14 child_state = node.get_state().flip(n);
		if (!(gp->get_state() == child_state)) {
			PancakeNode14 *child_node = child(child_state,
							  node.get_g() + 1,
							  node,
							  node_pool);
			succs.push_back(child_node);
		}
	}
}


void
PancakeInstance14::compute_predecessors(const PancakeNode14 &n,
					std::vector<PancakeNode14*> &succs,
					boost::pool<> &node_pool)
{
	compute_predecessors(n, succs, node_pool);
}

void PancakeInstance14::compute_heuristic(const PancakeNode14 &parent,
					  PancakeNode14 &child) const
{
	compute_heuristic(child);
}

void PancakeInstance14::compute_heuristic(PancakeNode14 &child) const
{
	// Just use h(n) = 0 for now, I guess.
	child.set_h(0);
}

bool
PancakeInstance14::should_abstract(unsigned int level, unsigned int i) const
{
	return abstraction_order[level][i];
}


const PancakeState14 &PancakeInstance14::get_start_state() const
{
	return start;
}


const PancakeState14 &PancakeInstance14::get_goal_state() const
{
	return goal;
}


PancakeState14 PancakeInstance14::abstract(unsigned int level,
					   const PancakeState14 &s) const
{
	std::vector<Pancake> cakes = s.get_cakes();
	boost::array<Pancake, 14> new_cakes;

	for (unsigned int i = 0; i < cakes.size(); i += 1) {
		if (should_abstract(level, cakes[i]))
			new_cakes[i] = -1;
		else
			new_cakes[i] = cakes[i];
	}

	return PancakeState14(new_cakes);

}
