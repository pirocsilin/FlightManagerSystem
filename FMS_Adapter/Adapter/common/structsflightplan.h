#ifndef STRUCTSFLIGHTPLAN_H
#define STRUCTSFLIGHTPLAN_H

#include <string>
#include <vector>

namespace fp {

enum class CommandStatus
{
    OK = 0,
    INVALID,
    NO_CONNECTION,
    ERROR_DATABASE,
};

enum class WaypointType
{
    NONE = 0,
    USER_POINT,
    AIRPORT,
    VOR,
    ALL,
};

struct FlightPlanInfo
{
    uint32_t    id;             //!< id плана
    std::string name;           //!< наименование плана
    float       totalDistance;  //!< общая протяженность маршрута, км
};

struct Runway
{
    uint32_t    id;         //!< id ВПП
    std::string name;       //!< наименование ВПП
    uint16_t    bearing;    //!< курс ВПП, градус
    uint16_t    length;     //!< длина ВПП, м
    uint16_t    width;      //!< ширина ВПП, м
};

struct Waypoint
{
    int32_t      id;             //!< id ППМ
    std::string  icao;           //!< ИКАО ППМ (max 4 символа)
    std::string  region;         //!< географическое расположение ППМ
    WaypointType type;           //!< тип точки
    double       latitude;       //!< широта, градусы
    double       longitude;      //!< долгота, градусы
    uint16_t     altitude;       //!< высота точки, м
    uint32_t     radioFrequency; //!< частота радиостанции, Гц
    uint32_t     runwayId;       //!< id ВПП
};

struct FlightPlan
{
    int32_t               id;           //!< id плана
    std::string           name;         //!< наименование плана
    std::vector<Waypoint> waypoints;    //!< список точек маршрута
};

struct WaypointRouteInfo
{
    uint32_t     id;        //!< id ППМ
    std::string  icao;      //!< ИКАО ППМ (max 4 символа)
    uint16_t     bearing;   //!< курс на ППМ, градус
    uint32_t     distance;  //!< дальность до ППМ, м
    uint16_t     altitude;  //!< высота ППМ, м
    bool isActive;          //!< true - активная ППМ
};

struct FlightPlanRouteInfo
{
    uint32_t                       id;           //!< id плана
    std::string                    name;         //!< наименование плана
    std::vector<WaypointRouteInfo> waypoints;    //!< список точек маршрута для отображения в каталоге
};

struct ActivePlanInfo
{
    uint32_t                       id;             //!< id плана
    std::string                    name;           //!< наименование плана
    uint32_t                       remainFlightDistance; //!< оставшееся расстояние полета по маршруту, м
    uint32_t                       remainFlightTime;     //!< оставшееся время полета по маршруту, c
    Waypoint                       activeWaypoint; //!< информация об активной точке
    std::vector<WaypointRouteInfo> waypoints;      //!< список точек маршрута с информацией
};

struct RouteInfo
{
    uint32_t                       id;           //!< id плана
    std::string                    name;         //!< наименование плана
    std::vector<WaypointRouteInfo> waypoints;    //!< список точек маршрута для отображения в каталоге
};

struct DeviceFlightData
{
    float latitude  {std::numeric_limits<float>::quiet_NaN()}; //!< широта, радианы
    float longitude {std::numeric_limits<float>::quiet_NaN()}; //!< долгота, радианы
    float speedVx   {std::numeric_limits<float>::quiet_NaN()}; //!< земная скорость на север, м/с
    float speedVz   {std::numeric_limits<float>::quiet_NaN()}; //!< земная скорость на восток, м/с
};

struct NavDataFms
{
    std::string activeWaypointIcao;     //!< наименование (код ИКАО) активной точки
    std::string nextWaypointIcao;       //!< наименование (код ИКАО) следующей точки
    uint32_t activeWaypointDistance;    //!< дальность до активной точки, м*10              // distanceToCurPoint
    uint32_t activeWaypointRemainTime;  //!< оставшееся время до активной точки, с          // remainTimeToCurPoint
    uint32_t nextWaypointRemainTime;    //!< оставшееся время до следующей точки, с
    uint32_t activeWaypointCourse;      //!< курс на активную точку, градусы                // azimuthToCurPoint
    uint32_t lateralDeviationPathLine;  //!< боковое отклонение от линии заданного пути, м  // trackDeviationZ
    uint32_t activeRadioBeaconDistance; //!< дальность до активного радиомаяка, м*10
    ActivePlanInfo activePlan;          //!< данные активного плана
};

typedef std::pair<fp::CommandStatus, fp::ActivePlanInfo>              ActivePlanInfoPair;
typedef std::pair<fp::CommandStatus, std::vector<fp::FlightPlanInfo>> FlightPlanInfoPair;
typedef std::pair<fp::CommandStatus, fp::FlightPlan>                  FlightPlanPair;
typedef std::pair<fp::CommandStatus, fp::FlightPlanRouteInfo>         FlightPlanRouteInfoPair;
typedef std::pair<fp::CommandStatus, fp::FlightPlan>                  FlightPlanPair;
typedef std::pair<fp::CommandStatus, fp::ActivePlanInfo>              ActivePlanInfoPair;
typedef std::pair<fp::CommandStatus, std::vector<fp::Waypoint>>       WaypointVectorPair;
typedef std::pair<fp::CommandStatus, fp::Waypoint>                    WaypointPair;

}

#endif // STRUCTSFLIGHTPLAN_H
