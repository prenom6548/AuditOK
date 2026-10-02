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
#include "utilsforuserinfo.h"
#include "changeuserlogo.h"

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMenu>

#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

UtilsForUserinfo::UtilsForUserinfo(bool isExisMfa, QObject *parent) : QObject(parent)
{
    m_isServerVersion = UkccCommon::isServerVersion();
    m_isExistMfa = isExisMfa;

    initUI();

    initConnections();
}

void UtilsForUserinfo::initUI()
{
    m_pMainItemFrame = new UkccFrame(nullptr, UkccFrame::Top, true);

    logoBtn = new KLabel();
    logoBtn->setFixedSize(QSize(50, 50));
    logoBtn->setObjectName("logo");
    logoBtn->setAccessibleName(ChangeUserLogo::tr("User Logo"));
    logoBtn->setAccessibleDescription(QString());

    nickNameLabel = new KLabel();
    nickNameLabel->setFixedHeight(26);

    typeLabel = new KLabel();
    typeLabel->setFixedHeight(20);

    changePwdBtn = new KPushButton();
    changePwdBtn->setText(tr("Password"));
    changePwdBtn->setObjectName("Password");
    changePwdBtn->setAccessibleName(tr("Password"));
    changeTypeBtn = new KPushButton();
    changeTypeBtn->setText(tr("Type"));
    changeTypeBtn->setObjectName("Type");
    changeTypeBtn->setAccessibleName(tr("Type"));
    delUserBtn = new KPushButton();
    delUserBtn->setText(tr("Delete"));
    delUserBtn->setObjectName("Delete");
    delUserBtn->setAccessibleName(tr("Delete"));

    m_pMoreBtn = new QToolButton();
    m_pMoreBtn->setAutoRaise(true);
    m_pMoreBtn->setProperty("useButtonPalette", true);
    m_pMoreBtn->setPopupMode(QToolButton::InstantPopup);
    m_pMoreBtn->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    QMenu *pMenu = new QMenu(m_pMoreBtn);
    m_pMoreBtn->setMenu(pMenu);
    m_pChangePwdAct = new QAction(tr("Password"),this);
    m_pChangeTypeAct = new QAction(tr("Type"),this);
    m_pDeleteAct = new QAction(tr("Delete"),this);
    m_pBindOtpAct = new QAction(tr("Bind OTP Dynamic Password"),this);
    m_pBindUkeyAct = new QAction(tr("Bind Security Password"),this);
    pMenu->addAction(m_pChangePwdAct);
    pMenu->addAction(m_pChangeTypeAct);
    pMenu->addAction(m_pBindOtpAct);
    pMenu->addAction(m_pBindUkeyAct);
    pMenu->addAction(m_pDeleteAct);
}

QToolButton *UtilsForUserinfo::getUtilsToolBtn()
{
    return m_pMoreBtn;
}

void UtilsForUserinfo::initConnections()
{
    connect(changePwdBtn, &KPushButton::clicked, this, [=]{
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), changePwdBtn->objectName(), QString("settings"));

        emit changePwdBtnPressed();
    });
    connect(changeTypeBtn, &KPushButton::clicked, this, [=]{
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), changeTypeBtn->objectName(), QString("settings"));

        emit changeTypeBtnPressed();
    });
    connect(delUserBtn, &KPushButton::clicked, this, [=]{
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), delUserBtn->objectName(), QString("settings"));

        emit deleteUserBtnPressed();
    });

    connect(m_pChangePwdAct, &QAction::triggered, this, [=](){
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), m_pChangePwdAct->objectName(), QString("settings"));

        emit changePwdBtnPressed();
    });

    connect(m_pChangeTypeAct, &QAction::triggered, this, [=](){
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), m_pChangeTypeAct->objectName(), QString("settings"));

        emit changeTypeBtnPressed();
    });

    connect(m_pDeleteAct, &QAction::triggered, this, [=](){
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), m_pDeleteAct->objectName(), QString("settings"));

        emit deleteUserBtnPressed();
    });

    connect(m_pBindOtpAct, &QAction::triggered, this, [=](){
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), m_pBindOtpAct->objectName(), QString("settings"));

        emit bindOtpActPressed();
    });
    connect(m_pBindUkeyAct, &QAction::triggered, this, [=](){
        UkccCommon::buriedSettings(QString("UtilsForUserinfo"), m_pBindUkeyAct->objectName(), QString("settings"));

        emit bindUkeyActPressed();
    });
}

