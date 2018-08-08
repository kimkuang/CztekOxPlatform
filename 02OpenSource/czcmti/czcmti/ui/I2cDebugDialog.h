#ifndef I2CDEBUGDIALOG_H
#define I2CDEBUGDIALOG_H
#include <QDialog>
#include "conf/ModuleSettings.h"
#include "ICzPlugin.h"

namespace Ui {
    class I2cDebugDialog;
}

class I2cDebugDialog : public QDialog
{
    Q_OBJECT

    enum I2cMode
    {
        NORMAL = 0,
        REG8DATA8,
        REG8DATA16,
        REG16DATA8,
        REG16DATA16,
    };

public:
    explicit I2cDebugDialog(uint windowIdx, uint chnIdx, T_ChannelContext *context, QWidget *parent = 0);
    ~I2cDebugDialog(void);

public slots:
    void OnBtnReadI2c(void);
    void OnBtnWriteI2c(void);
    void OnBtnSaveAEAndGain(void);
    void OnBtnSetCode(void);
    void OnBtnFindI2cAddr(void);
    void OnBtnGetFuseID(void);
    void OnBtnDump(void);
    void OnSliderAEValChange(void);
    void OnSliderGainValChange(void);
    void OnEditAEValChange(void);
    void OnEditGainValChange(void);
    void OnComboIndexChange(int index);

private:
    void UpdateUI(void);
    void Connect(void);

private:
    Ui::I2cDebugDialog *ui;
    uint m_chnIdx;
    ModuleSettings* m_pModuleSettings;
    T_ChannelContext* m_pContext;
};

#endif // I2CDEBUGDIALOG_H
