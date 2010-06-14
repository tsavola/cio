#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef assert
# undef assert
#endif

#define assert(expr) \
	do { \
		if (!(expr)) { \
			if (errno) \
				fprintf(stderr, "%s:%d: %s: assert(%s) failed: %s\n", __FILE__, __LINE__, __func__, #expr, strerror(errno)); \
			else \
				fprintf(stderr, "%s:%d: %s: assert(%s) failed\n", __FILE__, __LINE__, __func__, #expr); \
			abort(); \
		} \
	} while (0)
