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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QPluginLoader>
#include <QTimer>
#include <QMainWindow>
#include <QPaintEvent>
#include <QShowEvent>
#include <QMoveEvent>
#include <QHideEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QVariantMap>
#include <QGSettings/QGSettings>
#include <QHBoxLayout>
#include <QToolButton>
#include <QScrollArea>
#include <kmenubutton.h>
#include <QEvent>
#include <QFutureWatcher>
#include <QPointer>
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>


#include "kaboutdialog.h"
#include "interface.h"
#include "homepagewidget.h"
#include "modulepagewidget.h"
#include "searchwidget.h"
#include "knavigationbar.h"
#include "windowmanager/windowmanager.h"
#include "prescene.h"

class QLabel;
class QPushButton;
class QButtonGroup;

using namespace kdk;
namespace Ui {
class MainWindow;
}

class UkccArgEvent : public QEvent
{
private:
    QString mstr;

public:
    const static Type TYPE = static_cast<Type>(QEvent::User+0x10);
    explicit UkccArgEvent(QString s) : QEvent(TYPE)
    {
        mstr = s;
    }

    QString str()
    {
        return mstr;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QTime startTime;

public:
    void initPlugins();
    void closePrescene();
    void setModuleBtnHightLight(int id);
    void bootOptionsFilter(QString opt, bool firstIn = true);        // 模块跳转
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
    QString pluginBtnHoverColor(QString styleName, bool hoverFlag);
    QVariantMap getSearchItems();
    void loadAllModules();
    void initPageWidget(bool isDbus);
    void loadSearchXml();
    void initWindowSize();

    void initDbusConfigTranslation();
protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE; //键盘松开事件

private:
    Ui::MainWindow *ui;

private:
    HomePageWidget * homepageWidget;
    ModulePageWidget * modulepageWidget = nullptr;

    QButtonGroup * leftBtnGroup;
    QButtonGroup * leftMicBtnGroup; //

    QDir pluginsDir;
    QDir updatePluginDir;

    SearchWidget      * m_searchWidget;

    QPushButton *backBtn;
    KMenuButton *mOptionBtn;
    QPushButton *minBtn = nullptr;
    QPushButton *maxBtn = nullptr;
    QPushButton *closeBtn;
    QLabel      *titleLabel = nullptr; // 首页设置标题
    QLabel      *mTitleIcon;
    QLabel      *m_pTextSetLabel = nullptr; // 详情页侧边栏设置标题
    QIcon       m_titleIcon;
    QLabel      *mLogoSetLabel;
    QTimer      *timer;
    QTimer      *m_debounceTimer = nullptr;  // 防抖定时器，用于处理快速双击
    QTimer      *m_pluginRefreshTimer = nullptr;  // 防抖定时器，用于处理Tab键快速触发
    CommonInterface *m_pendingPlugin = nullptr;  // 待刷新的插件
    bool m_sidebarSkipClick = false;
    QStringList       m_updatePlugins;
    QVariantMap       m_ModuleMap;
    QGSettings       *m_fontSetting;

    QHBoxLayout     *titleLayout;
    KNavigationBar *scrollArea = nullptr;

    QDBusInterface *m_statusSessionDbus;
    bool m_tabletmode = false;
    bool m_isSetUI = false;

    QList<WindowId> m_listWinIds;
    QMap<QString, QGSettings *> vecGsettins;
    QList<char *> pluginPathList;
    int mFirstKey = 0;
    bool isLoad;
    bool loadWidget;
    bool m_presceneShown;  // 跟踪预览界面是否已显示
    QFutureWatcher<void> m_watcher;
    QGSettings* m_ukccSettings = nullptr;
    QList<CommonInterface*> m_pluginList;
    QList<QPluginLoader *> m_subPluginLoaders;

    QString m_secondWidgetName = "";
    bool m_isOpen3adm = false;
    QDBusInterface* m_fontInterface = nullptr;
    UkuiWindowHelper *m_pUkuiWindowHelper = nullptr;
    QStringList m_ukccPlugins;

    PreScene *m_prescene = nullptr;

    QSize m_windowSize;
    // 离开首页前保存的焦点（搜索框或首页卡片/插件），返回首页或切回窗口时恢复
    QPointer<QWidget> m_homepageSavedFocus;

private:
    void preInit();
    void initUI();
    void initTileBar();
    void setBtnLayout(QPushButton * &pBtn);
    void loadPlugins();
    void loadUpdatePlugins();
    void determinePlugin(const QString& fileName, const QDir& dir);
    QString subPluginDirFor(const QString &l1Name) const;
    void registerSubPluginSearchPaths(CommonInterface *l1);
    void loadSpecifiedPlugin(QString pluginFullName);
    void initLeftsideBar();
    void navigateSidebarPlugin(const QModelIndex &index);
    QGSettings *setGsettingsPath(const QList<char *> &list, const QString &name, bool isEnable);
    bool isExitsCloudAccount();
    bool isExitWirelessDevice();
    bool dblOnEdge(QMouseEvent *event);
    void initStyleSheet();
    void changeSearchSlot();
    void showGuide(QString pluName);
    void hideComponent();
    bool isFontSelectFile(const QString &msg = nullptr);
    bool isFontFile(QString path);
    void pluginWatchSlot(bool isDbus);
    void pluginSettingChangedSlot(QString key, QString plugini18nName, QString pluginName, bool show);
    void processMessageContent(const QString &msg);
    void presceneTimeoutSlot();
    void refreshSidebarAfterThemeChange();
    bool isHomepageContextFocus(QWidget *widget) const;
    // setupUi 且页面已挂载后才可访问 ui->stackedWidget
    bool isMainUiReady() const;
    void saveHomepageFocus();
    void restoreHomepageFocus();

public slots:
    void pluginBtnClicked(QObject * plugin);
    void functionBtnClicked(QObject * plugin, QString jumpText = "");
    void sltMessageReceived(const QString &msg);
    void switchPage(QString moduleName, QString jumpMoudle, QString jumpText = "");
    void mainWindow_statusDbusSlot(bool tablet_mode);
    void initUkccAbout();
    void onF1ButtonClicked();
    void initPageWidgetSlot();

Q_SIGNALS:
    void posChanged();
    void specifiedPluginLoaded(CommonInterface *pluginInstance);
    void tabletModeChanged(bool isTablet);
};

#endif // MAINWINDOW_H
