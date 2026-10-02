/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QRegExp>
#include <QRegExpValidator>
#include <QFileDialog>
#include <QDesktopServices>
#include <QHostInfo>
#include <QNetworkInterface>
#include <windowmanage.hpp>

#include "localinfo.h"
#include "localupdatename.h"
#include "global/utils/addrset.h"
#include "global/utils/globalutils.h"
#include "global/utils/global_data.h"
#include "view/friendlist/friendlist.h"
#include "view/kyview.h"
#include "controller/control.h"
#include "ukuiwindowhelper.h"


LocalUpdateName::LocalUpdateName(int nameType, bool isAuto, QWidget *parent) : QWidget(parent)
{
    // 初始化数据
    setInitData(nameType, isAuto);

    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();
}

LocalUpdateName::LocalUpdateName(int nameType, int friendId, QWidget *parent) : QWidget(parent)
{
    // 初始化数据
    setInitData(nameType, friendId);

    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();
}

LocalUpdateName::~LocalUpdateName() {}

// 初始化数据
void LocalUpdateName::setInitData(int nameType, bool isAuto)
{
    m_nameType = nameType;
    m_isAuto = isAuto;
}

// 初始化数据
void LocalUpdateName::setInitData(int nameType, int friendId)
{
    m_nameType = nameType;
    m_friendId = friendId;
}

