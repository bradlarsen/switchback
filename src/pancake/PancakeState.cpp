/**
 * \file PancakeState.cpp
 *
 *
 *
 * \author eaburns
 * \date 18-01-2010
 */

#include <boost/array.hpp>
#include <boost/functional/hash.hpp>

#include "pancake/PancakeState.hpp"

PancakeState14::PancakeState14(boost::array<Pancake, 14> cs) : cakes(cs) {}


std::size_t PancakeState14::get_hash_value(void) const
{
	return boost::hash_range(cakes.begin(), cakes.end());
}


PancakeState14 PancakeState14::flip(unsigned int n)
{
	boost::array<Pancake, 14> result;
	unsigned int nswaps = n / 2;

	// Copy the flip of the top of the stack.
	for (unsigned int i = 0; i < nswaps; i += 1) {
		unsigned int j = (n - 1) - i;
		result[i] = cakes[j];
		result[j] = cakes[i];
	}

	// If odd, copy the middle cake in the flip.
	if (n % 2 != 0)
		result[n / 2] = cakes[n / 2];

	// Copy the rest.
	for (unsigned int i = n; i < cakes.size(); i += 1)
		result[i] = cakes[i];

	return result;
}


std::ostream & operator<< (std::ostream &out, const PancakeState &s)
{
	std::vector<Pancake> cakes = s.get_cakes();

	for (unsigned int i = 0; i < cakes.size(); i += 1) {
		unsigned int num = cakes[i];
		out << num;
		if (i < cakes.size() - 1)
			out << " ";
	}

	return out;
}
