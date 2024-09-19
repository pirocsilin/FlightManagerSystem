#ifndef MATHPLAN_H
#define MATHPLAN_H

#include <QString>
#include <QVector>
#include "common/structsflightplan.h"
#include "common/labsflightplan.h"
#include "structs_data.h"

using namespace fp;

class ActivePlanManager : public QObject
{
    Q_OBJECT

public:

    explicit ActivePlanManager();
    ~ActivePlanManager() = default;

    void addWaypointToActivePlan(uint32_t position, Waypoint &point);

private:

    NavDataFms              navDataFms;         //!< возвращаемые расчитанные пилотажные данные
    ActivePlanGuide         fpg;                //!< параметры для расчета активного плана
    ActivePlanInfo          activePlanInfo;     //!< информация об активном плане
    FlightPlan              activePlan;         //!< активный план
    FlightPlan              editActivePlan;     //!< редактируемый активный план
    std::pair<float, float> currentPosition;    //!< текущая позиция ВС

    bool calcActivePlan(float latitudeCurPosition, float longitudeCurPosition, float Vx, float Vz);

    void calcWaypointFromDistAndTrack(float B0, float L0, float rng, float az, float *B, float *L);

    void calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
                                          float *r = nullptr, float *az1 = nullptr,
                                          float *az2 = nullptr);

    void setCurrentPosition (float lat, float lon);         //!< запомнить текущую позицию ВС
    void setRoute(FlightPlan &plan);                        //!< установка маршрута для активного плана
    void resetRoute();                                      //!< очистка расчетных параметров

    float getMeanSpeed(float speed);
    float bound_pi(float val, float bnd);
    float bound_2pi(float val, float bnd);

    uint32_t trySafeOldActivePoint(FlightPlan &plan);

public slots:

    void setDeviceFlightData(const DeviceFlightData& data);     //!< установка навигационных данных
    void sortWaypointByDistance(std::vector<Waypoint> &vector); //!< сортировка точек по удаленности
    void getActivePlanInfo();                                   //!< получить активный план(ActivePlanInfo) сигналом
    void getActivePlanInfo(ActivePlanInfoPair&);                //!< получить активный план(ActivePlanInfo) объектом
    void activatePlan(FlightPlan &plan);                        //!< активация плана
    void selectNextPoint(bool direction);                       //!< переключение на следующую ППМ
    void getCurrrentPosition(float &latitude, float &longitude);//!< получить текущее положение ВС

signals:

    void signalNavDataFms(NavDataFms navDataFms);
    void signalGetActivePlanInfo(ActivePlanInfoPair);
    void signalSelectNextPoint();
};


#endif // MATHPLAN_H
