#ifndef MANAGERFLIGHTROUTE_H
#define MANAGERFLIGHTROUTE_H

#if 0
#include <QString>
#include <vector>
#include "nlohmann/json.hpp"

#include "flightroute/structsflightroute.h"

struct RouteInfo
{
    QString  name;           //!< наименование маршрута
    QString  creationDate;   //!< дата создания маршрутa
    uint16_t pointCount;     //!< количество точек в маршруте
    bool     isActive;       //!< true - активный маршрут
};

class ManagerFlightRoute
{
public:
    explicit ManagerFlightRoute();
    ~ManagerFlightRoute() = default;

    void readRoutesFromFile(std::string routeFilePath);

    flightroute::FlightRoute getCurrentFlightRoute();
    flightroute::FlightRoute getFlightRouteByIndex(int index);
    void setIndexActiveRoute(int index);
    std::vector<RouteInfo> getRouteInfoList();
    uint32_t getRouteChecksum(uint32_t index);

private:
    int activeRouteIndex {-1};                      //!< индекс активированного маршрута
    std::vector<flightroute::FlightRoute> routes;   //!< список маршрутов

    nlohmann::json parseJsonData(std::string filePath);
    void fillRoutesFromJson(nlohmann::json& in);
    void createRouteLines();
};
#endif

#endif // MANAGERFLIGHTROUTE_H
