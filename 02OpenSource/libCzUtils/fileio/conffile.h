#ifndef CONFFILE_H
#define CONFFILE_H
#include "library_global.h"
#include <QHash>
#include <QString>

class LIBRARY_API ConfFile
{
public:
    struct T_LineContent {
        QString Original;
        QString Key;
        QString Value;
        QString Comment;
        T_LineContent() {
            Original = Key = Value = Comment = "";
        }
        T_LineContent(const QString &key, const QString &value, const QString &comment) {
            Key = key;
            Value = value;
            Comment = comment;
        }
    };
    ConfFile(const QString &fileName = "");

    void SetFileName(const QString &fileName);
    bool LoadConf();
    bool LoadConf(QList<T_LineContent> &lineContentList);
    bool SaveConf();
    bool SaveConf(const QList<T_LineContent> &lineContentList);
    QString GetValue(const QString &key);
    void SetValue(const QString &key, const QString &value);

private:
    QList<T_LineContent> m_lineContentList;
    QString m_fileName;

    bool loadToStringList(QStringList &lineList);
    bool saveFromStringList(const QStringList &lineList);
};

#endif // CONFFILE_H
