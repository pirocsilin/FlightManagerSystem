#include "managerdirectplank.h"

#if 0
#include <QDebug>
#include <QtMath>
#include <QObject>

#include "mapviewer.h"
#include "glissade_calculator.h"
#include "tManager.hpp"

#define TWO_PI (2.0*M_PI)  //!< Число 2*Пи

void ManagerDirectPlank::setNavDirectData(const NavDataForDirectPlank& data)
{
    if(std::isnan(data.pitch) || std::isnan(data.altitude) || std::isnan(data.latitude)
            || std::isnan(data.longitude) || std::isnan(data.course) || data.altitude < 0)
    {
        directPlankForGui.deviationCoursePlank   = NAN;
        directPlankForGui.deviationGlissadePlank = NAN;
        directPlankForGui.curPointDistance       = NAN;
        directPlankForGui.curPointNumber         = 0;
        directPlankForGui.countPointsInRoute     = directPlankRoute.points.size();
        return;
    }

    bool isOnRoute {false};     //! если true, то рисуем планки
    if (!directPlankRoute.points.empty())
    {
        isOnRoute = calcDirectPlank(qDegreesToRadians(data.latitude), qDegreesToRadians(data.longitude),
                                    data.altitude,
                                    data.speedVx, data.speedVz,
                                    data.pitch, false);
//        if(needUpdateRouteOnMap)
//        {
//            updateRouteInMapViewer();
//            needUpdateRouteOnMap = false;
//        }
    }

    if(isOnRoute)
    {
        //! если не первая точка, то точно не начало маршрута
        if(routeGuide.indexCurrentPoint != 0)
            isBeginOfRoute = false;

        if(routeGuide.indexCurrentPoint >= static_cast<uint32_t>(directPlankRoute.points.size()))
            routeGuide.indexCurrentPoint = 0;

        directPlankForGui.deviationCoursePlank   = routeGuide.coursePlank;
        directPlankForGui.deviationGlissadePlank = routeGuide.glissadePlank;
        directPlankForGui.curPointDistance       = routeGuide.distanceToCurPoint;
        directPlankForGui.curPointNumber         = routeGuide.indexCurrentPoint+1; // end
        directPlankForGui.countPointsInRoute     = directPlankRoute.points.size();
        if(routeGuide.indexCurrentPoint < static_cast<uint32_t>(directPlankRoute.points.size()))
            directPlankForGui.curPointAltitude = directPlankRoute.points[routeGuide.indexCurrentPoint].altitude;
        else
            directPlankForGui.curPointAltitude = NAN;

        directPlankForGui.insAltitude = data.altitude;
        float diffAzimuth = qRadiansToDegrees(routeGuide.azimuthToCurPoint) - data.course;
        directPlankForGui.curPointAzimuth = (diffAzimuth >= 0) ? diffAzimuth : 360.0f - std::abs(diffAzimuth);
#if 0
        qDebug() << __PRETTY_FUNCTION__ << data.course
                 << qRadiansToDegrees(directPlankRouteGuide.azimuthToNextPoint)
                 << diffAzimuth
                 << directPlank.azimuthToNextPPM;
#endif
    }else
    {
        directPlankForGui.deviationCoursePlank   = NAN;
        directPlankForGui.deviationGlissadePlank = NAN;
        directPlankForGui.curPointDistance       = NAN;
        directPlankForGui.curPointNumber         = 0;
        directPlankForGui.countPointsInRoute     = directPlankRoute.points.size();
        directPlankForGui.curPointAltitude       = NAN;
        directPlankForGui.curPointAzimuth        = NAN;
    }
}

