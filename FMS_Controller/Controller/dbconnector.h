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

#define MAX_RECENT_POINTS   10
#define MAX_NEAREST_POINTS  10

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

    void initConnection();
    bool executeQuery();
    bool executeQuery(QString &reqiest);            //!< выполнение запроса в базу данных
    bool checkExistsObject();                       //!< проверка запроса на пустоту
    std::string getNameForPlan(int id);             //!< сформировать имя для плана полета
    bool getPlanfromDB(int id, FlightPlan &plan);   //!< получить план полета из базы данных

    //

    bool addCommandForFMS_2(QByteArray *);
    void writeCommandToFMS_2(QTcpSocket &);

public:
    DBconnector();

    QDataStream *inputData;
    QDataStream *outData;
    HeaderData *hdr;

    std::vector<Waypoint> recentWaypoints;  //!< Последние точки запрошенные из базы
    int editablePlan{-1};                   //!< Редактируемый план

    void getPlan                ();
    void savePlan               ();
    void getWaypoint            ();
    void saveWaypoint           ();
    void getCatalogInfoOfPlans  ();
    void getPlanRouteInfo       ();
    //
    bool insertWaypointIntoPlan (FlightPlan &);
    //
    void findWaypoint           ();
    void getNearestWaypoints    ();
    void getWaypointByIdetifier ();
    void invertPlanAndGet       ();
    void addWaypointToEditPlan  ();
    void delWaypointFromEditPlan();
    void deleteStoredPlan       ();
    void setAndGetEditPlan      ();
    bool updatePath(QDataStream *);
};

#endif // DBCONNECTOR_H
