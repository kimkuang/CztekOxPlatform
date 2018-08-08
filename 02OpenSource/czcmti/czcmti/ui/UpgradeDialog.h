#ifndef UPGRADEDIALOG_H
#define UPGRADEDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "conf/SystemSettings.h"

namespace Ui {
class UpgradeDialog;
}

class UpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeDialog(QWidget *parent = 0);
    ~UpgradeDialog();

private slots:
    void btnCheckVersion_clicked();
    void btnUpgrade_clicked();
    void btnRevert_clicked();
    void btnClose_clicked();

private:
    Ui::UpgradeDialog *ui;
    SystemSettings *m_sysSettings;
    QString m_upgraderConfFile;
//    struct T_UpgradeParam {
//        enum E_PacketType {
//            PacketType_Directory = 0,
//            PacketType_TarGzip,
//        };
//        E_PacketType PacketType;
//        bool PartialUpdate;
//    } m_upgradeParam;
//    T_UpgradeParam m_upgradeParam;

    void initUi();
};

#endif // UPGRADEDIALOG_H
