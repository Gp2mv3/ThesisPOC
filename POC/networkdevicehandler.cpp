#include "networkdevicehandler.h"

NetworkDeviceHandler::NetworkDeviceHandler(int port)
{
    this->port = port;
    sockVerifier = new QTcpSocket(this);
    sockServer = new QTcpSocket(this);
    sizeMsg = 0;
}



void NetworkDeviceHandler::connectTo(QString addr, bool server)
{

    QTcpSocket* socket = sockVerifier;
    if(server) socket = sockServer;
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(readyRead()), this, SLOT(msgReceived()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    //connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));

    socket->connectToHost(addr, port);

    if(server) sockServer = socket;
    else sockVerifier = socket;

    sizeMsg = 0;
}




void NetworkDeviceHandler::listen()
{
    server = new QTcpServer();
    connect(server, SIGNAL(newConnection()),this, SLOT(newConnection()));
    server->listen(QHostAddress::Any, port);

    QAbstractSocket::SocketError error = server->serverError();
    if(error && server->errorString().length() > 0) emit log(QString("Server Error: %1").arg(server->errorString()));
}

void NetworkDeviceHandler::newConnection()
{
    emit log("New Connection !");

    QTcpSocket *client = server->nextPendingConnection();
    clients << client;

    connect(client, SIGNAL(readyRead()),this, SLOT(msgReceived()));
    connect(client, SIGNAL(disconnected()),this, SLOT(disconnected()));

}

void NetworkDeviceHandler::msgReceived()
{
    sockIn = qobject_cast<QTcpSocket *>(sender());
    if(sockIn == NULL) return;

    emit log(QString("Server receiving... %1 bytes").arg(sockIn->bytesAvailable()));

    QDataStream in(sockIn);
    if (sizeMsg == 0){ //Beginning of a message
        if (sockIn->bytesAvailable() < (int)sizeof(quint16))
            return;
        else
            in >> sizeMsg; //First bytes to message size
    }

    if (sockIn->bytesAvailable() < (int) sizeMsg)
        return;


    QChar type;
    in >> type;

    QByteArray lastMsg;
    in >> lastMsg;

    emit commReceived(sockIn, type, &lastMsg);

    sizeMsg = 0; //Reinit length to zero
}

void NetworkDeviceHandler::broadcast(const QChar type, QByteArray* payload)
{
    for (int i = 0; i < clients.length(); i++)
        sendComm(clients.at(i), type, payload);
}


void NetworkDeviceHandler::sendComm(bool server, const QChar type, QByteArray* payload)
{
    QTcpSocket* sock = sockVerifier;
    if(server) sock = sockServer;
    return sendComm(sock, type, payload);
}


void NetworkDeviceHandler::sendComm(QTcpSocket* socket, const QChar type, QByteArray* payload)
{
    emit log(QString("Send command %1").arg(QString(type)));

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    out << (quint16) 0;
    out << type;
    out.writeBytes(payload->data(), payload->length());
    out.device()->seek(0);
    out << (quint16) (packet.size() - sizeof(quint16));

    emit log(QString("Packet size: %1 bytes").arg(packet.size()));


    socket->write(packet);
}


void NetworkDeviceHandler::disconnectFrom(bool server)
{
    if(server && sockServer != NULL && (sockServer->state() == QTcpSocket::ConnectedState))
        sockServer->disconnectFromHost();
    else if(sockVerifier  != NULL && (sockVerifier->state() == QTcpSocket::ConnectedState))
        sockVerifier->disconnectFromHost();
}


void NetworkDeviceHandler::connected()
{
    emit log("Connected");
}

void NetworkDeviceHandler::disconnected()
{
    emit log("Disconnected");

    QTcpSocket *sock = qobject_cast<QTcpSocket *>(sender());

    if (sock == 0) return;

    clients.removeOne(sock);
    sock->deleteLater();
}

void NetworkDeviceHandler::bytesWritten(qint64 bytes)
{
    emit log(QString("%1 bytes written...").arg(bytes));
}

void NetworkDeviceHandler::readyRead()
{
    emit log("Reading");
}
