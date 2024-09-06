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
#include "math/math_fms.h"

using namespace fp;

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

    std::vector<Waypoint> recentWaypoints;      //!< Последние точки запрошенные из базы

    void initConnection();
    bool executeQuery();
    bool executeQuery(QString &reqiest);        //!< выполнение запроса в базу данных
    bool checkExistsObject();                   //!< проверка запроса на пустоту
    std::string getNameForPlan(int id);         //!< сформировать имя для плана полета
    bool getPlanfromDB(FlightPlan &plan);       //!< получить план полета из базы данных
    //
    bool addCommandForFMS_2     (QByteArray*);
    void writeCommandToFMS_2    (QTcpSocket&);
    bool insertWaypointIntoPlan (FlightPlan&);
    bool recordWaypointIntoBase (Waypoint&, bool=false);

public:
    DBconnector();

    QDataStream *inputData;
    QDataStream *outData;
    HeaderData  *hdr;

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
};

#endif // DBCONNECTOR_H
