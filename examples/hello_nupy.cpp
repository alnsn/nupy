#include "nupy/nupy.hpp"

struct Greeting
{
    NUPY_BEGIN(Greeting)

    char NUPY_MEMBER( greeting  ) [8];
    char NUPY_MEMBER( recipient ) [16];

    NUPY_END()
};

int main()
{
    char buf[128];
    int sz = Greeting::nupy_dtype(buf, sizeof(buf));
    printf("%d %s\n", sz, buf);
}
