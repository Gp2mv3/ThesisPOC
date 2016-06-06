#include "logger.h"

Logger::Logger(QObject *parent) : QObject(parent)
{
    buf = new QString();
}

void Logger::append(QString str)
{
    buf->append(str);
    qDebug() << str;
    emit valueChanged(str);
}

void Logger::append(const char *str)
{
    return this->append(QString(str));
}

