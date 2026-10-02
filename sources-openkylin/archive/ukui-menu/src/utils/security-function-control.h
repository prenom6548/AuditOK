/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#ifndef SECURITYFUNCTIONCONTROL_H
#define SECURITYFUNCTIONCONTROL_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

namespace UkuiMenu {

class SecurityFunctionControl : public QObject
{
    Q_OBJECT
public:
    static SecurityFunctionControl *instance();

    Q_PROPERTY(bool disableMouseLeftButton READ disableMouseLeftButton NOTIFY disableMouseLeftButtonChanged)
    Q_PROPERTY(bool disableMouseRightButton READ disableMouseRightButton NOTIFY disableMouseRightButtonChanged)
    Q_PROPERTY(bool disableMouseHover READ disableMouseHover NOTIFY disableMouseHoverChanged)
    Q_PROPERTY(bool showFoldButton READ showFoldButton NOTIFY showFoldButtonChanged)

    bool disable() const;
    bool disableMouseLeftButton() const;
    bool disableMouseRightButton() const;
    bool disableMouseHover() const;
    QStringList appWhiteList() const;
    QStringList appBlackList() const;
    //567643新增需求
    bool disableUninstallAction() const;
    bool windowMaximize() const;
    bool showFoldButton() const;
    QString powerButtonSoleOperation() const;

    bool canAppDisplay(const QString &desktop) const;
    bool windowMaximizeControl() const;

Q_SIGNALS:
    void disableChanged();
    void disableMouseLeftButtonChanged();
    void disableMouseRightButtonChanged();
    void disableMouseHoverChanged();
    void appWhiteListChanged();
    void appBlackListChanged();
    void disableUninstallActionChanged();
    void windowMaximizeChanged();
    void showFoldButtonChanged();
    void powerButtonSoleOperationChanged();
    void windowMaximizeControlChanged();

private:
    explicit SecurityFunctionControl(QObject *parent = nullptr);

    bool compareListBySort(QStringList &list1, QStringList &list2);

    bool m_disable {false};
    bool m_disableMouseLeftButton {false};
    bool m_disableMouseRightButton {false};
    bool m_disableMouseHover {false};
    QStringList m_appWhiteList {};
    QStringList m_appBlackList {};
    bool m_disableUninstallAction {false};
    bool m_windowMaximize {true};
    bool m_showFoldButton {true};
    QString m_powerButtonSoleOperation {""};
    bool m_windowMaximizeControl {false};
//  （logOut  lockScreen suspend hibernate shutDown reStart upgradeAndShutDown upgradeAndRestart）

private Q_SLOTS:
    void onFunctionControlChanged(const QString& name, bool exit = false);
};

} // UkuiMenu
#endif // SECURITYFUNCTIONCONTROL_H
