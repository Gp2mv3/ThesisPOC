#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //Config
    QObject::connect(ui->butConnectServer, SIGNAL(clicked()), this, SLOT(connectToServer()));
    QObject::connect(ui->butConnectVerifier, SIGNAL(clicked()), this, SLOT(connectToVerifier()));


    //Sign
    QObject::connect(ui->butGetKeys, SIGNAL(clicked()), this, SLOT(getKeys()));
    QObject::connect(ui->butSendMsg, SIGNAL(clicked()), this, SLOT(askMsg()));
    QObject::connect(ui->butOpen, SIGNAL(clicked()), this, SLOT(openLast()));

    //Server
    QObject::connect(ui->butGenKeys, SIGNAL(clicked()), this, SLOT(genKeys()));
    QObject::connect(ui->tableKeys, SIGNAL(clicked(QModelIndex)), this, SLOT(revokeKey(QModelIndex)));


    tableModel = new QStandardItemModel(0,3,this);
    tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("IP Address"));
    tableModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Revoked ?"));
    ui->tableKeys->setModel(tableModel);


    log = new Logger();
    QObject::connect(log, SIGNAL(valueChanged(QVariant)), this, SLOT(addLog(QVariant)));

    log->append("Started !");

    vlr = new VLRWrapper();
    QObject::connect(vlr, SIGNAL(log(QString)), log, SLOT(append(QString)));

    network = new NetworkDeviceHandler(8081);
    network->listen();
    QObject::connect(network, SIGNAL(log(QString)), log, SLOT(append(QString)));
    QObject::connect(network, SIGNAL(commReceived(QTcpSocket*, QChar, QByteArray*)), this, SLOT(commandReceived(QTcpSocket*, QChar, QByteArray*)));


    vlr->generate(0); //Init variables (gpk)

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

void MainWindow::getKeys()
{
    log->append("Get keys");
    QByteArray* d = new QByteArray();
    network->sendComm(true, 'G', d);
}

void MainWindow::askMsg()
{
    log->append("Ask Msg");
    QByteArray* d = new QByteArray();
    network->sendComm(false, 'A', d);
}

void MainWindow::genKeys()
{
    log->append("Gen keys");
    vlr->generate(ui->spinKeys->value());

    tableModel->removeRows(0, tableModel->rowCount());
}

void MainWindow::openLast()
{
    log->append("Ask Opening");
    QByteArray d;
    QDataStream s(&d, QIODevice::WriteOnly);

    quint16 len = lastMessage.length();

    s.writeRawData((const char*) vlr->getLastSign(), vlr->getSignLength());
    s.writeRawData((const char*) &len, sizeof(quint16));
    s.writeRawData(lastMessage.c_str(), len);

    network->sendComm(true, 'O', &d);
}


