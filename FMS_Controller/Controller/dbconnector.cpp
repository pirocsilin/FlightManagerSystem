
#include "dbconnector.h"
#include <QString>

DBconnector::DBconnector() :
    QSqlDatabase(QSqlDatabase::addDatabase(sqlDriver))
{
    if(isValid())           // if driver is valid
    {
        initConnection();
        DBquery = QSqlQuery(*this);
    }
}

void DBconnector::initConnection()
{
    QFile dbFile(dbName);
    if(dbFile.exists())
    {
        setDatabaseName (dbName);
        setUserName     (userName);
        setHostName     (hostName);
        setPassword     (password);

        this->open();
    }
}

bool DBconnector::executeQuery()
{
    if(!DBquery.exec(query))
    {
        QString msg = DBquery.lastError().text();

        msg = msg.isEmpty() || msg == ' ' ? "error with data base" : msg;

        *outData << cmdID::ERROR_DATABASE << "SQL_USER_ERROR: " + msg;

        qDebug() << "SQL_USER_ERROR: " + msg;

        return false;
    }
    return true;
}

bool DBconnector::addCommandForFMS_2(QByteArray *sqlCommand)
{
    query = "INSERT INTO reg_data(command) VALUES (:data)";
    DBquery.prepare(query);
    DBquery.bindValue(":data", *sqlCommand);

    return DBquery.exec();
}

void DBconnector::writeCommandToFMS_2(QTcpSocket &sokcetFms2)
{
    query = "SELECT command FROM reg_data;";

    if(!DBquery.exec(query))
    {
        qDebug() << "ERROR: SELECT command FROM reg_data";
    }
    else
    {
        rec = DBquery.record();
        if(DBquery.next())
        {
            QByteArray data;
            DBquery.seek(-1);
            while(DBquery.next())
            {
                data = DBquery.value(rec.indexOf("command")).toByteArray();
                sokcetFms2.write(data);
            }
            // Очищаем reg_data, так как базы данных сонхронизированы
            if(!DBquery.exec("DELETE FROM reg_data;"))
                qDebug() << "ERROR: DELETE FROM reg_data;";
            if(!DBquery.exec("UPDATE SQLITE_SEQUENCE SET seq=0 WHERE name='reg_data';"))
                qDebug() << "ERROR: set SQLITE_SEQUENCE for reg_table";
        }
    }
}

bool DBconnector::executeQuery(QString &request)
{
    if(!DBquery.exec(request))
    {
        hdr->id = cmdID::ERROR_DATABASE;
        QString errorMassage = DBquery.lastError().text();
        *outData << *hdr << "SQL_ERROR: "+errorMassage;
        return false;
    }
    return true;
}

bool DBconnector::checkExistsObject()
{
    if(!DBquery.next())
    {
        hdr->id = cmdID::ERROR_DATABASE;
        *outData << *hdr << "SQL_ERROR: запись в БД не найдена";
        return false;
    }
    DBquery.seek(-1);
    return true;
}

bool DBconnector::getPlanfromDB(FlightPlan &plan)
{
    query = QString("SELECT id_flight_plan FROM fpl_points WHERE id_flight_plan=%1 "
                    "GROUP BY id_flight_plan;").arg(plan.id);

    if(!executeQuery(query))
        return false;

    if(DBquery.next())
    {
        plan.id = DBquery.value(0).toInt();
        query = QString("SELECT id_waypoint, icao, latitude, longitude, altitude, region, type, radio_freq, runway_id FROM fpl_points "
                        "INNER JOIN waypoints ON id_waypoint = id "
                        "INNER JOIN flight_plans USING(id_flight_plan) "
                        "WHERE id_flight_plan = %1 "
                        "ORDER BY position_point ASC;").arg(plan.id);
        if(!executeQuery(query))
                return false;

        rec = DBquery.record();

        Waypoint point{};
        while(DBquery.next())
        {
            point.id           = DBquery.value(rec.indexOf("id_waypoint")).toInt();
            point.icao         = DBquery.value(rec.indexOf("icao")).toString().toStdString();
            point.latitude     = DBquery.value(rec.indexOf("latitude")).toDouble();
            point.longitude    = DBquery.value(rec.indexOf("longitude")).toDouble();
            point.altitude     = DBquery.value(rec.indexOf("altitude")).toInt();
            point.region       = DBquery.value(rec.indexOf("region")).toString().toStdString();
            point.type         = static_cast<WaypointType>(DBquery.value(rec.indexOf("type")).toInt());
            point.radioFrequency  = DBquery.value(rec.indexOf("radio_freq")).toInt();
            point.runwayId     = DBquery.value(rec.indexOf("runway_id")).toInt();

            plan.waypoints.push_back(point);
        }
    }
    createNameForPlan(plan);

    return true;
}

