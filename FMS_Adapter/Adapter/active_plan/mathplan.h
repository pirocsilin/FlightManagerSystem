#ifndef MATHPLAN_H
#define MATHPLAN_H

#include <QString>
#include <QVector>
#include <QMutex>
#include "common/structsflightplan.h"
#include "common/labsflightplan.h"
#include "structs_data.h"

using namespace fp;

class ManagerCalculationPlan
{
public:

    explicit ManagerCalculationPlan();
    ~ManagerCalculationPlan() = default;

    void setNavData(const DeviceFlightData& data);          //!< установка навигационных данных
    void setRoute(FlightPlan &plan, uint activePoint=0);    //!< установка маршрута для активного плана
    void resetRoute();                                      //!< очистка расчетных параметров

    NavDataFms navDataFms;                          //!< returning data

    ActivePlanInfo& getActivePlanInfo()  { return activePlanInfo; }
    void selectNextPoint(bool direction);

    FlightPlan& getEditActivePlan();
    void setEditActivePlan();
    uint32_t getIndexActivePoint();

    void addWaypointToActivePlan(uint32_t position, Waypoint &point);

//private:

    ActivePlanGuide fpg;            //!< параметры для расчета активного плана
    ActivePlanInfo activePlanInfo;  //!< информация об активном плане
    FlightPlan activePlan;          //!< активный план
    FlightPlan editActivePlan;      //!< редактируемый активный план

    bool calcActivePlan(float latitudeCurPosition, float longitudeCurPosition, float Vx, float Vz);

    void calcWaypointFromDistAndTrack(float B0, float L0, float rng, float az, float *B, float *L);

    void calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
                                          float *r = nullptr, float *az1 = nullptr,
                                          float *az2 = nullptr);

    float getMeanSpeed(float speed);
    float bound_pi(float val, float bnd);
    float bound_2pi(float val, float bnd);

    void trySafeOldActivePoint();

    const double EARTH_RADIUS {6371000.0}; //!< Условный радиус земной сферы, м
};


#endif // MATHPLAN_H
