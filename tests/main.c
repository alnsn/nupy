#include "tests.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    test_base_derived();
    test_fam();
    test_fam_c();

    return exit_status;
}
