#ifndef MANAGERDIRECTPLANK_H
#define MANAGERDIRECTPLANK_H

#if 0
#include <vector>
#include <QTimer>

#include "flightroute/directplankroute.h"
#include "structs.h"
#include "flightroute/structsflightroute.h"

struct DirectPlank
{
    float    deviationGlissadePlank {NAN};  //!< отклонение горизонтальной планки, градусы
    float    deviationCoursePlank {NAN};    //!< отклонение вертикальной планки, условные единицы
    float    curPointDistance {};           //!< дальность от ВС до текущей точки, м
    uint32_t curPointNumber;                //!< номер текущей точки, с 0
    uint32_t countPointsInRoute;            //!< количество точек в маршруте
    float    insAltitude;                   //!< высота БИНС, м
    float    curPointAltitude;              //!< высота текущей точки, м
    float    curPointAzimuth {};            //!< азимут на текущую точку, относительно ВС, градусы
    QString  routeName {"-"};               //!< наименование активного маршрута
};

namespace bus {
class Manager;
}

class MapViewer;
class ManagerDirectPlank
{
public:
    explicit ManagerDirectPlank() = default;
    ~ManagerDirectPlank() = default;

    void setNavDirectData(const NavDataForDirectPlank &data);
    void setRoute(const flightroute::FlightRoute &route);
    void resetRoute();

    //DirectPlank getDirectPlank();

    void switchToNextPoint();
    void switchToPrevPoint();

    //void setMapViewer(MapViewer *ptr);
    //void setBusManager(bus::Manager *manager);

private:
    flightroute::FlightRoute originFlightRoute;
    DirectPlankRoute         directPlankRoute;
    DirectPlankRouteGuide    routeGuide;
    DirectPlank              directPlankForGui;    //!< данные для отображения в gui * ret NavDataFms
    //bool isLoopRoute {true};    //!< true - маршрут зациклен
    //bool needUpdateRouteOnMap {false};  //!< true - надо обновить маршрут на карте
    bool isBeginOfRoute {true}; //!< true - начало маршрута (для особой отрисовки зацикленного маршрута)

    //MapViewer *mapViewerRoute {nullptr};
   // bus::Manager *busManager {nullptr};
    //QTimer timerNavData;

    //void updateRouteInMapViewer();
    bool calcDirectPlank(float latitudeCurPosition, float longitudeCurPosition,
                         float altitudeCurPosition,
                         float Vx, float Vz, float pitch, bool isHelicopter); // -alt, pitch
    void calcWaypointFromDistAndTrack(float B0, float L0, float rng, float az, float *B, float *L);

    void calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
                                          float *r = nullptr, float *az1 = nullptr,
                                          float *az2 = nullptr);

    float bound_2pi(float val, float bnd);
    float bound_pi(float val, float bnd);

    //int limitCountShownPoint {6};
    const double EARTH_RADIUS {6371000.0}; //!< Условный радиус земной сферы, м

    //void addShownLines(flightroute::FlightRoute &route, uint32_t indexActivePoint);
    //void addShownPoints(flightroute::FlightRoute &route, uint32_t indexActivePoint);
    //void setupTimerNavData();

    bool isCurPointIsLastInRoute();
    float getMeanSpeed(float speed);
};
#endif

#endif // MANAGERDIRECTPLANK_H
