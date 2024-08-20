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

    bool activePlanIsSet {false};
    ManagerCalculationPlan calculatPlan;

    template<typename... Args>
    bool createRequestAndSend(cmdID id, Args... args);

    void sendDataAndAwaite(QByteArray &data);

public:
    AdapterFMS();
    ~AdapterFMS() = default;

    bool awaiteProcess;

    // получение плана полета из базы данных по id
    std::pair<fp::CommandStatus, fp::FlightPlan> getPlan(uint32_t id);

    // получение информации о планах полета из БД для отображения в каталоге
    std::pair<fp::CommandStatus, std::vector<fp::FlightPlanInfo>> getCatalogInfoOfPlans();

    // получение информации о плане полета из БД для списка планов
    std::pair<fp::CommandStatus, fp::FlightPlanRouteInfo> getPlanRouteInfo(uint32_t id);

    // получить ППМ по id
    std::pair<fp::CommandStatus, fp::Waypoint> getWaypoint(uint32_t idWaypoint);

    // рассчитать и вернуть навигационные параметры
    fp::NavDataFms& setDeviceFlightData(const fp::DeviceFlightData& data);

    // активация плана полета по его идентификатору
    fp::CommandStatus activatePlan(uint32_t planId);

    // очистить активный план полета
    void deactivatePlan();

    // получение сведения об активном плане
    std::pair<fp::CommandStatus, fp::ActivePlanInfo> getActivePlanInfo();

    /**
     * @brief Активация режима Прямо На для точки в активном плане
     * @param id идентификатор точки
     */
    void activateDirectToMode(uint32_t id);

};

#endif // ADAPTER_H
