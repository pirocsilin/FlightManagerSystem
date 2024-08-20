
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

    // отправка команд в МФИ-2
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

bool Controller::decodeRequest()
{
    inputStream >> hdr;

    qDebug() << printCommandInfo(hdr.uniqueCmd, hdr.id, prevUniqueCmd).toUtf8().data();                                                //

    if(hdr.uniqueCmd <= prevUniqueCmd)          // Выход, если команда получена повторно
        return false;

    prevUniqueCmd = hdr.uniqueCmd;              // Сохраняем номер последней команды
    HeaderData::Name dst = hdr.name;            // Кто прислал запрос
    hdr.name = HeaderData::Name::CONTROLLER_ANS;// Формируем заголовок ответа

    outStream << qint16(0);

    switch (hdr.id)
    { 
    case cmdID::GET_PLAN:                   db.getPlan();               break;
    case cmdID::GET_WAYPOINT:               db.getWaypoint();           break;
    case cmdID::GET_CATALOG_INFO_OF_PLANS:  db.getCatalogInfoOfPlans(); break;
    case cmdID::GET_PLAN_ROUTE_INFO:        db.getPlanRouteInfo();      break;

    default: return false;
    }

    // Отправляем ответ только в Adapter так как он его ожидает.
    // Команда полученная из МФИ-2 пока без ответа!
    if(dst == HeaderData::Name::ADAPTER_COMMAND)
    {
        sendData();
    }
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

QString Controller::printCommandInfo(int uniqueCmd, int idCmd, int prevUniqueCmd)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString formattedTime = currentTime.toString("hh:mm:ss.zzz");
    QString msg = QString("[%1] uniqCmd: %2, idCmd: %3, prevUniqueCmd: %4")
                  .arg(formattedTime).arg(uniqueCmd).arg(idCmd).arg(prevUniqueCmd);

    return msg;
}