void ManagerDirectPlank::setRoute(const flightroute::FlightRoute &route)
{
    isBeginOfRoute = true;
    if(route.isEmpty())
    {
        //! маршрут не загружен
        timerNavData.stop();
        return;
    }

    setupTimerNavData();

    //! сохраняем полученный маршрут
    originFlightRoute = route;

    //! сбрасываем настройки ведения по маршруту
    routeGuide.reset();

    //! заполняем маршрут для ведения по нему
    directPlankRoute.clear();

    for(auto & point : route.points)
    {
       DirectPlankRoute::PointParams newPoint;
       newPoint.latitude  = qDegreesToRadians(point.latitude);
       newPoint.longitude = qDegreesToRadians(point.longitude);
       newPoint.altitude  = point.altitude;
       newPoint.name      = QString(point.name.c_str());
       newPoint.LUR       = 0;

       directPlankRoute.points.push_back(newPoint);
    }

    //isLoopRoute = route.isLoop;

    //updateRouteInMapViewer();

    directPlankForGui.routeName = QString::fromStdString(originFlightRoute.name);
}

void ManagerDirectPlank::resetRoute()
{
    timerNavData.stop();

    //! сбрасываем настройки ведения по маршруту
    routeGuide.reset();

    //! заполняем маршрут для ведения по нему
    directPlankRoute.clear();

    directPlankForGui.deviationCoursePlank   = NAN;
    directPlankForGui.deviationGlissadePlank = NAN;
    directPlankForGui.routeName = "-";
}

DirectPlank ManagerDirectPlank::getDirectPlank()
{
    return directPlankForGui;
}

void ManagerDirectPlank::switchToNextPoint()
{
    routeGuide.selectNextPoint(true);
    updateRouteInMapViewer();
}

void ManagerDirectPlank::switchToPrevPoint()
{
    routeGuide.selectNextPoint(false);
    updateRouteInMapViewer();
}

void ManagerDirectPlank::setMapViewer(MapViewer *ptr)
{
    mapViewerRoute = ptr;
}

void ManagerDirectPlank::setBusManager(bus::Manager *manager)
{
    busManager = manager;
}

void ManagerDirectPlank::updateRouteInMapViewer()
{
    flightroute::FlightRoute ret;

    ret.name         = originFlightRoute.name;
    ret.creationDate = originFlightRoute.creationDate;

    //! добавление видимых непройденных линий
    addShownLines(ret, routeGuide.indexCurrentPoint);

    //! настройка вида отображаемых точек
    addShownPoints(ret, routeGuide.indexCurrentPoint);

    mapViewerRoute->setFlightRoute(std::move(ret));
}

