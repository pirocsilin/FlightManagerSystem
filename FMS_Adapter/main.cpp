
#include <QCoreApplication>
#include "test_controller/controller.h"
#include <iostream>
#include <qmath.h>

void testSelectNextPoint();
void testAnyMethods(Controller&);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Controller controller;
    QThread::msleep(20);

    testAnyMethods(controller);

    return a.exec();
}

void testSelectNextPoint()
{
    ManagerCalculationPlan mngr;
    const double EARTH_RADIUS {6371000.0};

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

        mngr.calcDistAndTrackBetweenWaypoints(qDegreesToRadians(curLat+i),
                                              qDegreesToRadians(curLon+i),
                                              qDegreesToRadians(targetLat),
                                              qDegreesToRadians(targetLon),
                                              &dist);

        NavDataFms navData = controller.setDeviceFlightData(DeviceFlightData{curLat+i, curLon+i, 32, 22});

        qDebug() << QString("[%1] lat: %2, log: %3 -> target: %4, %5 | dist: %6").arg(idx).arg(curLat+i).arg(curLon+i).arg(targetLat).arg(targetLon).arg(dist*EARTH_RADIUS);

        printNavDataFms(navData);

        //controller.addWaypointToActivePlan(1, 600);
        QThread::msleep(20);

//        std::pair<CommandStatus, ActivePlanInfo> plan = controller.getActivePlanInfo();
//        printActivePlanInfo(plan);

        getchar();
    }
}

void testAnyMethods(Controller &controller)
{
    Waypoint p1{35, "" , "GPT", (fp::WaypointType)2, 55, 66, 900, 150, 5};

    controller.saveWaypoint(p1);


    //    controller.getPlan(1);
    //    controller.getCatalogInfoOfPlans();
    //    controller.getPlanRouteInfo(9);
    //    controller.getWaypoint(16);
    //    controller.getWaypoint(15);
    //    controller.getWaypoint(14);
    //    controller.getWaypoint(13);

    //    adapter.getCatalogInfoOfPlans();
    //    adapter.getPlanRouteInfo(3);
    //    adapter.getWaypoint(16);
    //    adapter.activatePlan(10);
    //    adapter.getActivePlanInfo();
}

# if 0
#include "main.moc"
#endif
