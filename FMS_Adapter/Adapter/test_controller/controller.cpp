#include "controller.h"


Controller::Controller()
{
    connect(this, &Controller::started, this, &Controller::init);
    connect(this, &QThread::finished, this, &QThread::deleteLater);
    this->moveToThread(this);
    this->start();

    qRegisterMetaType<uint32_t>("uint32_t");
}

void Controller::init()
{
    selfThread = QThread::currentThreadId();
}

void Controller::getPlan(uint32_t id)
{
    if(QThread::currentThreadId() != selfThread)
    {
        QMetaObject::invokeMethod(this, "getPlan", Qt::QueuedConnection, Q_ARG(uint32_t, id));
        return;
    }

    std::pair<fp::CommandStatus, fp::FlightPlan> flightPlan = adapter.getPlan(id);

    // emit ...
    printPlanInfo(flightPlan);
}

void Controller::getWaypoint(uint32_t id)
{
    if(QThread::currentThreadId() != selfThread)
    {
        QMetaObject::invokeMethod(this, "getWaypoint", Qt::QueuedConnection, Q_ARG(uint32_t, id));
        return;
    }

    std::pair<fp::CommandStatus, fp::Waypoint> waypoint = adapter.getWaypoint(id);

    // emit ...
    printWaypointInfo(waypoint);
}

void Controller::getCatalogInfoOfPlans()
{
    if(QThread::currentThreadId() != selfThread)
    {
        QMetaObject::invokeMethod(this, "getCatalogInfoOfPlans", Qt::QueuedConnection);
        return;
    }

    std::pair<fp::CommandStatus, std::vector<FlightPlanInfo>> planInfo = adapter.getCatalogInfoOfPlans();

    // emit ...
    printCatalogInfoOfPlans(planInfo);
}

void Controller::getPlanRouteInfo(uint32_t id)
{
    if(QThread::currentThreadId() != selfThread)
    {
        QMetaObject::invokeMethod(this, "getPlanRouteInfo", Qt::QueuedConnection, Q_ARG(uint32_t, id));
        return;
    }

    std::pair<CommandStatus, FlightPlanRouteInfo> flightPlanRouteInfo = adapter.getPlanRouteInfo(id);

    // emit ...
    printFlightPlanRouteInfo(flightPlanRouteInfo);
}

void Controller::activatePlan(uint32_t id)
{
    if(QThread::currentThreadId() != selfThread)
    {
        QMetaObject::invokeMethod(this, "activatePlan", Qt::QueuedConnection, Q_ARG(uint32_t, id));
        return;
    }

    CommandStatus statusRequest = adapter.activatePlan(id);

    // emit ..
    qDebug() << "Status: " << (int)statusRequest;
}

void Controller::deactivatePlan()
{
    if(QThread::currentThreadId() != selfThread)
    {
        QMetaObject::invokeMethod(this, "deactivatePlan", Qt::QueuedConnection);
        return;
    }

    adapter.deactivatePlan();
}

// synch
std::pair<CommandStatus, ActivePlanInfo> Controller::getActivePlanInfo()
{
    return adapter.getActivePlanInfo();
}

// synch
NavDataFms& Controller::setDeviceFlightData(const DeviceFlightData &data)
{
    return adapter.setDeviceFlightData(data);
}







