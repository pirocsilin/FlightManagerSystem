#ifndef LABSFLIGHTPLAN_H
#define LABSFLIGHTPLAN_H

#include <cmath>
#include <QDebug>
#include <QDataStream>
#include "structsflightplan.h"

#define TWO_PI (2.0*M_PI)       //!< Число 2*Пи
#define EARTH_RADIUS 6371000.0  //!< Условный радиус земной сферы, м

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
bool pointIsValid(const Waypoint &point);

// CommandStatus
void printCommandStatus(const CommandStatus &status);

// FlightPlanRouteInfo
void printWaypointRouteInfo(WaypointRouteInfo &point);
void printFlightPlanRouteInfo(const FlightPlanRouteInfoPair &info);
void printActivePlanInfo(const std::pair<CommandStatus, ActivePlanInfo> &info);

// Waypoints Info
double distanceToPoint      (double lat1, double lon1, double lat2, double lon2);
double calculateBearing     (double lat1, double lon1, double lat2, double lon2);
void printWaypointInfo      (const WaypointPair &point);
void printEditWaypointInfo  (const WaypointPair &point);
void printWaypointVectorInfo(const WaypointVectorPair &vector);
void printWaypointSmallInfo (Waypoint &pt);
void printWaypointFullInfo  (const Waypoint &pt);

// FlightPlan Info
void printPlanInfo          (const FlightPlanPair &plan);
void printCatalogInfoOfPlans(const std::pair<fp::CommandStatus, std::vector<FlightPlanInfo>> &planInfo);
void clearPlan              (FlightPlan &plan);
void invertPlan             (FlightPlan &plan);
void createNameForPlan      (FlightPlan &plan);

// NavDataInfo
void printNavDataFms(fp::NavDataFms& data);

enum cmdID : uint32_t
{
    GET_PLAN                   = 0x00001,   //!< получить информацию о плане по id
    SAVE_PLAN                  = 0x00002,   //!< сохранить план в базу
    DELETE_PLAN                = 0x00004,   //!< удалить план из базы
    INVERT_PLAN                = 0x00008,   //!< инвертировать план
    GET_WAYPOINT_BY_ID         = 0x00010,   //!< получить ППМ по id
    GET_WAYPOINT_BY_ICAO       = 0x00020,   //!< получить список точек по ИКАО
    GET_NEAREST_WAYPOINTS      = 0x00040,   //!< получить ближайшие точки
    SAVE_WAYPOINT              = 0x00080,   //!< сохранить ППМ в базу
    DELETE_WAYPOINT            = 0x00100,   //!< удалить ППМ из базы
    GET_CATALOG_INFO_OF_PLANS  = 0x00200,   //!< сведения о каждом плане полета в каталоге
    GET_PLAN_ROUTE_INFO        = 0x00400,   //!< данные о плане со списком точек
    //
    START_UPDATE_DATABASE      = 0x00800,   //!< начало обновления базы данных
    STOP_UPDATE_DATABASE       = 0x01000,   //!< конец обновления базы данных
    ERROR_DATABASE             = 0x02000,   //!< FMS вернул ошибку БД
    //
    MODIFICATION_CMD = SAVE_PLAN | DELETE_PLAN | INVERT_PLAN | SAVE_WAYPOINT | DELETE_WAYPOINT,
};
QDataStream &operator << (QDataStream &stream, const cmdID &data);
QDataStream &operator >> (QDataStream &stream, cmdID &data);

struct HeaderData
{
    enum Name : uint8_t
    {
        ADAPTER_COMMAND = 0,
        CONTROLLER_ANS,
        MFI_COMMAND,
        MFI_ANS
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

void calcDistAndTrackBetweenWaypoints(float b1, float l1, float b2, float l2,
                                      float *r=nullptr, float *az1=nullptr, float *az2=nullptr);

void sortWaypointByDistance(float curLat, float curLon, std::vector<Waypoint> &vector);

}

#endif // LABSFLIGHTPLAN_H
