#include "vlrWrapper.h"

VLRWrapper::VLRWrapper()
{
#ifdef __ANDROID_API__
    QFile file("assets:/params/d201.param");
#else
    QFile file("./params/d201.param");
#endif

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit log("File open error !");
        return;
    }

    char param[2048];
    size_t count = file.read(param, 2048);
    emit log(QString("VLR params: file read (%1 bytes)").arg(count));

    if (!count){
        emit log(QString("VLR params: Input error"));
        pbc_die("input error");
    }
    pairing_init_set_buf(pairing, param, count);

    vlr_gen_sys_param(sp, pairing);

    uskLocal = (vlr_user_private_key_s*) malloc(sizeof(vlr_user_private_key_s));
    if(!uskLocal) emit log("Error uskLocal malloc !");

    uskLocal->param = sp;
    element_init_G1(uskLocal->A, pairing);
    element_init_Zr(uskLocal->x, pairing);

    gpkInit = false;
    RLlen = 0;
    RLinit = false;
    nbKeys = 0;
    n = 0;

    lastSign = (unsigned char*)malloc(sp->signature_length);
}

int VLRWrapper::generate(int N)
{
    emit log(QString("Generation of %1 keys... This can take a while.").arg(N));

    nbKeys = N;

    grt = (vlr_group_revocation_token_t*) malloc(N*sizeof(vlr_group_revocation_token_t));
    if(!grt) return 1;

    usk = (vlr_user_private_key_t*) malloc(N*sizeof(vlr_user_private_key_t));
    if(!usk) return 2;

    RL = (vlr_revocation_list_t*) malloc(N*sizeof(vlr_revocation_list_t));
    if(!RL) return 3;

    vlr_gen(gpk, grt, N, usk, sp);
    gpkInit = true;
    RLinit = true;

    n = -1;

    emit log(QString("Keys generated !"));

    return 0;
}

/**
  Get a new us to transmit to a new user (executed server-side)
 * @brief VLRWrapper::getNewUsk
 * @param stream
 */
void VLRWrapper::getNewUsk(QDataStream* stream)
{
    n++;

    quint16 len = (quint16) element_length_in_bytes(usk[n]->A);
    unsigned char* tmp = (unsigned char*) malloc(len*sizeof(unsigned char));
    if(!tmp) return;

    if(element_to_bytes(tmp, usk[n]->A) != len){
        free(tmp);
        return;
    }
    else {
        stream->writeRawData((const char*) &len, sizeof(quint16));
        stream->writeRawData((const char*) tmp, len*sizeof(unsigned char));
    }

    len = (quint16) element_length_in_bytes(usk[n]->x);

    tmp = (unsigned char*) realloc(tmp, len);
    if(!tmp) return;

    if(element_to_bytes(tmp, usk[n]->x) != len){
        free(tmp);
        return;
    }
    else {
        stream->writeRawData((const char*) &len, sizeof(quint16));
        stream->writeRawData((const char*) tmp, len*sizeof(unsigned char));
    }
    free(tmp);
}

/**
  Get gpk
 * @brief VLRWrapper::getGpk
 * @param stream
 */
void VLRWrapper::getGpk(QDataStream* stream)
{
    quint16 len = (quint16) element_length_in_bytes(gpk->g1);
    unsigned char* tmp = (unsigned char*) malloc(len*sizeof(unsigned char));
    if(!tmp) return;

    if(element_to_bytes(tmp, gpk->g1) != len){
        free(tmp);
        return;
    }
    else {
        stream->writeRawData((const char*) &len, sizeof(quint16));
        stream->writeRawData((const char*) tmp, len*sizeof(unsigned char));
    }

    len = (quint16) element_length_in_bytes(gpk->g2);
    tmp = (unsigned char*) realloc(tmp, len);
    if(!tmp) return;

    if(element_to_bytes(tmp, gpk->g2) != len){
        free(tmp);
        return;
    }
    else {
        stream->writeRawData((const char*) &len, sizeof(quint16));
        stream->writeRawData((const char*) tmp, len*sizeof(unsigned char));
    }


    len = (quint16) element_length_in_bytes(gpk->omega);
    tmp = (unsigned char*) realloc(tmp, len);
    if(!tmp) return;

    if(element_to_bytes(tmp, gpk->omega) != len){
        free(tmp);
        return;
    }
    else {
        stream->writeRawData((const char*) &len, sizeof(quint16));
        stream->writeRawData((const char*) tmp, len*sizeof(unsigned char));
    }
    free(tmp);
}

/**
  Set received usk (client-side)
 * @brief VLRWrapper::setUsk
 * @param stream
 */
