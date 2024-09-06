#ifndef LABSFLIGHTPLAN_H
#define LABSFLIGHTPLAN_H

#include <QDebug>
#include <QDataStream>
#include "structsflightplan.h"

namespace fp {

// CommandStatus
QDataStream &operator <<(QDataStream &stream, const CommandStatus &type);
QDataStream &operator >>(QDataStream &stream, CommandStatus &type);

// std::string
QDataStream &operator <<(QDataStream &stream, const std::string &type);
QDataStream &operator >>(QDataStream &stream, std::string &type);

// WaypointType
QDataStream &operator << (QDataStream &stream, const WaypointType &type);
QDataStream &operator >> (QDataStream &stream, WaypointType &type);

// Waypoint
QDataStream& operator << (QDataStream &stream, const Waypoint &obj);
QDataStream& operator >> (QDataStream &stream, Waypoint &obj);

// std::vector<Waypoint>
QDataStream& operator << (QDataStream &stream, const std::vector<Waypoint> &obj);
QDataStream& operator >> (QDataStream &stream, std::vector<Waypoint> &obj);

// FlightPlan
QDataStream& operator << (QDataStream &stream, const FlightPlan &obj);
QDataStream& operator >> (QDataStream &stream, FlightPlan &obj);

// FlightPlanInfo
QDataStream& operator << (QDataStream &stream, const FlightPlanInfo &obj);
QDataStream& operator >> (QDataStream &stream, FlightPlanInfo &obj);

// std::vector<FlightPlanInfo>
QDataStream& operator << (QDataStream &stream, const std::vector<FlightPlanInfo> &vec);
QDataStream& operator >> (QDataStream &stream, std::vector<FlightPlanInfo> &vec);

// WaypointRouteInfo
QDataStream &operator << (QDataStream &stream, const WaypointRouteInfo &obj);
QDataStream &operator >> (QDataStream &stream, WaypointRouteInfo &obj);

// std::vector<WaypointRouteInfo>
QDataStream& operator << (QDataStream &stream, const std::vector<WaypointRouteInfo> &obj);
QDataStream& operator >> (QDataStream &stream, std::vector<WaypointRouteInfo> &obj);

// FlightPlanRouteInfo
QDataStream &operator << (QDataStream &stream, const FlightPlanRouteInfo &obj);
QDataStream &operator >> (QDataStream &stream, FlightPlanRouteInfo &obj);

// compare Waypoint
bool operator ==(const Waypoint &one, const Waypoint &two);
bool operator !=(const Waypoint &one, const Waypoint &two);

// CommandStatus
void printCommandStatus(const CommandStatus &status);

// FlightPlanRouteInfo
void printWaypointRouteInfo(WaypointRouteInfo &point);
void printFlightPlanRouteInfo(const FlightPlanRouteInfoPair &info);
void printActivePlanInfo(const std::pair<CommandStatus, ActivePlanInfo> &info);

// Waypoints Info
double distanceToPoint(double lat1, double lon1, double lat2, double lon2);
double calculateBearing(double lat1, double lon1, double lat2, double lon2);
void printWaypointInfo(std::pair<fp::CommandStatus, fp::Waypoint> &point);
void printWaypointVectorInfo(const WaypointVectorPair &vector);
void sortWaypointVector (std::vector<Waypoint> &vector);
void removeDistantPoint(std::vector<Waypoint> &vector, float dist, WaypointType type);
void printWaypointSmallInfo(Waypoint &pt);
void printWaypointFullInfo(Waypoint &pt);

// FlightPlan Info
void printPlanInfo(const std::pair<fp::CommandStatus, fp::FlightPlan> &plan);
void printCatalogInfoOfPlans(const std::pair<fp::CommandStatus, std::vector<FlightPlanInfo>> &planInfo);
void clearPlan(FlightPlan &plan);
void invertPlan(FlightPlan &plan);
void createNameForPlan(FlightPlan &plan);

// NavDataInfo
void printNavDataFms(fp::NavDataFms& data);

enum cmdID
{
    GET_PLAN,                   //!< получить информацию о плане по id
    SAVE_PLAN,                  //!< сохранить план в базу
    DELETE_PLAN,                //!< удалить план из базы
    INVERT_PLAN,                //!< инвертировать план
    GET_WAYPOINT_BY_ID,         //!< получить ППМ по id
    GET_WAYPOINT_BY_ICAO,       //!< получить список точек по ИКАО
    GET_NEAREST_WAYPOINTS,      //!< получить ближайшие точки
    SAVE_WAYPOINT,              //!< сохранить ППМ в базу
    DELETE_WAYPOINT,            //!< удалить ППМ из базы
    GET_CATALOG_INFO_OF_PLANS,  //!< сведения о каждом плане полета в каталоге
    GET_PLAN_ROUTE_INFO,        //!< данные о плане со списком точек
    //
    ERROR_DATABASE,             //!< FMS вернул ошибку БД
};
QDataStream &operator << (QDataStream &stream, const cmdID &data);
QDataStream &operator >> (QDataStream &stream, cmdID &data);

struct HeaderData
{
    enum Name : uint8_t
    {
        ADAPTER_COMMAND = 0,
        CONTROLLER_ANS,
        FMS_COMMAND,
    };

    Name        name;
    uint32_t    uniqueCmd;
    cmdID       id;

    bool checkAnswer(HeaderData& sendHdr)
    {
        return uniqueCmd == sendHdr.uniqueCmd &&
               name      == CONTROLLER_ANS;
    }
};
QDataStream &operator << (QDataStream &stream, const HeaderData &data);
QDataStream &operator >> (QDataStream &stream, HeaderData &data);

template<typename T>
void getDataFromResponse(QByteArray &data, T &type)
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_3);
    HeaderData hdr;

    stream >> hdr >> type;
}

void getHdrFromResponse(QByteArray &data, HeaderData &hdr);

}

#endif // LABSFLIGHTPLAN_H