void DBconnector::getPlan()
{
    FlightPlan plan{};
    *inputData >> plan.id;

    if(!getPlanfromDB(plan))
        return;

    *outData << *hdr << plan;
}

void DBconnector::saveWaypoint()
{
    Waypoint point;
    *inputData >> point;

    if(point.id != -1 && pointIsValid(point))
    {
        query = QString("SELECT id FROM waypoints WHERE id = %1;").arg(point.id);
        if(!executeQuery(query))
            return;

        if(!DBquery.next())
            point.id = -1;
        else
        {
            if(!recordWaypointIntoBase(point, false))
                return;
        }
    }

    if(point.id == -1 && pointIsValid(point))
    {
        query = QString("SELECT id FROM waypoints ORDER BY id ASC;");

        if(!executeQuery(query))
            return;

        if(!DBquery.next())
            point.id = 1;
        else
        {
            int newId = 1;
            DBquery.seek(-1);
            rec = DBquery.record();
            while(DBquery.next())
            {
                int curId = DBquery.value(rec.indexOf("id")).toInt();
                if(newId < curId)
                    break;
                else
                    newId++;
            }
            point.id = newId;
        }

        if(!recordWaypointIntoBase(point, true))
            return;
    }

    *outData << *hdr << fp::CommandStatus::OK;
}

void DBconnector::deleteWaypoint()
{
    uint32_t idPoint;
    *inputData >> idPoint;

    query = QString("SELECT id_flight_plan AS id, COUNT(DISTINCT id_waypoint) AS count FROM "
                    "fpl_points WHERE id IN (SELECT id_flight_plan AS id FROM fpl_points WHERE "
                    "id_waypoint=%1 GROUP BY id) GROUP BY id;").arg(idPoint);
    if(!executeQuery(query))
        return;

    rec = DBquery.record();
    std::vector<std::pair<int, int>> data{};
    while(DBquery.next())
    {
        int id = DBquery.value(rec.indexOf("id")).toInt();
        int count = DBquery.value(rec.indexOf("count")).toInt();
        data.push_back(std::make_pair(id, count));
    }

    query = QString("DELETE FROM waypoints WHERE id=%1;").arg(idPoint);
    if(!executeQuery(query))
        return;

    query = QString("DELETE FROM fpl_points WHERE id_waypoint=%1;").arg(idPoint);
    if(!executeQuery(query))
        return;

    for(auto val : data)
    {
        if(val.second == 1)
        {
            query = QString("DELETE FROM flight_plans WHERE id_flight_plan = %1;").arg(val.first);
            if(!executeQuery(query))
                return;
        }
    }

    *outData << *hdr << fp::CommandStatus::OK;
}

bool DBconnector::recordWaypointIntoBase(Waypoint &point, bool newPoint)
{
    if(newPoint)
    {
        query = QString("INSERT INTO waypoints(id, icao, latitude, longitude, region, type, "
                        "altitude, radio_freq, runway_id) VALUES (%1, \"%2\", %3, %4, \"%5\", %6, %7, %8, %9);")
                        .arg(point.id).arg(QString::fromStdString(point.icao)).arg(point.latitude)
                        .arg(point.longitude).arg(QString::fromStdString(point.region)).arg((int)point.type)
                        .arg(point.altitude).arg(point.radioFrequency).arg(point.runwayId);

        if(!executeQuery(query))
            return false;
    }
    else
    {
        query = QString("UPDATE waypoints SET icao = \"%1\", latitude = %2, longitude = %3, region = \"%4\", "
                        "type = %5, altitude = %6, radio_freq = %7, runway_id = %8 WHERE id = %9;")
                        .arg(QString::fromStdString(point.icao)).arg(point.latitude).arg(point.longitude)
                        .arg(QString::fromStdString(point.region)).arg((int)point.type).arg(point.altitude)
                        .arg(point.radioFrequency).arg(point.runwayId).arg(point.id);

        if(!executeQuery(query))
            return false;
    }
    return true;
}

