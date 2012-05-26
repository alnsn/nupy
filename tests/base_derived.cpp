#include "nupy/nupy.hpp"

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
#include "tests.h"
#include "util.h"
}

struct Base
{
    Base()
        : c(4)
    {}

    nupyStruct(Base)

    char          nupyM(a);
    volatile char nupyM(b) [7];

    const int32_t nupyM(c);
    uint32_t      nupyM(d);

    nupyEnd()
};

struct Derived : Base
{
    nupyStruct(Derived)
    nupyBase(Base)

    float nupyM(x) [2][3][4];
    char  nupyM(y) [8][2];
    Base  nupyM(z) [5];

    nupyEnd()
};

extern "C" void
test_base_derived(void)
{
    const char expected[] = "[('a','|S1'),('b','|S7'),('c','<i4'),('d','<u4'),('x','<f4',(2,3,4)),('y','|S2',(8)),('z',[('a','|S1'),('b','|S7'),('c','<i4'),('d','<u4')],(5))]";

    const size_t bufsz = sizeof(expected);
    char buf[bufsz];

    const int len = Derived::nupy_dtype(buf, bufsz);

    CHECK(len + 1u == bufsz);
    CHECK(strcmp(buf, expected) == 0);

    for(size_t sz = 0; sz <= bufsz; sz++) {
        char* newbuf = buf + (bufsz - sz);
        int newlen = Derived::nupy_dtype(newbuf, sz);

        CHECK(newlen == len);
    }
}
