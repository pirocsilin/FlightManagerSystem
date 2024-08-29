
#include <cmath>
#include <qmath.h>
#include "mathplan.h"

#define TWO_PI (2.0*M_PI)  //!< Число 2*Пи


ManagerCalculationPlan::ManagerCalculationPlan()
{
    resetRoute();
}
#include <QPair>
void ManagerCalculationPlan::setRoute(FlightPlan &plan, uint32_t activePoint)
{
    // сбрасываем настройки ведения по маршруту
    resetRoute();

    // запоминаем активный план
    activePlan = plan;

    // заполняем структуру ActivePlanInfo
    activePlanInfo.id   = plan.id;
    activePlanInfo.name = plan.name;
    activePlanInfo.activeWaypoint = plan.waypoints[activePoint];

    std::vector<Waypoint> &vec = plan.waypoints;
    for(int i{}; i<vec.size(); i++)
    {
        WaypointRouteInfo pointInfo{};

        pointInfo.id        = vec[i].id;
        pointInfo.icao      = vec[i].icao;
        pointInfo.altitude  = vec[i].altitude;
        pointInfo.isActive  = i==activePoint ? true : false;

        if(i >= activePoint + 1)
        {
            float distance;
            float bearing;
            calcDistAndTrackBetweenWaypoints(qDegreesToRadians(vec[i-1].latitude),
                                             qDegreesToRadians(vec[i-1].longitude),
                                             qDegreesToRadians(vec[i].latitude),
                                             qDegreesToRadians(vec[i].longitude),
                                             &distance, &bearing);

            pointInfo.distance = distance * EARTH_RADIUS;
            pointInfo.bearing  = bearing * 180 / M_PI;
        }
        activePlanInfo.waypoints.push_back(pointInfo);
    }

    // заполняем структуру ActivePlanGuide
    fpg.name = QString(plan.name.c_str());
    for(auto &point : plan.waypoints)
    {
        ActivePlanGuide::PointParams newPoint{};
        newPoint.latitude  = qDegreesToRadians(point.latitude);
        newPoint.longitude = qDegreesToRadians(point.longitude);
        newPoint.altitude  = point.altitude;
        newPoint.name      = QString(point.icao.c_str());

        fpg.points.push_back(newPoint);
    }
}

void ManagerCalculationPlan::resetRoute()
{
    //! сбрасываем настройки ведения по маршруту
    activePlanInfo     = {};
    navDataFms         = {};
    activePlan         = {};
    fpg                = {};
}

void ManagerCalculationPlan::setNavData(const fp::DeviceFlightData &data)
{
    navDataFms = {};

    if(std::isnan(data.longitude) || std::isnan(data.latitude) ||
            std::isnan(data.speedVx) || std::isnan(data.speedVz))
    {
        return;
    }

    bool isOnRoute {false};
    if(!fpg.points.empty())
    {
        isOnRoute = calcActivePlan(qDegreesToRadians(data.latitude),
                                   qDegreesToRadians(data.longitude),
                                   data.speedVx,
                                   data.speedVz);
    }

    if(isOnRoute)
    {
        // заполняем структуру ActivePlanInfo:
        //
        activePlanInfo.remainFlightDistance = fpg.distanceToNextPPM;
        for(uint32_t i{fpg.indexCurrentPoint+1}; i<activePlanInfo.waypoints.size(); i++)
            activePlanInfo.remainFlightDistance += activePlanInfo.waypoints[i].distance;

        activePlanInfo.remainFlightTime = activePlanInfo.remainFlightDistance / fpg.meanSpeed;
        activePlanInfo.activeWaypoint   = activePlan.waypoints[fpg.indexCurrentPoint];

        // заполняем структуру NavDataFms:
        //
        int indexNextPoint = fpg.indexCurrentPoint < fpg.points.size() - 1 ?
                             fpg.indexCurrentPoint + 1 : fpg.indexCurrentPoint;

        navDataFms.activeWaypointIcao       = fpg.points[fpg.indexCurrentPoint].name.toStdString();
        navDataFms.nextWaypointIcao         = fpg.points[indexNextPoint].name.toStdString();
        navDataFms.activeWaypointDistance   = fpg.distanceToNextPPM * 10;
        navDataFms.activeWaypointRemainTime = fpg.remainTimeToCurPoint;
        navDataFms.nextWaypointRemainTime   = fpg.remainTimeToNextPoint;
        navDataFms.activeWaypointCourse     = fpg.azimuthToNextPPM * 180 / M_PI;
        navDataFms.lateralDeviationPathLine = fpg.trackDeviationZ;
        //
        // navDataFms.activeRadioBeaconDistance
        //
        navDataFms.activePlan               = activePlanInfo;

        // проверка переключения на следующую точку маршрута
        //
        qDebug() << fpg.distanceToNextPPM << ":" << fpg.switchPointDistance;
        if(fpg.distanceToNextPPM <= fpg.switchPointDistance)
            selectNextPoint(true);
    }
}

