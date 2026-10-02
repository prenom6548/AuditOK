/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
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
#ifndef ACCESSIBILITY_H
#define ACCESSIBILITY_H

#include <QObject>
#include <QFrame>
#include <QVBoxLayout>
#include <QWidget>
#include <QtPlugin>
#include <QImageReader>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDBusObjectPath>
#include <QProcess>
#include <QStringList>
#include <QGSettings>
#include <QPluginLoader>

#include "shell/interface.h"
#include "subplugininterface.h"
#include "ukcccommon.h"
using namespace ukcc;

#include "kswitchbutton.h"
using namespace kdk;

#include "shell/interface.h"

#define STYLE_NAME_KEY              "style-name"
#define STYLE_NAME_CHANGE_KEY       "styleName"
#define ZOOM_LABEL_TRANSPARENT      0.35 * 255
#define COLOR_WHITE_NUMBER          255
#define COLOR_BALCK_NUMBER          0

namespace Ui {
class Accessibility;
}

class Accessibility : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Accessibility();
    ~Accessibility();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    int                 m_pluginType;
    bool                m_firstLoad;
    bool                m_zoomSwitchSync        = false;
    bool                m_screenReaderSwitchSync = false;
    QString             m_pluginName;

    Ui::Accessibility*  ui;
    QWidget*            m_pPluginWidget         = nullptr;
    QDBusInterface*     m_accessibilityInterface = nullptr;
    KSwitchButton*      m_pFilterSwitchButton   = nullptr;
    KSwitchButton*      m_pShortcutSwitchButton = nullptr;
    KSwitchButton*      m_pScreenReaderSwitchButton = nullptr;
    QGSettings*         m_pScreenReaderSetting = nullptr;
    QGSettings*         m_pUkccSetting = nullptr;
    bool                m_hasReaderVolumeKey = false;
    bool                m_hasReaderSpeedKey = false;
    bool                m_isSupportEffect = false;
    bool                m_isOrcaInstalled = false;
    int                 m_orcaVerifyAttempt = 0;
    qint64              m_screenReaderEnableRequestedAt = 0;

    struct SubPluginEntry {
        QPluginLoader *loader = nullptr;
        QWidget *widget = nullptr;
        QWidget *topSpacing = nullptr;
        QGSettings *settings = nullptr;
        SubPluginInterface *sub = nullptr;
    };
    QList<SubPluginEntry> m_subPlugins;

    void scheduleOrcaVerify();
    void verifyOrcaRunningAfterEnable();
    bool isWithinScreenReaderStartGracePeriod() const;
    void initUI();
    void initTabOrder();
    void initConnection();
    void initQGSettingsAndDBus();
    void initDefaultData();
    void initSubPluginArea();
    void loadSubPlugins();
    QGSettings *createSubPluginGsettings(const QString &subName, bool isEnable);
    void connectSubPluginSettings(int index);
    void onSubPluginGsettingsChanged(const QString &key, int index);
    void updateSubPluginVisible(SubPluginEntry &entry);
    void onSubPluginSettingChanged(const QString &key, SubPluginEntry *entry);

private slots:
    void setZoomMode(int index);
    void setFilterMode(int index);
    void setZoomSwitch(bool toggled);
    void setFilterSwitch(bool toggled);
    void setShortcutSwitch(bool enabled);
    void onScreenReaderStateChanged(bool checked);
    void onScreenReaderSwitchToggled(bool checked);
    void onScreenReaderSettingChanged(const QString &key);
    void syncScreenReaderSwitch(bool status);
    void onScreenReaderVolumeChanged(int value);
    void onScreenReaderSpeedChanged(int value);
    void syncZoomComboBox(int mode);
    void syncZoomSwitch(bool status);
    void syncFilterSwitch(bool status);
    void syncShortcutSwitch(bool status);
    void syncFilterComboBox(QString type);

};
#endif // ACCESSIBILITY_H
