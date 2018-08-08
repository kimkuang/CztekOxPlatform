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
    ui->m_spinListenPort1->setValue(m_option->ListenPort1);
    ui->m_spinListenPort2->setValue(m_option->ListenPort2);
    ui->m_spinListenPort3->setValue(m_option->ListenPort3);
    ui->m_spinListenPort4->setValue(m_option->ListenPort4);
    ui->m_edtHandshakeRequest->setText(m_option->HandshakeRequest);
    ui->m_edtHandshakeResponse->setText(m_option->HandshakeResponse);

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->ListenIp = ui->m_cbxListenIp->currentText();
    m_option->ListenPort1 = ui->m_spinListenPort1->value();
    m_option->ListenPort2 = ui->m_spinListenPort2->value();
    m_option->ListenPort3 = ui->m_spinListenPort3->value();
    m_option->ListenPort4 = ui->m_spinListenPort4->value();
    m_option->HandshakeRequest = ui->m_edtHandshakeRequest->text();
    m_option->HandshakeResponse = ui->m_edtHandshakeResponse->text();

    return ERR_NoError;
}

void ConfWidget::initComboBoxListenIp()
{
//    QString name = QHostInfo::localHostName();
//    if (!name.isEmpty()) {
//        ui->m_cbxListenIp->addItem(name);
//        QString domain = QHostInfo::localDomainName();
//        if (!domain.isEmpty())
//            ui->m_cbxListenIp->addItem(name + QChar('.') + domain);
//    }
//    if (name != QLatin1String("localhost"))
//        ui->m_cbxListenIp->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback() &&
                (ipAddressesList.at(i).protocol() == QAbstractSocket::IPv4Protocol))
            ui->m_cbxListenIp->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
//    for (int i = 0; i < ipAddressesList.size(); ++i) {
//        if (ipAddressesList.at(i).isLoopback()&&
//                (ipAddressesList.at(i).protocol() == QAbstractSocket::IPv4Protocol))
//            ui->m_cbxListenIp->addItem(ipAddressesList.at(i).toString());
//    }
}

