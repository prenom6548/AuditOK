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

#ifndef TEST_POPUP_NOTIFICATION_H
#define TEST_POPUP_NOTIFICATION_H

#include <QDateTime>
#include <QImage>
#include <QList>
#include <QStringList>

namespace UkuiNotification {

using ActionList = QList<QPair<QString, QString>>;

class PopupNotification
{
public:
    enum Urgency {
        LowUrgency = 0,
        NormalUrgency = 1,
        CriticalUrgency = 2
    };

    uint id() const { return m_id; }
    QString display() const { return m_display; }
    QString applicationName() const { return m_applicationName; }
    QString applicationIconName() const { return m_applicationIconName; }
    QString icon() const { return m_icon; }
    QString summary() const { return m_summary; }
    QString body() const { return m_body; }
    QString category() const { return m_category; }
    QImage image() const { return m_image; }
    QDateTime createdTime() const { return m_createdTime; }
    ActionList actions() const { return m_actions; }
    QStringList actionState() const { return m_actionState; }
    bool hasDefaultAction() const { return m_hasDefaultAction; }
    bool enableActionIcons() const { return m_enableActionIcons; }
    QString soundFile() const { return m_soundFile; }
    bool suppressSound() const { return m_suppressSound; }
    bool resident() const { return m_resident; }
    bool transient() const { return m_transient; }
    int urgency() const { return m_urgency; }
    int timeout() const { return m_timeout; }
    bool noFold() const { return m_noFold; }
    int popupTimeout() const { return m_popupTimeout; }
    QString desktopEntry() const { return m_desktopEntry; }
    QString soundName() const { return m_soundName; }

    void setId(uint value) { m_id = value; }
    void setDisplay(const QString &value) { m_display = value; }
    void setApplicationName(const QString &value) { m_applicationName = value; }
    void setApplicationIconName(const QString &value) { m_applicationIconName = value; }
    void setIcon(const QString &value) { m_icon = value; }
    void setSummary(const QString &value) { m_summary = value; }
    void setBody(const QString &value) { m_body = value; }
    void setCategory(const QString &value) { m_category = value; }
    void setImage(const QImage &value) { m_image = value; }
    void setCreatedTime(const QDateTime &value) { m_createdTime = value; }
    void setActions(const ActionList &value) { m_actions = value; }
    void setActionState(const QStringList &value) { m_actionState = value; }
    void setHasDefaultAction(bool value) { m_hasDefaultAction = value; }
    void setEnableActionIcons(bool value) { m_enableActionIcons = value; }
    void setSoundFile(const QString &value) { m_soundFile = value; }
    void setSuppressSound(bool value) { m_suppressSound = value; }
    void setResident(bool value) { m_resident = value; }
    void setTransient(bool value) { m_transient = value; }
    void setUrgency(int value) { m_urgency = value; }
    void setTimeout(int value) { m_timeout = value; }
    void setNoFold(bool value) { m_noFold = value; }
    void setPopupTimeout(int value) { m_popupTimeout = value; }
    void setDesktopEntry(const QString &value) { m_desktopEntry = value; }
    void setSoundName(const QString &value) { m_soundName = value; }

    bool operator==(const PopupNotification &other) const
    {
        return m_id == other.m_id
            && m_display == other.m_display
            && m_applicationName == other.m_applicationName
            && m_applicationIconName == other.m_applicationIconName
            && m_icon == other.m_icon
            && m_summary == other.m_summary
            && m_body == other.m_body
            && m_category == other.m_category
            && m_image == other.m_image
            && m_createdTime == other.m_createdTime
            && m_actions == other.m_actions
            && m_actionState == other.m_actionState
            && m_hasDefaultAction == other.m_hasDefaultAction
            && m_enableActionIcons == other.m_enableActionIcons
            && m_soundFile == other.m_soundFile
            && m_suppressSound == other.m_suppressSound
            && m_resident == other.m_resident
            && m_transient == other.m_transient
            && m_urgency == other.m_urgency
            && m_timeout == other.m_timeout
            && m_noFold == other.m_noFold
            && m_popupTimeout == other.m_popupTimeout
            && m_desktopEntry == other.m_desktopEntry
            && m_soundName == other.m_soundName;
    }

private:
    uint m_id {0};
    QString m_display;
    QString m_applicationName;
    QString m_applicationIconName;
    QString m_icon;
    QString m_summary;
    QString m_body;
    QString m_category;
    QImage m_image;
    QDateTime m_createdTime;
    ActionList m_actions;
    QStringList m_actionState;
    bool m_hasDefaultAction {false};
    bool m_enableActionIcons {false};
    QString m_soundFile;
    bool m_suppressSound {false};
    bool m_resident {false};
    bool m_transient {true};
    int m_urgency {NormalUrgency};
    int m_timeout {0};
    bool m_noFold {false};
    int m_popupTimeout {0};
    QString m_desktopEntry {"test.desktop"};
    QString m_soundName;
};

} // namespace UkuiNotification

Q_DECLARE_METATYPE(UkuiNotification::PopupNotification)

#endif
