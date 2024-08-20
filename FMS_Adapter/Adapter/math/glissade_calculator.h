#ifndef GLISSADE_CALCULATOR_H
#define GLISSADE_CALCULATOR_H

#include <cmath>

double getDistanceBetweenPoints(double lat1, double lon1,
                                                      double lat2, double lon2)
{
    double R = 6371; // Radius of the earth in km
    double dLat = lat2-lat1;  // deg2rad below
    double dLon = lon2-lon1;  // deg2rad below
    double a =  std::sin(dLat/2) * std::sin(dLat/2)
            + std::cos(lat1) * std::cos(lat2)
            * std::sin(dLon/2) * std::sin(dLon/2) ;
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    double d = R * c; // Distance in km
    return d;
}

double getGlissade(double lat1, double lon1, double alt1, double lat2, double lon2,
                   double alt2, double pitch)
{

    pitch = pitch * M_PI / 180.0;
    lat1 = lat1 * M_PI / 180.0;
    lon1 = lon1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    lon2 = lon2 * M_PI / 180.0;

    double r = getDistanceBetweenPoints(lat1, lon1, lat2, lon2) * 1000;

    double x1 = 0;
    double x2 = r;
    double y1 = alt1;
    double y2 = alt2;

    // вычислить угол между N2 и осью X
    double beta = atan2(y2 - y1, x2 - x1);

    // вычислить угол между N1 и N2
    double angle = pitch - beta;

    // преобразуем угол обратно в градусы
    angle = angle * 180.0 / M_PI;

    return -angle;
}

#endif // GLISSADE_CALCULATOR_H
