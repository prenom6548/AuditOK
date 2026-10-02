/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "DeviceManagerWidget.h"

#include <kswitchbutton.h>
#include <QHBoxLayout>
#include <QDebug>
#include <QGuiApplication>

#include "Ukui4CustomControl.h"
#include "ukui4.0/Ukui4SwitchButtonItem.h"

DeviceManagerItem::DeviceManagerItem(QLabel* label, std::shared_ptr<ISwitchButton> switchBtn, const QString& cardName, const QString& portName, QWidget* parent)
    : m_pLabel(label), m_pSwitchBtn(switchBtn), m_cardName(cardName), m_portName(portName), QFrame(parent)
{
    initUi();
    initSlots();
}

void DeviceManagerItem::initUi()
{
    setFixedSize(512, 60);
    setFrameShape(QFrame::Shape::Box);

    m_pLabel->setFixedHeight(28);
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pLabel);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hLayout->addWidget(m_pSwitchBtn->getWidget());
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->setSpacing(0);
    setLayout(hLayout);

    auto pal = palette();
    QColor cl = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
    pal.setBrush(QPalette::Base, cl);
    setPalette(pal);
}

void DeviceManagerItem::initSlots()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* themeName = new QGSettings("org.ukui.style");
        QObject::connect(themeName, &QGSettings::changed, this, [this](const QString& key){
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor"
                    || key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                auto pal = palette();
                QColor cl = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
                pal.setBrush(QPalette::Base, cl);
                setPalette(pal);
            }
        });
    }
}

bool DeviceManagerItem::isChecked() const
{
    return m_pSwitchBtn->isChecked();
}

void DeviceManagerItem::setChecked(bool checked)
{
    m_pSwitchBtn->setChecked(checked);
}

QWidget* DeviceManagerItem::getWidget()
{
    return this;
}

std::shared_ptr<ISwitchButton> DeviceManagerItem::getSwitchBtn() const
{
    return m_pSwitchBtn;
}

QVariant DeviceManagerItem::getData() const
{
    return QVariant(std::initializer_list<QString> {m_cardName, m_portName});
}

DeviceManagerWidget::DeviceManagerWidget(QLabel* label, QWidget* parent)
    : m_pLabel(label), QWidget(parent)
{
    initUi();
    initSlots();
}

void DeviceManagerWidget::initUi()
{
    auto labelLayout = new QHBoxLayout();
    m_pLabel->setFixedSize(480, 28);
    labelLayout->setSpacing(0);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->addSpacing(41);
    labelLayout->addWidget(m_pLabel);
    labelLayout->addSpacing(39);

    m_pScrollArea = new QScrollArea(this);
    m_pScrollArea->setContentsMargins(0, 0, 0, 0);
    m_pScrollArea->setFixedSize(512, 300);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_pDisDevManagerWidget = new QWidget(m_pScrollArea);
    m_pDisDevManagerWidget->setContentsMargins(0, 0, 0, 0);
    m_pDisDevManagerWidget->setFixedWidth(512);

    m_pScrollArea->setWidget(m_pDisDevManagerWidget);

    m_pVlayout = new QVBoxLayout(m_pDisDevManagerWidget);
    m_pVlayout->setSpacing(0);
    m_pVlayout->setContentsMargins(0, 0, 0, 0);
    m_pDisDevManagerWidget->setLayout(m_pVlayout);

    QHBoxLayout *scrollAreaLayout = new QHBoxLayout();
    scrollAreaLayout->setSpacing(0);
    scrollAreaLayout->setContentsMargins(0, 0, 0, 0);
    scrollAreaLayout->addSpacing(25);
    scrollAreaLayout->addWidget(m_pScrollArea);
    scrollAreaLayout->addSpacing(23);

    m_pConfirmBtn = new QPushButton(tr("Confirm"), this);
    m_pConfirmBtn->setFixedSize(96, 36);
    m_pConfirmBtn->setProperty("isImportant", true);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addStretch();
    hLayout->addWidget(m_pConfirmBtn);
    hLayout->addSpacing(25);

    QString theme = "ukui-default";
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* gst = new QGSettings("org.ukui.style");
        if (gst->keys().contains("styleName")) {
            theme = gst->get("style-name").toString();
        }
    }

    m_pNoDeviceImg = new QLabel();
    m_pNoDeviceImg->setAccessibleName(tr("No device label"));
    if (theme == "ukui-dark" || theme == "ukui-black") {
        m_pNoDeviceImg->setPixmap(QPixmap(":/images/no-device-black.svg"));
    } else {
        m_pNoDeviceImg->setPixmap(QPixmap(":/images/no-device-white.png"));
    }
    m_pNoDeviceImg->setAlignment(Qt::AlignCenter);

    m_pNoDeviceText = new QLabel(tr("Device not found"));
    m_pNoDeviceText->setAlignment(Qt::AlignCenter);
    m_pNoDeviceText->setFixedSize(480, 19);

    m_pNoDeviceVLayout = new QVBoxLayout();
    m_pNoDeviceVLayout->setSpacing(0);
    m_pNoDeviceVLayout->setContentsMargins(0, 120, 0, 0);
    m_pNoDeviceVLayout->addStretch();
    m_pNoDeviceVLayout->addWidget(m_pNoDeviceImg, 0, Qt::AlignCenter);
    m_pNoDeviceVLayout->addWidget(m_pNoDeviceText, 0, Qt::AlignCenter);
    m_pNoDeviceVLayout->addStretch();

    m_pNoDeviceImg->hide();
    m_pNoDeviceText->hide();

    // 进行整体布局
    m_pMainVLayout = new QVBoxLayout(this);
    m_pMainVLayout->setSpacing(0);
    m_pMainVLayout->setContentsMargins(0, 0, 0, 0);
    m_pMainVLayout->addLayout(labelLayout);
    m_pMainVLayout->addSpacing(10);
    m_pMainVLayout->addLayout(scrollAreaLayout);
    m_pMainVLayout->addStretch();
    m_pMainVLayout->addLayout(hLayout);
    m_pMainVLayout->addSpacing(40);

    setLayout(m_pMainVLayout);
}