void ManagerCalculationPlan::selectNextPoint(bool direction)
{
    uint32_t curIndex = fpg.indexCurrentPoint;

    if(fpg.selectNextPoint(direction))
    {
        activePlanInfo.waypoints[curIndex].isActive = false;
        activePlanInfo.waypoints[fpg.indexCurrentPoint].isActive = true;
        activePlanInfo.activeWaypoint = activePlan.waypoints[fpg.indexCurrentPoint];
    }
}

FlightPlan &ManagerCalculationPlan::getEditActivePlan()
{
    editActivePlan = activePlan;
    return editActivePlan;
}

void ManagerCalculationPlan::setEditActivePlan()
{
    activePlan = editActivePlan;
}

uint32_t ManagerCalculationPlan::getIndexActivePoint()
{
    return fpg.indexCurrentPoint;
}

void ManagerCalculationPlan::addWaypointToActivePlan(uint32_t position, Waypoint &point)
{
    if(position < 0 || position >= editActivePlan.waypoints.size())
        return;

    if(position == editActivePlan.waypoints.size()-1)
        editActivePlan.waypoints.push_back(point);
    else
    {
        auto it = editActivePlan.waypoints.begin();
        editActivePlan.waypoints.insert(it + position, point);
    }
    createNameForPlan(editActivePlan);

    uint32_t indexCurrentPoint = fpg.indexCurrentPoint;
    uint32_t indexPsitionAfterInsert = position <= indexCurrentPoint ?
                                       indexCurrentPoint + 1 :
                                       indexCurrentPoint;
    setRoute(editActivePlan);

    for(uint32_t i{}; i < indexPsitionAfterInsert; i++)
        selectNextPoint(true);
}

