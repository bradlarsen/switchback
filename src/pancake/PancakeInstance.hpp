/**
 * \file PancakeInstance.hpp
 *
 *
 *
 * \author eaburns
 * \date 18-01-2010
 */

#if !defined(_PANCAKE_H_)
#define _PANCAKE_H_

#include <boost/array.hpp>
#include <boost/pool/pool.hpp>
#include <boost/utility.hpp>

#include "pancake/PancakeState.hpp"
#include "pancake/PancakeNode.hpp"

#include <iostream>

class PancakeInstance14 : boost::noncopyable
{
public:
	// Number of levels that Rob Holte uses in their paper.
	static const unsigned num_abstraction_levels = 7;

	// Read a 14-pancake instance from an input stream.
	static PancakeInstance14 *read(std::istream &in);

private:
	typedef boost::array<boost::array<bool, 14>,
			     num_abstraction_levels + 1
			     > AbstractionOrder;

	// Create a simple abstraction order that is 7 deep and blanks
	// out one more tile each level.
	static AbstractionOrder
	simple_abstraction_order(const PancakeState14 &);

	// Wrap a child state in a new node.
	PancakeNode14 *child(const PancakeState14 &new_state,
			     PancakeCost new_g,
			     const PancakeNode14 &parent,
			     boost::pool<> &node_pool);


	// Test if pancake number [i] should be abstracted away.
	bool should_abstract(unsigned int level, unsigned int i) const;

public:

	// Create a new pancake puzzle instance
	PancakeInstance14(const PancakeState14 &start,
			  const PancakeState14 &goal);

	// Test if the given state is the goal state.
	inline bool is_goal(const PancakeState14 &s) const {
		return s == goal;
	}

	// Smallest out-edge cost.
	inline PancakeCost get_epsilon(const PancakeState14 &s) const {
		return 1;
	}

	// Compute the successors of the given node.
	//
	// Note that this does not assign the h values for the
	// successor nodes: that must be done by the caller.
	void compute_successors(const PancakeNode14 &n,
				std::vector<PancakeNode14*> &succs,
				boost::pool<> &node_pool);

	// Compute the predecessors of the given node.
	//
	// Note that this does not assign the h values for the
	// successor nodes: that must be done by the caller.
	void compute_predecessors(const PancakeNode14 &n,
				  std::vector<PancakeNode14*> &succs,
				  boost::pool<> &node_pool);


	// Compute/fill-in the heuristic value for a child node.
	void compute_heuristic(const PancakeNode14 &parent,
			       PancakeNode14 &child) const;

	void compute_heuristic(PancakeNode14 &child) const;


	// Access the start state.
	const PancakeState14 &get_start_state() const;


	// Access the goal state.
	const PancakeState14 &get_goal_state() const;


	// Get the abstract representation of a state at the given level.
	PancakeState14 abstract(unsigned int level,
				const PancakeState14 &s) const;


	// Is this level of abstraction valid?
	static bool is_valid_level(const unsigned level) {
		return level <= num_abstraction_levels;
	}

private:
	const PancakeState14 start;
	const PancakeState14 goal;
	const AbstractionOrder abstraction_order;
};

std::ostream &operator<< (std::ostream &, const PancakeInstance14 &);

#endif	// !_PANCAKE_H_
