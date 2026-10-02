/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QRadioButton>
#include <qgsettings.h>
#include <QFileSystemWatcher>
#include "customtype.h"
#include "contentpane.h"
#include "attributewindow.h"
#include "kswitchbutton.h"
#include "kaboutdialog.h"
#include "kalabel.h"
#include "uniauthservice.h"
#include "knavigationbar.h"
#include "kwidget.h"
#include "ui_stackedWidget.h"
#include <ukuiwindowhelper.h>
#include <windowmanager/windowmanager.h>
#include <kysdk/applications/accessinfohelper.h>

#define KYLIN_USER_GUIDE_PATH "/"
#define KYLIN_USER_GUIDE_SERVICE "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE "com.guide.hotel"

using namespace kdk;
class QLabel;
class AttributeWindow;
class QProcess;
class LoadingDialog;
class PreloadWidget;

typedef std::shared_ptr<DeviceInfo> DeviceInfoPtr;
typedef QList<DeviceInfoPtr> DeviceList;
typedef QMap<int, DeviceList> DeviceMap;
class MainWindow : public KWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QString usernameFromCmd, QWidget *parent = 0);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *event);
    /* Qt slots */
private slots:
    void onBtnDashBoardClicked();
    void onBtnFingerPrintClicked();
    void onBtnPermissionsClicked();
    void onBtnFingerVeinClicked();
    void onBtnIrisClicked();
    void onBtnVoicePrintClicked();
    void onBtnFaceClicked();
    void on_btnStatus_clicked();
    void on_btnLogin_clicked();
    void on_btnSaver_clicked();
    void on_btnSystem_clicked();
    void on_lblNoDevicePFingerPrint_clicked();
    void on_lblNoDevicePFace_clicked();
    void on_lblNoDevicePFingerVein_clicked();
    void on_lblNoDevicePIris_clicked();
    void on_lblNoDevicePVoicePrint_clicked();
    void on_listWidgetDevicesType_currentIndexChanged(int);
    void on_tableWidgetDevices_cellDoubleClicked(int row, int column);
    void onDriverStatusClicked();
    void onDefaultDeviceChanged(bool checked);
    bool changeDeviceStatus(DeviceInfo *deviceInfo, KSwitchButton *switchBtn);
    void onUSBDeviceHotPlug(int, int, int);
    void setCurStyleSheet();
    void onConfigurationChanged(QString key);
    void onUpdateDevice(int, int, int);
    void onDeleteFeatureState(int nState);
    void tabletStatusChanged(bool tablet_mode);
public slots:
    void onServiceStatusChanged(bool activate);
    void onReviceWindowMessage(QString message);

private:
    void delayInit();
    void prettify();
    void initSysMenu();
    void getDeviceInfo();
    void addContentPane(DeviceInfo *deviceInfo);
    void initialize();
    void initDeviceTypeList();
    void initBiometricPage();
    void initLeftsideBar();
    void updateBiometricPage(int type, int devNumNow);
    void initDashboardBioAuthSection();
    void initLoginSwitch();
    void initSaverSwitch();
    void initSystemSwitch();
    QPixmap *getUserAvatar(QString username);
    void setCurrentUser();
    void setVerificationStatus(QString userName, int type, bool status);
    void setLoginSwitch(bool status);
    void setSaverSwitch(bool status);
    void setSystemSwitch(bool status);
    int bioTypeToIndex(int type);
    bool restartService();
    void updateDevice();
    void updateDeviceListWidget(int biotype);
    void setDeviceStatus(KALabel *label, KALabel *label1, bool connected, int flag, int bioType, QString defaultName);
    int getDeviceNum(int type);
    void updateDeviceNum(int type);
    void clearDeviceNum();
    void raiseContentPane(DeviceInfo *deviceInfo);
    void setLastDeviceSelected();
    void sortContentPane();
    void showGuide(QString appName);
    int daemonIsNotRunning();
    void showAttributeWindow();
    void initPermissionWidget();
    void initPerwgtSize();
    QString getElidedText(QFont font, int width, QString strInfo);
    void setTText(QString text);
    void setStackedWidgetMain(QWidget *newWidget);
    void update_label_width();

    /* Members */
private:
    Ui::stackedWidget *uiStackedWidget;
    QLabel *lblStatus;
    QLabel *permissionLabel;
    AttributeWindow *dialog = nullptr;
    bool m_isShowDialog = false;

    KNavigationBar *scrollArea = nullptr;
    QStandardItem *permissionItem = nullptr;
    QStandardItem *dashBoardItem = nullptr;
    QStandardItem *fingerPrintItem = nullptr;
    QStandardItem *faceItem = nullptr;
    QStandardItem *fingerVeinItem = nullptr;
    QStandardItem *irisItem = nullptr;
    QStandardItem *voiceprintItem = nullptr;

    // QPushButton *labelIcon;
    DeviceMap deviceMap;
    // 由于互斥的QButtonGroup会保证至少有一个被选中，所以这里不使用它
    QList<QRadioButton *> btnGroup;
    //  监听平板模式变化
    QDBusInterface *m_statusSessionDbus;
    QDBusReply<bool> is_tabletmode;
    /* 用于和远端 DBus 对象交互的代理接口 */
    QDBusInterface *serviceInterface;
    int deviceCount;
    QMap<int, QList<DeviceInfo *>> deviceInfosMap;
    QMap<QString, ContentPane *> contentPaneMap;
    /* 通过命令行参数传入的用户名 */
    QString username;
    bool verificationStatus; // 生物识别开关状态
    bool loginSwitch;
    bool saverSwitch;
    bool systemSwitch;
    // for window move
    QPoint dragPos;
    bool dragWindow;

    QMenu *menu;
    KAboutDialog *aboutDlg;
    UniAuthService *m_uniAuthService = nullptr;

    /* 服务被关闭时提示 */
    QLabel *lblPrompt;
    DeviceInfo *lastDeviceInfo;
    QFileSystemWatcher *mWatcher;

    QColor m_colorBg;
    QGSettings *m_styleSettings = nullptr;
    float m_curFontSize;
    QString m_curStyle;

    QLabel *systemLabel;
    QLabel *saverLabel;
    QLabel *greetLabel;

    QProcess *m_procRestartService = nullptr;
    LoadingDialog *m_dialogLoading = nullptr;
    QProcess *m_procEnableDriver = nullptr;
    QList<QWidget *> m_listSwitchBtns;
    QList<WindowId> m_listWinIds;
    bool m_isShowDeleteAnimation = false;

    int fpNum = 0;
    int fcNum = 0;
    int fvNum = 0;
    int irNum = 0;
    int vpNum = 0;

    QString fp_defaultName;
    QString fc_defaultName;
    QString fv_defaultName;
    QString ir_defaultName;
    QString vp_defaultName;
    KWidget *widget = nullptr;

    QList<KALabel *> label_name;
    PreloadWidget *m_preLoadWidget = nullptr;
    UkuiWindowHelper *m_pUkuiWindowHelper = nullptr;
    bool m_bInited = false;
};

#endif // MAINWINDOW_H
