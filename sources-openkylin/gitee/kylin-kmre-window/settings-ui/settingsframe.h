/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SETTINGSFRAME_H
#define SETTINGSFRAME_H

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

class DbusClient;
class QGSettings;
class QStackedWidget;
class QListWidget;
class QListWidgetItem;
class CameraWidget;
class AppMultiplierWidget;
class TrayAppWidget;
class AppSettingsPanel;
class GeneralSettingWidget;
class DeveloperWidget;
class Preferences;
class PopupTip;
class DisplayModeWidget;
class PhoneInfoWidget;
class AdvancedWidget;

class SettingsFrame : public QWidget
{
    Q_OBJECT

public:
    SettingsFrame(QWidget *parent = 0);
    ~SettingsFrame();

    void initDBus();
    void initInfos();
    void initCategoryList();
    void initStack();
    void addStackPage(QWidget *widget);
    void bootOptionsFilter(QString opt);
    void showWindow();
    void openDeveloperWidget();

public slots:
    void onNavigationBarChanged(int index);
    void changeInfoPage(QListWidgetItem *item);
    void slotMessageReceived(const QString &msg);
    void updateCurrentCameraDevice(const QString &deviceName);
    void onKmreDockerStopped();
    void onRestartDocker();
    void onGetSystemProp(const QString &value);

signals:
    void requestSystemProp(int type, const QString &field);
protected:
#ifdef UKUI_WAYLAND
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
#endif

private:
#ifdef UKUI_WAYLAND
    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_minBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;
#endif
    QListWidget *m_categoryListWidget = nullptr;
    QStackedWidget *m_stack = nullptr;
    QStringList m_categoryList;
    QString m_gpuVendor;
    QString m_gpuModel;
    QString m_cpuType;
    QString m_displayType;
    QString m_displayTypes;

    DbusClient *m_dbusClient = nullptr;
    QThread *m_dbusThread = nullptr;

    GeneralSettingWidget *m_generalWidget = nullptr;
    CameraWidget *m_cameraWidget = nullptr;
    AppMultiplierWidget *m_appMultiplierWidget = nullptr;
    AppSettingsPanel *m_appSettingsPanel = nullptr;
    DeveloperWidget *m_DeveloperWidget = nullptr;
    DisplayModeWidget *m_DisplayWidget = nullptr;
    PhoneInfoWidget *m_PhoneInfoWidget = nullptr;
    AdvancedWidget *m_AdvancedWidget = nullptr;
    QGSettings *m_ukuiSettings = nullptr;
    QString m_loginUserName;
    QString m_loginUserId;
    QTimer *m_timer = nullptr;
    PopupTip *popuptip = nullptr;
    int stopNum = 0;
};

#endif // SETTINGSFRAME_H
