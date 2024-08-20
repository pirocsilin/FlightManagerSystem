#ifndef STRUCTSFLIGHTROUTE_H
#define STRUCTSFLIGHTROUTE_H

#include <string>
#include <vector>

namespace flightroute {

enum TypeRoutePoint
{
    TP_NONE = 0,                    //!< точка без изображения
    TP_AERODROME_CIVIL_LAND,        //!< гражданский сухопутный
    TP_AERODROME_MILIT_LAND,        //!< военный сухопутный
    TP_AERODROME_CIVIL_MILIT_LAND,  //!< совместный военный и гражданский сухопутный
    TP_AERODROME_CIVIL_GYDRO,       //!< гражданский гидроаэродром
    TP_AERODROME_MILIT_GYDRO,       //!< военный гидроаэродром
    TP_AERODROME_CIVIL_MILIT_GYDRO, //!< совместный военный и гражданский гидроаэродром
    TP_AERODROME_REZERV_1,          //!< запасной или необорудованный 1
    TP_AERODROME_REZERV_2,          //!< запасной или необорудованный 2
    TP_VERTODROME,                  //!< вертодром
    TP_USER_POINT,                  //!< пользовательские точки
    TP_RADIO_POD,                   //!< радиооборудование, ПОД
    TP_RADIO_PDZ,                   //!< радиооборудование, ПДЗ
    TP_RADIO_NDB,                   //!< радиооборудование, NDB
    TP_RADIO_VOR,                   //!< радиооборудование, VOR
    TP_RADIO_DME,                   //!< радиооборудование, DME
    TP_RADIO_VOR_DME,               //!< радиооборудование, VOR/DME
    TP_RADIO_POD2,                  //!< радиооборудование, ПОД2
    TP_RADIO_PDZ2,                  //!< радиооборудование, ПДЗ2
    TP_RADIO_BEACON_ELLIPTIC,       //!< радиооборудование, эллиптический радиомаяк
};

class RoutePoint
{
public:
    RoutePoint() = default;
    RoutePoint(double newLatitude, double newLongitude, int newType)
        : latitude(newLatitude),
        longitude(newLongitude),
        type(static_cast<TypeRoutePoint>(newType)) { }

    std::string name;       //!< наименование точки
    double latitude;        //!< широта, градусы
    double longitude;       //!< долгота, градусы
    uint32_t altitude;      //!< высота, м
    TypeRoutePoint type;    //!< тип точки для выбора значка
};

class RouteLine
{
public:
    RouteLine() = default;
    RouteLine(double newLatitudeBegin, double newLongitudeBegin,
              double newLatitudeEnd, double newLongitudeEnd,
              int newType, bool active)
        : latitudeBegin(newLatitudeBegin), longitudeBegin(newLongitudeBegin),
          latitudeEnd(newLatitudeEnd), longitudeEnd(newLongitudeEnd),
          type(newType), isActive(active)   { }

    double latitudeBegin;   //!< широта первой точки маршрута, градусы
    double longitudeBegin;  //!< долгота первой точки маршрута, градусы
    double latitudeEnd;     //!< широта последней точки маршрута, градусы
    double longitudeEnd;    //!< долгота последней точки маршрута, градусы
    int type;               //!< тип линии для отображения на карте
    bool isActive;          //!< true - текущая линия ведения по маршруту
};

struct FlightRoute
{
    std::string name;           //!< наименование маршрута
    std::string creationDate;   //!< дата создания маршрута (NOTE можно заменить на QDate)
    std::vector <RoutePoint> points;    //!< отображаемые точки маршрута
    std::vector <RouteLine> lines;      //!< отображаемые линии маршрута
    bool isLoop {false};         //!< true - маршрут зациклен

    bool isEmpty() const { return points.empty(); }
};

}
#endif // STRUCTSFLIGHTROUTE_H
