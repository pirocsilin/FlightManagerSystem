
#include <QCoreApplication>
#include "test_controller/controller.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Controller controller;

    QThread::msleep(100);

    qDebug() << "START" << __PRETTY_FUNCTION__;

    //controller.setDeviceFlightData(DeviceFlightData{55,66,32,22});

    controller.getPlanRouteInfo(1);
    controller.activatePlan(1);

    controller.getPlanRouteInfo(2);
    controller.activatePlan(2);


    QThread::msleep(100);
    std::pair<CommandStatus, ActivePlanInfo> plan = controller.getActivePlanInfo();
    printActivePlanInfo(plan);

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

    qDebug() << "END" << __PRETTY_FUNCTION__;

    return a.exec();
}

# if 0
#include "main.moc"
#endif
