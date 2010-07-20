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
	assert(c.write(num));

	if (num == COUNT / 2)
		throw std::runtime_error("test");
}

static void routine2(cio::channel<int> &c)
{
	for (int i = 0; i < COUNT; i++)
		assert(c.write(++count));

	c.close();
}

static void test()
{
	cio::channel<int> c;

	for (int i = 0; i < COUNT; i++)
		cio::routine(routine, c);

	for (int i = 0; i < COUNT; i++) {
		int num;
		assert(c.read(num));
		std::cout << num << std::endl;
	}

	cio::routine(routine2, c);

	for (cio::channel<int>::iterator i = c.begin(); i != c.end(); ++i)
		std::cout << *i << std::endl;
}

extern "C" void test_cpp()
{
	try {
		test();
	} catch (...) {
		assert(false);
	}
}
