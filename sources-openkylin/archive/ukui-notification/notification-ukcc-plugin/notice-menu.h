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

#ifndef NOTICEMENU_H
#define NOTICEMENU_H

#include <QObject>
#include <QMenu>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>
#include <QPointer>
#include <kswitchbutton.h>
#include "applications-settings.h"

using namespace kdk;

class NoticeMenu : public QMenu
{
    Q_OBJECT
public:
    explicit NoticeMenu(UkuiNotification::SingleApplicationSettings *settings, QWidget *parent = nullptr);

    void initStatus();
    void setVoiceEnable(bool state);
    void setDetailShowOnLockScreenEnable(bool state);
    void setShowOnLockScreenEnable(bool state);
    static QFrame *setLine();

    void updateState();

private:
    void initUi();
    void setFrame(QFrame *frame, QString str, KSwitchButton *btn);
    void setFrame(QFrame *frame, QString str, QRadioButton *btn);
    void setConnect();

private:
    KSwitchButton *m_voiceBtn = nullptr;
    KSwitchButton *m_detailBtn = nullptr;
    KSwitchButton *m_showBtn = nullptr;
    QButtonGroup *m_styleGroup = nullptr;
    QRadioButton *m_noneBtn = nullptr;
    QRadioButton *m_mutativeBtn = nullptr;
    QRadioButton *m_alwaysBtn = nullptr;
    QPointer<QWidget> m_parent;
    QPointer<UkuiNotification::SingleApplicationSettings> m_settings;
protected:
    void showEvent(QShowEvent *event) override;

Q_SIGNALS:
    void voiceSignals(bool checked);
    void detailSignals(bool checked);
    void showSignals(bool checked);
    void styleBtnSignals(int id);
};

#endif // NOTICEMENU_H
