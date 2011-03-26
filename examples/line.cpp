#include "nupy/nupy.hpp"

template<int D>
struct Line
{
    NUPY_BEGIN(Line<D>)

    double NUPY_MEMBER(start) [D];
    double NUPY_MEMBER(end  ) [D];
    char   NUPY_MEMBER(note ) [16];

    NUPY_END()
};

int main()
{
    char buf[128];
    int sz2 = Line<2>::nupy_dtype(buf, sizeof(buf));
    int sz3 = Line<3>::nupy_dtype(buf, sizeof(buf));
    printf("%d %s\n", sz2, buf);
    printf("%d %s\n", sz3, buf);
}
