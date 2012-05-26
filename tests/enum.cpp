#include "nupy/nupy.hpp"

#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
#include "tests.h"
#include "util.h"
}

enum IE { IEVal = INT_MAX  };
enum UE { UEVal = UINT_MAX };

struct WithEnums
{
    nupyStruct(WithEnums)

    IE nupyM(ie);
    UE nupyM(ue);

    nupyEnd()
};

extern "C" void
test_enum(void)
{
    const char expected[] = "[('ie','<i4'),('ue','<u4')]";

    const size_t bufsz = sizeof(expected);
    char buf[bufsz];

    const int len = WithEnums::nupy_dtype(buf, bufsz);

    CHECK(len + 1u == bufsz);
    CHECK(strcmp(buf, expected) == 0);

    for(size_t sz = 0; sz <= bufsz; sz++) {
        char* newbuf = buf + (bufsz - sz);
        int newlen = WithEnums::nupy_dtype(newbuf, sz);

        CHECK(newlen == len);
    }
}