bool ManagerCalculationPlan::calcActivePlan(float latitudeCurPosition, float longitudeCurPosition, float Vx, float Vz)
{
    if(fpg.indexCurrentPoint >= static_cast<uint32_t>(fpg.points.size()))
        return false;

    fpg.clearData();
    //
    fpg.curSpeed  = sqrt(Vx*Vx + Vz*Vz);        //!< текущая скорость ВС, м/с
    fpg.meanSpeed = getMeanSpeed(fpg.curSpeed); //!< средняя скорость ВС, м/с


    ActivePlanGuide::PointParams &curPoint = fpg.points[fpg.indexCurrentPoint];

    if(fpg.indexCurrentPoint == 0)
    {
        // первую точку летим в режиме "Прямо На"

        calcDistAndTrackBetweenWaypoints(latitudeCurPosition, longitudeCurPosition,
                                         curPoint.latitude, curPoint.longitude,
                                         &fpg.distanceToNextPPM, &fpg.azimuthToNextPPM);

        fpg.distanceToNextPPM   *= EARTH_RADIUS;
    }
    else
    {
        // иначе, дополнительно считаем боковое отклонение от ЛЗП

        ActivePlanGuide::PointParams &prevPoint = fpg.points[fpg.indexCurrentPoint-1];

        float az_lzp_back  {}; // азимут со следующей точки на предыдущую
        float az_true_back {}; // азимут со следующей точки на текущее положение

        calcDistAndTrackBetweenWaypoints(prevPoint.latitude, prevPoint.longitude,
                                         curPoint.latitude, curPoint.longitude,
                                         nullptr, nullptr, &az_lzp_back);

        calcDistAndTrackBetweenWaypoints(latitudeCurPosition, longitudeCurPosition,
                                         curPoint.latitude, curPoint.longitude,
                                         &fpg.distanceToNextPPM, &fpg.azimuthToNextPPM, &az_true_back);

        // азимут между азимутом со след ПМ на предыдущий и азимутом со след ПМ на текущее положение
        float delta_az_back = bound_pi(az_lzp_back - az_true_back, M_PI);
        fpg.distanceToNextPPM   *= EARTH_RADIUS;
        fpg.trackDeviationZ = fpg.distanceToNextPPM * sin(delta_az_back);
    }

    qDebug() << __PRETTY_FUNCTION__ << latitudeCurPosition * 180 /M_PI << longitudeCurPosition * 180 /M_PI << curPoint.latitude * 180 /M_PI << curPoint.longitude * 180 /M_PI;

    bool nextPointExists  = fpg.indexCurrentPoint < fpg.points.size()-1;
    //
    fpg.distanceToNextNextPPM = nextPointExists ? activePlanInfo.waypoints[fpg.indexCurrentPoint+1].distance : 0.0f;
    fpg.remainTimeToCurPoint  = qFuzzyCompare(fpg.curSpeed, 0.0f) ? 0 : (fpg.distanceToNextPPM / fpg.curSpeed);
    fpg.remainTimeToNextPoint = qFuzzyCompare(fpg.curSpeed, 0.0f) ? 0 : ((fpg.distanceToNextPPM + fpg.distanceToNextNextPPM) / fpg.curSpeed);

    return true;
}

