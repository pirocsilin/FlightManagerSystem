#include "controller.h"

#define ASYNC_INVOKE(methodName, ...)                                                      \
    if(QThread::currentThreadId() != selfThread) {                                         \
        QMetaObject::invokeMethod(this, #methodName, Qt::QueuedConnection, ##__VA_ARGS__); \
        return;                                                                            \
    }

ControllerFlightPlan::ControllerFlightPlan()
{
    connect(this, &ControllerFlightPlan::started, this, &ControllerFlightPlan::init);
    connect(this, &QThread::finished, this, &QThread::deleteLater);
    this->moveToThread(this);
    this->start();
}

void ControllerFlightPlan::init()
{
    selfThread = QThread::currentThreadId();
    connect(adapter.connectorPtr(), &Connector::signalUpdateDataBase,
            this, &ControllerFlightPlan::setStatusUpdateDataBase);
}

void ControllerFlightPlan::getPlan(uint32_t id)
{
    ASYNC_INVOKE(getPlan, Q_ARG(uint32_t, id))

    FlightPlanPair result = adapter.getPlan(id);

    emit signalGetPlan(result);
}

void ControllerFlightPlan::deletePlan(uint32_t id)
{
    ASYNC_INVOKE(deletePlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.deletePlan(id);

    emit signalDeletePlan(status);

    if(status == CommandStatus::OK && id == adapter.getActivePlan().id)
    {
        activatePlan(-1);
    }
}

void ControllerFlightPlan::invertPlan(uint32_t id)
{
    ASYNC_INVOKE(invertPlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.invertPlan(id);

    emit signalInvertPlan(status);

    if(status == CommandStatus::OK && id == adapter.getActivePlan().id)
    {
        activatePlan(id);
    }
}

void ControllerFlightPlan::getWaypointByIcao(std::string icao)
{
    ASYNC_INVOKE(getWaypointByIcao, Q_ARG(std::string, icao))

    WaypointVectorPair result = adapter.getWaypointByIcao(icao);

    emit signalGetWaypointByIcao(result);
}

void ControllerFlightPlan::getWaypointById(uint32_t id)
{
    ASYNC_INVOKE(getWaypointById, Q_ARG(uint32_t, id))

    WaypointPair result = adapter.getWaypointById(id);

    emit signalGetWaypointById(result);
}

void ControllerFlightPlan::deleteWaypoint(uint32_t id)
{
    ASYNC_INVOKE(deleteWaypoint, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.deleteWaypoint(id);

    emit signalDeleteWaypointFromBase(status);

    if(status == CommandStatus::OK && adapter.pointInActivePlan(id))
    {
        activatePlan(adapter.getActivePlan().id);
    }
}

void ControllerFlightPlan::startEditPoint(int32_t id)
{
    ASYNC_INVOKE(startEditPoint, Q_ARG(int32_t, id))

    CommandStatus status{CommandStatus::OK};

    if(id == EMPTY_ID)    //!< создать ППМ [4]
    {
        Waypoint newPoint{};
        newPoint.id = EMPTY_ID;
        adapter.setEditPoint(newPoint);
    }
    else                    //!< изменить ППМ [4]
    {
        WaypointPair res = adapter.getWaypointById(id);
        if(res.first != CommandStatus::OK)
        {
            Waypoint newPoint{};
            newPoint.id = EMPTY_ID;
            adapter.setEditPoint(newPoint);
        }
        else
            adapter.setEditPoint(res.second);

        status = res.first;
    }

    auto result = std::make_pair(status, adapter.getEditablePoint());

    emit signalStartEditPoint(result);
}

void ControllerFlightPlan::stopEditPoint(bool save)
{
    ASYNC_INVOKE(stopEditPoint, Q_ARG(bool, save))

    if(save)
    {
        Waypoint savePoint = adapter.getEditablePoint();
        IdPair result = adapter.saveWaypoint(savePoint);

        if(result.first == CommandStatus::OK)
        {
            savePoint.id = result.second;
            emit signalStopEditPoint(std::make_pair(CommandStatus::OK, savePoint));
        }
        else
            emit signalStopEditPoint(std::make_pair(result.first, Waypoint()));
    }
    else
        emit signalStopEditPoint(std::make_pair(CommandStatus::INVALID, Waypoint()));
}

void ControllerFlightPlan::setLatitudeForWaypoint(float latitude)
{
    ASYNC_INVOKE(setLatitudeForWaypoint, Q_ARG(float, latitude))

    adapter.getEditablePoint().latitude = latitude;

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setLongitudeForWaypoint(float longitude)
{
    ASYNC_INVOKE(setLongitudeForWaypoint, Q_ARG(float, longitude))

    adapter.getEditablePoint().longitude = longitude;

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setIcaoForWaypoint(std::string icao)
{
    ASYNC_INVOKE(setIcaoForWaypoint, Q_ARG(std::string, icao))

    WaypointVectorPair res = adapter.getWaypointByIcao(icao);

    if(res.first == CommandStatus::OK)
    {
        adapter.getEditablePoint() = res.second[0];
    }
    else
    {
        adapter.getEditablePoint()      = {};
        adapter.getEditablePoint().id   = EMPTY_ID;
        adapter.getEditablePoint().icao = icao;
    }
    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setRegionForWaypoint(std::string region)
{
    ASYNC_INVOKE(setRegionForWaypoint, Q_ARG(std::string, region))

    adapter.getEditablePoint().region = region;

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setUtcForWaypoint(int32_t utc)
{
    ASYNC_INVOKE(setUtcForWaypoint, Q_ARG(int32_t, utc))

    // coming soon

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setTypeForWaypoint(WaypointType type)
{
    ASYNC_INVOKE(setTypeForWaypoint, Q_ARG(WaypointType, type))

    adapter.getEditablePoint().type = type;

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setFrequencyForWaypoint(double frequency)
{
    ASYNC_INVOKE(setFrequencyForWaypoint, Q_ARG(double, frequency))

    adapter.getEditablePoint().radioFrequency = frequency * 1000000;

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setAltitudeForWaypoint(int32_t altitude)
{
    ASYNC_INVOKE(setAltitudeForWaypoint, Q_ARG(int32_t, altitude))

    adapter.getEditablePoint().altitude = altitude;

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setSchemeForWaypoint(std::string scheme)
{
    ASYNC_INVOKE(setSchemeForWaypoint, Q_ARG(std::string, scheme))

    // coming soon

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::setVppForWaypoint(std::string vpp)
{
    ASYNC_INVOKE(setVppForWaypoint, Q_ARG(std::string, vpp))

    // coming soon

    emit signalGetEditableWaypoint(std::make_pair(CommandStatus::OK, adapter.getEditablePoint()));
}

void ControllerFlightPlan::getCatalogInfoOfPlans()
{
    ASYNC_INVOKE(getCatalogInfoOfPlans)

    FlightPlanInfoPair planInfo = adapter.getCatalogInfoOfPlans();

    emit signalGetCatalogInfoOfPLans(planInfo);
}

void ControllerFlightPlan::getPlanRouteInfo(uint32_t id)
{
    ASYNC_INVOKE(getPlanRouteInfo, Q_ARG(uint32_t, id))

    FlightPlanRouteInfoPair flightPlanRouteInfo = adapter.getPlanRouteInfo(id);

    emit signalGetPlanRouteInfo(flightPlanRouteInfo);
}

void ControllerFlightPlan::activatePlan(uint32_t id)
{
    ASYNC_INVOKE(activatePlan, Q_ARG(uint32_t, id))

    CommandStatus status = adapter.activatePlan(id);

    emit signalActivatePlan(status);
}

void ControllerFlightPlan::startEditPlan(int32_t id)
{
    ASYNC_INVOKE(startEditPlan, Q_ARG(int32_t, id))

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

    adapter.createRouteInfo(adapter.getEditablePlan());
    auto result = std::make_pair(status, adapter.getEditablePlanInfo());

    emit signalStartEditPlan(result);
}

void ControllerFlightPlan::stopEditPlan(bool save, bool activate)
{
    ASYNC_INVOKE(stopEditPlan, Q_ARG(bool, save), Q_ARG(bool, activate))

    adapter.setStateEditPlan(false);
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

void ControllerFlightPlan::getNearestWaypoints(float dist)
{
    ASYNC_INVOKE(getNearestWaypoints, Q_ARG(float, dist))

    WaypointVectorPair result = adapter.getNearestWaypoints(dist);

    emit signalGetNearestWaypoints(result);
}

void ControllerFlightPlan::addWaypointToEditPlan(uint32_t pos, uint32_t id)
{
    ASYNC_INVOKE(addWaypointToEditPlan, Q_ARG(uint32_t, pos), Q_ARG(uint32_t, id))

    WaypointPair result = adapter.getWaypointById(id);

    if(result.first == CommandStatus::OK)
    {
        CommandStatus status = adapter.addWaypointToEditPlan(pos, result.second);
        if(status == CommandStatus::OK)
            adapter.createRouteInfo(adapter.getEditablePlan());

        emit signalAddWaypoint(std::make_pair(status, adapter.getEditablePlanInfo()));
    }
    else
        emit signalAddWaypoint(std::make_pair(result.first, adapter.getEditablePlanInfo()));
}

void ControllerFlightPlan::deleteWaypointFromEditPlan(uint32_t pos)
{
    ASYNC_INVOKE(deleteWaypointFromEditPlan, Q_ARG(uint32_t, pos))

    CommandStatus status = adapter.deleteWaypointFromEditPlan(pos);

    if(status == CommandStatus::OK)
        adapter.createRouteInfo(adapter.getEditablePlan());

    emit signalDeleteWaypont(std::make_pair(status, adapter.getEditablePlanInfo()));
}

void ControllerFlightPlan::getActivePlanInfo()
{
    adapter.getActivePlanInfo();
}

void ControllerFlightPlan::setDeviceFlightData(const DeviceFlightData &data)
{
    adapter.setDeviceFlightData(data);
}

void ControllerFlightPlan::setStatusUpdateDataBase(bool state)
{
    if(state)
        emit signalStartUpdateDataBase();
    else
        emit signalStopUpdateDataBase();
}

void ControllerFlightPlan::selectNextPoint(bool direction)
{
    adapter.selectNextPoint(direction);
}







