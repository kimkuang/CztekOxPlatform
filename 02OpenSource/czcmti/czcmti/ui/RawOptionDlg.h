#ifndef RAWOPTIONDLG_H
#define RAWOPTIONDLG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class RawOptionDlg;
}

class RawOptionDlg : public QDialog
{
    Q_OBJECT

public:
    explicit RawOptionDlg(uint rawSize, QWidget *parent = 0);
    ~RawOptionDlg();

    void Cache2Ui(void);

public:
    uint m_rawWidth;
    uint m_rawHeight;
    uint m_rawBits;
    uint m_rawOutFormat;
    uint m_rawShowType;
    uint m_rawSize;

private:
    void OnBtnBoxClick(QAbstractButton *button);

private:
    Ui::RawOptionDlg *ui;
};

#endif // RAWOPTIONDLG_H
