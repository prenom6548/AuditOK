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
#include "changeusertype.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>

#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>

#include "ukcccommon.h"
using namespace ukcc;

#define DIALOG_WIDTH 420

class SettingFrame : public QFrame
{
public:
    SettingFrame(QWidget *parent){}
    ~SettingFrame() {}

protected:
    void paintEvent(QPaintEvent *event)
    {
        int radius = qApp->property("kradius-normal").toInt();
        QPainter painter(this);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        if (qApp->property("kcontain-secondary-normal").canConvert<QBrush>()) {
            QBrush containColor = qApp->property("kcontain-secondary-normal").value<QBrush>();
            painter.setBrush(containColor);
        }
        painter.setPen(Qt::transparent);
        QRect rect = this->rect();
        painter.drawRoundedRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, rect.height() - 2, radius, radius);
        painter.restore();
        QFrame::paintEvent(event);
    }
};

ChangeUserType::ChangeUserType(QWidget *parent) :
    KDialog(parent)
{
    setFixedWidth(DIALOG_WIDTH);
    UkuiWindowHelper *m_pUkuiWindowHelper = new UkuiWindowHelper(this);
    // 禁用最大化
    UkuiWindowHelper::States defaultState = UkuiWindowHelper::States(0xff);
    m_pUkuiWindowHelper->setWindowState(defaultState & ~UkuiWindowHelper::State::Maximizable);

    setWindowTitle(tr("UserType"));
    setWindowIcon("ukui-control-center");
    initUI();
    setConnect();
}

ChangeUserType::~ChangeUserType()
{
}

void ChangeUserType::initUI()
{
    m_pUserLogoBtn = new KLabel();
    m_pUserLogoBtn->setFixedSize(QSize(64, 64));
    m_pNickNameLabel = new KLabel();
    m_pUserTypeLabel = new KLabel();
    m_pUserTypeLabel->setFontColor(Qt::gray);
    m_pUserTypeLabel->setFixedHeight(24);
    QVBoxLayout *userInfoVerLayout = new QVBoxLayout;
    userInfoVerLayout->setSpacing(0);
    userInfoVerLayout->setContentsMargins(0, 0, 0, 0);
    userInfoVerLayout->addStretch();
    userInfoVerLayout->addWidget(m_pNickNameLabel);
    userInfoVerLayout->addWidget(m_pUserTypeLabel);
    userInfoVerLayout->addStretch();
    QHBoxLayout *userHorLayout = new QHBoxLayout;
    userHorLayout->setSpacing(16);
    userHorLayout->setContentsMargins(0, 0, 0, 0);
    userHorLayout->addWidget(m_pUserLogoBtn);
    userHorLayout->addLayout(userInfoVerLayout);
    userHorLayout->addStretch();

    KLabel *slecteLabel = new KLabel;
    slecteLabel->setWordWrap(true);
    slecteLabel->setText(tr("Select account type (Ensure have admin on system):"));

    m_pAdminRadioBtn = new QRadioButton;
    m_pAdminRadioBtn->setFixedWidth(16);
    m_pStandardRadioBtn = new QRadioButton;
    m_pStandardRadioBtn->setFixedWidth(16);
    m_pTypesBtnGroup = new QButtonGroup;
    m_pTypesBtnGroup->addButton(m_pAdminRadioBtn, 1);
    m_pTypesBtnGroup->addButton(m_pStandardRadioBtn, 0);
    slecteLabel->setAccessibleName(tr("Select account type (Ensure have admin on system):"));
    slecteLabel->setBuddy(m_pAdminRadioBtn);
    m_pAdminRadioBtn->setAccessibleName(tr("administrator"));
    m_pStandardRadioBtn->setAccessibleName(tr("standard user"));

    KLabel *adminUserLabel = new KLabel();
    adminUserLabel->setText(tr("administrator"));
    KLabel *adminNoteLabel = new KLabel;
    adminNoteLabel->setAlignment(Qt::AlignTop);
    adminNoteLabel->setWordWrap(true);
    adminNoteLabel->setFontColor(Qt::gray);
    adminNoteLabel->setText(tr("The administrator account can modify any system configuration, including the installation and upgrading of software."));
    QVBoxLayout *adminVerLayout = new QVBoxLayout();
    adminVerLayout->setSpacing(0);
    adminVerLayout->setContentsMargins(0, 0, 0, 0);
    adminVerLayout->addWidget(adminUserLabel);
    adminVerLayout->addWidget(adminNoteLabel);
    QHBoxLayout *adminHorLayout = new QHBoxLayout();
    adminHorLayout->addWidget(m_pAdminRadioBtn);
    adminHorLayout->addLayout(adminVerLayout);
    m_pAdminFrame = new SettingFrame(this);
    m_pAdminFrame->setLayout(adminHorLayout);
    m_pAdminFrame->installEventFilter(this);

    KLabel *standardUserLabel = new KLabel();
    standardUserLabel->setText(tr("standard user"));
    KLabel *standardNoteLabel = new KLabel;
    standardNoteLabel->setAlignment(Qt::AlignTop);
    standardNoteLabel->setWordWrap(true);
    standardNoteLabel->setFontColor(Qt::gray);
    standardNoteLabel->setText(tr("Standard users can use the vast majority of software but are not allowed to modify system configurations."));
    QVBoxLayout *standardVerLayout = new QVBoxLayout();
    standardVerLayout->setSpacing(0);
    standardVerLayout->setContentsMargins(0, 0, 0, 0);
    standardVerLayout->addWidget(standardUserLabel);
    standardVerLayout->addWidget(standardNoteLabel);
    QHBoxLayout *standardHorLayout = new QHBoxLayout();
    standardHorLayout->addWidget(m_pStandardRadioBtn);
    standardHorLayout->addLayout(standardVerLayout);
    m_pStandardFrame = new SettingFrame(this);
    m_pStandardFrame->setLayout(standardHorLayout);
    m_pStandardFrame->installEventFilter(this);

    m_pConfirmBtn = new KPushButton;
    m_pConfirmBtn->setText(tr("Confirm"));
    m_pConfirmBtn->setEnabled(false);
    m_pConfirmBtn->setBackgroundColorHighlight(true);
    m_pCancelBtn = new KPushButton;
    m_pCancelBtn->setText(tr("Cancel"));
    QHBoxLayout *bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(m_pCancelBtn);
    bottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    QVBoxLayout *mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(24, 16, 24, 24);
    mainVerLayout->addLayout(userHorLayout);
    mainVerLayout->addSpacing(8);
    mainVerLayout->addWidget(slecteLabel);
    mainVerLayout->addWidget(m_pAdminFrame);
    mainVerLayout->addWidget(m_pStandardFrame);
    mainVerLayout->addSpacing(16);
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    mainVerLayout->setSizeConstraint(QLayout::SetMinimumSize);
    mainVerLayout->invalidate();
    mainVerLayout->activate();
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    mainWidget()->setLayout(mainVerLayout);

    this->adjustSize();
}

