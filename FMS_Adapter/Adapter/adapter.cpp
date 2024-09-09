
#include "adapter.h"

AdapterFMS::AdapterFMS()
{
    connector = QSharedPointer<Connector>(new Connector);
    connector->moveToThread(&connectorThread);
    connect(&connectorThread, &QThread::started, connector.data(), &Connector::initConnection);
    connect(&connectorThread, &QThread::finished, connector.data(), &Connector::deleteLater);
    connectorThread.setObjectName("Connector");
    connectorThread.start();

    activePlanManager = QSharedPointer<ActivePlanManager>(new ActivePlanManager);
    activePlanManager->moveToThread(&activePlanManagerThread);
    connect(&activePlanManagerThread, &QThread::finished, activePlanManager.data(), &ActivePlanManager::deleteLater);
    activePlanManagerThread.setObjectName("ActivePlanManager");
    activePlanManagerThread.start();

    qRegisterMetaType<std::string>  ("std::string");
    qRegisterMetaType<uint32_t>     ("uint32_t");
    qRegisterMetaType<bool>         ("bool");
    qRegisterMetaType<Waypoint>     ("Waypoint");
    qRegisterMetaType<FlightPlan>   ("FlightPlan");
    qRegisterMetaType<NavDataFms>   ("NavDataFms");
    qRegisterMetaType<QByteArray*>  ("QByteArray*");
    //
    qRegisterMetaType<DeviceFlightData>     ("DeviceFlightData");
    qRegisterMetaType<std::vector<Waypoint>>("std::vector<Waypoint>");
    qRegisterMetaType<CommandStatus>        ("CommandStatus");
    qRegisterMetaType<ActivePlanInfo>       ("ActivePlanInfo");
    qRegisterMetaType<ActivePlanInfoPair>   ("ActivePlanInfoPair");
    qRegisterMetaType<FlightPlanPair>       ("FlightPlanPair");
    qRegisterMetaType<FlightPlanInfoPair>   ("FlightPlanInfoPair");
    //
    qRegisterMetaType<FlightPlanRouteInfoPair>  ("FlightPlanRouteInfoPair");
    qRegisterMetaType<WaypointVectorPair>       ("WaypointVectorPair");
}

template<typename... Args>
bool AdapterFMS::createRequestAndSend(cmdID id, Args... params)
{
    bool answer {false};
    QByteArray  outputData;
    QDataStream out(&outputData, QIODevice::ReadWrite);
    out.setVersion(QDataStream::Qt_5_3);

    HeaderData sendHdr{HeaderData::Name::ADAPTER_COMMAND, uniqueCmd, id};

    out << qint16(0) << sendHdr << (out << ... << params);

    // device()->seek(0) не работает, возможно из за распакрвки Args...
    QDataStream _out(&outputData, QIODevice::WriteOnly);
    _out.setVersion(QDataStream::Qt_5_3);
    _out << qint16(outputData.size() - sizeof(qint16));

    int countAttempt{3};
    while(countAttempt--)
    {
        QMetaObject::invokeMethod(connector.data(),
                                  "sendDataAndAwaite",
                                  Qt::BlockingQueuedConnection,
                                  Q_ARG(QByteArray*, &outputData));

        HeaderData receivHdr{};
        getHdrFromResponse(connector->inputData, receivHdr);

        if(receivHdr.checkAnswer(sendHdr))
        {
            if(receivHdr.id == cmdID::ERROR_DATABASE)
                answer = false;
            else
                answer = true;
            break;
        }
    }
    uniqueCmd++;

    return answer;
}

FlightPlanPair AdapterFMS::getPlan(uint32_t id)
{
    if(!connector->isTcpConnected)
        return std::make_pair(CommandStatus::NO_CONNECTION, FlightPlan());
    else
        if(!createRequestAndSend(cmdID::GET_PLAN, id))
            return std::make_pair(CommandStatus::ERROR_DATABASE, FlightPlan());
    else{
            FlightPlan plan;
            getDataFromResponse(connector->inputData, plan);
            return std::make_pair(CommandStatus::OK, plan);
        }
}

