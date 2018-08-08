#ifndef DECODERDIALOG_H
#define DECODERDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QAbstractButton>
#include "conf/ModuleSettings.h"
#include "service/DecoderService.h"

namespace Ui {
class DecoderDialog;
}

class DecoderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DecoderDialog(uint windowIdx, uint chnIdx, QWidget *parent = 0);
    ~DecoderDialog();

private slots:
    void lstwdtDecoderList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void buttonBox_clicked(QAbstractButton *button);

private:
    Ui::DecoderDialog *ui;
    uint m_windowIdx;
    ModuleSettings *m_moduleSettings;
    DecoderService *m_decoderService;
    QMap<QString, QWidget *> m_name2WidgetMap;

    void initListWidget();
};

#endif // DECODERDIALOG_H
