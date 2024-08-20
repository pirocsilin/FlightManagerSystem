#include "managerflightroute.h"

#if 0
#include <iostream>
#include <fstream>
#include <algorithm>
#include <QDebug>
#include <QHash>

using json = nlohmann::json;
using namespace flightroute;

ManagerFlightRoute::ManagerFlightRoute() { }

void ManagerFlightRoute::readRoutesFromFile(std::string routeFilePath)
{
    json readedJsonRoute = parseJsonData(routeFilePath);
    if(readedJsonRoute.empty())
    {
        qDebug() << "[Warning]: No routes in flight_route_sr30.json!!!";
        return;
    }else
    {
        fillRoutesFromJson(readedJsonRoute);
    }
}

FlightRoute ManagerFlightRoute::getCurrentFlightRoute()
{
    return routes.at(activeRouteIndex);
}

FlightRoute ManagerFlightRoute::getFlightRouteByIndex(int index)
{
    if(index < 0 || index >= routes.size())
        return FlightRoute();

    return routes.at(index);
}

/**
 * @brief Установка индекса активного маршрута
 * @param index индекс активируемого маршрута в списке (0-14)
 */
void ManagerFlightRoute::setIndexActiveRoute(int index)
{
    if((index < 0) || (index >= routes.size()))
        return;

    activeRouteIndex = index;
}

nlohmann::json ManagerFlightRoute::parseJsonData(std::string filePath)
{
    json ret;
    std::ifstream inputFile(filePath);
    try {
        ret = json::parse(inputFile);
    } catch (...) {
        qDebug() << "[Error]: Disable to parse file flight_route_sr30.json!!!";
    }
    inputFile.close();
    return ret;
}

/**
 * @brief Преобразование json в список маршрутов
 * @param in маршрут в формате json
 * @return считанный маршрут из файла
 */
void ManagerFlightRoute::fillRoutesFromJson(nlohmann::json &in)
{
    if(in.empty())
        return;

    for(auto & route : in["routes"].items())
    {
        FlightRoute loadedRoute;

        loadedRoute.name = (route.value()["name"].is_string())
                ? route.value()["name"].get<std::string>() : "";

        loadedRoute.creationDate = (route.value()["date"].is_string())
                ? route.value()["date"].get<std::string>() : "01.01.1970";

        for(auto & point : route.value()["points"].items())
        {
            RoutePoint newPoint;

            newPoint.name = (point.value()["name"].is_string())
                    ? point.value()["name"].get<std::string>() : "";

            newPoint.type = (point.value()["type"].is_number_integer())
                    ? static_cast<TypeRoutePoint>(point.value()["type"]) : TypeRoutePoint::TP_NONE;

            newPoint.altitude = (point.value()["altitude"].is_number_integer())
                    ? point.value()["altitude"].get<int>() : 0;

            if(point.value()["coordinates"].is_object())
            {
                if(point.value()["coordinates"]["latitude"].is_number_float())
                    newPoint.latitude  = point.value()["coordinates"]["latitude"];

                if(point.value()["coordinates"]["longitude"].is_number_float())
                    newPoint.longitude = point.value()["coordinates"]["longitude"];
            }
            loadedRoute.points.push_back(newPoint);
        }
        routes.push_back(loadedRoute);
    }
}

std::vector<RouteInfo> ManagerFlightRoute::getRouteInfoList()
{
    std::vector<RouteInfo> ret;

    uint16_t counter {};
    for(auto & route : routes)
    {
        RouteInfo routeInfo;
        routeInfo.name = QString::fromStdString(route.name);
        routeInfo.creationDate = QString::fromStdString(route.creationDate);
        routeInfo.pointCount = route.points.size();
        routeInfo.isActive = ((activeRouteIndex > -1) && (activeRouteIndex == counter));
        ret.push_back(routeInfo);
        counter++;
    }

    return ret;
}

uint32_t ManagerFlightRoute::getRouteChecksum(uint32_t index)
{
    static uint seed {2023};
    if(index >= routes.size())
        return 0;

    uint32_t ret;
    ret =  qHash(QString::fromStdString(routes.at(index).name),         seed);
    ret += qHash(QString::fromStdString(routes.at(index).creationDate), seed);
    ret += routes.at(index).points.size();
    return ret;
}

void ManagerFlightRoute::createRouteLines()
{
    routes.at(activeRouteIndex).lines.clear();
    if(routes.at(activeRouteIndex).points.size() < 2)
        return;

    for(int i = 0; i < routes.at(activeRouteIndex).points.size()-1; ++i)
    {
        RouteLine newLine;
        newLine.latitudeBegin  = routes.at(activeRouteIndex).points.at(i).latitude;
        newLine.longitudeBegin = routes.at(activeRouteIndex).points.at(i).longitude;
        newLine.latitudeEnd    = routes.at(activeRouteIndex).points.at(i+1).latitude;
        newLine.longitudeEnd   = routes.at(activeRouteIndex).points.at(i+1).longitude;
        newLine.type           = 0;
        newLine.isActive       = false;
        routes.at(activeRouteIndex).lines.push_back(newLine);
    }
}
#endif
