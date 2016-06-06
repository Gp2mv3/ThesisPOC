#ifndef NETWORKDEVICEHANDLER_H
#define NETWORKDEVICEHANDLER_H

#define TIMEOUT 5000


#include <QObject>
#include <QtNetwork>
#include <QDebug>

class NetworkDeviceHandler : public QObject
{
    Q_OBJECT
public:
    explicit NetworkDeviceHandler(int port);

    void connectTo(QString addr, bool server);
    void disconnectFrom(bool server);

    void listen();

signals:
    void commReceived(QTcpSocket*, QChar type, QByteArray* data);
    void log(QString);

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
    void newConnection();
    void msgReceived();
    void sendComm(bool server, const QChar type, QByteArray* payload);
    void sendComm(QTcpSocket* socket, const QChar type, QByteArray* payload);
    void broadcast(const QChar type, QByteArray* payload);

private:
    QTcpSocket *sockServer;
    QTcpSocket *sockVerifier;

    QTcpServer *server;
    QList<QTcpSocket *> clients;

    quint16 sizeMsg;
    char typeMsg;
    QString* payload;
    QTcpSocket* sockIn;


    QString addr;
    QString toSend;

    int port;
};

#endif // NETWORKDEVICEHANDLER_H