void MainWindow::commandReceived(QTcpSocket* socket, QChar type, QByteArray* payload)
{
    log->append(QString("Command received: %1").arg(QString(type)));
    if(type == 'G') {

        if(vlr->keysAvailable()) {
            QByteArray data;

            QDataStream* s = new QDataStream(&data, QIODevice::ReadWrite);
            vlr->getNewUsk(s);
            vlr->getGpk(s);

            addItemToTable(tableModel, (int)vlr->getN(), socket->peerAddress().toString(), false);

            network->sendComm(socket, 'g', &data);
        }
        else {
            QMessageBox::warning(this, "Keys not generated !", "Key is requested but none have been generated.");
        }
    }
    else if (type == 'g') {
        QDataStream* s = new QDataStream(payload, QIODevice::ReadWrite);

        vlr->setUsk(s);
        vlr->setGpk(s);
    }
    else if (type == 'A') {
        QByteArray data;

        // Generate pseudo random message
        QString str;
        str.resize(100);
        for (int i = 0; i < 100 ; i++)
            str[i] = QChar((qrand()%128));

        data.append(str);
        lastMessage = data.toStdString();

        network->sendComm(socket, 'a', &data);
    }
    else if (type == 'a') {

        QByteArray data;
        QDataStream* s = new QDataStream(&data, QIODevice::ReadWrite);

        lastMessage = payload->toStdString();

        unsigned char* sign = vlr->sign(payload->toStdString());
        if(!sign)
        {
            log->append("Malloc sign error !");
            return;
        }

        s->writeRawData((const char*) sign, vlr->getSignLength());

        network->sendComm(socket, 'S', &data);
    }
    else if (type == 'S') {
        QDataStream* s = new QDataStream(payload, QIODevice::ReadWrite);

        unsigned char* sig = (unsigned char*) malloc(vlr->getSignLength());
        if(!sig) {
            log->append(QString("Malloc error sig !"));
            return;
        }
        s->readRawData((char*) sig, vlr->getSignLength());

        int verifRes = vlr->verif(lastMessage, sig);
        free(sig);


        log->append(QString("Verif state: %1").arg(verifRes));
        QByteArray data;
        QDataStream in(&data, QIODevice::WriteOnly);
        in << (qint8) verifRes;

        network->sendComm(socket, 's', &data);

        if(verifRes == 0)
            QMessageBox::critical(this, "Signature rejected", "The signature is not valid");
        else if (verifRes == -1)
            QMessageBox::critical(this, "Identity revoked !", "This group-member is revoked.");
        else
            QMessageBox::information(this, "Signature validated", "Signature is checked and valid !");
    }
    else if (type == 's') {
        QDataStream s(payload, QIODevice::ReadWrite);

        qint8 verifRes;
        s >> verifRes;

        if(verifRes == 0)
            QMessageBox::critical(this, "Signature rejected", "The signature is not valid");
        else if (verifRes == -1)
            QMessageBox::critical(this, "Identity revoked !", "This group-member is revoked.");
        else
            QMessageBox::information(this, "Signature validated", "Signature is checked and valid !");
    }
    else if (type == 'R') {
        QDataStream s(payload, QIODevice::ReadOnly);

        vlr->addToRL(&s);


        QByteArray data;
        network->sendComm(socket, 'r', &data);
    }
    else if (type == 'O') {
        QDataStream s(payload, QIODevice::ReadOnly);

        int id = vlr->open(&s);

        log->append(QString("Open ID: %1").arg(id));

        if(id >= 0){
            QMessageBox::information(this, "Open successful !", QString("User that signed this message has ID %1").arg(id));
        }

        QByteArray data;
        data.append(id);
        network->sendComm(socket, 'o', &data);
    }
    else {

    }
}

void MainWindow::connectToServer()
{
    network->disconnectFrom(true);
    network->connectTo(ui->txtServerIp->text(), true);
}

void MainWindow::connectToVerifier()
{
    network->disconnectFrom(false);
    network->connectTo(ui->txtVerifierIp->text(), false);
}


void MainWindow::addLog(QVariant log)
{
    ui->txtLog->append(log.toString());
}

void MainWindow::addItemToTable(QStandardItemModel* model, int id, QString ip, bool revoked)
{
    QStandardItem* itemID = new QStandardItem();
    QStandardItem* itemIP = new QStandardItem();
    QStandardItem* itemState = new QStandardItem();


    itemID->setText(QString("%1").arg(id));
    itemID->setData(id);

    itemIP->setText(ip);

    if(revoked) itemState->setText("Yes");
    else        itemState->setText("No");

    QList<QStandardItem*> row;

    row.append(itemID);
    row.append(itemIP);
    row.append(itemState);

    model->appendRow(row);
}

void MainWindow::revokeKey(QModelIndex index)
{
    log->append(QString(index.row()));
    QStandardItem* item = tableModel->item(index.row(), 0);

    int id = item->data().toInt();
    log->append(QString("Revoke key : %1").arg(id));


    int ret = QMessageBox::question(this, "Revoke this key ?", QString("Do you really want to revoke key %1 ?").arg(id));
    log->append(QString("Revoke question : %1").arg(ret));


    if(ret == QMessageBox::Yes){
        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);

        vlr->getForRL(id, &s);
        network->broadcast('R', &data);

        tableModel->item(index.row(), 2)->setText("Yes");

        //Also add to local RL
        QDataStream s2(&data, QIODevice::ReadOnly);
        vlr->addToRL(&s2);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
