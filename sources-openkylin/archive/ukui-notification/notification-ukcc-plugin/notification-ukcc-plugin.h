/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 *
 */

#ifndef NOTIFICATION_UKCC_PLUGIN_H
#define NOTIFICATION_UKCC_PLUGIN_H

#include <QVBoxLayout>
#include <QDir>
#include <QComboBox>

#include <ukui-search/application-info.h>

#include <kysdk/applications/kswitchbutton.h>
#include <ukcc/interface/interface.h>
#include <ukcc/widgets/titlelabel.h>

#include "applications-settings.h"
#include "notification-global-settings.h"

using namespace kdk;

class Notice : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Notice();
    ~Notice();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    QString translationPath() const override;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    static QString baseAccessibleName();

    void initUi(QWidget *widget);
    void initNotFaze(QFrame *frame);
    void setFrame(QFrame *frame, KSwitchButton *btn, QString str);
    void initSearchText();
    void initConnection();
    void initNoticeStatus();
    void initListUI();
    void addItemUi(const UkuiSearch::ApplicationInfoMap& appInfoMap);
    void initItemUi(const QString &desktopPath,
                    const UkuiSearch::ApplicationPropertyMap& properties,
                    UkuiNotification::SingleApplicationSettings *settings);
    void setAppIcon(QLabel *iconlabel, const QString &icon);

Q_SIGNALS:
    void appDataInfo(const UkuiSearch::ApplicationInfoMap&);

private:
    void setHiddenNoticeApp(bool status);
    void setComBoxStatus(bool status);
    void removeLayoutItem(int i);

private:
    QString m_pluginName;
    int m_pluginType;
    QVector<QString> m_appList;

    QWidget *m_pluginWidget = nullptr;

    TitleLabel *m_noticeLabel = nullptr;
    TitleLabel *m_notFazeLabel = nullptr;
    QLabel *m_getNoticeLabel = nullptr;

    QFrame *m_noticeAppFrame = nullptr;
    QFrame *m_getNoticeFrame = nullptr;

    KSwitchButton *m_autoOpenSwitchBtn = nullptr;
    KSwitchButton *m_multiScreenSwitchBtn = nullptr;
    KSwitchButton *m_fullScreenSwitchBtn = nullptr;
    KSwitchButton *m_allowAlarmSwitchBtn = nullptr;
    KSwitchButton *m_enableSwitchBtn = nullptr;

    QComboBox *m_openTimeHComboBox = nullptr;
    QComboBox *m_openTimeMComboBox = nullptr;
    QComboBox *m_closeTimeHComboBox = nullptr;
    QComboBox *m_closeTimeMComboBox = nullptr;

    QGSettings *m_themeSetting = nullptr;

    QVBoxLayout *m_appListVerticalLayout = nullptr;

    UkuiNotification::NotificationGlobalSettings *m_globalSettings = nullptr;
};

#endif // NOTIFICATION_UKCC_PLUGIN_H
