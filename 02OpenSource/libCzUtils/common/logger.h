#ifndef LOGGER_H
#define LOGGER_H

#include <QMessageLogContext>
#include <QString>
#include <QMutex>
#include "library_global.h"

class LIBRARY_API Logger
{
public:
    static void StandardOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void FileOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void StandardFileOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static QMutex m_mutex;
    Logger() {}

    static QString formatMessage(QString &formattedMessage, QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // LOGGER_H
