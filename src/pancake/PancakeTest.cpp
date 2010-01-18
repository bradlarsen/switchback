/**
 * \file PancakeTest.cpp
 *
 * Some simple tests to make sure the pancake domain works properly.
 *
 * \author eaburns
 * \date 18-01-2010
 */

#include "pancake/PancakeTypes.hpp"
#include "pancake/PancakeState.hpp"

#include <stdlib.h>

int main(void)
{
	boost::array<Pancake, 14> cakes;

	for (unsigned int i = 0; i < 14; i += 1)
		cakes[i] = i +  1;

 	PancakeState14 s(cakes);

	std::cout << s.get_hash_value() << ": " << s << std::endl;
	std::cout << s s.get_hash_value() << ": " <<.flip(4) << std::endl;
	std::cout << s s.get_hash_value() << ": " <<.flip(5) << std::endl;

	return EXIT_SUCCESS;
}
