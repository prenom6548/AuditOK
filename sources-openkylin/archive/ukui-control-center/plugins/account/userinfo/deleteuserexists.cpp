/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "deleteuserexists.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QFrame>
#include <QButtonGroup>
#include <QMouseEvent>
#include <QDebug>

#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>

#define DIALOG_WIDTH 416

DeleteUserExists::DeleteUserExists(QString name, QString nick, QWidget *parent) :
    KDialog(parent),
    _name(name),
    _nick(nick)
{
    setFixedWidth(DIALOG_WIDTH);
    setWindowTitle(tr("Delete User"));
    setWindowIcon("ukui-control-center");
    UkuiWindowHelper *m_pUkuiWindowHelper = new UkuiWindowHelper(this);
    // 禁用最大化
    UkuiWindowHelper::States defaultState = UkuiWindowHelper::States(0xff);
    m_pUkuiWindowHelper->setWindowState(defaultState & ~UkuiWindowHelper::State::Maximizable);

    initUI();
    setConnect();
}

DeleteUserExists::~DeleteUserExists()
{
}

void DeleteUserExists::initUI()
{
    //提示信息
    QString note1 = tr("Delete user '");
    QString note2 = tr("'? And:");
    KLabel *noteLabel = new KLabel(this);
    if (!_nick.isEmpty()){
        noteLabel->setText(note1 + _nick + note2);
    } else {
        noteLabel->setText(note1 + _name + note2);
    }

    QHBoxLayout *noteHorLayout = new QHBoxLayout;
    noteHorLayout->setSpacing(0);
    noteHorLayout->setContentsMargins(0, 0, 0, 0);
    noteHorLayout->addWidget(noteLabel);

    m_pRemoveBtnGroup = new QButtonGroup(this);
    m_pRemoveButKeepFilesRadioBtn = new QRadioButton;
    m_pRemoveButKeepFilesRadioBtn->setFixedSize(QSize(16, 16));
    m_pRemoveWholeRadioBtn = new QRadioButton;
    m_pRemoveWholeRadioBtn->setFixedSize(QSize(16, 16));
    m_pRemoveBtnGroup->addButton(m_pRemoveButKeepFilesRadioBtn, 0);
    m_pRemoveBtnGroup->addButton(m_pRemoveWholeRadioBtn, 1);
    m_pRemoveWholeRadioBtn->setChecked(true);
    m_pRemoveButKeepFilesRadioBtn->setAccessibleName(tr("Keep desktop, files, favorites, music of the user"));
    m_pRemoveWholeRadioBtn->setAccessibleName(tr("Delete whole data belong user"));

    KLabel *removeButKeepFilesLabel = new KLabel(this);
    removeButKeepFilesLabel->setWordWrap(true);
    removeButKeepFilesLabel->setText(tr("Keep desktop, files, favorites, music of the user"));
    KLabel *removeWholeLabel = new KLabel(this);
    removeWholeLabel->setWordWrap(true);
    removeWholeLabel->setText(tr("Delete whole data belong user"));

    QHBoxLayout *removeButKeepFilesHorLayout = new QHBoxLayout;
    removeButKeepFilesHorLayout->setContentsMargins(0, 0, 0, 0);
    removeButKeepFilesHorLayout->addWidget(m_pRemoveButKeepFilesRadioBtn);
    removeButKeepFilesHorLayout->addWidget(removeButKeepFilesLabel);

    QHBoxLayout *removeWholeHorLayout = new QHBoxLayout;
    removeWholeHorLayout->setContentsMargins(0, 0, 0, 0);
    removeWholeHorLayout->addWidget(m_pRemoveWholeRadioBtn);
    removeWholeHorLayout->addWidget(removeWholeLabel);

    m_pRemoveButKeepFilesFrame = new QFrame;
    m_pRemoveButKeepFilesFrame->setFixedWidth(DIALOG_WIDTH - 48);
    m_pRemoveButKeepFilesFrame->setFrameShape(QFrame::Box);
    m_pRemoveButKeepFilesFrame->setFrameStyle(QFrame::Plain);
    m_pRemoveButKeepFilesFrame->setLayout(removeButKeepFilesHorLayout);
    m_pRemoveButKeepFilesFrame->installEventFilter(this);

    m_pRemoveWholeFrame = new QFrame;
    m_pRemoveWholeFrame->setFixedWidth(DIALOG_WIDTH - 48);
    m_pRemoveWholeFrame->setFrameShape(QFrame::Box);
    m_pRemoveWholeFrame->setFrameStyle(QFrame::Plain);
    m_pRemoveWholeFrame->setLayout(removeWholeHorLayout);
    m_pRemoveWholeFrame->installEventFilter(this);

    //底部按钮
    m_pCancelBtn = new KPushButton;
    m_pCancelBtn->setText(tr("Cancel"));
    m_pConfirmBtn = new KPushButton;
    m_pConfirmBtn->setText(tr("Confirm"));

    QHBoxLayout *bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
    bottomBtnsHorLayout->addSpacing(16);
    bottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    QVBoxLayout *mainVerLayout = new QVBoxLayout;
    mainVerLayout->setSpacing(0);
    mainVerLayout->setContentsMargins(24, 24, 24, 24);
    mainVerLayout->addLayout(noteHorLayout);
    mainVerLayout->addSpacing(16);
    mainVerLayout->addWidget(m_pRemoveButKeepFilesFrame);
    mainVerLayout->addSpacing(16);
    mainVerLayout->addWidget(m_pRemoveWholeFrame);
    mainVerLayout->addSpacing(32);
    mainVerLayout->addLayout(bottomBtnsHorLayout);
    mainVerLayout->setSizeConstraint(QLayout::SetMinimumSize);
    mainVerLayout->invalidate();
    mainVerLayout->activate();

    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    mainWidget()->setLayout(mainVerLayout);
    this->adjustSize();
}

QSize DeleteUserExists::sizeHint() const
{
    QSize size = QDialog::sizeHint();
    size.setWidth(DIALOG_WIDTH);  // 强制宽度
    return size;
}

void DeleteUserExists::setConnect()
{
    connect(m_pCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(m_pConfirmBtn, &QPushButton::clicked, this, [=]{
        emit confirmChanged(m_pRemoveBtnGroup->checkedId() == 1 ? true : false);

        close();
    });
}

bool DeleteUserExists::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == m_pRemoveButKeepFilesFrame){
                m_pRemoveButKeepFilesRadioBtn->setChecked(true);
            } else if (watched == m_pRemoveWholeFrame){
                m_pRemoveWholeRadioBtn->setChecked(true);
            }
        }
    }

    return KDialog::eventFilter(watched, event);
}

void DeleteUserExists::keyPressEvent(QKeyEvent * event)
{
    switch (event->key())
    {
    case Qt::Key_Enter:
        if (m_pConfirmBtn->isEnabled())
            m_pConfirmBtn->clicked();
        break;
    case Qt::Key_Return:
        if (m_pConfirmBtn->isEnabled())
            m_pConfirmBtn->clicked();
        break;
    default:
        KDialog::keyPressEvent(event);
    }
}

