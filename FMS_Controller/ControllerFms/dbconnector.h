#ifndef DBCONNECTOR_H
#define DBCONNECTOR_H

#include <QSql>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QTcpSocket>
#include "common/labsflightplan.h"
#include "common/structsflightplan.h"

using namespace fp;

typedef std::pair<int, QByteArray> CommandMfi2;

class DBconnector : public QSqlDatabase
{

private:
    static inline QString sqlDriver {"QSQLITE"};
    QString dbName      {"../points"},
            userName    {"user"},
            hostName    {"local"},
            password    {"1234"};

    QString     query, errorMsg;
    QSqlQuery   DBquery;
    QSqlRecord  rec;

    CommandMfi2 lastCmdToMfi2   {INVALID_ID, QByteArray()};    //!< последняя команда отправленная к MFI-2
    CommandStatus stateLastReguest  {CommandStatus::INVALID};  //!< статус выполнения последнего запроса в БД

    std::vector<Waypoint> recentWaypoints;      //!< Последние точки запрошенные из базы

    void initConnection();
    bool executeQuery();
    bool executeQuery(QString &reqiest);        //!< выполнение запроса в базу данных
    bool checkExistsObject();                   //!< проверка запроса на пустоту
    std::string getNameForPlan(int id);         //!< сформировать имя для плана полета
    bool getPlanfromDB(FlightPlan &plan);       //!< получить план полета из базы данных
    //
    void writeCommandToFMS_2    (QTcpSocket&);
    bool insertWaypointIntoPlan (FlightPlan&);
    bool recordWaypointIntoBase (Waypoint&, bool=false);

public:
    DBconnector();

    enum StateRecordQuery : uint8_t
    {
        SENT = 1,
        DONE = 2
    };

    QDataStream *inputData;
    QDataStream *outData;
    HeaderData  *hdr;

    bool statusLastDeleteCommand{true};

    void getPlan                ();
    void savePlan               ();
    void deletePlan             ();
    void getWaypointById        ();
    void getWaypointByIcao      ();
    void saveWaypoint           ();
    void deleteWaypoint         ();
    void getCatalogInfoOfPlans  ();
    void getPlanRouteInfo       ();
    //
    void getNearestWaypoints    ();
    void invertPlan             ();
    //
    bool addCommandForFMS_2     (QByteArray &data);
    bool getIdRecordQuery       (int &id);
    bool getLastCmdIdFromFms2   (int &id);
    bool setLastCmdIdFromFms2   (int id);
    void getRecordQuery         (QByteArray &data);
    bool delRecordQuery         ();

    //
    CommandStatus getRequestStatus(){ return stateLastReguest; }
};

#endif // DBCONNECTOR_H
