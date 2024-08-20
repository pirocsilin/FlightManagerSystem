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
    qRegisterMetaType<NavDataFp>("NavDataFp");
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

NavDataFms& AdapterFMS::setDeviceFlightData(const DeviceFlightData &data)
{
    sharedDataCtrl.lock();
    calculatPlan.setNavData(data);
    sharedDataCtrl.unlock();

    return calculatPlan.navDataFms;
}

CommandStatus AdapterFMS::activatePlan(uint32_t planId)
{
    std::pair<CommandStatus, FlightPlan> response = getPlan(planId);
    if(response.first != CommandStatus::OK)
        return response.first;

    if(response.second.waypoints.empty())
        return CommandStatus::INVALID;

    sharedDataCtrl.lock();
    calculatPlan.setRoute(response.second);
    activePlanIsSet = true;
    sharedDataCtrl.unlock();

    return CommandStatus::OK;
}

void AdapterFMS::deactivatePlan()
{
    sharedDataCtrl.lock();
    calculatPlan.resetRoute();
    activePlanIsSet = false;
    sharedDataCtrl.unlock();
}

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

