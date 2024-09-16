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

    IdPair result = adapter.savePlan(plan);

    emit signalSavePlan(result.first);
}

void Controller::deletePlan(uint32_t id)
{
    ASYNC_INVOKE(deletePlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.deletePlan(id);

    emit signalDeletePlan(status);

    if(status == CommandStatus::OK && id == adapter.getActivePlan().id)
    {
        activatePlan(-1);
    }
}

void Controller::invertPlan(uint32_t id)
{
    ASYNC_INVOKE(invertPlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.invertPlan(id);

    emit signalInvertPlan(status);

    if(status == CommandStatus::OK && id == adapter.getActivePlan().id)
    {
        activatePlan(id);
    }
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

    // если id != -1 (точка не новая) и если есть в активном плане,
    // то необходима активация плана при успешном статусе сохранения точки
}

void Controller::deleteWaypoint(uint32_t id)
{
    ASYNC_INVOKE(deleteWaypoint, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.deleteWaypoint(id);

    emit signalDeleteWaypointFromBase(status);

    if(status == CommandStatus::OK && adapter.pointInActivePlan(id))
    {
        activatePlan(adapter.getActivePlan().id);
    }
}

void Controller::getCatalogInfoOfPlans()
{
    ASYNC_INVOKE(getCatalogInfoOfPlans)

    FlightPlanInfoPair planInfo = adapter.getCatalogInfoOfPlans();

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

    adapter.setStateEditPlan(true);
    CommandStatus status{CommandStatus::OK};

    if(id == -1)    //!< создать план [3]
    {
        FlightPlan newPlan{};
        newPlan.id = id;
        adapter.setEditablePlan(newPlan);
    }
    else            //!> изменить план [3]
    {
        FlightPlanPair res = adapter.getPlan(id);
        if(res.first == CommandStatus::OK)
        {
            if(res.second.waypoints.empty())
            {
                FlightPlan newPlan{};
                newPlan.id = -1;
                adapter.setEditablePlan(newPlan);
            }
            else
                adapter.setEditablePlan(res.second);
        }
        status = res.first;
    }

    emit signalStartEditPlan(status);
}

void Controller::stopEditPlan(bool save, bool activate)
{
    ASYNC_INVOKE(stopEditPlan, Q_ARG(bool, save), Q_ARG(bool, activate))

    adapter.setStateEditPlan(false);
    CommandStatus saveStatus;
    if(save)
    {
        FlightPlan savePlan = adapter.getEditablePlan();
        IdPair result = adapter.savePlan(savePlan);

        int idActivePlan = adapter.getActivePlan().id;
        int idSavedPlan  = result.second;
        CommandStatus saveStatus = result.first;

        if(saveStatus == CommandStatus::OK)
        {
            if(activate || idActivePlan == idSavedPlan)
            {
                idSavedPlan = savePlan.waypoints.empty() ? -1 : idSavedPlan;
                CommandStatus activateStatus = adapter.activatePlan(idSavedPlan);

                emit signalActivatePlan(activateStatus);
            }
        }
        emit signalStopEditPlan(saveStatus);
    }
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

    WaypointPair result = adapter.getWaypointById(id);

    if(result.first == CommandStatus::OK)
    {
        CommandStatus status = adapter.addWaypointToEditPlan(pos, result.second);

        emit signalAddWaypoint(status);
    }
    else
        emit signalAddWaypoint(result.first);
}

void Controller::deleteWaypointFromEditPlan(uint32_t pos)
{
    ASYNC_INVOKE(deleteWaypointFromEditPlan, Q_ARG(uint32_t, pos))

    CommandStatus status = adapter.deleteWaypointFromEditPlan(pos);

    emit signalDeleteWaypont(status);
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