void DBconnector::invertPlan()
{
    uint32_t id;
    *inputData >> id;

    query = QString("SELECT id_waypoint AS idPoint FROM fpl_points "
                    "WHERE id_flight_plan=%1 ORDER BY position_point ASC;").arg(id);
    if(!executeQuery())
        return;

    uint32_t idPoint;
    std::vector<uint32_t> arr{};

    rec = DBquery.record();
    while(DBquery.next())
    {
        idPoint = DBquery.value(rec.indexOf("idPoint")).toInt();
        arr.push_back(idPoint);
    }

    if(arr.size() > 1)
    {
        query = QString("DELETE FROM fpl_points WHERE id_flight_plan=%1;").arg(id);
        if(!executeQuery())
            return;

        query = QString("INSERT INTO fpl_points(id_flight_plan, id_waypoint, position_point) VALUES");
        QString insertPart("");

        int pos{1};
        for(auto it = arr.rbegin(); it != arr.rend(); it++)
        {
            insertPart += QString("(%1, %2, %3),").arg(id).arg(*it).arg(pos++);
        }
        query += insertPart.remove(insertPart.size()-1, 1) + ';';

        if(!executeQuery())
            return;
    }

    *outData << *hdr << CommandStatus::OK;
}

void DBconnector::savePlan()
{
    FlightPlan plan;
    *inputData >> plan;

    if(plan.id != -1)                               // перезаписываем существующий план с проверкой на существование
    {
        query = QString("SELECT id_flight_plan FROM flight_plans WHERE id_flight_plan = %1;").arg(plan.id);
        if(!executeQuery(query))
            return;

        if(!DBquery.next())
            plan.id = -1;
        else
        {
            if(!insertWaypointIntoPlan(plan))       // обновляем список точек
                return;
        }
    }

    if(plan.id == -1 && plan.waypoints.size() > 0)   // добавляем новый план в базу
    {
        query = QString("SELECT id_flight_plan AS id FROM flight_plans ORDER BY id ASC;");
        if(!executeQuery(query))
            return;

        if(!DBquery.next())
            plan.id = 1;
        else
        {
            int newId = 1;
            DBquery.seek(-1);
            rec = DBquery.record();
            while(DBquery.next())
            {
                int curId = DBquery.value(rec.indexOf("id")).toInt();
                if(newId < curId)
                    break;
                else
                    newId++;
            }
            plan.id = newId;
        }

        query = QString("INSERT INTO flight_plans (id_flight_plan) VALUES (%1);").arg(plan.id);
        if(!executeQuery(query))
            return;

        if(!insertWaypointIntoPlan(plan))
            return;
    }

    *outData << *hdr << plan.id;
}

void DBconnector::deletePlan()
{
    FlightPlan emptyPlan{};
    *inputData >> emptyPlan.id;

    if(!insertWaypointIntoPlan(emptyPlan))
        return;

    *outData << *hdr << fp::CommandStatus::OK;
}

bool DBconnector::insertWaypointIntoPlan(FlightPlan &plan)
{
    query = QString("DELETE FROM fpl_points WHERE id_flight_plan = %1;").arg(plan.id);
    if(!executeQuery(query))
        return false;

    if(plan.waypoints.size() == 0)
    {
        query = QString("DELETE FROM flight_plans WHERE id_flight_plan = %1;").arg(plan.id);
        if(!executeQuery(query))
            return false;
    }
    else
    {
        query = QString("INSERT INTO fpl_points(id_flight_plan, id_waypoint, position_point) VALUES");
        QString insertPart("");

        int posPoint{1};
        for(auto wPoint : plan.waypoints)
        {
            insertPart += QString("(%1, %2, %3),").arg(plan.id).arg(wPoint.id).arg(posPoint++);
        }

        query += insertPart.remove(insertPart.size()-1, 1) + ';';

        if(posPoint > 1)
            if(!executeQuery(query))
                return false;
    }
    return true;
}

