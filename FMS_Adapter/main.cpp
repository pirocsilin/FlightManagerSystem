
#include <QCoreApplication>
#include <iostream>
#include <qmath.h>
#include <stdlib.h>
#include "controller.h"

void testSelectNextPoint(ControllerFlightPlan &controller);
void Connect(ControllerFlightPlan&);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ControllerFlightPlan controller;
    QThread::msleep(20);

    qDebug() << __PRETTY_FUNCTION__ << "start main";

    Connect(controller);
    testSelectNextPoint(controller);

    qDebug() << __PRETTY_FUNCTION__ << "end main";

    return a.exec();
}

void Connect(ControllerFlightPlan &controller)
{
    QObject::connect(&controller, &ControllerFlightPlan::signalGetCatalogInfoOfPLans, [&](const FlightPlanInfoPair &info){printCatalogInfoOfPlans(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalGetPlan, [](const FlightPlanPair &info){printPlanInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalGetPlanRouteInfo, [](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalActivatePlan, [](const CommandStatus &info){printCommandStatus(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalGetNearestWaypoints, [](const WaypointVectorPair &info){printWaypointVectorInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalDeletePlan, [](const CommandStatus &info){printCommandStatus(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalInvertPlan, [](const CommandStatus &info){printCommandStatus(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalGetWaypointByIcao, [](const WaypointVectorPair &info){printWaypointVectorInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalGetWaypointById, [](const WaypointPair &info){printWaypointInfo(info);});
    //
    QObject::connect(&controller, &ControllerFlightPlan::signalStartEditPoint, [](const WaypointPair &info){printEditWaypointInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalStopEditPoint,  [](const WaypointPair &info){printEditWaypointInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalGetEditableWaypoint,  [](const WaypointPair &info){printEditWaypointInfo(info);});
    //
    QObject::connect(&controller, &ControllerFlightPlan::signalDeleteWaypointFromBase, [](const CommandStatus &info){printCommandStatus(info);});
    //
    QObject::connect(&controller, &ControllerFlightPlan::signalStartEditPlan, [&](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalAddWaypoint,   [&](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    QObject::connect(&controller, &ControllerFlightPlan::signalDeleteWaypont, [&](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    //
    QObject::connect(controller.adapterPrt()->actPlanMngrPtr(), &ActivePlanManager::signalGetActivePlanInfo, [](const ActivePlanInfoPair &info){printActivePlanInfo(info);});
    QObject::connect(controller.adapterPrt()->actPlanMngrPtr(), &ActivePlanManager::signalSelectNextPoint, [&](){controller.getActivePlanInfo();});
    QObject::connect(controller.adapterPrt()->actPlanMngrPtr(), &ActivePlanManager::signalNavDataFms, [&](const NavDataFms &info){printNavDataFms(info);});
}

void testSelectNextPoint(ControllerFlightPlan &controller)
{
    controller.activatePlan(1);
    controller.getCatalogInfoOfPlans();

    getchar();

    QVector<QPair<float, float>> currPos {QPair(56.900711, 60.396275),
                                          QPair(56.982901, 61.982901),
                                          QPair(56.770178, 60.7253),
                                          QPair(58.770414, 56.770414)};

    QVector<QPair<float, float>> targetPos {QPair(56.907711, 60.403275), // 4
                                           QPair(56.990901, 61.990901),  // 6
                                           QPair(56.776178, 60.7313),    // 2
                                           QPair(58.779414, 56.779414)}; // 3

    int index{0};
    float dist{}, curLat{currPos[index].first}, curLon{currPos[index].second},
                  targetLat{targetPos[index].first}, targetLon{targetPos[index].second};

    for(float i{0.001}; i<1.0; i+=0.001)
    {
        if(i >= 0.01)
        {
            i=0.001;
            if(++index > 3)
            {
                for(int j{0}; j<3; j++)
                {
                    controller.selectNextPoint(false);
                    getchar();
                }
                index=0;
            }
            curLat = currPos[index].first;
            curLon = currPos[index].second;
            targetLat = targetPos[index].first;
            targetLon = targetPos[index].second;
        }

        calcDistAndTrackBetweenWaypoints(curLat+i, curLon+i, targetLat, targetLon, &dist);

        NavDataFms navData{};
        controller.setDeviceFlightData(DeviceFlightData{curLat+i, curLon+i, 32, 22});

        qDebug() << QString("[%1] lat: %2, log: %3 -> target: %4, %5 | dist: %6")
                    .arg(index)
                    .arg(curLat+i)
                    .arg(curLon+i)
                    .arg(targetLat)
                    .arg(targetLon)
                    .arg(dist*EARTH_RADIUS);

        getchar();
    }
}

# if 0
#include "main.moc"
#endif
