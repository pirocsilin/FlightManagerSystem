#include "controller.h"

#define ASYNC_INVOKE(methodName, ...)                                                      \
    if(QThread::currentThreadId() != selfThread) {                                         \
        QMetaObject::invokeMethod(this, #methodName, Qt::QueuedConnection, ##__VA_ARGS__); \
        return;                                                                            \
    }

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

    FlightPlanPair result = adapter.getPlan(id);

    emit signalGetPlan(result);
}

void Controller::savePlan(FlightPlan plan)
{
    ASYNC_INVOKE(savePlan, Q_ARG(FlightPlan, plan))

    CommandStatus status = adapter.savePlan(plan);

    emit signalSavePlan(status);
}

void Controller::deletePlan(uint32_t id)
{
    ASYNC_INVOKE(deletePlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.deletePlan(id);

    emit signalDeletePlan(status);
}

void Controller::invertPlan(uint32_t id)
{
    ASYNC_INVOKE(invertPlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.invertPlan(id);

    emit signalInvertPlan(status);
}

void Controller::getWaypointByIcao(std::string icao)
{
    ASYNC_INVOKE(getWaypointByIcao, Q_ARG(std::string, icao))

    WaypointVectorPair result = adapter.getWaypointByIcao(icao);

    emit signalGetWaypointByIcao(result);
}

void Controller::getWaypointById(uint32_t id)
{
    ASYNC_INVOKE(getWaypointById, Q_ARG(uint32_t, id))

    WaypointPair result = adapter.getWaypointById(id);

    emit signalGetWaypointById(result);
}

void Controller::saveWaypoint(Waypoint point)
{
    ASYNC_INVOKE(saveWaypoint, Q_ARG(Waypoint, point))

    CommandStatus status = adapter.saveWaypoint(point);

    emit signalSaveWaypoint(status);
}

void Controller::deleteWaypoint(uint32_t id)
{
    ASYNC_INVOKE(deleteWaypoint, Q_ARG(uint32_t, id))

    CommandStatus status{CommandStatus::INVALID};
    ActivePlanInfoPair res{};

    QMetaObject::invokeMethod(adapter.actPlanMngrPtr(),
                              "getActivePlanInfo",
                              Qt::BlockingQueuedConnection,
                              Q_ARG(ActivePlanInfoPair&, res));

    if(res.first == CommandStatus::OK)
    {
        bool deletedPointInActivePlan {false};
        for(auto point : res.second.waypoints)
            if(point.id == id)
            {
                deletedPointInActivePlan = true;
                break;
            }
        if(!deletedPointInActivePlan)
        {
            status = adapter.deleteWaypoint(id);
        }
    }
    else
        status = adapter.deleteWaypoint(id);

    emit signalDeleteWaypont(status);
}

void Controller::getCatalogInfoOfPlans()
{
    ASYNC_INVOKE(getCatalogInfoOfPlans)

    std::pair<fp::CommandStatus, std::vector<FlightPlanInfo>> planInfo = adapter.getCatalogInfoOfPlans();

    emit signalGetCatalogInfoOfPLans(planInfo);
}

void Controller::getPlanRouteInfo(uint32_t id)
{
    ASYNC_INVOKE(getPlanRouteInfo, Q_ARG(uint32_t, id))

    FlightPlanRouteInfoPair flightPlanRouteInfo = adapter.getPlanRouteInfo(id);

    emit signalGetPlanRouteInfo(flightPlanRouteInfo);
}

void Controller::activatePlan(uint32_t id)
{
    ASYNC_INVOKE(activatePlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.activatePlan(id);

    emit signalActivatePlan(status);
}

void Controller::startEditPlan(uint32_t id)
{
    ASYNC_INVOKE(startEditPlan, Q_ARG(uint32_t, id))

    CommandStatus status = CommandStatus::OK;
    if(id == -1)
    {
        FlightPlan newPlan{};
        newPlan.id = id;
        adapter.setEditablePlan(newPlan);
    }
    else
    {
        FlightPlanPair res = adapter.getPlan(id);
        if(res.first == CommandStatus::OK)
            adapter.setEditablePlan(res.second);

        status = res.first;
    }

    emit signalStartEditPlan(status);
}

void Controller::stopEditPlan(bool safe, bool activate)
{
    ASYNC_INVOKE(stopEditPlan, Q_ARG(bool, safe), Q_ARG(bool, activate))

    FlightPlan plan = adapter.getEditablePlan();

    CommandStatus status{CommandStatus::OK};
    if(safe)
    {
        // сохранить план в базу
        status = adapter.savePlan(plan);

        if(activate && status == CommandStatus::OK)
        {
            CommandStatus status = adapter.activatePlan(plan.id);

            emit signalActivatePlan(status);
        }
    }

    emit signalStopEditPlan(status);
}

void Controller::getNearestWaypoints(float dist)
{
    ASYNC_INVOKE(getNearestWaypoints, Q_ARG(float, dist))

    WaypointVectorPair result = adapter.getNearestWaypoints(dist);

    emit signalGetNearestWaypoints(result);
}

void Controller::addWaypointToEditPlan(uint32_t pos, uint32_t id)
{
    ASYNC_INVOKE(addWaypointToEditPlan, Q_ARG(uint32_t, pos), Q_ARG(uint32_t, id))

    WaypointPair waypoint = adapter.getWaypointById(id);

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

void Controller::getActivePlanInfo()
{
    adapter.getActivePlanInfo();
}

void Controller::setDeviceFlightData(const DeviceFlightData &data)
{
    adapter.setDeviceFlightData(data);
}

void Controller::selectNextPoint(bool direction)
{
    adapter.selectNextPoint(direction);
}







