#ifndef KEYS_H
#define KEYS_H

#include <QObject>
#include <QVariant>

class Keys : public QObject
{
public:
    explicit Keys();

    QByteArray gpk;
    QByteArray usk;
    QByteArray RL;

};
/*
QDataStream &operator<<(QDataStream &out, const Keys &keys);
QDataStream &operator>>(QDataStream &in, Keys &keys);
*/

#endif // KEYS_H