void DBconnector::getWaypointById()
{
    Waypoint point{};
    *inputData >> point.id;

    query = QString("SELECT * FROM waypoints WHERE id = %1;").arg(point.id);

    if(!executeQuery(query))
        return;

    rec = DBquery.record();
    if(DBquery.next())
    {
        point.icao         = DBquery.value(rec.indexOf("icao")).toString().toStdString();
        point.latitude     = DBquery.value(rec.indexOf("latitude")).toDouble();
        point.longitude    = DBquery.value(rec.indexOf("longitude")).toDouble();
        point.altitude     = DBquery.value(rec.indexOf("altitude")).toInt();
        point.region       = DBquery.value(rec.indexOf("region")).toString().toStdString();
        point.type         = static_cast<WaypointType>(DBquery.value(rec.indexOf("type")).toInt());
        point.radioFrequency  = DBquery.value(rec.indexOf("radio_freq")).toInt();
        point.runwayId     = DBquery.value(rec.indexOf("runway_id")).toInt();
    }

    *outData << *hdr << point;
}

void DBconnector::getWaypointByIcao()
{
    std::string identifier{};

    *inputData >> identifier;

    query = QString("SELECT * FROM waypoints WHERE icao='%1' ORDER BY icao ASC;")
                    .arg(QString::fromStdString(identifier));
    if(!executeQuery())
        return;

    rec = DBquery.record();

    std::vector<Waypoint> vector{};
    Waypoint point{};

    while(DBquery.next())
    {
        point.id           = DBquery.value(rec.indexOf("id")).toInt();
        point.icao         = DBquery.value(rec.indexOf("icao")).toString().toStdString();
        point.latitude     = DBquery.value(rec.indexOf("latitude")).toDouble();
        point.longitude    = DBquery.value(rec.indexOf("longitude")).toDouble();
        point.altitude     = DBquery.value(rec.indexOf("altitude")).toInt();
        point.region       = DBquery.value(rec.indexOf("region")).toString().toStdString();
        point.type         = static_cast<WaypointType>(DBquery.value(rec.indexOf("type")).toInt());
        point.radioFrequency  = DBquery.value(rec.indexOf("radio_freq")).toInt();
        point.runwayId     = DBquery.value(rec.indexOf("runway_id")).toInt();

        vector.push_back(point);
    }

    *outData << *hdr << vector;
}

void DBconnector::getNearestWaypoints()
{
    short maxPointInRes  {20};   // размер результирующего массива
    float latitude, longitude,   // deg
                 maxDistance ;   // m

    *inputData >> latitude >> longitude >> maxDistance;

    query = QString("SELECT * FROM waypoints;");
    if(!executeQuery())
        return;

    float dist;
    Waypoint point;
    std::vector<Waypoint> points{};
    rec = DBquery.record();

    while(DBquery.next())
    {
        point.latitude     = DBquery.value(rec.indexOf("latitude")).toDouble();
        point.longitude    = DBquery.value(rec.indexOf("longitude")).toDouble();

        calcDistAndTrackBetweenWaypoints(latitude, longitude, point.latitude, point.longitude, &dist);

        if(dist * EARTH_RADIUS <= maxDistance)
        {
            point.id           = DBquery.value(rec.indexOf("id")).toInt();
            point.icao         = DBquery.value(rec.indexOf("icao")).toString().toStdString();
            point.altitude     = DBquery.value(rec.indexOf("altitude")).toInt();
            point.region       = DBquery.value(rec.indexOf("region")).toString().toStdString();
            point.type         = static_cast<WaypointType>(DBquery.value(rec.indexOf("type")).toInt());
            point.radioFrequency  = DBquery.value(rec.indexOf("radio_freq")).toInt();
            point.runwayId     = DBquery.value(rec.indexOf("runway_id")).toInt();

            points.push_back(point);
        }
    }

    sortWaypointByDistance(latitude, longitude, points);

    if(points.size() > maxPointInRes)
        points.erase(points.begin() + maxPointInRes, points.end());

    *outData << *hdr << points;
}

