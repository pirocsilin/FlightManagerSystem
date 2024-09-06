#ifndef MATHPLAN_H
#define MATHPLAN_H

#include <QString>
#include <QVector>
#include <QMutex>
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

    Qt::HANDLE selfThread;
    bool activePlanIsSet {false};

    void setRoute(FlightPlan &plan, uint activePoint=0);    //!< установка маршрута для активного плана
    void resetRoute();                                      //!< очистка расчетных параметров

    NavDataFms navDataFms;                                  //!< возвращаемые расчитанные пилотажные данные

    FlightPlan& getEditActivePlan();
    void setEditActivePlan();
    uint32_t getIndexActivePoint();

    void addWaypointToActivePlan(uint32_t position, Waypoint &point);
    void setCurrentPosition (float latitude, float longitude);

//private:

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

    float getMeanSpeed(float speed);
    float bound_pi(float val, float bnd);
    float bound_2pi(float val, float bnd);

    void trySafeOldActivePoint();

    const double EARTH_RADIUS {6371000.0}; //!< Условный радиус земной сферы, м

public slots:

    void setDeviceFlightData(const DeviceFlightData& data);     //!< установка навигационных данных
    void sortWaypointByDistance(std::vector<Waypoint> &vector); //!< сортировка точек по удаленности
    void getActivePlanInfo();                                   //!< получить активный план(ActivePlanInfo) сигналом
    void getActivePlanInfo(std::pair<fp::CommandStatus,         //!< получить активный план(ActivePlanInfo) объектом
                           fp::ActivePlanInfo> &);
    void activatePlan(FlightPlan &plan);                        //!< активация плана
    void selectNextPoint(bool direction);                       //!< переключение на следующую ППМ
    void getCurrrentPosision(float &latitude, float &longitude);//!< получить текущее положение ВС

signals:

    void signalNavDataFms(NavDataFms navDataFms);
    void signalGetActivePlanInfo(ActivePlanInfoPair);
    void signalSelectNextPoint();
};


#endif // MATHPLAN_H
