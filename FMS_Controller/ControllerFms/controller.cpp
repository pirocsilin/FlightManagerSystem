
#include "controller.h"

Controller::Controller(QObject *parent) :
    outStream(&outData, QIODevice::WriteOnly),
    streamMfi2(&dataMfi2, QIODevice::WriteOnly),
    inputStream(&inputData, QIODevice::ReadWrite)
{
    db.inputData= &inputStream;
    db.outData  = &outStream;
    db.hdr      = &hdr;

    if(!server.listen(QHostAddress(hostName), sourcePort))
    {
        qDebug() << "ERROR start to server:" << server.errorString();
        server.close();
        exit(-1);
    }

    connTimerMfi2.setInterval(500);

    connect(&server, &QTcpServer::newConnection, this, &Controller::slotNewConnection);
    connect(&connTimerMfi2, &QTimer::timeout, this, &Controller::tryConnectToMfi2);
    connect(&mfi2Socket, &QTcpSocket::connected, this, &Controller::connectedToMfi2);
    connect(&mfi2Socket, &QTcpSocket::disconnected, this, &Controller::tryConnectToMfi2);
    connect(&mfi2Socket, &QTcpSocket::readyRead, this, &Controller::slotReadyRead);

    tryConnectToMfi2();
}

void Controller::tryConnectToMfi2()
{
    if(!connTimerMfi2.isActive())
    {
        connTimerMfi2.start();
        qDebug() << "Start timer for connect to MFI-2";
    }
    else if(mfi2Socket.state() == QAbstractSocket::UnconnectedState)
    {
        mfi2Socket.connectToHost(QHostAddress::Any, destPort);

        emit signalConnectionToMfi2(false);
    }
}

void Controller::connectedToMfi2()
{
    connTimerMfi2.stop();

    emit signalConnectionToMfi2(true);
#ifdef SYNCH_DATA_BASE
    trySendRecordQuery();
#endif
}

void Controller::slotNewConnection()
{
    QTcpSocket *client = server.nextPendingConnection();

    connect(client, &QTcpSocket::disconnected, this, &Controller::slotClientDisconnected);
    connect(client, &QTcpSocket::disconnected, client, &QTcpSocket::deleteLater);
    connect(client, &QTcpSocket::readyRead, this, &Controller::slotReadyRead);

    emit signalNewClientConnected("Client connected");
}

void Controller::slotClientDisconnected()
{
    // ОПАСНЫЙ МАНЕВР !!!! (нужна проверка сокета)

    prevUniqueCmd = 0;

    emit signalClientDisconnected("Client disconnected!");
}

void Controller::slotReadyRead()
{
    clientSocket = static_cast<QTcpSocket*>(sender());
    streamSocket.setDevice(clientSocket);

    for (;;)
    {
        if(!nextBlockSize)
        {
            if(clientSocket->bytesAvailable() < sizeof(qint16))
                break;
            streamSocket >> nextBlockSize;
        }

        if(clientSocket->bytesAvailable() < nextBlockSize)
            break;

        clearBuffers();
        inputData = clientSocket->read(nextBlockSize);

        if(decodeRequest())
        {
            // Any if
        }
        else
        {
            // Any else
        }

        nextBlockSize = 0;
    }
}

#ifdef SYNCH_DATA_BASE
void Controller::trySendRecordQuery()
{
    if(mfi2Socket.state() != QAbstractSocket::ConnectedState)
        return;

    db.getRecordQuery(dataMfi2);
    if(db.getRequestStatus() == CommandStatus::OK)
    {
        /* отправка сохраненной команды в МФИ-2 */
        if(!stateUpdatingDataBaseToMfi2)
        {
            sendMsgAboutUpdate(START_UPDATE_DATABASE);
        }
        mfi2Socket.write(dataMfi2);
    }
    else
    if(db.getRequestStatus() == CommandStatus::INVALID)
    {
        /* таблица пуста */
        sendMsgAboutUpdate(STOP_UPDATE_DATABASE);
    }
    else
    if(db.getRequestStatus() == CommandStatus::ERROR_DATABASE)
        { /* ошибка при запросе в базу данных */ }
}

void Controller::sendMsgAboutUpdate(cmdID state)
{
    if(mfi2Socket.state() == QAbstractSocket::ConnectedState)
    {
        stateUpdatingDataBaseToMfi2 = state == cmdID::START_UPDATE_DATABASE ? true : false;
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << qint16(0) << HeaderData{HeaderData::MFI_COMMAND, 0, state};
        sendData(mfi2Socket, data);
    }
}
#endif