QSize ChangeUserType::sizeHint() const
{
    QSize size = QDialog::sizeHint();
    size.setWidth(DIALOG_WIDTH);  // 强制宽度
    return size;
}

void ChangeUserType::setConnect()
{
    connect(m_pCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(m_pConfirmBtn, &QPushButton::clicked, this, [=]{
        emit confirmChanged(m_pTypesBtnGroup->checkedId());

        accept();
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(m_pTypesBtnGroup), static_cast<void (QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled), [=](int id, bool status){
#else
    connect(m_pTypesBtnGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), [=](int id, bool status){
#endif
        if (id != oldid){
            if (status){
                m_pConfirmBtn->setEnabled(true);
            } else {
                m_pConfirmBtn->setEnabled(false);
            }
        }
    });
}

void ChangeUserType::requireUserInfo(QString logo, QString nname, int id, QString utype)
{
    //设置头像
    m_pUserLogoBtn->setPixmap(UkccCommon::makeRoundLogo(logo, m_pUserLogoBtn->width(), m_pUserLogoBtn->height(), m_pUserLogoBtn->width()/2, m_pUserLogoBtn->devicePixelRatioF()));

    //设置昵称
    m_pNickNameLabel->setText(nname);

    //
    m_pUserTypeLabel->setText(utype);

    //设置类型
    m_pTypesBtnGroup->blockSignals(true);
    if (id >= 0 && id < m_pTypesBtnGroup->buttons().length()){
        m_pTypesBtnGroup->button(id)->setChecked(true);
        //记录原始状态
        oldid = id;
    }
    m_pTypesBtnGroup->blockSignals(false);
}

bool ChangeUserType::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == m_pStandardFrame){
                m_pStandardRadioBtn->setChecked(true);
            } else if (watched == m_pAdminFrame){
                m_pAdminRadioBtn->setChecked(true);
            }
        }
    }

    return KDialog::eventFilter(watched, event);
}

void ChangeUserType::keyPressEvent(QKeyEvent * event)
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