void VLRWrapper::setUsk(QDataStream* stream)
{
    quint16 len = 0;
    stream->readRawData((char*)&len, sizeof(quint16));
    unsigned char* tmp = (unsigned char*) malloc(len);
    stream->readRawData((char*)tmp, len);
    element_from_bytes(uskLocal->A, tmp);

    stream->readRawData((char*)&len, sizeof(quint16));
    tmp = (unsigned char*) realloc(tmp, len);
    stream->readRawData((char*) tmp, len);
    element_from_bytes(uskLocal->x, tmp);

    free(tmp);
}

/**
  Set received gpk (client-side)
 * @brief VLRWrapper::setGpk
 * @param stream
 */
void VLRWrapper::setGpk(QDataStream* stream)
{

    if(!gpkInit) {
        gpk->param = sp;
        element_init_G1(gpk->g1, pairing);
        element_init_G2(gpk->g2, pairing);
        element_init_G2(gpk->omega, pairing);
        gpkInit = true;
    }

    quint16 len = 0;
    stream->readRawData((char*)&len, sizeof(quint16));

    unsigned char* tmp = (unsigned char*) malloc(len);
    stream->readRawData((char*)tmp, len);
    element_from_bytes(gpk->g1, tmp);

    stream->readRawData((char*)&len, sizeof(quint16));
    tmp = (unsigned char*) realloc(tmp, len);
    stream->readRawData((char*) tmp, len);
    element_from_bytes(gpk->g2, tmp);


    stream->readRawData((char*)&len, sizeof(quint16));
    tmp = (unsigned char*) realloc(tmp, len);
    stream->readRawData((char*) tmp, len);
    element_from_bytes(gpk->omega, tmp);

    free(tmp);
}


unsigned char* VLRWrapper::sign(std::string msg)
{
    emit log(QString("Signing message... "));

    vlr_sign(lastSign, msg.length(), (unsigned char*) msg.c_str(), gpk, uskLocal);

    emit log(QString("Message signed !"));
    return lastSign;
}


int VLRWrapper::verif(std::string msg, unsigned char* sign)
{
    emit log(QString("Verify signature"));

    int ret =  vlr_verify(sign, msg.length(), (unsigned char*) msg.c_str(), gpk, RL, RLlen);

    memcpy(lastSign, sign, getSignLength());
    return ret;
}

quint16 VLRWrapper::getSignLength()
{
    return sp->signature_length;
}

quint32 VLRWrapper::getN()
{
    return (quint32) n;
}

void VLRWrapper::getForRL(int id, QDataStream* stream)
{
    quint16 len = (quint16) element_length_in_bytes(grt[id]->A);
    unsigned char* tmp = (unsigned char*) malloc(len*sizeof(unsigned char));
    if(!tmp) return;

    if(element_to_bytes(tmp, grt[id]->A) != len){
        free(tmp);
        return;
    }
    else {
        stream->writeRawData((const char*) &len, sizeof(quint16));
        stream->writeRawData((const char*) tmp, len*sizeof(unsigned char));
    }

    free(tmp);
}

void VLRWrapper::addToRL(QDataStream* stream)
{
    if(!RLinit) {
        RL = (vlr_revocation_list_t*) malloc(RLMAXLEN*sizeof(vlr_revocation_list_t));
        if(!RL) {
            emit log("Error RL malloc");
            return;
        }
        RLinit = true;
    }

    quint16 len = 0;
    stream->readRawData((char*)&len, sizeof(quint16));
    unsigned char* tmp = (unsigned char*) malloc(len);
    if(!tmp) {
        emit log("Error tmp malloc");
        return;
    }

    stream->readRawData((char*)tmp, len);

    element_init_G1(RL[RLlen]->A, pairing);
    RL[RLlen]->param = sp;

    element_from_bytes(RL[RLlen]->A, tmp);

    RLlen++;
    free(tmp);
}

int VLRWrapper::open(QDataStream* stream)
{
    vlr_revocation_list_t* locRL = (vlr_revocation_list_t*) malloc(sizeof(vlr_revocation_list_t));

    unsigned char* sig = (unsigned char*) malloc(getSignLength());
    if(!sig) {
        emit log(QString("Error sig malloc"));
    }

    stream->readRawData((char*) sig, getSignLength());

    quint16 msgLen = 0;
    stream->readRawData((char*) &msgLen, sizeof(quint16));
    unsigned char* msg = (unsigned char*) malloc(msgLen);
    stream->readRawData((char*) msg, msgLen);

    locRL[0]->param = sp;

    int res = -1;
    for(int i = 0; i <= n; i++)
    {
        memcpy((void*) locRL[0]->A, (void*) grt[i]->A, sizeof(element_t));
        if(vlr_verify(sig, (int)msgLen, msg, gpk, locRL, 1) == -1) {
            res = i;
            break;
        }
    }

    free(sig);
    free(msg);
    return res;
}

bool VLRWrapper::keysAvailable()
{
    return (n < nbKeys && nbKeys > 0);
}

unsigned char* VLRWrapper::getLastSign()
{
    return lastSign;
}

VLRWrapper::~VLRWrapper()
{

}
