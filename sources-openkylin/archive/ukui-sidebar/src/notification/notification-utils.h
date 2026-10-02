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

#ifndef UKUI_SIDEBAR_NOTIFICATION_UTILS_H
#define UKUI_SIDEBAR_NOTIFICATION_UTILS_H

#include <QObject>

namespace UkuiNotification {
/**
    bool hasDefaultAction() const;
    QString defaultActionLabel();
    ActionList actions() const;
    QStringList actionState() const;
 */
class NotificationAction
{
    Q_GADGET
    Q_PROPERTY(bool isEnable READ isEnable)
    Q_PROPERTY(bool isDefault READ isDefault)
    Q_PROPERTY(int index READ index)
    Q_PROPERTY(Mode mode READ mode)
    Q_PROPERTY(QString icon READ icon)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString action READ action)
    Q_PROPERTY(QString state READ state)
public:
    enum Mode {
        Icon = 0,
        Text,
        IconAndText
    };
    Q_ENUM(Mode)

    enum State {
        Enable = 0,
        Disable,
    };
    Q_ENUM(State)

    NotificationAction() = default;
    NotificationAction(bool e, bool d, int index, Mode mode, QString icon, QString name, QString state);

    bool isEnable() const;
    void setEnable(bool e);
    bool isDefault() const;
    void setDefault(bool d);

    int index() const;
    void setIndex(int index);

    NotificationAction::Mode mode() const;
    void setMode(Mode mode);

    QString icon() const;
    void setIcon(const QString &icon);

    QString name() const;
    void setName(const QString &name);

    QString action() const;
    void setAction(const QString &action);

    QString state() const;
    void setState(const QString &state);

private:
    bool m_enable {true};
    bool m_default {true};
    int  m_index {0};
    Mode m_mode {Text};
    QString m_icon;
    QString m_name;
    QString m_action;
    // TODO use enum?
    QString m_state;
};

} // UkuiNotification

Q_DECLARE_METATYPE(UkuiNotification::NotificationAction)
#endif //UKUI_SIDEBAR_NOTIFICATION_UTILS_H