bool ManagerDirectPlank::calcDirectPlank(float latitudeCurPosition, float longitudeCurPosition,
                                         float altitudeCurPosition,
                                         float Vx, float Vz, float pitch, bool isHelicopter)
{
    if(routeGuide.indexCurrentPoint != 0)
        isBeginOfRoute = false;

    if(isLoopRoute)
    {
        if(routeGuide.indexCurrentPoint >= static_cast<uint32_t>(directPlankRoute.points.size()))
            routeGuide.indexCurrentPoint = 0;
    }else
    {
        if(routeGuide.indexCurrentPoint >= static_cast<uint32_t>(directPlankRoute.points.size()))
            return false;

        //TODO
#if 0
    // при завершении маршрута прекращает ведение
    if (guide.currPPM >= static_cast<quint32>(route.points.size()))
    {

        for (int i=0; i < route.points.size(); ++i)
        {
            ROUTE_POINT &pt0 = route.points[i];
            pt0.cumDistTo = 0;
            pt0.cumTimeTo = 0;
            pt0.distTo = 0;
            pt0.timeTo = 0;
        }

        return false;
    }
#endif

    }

    float psi = atan2(Vz, Vx);   //! фактический путевой угол
    if(psi < 0)
        psi += TWO_PI;

    float psi_route_abeam {0.0f};

    float Kh      {0.0f};
    float dS      {0.0f};
    float psi_des {0.0f};
    float dZ      {0.0f};
    float ddZ     {0.0f};
    float curSpeed = Vx*Vx + Vz*Vz;
    float Rr = curSpeed / (9.81 * tan(qDegreesToRadians(25*0.9)));
    curSpeed = sqrt(curSpeed);
    float az {0.0f};                //! азимут текущей ЛЗП
    float azimuthToNextPPM {0.0f};  //! азимут от ВС на след ППМ
    float distanceToNextPPM {0.0f}; //! дальность от ВС на след ППМ
    float az_lzp {0.0f};

    //! расчет средней скорости
    float meanSpeed = getMeanSpeed(curSpeed);

    DirectPlankRoute::PointParams &curPoint = directPlankRoute.points[routeGuide.indexCurrentPoint]; //! текущий ППМ

    if(isBeginOfRoute)
    {
        if(routeGuide.indexCurrentPoint == 0)
        {
            /// первую точку летим в режиме "Прямо На"
            calcDistAndTrackBetweenWaypoints(latitudeCurPosition, longitudeCurPosition,
                                             curPoint.latitude, curPoint.longitude,
                                             &distanceToNextPPM, &azimuthToNextPPM);
            distanceToNextPPM *= EARTH_RADIUS;
            dS = distanceToNextPPM;
            psi_des = azimuthToNextPPM;
            az = azimuthToNextPPM;
            routeGuide.isNav = true;
            routeGuide.isDirectTo = true;
            Kh = routeGuide.Kh_nav;
        }else
        {
            isBeginOfRoute = false;
        }
    }else
    {
        DirectPlankRoute::PointParams &prevPoint = (routeGuide.indexCurrentPoint == 0)
                ? directPlankRoute.points[directPlankRoute.points.size()-1]     //!< если loop route
                : directPlankRoute.points[routeGuide.indexCurrentPoint-1];

        float S, az0, S0, Bt, Lt;
        //! п. 1.1
        /** az_lzp - азимут из предыдущей точки на следующую
         * az - азимут со следующей точки на предыдущую
         * */
        calcDistAndTrackBetweenWaypoints(prevPoint.latitude, prevPoint.longitude,
                                         curPoint.latitude, curPoint.longitude,
                                         &S, &az_lzp, &az);
        //! п. 1.2
        //! расчет азимута az0 из предыдущей точки на текущее положение
        calcDistAndTrackBetweenWaypoints(prevPoint.latitude, prevPoint.longitude,
                                         latitudeCurPosition, longitudeCurPosition,
                                         &S0, &az0);

        //! п. 1.3
        float az2pt;
        /** az2pt - азимут из текущего положения на след точку
         * az1 - азимут со следующей точки на текущее положение
         */
        calcDistAndTrackBetweenWaypoints(latitudeCurPosition, longitudeCurPosition,
                                         curPoint.latitude, curPoint.longitude,
                                         &distanceToNextPPM, &az2pt, &azimuthToNextPPM);
        distanceToNextPPM *= EARTH_RADIUS;

        //! п. 1.4 (формулы в скобках)
        //! азимут между азимутом со след ПМ на предыдущий и азимутом со след ПМ на текущее положение
        float dAz = bound_pi(az - azimuthToNextPPM, M_PI);
        dZ = distanceToNextPPM * sin(dAz);
        dS = distanceToNextPPM * cos(dAz);

        //! п. 1.5
        calcWaypointFromDistAndTrack(curPoint.latitude, curPoint.longitude, dS/EARTH_RADIUS,
                                     az, &Bt, &Lt);
        calcDistAndTrackBetweenWaypoints(Bt, Lt, curPoint.latitude, curPoint.longitude,
                                         nullptr, &psi_route_abeam);//&psi_route_abeam);

        //! п.1.7
        ddZ = -Vx*sin(psi_route_abeam) + Vz*cos(psi_route_abeam);

        //! п. 1.6
        float dddd = dZ/Rr + ddZ/120.0f;
        if(dddd > 1)
        {
            dddd = 1;
            routeGuide.isNav = true;
            Kh = routeGuide.Kh_nav;
        }else if(dddd < -1)
        {
            dddd = -1;
            routeGuide.isNav = true;
            Kh = routeGuide.Kh_nav;
        }else
        {
            routeGuide.isNav = false;
            Kh = routeGuide.Kh_stb;
        }
        psi_des = bound_2pi(psi_route_abeam - M_PI/2*dddd, TWO_PI);
        routeGuide.isDirectTo = false;
    }

#if 0
    //! зануляем предыдущие точки ???
    for (uint32_t i = 0; i < routeGuide.curPointNumber; ++i)
    {
        DirectPlankRoute::PointParams &pt0 = directPlankRoute.points[i];
        pt0.distanceFromCurPosition = 0;
        pt0.timeSecFromCurPosition = 0;
        pt0.distanceFromPrevPoint = 0;
        pt0.timeSecFromPrevPoint = 0;
    }
#endif

    //! расстояние до текущей точки
    curPoint.distanceFromPrevPoint = 0; // NOTE пока не используется
    curPoint.distanceFromCurPosition = distanceToNextPPM;
    //! время до текущей точки
    curPoint.timeSecFromCurPosition = static_cast<uint32_t>(curPoint.distanceFromCurPosition/meanSpeed);
    curPoint.timeSecFromPrevPoint = 0;  // NOTE пока не используется

#if 0
    //! расчет расстояния и времени для следующих точек
    for (quint32 i = isCurPointIsLastInRoute() ? 0 : routeGuide.curPointNumber+1;
         i < static_cast<quint32>(directPlankRoute.points.size()); ++i)
    {
        DirectPlankRoute::PointParams &pt0 = directPlankRoute.points[(i == 0)
                ? directPlankRoute.points.size()-1 : i-1];
        DirectPlankRoute::PointParams &pt1 = directPlankRoute.points[i];

        calcDistAndTrackBetweenWaypoints(pt0.latitude, pt0.longitude,
                                         pt1.latitude, pt1.longitude,
                                         &pt1.distanceFromPrevPoint);
        pt1.distanceFromPrevPoint *= EARTH_RADIUS;
        pt1.timeSecFromPrevPoint = static_cast<quint32>(pt1.distanceFromPrevPoint / meanSpeed);

        pt1.distanceFromCurPosition = pt0.distanceFromCurPosition + pt0.distanceFromPrevPoint;
        pt1.timeSecFromCurPosition  = pt0.timeSecFromCurPosition + pt0.timeSecFromPrevPoint;
    }
#endif

    //! проверка переключения
    if(curPoint.LUR)
    {
        //! двигаемся с ЛУР
        float az2;
        DirectPlankRoute::PointParams &nextPoint = isCurPointIsLastInRoute()
                ? directPlankRoute.points[0]
                : directPlankRoute.points[routeGuide.indexCurrentPoint+1];
        calcDistAndTrackBetweenWaypoints(curPoint.latitude, curPoint.longitude,
                                         nextPoint.latitude, nextPoint.longitude,
                                         nullptr, &az2);

        float dLur = Rr*fabs(tan((az2 - az_lzp)/2));
        if(distanceToNextPPM < dLur)
        {
            ++routeGuide.indexCurrentPoint;
            if(isLoopRoute && routeGuide.indexCurrentPoint >= directPlankRoute.points.size())
                routeGuide.indexCurrentPoint = 0;

            needUpdateRouteOnMap = true;
        }
    }else
    {
        //! прямо через точку
        if(fabs(dS) < routeGuide.switchPointDistance)
        {
            ++routeGuide.indexCurrentPoint;
            if(isLoopRoute && routeGuide.indexCurrentPoint >= directPlankRoute.points.size())
                routeGuide.indexCurrentPoint = 0;

            needUpdateRouteOnMap = true;
        }
    }
    if(routeGuide.indexCurrentPoint >= static_cast<uint32_t>(directPlankRoute.points.size()))
        return false;

    //! выходные данные
    routeGuide.trackDeviationZ             = dZ;  //! боковое отклонение в метрах
    routeGuide.distanceTrackLineToCurPoint = dS;
    routeGuide.distanceToCurPoint          = distanceToNextPPM;
    routeGuide.desiredTrackAngleToCurPoint = psi_des;   //delta_psi_des;
    routeGuide.desiredAltitudeToCurPoint   = curPoint.altitude;
    routeGuide.azimuthToCurPoint           = azimuthToNextPPM;
    routeGuide.remainTimeToCurPoint = qFuzzyCompare(curSpeed, 0.0f) ? 0 : (routeGuide.distanceToCurPoint/curSpeed);

    if(isHelicopter)
    {
        //! для вертолета
        routeGuide.coursePlank = qRadiansToDegrees(bound_pi(psi_des - psi, M_PI));  //! разница между азимутом на точку и фактическим путевом углом
//        routeGuide.glissadeIndex = currentPoint.h - altitude;    //! угол отклонения
    }else
    {
        //! для самолета
        routeGuide.coursePlank = qRadiansToDegrees(routeGuide.Kpsi*bound_pi(psi_des - psi, M_PI)
                                                   + routeGuide.Kz*ddZ);
//        routeGuide.glissadeIndex = Kh * (currentPoint.h - altitude) + routeGuide.Kv * Vy;
    }

    routeGuide.glissadePlank = getGlissade(latitudeCurPosition, longitudeCurPosition, altitudeCurPosition,
                                           directPlankRoute.points[routeGuide.indexCurrentPoint].latitude,
            directPlankRoute.points[routeGuide.indexCurrentPoint].longitude,
            directPlankRoute.points[routeGuide.indexCurrentPoint].altitude, pitch);
        return true;
}