UkccFrame * UtilsForUserinfo::buildItemForUsers()
{
    subUserInfoVerLayout = new QVBoxLayout();
    subUserInfoVerLayout->setSpacing(0);
    subUserInfoVerLayout->setContentsMargins(0, 0, 0, 0);
//    subUserInfoVerLayout->addStretch();
    subUserInfoVerLayout->addWidget(nickNameLabel, Qt::AlignHCenter);
    subUserInfoVerLayout->addWidget(typeLabel, Qt::AlignHCenter);
//    subUserInfoVerLayout->addStretch();

    mUserInfoHorLayout = new QHBoxLayout();
    mUserInfoHorLayout->setSpacing(8);
    mUserInfoHorLayout->setContentsMargins(16, 0, 16, 0);
    mUserInfoHorLayout->addWidget(logoBtn);
    mUserInfoHorLayout->addLayout(subUserInfoVerLayout);
    mUserInfoHorLayout->addStretch();
    mUserInfoHorLayout->addWidget(changePwdBtn);
    KVLineFrame *line1 = new KVLineFrame(m_pMainItemFrame);
    line1->setFixedHeight(10);
    mUserInfoHorLayout->addWidget(line1);
    mUserInfoHorLayout->addWidget(changeTypeBtn);
    KVLineFrame *line2 = new KVLineFrame(m_pMainItemFrame);
    line2->setFixedHeight(10);
    mUserInfoHorLayout->addWidget(line2);
    mUserInfoHorLayout->addWidget(delUserBtn);
    mUserInfoHorLayout->addWidget(m_pMoreBtn);

    if (m_isExistMfa) {
        if (m_isServerVersion) {
            if (!getuid()) {
                changePwdBtn->hide();
                line1->hide();
                changeTypeBtn->hide();
                line2->hide();
                delUserBtn->hide();
            } else {
                m_pMoreBtn->hide();
            }
        } else {
            changePwdBtn->hide();
            line1->hide();
            changeTypeBtn->hide();
            line2->hide();
            delUserBtn->hide();
        }
    } else {
        m_pMoreBtn->hide();
    }

    mainUserVerLayout = new QVBoxLayout();
    mainUserVerLayout->setSpacing(0);
    mainUserVerLayout->setContentsMargins(0, 0, 0, 0);
    mainUserVerLayout->addLayout(mUserInfoHorLayout);

    m_pMainItemFrame->setRadiusType(UkccFrame::Top);
    m_pMainItemFrame->setLayout(mainUserVerLayout);

    return m_pMainItemFrame;
}

void UtilsForUserinfo::refreshUserLogo(QString logo)
{
    QPixmap iconfile = makeRoundLogo(logo, logoBtn->width(), logoBtn->height(), logoBtn->width()/2);
    logoBtn->setPixmap(iconfile);
}

void UtilsForUserinfo::refreshUserNickname(QString name)
{
    nickNameLabel->setText(name);
}

void UtilsForUserinfo::refreshUserType(int type)
{
    QString t = _accountTypeIntToString(type);
    typeLabel->setText(t);
}

void UtilsForUserinfo::refreshDelStatus(bool enabled)
{
    if (m_isExistMfa) {
        if (m_isServerVersion) {
            if (!getuid()) {
                m_pDeleteAct->setEnabled(enabled);
            } else {
                delUserBtn->setEnabled(enabled);
            }
        } else {
            m_pDeleteAct->setEnabled(enabled);
        }
    } else {
        delUserBtn->setEnabled(enabled);
    }
}

void UtilsForUserinfo::refreshPwdStatus(bool enabled)
{
    if (m_isExistMfa) {
        if (m_isServerVersion) {
            if (!getuid()) {
                m_pChangePwdAct->setEnabled(enabled);
            } else {
                changePwdBtn->setEnabled(enabled);
            }
        } else {
            m_pChangePwdAct->setEnabled(enabled);
        }
    } else {
        changePwdBtn->setEnabled(enabled);
    }
}

