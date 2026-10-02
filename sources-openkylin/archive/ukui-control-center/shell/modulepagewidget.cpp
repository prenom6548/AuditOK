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
#include "modulepagewidget.h"
#include "ui_modulepagewidget.h"

#include <QListWidgetItem>
#include <QDebug>
#include <QScrollBar>

#include "mainwindow.h"
#include "interface.h"
#include "utils/functionselect.h"
#include "interface/ukcccommon.h"
using namespace ukcc;

ModulePageWidget::ModulePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModulePageWidget)
{
    ui->setupUi(this);
    initUI();
}

ModulePageWidget::~ModulePageWidget()
{
    delete ui;
    ui = nullptr;
}

void ModulePageWidget::initUI()
{
    //设置伸缩策略

    QSizePolicy rightSizePolicy = ui->widget->sizePolicy();

    rightSizePolicy.setHorizontalStretch(5);

    ui->widget->setSizePolicy(rightSizePolicy);
    ui->widget->setObjectName("widget");
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    connect(ui->scrollArea->verticalScrollBar(),&QScrollBar::rangeChanged,this, [=](){
        if (ui->scrollArea->verticalScrollBar()->maximum() == 0) {
            ui->scrollArea->verticalScrollBar()->setHidden(true);
        } else {
            ui->scrollArea->verticalScrollBar()->setHidden(false);
        }
    });

    connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged, this, [=](){
        if (ui->scrollArea->horizontalScrollBar()->maximum() == 0) {
            emit hScrollBarHide();
        } else {
            emit hScrollBarShow();
        }
    });

    //设置qss之后,点击屏保再主页进入插件会卡死,原因未知
//    ui->scrollArea->setStyleSheet("QScrollArea{background-color: palette(window);}");
    ui->scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    // ClickFocus：鼠标点空白可焦到内容区，随后 Tab 进插件；不含 TabFocus，侧栏 Tab 不会多停 filler
    ui->scrollArea->setFocusPolicy(Qt::ClickFocus);
    if (ui->scrollArea->viewport()) {
        ui->scrollArea->viewport()->setFocusPolicy(Qt::ClickFocus);
    }
}

void ModulePageWidget::switchPage(QObject *plugin, bool recorded, QString jumpText)
{
    Q_UNUSED(recorded);
    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    refreshPluginWidget(pluginInstance);

    if (pluginInstance->name() == "Wallpaper" ||
            pluginInstance->name() == "Screenlock") {
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        qDebug() << Q_FUNC_INFO << pluginInstance->name() << "hide ScrollBar";
    } else {
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        qDebug() << Q_FUNC_INFO << pluginInstance->name() << "hide ScrollBar";
    }

    if (jumpText.isEmpty() || !pluginwidget) {
        return;
    }

    for (QLabel *o : pluginwidget->findChildren<QLabel*>()) {
        if (o->text() == jumpText) {
            QPoint pos = o->mapTo(ui->scrollArea->widget(), o->pos());
            int widgetHeight = ui->scrollArea->widget()->height();
            int areaHeight   = ui->scrollArea->height();
            if (pluginInstance->name() == "Theme") {
               pos.setY(pos.y()/2);
            }
            int maxNum = widgetHeight - areaHeight;
            int value = pos.y() + o->height() - areaHeight;
            value = value + areaHeight/2; //尽量让选中的显示在中间位置
            if (value <= 0) {
               ui->scrollArea->verticalScrollBar()->setValue(0);
            } else if (value > maxNum){
                ui->scrollArea->verticalScrollBar()->setValue(maxNum);
            } else {
               ui->scrollArea->verticalScrollBar()->setValue(value);
            }
            return;
       }
    }
}

void ModulePageWidget::refreshPluginWidget(CommonInterface *plu)
{
    if (!plu) {
        qWarning() << Q_FUNC_INFO << "plugin pointer is nullptr";
        return;
    }

    prePlugin = currentPlugin;
    // 侧栏勾选互斥：不清旧项时，首页进「设备→鼠标」仍会残留「显示器」勾选，读屏也读旧项
    if (prePlugin && prePlugin != plu && prePlugin->pluginBtn) {
        prePlugin->pluginBtn->setCheckState(Qt::Unchecked);
    }
    if (plu->pluginBtn) {
        plu->pluginBtn->setCheckState(Qt::Checked);
    }
    if (prePlugin) {
        qInfo()<<"plugin_leave(): "<<prePlugin->name();
        prePlugin->plugin_leave();
    }
    currentPlugin = plu;
    ui->scrollArea->takeWidget();
    delete(ui->scrollArea->widget());

    pluginwidget =  plu->pluginUi();

    // 检查pluginUi()返回的widget是否为nullptr，防止崩溃
    if (!pluginwidget) {
        qWarning() << Q_FUNC_INFO << "pluginUi() returned nullptr for plugin:" << plu->name();
        return;
    }

    if (pluginwidget && UkccCommon::isV11()) {
        pluginwidget->update();
        qDebug() << Q_FUNC_INFO << "update plugin ui";
    }
    qInfo()<<"pluginUi(): "<<currentPlugin->name();
    pluginwidget->setContentsMargins(24,0,4,40);
    ui->scrollArea->setWidget(pluginwidget);
    mCurrentPluName = plu->name();
    pluginwidget->setEnabled(enableItemsByConfig(mCurrentPluName));

    //记录打开历史
    if (flagBit){
        FunctionSelect::pushRecordValue(plu->pluginTypes(), plu->plugini18nName());
    }

    //恢复标志位
    flagBit = true;

    if (plu->name() == "Wallpaper" ||
            plu->name() == "Screenlock") {
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        qDebug() << Q_FUNC_INFO << plu->name() << "hide ScrollBar";
    } else {
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        qDebug() << Q_FUNC_INFO << plu->name() << "show ScrollBar";
    }
}

void ModulePageWidget::pluginLeave()
{
    if (currentPlugin) {
        // 回首页时清侧栏勾选，避免下次进其它模块仍显示旧项选中
        if (currentPlugin->pluginBtn) {
            currentPlugin->pluginBtn->setCheckState(Qt::Unchecked);
        }
        currentPlugin->plugin_leave();
    }
}

bool ModulePageWidget::enableItemsByConfig(const QString &pluginName)
{
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSetings = ModuleMap.value(pluginName.toLower() + "Enable").toString();
    QStringList settingItems = moduleSetings.split(",");

    foreach (QString setItem, settingItems) {
        QStringList item = setItem.split(":");
        qDebug() << Q_FUNC_INFO << "set item Name: " << item.at(0) << pluginName << moduleSetings;
        if (QString(pluginName.toLower() + "AllPage").compare(item.at(0), Qt::CaseInsensitive) == 0) {
            return (item.at(1) == "true");
        }
    }
    // 若配置文件中无AllPage项，返回true，不影响原程序显示逻辑
    return true;
}