/** Вычисляет координаты точки ортодромии, заданной расстоянием и начальным азимтутом от заданной точки
 * \param B0 широта заданной точки, радианы
 * \param L0 долгота заданной точки, радианы
 * \param rng расстояние до искомой точки, радианы дуги
 * \param az начальный азимут ортодромии
 * \param[out] B широта искомой точки, радианы
 * \param[out] L долгота искомой точки, радианы
 */
void ManagerDirectPlank::calcWaypointFromDistAndTrack(float B0, float L0, float rng, float az,
                                                      float *B, float *L)
{
    if(B0 > M_PI_2 - 1e-8)
        az = M_PI;
    else if(B0 < 1e-8 - M_PI_2)
        az = 0;

    float sin_phi0 = sin(B0);
    float cos_phi0 = cos(B0);
    float cos_az   = cos(az);
    float sin_rng  = sin(rng);
    float cos_rng  = cos(rng);
    if(B)
        *B = asin(sin_phi0*cos_rng + cos_phi0*sin_rng*cos_az);
    if(L)
        *L = L0 + atan2(sin_rng*sin(az), cos_phi0*cos_rng - sin_phi0*sin_rng*cos_az);
}

/** Расчет азимута, обратного азимута и дистанции между двумя геодезическими точками
 * \param[in] b1 широта первой точки, радианы
 * \param[in] l1 долгота первой точки, радианы
 * \param[in] b2 широта второй точки, радианы
 * \param[in] l2 долгота второй точки, радианы
 * \param r[out] дистанция между точками, радианы дуги
 * \param az1[out] азимут из первой точки на вторую, радианы
 * \param az2[out] азимут из второй точки на первую, радианы
 */
