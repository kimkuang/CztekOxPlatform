#include "RawOptionDlg.h"
#include "ui_RawOptionDlg.h"
#include <QMessageBox>

RawOptionDlg::RawOptionDlg(uint rawSize, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RawOptionDlg)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &RawOptionDlg::OnBtnBoxClick);
    ui->comboBoxRawBits->addItem(QString("Image Raw8"));
    ui->comboBoxRawBits->addItem(QString("Image Raw10"));
    ui->comboBoxRawBits->addItem(QString("Image Raw12"));
    ui->comboBoxRawBits->addItem(QString("Image Raw14"));
    ui->comboBoxRawBits->addItem(QString("Image Raw16"));

    ui->comboBoxOutFormat->addItem(QString("RGGB"));
    ui->comboBoxOutFormat->addItem(QString("GRBG"));
    ui->comboBoxOutFormat->addItem(QString("GBRG"));
    ui->comboBoxOutFormat->addItem(QString("BGGR"));

    ui->comboBoxShowType->addItem(QString("Show Color"));
    ui->comboBoxShowType->addItem(QString("Show Gray"));
    ui->comboBoxShowType->addItem(QString("Show RGB24"));

    m_rawBits = 0;
    m_rawOutFormat = 0;
    m_rawShowType = 0;
    m_rawSize = rawSize;
    Cache2Ui();
}

RawOptionDlg::~RawOptionDlg()
{
    delete ui;
}

void RawOptionDlg::Cache2Ui()
{
    ui->lineEditRawHeight->setText(QString("%1").arg(m_rawHeight));
    ui->lineEditRawWidth->setText(QString("%1").arg(m_rawWidth));
    ui->comboBoxRawBits->setCurrentIndex(m_rawBits);
    ui->comboBoxOutFormat->setCurrentIndex(m_rawOutFormat);
    ui->comboBoxShowType->setCurrentIndex(m_rawShowType);
    ui->lineEditSize->setText(QString("%1").arg(m_rawSize));
}

void RawOptionDlg::OnBtnBoxClick(QAbstractButton *button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok)
    {
        m_rawWidth = ui->lineEditRawWidth->text().toInt();
        m_rawHeight = ui->lineEditRawHeight->text().toInt();
        m_rawBits = ui->comboBoxRawBits->currentIndex();
        m_rawOutFormat = ui->comboBoxOutFormat->currentIndex();
        m_rawShowType = ui->comboBoxShowType->currentIndex();
    }
}