void LocalUpdateName::setWidgetUi()
{
    this->setWindowFlag(Qt::Tool);

    // if (m_nameType == LocalUpdateName::LocalName) {
    this->setWindowModality(Qt::WindowModal);
    this->setWindowModality(Qt::ApplicationModal);
    // }

    /* 适配kysdk的窗管 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    windowhelper->removeTitleBar();

    m_nameTypeLab = new QLabel(this);    //目录名称
    m_nameEdit = new QLineEdit(this);    //昵称
    m_tipsLab = new QLabel(this);        //提示
    m_cancelBtn = new QPushButton(this); //取消按钮
    m_sureBtn = new QPushButton(this);   //确定按钮

    m_titleBar = new TitleBar(false, false, false);

    //设置objectname
    m_nameTypeLab->setAccessibleName("kylin-ipmsg_localupdatename_QLabel_mnameTypeLab");
    m_nameEdit->setAccessibleName("kylin-ipmsg_localupdatename_QLineEdit_m_nameEdit");
    m_tipsLab->setAccessibleName("kylin-ipmsg_localupdatename_QLabel_mtipsLab");
    m_cancelBtn->setAccessibleName("kylin-ipmsg_localupdatename_QPushButton_mcancelBtn");
    m_sureBtn->setAccessibleName("kylin-ipmsg_localupdatename_QPushButton_msureBtn");
    m_titleBar->setAccessibleName("kylin-ipmsg_localupdatename_TitleBar_mtitleBar");

    // m_nameTypeLab->setFixedHeight(35);
    // m_nameEdit->setFixedSize(312, 40);
    m_nameEdit->setFixedWidth(312);
    m_nameEdit->setMinimumHeight(36);
    m_cancelBtn->setFixedHeight(36);
    m_sureBtn->setFixedHeight(36);
    m_sureBtn->setEnabled(false);

    m_mainLayout = new QVBoxLayout(this); // 设置页面标题布局
    m_vControlLayout = new QVBoxLayout(); // 总体控件纵向布局
    m_hControlLayout = new QHBoxLayout(); // 总体控件横向布局
    m_hButtonLayout = new QHBoxLayout();  // 按钮的横向布局
    //设置objectname
    m_mainLayout->setObjectName("kylin-ipmsg_localupdatename_QVBoxLayout_mmainLayout");
    m_vControlLayout->setObjectName("kylin-ipmsg_localupdatename_QVBoxLayout_mvControlLayout");
    m_hControlLayout->setObjectName("kylin-ipmsg_localupdatename_QHBoxLayout_mhControlLayout");
    m_hButtonLayout->setObjectName("kylin-ipmsg_localupdatename_QHBoxLayout_mhButtonLayout");

    m_hButtonLayout->setSpacing(0);
    m_hButtonLayout->addStretch();
    m_hButtonLayout->addWidget(m_cancelBtn);
    m_hButtonLayout->addSpacing(16);
    m_hButtonLayout->addWidget(m_sureBtn);
    m_hButtonLayout->setMargin(0);

    // 对组件进行布局
    m_vControlLayout->setSpacing(0);
    m_vControlLayout->addWidget(m_nameTypeLab, Qt::AlignBottom);
    m_vControlLayout->addSpacing(12);
    m_vControlLayout->addWidget(m_nameEdit);
    m_vControlLayout->addSpacing(5);
    m_vControlLayout->addWidget(m_tipsLab);
    m_vControlLayout->addSpacing(5);
    m_vControlLayout->addLayout(m_hButtonLayout);
    m_vControlLayout->addSpacing(12);
    m_vControlLayout->setSpacing(0);
    m_vControlLayout->setMargin(0);

    m_hControlLayout->addStretch();
    m_hControlLayout->addLayout(m_vControlLayout);
    m_hControlLayout->addStretch();
    //    m_hLayout->setMargin(0);
    //    m_hLayout->setSpacing(0);

    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_titleBar);
    m_mainLayout->addSpacing(12);
    m_mainLayout->addLayout(m_hControlLayout);
    m_mainLayout->addStretch();
    m_mainLayout->setMargin(0);

    m_settings = IniSettings::getInstance();

    connect(m_nameEdit, &QLineEdit::returnPressed, this, &LocalUpdateName::slotConfirmBtn);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &LocalUpdateName::slotEnableSureBtn);
    connect(m_sureBtn, &QPushButton::clicked, this, &LocalUpdateName::slotConfirmBtn);
    connect(m_cancelBtn, &QPushButton::clicked, this, &LocalUpdateName::slotCancelBtn);
    connect(m_titleBar->m_pCloseButton, &QPushButton::clicked, this, &LocalUpdateName::slotCancelBtn);
}

void LocalUpdateName::setWidgetStyle()
{
    // 设置背景色
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    this->setFixedSize(QSize(376, 225));

    m_titleBar->m_pMinimizeButton->hide();

    QFont fontLabel = GlobalData::getInstance()->getFontSize16px();
    m_nameTypeLab->setFont(fontLabel);

    m_tipsLab->setMargin(0);
    m_tipsLab->setStyleSheet("color:#f44e50;font:14px;");

    QFont font = GlobalData::getInstance()->getFontSize14px();
    m_nameEdit->setFont(font);
    m_nameEdit->setTextMargins(8, 6, 8, 6);
    m_nameEdit->setMaxLength(20);

    if (m_nameType == LocalUpdateName::LocalName) {
        m_nameTypeLab->setText(tr("Set Username"));
        m_nameEdit->setPlaceholderText(tr("Please enter username"));

    } else if (m_nameType == LocalUpdateName::FriendName) {
        m_nameTypeLab->setText(tr("Change nickname"));
        m_nameEdit->setPlaceholderText(tr("Please enter friend nickname"));
        QStandardItem *item = FriendListModel::getInstance()->getFriendById(m_friendId);
        if(item){
            QString nickname = item->data(FriendListModel::Nickname).toString();
            m_nameEdit->setText(nickname);
        }
    }

    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFont(font);
    // m_cancelBtn->setProperty("isWindowButton", 0x1);
    // m_cancelBtn->setProperty("useIconHighlightEffect", 0x2);

    m_sureBtn->setText(tr("Confirm"));
    m_sureBtn->setFont(font);
    m_sureBtn->setProperty("isImportant", true);
    // m_sureBtn->setProperty("isWindowButton", 0x1);
    // m_sureBtn->setProperty("useIconHighlightEffect", 0x2);

    if ((m_nameType == LocalName) && m_isAuto) {
        m_cancelBtn->setText(tr("Skip"));

        // 设置应用内居中
        QScreen *screen = QGuiApplication::primaryScreen();
        this->move(screen->geometry().center() - this->rect().center());

    } else {
        this->move(KyView::getInstance()->geometry().center() - this->rect().center());
    }

    /* 监听字体变化 */
    connect(Control::getInstance(), &Control::sigFontChange, this, [=]() {
        QFont font14 = GlobalData::getInstance()->getFontSize14px();
        QFont font16 = GlobalData::getInstance()->getFontSize16px();

        this->m_titleBar->m_pFuncLabel->setFont(font14);
        this->m_nameTypeLab->setFont(font16);
        this->m_nameEdit->setFont(font14);
        this->m_cancelBtn->setFont(font14);
        this->m_sureBtn->setFont(font14);
    });

    m_nameEdit->setFocus();
    this->update();
}

