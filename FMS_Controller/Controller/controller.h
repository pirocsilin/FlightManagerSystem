#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDate>
#include <QTimer>
#include <QObject>
#include "dbconnector.h"

using namespace fp;

class Controller : public QObject{

    Q_OBJECT

public:
    Controller(QObject *parent = nullptr);

private:

    QTcpServer server;

    QString hostName    {"localhost"};
    int sourcePort      {5555};
    int destPort        {6666};

    int prevUniqueCmd   {};     //!< предыдущая уникальная команда
    DBconnector db;             //!< объект для работы с БД

    qint16 nextBlockSize{};     //!< размер данныз для считывания из сокета
    QTimer connTimerMfi2;

    QTcpSocket mfi2Socket;      //!< сокет для коммуникации с МФИ-2

    QTcpSocket *clientSocket;   //!< подключившийся клиент
    QDataStream streamSocket;

    QByteArray inputData;       //!< приемный буфер от клиента
    QDataStream inputStream;

    QByteArray  outData;        //!< отправной буфер для клиента
    QDataStream outStream;

    QByteArray  dataMfi2;       //!< буфер для команды МФИ-2
    QDataStream streamMfi2;

    HeaderData hdr;             //!< заголовок сообщения

    bool decodeRequest();
    void clearBuffers ();
    void sendData     ();

    QString printCommandInfo(int uniqueCmd, int idCmd, int prevUniqueCmd);

public slots:

    void slotNewConnection      ();
    void slotClientDisconnected ();
    void slotReadyRead          ();
    void tryConnectToMfi2       ();
    void connectedToMfi2        ();

signals:

    void signalNewClientConnected(QString);
    void signalClientDisconnected(QString);
    void signalConnectionToMfi2(bool);
};

#endif // CONTROLLER_H
