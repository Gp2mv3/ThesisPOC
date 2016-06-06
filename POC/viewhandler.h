#ifndef VIEWHANDLER_H
#define VIEWHANDLER_H

#include <QObject>
#include <QDebug>
#include <QString>

#include <string>

class ViewHandler : public QObject
{
    Q_OBJECT

public:
    explicit ViewHandler(QObject *parent = 0);


signals:
    void newLog(QString txt);

private:
    QString msg;
    unsigned char* sig;

    Logger *log;
    VLRWrapper *vlr;
    NfcHandler *nfc;
    NetworkDeviceHandler *network;

    QString lastLog;


public slots:
    void genKeys();
    void genMsg();
    void signMsg();
    void checkSign();
    void setReading(bool r);

    void logAppend(QString value);
};

#endif // VIEWHANDLER_H
