/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef AUTOTEST_WINDOWS_FAKE_SECURITY_FUNCTION_CONTROL_H
#define AUTOTEST_WINDOWS_FAKE_SECURITY_FUNCTION_CONTROL_H

#include <QObject>
#include <QStringList>

namespace UkuiMenu {

class SecurityFunctionControl : public QObject
{
    Q_OBJECT
public:
    static SecurityFunctionControl *instance();

    bool disable() const;
    bool disableMouseLeftButton() const;
    bool disableMouseRightButton() const;
    bool disableMouseHover() const;
    QStringList appWhiteList() const;
    QStringList appBlackList() const;
    bool disableUninstallAction() const;
    bool windowMaximize() const;
    bool showFoldButton() const;
    QString powerButtonSoleOperation() const;
    bool canAppDisplay(const QString &desktop) const;

    void setDisable(bool value);
    void setWindowMaximize(bool value);

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

private:
    explicit SecurityFunctionControl(QObject *parent = nullptr);

    bool m_disable = false;
    bool m_windowMaximize = true;
};

} // namespace UkuiMenu

#endif
