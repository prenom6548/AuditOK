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

#ifndef CLEANDETAILVEIW_H
#define CLEANDETAILVEIW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QVBoxLayout>
#include <QLine>
#include <QPixmap>
#include <QString>
#include <QMovie>
#include <QMap>
#include <QVariant>
#include <QTimer>
#include <QStackedWidget>

#include "utils.h"
#include "kalertdialog.h"
#include "kborderlessbutton.h"

namespace KylinRubbishClear
{

class CleandetailVeiw : public QWidget
{
    Q_OBJECT
public:
    CleandetailVeiw(int theme, int fontSize, QWidget *parent = nullptr);
    ~CleandetailVeiw();

    void InitWidget();
    void InitTopWidget();
    void InitBottomWidget();
    void ResetUI();

    void getAllSelectedItems();

    void animationForTip(int f);
    void timerUpDate(bool t);
    void changeThemeColor(int color);
    void changeSystemSize(int size);

Q_SIGNALS:
    void hideThisWidget();
    void startOneKeyClean();
    void startCleanSystem(QMap<QString, QVariant> itemsMap);
    void sigStartCleanSystem(QMap<QString, QVariant> itemsMap);

private Q_SLOTS:
    void getScanResult(QString msg);
    void finishScanResult(QString msg);
    void getScanAllResult(QString flag, QString msg);

    void OnClikedCleanBtn();
    void showReciveData(const QStringList &data);
    void showReciveStatus(const QString &status);
    void getCleanResult(QString msg);
    void ShowDetailsPage();

    void showCleanerData(const QStringList &data);
    void showCleanerStatus(const QString &status, const QString &domain);
    void showCleanerError(const QString &status);

    void onRefreshSelectedItems(CleanerModuleID id, const QStringList &infos);

    void receiveCleanSignal();
    void receivePolicyKitSignal(bool status);
    void showCleanOverStatus();

public Q_SLOTS:
    void slotScanDetailData(const QString status, const QStringList &list);
    void slotScanDetailStatus(const QString &str);
    void slotCleanStatus(const QString status, const QString belong);
    void statusTipSetText(QString text);

private:
    QVBoxLayout *main_layout;
    QHBoxLayout *top_layout;
    QVBoxLayout *bottom_layout;

    QLabel *mLabelStatus = nullptr;
    QPushButton *mBtnStartClear = nullptr;

    QPushButton *cancel_btn;
    QPushButton *onkeyclean;
    QPushButton *return_btn;
    QPushButton *btn_return;

    KBorderlessButton *cache_btn;
    QPushButton *cookie_btn;
    QPushButton *history_btn;

    QLabel *top_icon;
    QLabel *top_tip;
    QLabel *status_tip;

    QLabel *cache_tip;
    QLabel *cache_icon;

    QLabel *cookie_tip;
    QLabel *cookie_icon;

    QLabel *history_tip;
    QLabel *history_icon;

    QString trace;
    QString cookies;
    QString garbage;

    qreal cache_sum = 0;   //缓存数量
    qreal cookie_sum = 0;  //浏览器cookies数量
    qreal history_sum = 0; //历史痕迹

    QString root_key;

    QMap<QString, QVariant> argsData;
    QMap<QString, qreal> cache_files_sizes;

    QStringList cache_list;
    QStringList select_cache_apt_list;
    bool cache_flag = true;

    QStringList select_cookies_list;
    bool cache_chromium_flag = true;

    QStringList cookies_list;
    QStringList cookies_chromium_list;
    QStringList cookies_qaxbrowser_list;
    QString trace_firefox_count;
    QString trace_chromium_count;
    QString trace_qaxbrowser_count;
    QString trace_system_count;
    QString trace_bash_size;
    QString trace_bash_path;
    QStringList trace_x11_list;

    QStringList trash_list;
    QStringList m_selected_trash_list;

    QStringList m_selectedAptList;
    QStringList m_selectedCookieList;
    QMap<QString, int> Cookie_map;

    QStringList m_selectedTraceX11List;

    void setCacheTip(qreal);
    void setCookieTip(int);

    bool isCookieFinish = false;
    bool isOthersFinish = false;
    void isAllScanAreFinish();
    QMap<QString, QVariant> mClearMap;

    bool isLocalCleanFinish = false;
    bool isOtherCLeanFinish = false;
    void isAllCleanAreFinish();

    bool isTrashCleanFinish = false;
    bool isOtherCacheCleanFinish = false;
    void isAllCacheCleanFinish();

    bool isTrashScanFinish = false;
    bool isOtherCacheScanFinish = false;
    void isAllCacheScanFinish();

    int themeColor;

    bool isSystemPix = true; //用来标记小图标是否需要在主题发生变化后进行改变
    bool isCookiePix = true;
    bool isTracePix = true;

    int mFontSize;
    QList<QLabel *> mTipLabelList;

    KAlertDialog *alertDialog = nullptr;

    QString mStrStatusTip;
};
} // namespace KylinRubbishClear


#endif // CLEANDETAILVEIW_H
