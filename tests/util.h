#ifndef FILE_nupy_tests_util_h_INCLUDED
#define FILE_nupy_tests_util_h_INCLUDED

#include <err.h>
#include <stdlib.h>

#define lengthof(s) ((ssize_t)(sizeof(s) - 1))

extern int exit_status;

#define REQUIRE(x) if (!(x)) { \
		errx(EXIT_FAILURE, "%s:%u: %s", __FILE__, __LINE__, #x); }

#define CHECK(x) if (!(x)) { \
		exit_status = EXIT_FAILURE; \
		warnx("%s:%u: %s", __FILE__, __LINE__, #x); }

#endif /* #ifndef FILE_nupy_tests_util_h_INCLUDED */
