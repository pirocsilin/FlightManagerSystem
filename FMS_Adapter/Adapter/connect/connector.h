#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QDataStream>
#include <QEventLoop>

class Connector : public QObject
{
    Q_OBJECT

public:

    Connector() = default;
    ~Connector();

    bool isTcpConnected {false};
    QByteArray inputData;

private:

    QString localHost {"localhost"};
    int localPort {6666};

    QString remoteHost {"localhost"};
    int remotePort {5555};

    qint16 nextBlockSize      {0};
    bool updStateDataBaseMfi2 {false};

    QSharedPointer<QTcpSocket> connectSocket;
    QSharedPointer<QTimer>     connectTimer;

public slots:

    bool connectionIsSuccess();
    void slotTryConnectToController();
    void slotConnectedToController();
    void slotErrorSocket(QAbstractSocket::SocketError);
    void slotReadyRead();
    void initConnection();
    //
    void sendDataAndAwaite(QByteArray *data);

signals:

    void signalReadyRead();
    void signalUpdateDataBase(bool);

};

#endif // CONNECTOR_H
