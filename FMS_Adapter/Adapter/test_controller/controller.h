
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
    void getPlan(uint32_t id);              //!< получить план из базы
    void savePlan(FlightPlan plan);         //!< сохранить план в базу [id = -1, сохраняем новый план]
    void deletePlan(uint32_t id);           //!< удалить план из базы
    void getWaypoint(uint32_t id);          //!< получить ППМ из базы
    void saveWaypoint(Waypoint point);      //!< сохранить ППМ в базу  [id = -1, сохраняем новую ППМ]
    void getCatalogInfoOfPlans();
    void getPlanRouteInfo(uint32_t id);
    //
    void activatePlan(uint32_t planId);             //!< активация плана по id
    void startEditPlan(uint32_t id);                //!< старт редактирования плана [id = -1, создаем новый план]
    void endEditPlan(bool safe, bool activate);     //!< стоп  редактирования плана
    //
    void addWaypointToEditPlan(uint32_t pos, uint32_t id);  //!< вставить точку в редактируемый план
    void deleteWaypointFromEditPlan(uint32_t pos);          //!< удалить точку из редактируемого плана
    //
    // synch
    std::pair<fp::CommandStatus, fp::ActivePlanInfo> getActivePlanInfo();
    fp::NavDataFms& setDeviceFlightData(const fp::DeviceFlightData& data);
    //
    bool selectNextPoint(bool direction);           //!< установка активной точки, true - next, false - back
    //
    //fp::NavDataFms setDeviceFlightData(const fp::DeviceFlightData& data);

};
