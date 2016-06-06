#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QDebug>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);

signals:
    void valueChanged(QVariant value);

public slots:
    void append(QString str);
    void append(const char* str);

public:
    QString *buf;
};

#endif // LOGGER_H
