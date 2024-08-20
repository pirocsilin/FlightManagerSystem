
#include "labsflightplan.h"
#include <cmath>

namespace fp {

QDataStream &operator <<(QDataStream &stream, const CommandStatus &type)
{
    stream << static_cast<int>(type);
    return stream;
}

QDataStream &operator >>(QDataStream &stream, CommandStatus &type)
{
    int _type;
    stream >> _type;
    type = static_cast<CommandStatus>(_type);

    return stream;
}

QDataStream &operator <<(QDataStream &stream, const WaypointType &type)
{
    stream << static_cast<int>(type);
    return stream;
}

QDataStream &operator >>(QDataStream &stream, WaypointType &type)
{
    int _type;
    stream >> _type;
    type = static_cast<WaypointType>(_type);

    return stream;
}

QDataStream &operator <<(QDataStream &stream, const Waypoint &point)
{
    stream << point.id
           << QString::fromStdString(point.icao)
           << QString::fromStdString(point.region)
           << point.type
           << point.latitude
           << point.longitude
           << point.altitude
           << point.radioFrequency
           << point.runwayId;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, Waypoint &point)
{
    QString _icao, _region;
    stream >> point.id
           >> _icao
           >> _region
           >> point.type
           >> point.latitude
           >> point.longitude
           >> point.altitude
           >> point.radioFrequency
           >> point.runwayId;

    point.icao = _icao.toStdString();
    point.region = _region.toStdString();

    return stream;
}

QDataStream &operator <<(QDataStream &stream, const std::vector<Waypoint> &vec)
{
    for(auto point : vec)
    {
        stream << point;
    }
    return stream;
}

QDataStream &operator >>(QDataStream &stream, std::vector<Waypoint> &vec)
{
    while(!stream.atEnd())
    {
        Waypoint point;
        stream >> point;
        vec.push_back(point);
    }
    return stream;
}

void printWaypointSmallInfo(Waypoint &pt)
{
    QString msg = QString("\t%1\t%2\t\t%3").arg(QString::fromStdString(pt.icao)).
                  arg(pt.latitude).arg(pt.longitude);

    qDebug() << msg.toUtf8().data();
}

void printWaypointFullInfo(Waypoint &pt)
{
    QString info = QString("\tid: %1\ticao: %2\treg: %3\ttype: %4\tlat: "
                           "%5\t lon: %6\talt: %7\tfreq: %8\t rwId: %9")
                           .arg(pt.id).arg(QString::fromStdString(pt.icao))
                           .arg(QString::fromStdString(pt.region)).arg(static_cast<int>(pt.type))
                           .arg(pt.latitude).arg(pt.longitude)
                           .arg(pt.altitude).arg(pt.radioFrequency).arg(pt.runwayId);

    qDebug() << info.toUtf8().data();
}

QDataStream &operator <<(QDataStream &stream, const FlightPlanInfo &obj)
{
    stream << obj.id
           << QString::fromStdString(obj.name)
           << obj.totalDistance;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, FlightPlanInfo &obj)
{
    QString _name;

    stream >> obj.id
           >> _name
           >> obj.totalDistance;

    obj.name = _name.toStdString();

    return stream;
}

QDataStream &operator <<(QDataStream &stream, const std::vector<FlightPlanInfo> &vec)
{
    for(auto info : vec)
    {
        stream << info;
    }
    return stream;
}

QDataStream &operator >>(QDataStream &stream, std::vector<FlightPlanInfo> &vec)
{
    while(!stream.atEnd())
    {
        FlightPlanInfo info;
        stream >> info;
        vec.push_back(info);
    }
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const FlightPlan &plan)
{
    stream << plan.id << QString::fromStdString(plan.name);

    for(auto point : plan.waypoints)
    {
        stream << point;
    }
    return stream;
}

QDataStream &operator >>(QDataStream &stream, FlightPlan &plan)
{
    QString _name;
    stream >> plan.id >> _name;
    plan.name = _name.toStdString();

    while(!stream.atEnd())
    {
        Waypoint point;
        stream >> point;
        plan.waypoints.push_back(point);
    }
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const std::pair<CommandStatus, FlightPlan> &pair)
{
    stream << pair.first << pair.second;
    return stream;
}

QDataStream &operator >>(QDataStream &stream, std::pair<CommandStatus, FlightPlan> &pair)
{
    stream >> pair.first >> pair.second;
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const cmdID &data)
{
    stream << static_cast<int>(data);
    return stream;
}

QDataStream &operator >>(QDataStream &stream, cmdID &data)
{
    int _data;
    stream >> _data;
    data = static_cast<cmdID>(_data);

    return stream;
}

void printWaypointInfo(std::pair<fp::CommandStatus, fp::Waypoint> &point)
{
    qDebug() << "Status: " << (int)point.first;

    if(point.first == fp::CommandStatus::OK)
    {
        printWaypointFullInfo(point.second);
    }
}

void printPlanInfo(std::pair<fp::CommandStatus, fp::FlightPlan> &plan)
{
    qDebug() << "Status: " << (int)plan.first;

    if(plan.first == fp::CommandStatus::OK)
     {
        qDebug() << QString("id: %1, name: %2")
                    .arg(plan.second.id)
                    .arg(QString::fromStdString(plan.second.name));

        for(auto point : plan.second.waypoints)
        {
            printWaypointFullInfo(point);
        }
    }
}

void printCatalogInfoOfPlans(std::pair<fp::CommandStatus, std::vector<FlightPlanInfo>> &plansInfo)
{
    qDebug() << "Status: " << (int)plansInfo.first;

    if(plansInfo.first == fp::CommandStatus::OK)
    {
    for(auto planInfo : plansInfo.second)
        {
            qDebug() << QString("id: %1, name: %2, totalDistance: %3")
                        .arg(planInfo.id)
                        .arg(QString::fromStdString(planInfo.name))
                        .arg(planInfo.totalDistance);
        }
    }
}

void printWaypointRouteInfo(WaypointRouteInfo &point)
{
    QString msg = QString("\tid: %1\ticao: %2 \tbearing: %3 \tdist: %4 \talt: %5 \tactive: %6")
                .arg(point.id)
                .arg(QString::fromStdString(point.icao))
                .arg(point.bearing)
                .arg(point.distance)
                .arg(point.altitude)
                .arg(point.isActive);

    qDebug() << msg.toUtf8().data();
}


void printFlightPlanRouteInfo(std::pair<CommandStatus, FlightPlanRouteInfo> &data)
{
    qDebug() << "Status: " << (int)data.first;

    if(data.first == fp::CommandStatus::OK)
    {
        qDebug() << QString("id: %1, name: %2")
                    .arg(data.second.id)
                    .arg(QString::fromStdString(data.second.name));
        for(auto point : data.second.waypoints)
        {
            printWaypointRouteInfo(point);
        }
    }
}

void printActivePlanInfo(std::pair<CommandStatus, ActivePlanInfo> &data)
{
    qDebug() << "Status: " << (int)data.first;

    if(data.first == fp::CommandStatus::OK)
    {
        qDebug() << QString("id: %1, name: %2, r_Dist: %3, r_Time: %4")
                    .arg(data.second.id)
                    .arg(QString::fromStdString(data.second.name))
                    .arg(data.second.remainFlightDistance)
                    .arg(data.second.remainFlightTime);

        for(auto point : data.second.waypoints)
            printWaypointRouteInfo(point);

        printWaypointFullInfo(data.second.activeWaypoint);
    }
}

QDataStream &operator <<(QDataStream &stream, const WaypointRouteInfo &data)
{
    stream << data.id
           << QString::fromStdString(data.icao)
           << data.bearing
           << data.distance
           << data.altitude
           << data.isActive;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, WaypointRouteInfo &data)
{
    QString _icao;

    stream >> data.id
           >> _icao
           >> data.bearing
           >> data.distance
           >> data.altitude
           >> data.isActive;

    data.icao =  _icao.toStdString();
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const std::vector<WaypointRouteInfo> &vec)
{
    for(auto info : vec)
        stream << info;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, std::vector<WaypointRouteInfo> &vec)
{
    while(!stream.atEnd())
    {
        WaypointRouteInfo info;
        stream >> info;
        vec.push_back(info);
    }
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const FlightPlanRouteInfo &data)
{
    stream << data.id
           << QString::fromStdString(data.name)
           << data.waypoints;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, FlightPlanRouteInfo &data)
{
    QString _name;

    stream >> data.id
           >> _name
           >> data.waypoints;

    data.name = _name.toStdString();
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const HeaderData &data)
{
    stream << static_cast<uint8_t>(data.name)
           << data.uniqueCmd
           << data.id;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, HeaderData &data)
{
    uint8_t  _name;

    stream >> _name
           >> data.uniqueCmd
           >> data.id;

    data.name = static_cast<HeaderData::Name>(_name);

    return stream;
}

void getHdrFromResponse(QByteArray &data, HeaderData &hdr)
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_3);

    stream >> hdr;
}

