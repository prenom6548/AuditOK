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
#ifndef HOMEPAGEWIDGET_H
#define HOMEPAGEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QVariantMap>
#include <QGSettings>
#include <QKeyEvent>
#include <QShowEvent>

#include <kysdk/applications/klabel.h>
using namespace kdk;

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

enum COLOR{
    BLUE,
    WHITE,
    BLACK,
    GRAY,
    HIGHLIGHT,
};

class MainWindow;
class QListWidgetItem;

#define PLUGIN_STATUS_SCHEMA "org.ukui.control-center.pluginstatus"
#define PLUGINS_SCHEMA       "org.ukui.control-center.plugins"
#define PLUGINS_PATH         "/org/ukui/control-center/plugins/"
#define PLUGIN_NAME          "plugin-name"
#define SHOW_KEY             "show"

namespace Ui {
class HomePageWidget;
}

class HomePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomePageWidget(QWidget *parent = nullptr, QMap<QString, QGSettings *> map = {});
    ~HomePageWidget();

public:
    void initUI();
    QGSettings * setGsettingsPath(QList<char *> list , QString name);
    // 清除插件名点击后残留的高亮色（返回首页时调用）
    void clearPluginLabelPressedStyle();

protected:
    void showEvent(QShowEvent *event) override;

private:
    const QPixmap loadSvg(const QString &fileName, COLOR color);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, COLOR color);
    bool eventFilter(QObject *watched, QEvent *event);
    // 插件 label：组内仅一个 Tab 落点，同组二级项用方向键切换；模块间只走 Tab
    void setupPluginLabelTabGroup(const QList<KLabel *> &labels);
    void takePluginLabelTabStop(KLabel *current);
    KLabel *findPluginLabelNeighbor(KLabel *current, int key) const;
    bool handlePluginLabelKeyEvent(KLabel *label, QKeyEvent *keyEvent);
    QList<KLabel *> pluginLabelGroup(KLabel *label) const;
    bool isHomePluginLabel(QObject *obj) const;
public:
     QMap<QString, QGSettings *> vecGsettins;
private:
    Ui::HomePageWidget *ui;
    MainWindow * pmainWindow;
    QVariantMap mModuleMap;
    QMap<int, QList<KLabel *>> mLabels;
};

#endif // HOMEPAGEWIDGET_H
