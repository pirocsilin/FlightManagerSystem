
#include <cmath>
#include <vector>
#include <common/structsflightplan.h>

using namespace fp;

#define TWO_PI (2.0*M_PI)       //!< Число 2*Пи
#define EARTH_RADIUS 6371000.0  //!< Условный радиус земной сферы, м

struct SimplePoint {
    double id, lat, lon;
};

void calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
                                      float *r=nullptr, float *az1=nullptr, float *az2=nullptr);

void sortWaypointByDistance(float curLat, float curLon, std::vector<SimplePoint> &vector);
void sortWaypointByDistance(float curLat, float curLon, std::vector<Waypoint> &vector);
