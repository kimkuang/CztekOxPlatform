#ifndef SENSORINI_H
#define SENSORINI_H

#include "library_global.h"
#include "entities/sensor.h"
#include <QString>
#include <QFile>
#include <QTextStream>

class LIBRARY_API SensorIni : public QObject
{
    Q_OBJECT
public:
    explicit SensorIni(QObject *parent = 0);
    ~SensorIni();

    bool SaveTemplateToFile(const QString &fileName);
    bool SaveToFile(const Sensor &sensor, const QString &fileName);
    bool LoadFromFile(Sensor &sensor, const QString &fileName);
    void FileVersion(QString &fileVersion);

signals:

public slots:

private:
    QFile m_file;
    QTextStream m_textStream;
    QString m_fileVersion;

    bool generateCommentPart(QString &commentPart);
    bool getSection(const QString &fileContent, const QString &sectionName, QString &sectionContent);
};

#endif // SENSORINI_H