void clearPlan(FlightPlan &plan)
{
    plan = FlightPlan();
}

void invertPlan(FlightPlan &plan)
{
    int lenght = plan.waypoints.size();

    for(int i = 0; i < lenght / 2; i++)
    {
        std::swap(plan.waypoints[i], plan.waypoints[lenght-i-1]);
    }
}

void createNameForPlan(FlightPlan &plan)
{
    std::string first = plan.waypoints.size() > 0 ?  plan.waypoints.front().icao : "______";
    std::string last  = plan.waypoints.size() > 1 ?  plan.waypoints.back().icao  : "______";

    plan.name = first + " / " + last;
}

double distanceToPoint(double lat1, double lon1, double lat2, double lon2)
{
    // Радиус земли, м [https://gis-lab.info/qa/great-circles.html]
    double earthRadius { 6372795.0 };
    // Морская миля, км [ru.wikipedia.org]
    double Nautical_Mile { 1.852 };

    double lat1Rad = lat1   * M_PI / 180. ;
    double lon1Rad = lon1   * M_PI / 180. ;
    double lat2Rad = lat2   * M_PI / 180. ;
    double lon2Rad = lon2   * M_PI / 180. ;

    double deltaLat = lat2Rad - lat1Rad;
    double deltaLon = lon2Rad - lon1Rad;

    // Формула гаверсинусов [https://gis-lab.info/qa/great-circles.html]
    return (2 * earthRadius * asin(sqrt(
               pow(sin(deltaLat/2),2) + cos(lat1Rad) *
                     cos(lat2Rad) * pow(sin(deltaLon/2),2)))) /
            1000; // / Nautical_Mile;
}

double calculateBearing(double lat1, double lon1, double lat2, double lon2)
{
    lat1 = lat1 * M_PI / 180.;
    lon1 = lon1 * M_PI / 180.;
    lat2 = lat2 * M_PI / 180.;
    lon2 = lon2 * M_PI / 180.;

    double dLon = lon2 - lon1;

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);

    double bearing = atan2(y, x);
    bearing = fmod((bearing + 2 * M_PI), (2 * M_PI));

    return bearing * 180.0 / M_PI;
}

void sortWaypointVector(std::vector<Waypoint> &vector)
{
    //    std::sort(vector.begin(), vector.end(),
    //         [](Waypoint &obj1, Waypoint &obj2){
    //        return obj1.distanceTo < obj2.distanceTo;
    //    });
}

void removeDistantPoint(std::vector<Waypoint> &vector, float dist, WaypointType type)
{
    //    vector.erase(std::remove_if(vector.begin(),
    //                                vector.end(),
    //                                [&](Waypoint pt)
    //                                {return pt.distanceTo > dist ||
    //                                     !(static_cast<int>(pt.type) & static_cast<int>(type)) ;}),
    //                                vector.end());
}

}
