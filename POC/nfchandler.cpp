#include "nfchandler.h"

NfcHandler::NfcHandler(QObject *parent) : QObject(parent)
{

    manager = new QNearFieldManager(this);
    if (!manager->isAvailable()) {
        log("NFC not available");
        return;
    }
    manager->registerNdefMessageHandler(this, SLOT(handleNdefMessage(const QNdefMessage,QNearFieldTarget*)));
    manager->setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess | QNearFieldManager::NdefReadTargetAccess);

/*
    shareManager = new QNearFieldShareManager(this);
    shareManager->setShareModes(QNearFieldShareManager::NdefShare);
*/
}

void NfcHandler::startListen()
{
/*
    connect(shareManager, SIGNAL(targetDetected(QNearFieldShareTarget*)), this, SLOT(targetDetected(QNearFieldShareTarget*)));
    connect(shareManager, SIGNAL(targetLost(QNearFieldShareTarget*)), this, SLOT(targetLost(QNearFieldShareTarget*)));
*/

    connect(manager, SIGNAL(targetLost(QNearFieldTarget*)), this, SLOT(targetLost(QNearFieldTarget*)));
    connect(manager, SIGNAL(targetDetected(QNearFieldTarget*)), this, SLOT(targetDetected(QNearFieldTarget*)));

    manager->startTargetDetection();

}

void NfcHandler::stopListen()
{
    manager->stopTargetDetection();
}

void NfcHandler::targetDetected(QNearFieldShareTarget* target)
{
    log("target detected SHARE");

    //target->deleteLater();
}

void NfcHandler::targetLost(QNearFieldShareTarget* target)
{
    log("target lost SHARE");
    target->deleteLater();
}



void NfcHandler::targetDetected(QNearFieldTarget* target)
{
    log("target detected");

    QNearFieldTarget::RequestId m_request;

    if(!reading) {
        connect(target, SIGNAL(ndefMessagesWritten()), this, SLOT(ndefMessageWritten()));
        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));


        QNdefMessage ndefMessage;
        ndefMessage.fromByteArray("Hello World");
        m_request = target->writeNdefMessages(QList<QNdefMessage>() << ndefMessage);

        if (!m_request.isValid()) // cannot write messages
        {
            log("error HERE");
            emit targetError(QNearFieldTarget::NdefWriteError, m_request);
        }
    }
    else
    {
        connect(target, SIGNAL(ndefMessageRead(QNdefMessage)), this, SLOT(ndefMessageRead(QNdefMessage)));
        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        m_request = target->readNdefMessages();
        if (!m_request.isValid()) // cannot read messages
            targetError(QNearFieldTarget::NdefReadError, m_request);
    }
    //target->deleteLater();
}

void NfcHandler::targetLost(QNearFieldTarget* target)
{
    log("target lost ");
    target->deleteLater();
}


void NfcHandler::handleNdefMessage(QNdefMessage msg,QNearFieldTarget* target)
{
    log("NDEF from device received");
    log("UID: " + target->uid());


    foreach (const QNdefRecord &record, msg)
    {
        if (record.isRecordType<QNdefNfcTextRecord>())
        {
            QNdefNfcTextRecord textRecord(record);

            QString title = textRecord.text();
            //QLocale locale(textRecord.locale());
            log("READ title: "+title);

        }
        else if (record.isRecordType<QNdefNfcUriRecord>())
        {
            QNdefNfcUriRecord uriRecord(record);

            log("READ url: ");
            log(uriRecord.uri().toString());
        }
    }
}

void NfcHandler::targetError(QNearFieldTarget::Error error,QNearFieldTarget::RequestId reqId)
{
    log(QString("targetError %1").arg(error));
}

void NfcHandler::ndefMessageWritten()
{
    log("Message Written");
}

void NfcHandler::ndefMessageRead(QNdefMessage msg)
{
    log("ndef Received");
    log(msg.at(0).payload());
}


void NfcHandler::setReading(bool r)
{
    log(QString("Reading set %1").arg(r));

    reading = r;
}


void NfcHandler::log(QString msg)
{
    emit logAdded(msg);
}
