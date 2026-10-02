/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ipv4page.h"
#include "netdetail.h"
#include "math.h"

Ipv4Page::Ipv4Page(QWidget *parent):QFrame(parent)
{
    initUI();
    initComponent();
    // store default palette so we can restore it when leaving OOBE
    m_defaultPalette = this->palette();
}

void Ipv4Page::initUI() {
    ipv4ConfigCombox = new QComboBox(this);
    ipv4addressEdit = new LineEdit(this);
    netMaskEdit = new LineEdit(this);
    gateWayEdit = new LineEdit(this);
    firstDnsEdit = new LineEdit(this);
    secondDnsEdit = new LineEdit(this);

    m_configLabel = new QLabel(this);
    m_addressLabel = new QLabel(this);
    m_maskLabel = new QLabel(this);
    m_gateWayLabel = new QLabel(this);
    m_dnsLabel = new QLabel(this);
    m_secDnsLabel = new QLabel(this);

    m_configLabel->setText(tr("Ipv4Config"));
    m_addressLabel->setText(tr("Address"));
    m_maskLabel->setText(tr("Netmask"));
    m_gateWayLabel->setText(tr("Default Gateway"));
    m_dnsLabel->setText(tr("Prefs DNS"));
    m_secDnsLabel->setText(tr("Alternative DNS"));

    m_detailLayout = new QFormLayout(this);
    m_detailLayout->addRow(m_configLabel,ipv4ConfigCombox);
    m_detailLayout->addRow(m_addressLabel,ipv4addressEdit);
    m_detailLayout->addRow(m_maskLabel,netMaskEdit);
    m_detailLayout->addRow(m_gateWayLabel,gateWayEdit);
    m_detailLayout->addRow(m_dnsLabel,firstDnsEdit);
    m_detailLayout->addRow(m_secDnsLabel,secondDnsEdit);

    ipv4ConfigCombox->addItem(tr("Auto(DHCP)")); //"自动(DHCP)"
    ipv4ConfigCombox->addItem(tr("Manual")); //"手动"


    // IP的正则格式限制
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    ipv4addressEdit->setValidator(new QRegExpValidator(rx, this));
    gateWayEdit->setValidator(new QRegExpValidator(rx, this));
    netMaskEdit->setValidator(new QRegExpValidator(rx, this));
    firstDnsEdit->setValidator(new QRegExpValidator(rx, this));
    secondDnsEdit->setValidator(new QRegExpValidator(rx, this));
}

void Ipv4Page::setOobe(bool on)
{
    if (on) {
        m_isOobe = true;   
        QPalette p = m_defaultPalette;
        QColor white("#FFFFFF");
        // ensure text colors are white
        p.setColor(QPalette::WindowText, white);
        p.setColor(QPalette::Text, white);
        p.setColor(QPalette::ButtonText, white);

        m_configLabel->setPalette(p);
        m_addressLabel->setPalette(p);
        m_maskLabel->setPalette(p);
        m_gateWayLabel->setPalette(p);
        m_dnsLabel->setPalette(p);
        m_secDnsLabel->setPalette(p);

        ipv4addressEdit->setPalette(p);
        netMaskEdit->setPalette(p);
        gateWayEdit->setPalette(p);
        firstDnsEdit->setPalette(p);
        secondDnsEdit->setPalette(p);
        ipv4ConfigCombox->setPalette(p);
        ipv4addressEdit->setOobeMode(true);
        netMaskEdit->setOobeMode(true);
        gateWayEdit->setOobeMode(true);
        firstDnsEdit->setOobeMode(true);
        secondDnsEdit->setOobeMode(true);

        ipv4ConfigCombox->setFixedHeight(34);
        ipv4ConfigCombox->setStyleSheet(QString(
            "QComboBox {"
            "   background-color: #4A4A4A;"
            "   color: white;"  
            "}"
            "QComboBox QAbstractItemView {"
            "   background-color: #4A4A4A;"  
            "   color: white;"              
            "   selection-background-color: #5A5A5A;" 
            "   selection-color: white;"                
            "}"
        ));

        ipv4ConfigCombox->installEventFilter(this);
        ipv4ConfigCombox->setView(new QListView());
        ipv4ConfigCombox->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        ipv4ConfigCombox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
        ipv4ConfigCombox->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

    }
}

bool Ipv4Page::eventFilter(QObject *watched, QEvent *event)
{
    return QObject::eventFilter(watched,event);
}