// 确认按钮槽函数
void LocalUpdateName::slotConfirmBtn()
{
//    QRegExp rx("^[a-zA-Z0-9\u4e00-\u9fa5]+$"); //判断是否有特殊字符
//    if (m_nameEdit->text() == "") {
//        if (m_nameType == LocalUpdateName::LocalName) {
//            m_tipsLab->setText(tr("Please enter username"));
//        } else if (m_nameType == LocalUpdateName::FriendName) {
//            m_tipsLab->setText(tr("Please enter friend nickname"));
//        }
//    QRegularExpression re("^[a-zA-Z0-9\\p{Han}\\x{1800}-\\x{18AF}\\x{0400}-\\x{04FF}]+$");
    QRegularExpression re(R"([~!@#$%^&*()_+\-={}|:\"<>?\[\]\\;',./，。、；‘’【】{}：“”《》？！￥……（）·∑∏〓＂＃﹪﹫\s]+)");
    QRegularExpressionMatch match = re.match(m_nameEdit->text());
    if (m_nameEdit->text() == "" && m_nameType == LocalUpdateName::LocalName) {
        m_tipsLab->setText(tr("Please enter username"));
    } else if (m_nameEdit->text().count() > 20) {
        m_tipsLab->setText(tr("The length of user name is less than 20 words"));
    }
    else if ((match.hasMatch() && m_nameEdit->text() != "" && m_nameType == LocalUpdateName::FriendName)||(match.hasMatch() && m_nameType == LocalUpdateName::LocalName)) {
        m_tipsLab->setText(tr("Please do not enter special characters"));
    } else {
        m_tipsLab->setText("");
        QString localnickName = m_nameEdit->text();

        if (m_nameEdit->text() == "") {
            QStandardItem *item = FriendListModel::getInstance()->getFriendById(m_friendId);
            QString usernameStr = item->data(FriendListModel::Username).toString();
            QString localnickName = usernameStr;
        }

        if (m_nameType == LocalUpdateName::LocalName) {
            m_settings->setLocalNickname(localnickName);

            QString ip = AddrSet::getInstance()->getHostIpAddress();
            LocalInfo::getInstance()->displayInfo(localnickName, ip);

        } else if (m_nameType == LocalUpdateName::FriendName) {
            FriendInfoData *friendInfo = new FriendInfoData();
            friendInfo->setObjectName("kylin-ipmsg_localupdatename_FriendInfoData_friendInfo");
            friendInfo->m_friendId = m_friendId;
            friendInfo->m_nickname = localnickName;

            FriendListModel::getInstance()->updateNickname(friendInfo);
        }

        if (m_settings->getLocalUuid().isEmpty()) {
            m_settings->setLocalUuid(GlobalUtils::getUserUuid());
            m_settings->setFilePath(GlobalUtils::getDefaultPath());
        }

        if (GlobalData::getInstance()->m_uuid.isEmpty()) {
            GlobalData::getInstance()->m_uuid = IniSettings::getInstance()->getLocalUuid();
        }
        this->close();
        if(parentWidget())
            parentWidget()->activateWindow();
    }
}

// 取消按钮槽函数
void LocalUpdateName::slotCancelBtn()
{
    if (IniSettings::getInstance()->getLocalNickname().isEmpty()) {

        QString defaultNickname = GlobalUtils::getUsername() + m_settings->getLocalUuid().right(6);
        QString ip = AddrSet::getInstance()->getHostIpAddress();

        m_settings->setLocalNickname(defaultNickname);
        LocalInfo::getInstance()->displayInfo(defaultNickname, ip);
    }

    if (m_settings->getLocalUuid().isEmpty()) {
        m_settings->setLocalUuid(GlobalUtils::getUserUuid());
        m_settings->setFilePath(GlobalUtils::getDefaultPath());
    }

    if (GlobalData::getInstance()->m_uuid.isEmpty()) {
        GlobalData::getInstance()->m_uuid = IniSettings::getInstance()->getLocalUuid();
    }

    this->close();
    this->deleteLater();
}

void LocalUpdateName::slotEnableSureBtn(QString text)
{
    m_sureBtn->setEnabled(!text.isEmpty());
}

//弹窗显示后按Esc关闭弹窗
void LocalUpdateName::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Escape){
        this->window()->close();
    }
}
