#include <QCoreApplication>
#include "adapter.h"

AdapterFMS::AdapterFMS()
{
    connector = QSharedPointer<Connector>(new Connector);
    connector->moveToThread(&connectorThread);
    connect(&connectorThread, &QThread::started, connector.data(), &Connector::initConnection);
    connect(&connectorThread, &QThread::finished, connector.data(), &Connector::deleteLater);
    connectorThread.start();

    qRegisterMetaType<Waypoint>("Waypoint");
    qRegisterMetaType<FlightPlan>("FlightPlan");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<QByteArray*>("QByteArray*");
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
        awaiteReceivingData.lock();
        QMetaObject::invokeMethod(connector.data(), "sendDataAndAwaite", Q_ARG(QByteArray*, &outputData));
        connector->waitCondition.wait(&awaiteReceivingData);
        awaiteReceivingData.unlock();

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

std::pair<CommandStatus, FlightPlan> AdapterFMS::getPlan(uint32_t id)
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

std::pair<CommandStatus, FlightPlanRouteInfo> AdapterFMS::getPlanRouteInfo(uint32_t id)
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

std::pair<CommandStatus, Waypoint> AdapterFMS::getWaypoint(uint32_t id)
{
    if(!connector->isTcpConnected)
        return std::make_pair(CommandStatus::NO_CONNECTION, Waypoint());
    else
        if(!createRequestAndSend(cmdID::GET_WAYPOINT, id))
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

// synch
NavDataFms& AdapterFMS::setDeviceFlightData(const DeviceFlightData &data)
{
    sharedDataCtrl.lock();
    calculatPlan.setNavData(data);
    sharedDataCtrl.unlock();

    return calculatPlan.navDataFms;
}

// asynch
std::pair<CommandStatus, FlightPlan> AdapterFMS::activatePlan(uint32_t planId)
{
    std::pair<CommandStatus, FlightPlan> res = getPlan(planId);
    if(res.first != CommandStatus::OK)
        return std::make_pair(res.first, FlightPlan());

    if(res.second.waypoints.empty())
        return std::make_pair(CommandStatus::INVALID, FlightPlan());

    sharedDataCtrl.lock();
    calculatPlan.setRoute(res.second);
    activePlanIsSet = true;
    sharedDataCtrl.unlock();

    return std::make_pair(CommandStatus::OK, res.second);
}

// synch
std::pair<fp::CommandStatus, fp::ActivePlanInfo> AdapterFMS::getActivePlanInfo()
{
    ActivePlanInfo activePlanInfo{};
    CommandStatus statusCommand {CommandStatus::INVALID};

    sharedDataCtrl.lock();
    if(activePlanIsSet)
    {
        activePlanInfo = calculatPlan.getActivePlanInfo();
        statusCommand = CommandStatus::OK;
    }
    sharedDataCtrl.unlock();

    return std::make_pair(statusCommand, activePlanInfo);
}

// synch
bool AdapterFMS::selectNextPoint(bool direction)
{
    sharedDataCtrl.lock();
    bool status {activePlanIsSet};
    if(status)
        calculatPlan.selectNextPoint(direction);
    sharedDataCtrl.unlock();

    return status;
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

//void AdapterFMS::getNearestWaypoints(double lat, double lon, float dist, fp::WaypointType type)
//{
//    createRequest(cmdID::GET_NEAREST_WAYPOINTS, lat, lon, dist, type);
//}

//void AdapterFMS::getWaypointsFromActivePlan()
//{
//    createRequest(cmdID::GET_WAYPOINTS_FROM_ACTIVE_PLAN);
//}

//void AdapterFMS::getRecentWaypoints()
//{
//    createRequest(cmdID::GET_RECENT_WAYPOINTS);
//}

