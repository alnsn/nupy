#include "nupy/nupy.hpp"

#include <inttypes.h>

#ifdef __cplusplus
#define FAM_DIM(n) n
#else
#define FAM_DIM(n)
#endif

struct Fam1
{
    nupyStruct(Fam1)

    double nupyM( numbers ) [3][2][1];
    uint64_t nupyFAM( fam ) [FAM_DIM(1)][2][3];

    nupyEnd()
};

struct Fam2
{
    nupyStruct(Fam2)

    char nupyM( not_fam );
    char nupyFAM( fam ) [FAM_DIM(7)];

    nupyEnd()
};

struct FamElem
{
    nupyStruct(FamElem)

    int32_t nupyM( member );

    nupyEnd()
};

struct Fam3
{
    nupyStruct(Fam3)

    uint8_t        nupyM( bytes ) [8];
    struct FamElem nupyFAM( fam ) [FAM_DIM(1)];

    nupyEnd()
};
