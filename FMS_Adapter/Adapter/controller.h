
#include "adapter.h"

class ControllerFlightPlan : public QThread
{
    Q_OBJECT

    AdapterFMS adapter;
    Qt::HANDLE selfThread;

public:

    ControllerFlightPlan();
    AdapterFMS* adapterPrt() { return &adapter; }

private slots:

    void init();

public slots:

    void getPlan(uint32_t id);                  // получить план полета из базы по id
    void deletePlan(uint32_t id);               // удалить план из базы по id
    void invertPlan(uint32_t);                  // инвертировать план
    //
    void startEditPlan(int32_t id);                         // старт редактирования плана [if id = -1, создаем новый план]
    void stopEditPlan(bool save, bool activate);            // стоп  редактирования плана
    void addWaypointToEditPlan(uint32_t pos, uint32_t id);  // вставить точку в редактируемый план
    void deleteWaypointFromEditPlan(uint32_t pos);          // удалить точку из редактируемого плана
    //
    void getNearestWaypoints(float dist);       // получить ближайшие 20 точек не дальше чем dist [m]
    void getWaypointByIcao  (std::string name); // получить ППМ из базы по ICAO
    void getWaypointById    (uint32_t id);      // получить ППМ из базы по id
    void deleteWaypoint     (uint32_t id);      // удалить ППМ из базы
    //
    void startEditPoint         (int32_t id);   // старт редактирвания ППМ [if id = -1, создаем новую]
    void stopEditPoint          (bool save);    // стоп редактирования ППМ
    void setLatitudeForWaypoint (float);        // установить широту для ППМ
    void setLongitudeForWaypoint(float);        // установить долготу для ППМ
    void setIcaoForWaypoint     (std::string);  // установить ИКАО для ППМ
    void setRegionForWaypoint   (std::string);  // установить регион для ППМ
    void setUtcForWaypoint      (int32_t);      // установить UTC для ППМ
    void setTypeForWaypoint     (WaypointType); // установить тип ППМ
    void setFrequencyForWaypoint(double);       // установить частоту радиостанции для ППМ
    void setAltitudeForWaypoint (int32_t);      // установить высоту для ППМ
    void setSchemeForWaypoint   (std::string);  // установить схему для ППМ
    void setVppForWaypoint      (std::string);  // установить ВПП для ППМ
    //
    void getCatalogInfoOfPlans();               // получение данных для каталога планов
    void getPlanRouteInfo(uint32_t id);         // получение данных о плане со списком точек
    //
    void activatePlan(uint32_t planId);         // активация плана по id
    void selectNextPoint(bool direction);       // изменить активную точку: true - next, false - back
    //
    void getActivePlanInfo();                                   // получить активный план сигналом
    void setDeviceFlightData(const fp::DeviceFlightData& data); //!< передача пилотажных данных и возврат сигналом параметров полета по активному плану
    //
    void setStatusUpdateDataBase(bool);
signals:

    void signalGetPlan(FlightPlanPair);
    void signalDeletePlan(CommandStatus);
    void signalInvertPlan(CommandStatus);
    void signalStartEditPlan(FlightPlanRouteInfoPair);
    void signalStopEditPlan(CommandStatus);
    //
    void signalStartEditPoint(WaypointPair);
    void signalStopEditPoint(WaypointPair);
    void signalGetNearestWaypoints(WaypointVectorPair);
    void signalGetEditableWaypoint(WaypointPair);
    void signalGetWaypointByIcao(WaypointVectorPair);
    void signalGetWaypointById(WaypointPair);
    void signalDeleteWaypont(FlightPlanRouteInfoPair);
    void signalDeleteWaypointFromBase(CommandStatus);
    void signalAddWaypoint(FlightPlanRouteInfoPair);
    //
    void signalGetCatalogInfoOfPLans(FlightPlanInfoPair);
    void signalGetPlanRouteInfo(FlightPlanRouteInfoPair);
    void signalActivatePlan(CommandStatus);
    //
    void signalStartUpdateDataBase();
    void signalStopUpdateDataBase();
};
