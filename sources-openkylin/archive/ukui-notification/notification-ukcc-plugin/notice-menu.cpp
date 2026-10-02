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

#include "notice-menu.h"
#include <QDebug>
#include <QVBoxLayout>
#include <ukcc/widgets/fixlabel.h>
#include "security-control-config.h"

NoticeMenu::NoticeMenu(UkuiNotification::SingleApplicationSettings *settings, QWidget *parent) :
    QMenu(parent),
    m_settings(settings),
    m_parent(parent)
{
    setFixedSize(380, 190);
    initUi();
    setConnect();
}

void NoticeMenu::initStatus()
{
    m_voiceBtn->blockSignals(true);
    m_showBtn->blockSignals(true);
    m_detailBtn->blockSignals(true);

    m_voiceBtn->setChecked(m_settings->allowSound());
    m_detailBtn->setChecked(m_settings->showContentOnLockScreen());
    m_showBtn->setChecked(m_settings->showNotificationOnLockScreen());
    m_showBtn->setEnabled(m_settings->isShowNotificationSwitchOnLockScreenEditable());

    m_voiceBtn->blockSignals(false);
    m_showBtn->blockSignals(false);
    m_detailBtn->blockSignals(false);
}

void NoticeMenu::setVoiceEnable(bool state)
{
    m_voiceBtn->blockSignals(true);
    m_voiceBtn->setChecked(state);
    m_voiceBtn->blockSignals(false);
}

void NoticeMenu::setDetailShowOnLockScreenEnable(bool state)
{
    m_detailBtn->blockSignals(true);
    m_detailBtn->setChecked(state);
    m_detailBtn->blockSignals(false);
}

void NoticeMenu::setShowOnLockScreenEnable(bool state)
{
    m_showBtn->blockSignals(true);
    m_showBtn->setChecked(state);
    m_showBtn->blockSignals(false);
}

void NoticeMenu::initUi()
{
    QVBoxLayout *menuLyt = new QVBoxLayout(this);
    menuLyt->setSpacing(0);
    menuLyt->setContentsMargins(8, 0, 8, 0);

    const QString baseAccessibleName = m_parent->property("localName").toString();
    QFrame *voiceFrame = new QFrame(this);
    m_voiceBtn = new KSwitchButton;
    const QString voiceBtnText = tr("Beep sound when notified");
    m_voiceBtn->setTranslucent(true);
    m_voiceBtn->setAccessibleName(baseAccessibleName + tr("allow") + voiceBtnText);
    setFrame(voiceFrame, voiceBtnText, m_voiceBtn);

    QFrame *line_1 = setLine();

    QFrame *detailFrame = new QFrame(this);
    const QString detailBtnText = tr("Show message  on screenlock");
    m_detailBtn = new KSwitchButton;
    m_detailBtn->setTranslucent(true);
    m_detailBtn->setAccessibleName(baseAccessibleName + tr("allow") + detailBtnText);

    setFrame(detailFrame, detailBtnText, m_detailBtn);

    QFrame *line_2 = setLine();

    QFrame *showFrame = new QFrame(this);
    m_showBtn = new KSwitchButton;
    const QString showBtnText = tr("Show noticfication  on screenlock");
    m_showBtn->setTranslucent(true);
    m_showBtn->setAccessibleName(baseAccessibleName + tr("allow") + showBtnText);
    setFrame(showFrame, showBtnText, m_showBtn);

    menuLyt->addWidget(voiceFrame);
    menuLyt->addWidget(line_1);
    menuLyt->addWidget(showFrame);
    menuLyt->addWidget(line_2);
    menuLyt->addWidget(detailFrame);
    menuLyt->addSpacing(16);
    menuLyt->addStretch();
}

void NoticeMenu::setFrame(QFrame *frame, QString str, KSwitchButton *btn)
{
    frame->setFixedHeight(60);
    frame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *hLyt = new QHBoxLayout(frame);
    hLyt->setContentsMargins(24, 0, 24, 0);
    hLyt->setSpacing(8);
    FixLabel *label = new FixLabel(str, frame);
    label->setFixedWidth(250);
    hLyt->addWidget(label);
    hLyt->addStretch();
    hLyt->addWidget(btn);
}

void NoticeMenu::setFrame(QFrame *frame, QString str, QRadioButton *btn)
{
    frame->setFixedHeight(48);
    frame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *hLyt = new QHBoxLayout(frame);
    hLyt->setContentsMargins(24, 0, 24, 0);
    FixLabel *label = new FixLabel(str, frame);
    label->setFixedWidth(288);
    hLyt->addWidget(btn);
    hLyt->addWidget(label);
    hLyt->addStretch();
}

QFrame *NoticeMenu::setLine()
{
    QFrame *line = new QFrame();
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void NoticeMenu::updateState()
{
    m_showBtn->blockSignals(true);
    m_showBtn->setChecked(m_settings->showNotificationOnLockScreen());
    m_showBtn->setEnabled(m_settings->isShowNotificationSwitchOnLockScreenEditable());
    m_showBtn->blockSignals(false);
}

void NoticeMenu::setConnect()
{
    connect(m_voiceBtn, &KSwitchButton::stateChanged, this, &NoticeMenu::voiceSignals);
    connect(m_detailBtn, &KSwitchButton::stateChanged, this, &NoticeMenu::detailSignals);
    connect(m_showBtn, &KSwitchButton::stateChanged, this, &NoticeMenu::showSignals);
}

void NoticeMenu::showEvent(QShowEvent *event)
{
    int menuXPos = this->pos().x();
    int menuWidth = this->size().width()-4;
    int buttonWidth = 36;
    QPoint pos = QPoint(menuXPos - menuWidth + buttonWidth,
                        this->pos().y());
    this->move(pos);
}
