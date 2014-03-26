#include "SettingDialog.h"
#include "ui_SettingDialog.h"
#include <QtNetwork>
#include <QList>

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        ui->hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty()) {
            ui->hostCombo->addItem(name + QChar('.') + domain);
        }
    }
    if (name != QString("localhost")) {
        ui->hostCombo->addItem(QString("localhost"));
    }
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback()) {
            ui->hostCombo->addItem(ipAddressesList.at(i).toString());
        }
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback()) {
            ui->hostCombo->addItem(ipAddressesList.at(i).toString());
        }
    }

    ui->portNumberSpinBox->setFocus();
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::on_connectButton_clicked()
{
    QString hostName = ui->hostCombo->currentText();
    int portNumber = ui->portNumberSpinBox->value();
    emit signalHostNameAndPortNumber(hostName, portNumber);
    close();
}

void SettingDialog::on_cancelButton_clicked()
{
    close();
}