CommandStatus AdapterFMS::savePlan(FlightPlan &plan)
{
    if(!connector->isTcpConnected)
        return CommandStatus::NO_CONNECTION;
    else
        if(!createRequestAndSend(cmdID::SAVE_PLAN, plan))
            return CommandStatus::ERROR_DATABASE;
    else{
            CommandStatus status;
            getDataFromResponse(connector->inputData, status);
            return status;
        }
}

CommandStatus AdapterFMS::deletePlan(uint32_t id)
{
    if(!connector->isTcpConnected)
        return CommandStatus::NO_CONNECTION;
    else
        if(!createRequestAndSend(cmdID::DELETE_PLAN, id))
            return CommandStatus::ERROR_DATABASE;
    else{
            CommandStatus status;
            getDataFromResponse(connector->inputData, status);
            return status;
        }
}

CommandStatus AdapterFMS::invertPlan(uint32_t id)
{
    if(!connector->isTcpConnected)
        return CommandStatus::NO_CONNECTION;
    else
        if(!createRequestAndSend(cmdID::INVERT_PLAN, id))
            return CommandStatus::ERROR_DATABASE;
    else{
            CommandStatus status;
            getDataFromResponse(connector->inputData, status);
            return status;
        }
}

std::pair<CommandStatus, std::vector<Waypoint> > AdapterFMS::getNearestWaypoints(float dist)
{
    if(!connector->isTcpConnected)
        return std::make_pair(CommandStatus::NO_CONNECTION, std::vector<Waypoint>());

    float curLatitude{}, curLongitude{};
    QMetaObject::invokeMethod(activePlanManager.data(),
                              "getCurrrentPosition",
                              Qt::BlockingQueuedConnection,
                              Q_ARG(float&, curLatitude),
                              Q_ARG(float&, curLongitude));

    if(std::isnan(curLatitude) || std::isnan(curLongitude))
        return std::make_pair(CommandStatus::INVALID, std::vector<Waypoint>());

    if(!createRequestAndSend(cmdID::GET_NEAREST_WAYPOINTS, curLatitude, curLongitude, dist))
        return std::make_pair(CommandStatus::ERROR_DATABASE, std::vector<Waypoint>());
    else
    {
        std::vector<Waypoint> points{};
        getDataFromResponse(connector->inputData, points);
        return std::make_pair(CommandStatus::OK, points);
    }
}

WaypointVectorPair AdapterFMS::getWaypointByIcao(std::string icao)
{
    if(!connector->isTcpConnected)
        return std::make_pair(CommandStatus::NO_CONNECTION, std::vector<Waypoint>());
    else
        if(!createRequestAndSend(cmdID::GET_WAYPOINT_BY_ICAO, icao))
            return std::make_pair(CommandStatus::ERROR_DATABASE, std::vector<Waypoint>());
    else
        {
            std::vector<Waypoint> points{};
            getDataFromResponse(connector->inputData, points);
            if(points.size() > 1)
            {
                QMetaObject::invokeMethod(activePlanManager.data(),
                                          "sortWaypointByDistance",
                                          Qt::BlockingQueuedConnection,
                                          Q_ARG(std::vector<Waypoint>&, points));

            }
            return std::make_pair(CommandStatus::OK, points);
        }
}

std::pair<CommandStatus, std::vector<FlightPlanInfo> > AdapterFMS::getCatalogInfoOfPlans()
{
    if(!connector->isTcpConnected)
        return std::make_pair(CommandStatus::NO_CONNECTION, std::vector<FlightPlanInfo>());
    else
        if(!createRequestAndSend(cmdID::GET_CATALOG_INFO_OF_PLANS))
            return std::make_pair(CommandStatus::ERROR_DATABASE, std::vector<FlightPlanInfo>());
    else{
            std::vector<FlightPlanInfo> planInfo;
            getDataFromResponse(connector->inputData, planInfo);
            return std::make_pair(CommandStatus::OK, planInfo);
        }
}

