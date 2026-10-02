/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SUBSYSTEMINSTALLMODULE_H
#define SUBSYSTEMINSTALLMODULE_H

#include <QFrame>
#include <QLabel>
#include <QObject>
#include <QVBoxLayout>
#include <ukcc/interface/interface.h>

#include "subsystemwidget.h"

class SubSystemInstallModule : public QObject, CommonInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit SubSystemInstallModule(QObject *parent = nullptr);

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    QString translationPath() const Q_DECL_OVERRIDE;

private:
    void initUI();
    void connectSignals();
    void initSearchText();

private:
    QString pluginName;
    int pluginType;

    QWidget *mainWidget = nullptr;
    QVBoxLayout *mainVLayout = nullptr;
    QVBoxLayout *pluginVLayout = nullptr;
    QFrame *mainFrame = nullptr;
    QObject *plugin = nullptr;
    QWidget *modelConfigWidget = nullptr;
    QWidget *computePowerSharingWidget = nullptr;
    QLabel *loadingPluginText = nullptr;

    SubSystemWidget *subSystemWidget = nullptr;
    bool mFirstLoad = true;
};

#endif // SUBSYSTEMINSTALLMODULE_H