void DeviceManagerWidget::initSlots()
{
    connect(m_pConfirmBtn, &QPushButton::clicked, this, &DeviceManagerWidget::closeWindow);

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* themeName = new QGSettings("org.ukui.style");
        QObject::connect(themeName, &QGSettings::changed, this, [this](const QString& key){
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor"
                    || key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                QString theme = "ukui-default";
                if (QGSettings::isSchemaInstalled("org.ukui.style")) {
                    QGSettings* gst = new QGSettings("org.ukui.style");
                    if (gst->keys().contains("styleName")) {
                        theme = gst->get("style-name").toString();
                    }
                }

                if (m_pNoDeviceImg) {
                    if (theme == "ukui-dark" || theme == "ukui-black") {
                        m_pNoDeviceImg->setPixmap(QPixmap(":/images/no-device-black.svg"));
                    } else {
                        m_pNoDeviceImg->setPixmap(QPixmap(":/images/no-device-white.png"));
                    }
                }

                if (m_pNoDeviceText) {
//                    QPalette palette = m_pNoDeviceText->palette();
//                    QColor color = qApp->property("kfont-secondary").value<QBrush>().color();
//                    palette.setColor(QPalette::Text, color);
//                    m_pNoDeviceText->setPalette(palette);
                }
            }
        });
    }
}

void DeviceManagerWidget::closeWindow()
{
    Q_EMIT closeWindowSignals();
}

void DeviceManagerWidget::addItem(const QString& displayStr, const QString& cardName, const QString& portName)
{
    std::shared_ptr<ISwitchButton> switchBtn = std::make_shared<Ukui4SwitchButton>(new kdk::KSwitchButton);
    std::shared_ptr<IDeviceManagerItem> item = std::make_shared<DeviceManagerItem>(new AutoOmitLabel(displayStr), switchBtn, cardName, portName, this);

    auto idx = m_devManagerItemVec.size();
    m_devManagerItemVec.emplace_back(item);
    qDebug() << "DeviceManagerMainWidget::addItem " << displayStr << " cardName:" << cardName << " portName: " << portName;
    insertToWidget(idx);

    // connect
    auto btn = static_cast<QPushButton*>(switchBtn->getWidget());
    connect(btn, &QPushButton::clicked, this, [this, cardName, portName, btn]() {
        qDebug() << QString("btn enableDeviceSignals, card: %1, port: %2, enable: %3").arg(cardName).arg(portName).arg(btn->isChecked());
        Q_EMIT enableDeviceSignals(cardName, portName, btn->isChecked());
    });
}

void DeviceManagerWidget::insertToWidget(int idx)
{
    m_pVlayout->addWidget(m_devManagerItemVec[idx]->getWidget());
    m_pDisDevManagerWidget->resize(512, m_devManagerItemVec.size() * 60);
}

void DeviceManagerWidget::setChecked(int idx, bool checked)
{
    if (idx >= m_devManagerItemVec.size()) {
        qWarning() << QString("Invaild index %1, set device manager item checked failed.").arg(idx);
        return;
    }

    m_devManagerItemVec[idx]->setChecked(checked);
}

void DeviceManagerWidget::removeItem(int idx)
{
    if (idx > m_devManagerItemVec.size()) {
        qDebug() << QString("invaild index %1 , can not remove item in device manager widget.").arg(idx);
        return;
    }

    m_devManagerItemVec.erase(m_devManagerItemVec.begin() + idx);
    m_pDisDevManagerWidget->resize(512, m_devManagerItemVec.size() * 60);
}

std::list<QVariant> DeviceManagerWidget::getDataList() const
{
    std::list<QVariant> list;

    for (int i = 0; i < m_devManagerItemVec.size(); ++i) {
        auto data = m_devManagerItemVec[i]->getData();
        QList<QVariant> variantList;
        variantList.push_back(i);
        variantList.push_back(data.toStringList().at(0));
        variantList.push_back(data.toStringList().at(1));
        list.push_back(QVariant(variantList));
    }

    return list;
}

void DeviceManagerWidget::updateDeviceStatus(bool hasDevice)
{
    if (hasDevice) {
        m_pLabel->show();
        m_pScrollArea->show();
        m_pConfirmBtn->show();
        m_pMainVLayout->removeItem(m_pNoDeviceVLayout);
        if (m_pNoDeviceImg) {
            m_pNoDeviceImg->hide();
        }
        if (m_pNoDeviceText) {
            m_pNoDeviceText->hide();
        }
    } else {
        m_pLabel->hide();
        m_pScrollArea->hide();
        m_pConfirmBtn->hide();
        m_pMainVLayout->removeItem(m_pNoDeviceVLayout);
        m_pMainVLayout->insertLayout(2, m_pNoDeviceVLayout);
        if (m_pNoDeviceImg) {
            m_pNoDeviceImg->show();
        }
        if (m_pNoDeviceText) {
            m_pNoDeviceText->show();
        }
    }
}
