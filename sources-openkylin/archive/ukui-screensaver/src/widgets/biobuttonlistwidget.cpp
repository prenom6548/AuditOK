/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "biobuttonlistwidget.h"
#include "statusbutton.h"
#include "commonfunc.h"
#include "definetypes.h"
#include "loginauthinterface.h"
#include "pluginsloader.h"

BioButtonListWidget::BioButtonListWidget(QWidget *parent) : MyListWidget(parent)
{
    //    installEventFilter(this);
    initUI();
    initConnections();
}

void BioButtonListWidget::initUI()
{
    this->setFlow(QListWidget::LeftToRight);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QString buttonListStyle = "QListWidget{ background:rgba(255, 255, 255, 0%); border: 0px;}"
                              "QListWidget::item{border-radius: 4px; background:rgba(255, 255, 255, 0%);}";
    this->setStyleSheet(buttonListStyle);
    this->setSpacing(8);
}

void BioButtonListWidget::initConnections()
{
    //  响应点击事件
    connect(this, &BioButtonListWidget::itemClicked, this, &BioButtonListWidget::onListItemClicked);
    connect(this, &BioButtonListWidget::currentItemChanged, this, &BioButtonListWidget::onListItemChanged);
}

void BioButtonListWidget::resizeEvent(QResizeEvent *event)
{
    updateUI();
}

void BioButtonListWidget::addOptionButton(unsigned uLoginOptType, int nDrvId, QString strDrvName)
{
    m_listDriveId.append(nDrvId);

    StatusButton *buttonWidget = new StatusButton(this, BOTBUTTON);
    QListWidgetItem *buttonItem = new QListWidgetItem();
    buttonItem->setSizeHint(QSize(48, 48));
    this->insertItem(this->count(), buttonItem);
    this->setItemWidget(buttonItem, buttonWidget);
    buttonWidget->installEventFilter(this);
    this->addItem(buttonItem);

    QPixmap iconPixmap;
    switch (uLoginOptType) {
        case LOGINOPT_TYPE_PASSWORD:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-password.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_GENERAL_UKEY:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-ukey.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_FACE:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-face.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_FINGERPRINT:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-finger.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_IRIS:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-iris.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_VOICEPRINT:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-voice.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_FINGERVEIN:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-fingervein.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_QRCODE:
            iconPixmap = loadSvg(":/image/assets/ukui-loginopt-qrcode.svg", "white", 16);
            break;
        case LOGINOPT_TYPE_CUSTOM:
            LoginAuthInterface *plugin = dynamic_cast<LoginAuthInterface *>(
                PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).values().first());
            QString strIcon = plugin->icon();
            if (strIcon.startsWith("/")) {
                if (strIcon.endsWith(".svg")) {
                    iconPixmap = loadSvg(strIcon, "white", 16);
                } else {
                    iconPixmap.load(strIcon);
                    iconPixmap.scaled(40, 40);
                }
            } else {
                iconPixmap = QIcon::fromTheme(strIcon).pixmap(48, 48).scaled(40, 40);
            }
            if (iconPixmap.isNull()) {
                iconPixmap = loadSvg(":/images/ukui-loginopt-custom.svg", "white", 16);
            }
            break;
    }
    buttonWidget->setIcon(iconPixmap);
    buttonWidget->setToolTip(strDrvName);
    connect(buttonWidget, SIGNAL(buttonClicked(int)), this, SLOT(onOptionSelected(int)));
}

void BioButtonListWidget::onOptionSelected(int nIndex)
{
    if (nIndex < 0)
        return;
}

void BioButtonListWidget::updateUI()
{
    this->setSpacing(8 * scale);
    QList<StatusButton *> statusButton = this->findChildren<StatusButton *>();
    for (auto button : statusButton) {
        button->setFixedSize(48 * scale, 48 * scale);
        button->setIconSize(QSize(24 * scale, 24 * scale));
    }
}

void BioButtonListWidget::onListItemClicked(QListWidgetItem *item)
{
    QWidget *widget = itemWidget(item);
    StatusButton *currentItem = qobject_cast<StatusButton *>(widget);
    if (currentItem) {
        currentItem->setClickedStatus(CLICKED);
        currentItem->buttonClicked(currentItem->getDrivedId());
    }
}

void BioButtonListWidget::onListItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QWidget *currentWidget = itemWidget(current);
    StatusButton *currentItem = static_cast<StatusButton *>(currentWidget);
    if (currentItem)
        currentItem->setClickedStatus(SELECT);
    if (previous) {
        QWidget *previousWidget = itemWidget(previous);
        StatusButton *previousItem = static_cast<StatusButton *>(previousWidget);
        if (previousItem)
            previousItem->setClickedStatus(NORMAL);
    }
}
