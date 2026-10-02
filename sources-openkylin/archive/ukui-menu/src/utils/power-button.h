/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */
#ifndef POWERBUTTON_H
#define POWERBUTTON_H

#include <QObject>
#include <QDBusInterface>
#include <QMenu>
#include <QPointer>

namespace UkuiMenu {

class PowerButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString icon READ getIcon NOTIFY iconChanged)
    Q_PROPERTY(QString toolTip READ getToolTip NOTIFY toolTipChanged)
public:
    explicit PowerButton(QObject *parent = nullptr);
    ~PowerButton() override;
    static void defineModule(const char *uri, int versionMajor, int versionMinor);
    QString getIcon();
    QString getToolTip();

public Q_SLOTS:
    void clicked(bool leftButtonClicked, int mouseX, int mouseY, bool isFullScreen);

private:
    void openMenu(int menuX, int menuY, bool isFullScreen);
    bool functionControlOperation(QString operation);
    bool hasMultipleUsers();
    bool canSwitch();
    //检测更新
    bool canUpgrade();
    //检测有更新时是否需要隐藏关机和重启选项
    bool checkHideUpdate();
    void execUpgradeAction(const QString &action);
    QString minutesToHour(int estmateTime);

    QPointer<QMenu> m_contextMenu;
    //检测更新并重启和更新并关机预计剩余时间(单位：分钟)
    int m_estimatedLeftMinutes = 0;

Q_SIGNALS:
    void iconChanged();
    void toolTipChanged();
};

} // UkuiMenu

#endif // POWERBUTTON_H
