
#include <cmath>

#define TWO_PI (2.0*M_PI)       //!< Число 2*Пи
#define EARTH_RADIUS 6371000.0  //!< Условный радиус земной сферы, м


void calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
                                      float *r=nullptr, float *az1=nullptr, float *az2=nullptr);
