#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "assert.h"

static const char *progname;
static int tests;
static int passed;

static void run(void (*func)(void), const char *name)
{
	pid_t pid;
	int status;

	printf("  %-9s %s %s\n", "Check", progname, name);

	tests++;

	pid = fork();
	assert(pid >= 0);

	if (pid == 0) {
		func();
		exit(0);
	}

	assert(waitpid(pid, &status, 0) == pid);

	if (status == 0)
		passed++;
	else
		fprintf(stderr, "%s: test_%s() failed with status %d\n", progname, name, status);
}

#define test(name) \
	do { \
		void test_##name(void); \
		if (argc == 1) { \
			run(test_##name, #name); \
		} else if (argc == 2 && strcmp(argv[1], #name) == 0) { \
			test_##name(); \
			return 0; \
		} \
	} while (0)

int main(int argc, char **argv)
{
	progname = argv[0];

#include "test/suite.h"

	if (argc > 1) {
		fprintf(stderr, "%s: no such test\n", progname);
		return 1;
	}

	return passed == tests ? 0 : 1;
}