FlightPlanRouteInfoPair AdapterFMS::getPlanRouteInfo(uint32_t id)
{
    if(!connector->isTcpConnected)
        return std::make_pair(CommandStatus::NO_CONNECTION, FlightPlanRouteInfo());
    else
        if(!createRequestAndSend(cmdID::GET_PLAN_ROUTE_INFO, id))
            return std::make_pair(CommandStatus::ERROR_DATABASE, FlightPlanRouteInfo());
    else
        {
            FlightPlanRouteInfo planRouteInfo;
            getDataFromResponse(connector->inputData, planRouteInfo);
            return std::make_pair(CommandStatus::OK, planRouteInfo);
        }
}

std::pair<CommandStatus, Waypoint> AdapterFMS::getWaypointById(uint32_t id)
{
    if(!connector->isTcpConnected)
        return std::make_pair(CommandStatus::NO_CONNECTION, Waypoint());
    else
        if(!createRequestAndSend(cmdID::GET_WAYPOINT_BY_ID, id))
            return std::make_pair(CommandStatus::ERROR_DATABASE, Waypoint());
    else
        {
            Waypoint point;
            getDataFromResponse(connector->inputData, point);
            return std::make_pair(CommandStatus::OK, point);
        }
}

CommandStatus AdapterFMS::saveWaypoint(Waypoint &point)
{
    if(!connector->isTcpConnected)
        return CommandStatus::NO_CONNECTION;
    else
        if(!createRequestAndSend(cmdID::SAVE_WAYPOINT, point))
            return CommandStatus::ERROR_DATABASE;
    else
        {
            CommandStatus status;
            getDataFromResponse(connector->inputData, status);
            return status;
        }
}

CommandStatus AdapterFMS::deleteWaypoint(uint32_t id)
{
    if(!connector->isTcpConnected)
        return CommandStatus::NO_CONNECTION;
    else
        if(!createRequestAndSend(cmdID::DELETE_WAYPOINT, id))
            return CommandStatus::ERROR_DATABASE;
    else
        {
            CommandStatus status;
            getDataFromResponse(connector->inputData, status);
            return status;
        }
}

CommandStatus AdapterFMS::activatePlan(uint32_t planId)
{
    FlightPlanPair res = getPlan(planId);
    if(res.first != CommandStatus::OK)
        return res.first;

    if(res.second.waypoints.empty())
        return CommandStatus::INVALID;

    QMetaObject::invokeMethod(activePlanManager.data(),
                              "activatePlan",
                              Qt::BlockingQueuedConnection,
                              Q_ARG(FlightPlan&, res.second));

    return CommandStatus::OK;
}

void AdapterFMS::setDeviceFlightData(const DeviceFlightData &data)
{
    QMetaObject::invokeMethod(activePlanManager.data(),
                              "setDeviceFlightData",
                              Qt::QueuedConnection,
                              Q_ARG(DeviceFlightData, data));
}

void AdapterFMS::getActivePlanInfo()
{
    QMetaObject::invokeMethod(activePlanManager.data(),
                              "getActivePlanInfo",
                              Qt::QueuedConnection);
}

void AdapterFMS::selectNextPoint(bool direction)
{
    QMetaObject::invokeMethod(activePlanManager.data(),
                              "selectNextPoint",
                              Qt::QueuedConnection,
                              Q_ARG(bool, direction));
}

void AdapterFMS::addWaypointToEditPlan(uint32_t position, Waypoint &point)
{
    if(beginEditPlan)
    {
        if(position < 0 || position > editablePlan.waypoints.size())
            return;

        auto it = editablePlan.waypoints.begin();
        editablePlan.waypoints.insert(it + position, point);

        createNameForPlan(editablePlan);
    }
}

void AdapterFMS::deleteWaypointFromEditPlan(uint32_t position)
{
    if(beginEditPlan)
    {
        if(position < 0 || position >= editablePlan.waypoints.size())
            return;

        auto it = editablePlan.waypoints.begin();
        editablePlan.waypoints.erase(it + position);

        createNameForPlan(editablePlan);
    }
}

void AdapterFMS::setEditablePlan(FlightPlan &plan)
{
    beginEditPlan = true;
    editablePlan = plan;
}

FlightPlan &AdapterFMS::getEditablePlan()
{
    beginEditPlan = false;
    return editablePlan;
}