bool Controller::decodeRequest()
{
    QByteArray currentRawData = inputData;
    inputStream >> hdr;
    outStream   << qint16(0);

    qDebug() << printCommandInfo(hdr.uniqueCmd, hdr.id, prevUniqueCmd).toUtf8().data();


    HeaderData sourceHdr = hdr;
    switch(sourceHdr.name)
    {
    case HeaderData::Name::ADAPTER_COMMAND:     //!< команда на выполнение от адаптера
    {
        if(hdr.uniqueCmd <= prevUniqueCmd)
            return false;
#ifdef SYNCH_DATA_BASE
        if((hdr.id & cmdID::MODIFICATION_CMD))
            sendMsgAboutUpdate(START_UPDATE_DATABASE);
#endif
        prevUniqueCmd = hdr.uniqueCmd;
        hdr.name = HeaderData::Name::CONTROLLER_ANS;
        break;
    }
#ifdef SYNCH_DATA_BASE
    case HeaderData::Name::MFI_COMMAND:         //!< команда на выполнение от МФИ-2
    {
        int idLastCmd;
        if(db.getLastCmdIdFromFms2(idLastCmd) && hdr.uniqueCmd == idLastCmd)
                return false;

        hdr.name = HeaderData::Name::MFI_ANS;
        break;
    }
    case HeaderData::Name::MFI_ANS:             //!< ответ на выполнение команды от МФИ-2
    {
        if(hdr.id != cmdID::ERROR_DATABASE && db.delRecordQuery())
            trySendRecordQuery();

        return true;
    }
    default: return false;
#endif
    }

    switch (hdr.id)
    { 
    case cmdID::GET_PLAN:                   db.getPlan();               break;
    case cmdID::SAVE_PLAN:                  db.savePlan();              break;
    case cmdID::DELETE_PLAN:                db.deletePlan();            break;
    case cmdID::INVERT_PLAN:                db.invertPlan();            break;
    case cmdID::GET_WAYPOINT_BY_ID:         db.getWaypointById();       break;
    case cmdID::GET_WAYPOINT_BY_ICAO:       db.getWaypointByIcao();     break;
    case cmdID::GET_NEAREST_WAYPOINTS:      db.getNearestWaypoints();   break;
    case cmdID::SAVE_WAYPOINT:              db.saveWaypoint();          break;
    case cmdID::DELETE_WAYPOINT:            db.deleteWaypoint();        break;
    case cmdID::GET_CATALOG_INFO_OF_PLANS:  db.getCatalogInfoOfPlans(); break;
    case cmdID::GET_PLAN_ROUTE_INFO:        db.getPlanRouteInfo();      break;
    default: return false;
    }

    sendData();

#ifdef SYNCH_DATA_BASE
    if(sourceHdr.name == HeaderData::Name::MFI_COMMAND && hdr.id != cmdID::ERROR_DATABASE)
    {
        if(db.setLastCmdIdFromFms2(hdr.uniqueCmd))
            sendData();
        return true;
    }

    // после отправки ответа в Адаптер, если выполнена модификация БД,
    // сохраняем команду и пытаемся выполнить синхронизацию базы с МФИ-2

    if((sourceHdr.id & cmdID::MODIFICATION_CMD) && hdr.id != cmdID::ERROR_DATABASE)
    {
        if(db.addCommandForFMS_2(currentRawData))
            trySendRecordQuery();
    }
#endif

    return true;
}

void Controller::clearBuffers()
{
    inputData.clear();
    inputStream.device()->seek(0);
    outData.clear();
    outStream.device()->seek(0);
    dataMfi2.clear();
    streamMfi2.device()->seek(0);
    hdr = {};
}

void Controller::sendData()
{
    outStream.device()->seek(0);
    outStream << qint16(outData.size() - sizeof(qint16));
    clientSocket->write(outData);
}

void Controller::sendData(QTcpSocket &socket, QByteArray &outData)
{
    outStream.device()->seek(0);
    outStream << qint16(outData.size() - sizeof(qint16));
    socket.write(outData);
}

QString Controller::printCommandInfo(int uniqueCmd, int idCmd, int prevUniqueCmd)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString formattedTime = currentTime.toString("hh:mm:ss.zzz");
    QString msg = QString("[%1] uniqCmd: %2, idCmd: %3, prevUniqueCmd: %4")
                  .arg(formattedTime).arg(uniqueCmd).arg(idCmd).arg(prevUniqueCmd);

    return msg;
}