void UtilsForUserinfo::refreshTypeStatus(bool enabled)
{
    if (m_isExistMfa) {
        if (m_isServerVersion) {
            if (!getuid()) {
                m_pChangeTypeAct->setEnabled(enabled);
            } else {
                changeTypeBtn->setEnabled(enabled);
            }
        } else {
            m_pChangeTypeAct->setEnabled(enabled);
        }
    } else {
        changeTypeBtn->setEnabled(enabled);
    }
}

void UtilsForUserinfo::setMoreBtnVisible(bool visible)
{
    // 不存kylin-mfa或者服务器版本非root用户无更多设置按钮，不可设置为可见
    if (!m_isExistMfa || (m_isServerVersion && getuid())) {
        return;
    }

    m_pMoreBtn->setVisible(visible);

}

void UtilsForUserinfo::setDelVisible(bool visible)
{
    if (m_isExistMfa) {
        if (m_isServerVersion) {
            if (!getuid()) {
                m_pDeleteAct->setVisible(visible);
            } else {
                delUserBtn->setVisible(visible);
            }
        } else {
            m_pDeleteAct->setVisible(visible);
        }
    } else {
        delUserBtn->setVisible(visible);
    }
}

void UtilsForUserinfo::setPwdVisible(bool visible)
{
    if (m_isExistMfa) {
        if (m_isServerVersion) {
            if (!getuid()) {
                m_pChangePwdAct->setVisible(visible);
            } else {
                changePwdBtn->setVisible(visible);
            }
        } else {
            m_pChangePwdAct->setVisible(visible);
        }
    } else {
        changePwdBtn->setVisible(visible);
    }
}

void UtilsForUserinfo::setTypeVisible(bool visible)
{
    if (m_isExistMfa) {
        if (m_isServerVersion) {
            if (!getuid()) {
                m_pChangeTypeAct->setVisible(visible);
            } else {
                changeTypeBtn->setVisible(visible);
            }
        } else {
            m_pChangeTypeAct->setVisible(visible);
        }
    } else {
        changeTypeBtn->setVisible(visible);
    }
}

void UtilsForUserinfo::setObjectPathData(QString op)
{
    m_pMainItemFrame->setObjectName(op);
}

void UtilsForUserinfo::userPropertyChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList)
{
    Q_UNUSED(property);
    Q_UNUSED(propertyList);

    if (propertyMap.keys().contains("IconFile")){
        QString current = propertyMap.value("IconFile").toString();

        refreshUserLogo(current);
    }

    if (propertyMap.keys().contains("AccountType")){
        int current = propertyMap.value("AccountType").toInt();

        refreshUserType(current);
    }

    if (propertyMap.keys().contains("RealName")){
        QString current = propertyMap.value("RealName").toString();
        if (QString::compare(current, nickNameLabel->text()) != 0){
            //更新用户昵称
            refreshUserNickname(current);
        }
    }
}

QString UtilsForUserinfo::_accountTypeIntToString(int type)
{
    QString atype;
    if (type == 0)
        atype = tr("Standard");
    else if (type == 1)
        atype = tr("Admin");

    return atype;
}

QPixmap UtilsForUserinfo::makeRoundLogo(QString logo, int wsize, int hsize, int radius)
{
    QPixmap rectPixmap;
    QPixmap iconcop = QPixmap(logo);

    if (iconcop.width() > iconcop.height()) {
        QPixmap iconPixmap = iconcop.copy((iconcop.width() - iconcop.height())/2, 0, iconcop.height(), iconcop.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(hsize);
    } else {
        QPixmap iconPixmap = iconcop.copy(0, (iconcop.height() - iconcop.width())/2, iconcop.width(), iconcop.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(wsize);
    }

    if (rectPixmap.isNull()) {
        return QPixmap();
    }
    QPixmap pixmapa(rectPixmap);
    QPixmap pixmap(radius*2,radius*2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius*2, radius*2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius*2, radius*2, pixmapa);
    return pixmap;
}
