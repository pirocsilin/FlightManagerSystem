
#include "adapter.h"

class Controller : public QThread
{
    Q_OBJECT

    AdapterFMS adapter;
    Qt::HANDLE selfThread;

public:

    Controller();
    AdapterFMS* adapterPrt() { return &adapter; }

private slots:

    void init();

public slots:

    void getPlan(uint32_t id);                  //!< получить план полета из базы по id
    void savePlan(FlightPlan plan);             //!< сохранить план в базу [if id = -1, сохраняем новый план]
    void deletePlan(uint32_t id);               //!< удалить план из базы по id
    void invertPlan(uint32_t);                  //!< инвертировать план
    void startEditPlan(uint32_t id);            //!< старт редактирования плана [if id = -1, создаем новый план]
    void stopEditPlan(bool safe, bool activate);//!< стоп  редактирования плана
    //
    void getNearestWaypoints(float dist);       //!< получить ближайшие 20 точек не дальше чем dist [m]
    void getWaypointByIcao(std::string name);   //!< получить ППМ из базы по ICAO
    void getWaypointById(uint32_t id);          //!< получить ППМ из базы по id
    void saveWaypoint(Waypoint point);          //!< сохранить ППМ в базу  [if id = -1, сохраняем новую ППМ]
    void deleteWaypoint(uint32_t id);           //!< удалить ППМ из базы
    //
    void getCatalogInfoOfPlans();               //!< получение данных для каталога планов
    void getPlanRouteInfo(uint32_t id);         //!< получение данных о плане со списком точек
    //
    void activatePlan(uint32_t planId);         //!< активация плана по id
    void selectNextPoint(bool direction);       //!< изменить активную точку: true - next, false - back
    //
    void addWaypointToEditPlan(uint32_t pos, uint32_t id);      //!< вставить точку в редактируемый план
    void deleteWaypointFromEditPlan(uint32_t pos);              //!< удалить точку из редактируемого плана
    //
    void getActivePlanInfo();                                   //!< получить активный план сигналом
    void setDeviceFlightData(const fp::DeviceFlightData& data); //!< передача пилотажных данных и возврат сигналом параметров полета по активному плану

signals:

    void signalGetPlan(FlightPlanPair);
    void signalSavePlan(CommandStatus);
    void signalDeletePlan(CommandStatus);
    void signalInvertPlan(CommandStatus);
    void signalStartEditPlan(CommandStatus);
    void signalStopEditPlan(CommandStatus);
    //
    void signalGetNearestWaypoints(WaypointVectorPair);
    void signalGetWaypointByIcao(WaypointVectorPair);
    void signalGetWaypointById(WaypointPair);
    void signalSaveWaypoint(CommandStatus);
    void signalDeleteWaypont(CommandStatus);
    //
    void signalGetCatalogInfoOfPLans(FlightPlanInfoPair);
    void signalGetPlanRouteInfo(FlightPlanRouteInfoPair);
    void signalActivatePlan(CommandStatus);
};
