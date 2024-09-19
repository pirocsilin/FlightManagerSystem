#ifndef ADAPTER_H
#define ADAPTER_H

#include <QPair>
#include <QThread>
#include <QWaitCondition>
#include <QTcpSocket>
#include <QTimer>
#include <QTime>
#include <QEventLoop>
#include <QHostAddress>
#include <utility>
#include <cmath>
#include "common/structsflightplan.h"
#include "active_plan/structs_data.h"
#include "common/labsflightplan.h"
#include "active_plan/mathplan.h"
#include "connect/connector.h"

using namespace fp;

class AdapterFMS : public QObject
{
    Q_OBJECT

private:

    uint32_t uniqueCmd {10};                                //!< уникальный id запрса к FMS

    QSharedPointer<Connector> connector;                    //!< коммуникация с FMS
    QThread connectorThread;

    QSharedPointer<ActivePlanManager> activePlanManager;    //!< манагер рассчета параметров активного плана
    QThread activePlanManagerThread;

    bool beginEditPlan {false};                             //!< признак редактирования плана полета
    bool beginEditPoint{false};                             //!< признак редактирования ППМ

    Waypoint  editablePoint   {};                           //!< редактируемая точка
    FlightPlan editablePlan   {};                           //!< редактируемый план полета
    FlightPlanRouteInfo editablePlanInfo{};                 //!< отображаемый редактируемый план полета
    //
    FlightPlan activePlan     {};                           //!< активный план полета

    template<typename... Args>
    bool createRequestAndSend(cmdID id, Args... args);      //!< создание запроса к FMS

public:
    AdapterFMS();
    ~AdapterFMS() = default;

    // получение плана полета из базы данных по id
    FlightPlanPair getPlan(uint32_t id);

    // сохранить план в базу
    IdPair savePlan(FlightPlan &plan);

    // удалить план из базы
    fp::CommandStatus deletePlan(uint32_t);

    // инвертировать план
    fp::CommandStatus invertPlan(uint32_t id);

    // получить ближайшие 20 точек не дальше чем dist [m]
    WaypointVectorPair getNearestWaypoints(float dist);

    // получить точки ППМ по ICAO
    WaypointVectorPair getWaypointByIcao(std::string icao);

    // получить ППМ по id
    WaypointPair getWaypointById(uint32_t idWaypoint);

    // сохранить ППМ в базе
    IdPair saveWaypoint(Waypoint &point);

    // удалить ППМ из базы
    fp::CommandStatus deleteWaypoint(uint32_t id);

    // получение информации о планах полета из БД для отображения в каталоге
    FlightPlanInfoPair getCatalogInfoOfPlans();

    // получение информации о плане полета из БД для списка планов
    FlightPlanRouteInfoPair getPlanRouteInfo(uint32_t id);

    // рассчитать и вернуть сигналом навигационные параметры NavDataFms
    void setDeviceFlightData(const fp::DeviceFlightData& data);

    // активация плана полета по его идентификатору
    CommandStatus activatePlan(uint32_t planId);

    // полученить сигналом сведения об активном плане ActivePlanInfo
    void getActivePlanInfo();

    // полученить объектом сведения об активном плане ActivePlanInfo
    void getActivePlanInfo(std::pair<fp::CommandStatus, fp::ActivePlanInfo> &);

    // переключение на следующую или предыдущую ППМ (true - след, false - перд)
    void selectNextPoint(bool direction);

    // вставить точку в редактируемый план
    CommandStatus addWaypointToEditPlan(uint32_t position, Waypoint &point);

    // удалить точку из редактируемого плана
    CommandStatus deleteWaypointFromEditPlan(uint32_t position);

    // установить редактируемый план
    void setEditablePlan(FlightPlan&);

    // установить редактируемую точку
    void setEditPoint(Waypoint&);

    // создать маршрут следования из плана полета
    void createRouteInfo(FlightPlan&);

    // //

    ActivePlanManager* actPlanMngrPtr()       { return activePlanManager.data(); }
    FlightPlan& getActivePlan()               { return activePlan;       }
    FlightPlan& getEditablePlan()             { return editablePlan;     }
    FlightPlanRouteInfo& getEditablePlanInfo(){ return editablePlanInfo; }
    Waypoint&   getEditablePoint()            { return editablePoint;    }
    void setStateEditPlan(bool state)         { beginEditPlan = state;   }
    bool pointInActivePlan(int idPoint);

    // Активация режима Прямо На для точки в активном плане
    // void activateDirectToMode(uint32_t id);
};

#endif // ADAPTER_H
