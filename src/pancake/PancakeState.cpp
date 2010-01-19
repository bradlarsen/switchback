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

PancakeState14 PancakeState14::canonical_goal(void)
{
	boost::array<Pancake, 14> cakes;

	for (unsigned int i = 0; i < cakes.size(); i += 1)
		cakes[i] = i + 1;

	return PancakeState14(cakes);
}

// Read the pancake numbers from a file.
// Return true on success and false on failure.
static bool readCakes14(std::istream &in, boost::array<Pancake, 14> &cakes)
{
	for (unsigned int i = 0; i < cakes.size(); i += 1) {
		unsigned int c;

		in >> c;
		if (c <= 0 || c > 14) {
			std::cout << "Cake " << c << " is out of bounds"
				  << std::endl;
			return false;
		}

		cakes[i] = c;
	}

	return true;
}


PancakeState14 *PancakeState14::read(std::istream &in)
{
	boost::array<Pancake, 14> cakes;

	if (!readCakes14(in, cakes)) {
		std::cerr << "Error reading pancakes" << std::endl;
		return NULL;
	}

	return new PancakeState14(cakes);
}

////////////////////////////////////////////////////////////
// Members
////////////////////////////////////////////////////////////

PancakeState14::PancakeState14(boost::array<Pancake, 14> cs) : cakes(cs) {}


PancakeState14::PancakeState14(const PancakeState14 &s)
	: cakes(s.cakes) { }


std::size_t PancakeState14::get_hash_value(void) const
{
	return boost::hash_range(cakes.begin(), cakes.end());
}


PancakeState14 PancakeState14::flip(unsigned int n) const
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


bool PancakeState14::operator ==(const PancakeState14 &other) const
{
	unsigned int n = cakes.size();

	for (unsigned int i = 0; i <  n; i += 1)
		if (cakes[i] != other.cakes[i])
			return false;

	return true;
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
