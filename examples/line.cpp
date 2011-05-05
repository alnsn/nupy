#include "nupy/nupy.hpp"

template<int D>
struct Line
{
    nupyStruct(Line<D>)

    double nupyM(start) [D];
    double nupyM(end  ) [D];
    char   nupyM(note ) [16];

    nupyEnd()
};

int main()
{
    char buf[128];
    int sz2 = Line<2>::nupy_dtype(buf, sizeof(buf));
    int sz3 = Line<3>::nupy_dtype(buf, sizeof(buf));
    printf("%d %s\n", sz2, buf);
    printf("%d %s\n", sz3, buf);
}
