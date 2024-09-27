#include <connect/connector.h>
#include <common/labsflightplan.h>

Connector::~Connector()
{
    connectSocket->disconnectFromHost();
    connectSocket->close();
}

void Connector::initConnection()
{
    connectSocket = QSharedPointer<QTcpSocket>(new QTcpSocket);
    connectTimer = QSharedPointer<QTimer>(new QTimer);

    connect(connectTimer.data(),  &QTimer::timeout, this, &Connector::slotTryConnectToController);
    connect(connectSocket.data(), &QTcpSocket::disconnected, this, &Connector::slotTryConnectToController);
    connect(connectSocket.data(), &QTcpSocket::connected, this, &Connector::slotConnectedToController);
    connect(connectSocket.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotErrorSocket(QAbstractSocket::SocketError)));
    connect(connectSocket.data(), &QTcpSocket::readyRead, this, &Connector::slotReadyRead);

    connectTimer->setInterval(1000);
    slotTryConnectToController();
}

void Connector::slotReadyRead()
{
    QDataStream in(connectSocket.data());
    in.setVersion(QDataStream::Qt_5_3);
    for (;;)
    {
        if(!nextBlockSize)
        {
            if(connectSocket->bytesAvailable() < sizeof(qint16))
                break;
            in >> nextBlockSize;
        }

        if(connectSocket->bytesAvailable() < nextBlockSize)
            break;

        inputData = connectSocket->read(nextBlockSize);

#ifdef SYNCH_DATA_BASE
        fp::HeaderData hdr;
        fp::getHdrFromResponse(inputData, hdr);
        if(hdr.id == fp::cmdID::START_UPDATE_DATABASE)
        {
            updStateDataBaseMfi2 = true;
            emit signalUpdateDataBase(true);
        }
        else
        if(hdr.id == fp::cmdID::STOP_UPDATE_DATABASE)
        {
            updStateDataBaseMfi2 = false;
            emit signalUpdateDataBase(false);
        }
        else
  #endif
        emit signalReadyRead();

        nextBlockSize = 0;
    }
}

void Connector::sendDataAndAwaite(QByteArray *data)
{
    QTimer timer;
    int mSeconds{};
    QEventLoop loop;

    QEventLoopLocker locker(&loop);

    connect(this, &Connector::signalReadyRead, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, [&](){
        if(mSeconds >= 500)
            loop.quit();
        else
        {
            mSeconds += 100;
            qDebug() << "awaite receiving data ..." << mSeconds << "m sec";
        }
    });

    inputData.clear();
    connectSocket->write(*data);

    timer.start(100);
    loop.exec();
}

void Connector::slotTryConnectToController()
{
#ifdef SYNCH_DATA_BASE
    if(updStateDataBaseMfi2 == true)
    {
        updStateDataBaseMfi2 = false;
        emit signalUpdateDataBase(false);
    }
#endif
    if(!connectTimer->isActive())
    {
        connectTimer->start();
        isTcpConnected = false;
    }
    if(connectSocket->state() != QAbstractSocket::ConnectedState)
    {
        qDebug() << "[controller] try connect to FMS";

        connectSocket->connectToHost(QHostAddress::Any, remotePort);
        if(!connectSocket->waitForConnected(1000))
        {
            qDebug() << ("[controller] " + connectSocket->errorString()).toUtf8().data();
        }
    }
}

void Connector::slotConnectedToController()
{
    connectTimer->stop();
    isTcpConnected = true;
    qDebug() << "[controller] connected to FMS";
}

bool Connector::connectionIsSuccess()
{
    return connectSocket->state() == QTcpSocket::ConnectedState;
}

void Connector::slotErrorSocket(QAbstractSocket::SocketError)
{
    // any
}



