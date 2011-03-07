#include "nupy/nupy.hpp"

struct Line
{
    NUPY_BEGIN(Line)

    double NUPY_MEMBER(start) [2];
    double NUPY_MEMBER(end  ) [2];
    char   NUPY_MEMBER(note ) [16];

    NUPY_END()
};

int main()
{
    char buf[128];
    int sz = Line::nupy_dtype(buf, sizeof(buf));
    printf("%d %s\n", sz, buf);
}
