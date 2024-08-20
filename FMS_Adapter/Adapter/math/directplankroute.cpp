#include "directplankroute.h"
#include <QtCore>

#include <math.h>

DirectPlankRouteGuide::DirectPlankRouteGuide()
{
    reset();
}

void DirectPlankRouteGuide::reset()
{
    indexCurrentPoint = 0;

    trackDeviationZ      = 0;
    distanceTrackLineToCurPoint      = 0;
    azimuthToCurPoint     = 0;
    desiredTrackAngleToCurPoint = 0;
    desiredAltitudeToCurPoint   = 0;
    distanceToCurPoint     = 0;
    remainTimeToCurPoint    = 0;
    coursePlank     = 0;
    glissadePlank   = 0;
    isNav           = 0;
    isDirectTo      = 0;

    Kpsi = 0.3f;      //!< коэффициент для горизонтального индекса (было 0.3 для ограничения до -54..+54 градуса)
    Kz   = -0.2f;     //!< коэффициент для горизонтального индекса.

    Kh_nav = 0.1f;      //!< коэффициент вертикального индекса в режиме наведения
    Kh_stb = 0.1f;      //!< коэффициент вертикального индекса в режиме стабилизации
    Kv     = -8e-4f;    //!< коэффициент для вертикального индекса

    Vbufsize = 100;      //Размер буфера для осреднения скорости для оценки времени прибытия
    Vbuf.clear();

    switchPointDistance    = 100;      //дальность для переключения на след ПМ, м
    Vnom    = 300/3.6;  //Номинальная скорость для расчета времени прибытия. 100 km/h пока так, потом решить
}
