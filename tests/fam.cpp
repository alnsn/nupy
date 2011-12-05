#include "fam.h"

#include <string.h>

extern "C" {
#include "tests.h"
#include "util.h"
}

extern "C" void
test_fam()
{
    const char expected1[] = "[('numbers','<f8',(3,2,1)),('fam','<u8',(5,2,3))]";

    const size_t buf1sz = sizeof(expected1);
    char buf1[buf1sz];

    int len1 = Fam1::nupy_dtype(buf1, buf1sz, 5);

    CHECK(len1 + 1u == buf1sz);
    CHECK(strcmp(buf1, expected1) == 0);

    const char expected2[] = "[('not_fam','|S1'),('fam','|S2')]";

    const size_t buf2sz = sizeof(expected2);
    char buf2[buf2sz];

    int len2 = Fam2::nupy_dtype(buf2, buf2sz, 2);

    CHECK(len2 + 1u == buf2sz);
    CHECK(strcmp(buf2, expected2) == 0);

    const char expected3[] = "[('bytes','<u1',(8)),('fam',[('member','<i4')],(12))]";

    const size_t buf3sz = sizeof(expected3);
    char buf3[buf3sz];

    int len3 = Fam3::nupy_dtype(buf3, buf3sz, 12);

    CHECK(len3 + 1u == buf3sz);
    CHECK(strcmp(buf3, expected3) == 0);
}
