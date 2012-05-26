#include "tests.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    test_base_derived();
    test_fam();
    test_fam_c();
    test_enum();

    return exit_status;
}
