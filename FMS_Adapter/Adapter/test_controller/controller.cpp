﻿#include "controller.h"


Controller::Controller()
{
    connect(this, &Controller::started, this, &Controller::init);
    connect(this, &QThread::finished, this, &QThread::deleteLater);
    this->moveToThread(this);
    this->start();
}

void Controller::init()
{
    selfThread = QThread::currentThreadId();
}

void Controller::getPlan(uint32_t id)
{
    ASYNC_INVOKE(getPlan, Q_ARG(uint32_t, id))

    std::pair<fp::CommandStatus, fp::FlightPlan> flightPlan = adapter.getPlan(id);

    // emit ...
    printPlanInfo(flightPlan);
}

void Controller::savePlan(FlightPlan plan)
{
    //ASYNC_INVOKE(savePlan, Q_ARG(FlightPlan, plan))

    qDebug() << QThread::currentThreadId() << " : " << selfThread;

    if(QThread::currentThreadId() != selfThread)
    {
        QMetaObject::invokeMethod(this, "savePlan", Qt::QueuedConnection, Q_ARG(FlightPlan, plan));
        return;
    }


    CommandStatus status = adapter.savePlan(plan);

    // emit ...
}

void Controller::getWaypoint(uint32_t id)
{
    ASYNC_INVOKE(getWaypoint, Q_ARG(uint32_t, id))

    std::pair<fp::CommandStatus, fp::Waypoint> waypoint = adapter.getWaypoint(id);

    // emit ...
    printWaypointInfo(waypoint);
}

void Controller::saveWaypoint(Waypoint point)
{
    ASYNC_INVOKE(saveWaypoint, Q_ARG(Waypoint, point))

    CommandStatus status = adapter.saveWaypoint(point);

    // emit ...
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
    ASYNC_INVOKE(activatePlan, Q_ARG(uint32_t, id))

    std::pair<CommandStatus, FlightPlan> res = adapter.activatePlan(id);

    // emit signalActivatePlan(res)

    qDebug() << "Status: " << (int)res.first;
}

void Controller::startEditPlan(uint32_t id)
{
    ASYNC_INVOKE(startEditPlan, Q_ARG(uint32_t, id))

    std::pair<fp::CommandStatus, fp::FlightPlan> res = adapter.getPlan(id);

    if(res.first == CommandStatus::OK)
    {
        adapter.setEditablePlan(res.second);
    }
    else
    {
        FlightPlan empty{};
        adapter.setEditablePlan(empty);
    }
    // emit CommandStatus
}

void Controller::endEditPlan(bool safe, bool activate)
{
    ASYNC_INVOKE(endEditPlan, Q_ARG(bool, safe), Q_ARG(bool, activate))

    FlightPlan plan = adapter.getEditablePlan();

    if(safe)
    {
        // сохранить план в базу

        if(activate)
        {
            std::pair<CommandStatus, FlightPlan> res = adapter.activatePlan(plan.id);   // i'm here

            // emit signalActivatePlan(res)
        }
    }
    //! else nothing
}

void Controller::addWaypointToEditPlan(uint32_t pos, uint32_t id)
{
    ASYNC_INVOKE(addWaypointToEditPlan, Q_ARG(uint32_t, pos), Q_ARG(uint32_t, id))

    std::pair<fp::CommandStatus, fp::Waypoint> waypoint = adapter.getWaypoint(id);

    if(waypoint.first == CommandStatus::OK)
    {
        adapter.addWaypointToEditPlan(pos, waypoint.second);
    }

    // emit signalRedrawScreen
}

void Controller::deleteWaypointFromEditPlan(uint32_t pos)
{
    ASYNC_INVOKE(deleteWaypointFromEditPlan, Q_ARG(uint32_t, pos))

    adapter.deleteWaypointFromEditPlan(pos);

    // emit signalRedrawScreen
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

// synch
bool Controller::selectNextPoint(bool direction)
{
    return adapter.selectNextPoint(direction);
}







