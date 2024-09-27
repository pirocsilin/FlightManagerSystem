
#include <cmath>
#include <qmath.h>
#include <QThread>
#include <QPair>
#include "mathplan.h"

#define TWO_PI (2.0*M_PI)  //!< Число 2*Пи


ActivePlanManager::ActivePlanManager()
{
    resetRoute();
    currentPosition.first  = std::numeric_limits<float>::quiet_NaN();
    currentPosition.second = std::numeric_limits<float>::quiet_NaN();
}

void ActivePlanManager::setRoute(FlightPlan &plan)
{
    uint32_t activePoint = trySafeOldActivePoint(plan);

    // сбрасываем настройки ведения по маршруту
    resetRoute();

    // запоминаем активный план
    activePlan = plan;

    if(activePlan.waypoints.empty()) return;

    // заполняем структуру ActivePlanInfo
    activePlanInfo.id   = activePlan.id;
    activePlanInfo.name = activePlan.name;
    activePlanInfo.activeWaypoint = activePlan.waypoints[activePoint];

    std::vector<Waypoint> &vec = activePlan.waypoints;
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
    fpg.name = QString(activePlan.name.c_str());
    for(auto &point : activePlan.waypoints)
    {
        ActivePlanGuide::PointParams newPoint{};
        newPoint.latitude  = qDegreesToRadians(point.latitude);
        newPoint.longitude = qDegreesToRadians(point.longitude);
        newPoint.altitude  = point.altitude;
        newPoint.name      = QString(point.icao.c_str());

        fpg.points.push_back(newPoint);
    }
}

void ActivePlanManager::resetRoute()
{
    //! сбрасываем настройки ведения по маршруту
    activePlanInfo     = {};
    navDataFms         = {};
    fpg.clearData();
    fpg.reset();
}

void ActivePlanManager::getActivePlanInfo()
{
    CommandStatus statusCmd = activePlanInfo.waypoints.size() > 0 ? CommandStatus::OK :
                                                                    CommandStatus::INVALID;
    auto result = std::make_pair(statusCmd, activePlanInfo);

    emit signalGetActivePlanInfo(result);
}

void ActivePlanManager::getActivePlanInfo(ActivePlanInfoPair &data)
{
    CommandStatus statusCmd = activePlanInfo.waypoints.size() > 0 ? CommandStatus::OK :
                                                                    CommandStatus::INVALID;
    data = std::make_pair(statusCmd, activePlanInfo);
}

void ActivePlanManager::activatePlan(FlightPlan &plan)
{
    setRoute(plan);
}

void ActivePlanManager::setDeviceFlightData(const fp::DeviceFlightData &data)
{
    navDataFms = {};
    setCurrentPosition(data.latitude, data.longitude);

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
        if(fpg.distanceToNextPPM <= fpg.switchPointDistance)
            selectNextPoint(true);

        emit signalNavDataFms(navDataFms);
    }
}

void ActivePlanManager::selectNextPoint(bool direction)
{
    if(activePlanInfo.waypoints.size() > 1)
    {
        uint32_t curIndex = fpg.indexCurrentPoint;
        if(fpg.selectNextPoint(direction))
        {
            activePlanInfo.waypoints[curIndex].isActive = false;
            activePlanInfo.waypoints[fpg.indexCurrentPoint].isActive = true;
            activePlanInfo.activeWaypoint = activePlan.waypoints[fpg.indexCurrentPoint];
        }

        emit signalSelectNextPoint();
    }
}

void ActivePlanManager::addWaypointToActivePlan(uint32_t position, Waypoint &point)
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
    uint32_t indexPositionAfterInsert = position <= indexCurrentPoint ?
                                       indexCurrentPoint + 1 :
                                       indexCurrentPoint;
    setRoute(editActivePlan);

    for(uint32_t i{}; i < indexPositionAfterInsert; i++)
        selectNextPoint(true);
}

void ActivePlanManager::setCurrentPosition(float latitude, float longitude)
{
    currentPosition.first = latitude;
    currentPosition.second = longitude;
}

