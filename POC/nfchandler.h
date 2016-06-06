#ifndef NFCHANDLER_H
#define NFCHANDLER_H

#include <QObject>
#include <QDebug>
#include <QtNfc>

class NfcHandler : public QObject
{
    Q_OBJECT
public:
    explicit NfcHandler(QObject *parent = 0);
    void startListen();
    void stopListen();

    void setReading(bool r);

private:
    QNearFieldShareManager* shareManager;
    QNearFieldManager* manager;
    bool reading;

    void log(QString);


signals:
    void logAdded(QString);

public slots:
    void targetDetected(QNearFieldShareTarget* target);
    void targetLost(QNearFieldShareTarget* target);
    void handleNdefMessage(QNdefMessage msg,QNearFieldTarget* target);

    void targetDetected(QNearFieldTarget* target);
    void targetLost(QNearFieldTarget* target);
    //void handleNdefMessage(QNdefMessage msg,QNearFieldTarget* target);

    void targetError(QNearFieldTarget::Error error,QNearFieldTarget::RequestId reqId);
    void ndefMessageWritten();
    void ndefMessageRead(QNdefMessage msg);
};

#endif // NFCHANDLER_H
