
#include "math_fms.h"


void calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2, float *r, float *az1, float *az2)
{
    b1 = b1 * M_PI / 180;
    l1 = l1 * M_PI / 180;
    b2 = b2 * M_PI / 180;
    l2 = l2 * M_PI / 180;

    float sinb1 = sin(b1);
    float sinb2 = sin(b2);
    float cosb1 = cos(b1);
    float cosb2 = cos(b2);
    float sindb2 = sin((b1 - b2)/2);
    sindb2 *= sindb2;
    float dl = l2-l1;
    float sindl = sin(dl);
    float cosdl = cos(dl);
    float sindl2 = sin((dl)/2);
    sindl2 *= sindl2;
    float a = sindb2 + cosb1*cosb2*sindl2;
    if(r)
        *r = 2*atan2(sqrt(a), sqrt(1 - a)); //! расстояние в радианах

    if(az1)
    {
        *az1 = atan2(cosb2*sindl, cosb1*sinb2 - sinb1*cosb2*cosdl);
        if(*az1 < 0)
            *az1 += TWO_PI;
    }

    if(az2)
    {
        *az2 = atan2(-cosb1*sindl, cosb2*sinb1 - sinb2*cosb1*cosdl);
        if(*az2 < 0) *az2 += TWO_PI;
    }
}