void Ipv4Page::initComponent() {
    if (ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        setLineEnabled(false);
    } else if (ipv4ConfigCombox->currentIndex() == MANUAL_CONFIG) {
        setLineEnabled(true);
    }
    connect(ipv4ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(configChanged(int)));

    connect(ipv4ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfSaveBtn()));
    connect(ipv4addressEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(netMaskEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(gateWayEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(firstDnsEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(secondDnsEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
}

void Ipv4Page::setIpv4Config(KyIpConfigType ipv4Config)
{
    if (ipv4Config ==  CONFIG_IP_MANUAL) {
        ipv4ConfigCombox->setCurrentIndex(MANUAL_CONFIG);
    } else {
        ipv4ConfigCombox->setCurrentIndex(AUTO_CONFIG);
    }
}

void Ipv4Page::setIpv4(const QString &ipv4)
{
    ipv4addressEdit->setText(ipv4);
}

void Ipv4Page::setNetMask(const QString &netMask)
{
    netMaskEdit->setText(netMask);
}

void Ipv4Page::setIpv4FirDns(const QString &ipv4FirDns)
{
    firstDnsEdit->setText(ipv4FirDns);
}

void Ipv4Page::setIpv4SecDns(const QString &ipv4SecDns)
{
    secondDnsEdit->setText(ipv4SecDns);
}

void Ipv4Page::setGateWay(const QString &gateWay)
{
    gateWayEdit->setText(gateWay);
}

bool Ipv4Page::checkIsChanged(const ConInfo info)
{
    bool isChanged = false;
    if (ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        if (info.ipv4ConfigType != CONFIG_IP_DHCP) {
            isChanged = true;
        }
    } else {
        if (info.ipv4ConfigType != CONFIG_IP_MANUAL) {
            isChanged =  true;
        }
        if(info.strIPV4Address != ipv4addressEdit->text()
                || info.strIPV4NetMask != /*netMaskEdit->text()*/getNetMaskText(netMaskEdit->text())
                || info.strIPV4GateWay != gateWayEdit->text()
                || info.strIPV4FirDns  != firstDnsEdit->text()
                || info.strIPV4SecDns  != secondDnsEdit->text()) {
            isChanged =  true;
        }
    }
    return isChanged;
}

void Ipv4Page::updateSetting(KyConnectSetting &setting)
{
    if (ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        setting.setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);
        QString ipv4address("");
        QString netMask("");
        QString gateWay("");
        QStringList dnsList;
        dnsList.empty();
        qDebug() << ipv4address << netMask << gateWay;
        setting.ipv4AddressConstruct(ipv4address, netMask, gateWay, dnsList);
    }else {
        setting.setIpConfigType(IPADDRESS_V4, CONFIG_IP_MANUAL);

        QStringList dnsList;
        dnsList.empty();
        if (!firstDnsEdit->text().isEmpty()) {
            dnsList << firstDnsEdit->text();
            if (!secondDnsEdit->text().isEmpty()) {
                dnsList << secondDnsEdit->text();
            }
        }

        QString ipv4address =ipv4addressEdit->text();
        QString netMask = getNetMaskText(netMaskEdit->text());
        QString gateWay = gateWayEdit->text();
        qDebug() << ipv4address << netMask << gateWay;
        setting.ipv4AddressConstruct(ipv4address, netMask, gateWay, dnsList);
        setting.dumpInfo();
    }
}

bool Ipv4Page::checkConnectBtnIsEnabled()
{
    qDebug() << "checkConnectBtnIsEnabled currentIndex" << ipv4ConfigCombox->currentIndex();
    if (ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        return true;
    } else {
        if (ipv4addressEdit->text().isEmpty() || !getTextEditState(ipv4addressEdit->text())) {
            qDebug() << "ipv4address empty or invalid";
            return false;
        }

        if (netMaskEdit->text().isEmpty() || !netMaskIsValide(netMaskEdit->text())) {
            qDebug() << "ipv4 netMask empty or invalid";
            return false;
        }

//        if (gateWayEdit->text().isEmpty() || !getTextEditState(gateWayEdit->text())) {
//            qDebug() << "ipv4 gateway empty or invalid";
//            return false;
//        }

        if (firstDnsEdit->text().isEmpty() && !secondDnsEdit->text().isEmpty()) {
            qDebug() << "ipv4 dns sort invalid";
            return false;
        }

        if (!getTextEditState(firstDnsEdit->text())) {
            qDebug() << "ipv4 first dns invalid";
            return false;
        }

        if (!getTextEditState(secondDnsEdit->text())) {
            qDebug() << "ipv4 second dns invalid";
            return false;
        }
    }
    return true;
}

void Ipv4Page::configChanged(int index) {
    if (index == AUTO_CONFIG) {
        setLineEnabled(false);
    }
    if (index == MANUAL_CONFIG) {
        setLineEnabled(true);
    }
}

void Ipv4Page::setLineEnabled(bool check) {

    ipv4addressEdit->setEnabled(check);
    netMaskEdit->setEnabled(check);
    gateWayEdit->setEnabled(check);
    firstDnsEdit->setEnabled(check);
    secondDnsEdit->setEnabled(check);

    if (!check) {
        ipv4addressEdit->clear();
        netMaskEdit->clear();
        gateWayEdit->clear();
        firstDnsEdit->clear();
        secondDnsEdit->clear();
    }
}

void Ipv4Page::setEnableOfSaveBtn() {
    Q_EMIT setIpv4PageState(checkConnectBtnIsEnabled());
}

bool Ipv4Page::getTextEditState(QString text)
{
    if (text.isEmpty()) {
        return true;
    }
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    bool match = false;
    match = rx.exactMatch(text);

    return match;
}

bool Ipv4Page::netMaskIsValide(QString text)
{
    if (getTextEditState(text)) {
        return true;
    } else {
        if (text.length() > 0 && text.length() < 3) {
            int num = text.toInt();
            if (num > 0 && num < 33) {
                return true;
            }
        }
    }
    return false;
}

QString Ipv4Page::getNetMaskText(QString text)
{
    if (text.length() > 2) {
        return text;
    }

    int num = text.toInt();
    QStringList list;
    list << "0" << "0" << "0" << "0";
    int count = 0;
    while (num - 8 >= 0) {
        list[count] = "255";
        num = num - 8;
        count ++;
    }
    if (num > 0) {
        int size = pow(2, 8) - pow(2,(8-num));
        list[count] = QString::number(size);
    }
    return QString("%1.%2.%3.%4").arg(list[0],list[1],list[2],list[3]);
}
