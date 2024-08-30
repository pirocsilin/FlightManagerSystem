#ifndef LABSFLIGHTPLAN_H
#define LABSFLIGHTPLAN_H

#include <QDebug>
#include <QDataStream>
#include "structsflightplan.h"

namespace fp {

// CommandStatus
QDataStream &operator <<(QDataStream &stream, const CommandStatus &type);
QDataStream &operator >>(QDataStream &stream, CommandStatus &type);

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
bool pointIsValid(const Waypoint &point);

// FlightPlanRouteInfo
void printWaypointRouteInfo(WaypointRouteInfo &point);
void printFlightPlanRouteInfo(std::pair<CommandStatus, FlightPlanRouteInfo> &info);
void printActivePlanInfo(ActivePlanInfo &info);

// Waypoints Info
double distanceToPoint(double lat1, double lon1, double lat2, double lon2);
double calculateBearing(double lat1, double lon1, double lat2, double lon2);
void printWaypointInfo(std::pair<fp::CommandStatus, fp::Waypoint> &point);
void sortWaypointVector (std::vector<Waypoint> &vector);
void removeDistantPoint(std::vector<Waypoint> &vector, float dist, WaypointType type);
void printWaypointSmallInfo(Waypoint &pt);
void printWaypointFullInfo(Waypoint &pt);

// FlightPlan Info
void printPlanInfo(std::pair<fp::CommandStatus, fp::FlightPlan> &plan);
void printCatalogInfoOfPlans(std::pair<fp::CommandStatus, std::vector<FlightPlanInfo>> &planInfo);
void clearPlan(FlightPlan &plan);
void invertPlan(FlightPlan &plan);
void createNameForPlan(FlightPlan &plan);

enum cmdID
{
    GET_PLAN,                   //!< получить информацию о плане по id
    SAVE_PLAN,                  //!< сохранить план в базу
    DELETE_PLAN,                //!< удалить план из базы
    GET_WAYPOINT,               //!< получить ППМ
    GET_CATALOG_INFO_OF_PLANS,  //!< сведения о каждом плане полета в каталоге
    GET_PLAN_ROUTE_INFO,        //!< данные о плане со списком точек
    //
    SAVE_WAYPOINT,              //!< сохранить ППМ в базу
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
