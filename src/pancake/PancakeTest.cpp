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
#include "pancake/PancakeInstance.hpp"

#include <iostream>

#include <stdlib.h>

// Some clumbsy testing on pancake states.
bool test_states(void)
{
	PancakeState14 *s_ptr = PancakeState14::read(std::cin);
	if (!s_ptr) {
		std::cerr << "PancakeState14::read failed" << std::endl;
		return EXIT_FAILURE;
	}

 	PancakeState14 s(*s_ptr);
	delete s_ptr;

	std::cout << s.get_hash_value() << ": " << s << std::endl;
	PancakeState14 flip4 = s.flip(4);
	std::cout << flip4.get_hash_value() << ": " << flip4 << std::endl;
	PancakeState14 flip5 = s.flip(5);
	std::cout << flip5.get_hash_value() << ": " << flip5 << std::endl;

	PancakeState14 s_again = flip5.flip(5);
	std::cout << s_again.get_hash_value() << ": " << s_again << std::endl;
	std::cout << "same: " << (s_again == s) << std::endl;

	return EXIT_SUCCESS;
}

// Testing some functionality of the pancake instances.
bool test_instance(void)
{
	PancakeInstance14 *inst = PancakeInstance14::read(std::cin);
	if (!inst)
		return EXIT_FAILURE;

	PancakeState14 goal = PancakeState14::canonical_goal();

	if (!inst->is_goal(goal)) {
		std::cerr << "PancakeInstance14::is_goal failed"
			  << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(void)
{
/*
	if (!test_states())
		return EXIT_FAILURE;
*/

	if (!test_instance())
		return EXIT_FAILURE;
}
