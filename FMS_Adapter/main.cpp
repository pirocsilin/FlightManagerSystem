﻿
#include <QCoreApplication>
#include "controller.h"
#include <iostream>
#include <qmath.h>
#include <stdlib.h>

void testSelectNextPoint();
void testAnyMethods(Controller&);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Controller controller;
    QThread::msleep(20);

    qDebug() << __PRETTY_FUNCTION__ << "start main";
    testAnyMethods(controller);
    qDebug() << __PRETTY_FUNCTION__ << "end main";

    return a.exec();
}

void testAnyMethods(Controller &controller)
{
    QObject::connect(&controller, &Controller::signalGetCatalogInfoOfPLans, [&](const FlightPlanInfoPair &info){printCatalogInfoOfPlans(info);});
    QObject::connect(&controller, &Controller::signalGetPlan, [](const FlightPlanPair &info){printPlanInfo(info);});
    QObject::connect(&controller, &Controller::signalGetPlanRouteInfo, [](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    QObject::connect(&controller, &Controller::signalActivatePlan, [](const CommandStatus &info){printCommandStatus(info);});
    QObject::connect(&controller, &Controller::signalGetNearestWaypoints, [](const WaypointVectorPair &info){printWaypointVectorInfo(info);});
    QObject::connect(&controller, &Controller::signalDeletePlan, [](const CommandStatus &info){printCommandStatus(info);});
    QObject::connect(&controller, &Controller::signalInvertPlan, [](const CommandStatus &info){printCommandStatus(info);});
    QObject::connect(&controller, &Controller::signalGetWaypointByIcao, [](const WaypointVectorPair &info){printWaypointVectorInfo(info);});
    QObject::connect(&controller, &Controller::signalGetWaypointById, [](const WaypointPair &info){printWaypointInfo(info);});
    //
    QObject::connect(&controller, &Controller::signalStartEditPoint, [](const WaypointPair &info){printEditWaypointInfo(info);});
    QObject::connect(&controller, &Controller::signalStopEditPoint,  [](const WaypointPair &info){printEditWaypointInfo(info);});
    QObject::connect(&controller, &Controller::signalGetEditableWaypoint,  [](const WaypointPair &info){printEditWaypointInfo(info);});
    //
    QObject::connect(&controller, &Controller::signalDeleteWaypointFromBase, [](const CommandStatus &info){printCommandStatus(info);});
    //
    QObject::connect(&controller, &Controller::signalStartEditPlan, [&](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    QObject::connect(&controller, &Controller::signalAddWaypoint,   [&](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    QObject::connect(&controller, &Controller::signalDeleteWaypont, [&](const FlightPlanRouteInfoPair &info){printFlightPlanRouteInfo(info);});
    //
    QObject::connect(controller.adapterPrt()->actPlanMngrPtr(), &ActivePlanManager::signalGetActivePlanInfo, [](const ActivePlanInfoPair &info){printActivePlanInfo(info);});
    QObject::connect(controller.adapterPrt()->actPlanMngrPtr(), &ActivePlanManager::signalSelectNextPoint, [&](){controller.getActivePlanInfo();});

    controller.startEditPoint(25);
    getchar();
    controller.setLatitudeForWaypoint(55.55);
    getchar();
    controller.setIcaoForWaypoint("DDT");
    getchar();
    controller.setLatitudeForWaypoint(57.12);
    controller.setLongitudeForWaypoint(60.12);
    controller.setRegionForWaypoint("tagil");
    getchar();
    controller.setIcaoForWaypoint("FT");
    getchar();
    controller.setTypeForWaypoint(WaypointType::VOR);
    controller.setFrequencyForWaypoint(152.32);
    controller.setAltitudeForWaypoint(1080);
    getchar();
    controller.stopEditPoint(true);
    getchar();

//    int point{};
//    bool direct = false;
//    while (true)
//    {
//        if(point++ % 3 == 0)
//        {
//            direct = !direct;
//        }
//        std::getchar();
//        controller.selectNextPoint(direct);
//    }
}

void testSelectNextPoint()
{
    ActivePlanManager mngr;

    Controller controller;
    QThread::msleep(20);
    controller.getPlan(1);
    controller.activatePlan(1);
    QThread::msleep(20);

    QVector<QPair<float, float>> currPos {QPair(56.900711, 60.396275),
                                          QPair(56.982901, 56.982901),
                                          QPair(56.770178, 60.7253),
                                          QPair(56.770414, 56.770414)};

    QVector<QPair<float, float>> nextPos {QPair(56.907711, 60.403275),  // 4
                                          QPair(56.990901, 56.990901),  // 6
                                          QPair(56.776178, 60.7313),    // 2
                                          QPair(56.779414, 56.779414)}; // 3

    float dist{}, curLat{currPos[0].first}, curLon{currPos[0].second},
                  targetLat{nextPos[0].first}, targetLon{nextPos[0].second};

    for(float i{0.001}, idx{0}; i<1.0; i+=0.001)
    {
        if(i * 100 >= 1 && idx < 3){
            i=0.001; idx++;
            curLat = currPos[(int)idx].first;
            curLon = currPos[(int)idx].second;
            targetLat = nextPos[(int)idx].first;
            targetLon = nextPos[(int)idx].second;
        }

        if((int)idx == 3){
            for(int j{0}; j<3; j++)
                controller.selectNextPoint(false);
            i=0.001; idx=0;
            curLat = currPos[0].first;
            curLon = currPos[0].second;
            targetLat = nextPos[0].first;
            targetLon = nextPos[0].second;
        }

//        mngr.calcDistAndTrackBetweenWaypoints(qDegreesToRadians(curLat+i),
//                                              qDegreesToRadians(curLon+i),
//                                              qDegreesToRadians(targetLat),
//                                              qDegreesToRadians(targetLon),
//                                              &dist);

        NavDataFms navData{};
        controller.setDeviceFlightData(DeviceFlightData{curLat+i, curLon+i, 32, 22});

        qDebug() << QString("[%1] lat: %2, log: %3 -> target: %4, %5 | dist: %6").arg(idx).arg(curLat+i).arg(curLon+i).arg(targetLat).arg(targetLon).arg(dist*EARTH_RADIUS);

        printNavDataFms(navData);

        //controller.addWaypointToActivePlan(1, 600);
        QThread::msleep(20);

//        ActivePlanInfoPair plan = controller.getActivePlanInfo();
//        printActivePlanInfo(plan);

        getchar();
    }
}

# if 0
#include "main.moc"
#endif