void ActivePlanManager::getCurrrentPosition(float &latitude, float &longitude)
{
    latitude  = currentPosition.first;
    longitude = currentPosition.second;
}

void ActivePlanManager::sortWaypointByDistance(std::vector<Waypoint> &vector)
{
    float latitudeCurPosition {},
          longitudeCurPosition{},
          distanceToOne {},
          distanceToTwo {};

    getCurrrentPosition(latitudeCurPosition, longitudeCurPosition);
    if(!std::isnan(latitudeCurPosition) && !std::isnan(longitudeCurPosition))
    {
        std::sort(vector.begin(), vector.end(), [&](Waypoint &one, Waypoint &two){

            calcDistAndTrackBetweenWaypoints(latitudeCurPosition, longitudeCurPosition,
                                             one.latitude, one.longitude, &distanceToOne);

            calcDistAndTrackBetweenWaypoints(latitudeCurPosition, longitudeCurPosition,
                                             two.latitude, two.longitude, &distanceToTwo);

            return distanceToOne < distanceToTwo;
        });
    }
}

bool ActivePlanManager::calcActivePlan(float latitudeCurPosition, float longitudeCurPosition, float Vx, float Vz)
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
        fpg.trackDeviationZ = fpg.distanceToNextPPM * abs(sin(delta_az_back));

        qDebug() << QString("trackDeviationZ = %1 * sin(%2)")
                    .arg(fpg.distanceToNextPPM)
                    .arg(abs(sin(delta_az_back)));
    }

    qDebug() << QString("[%1] lat: %2, log: %3 -> target: %4, %5 | dist: %6 / %7m")
                .arg(fpg.indexCurrentPoint)
                .arg(latitudeCurPosition * 180 /M_PI)
                .arg(longitudeCurPosition * 180 /M_PI)
                .arg(curPoint.latitude * 180 /M_PI)
                .arg(curPoint.longitude * 180 /M_PI)
                .arg(fpg.distanceToNextPPM)
                .arg(fpg.switchPointDistance);

    bool nextPointExists  = fpg.indexCurrentPoint < fpg.points.size()-1;
    //
    fpg.distanceToNextNextPPM = nextPointExists ? activePlanInfo.waypoints[fpg.indexCurrentPoint+1].distance : 0.0f;
    fpg.remainTimeToCurPoint  = qFuzzyCompare(fpg.curSpeed, 0.0f) ? 0 : (fpg.distanceToNextPPM / fpg.curSpeed);
    fpg.remainTimeToNextPoint = qFuzzyCompare(fpg.curSpeed, 0.0f) ? 0 : ((fpg.distanceToNextPPM + fpg.distanceToNextNextPPM) / fpg.curSpeed);

    return true;
}

void ActivePlanManager::calcWaypointFromDistAndTrack(float B0, float L0, float rng, float az, float *B, float *L)
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

void ActivePlanManager::calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
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

float ActivePlanManager::getMeanSpeed(float speed)
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

float ActivePlanManager::bound_pi(float val, float bnd)
{
    float two_bnd = 2*bnd;
    while (val > bnd)
        val -= two_bnd;

    while (val < -bnd)
        val += two_bnd;

    return val;
}

float ActivePlanManager::bound_2pi(float val, float bnd)
{
    while (val > bnd)
        val -= bnd;

    while (val < 0)
        val += bnd;

    return val;
}

uint32_t ActivePlanManager::trySafeOldActivePoint(FlightPlan &plan)
{
    if(plan.id != -1 && plan.id == activePlan.id && fpg.indexCurrentPoint < plan.waypoints.size())
    {
        bool pointsIsIdentity {true};
        int  pos {0};
        for(; pos <= fpg.indexCurrentPoint; pos++)
        {
            if(plan.waypoints[pos] != activePlan.waypoints[pos])
            {
                pointsIsIdentity = false;
                break;
            }
        }
        return pointsIsIdentity ? pos-1 : 0;
    }
    return 0;
}



