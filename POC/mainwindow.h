#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDataStream>
#include <QMessageBox>

#include "logger.h"
#include "vlrWrapper.h"
#include "networkdevicehandler.h"
#include "keys.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void getKeys(void);
    void genKeys(void);
    void askMsg(void);


    void addLog(QVariant);
    void connectToServer();
    void connectToVerifier();
    void commandReceived(QTcpSocket* socket, QChar type, QByteArray* payload);
    void revokeKey(QModelIndex index);
    void openLast();


private:
    Ui::MainWindow *ui;
    QPushButton *butGetKeys;

    Logger *log;
    VLRWrapper *vlr;
    NetworkDeviceHandler *network;


    QStandardItemModel* tableModel;
    std::string lastMessage;

    void addItemToTable(QStandardItemModel* model, int id, QString ip, bool revoked);

};

#endif // MAINWINDOW_H
