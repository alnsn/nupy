#include "nupy/nupy.hpp"

#include <string.h>
#include <stdlib.h>
#include <err.h>

#include <boost/shared_array.hpp>

struct Base
{
    Base()
        : c(4)
    {}

    nupyClass(Base)

    char          nupyM(a);
    volatile char nupyM(b) [7];

    const int32_t nupyM(c);
    uint32_t      nupyM(d);

    nupyEnd
};

struct Derived : Base
{
    nupyClass(Derived)
    nupyBase(Base)

    float nupyM(x) [2][3][4];
    char  nupyM(y) [8][2];
    Base  nupyM(z) [5];

    nupyEnd
};

int main()
{
    /* XXX use mmap/mprotect */
    size_t bufsz = 4096;
    boost::shared_array<char> buf(new char[bufsz]);

    const int len = Derived::nupy_dtype(buf.get(), bufsz);

    if(len + 0u >= bufsz)
        errx(EXIT_FAILURE, "len >= bufsz: %d %zu\n", len, bufsz);

    printf("sizeof(Derived): %zu\n", sizeof(Derived));
    printf("\"%s\" (%d bytes)\n", buf.get(), len);

    if(len < 2)
        errx(EXIT_FAILURE, "len < 2: %d\n", len);

    if(len + 0u != strlen(buf.get()))
        errx(EXIT_FAILURE, "len != strlen(buf.get()): %d %zu\n", len, strlen(buf.get()));

    if(buf[0] != '[')
        errx(EXIT_FAILURE, "first char != '[': '%c'\n", buf[0]);

    if(buf[len - 1] != ']')
        errx(EXIT_FAILURE, "last char != ']': '%c'\n", buf[len - 1]);

    for(size_t sz = 0; sz <= bufsz; sz++) {
        char* newbuf = buf.get() + (bufsz - sz);
        int newlen = Derived::nupy_dtype(newbuf, sz);
        if(newlen != len)
            errx(EXIT_FAILURE, "newlen != len: %d %d, sz=%zu\n", newlen, len, sz);
    }

    return EXIT_SUCCESS;
}