#if 0
bool ManagerCalculationPlan::calcActivePlan(float latitudeCurPosition, float longitudeCurPosition,
                                             float Vx, float Vz)
{
    if(routeGuide.indexCurrentPoint >= static_cast<uint32_t>(activePlanGuide.points.size()))
        return false;

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
    float distanceToNextNextPPM {0.0f}; //! дальность от ВС на след после активной ППМ
    float az_lzp {0.0f};

    //! расчет средней скорости
    float meanSpeed = getMeanSpeed(curSpeed);

    ActivePlanGuide::PointParams &curPoint = activePlanGuide.points[routeGuide.indexCurrentPoint];

    if(routeGuide.indexCurrentPoint == 0)
    {
        /// первую точку летим в режиме "Прямо На"
        calcDistAndTrackBetweenWaypoints(latitudeCurPosition, longitudeCurPosition, // i'm here
                                         curPoint.latitude, curPoint.longitude,
                                         &distanceToNextPPM, &azimuthToNextPPM);
        distanceToNextPPM *= EARTH_RADIUS;
        dS = distanceToNextPPM;
        psi_des = azimuthToNextPPM;
        az = azimuthToNextPPM;
        routeGuide.isNav = true;
        routeGuide.isDirectTo = true;
        Kh = routeGuide.Kh_nav;
    }
    else
    {
        ActivePlanGuide::PointParams &prevPoint = activePlanGuide.points[routeGuide.indexCurrentPoint-1];

        float S, az0, S0, Bt, Lt;
        //! п. 1.1
        /** az_lzp - азимут из предыдущей точки на следующую
         *
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

        // Если есть следующая точка от активной, вычисляем расстояние от активной до нее
        if(routeGuide.indexCurrentPoint < activePlanGuide.points.size()-1)
        {
            calcDistAndTrackBetweenWaypoints(curPoint.latitude,
                                             curPoint.longitude,
                                             activePlanGuide.points[routeGuide.indexCurrentPoint+1].latitude,
                                             activePlanGuide.points[routeGuide.indexCurrentPoint+1].longitude,

                                             &distanceToNextNextPPM, nullptr, nullptr);
            distanceToNextNextPPM *= EARTH_RADIUS;
        }

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

        //! расстояние до текущей точки
        curPoint.distanceFromPrevPoint = 0; // NOTE пока не используется
        curPoint.distanceFromCurPosition = distanceToNextPPM;
        //! время до текущей точки
        curPoint.timeSecFromCurPosition = static_cast<uint32_t>(curPoint.distanceFromCurPosition/meanSpeed);
        curPoint.timeSecFromPrevPoint = 0;  // NOTE пока не используется

        //! ПЕРЕКЛЮЧЕНИЕ! режим: прямо через точку
        if(fabs(dS) < routeGuide.switchPointDistance)
        {
            ++routeGuide.indexCurrentPoint;
        }

        if(routeGuide.indexCurrentPoint >= static_cast<uint32_t>(activePlanGuide.points.size()))
        {
            return false;
        }

        //! выходные данные
        routeGuide.trackDeviationZ             = dZ;  //! боковое отклонение в метрах
        routeGuide.distanceTrackLineToCurPoint = dS;
        routeGuide.distanceToCurPoint          = distanceToNextPPM;
        routeGuide.desiredTrackAngleToCurPoint = psi_des;   //delta_psi_des;
        routeGuide.desiredAltitudeToCurPoint   = curPoint.altitude;
        routeGuide.azimuthToCurPoint           = azimuthToNextPPM;
        routeGuide.remainTimeToCurPoint        = qFuzzyCompare(curSpeed, 0.0f) ? 0 : (distanceToNextPPM/curSpeed);
        routeGuide.remainTimeToNextPoint       = qFuzzyCompare(curSpeed, 0.0f) ? 0 : ((distanceToNextPPM + distanceToNextNextPPM)/curSpeed);

        activePlanInfo.remainFlightDistance = static_cast<uint16_t>(distanceToNextPPM);

        for(uint32_t i{routeGuide.indexCurrentPoint+1}; i<activePlanInfo.waypoints.size(); i++)
            activePlanInfo.remainFlightDistance += activePlanInfo.waypoints[i].distance;

        activePlanInfo.remainFlightTime = activePlanInfo.remainFlightDistance/curSpeed;

        return true;
    }
}
#endif

void ManagerCalculationPlan::calcWaypointFromDistAndTrack(float B0, float L0, float rng, float az, float *B, float *L)
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

void ManagerCalculationPlan::calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
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

float ManagerCalculationPlan::getMeanSpeed(float speed)
{
    speed = (speed < fpg.Vnom) ? fpg.Vnom : speed;
    fpg.Vbuf.enqueue(speed);
    if(fpg.Vbuf.size() > static_cast<int>(fpg.Vbufsize))
        fpg.Vbuf.dequeue();

    float ret {};
    for(float v : fpg.Vbuf)
        ret += v;

    return ret /= fpg.Vbuf.size();
}

float ManagerCalculationPlan::bound_pi(float val, float bnd)
{
    float two_bnd = 2*bnd;
    while (val > bnd)
        val -= two_bnd;

    while (val < -bnd)
        val += two_bnd;

    return val;
}

float ManagerCalculationPlan::bound_2pi(float val, float bnd)
{
    while (val > bnd)
        val -= bnd;

    while (val < 0)
        val += bnd;

    return val;
}

void ManagerCalculationPlan::trySafeOldActivePoint()
{
    bool pointsIsIdentity {true};
    int pos {};
    for(; pos <= fpg.indexCurrentPoint; pos++)
    {
        if(editActivePlan.waypoints[pos] != activePlan.waypoints[pos])
        {
            pointsIsIdentity = false;
            break;
        }
    }
    pos = pointsIsIdentity ? pos - 1 : 0;
    setRoute(editActivePlan, pos);
}
