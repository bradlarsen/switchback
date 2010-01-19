/**
 * \file PancakeState.hpp
 *
 * A state in the pancake puzzle.
 *
 * \author eaburns
 * \date 18-01-2010
 */

#if !defined(_PANCAKE_STATE_H_)
#define _PANCAKE_STATE_H_

#include <boost/array.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <vector>

#include "pancake/PancakeTypes.hpp"

class PancakeState
{
public:
	// How many pancakes are there?
	virtual unsigned int size(void) const = 0;

	// Get an iterator over the pancakes.
	virtual std::vector<Pancake> get_cakes(void) const = 0;
};


class PancakeState14 : public PancakeState
{
public:
	// Get the canonical goal state.
	static PancakeState14 canonical_goal(void);

	// Read a pancake state from the given stream.
	static PancakeState14 *read(std::istream &i);

	// An empty state.
	PancakeState14();

	// Make a new 14-pancake state from the given array.
	PancakeState14(boost::array<Pancake, 14>);

	// Copy a pancake state.
	PancakeState14(const PancakeState14 &s);

	// Gets the pancake at the given index.
	inline Pancake operator[] (unsigned int i) const {
		return cakes[i];
	}

	// Gets the hash value of the state.
	std::size_t get_hash_value(void) const;

	// Flips the top [n] pancakes.
	PancakeState14 flip(unsigned int n) const;

	// Equality
	bool operator ==(const PancakeState14 &other) const;

	// Inequality
	bool operator !=(const PancakeState14 &other) const;

	// How many pancakes are there?
	inline unsigned int size(void) const {
		return 14;
	}

	// Get an iterator over the pancakes.
	inline std::vector<Pancake> get_cakes(void) const {
		return std::vector<Pancake>(cakes.begin(), cakes.end());
	}

private:
	// The pancake numbers.
	boost::array<Pancake, 14> cakes;
};

// Output a pancake state in a human-readable format.
std::ostream & operator<< (std::ostream &out, const PancakeState &cakes);

inline std::size_t hash_value(PancakeState14 const &s)
{
  return s.get_hash_value();
}

#endif /* !_PANCAKE_STATE_H_ */
