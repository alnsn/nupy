#include "fam.h"

#include "tests.h"
#include "util.h"

void
test_fam_c(void)
{
    CHECK(sizeof(struct Fam1) > 0);
    CHECK(sizeof(struct Fam2) > 0);
}
