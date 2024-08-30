#ifndef ADAPTER_H
#define ADAPTER_H

#include <QPair>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTcpSocket>
#include <QTimer>
#include <QTime>
#include <QEventLoop>
#include <QHostAddress>
#include <utility>
#include "common/structsflightplan.h"
#include "active_plan/structs_data.h"
#include "common/labsflightplan.h"
#include "active_plan/mathplan.h"
#include "connector.h"

#define ASYNC_INVOKE(methodName, ...) \
    if(QThread::currentThreadId() != selfThread) { \
        QMetaObject::invokeMethod(this, #methodName, Qt::QueuedConnection, __VA_ARGS__); \
        return; \
    }

using namespace fp;

class AdapterFMS : public QObject
{
    Q_OBJECT 

private:

    QMutex awaiteReceivingData;
    QMutex sharedDataCtrl;

    uint32_t uniqueCmd {10};

    QSharedPointer<Connector> connector;
    QThread connectorThread;

    bool activePlanIsSet  {false};
    bool beginEditPlan    {false};
    ManagerCalculationPlan calculatPlan;    //!< манагер рассчета параметров активного плана
    FlightPlan editablePlan;                //!< редактируемый план

    template<typename... Args>
    bool createRequestAndSend(cmdID id, Args... args);

    void sendDataAndAwaite(QByteArray &data);

public:
    AdapterFMS();
    ~AdapterFMS() = default;

    bool awaiteProcess;

    // получение плана полета из базы данных по id
    std::pair<fp::CommandStatus, fp::FlightPlan> getPlan(uint32_t id);

    // сохранить план в базу
    fp::CommandStatus savePlan(FlightPlan &plan);

    // удалить план из базы
    fp::CommandStatus deletePlan(uint32_t);

    // загрузить
    std::pair<fp::CommandStatus, fp::FlightPlan> setPlan(uint32_t id);

    // получение информации о планах полета из БД для отображения в каталоге
    std::pair<fp::CommandStatus, std::vector<fp::FlightPlanInfo>> getCatalogInfoOfPlans();

    // получение информации о плане полета из БД для списка планов
    std::pair<fp::CommandStatus, fp::FlightPlanRouteInfo> getPlanRouteInfo(uint32_t id);

    // получить ППМ по id
    std::pair<fp::CommandStatus, fp::Waypoint> getWaypoint(uint32_t idWaypoint);

    // сохранить ППМ в базе
    fp::CommandStatus saveWaypoint(Waypoint &point);

    // рассчитать и вернуть навигационные параметры
    fp::NavDataFms& setDeviceFlightData(const fp::DeviceFlightData& data);

    // активация плана полета по его идентификатору
    std::pair<CommandStatus, FlightPlan> activatePlan(uint32_t planId);

    // получение сведения об активном плане
    std::pair<fp::CommandStatus, fp::ActivePlanInfo> getActivePlanInfo();

    // переключение на следующую и предыдущую ППМ (true - след, false - перд)
    bool selectNextPoint(bool direction);

    void addWaypointToEditPlan(uint32_t position, Waypoint &point); //!< вставить точку в редактируемый план
    void deleteWaypointFromEditPlan(uint32_t position);             //!< удалить точку из редактируемого плана

    void setEditablePlan(FlightPlan &);     //!< установить редактируемый план
    FlightPlan& getEditablePlan();          //!< получить редактируемый план

    /**
     * @brief Активация режима Прямо На для точки в активном плане
     * @param id идентификатор точки
     */
    void activateDirectToMode(uint32_t id);

};

#endif // ADAPTER_H