std::string DBconnector::getNameForPlan(int id)
{
    query = QString("SELECT icao FROM fpl_points "
                    "INNER JOIN waypoints ON id_waypoint = id "
                    "INNER JOIN flight_plans USING(id_flight_plan) "
                    "WHERE id_flight_plan = %1 "
                    "ORDER BY position_point ASC;").arg(id);

    if(!DBquery.exec(query))
        return "";

    std::string nameIcao;
    std::vector<std::string> namesIcao;
    rec = DBquery.record();

    while(DBquery.next()){
        nameIcao = DBquery.value(rec.indexOf("icao")).toString().toStdString();
        namesIcao.push_back(nameIcao);
    }
    std::string first = namesIcao.size() > 0 ?  namesIcao.front() : "______";
    std::string last  = namesIcao.size() > 1 ?  namesIcao.back()  : "______";

    return first + " / " + last;
}

void DBconnector::getCatalogInfoOfPlans()
{
    query = QString("SELECT id_flight_plan FROM flight_plans WHERE id_flight_plan "
                    "IN (SELECT DISTINCT id_flight_plan FROM fpl_points);");
    if(!executeQuery(query))
        return;

    FlightPlanInfo planInfo{};
    std::vector<FlightPlanInfo> planInfoCatalog{};
    rec = DBquery.record();

    while(DBquery.next())
    {
        planInfo.id = DBquery.value(rec.indexOf("id_flight_plan")).toInt();
        planInfoCatalog.push_back(planInfo);
    }

    for(auto &planInfo : planInfoCatalog)
    {
        query = QString("SELECT latitude, longitude FROM fpl_points "
                        "INNER JOIN waypoints ON id_waypoint = id "
                        "INNER JOIN flight_plans USING(id_flight_plan) "
                        "WHERE id_flight_plan = %1 "
                        "ORDER BY position_point ASC;").arg(planInfo.id);

        if(!executeQuery(query))
            return;

        int countPoints{1};
        double summDistance{};
        float distance;
        std::pair<double, double> point, prevPoint;
        rec = DBquery.record();

        while(DBquery.next())
        {
            point = std::make_pair(DBquery.value(rec.indexOf("latitude")).toDouble(),
                                   DBquery.value(rec.indexOf("longitude")).toDouble());

            if(countPoints > 1)
            {
                calcDistAndTrackBetweenWaypoints(prevPoint.first,
                                                 prevPoint.second,
                                                 point.first,
                                                 point.second,
                                                 &distance);

                summDistance += distance * EARTH_RADIUS / 1000;
            }
            countPoints++;
            prevPoint = point;
        }
        planInfo.totalDistance = summDistance;
        planInfo.name = getNameForPlan(planInfo.id);
    }

    *outData << *hdr << planInfoCatalog;
}

void DBconnector::getPlanRouteInfo()
{
    FlightPlan plan{};
    *inputData >> plan.id;

    if(!getPlanfromDB(plan))
        return;

    WaypointRouteInfo wpRouteInfo{};
    FlightPlanRouteInfo planRouteInfo{};
    planRouteInfo.id = plan.id;
    planRouteInfo.name = plan.name;

    float distance;
    float bearing;

    for(int i{}; i<plan.waypoints.size(); i++)
    {
        wpRouteInfo.id   = plan.waypoints[i].id;
        wpRouteInfo.icao = plan.waypoints[i].icao;
        wpRouteInfo.altitude = plan.waypoints[i].altitude;
        wpRouteInfo.isActive = false;

        if(i)
        {
            calcDistAndTrackBetweenWaypoints(
                        plan.waypoints[i-1].latitude,
                        plan.waypoints[i-1].longitude,
                        plan.waypoints[i].latitude,
                        plan.waypoints[i].longitude,
                        &distance,
                        &bearing);

            wpRouteInfo.distance = distance * EARTH_RADIUS;
            wpRouteInfo.bearing = bearing * 180 / M_PI;
        }
        if(!i)
            wpRouteInfo.isActive = true;

        planRouteInfo.waypoints.push_back(wpRouteInfo);
    }

    *outData << *hdr << planRouteInfo;
}
