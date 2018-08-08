#include "ConfWidget.h"
#include "ui_ConfWidget.h"
#include <QHostInfo>
#include <QNetworkInterface>

ConfWidget::ConfWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfWidget)
{
    ui->setupUi(this);
    initComboBoxListenIp();

    m_option = Option::GetInstance();
}

ConfWidget::~ConfWidget()
{
    delete ui;
    m_option = nullptr;
}

int ConfWidget::RestoreDefaults()
{
    m_option->RestoreDefaults();
    Cache2Ui();
    return ERR_NoError;
}

int ConfWidget::Cache2Ui()
{
    ui->m_cbxListenIp->setCurrentText(m_option->ListenIp);
    ui->m_edtForwardIp->setText(m_option->ForwardIp);
    ui->m_spinListenPort->setValue(m_option->ListenPort);
    ui->m_spinForwardPort->setValue(m_option->ForwardPort);
    ui->m_edtHandshakeRequest->setText(m_option->HandshakeRequest);
    ui->m_edtHandshakeResponse->setText(m_option->HandshakeResponse);

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->ListenIp = ui->m_cbxListenIp->currentText();
    m_option->ForwardIp = ui->m_edtForwardIp->text().trimmed();
    m_option->ListenPort = ui->m_spinListenPort->value();
    m_option->ForwardPort = ui->m_spinForwardPort->value();
    m_option->HandshakeRequest = ui->m_edtHandshakeRequest->text().trimmed();
    m_option->HandshakeResponse = ui->m_edtHandshakeResponse->text().trimmed();

    return ERR_NoError;
}

void ConfWidget::initComboBoxListenIp()
{
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback() &&
                (ipAddressesList.at(i).protocol() == QAbstractSocket::IPv4Protocol))
            ui->m_cbxListenIp->addItem(ipAddressesList.at(i).toString());
    }
}

