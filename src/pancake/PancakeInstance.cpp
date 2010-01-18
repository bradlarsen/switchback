/**
 * \file PancakeInstance.cpp
 *
 *
 *
 * \author eaburns
 * \date 18-01-2010
 */

#include "pancake/PancakeInstance.hpp"
#include "pancake/PancakeState.hpp"

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

