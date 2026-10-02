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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_PANEL_CALENDAR_H
#define UKUI_PANEL_CALENDAR_H

#include <QObject>

class Calendar : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString time READ time NOTIFY timeUpdated)
    Q_PROPERTY(QString ap READ ap NOTIFY timeUpdated)
    Q_PROPERTY(QString week READ week NOTIFY timeUpdated)
    Q_PROPERTY(QString year READ year NOTIFY timeUpdated)
    Q_PROPERTY(QString month READ month NOTIFY timeUpdated)
    Q_PROPERTY(QString separator READ separator NOTIFY timeUpdated)
    Q_PROPERTY(QString tooltipText READ tooltipText NOTIFY timeUpdated)
    Q_PROPERTY(QStringList allData READ allData NOTIFY timeUpdated)
    Q_PROPERTY(int unreadMsgCount READ unreadMsgCount NOTIFY unreadMsgChanged)
    Q_PROPERTY(QString notifyIconTooltipText READ notifyIconTooltipText NOTIFY unreadMsgChanged)
    Q_PROPERTY(bool disableMouseLeftButton READ disableMouseLeftButton NOTIFY disableMouseLeftButtonChanged)
    Q_PROPERTY(bool disableMouseRightButton READ disableMouseRightButton NOTIFY disableMouseRightButtonChanged)

public:
    explicit Calendar(QObject *parent = nullptr);
    ~Calendar() override;

    QString time() const;
    QString ap() const;
    QString week() const;
    QString year() const;
    QString month() const;
    QString separator() const;
    QString tooltipText() const;

    QStringList allData() const;

    int  unreadMsgCount() const;
    QString notifyIconTooltipText();

    bool disableMouseLeftButton() const;
    bool disableMouseRightButton() const;

    Q_INVOKABLE void openSidebar();
    Q_INVOKABLE void activeCalendar();

Q_SIGNALS:
    void timeUpdated();
    void unreadMsgChanged();
    void disableMouseLeftButtonChanged();
    void disableMouseRightButtonChanged();

private Q_SLOTS:
    void timeSignalSlot();
    void onStateChanged(const QString &module, const QVariantMap &data);
    void onShortDateSignal();
    void onFunctionControlChanged(const QString& name, bool exit = false);

private:
    void initGSetting();
    void updateShortformatDate();
    void initFunctionControl();

    bool m_disableMouseLeftButton = false;
    bool m_disableMouseRightButton = false;

    QString m_dateFormat {"cn"};
    QString m_hourSystem {"24"};
    QString m_separator;
    QString m_shortDate;
    int m_unreadmsgCount = 0;
};


#endif //UKUI_PANEL_CALENDAR_H