void ManagerDirectPlank::calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
                                                          float *r, float *az1, float *az2)
{
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

/** Переводит значение переменных в диапазон 0..bnd
 * Используется для приведения углов курса, азимута, путевого угла итд в диапазон значений 0..2*pi
 * \param val число для перевода
 * \param bnd диапазон
 * \return число в диапазоне 0..bnd
 */
float ManagerDirectPlank::bound_2pi(float val, float bnd)
{
    while (val > bnd)
        val -= bnd;

    while (val < 0)
        val += bnd;

    return val;
}

/** Переводит значение переменных в диапазон -bnd..bnd
 * Используется для приведения разниц углов в диапазон значений -pi..pi
 * \param val число для перевода
 * \param bnd диапазон
 * \return число в диапазоне -bnd..bnd
 */
float ManagerDirectPlank::bound_pi(float val, float bnd)
{
    float two_bnd = 2*bnd;
    while (val > bnd)
        val -= two_bnd;

    while (val < -bnd)
        val += two_bnd;

    return val;
}

void ManagerDirectPlank::addShownLines(flightroute::FlightRoute &route, uint32_t indexActivePoint)
{
    route.lines.clear();
    if(directPlankRoute.points.size() <= 1)
        return;

    if(isLoopRoute)
    {

    }else
    {
        int countPointsFromActive = directPlankRoute.points.size() - indexActivePoint;

        if(countPointsFromActive <= 0)
            return;

        //! дальше идем только если текущая точка в маршруте или маршрут loop
        int countLinesForGui {};

        //! добавляем текущyю линию если не первая точка маршрута
        if(indexActivePoint > 0)
            ++countLinesForGui;

        if(countPointsFromActive > limitCountShownPoint)
        {
            //! ограничиваем заданным лимитом с линией без точки на конце
            countLinesForGui += limitCountShownPoint;
        }else
        {
            //! убираем последнюю линию без точки на конце
            countLinesForGui += countPointsFromActive - 1;
        }

        int indexFirstPointForLines {};
        if(indexActivePoint > 0)
            indexFirstPointForLines = indexActivePoint - 1;

        for(int i = indexFirstPointForLines; i < (indexFirstPointForLines + countLinesForGui); ++i)
        {
            flightroute::RouteLine newLine;
            newLine.latitudeBegin  = qRadiansToDegrees(directPlankRoute.points.at(i).latitude);
            newLine.longitudeBegin = qRadiansToDegrees(directPlankRoute.points.at(i).longitude);
            newLine.latitudeEnd    = qRadiansToDegrees(directPlankRoute.points.at(i+1).latitude);
            newLine.longitudeEnd   = qRadiansToDegrees(directPlankRoute.points.at(i+1).longitude);
            newLine.type           = 0;
            newLine.isActive       = (i == 0 && indexActivePoint == 0) ? false : (indexFirstPointForLines == i);
            route.lines.push_back(newLine);
        }
    }
}

/**
 * @brief Добавление заданного количества отображаемых предстоящих точек на карту
 * @param route маршрут для добавления точек
 * @param curActivePoint номер текущей активной точки в массиве точек маршрута
 */
void ManagerDirectPlank::addShownPoints(flightroute::FlightRoute &route, uint32_t indexActivePoint)
{
    route.points.clear();

    int countShownPointsWithActive = directPlankRoute.points.size() - indexActivePoint;
    countShownPointsWithActive = std::min(countShownPointsWithActive, limitCountShownPoint);

    for(uint32_t i = indexActivePoint; i < (indexActivePoint + countShownPointsWithActive); ++i)
    {
       flightroute::RoutePoint newPoint;

       //TODO выбрать обозначение для активной точки
       if(i == indexActivePoint)
           newPoint.type = flightroute::TypeRoutePoint::TP_AERODROME_MILIT_LAND;
       else
           newPoint.type = flightroute::TypeRoutePoint::TP_USER_POINT;

       newPoint.latitude  = qRadiansToDegrees(directPlankRoute.points[i].latitude);
       newPoint.longitude = qRadiansToDegrees(directPlankRoute.points[i].longitude);
       newPoint.altitude  = directPlankRoute.points[i].altitude;
       newPoint.name      = directPlankRoute.points[i].name.toStdString();

       route.points.push_back(newPoint);
    }
}

void ManagerDirectPlank::setupTimerNavData()
{
    QObject::connect(&timerNavData, &QTimer::timeout,
                     [this](){
        setNavDirectData(busManager->dataNavForDirectPlank());
    });

    timerNavData.start(50);
}

bool ManagerDirectPlank::isCurPointIsLastInRoute()
{
    return routeGuide.indexCurrentPoint == directPlankRoute.points.size()-1;
}

float ManagerDirectPlank::getMeanSpeed(float speed)
{
    speed = (speed < routeGuide.Vnom) ? routeGuide.Vnom : speed;
    routeGuide.Vbuf.enqueue(speed);
    if(routeGuide.Vbuf.size() > static_cast<int>(routeGuide.Vbufsize))
        routeGuide.Vbuf.dequeue();

    float ret {};
    for(float v : routeGuide.Vbuf)
        ret += v;

    return ret /= routeGuide.Vbuf.size();
}
#endif
