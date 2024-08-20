
#include "adapter.h"

class Controller : public QThread
{
    Q_OBJECT

public:

    Controller();

    AdapterFMS adapter;
    Qt::HANDLE selfThread;

private slots:

    void init();

public slots:

    // asynch
    void getPlan(uint32_t id);
    void getWaypoint(uint32_t id);
    void getCatalogInfoOfPlans();
    void getPlanRouteInfo(uint32_t id);
    //
    void activatePlan(uint32_t planId);
    void deactivatePlan();
    //
    // synch
    std::pair<fp::CommandStatus, fp::ActivePlanInfo> getActivePlanInfo();
    fp::NavDataFms& setDeviceFlightData(const fp::DeviceFlightData& data);
    //
    //fp::NavDataFms setDeviceFlightData(const fp::DeviceFlightData& data);

};
