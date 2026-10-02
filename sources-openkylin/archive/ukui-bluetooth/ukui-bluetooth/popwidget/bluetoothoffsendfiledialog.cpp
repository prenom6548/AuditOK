#include "bluetoothoffsendfiledialog.h"

#include <QIcon>
#include <QLabel>
#include <ukui-log4qt.h>

#include "config/config.h"


BluetoothOffSendFileDialog::BluetoothOffSendFileDialog()
{
    KyInfo();
    m_cancel = new QPushButton(tr("Keep Bluetooth turned off"), this);
    m_ok = new QPushButton(tr("Enable Bluetooth"), this);
    this->addButton(m_cancel, QMessageBox::RejectRole);
    this->addButton(m_ok, QMessageBox::AcceptRole);

    m_ok->setDefault(true);

    QString txt =  QString(tr("Bluetooth function is disabled."));
    QString info =  QString(tr("To use this feature, Bluetooth needs to be enabled. Would you like to enable it?"));

    this->setIcon(QMessageBox::Warning);
    this->setInformativeText(info);
    this->setText(txt);

    connect(this, &QMessageBox::rejected, this, &BluetoothOffSendFileDialog::bt_rejected);
    connect(this, &QMessageBox::accepted, this, &BluetoothOffSendFileDialog::bt_accepted);
}

BluetoothOffSendFileDialog::~BluetoothOffSendFileDialog()
{
    KyInfo();
}

void BluetoothOffSendFileDialog::bt_rejected()
{
    KyInfo();

    this->deleteLater();
}

void BluetoothOffSendFileDialog::bt_accepted()
{
    KyInfo();

    QMap<QString, QVariant> attr;
    attr["Powered"] = true;
    Config::setDefaultAdapterAttr(attr);

    this->deleteLater();
}
