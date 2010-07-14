#include <iostream>
#include <stdexcept>

#include <cio/channel.hpp>
#include <cio/routine.hpp>

#include "assert.h"

#define COUNT 8

static int count;

static void routine(cio::channel<int> &c)
{
	int num = ++count;
	c << num;

	if (num == COUNT / 2)
		throw std::runtime_error("test");
}

static void test()
{
	cio::channel<int> c;

	for (int i = 0; i < COUNT; i++)
		cio::routine(routine, c);

	for (int i = 0; i < COUNT; i++) {
		int num;
		c >> num;
		std::cout << num << std::endl;
	}
}

extern "C" void test_cpp()
{
	try {
		test();
	} catch (...) {
		assert(false);
	}
}
