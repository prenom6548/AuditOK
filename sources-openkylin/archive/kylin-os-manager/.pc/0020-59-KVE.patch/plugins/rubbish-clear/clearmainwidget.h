/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMap>

#include "clearwidgetkinditem.h"
#include "utils.h"
#include "kalertdialog.h"


namespace KylinRubbishClear
{

class ClearMainWidget : public QWidget
{
    Q_OBJECT

public:
    ClearMainWidget(int theme, int size, QWidget *parent = nullptr);
    ~ClearMainWidget();

    void getAllScanSelectedItems();
    void onRefreshSelectedList();
    void resetDefaultStatus();
    void Browser_to_judge_existence();
    void changeThemeColor(int color);
    void changeSystemSize(int size);

private:
	bool isAdminUser(void);

    QLabel *mLabelSlogen = nullptr;
    QLabel *mLabelDescribe = nullptr;
    QLabel *mLabelTotal = nullptr;
    QLabel *mLabelPic = nullptr;
    ClearWidgetKindItem *mKindSystemCache = nullptr;
    ClearWidgetKindItem *mKindCookies = nullptr;
    ClearWidgetKindItem *mKindHistory = nullptr;
    QPushButton *mBtnOneKeyClear = nullptr;

    QStringList cache_list;
    QStringList cache_status_list;
    QStringList cookies_list;
    QStringList cookies_status_list;
    QStringList trace_list;
    QStringList trace_status_list;
    QStringList mSelectedTrash;

    QMap<QString, QVariant> argsMap;        // 传递给 python dbus 的参数（Cache, History）
    QMap<QString, QVariant> mScanSystemMap; // 传递给 clearmanager 的参数（Cookies, Trash）
    QStringList m_selectedCache;            // 缓存项：apt, 缩略图, 齐安信浏览器, 回收站
    QStringList m_selectedCookie;           // Cookie 项：齐安信浏览器, 火狐浏览器
    QStringList m_selectedTrace;            // 历史痕迹：system, bash

    bool flag_cache = true;
    bool flag_cookie = true;
    bool flag_trace = true;

    bool google = false;
    bool firefox = false;
    bool browser360 = false;
    bool qaxbrowser = false;

    void initMainUi();
    void initDataList();

    int themeColor;
    int mFontSize;
    KAlertDialog *alertDialog = nullptr;

Q_SIGNALS:
    void startScanSystem(QMap<QString, QVariant> itemsMap);
    void sigStartScanSystem(QMap<QString, QVariant> itemsMap);
    void hideThisWidget();

public Q_SLOTS:
    void slotShowSelectItemDialog(QString str);
    void onRefreshSelectedItems(CleanerCategoryID id, const QStringList &infos);
    void slotOnClickedClearBtn();
};

} // namespace KylinRubbishClear

#endif // WIDGET_H
