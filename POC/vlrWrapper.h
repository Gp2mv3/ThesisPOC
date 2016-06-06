#ifndef VLRWRAPPER_H
#define VLRWRAPPER_H

#include <QDebug>
#include <QObject>
#include <QFile>
#include <QDataStream>


#define RLMAXLEN 1000

#include <stdio.h>
#include "lib/inc/vlr.h"


class VLRWrapper : public QObject
{
    Q_OBJECT


public:
    explicit VLRWrapper();
    ~VLRWrapper();


    int generate(int N);

    unsigned char* sign(std::string msg);
    int verif(std::string msg, unsigned char*);

    void getNewUsk(QDataStream* stream);
    void setUsk(QDataStream* stream);

    void setGpk(QDataStream* stream);
    void getGpk(QDataStream* stream);

    void getForRL(int id, QDataStream* stream);
    void addToRL(QDataStream* stream);

    int open(QDataStream* stream);


    unsigned char* getLastSign();

    quint16 getSignLength();
    quint32 getN();
    bool keysAvailable();



signals:
    void log(QString);


private:
    pairing_t pairing;

    vlr_sys_param_t sp;
    vlr_group_public_key_t gpk;

    vlr_group_revocation_token_t *grt;
    vlr_user_private_key_t *usk;
    vlr_user_private_key_s *uskLocal;

    vlr_revocation_list_t *RL;
    int RLlen;

    bool gpkInit;
    bool RLinit;
    int n;
    int nbKeys;

    unsigned char* lastSign;
};

#endif // VLR_H
