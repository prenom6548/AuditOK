/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "activeconnectionwidget.h"
#include "config/xatom-helper.h"
#include "config/config.h"
#include <QPainterPath>

ActiveConnectionWidget::ActiveConnectionWidget(QString address, QString name, QString type, int rssi) :
    m_devAddr(address),
    m_devName(name),
    m_devType(type),
    m_devRSSI(rssi)
{
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth Connection"));

    QFontMetrics fontMetrics(this->font());
    QString tip = QString(tr("Found audio device \"") + m_devName + tr("\", connect it or not?"));
    QString _tip = fontMetrics.elidedText(tip, Qt::ElideMiddle, this->width() - 80);
    if (tip != _tip) {
        tip = _tip;
    }
    this->setText(tip);


    m_cancel_btn = new QPushButton(tr("Cancel"), this);
    m_connect_btn = new QPushButton(tr("Connect"), this);
    this->addButton(m_cancel_btn,QMessageBox::RejectRole);
    this->addButton(m_connect_btn,QMessageBox::AcceptRole);

    m_actCnt_box = new QCheckBox(tr("No prompt"), this);
    this->setCheckBox(m_actCnt_box);

    connect(m_actCnt_box, &QCheckBox::toggled, this, &ActiveConnectionWidget::toggled);
    connect(this, &QMessageBox::accepted, this, &ActiveConnectionWidget::onClick_connect_btn);
    connect(this, &QMessageBox::rejected, this, &ActiveConnectionWidget::onClick_cancel_btn);
}

ActiveConnectionWidget::~ActiveConnectionWidget()
{
    KyDebug();
}

void ActiveConnectionWidget::autoConnChanged(bool activate) {
    m_actCnt_box->setChecked(!activate);
    if (m_mutex) {
        m_mutex = false;
        return;
    }
    m_mutex = true;
}

void ActiveConnectionWidget::onClick_connect_btn()
{
    KyDebug() << "accpet";
    emit replyActiveConnection(m_devAddr, true);
    this->hide();
    this->deleteLater();
}

void ActiveConnectionWidget::onClick_cancel_btn()
{
    KyDebug() << "cancel";
    emit replyActiveConnection(m_devAddr, false);
    this->hide();
    this->deleteLater();
}

void ActiveConnectionWidget::toggled(bool checked)
{
    if (QGSettings::isSchemaInstalled(GSETTING_SCHEMA_UKUIBLUETOOH)) {
        QGSettings(GSETTING_SCHEMA_UKUIBLUETOOH).set("active-connection", QVariant(!checked));
    }
    
    if (m_mutex) {
        m_mutex = false;
        return;
    }
    m_mutex = true;
    QMap<QString, QVariant> attr;
    attr.insert("ActiveConnection", QVariant(!checked));
    Config::setDefaultAdapterAttr(attr);
}
