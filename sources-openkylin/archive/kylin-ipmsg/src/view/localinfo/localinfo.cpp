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
#include <KWindowEffects>
#include <QPainterPath>
#include "windowmanage.hpp"

#include "global/utils/global_data.h"
#include "view/common/globalsizedata.h"
#include "view/kyview.h"
#include "localinfo.h"
#include "ukuiwindowhelper.h"

LocalInfo::LocalInfo(QWidget *parent) : QWidget(parent)
{
    // 初始化组件
    setWidgetUi();

    m_windowHelper = new UkuiWindowHelper(this);


    this->setAttribute(Qt::WA_TranslucentBackground, true);
    // 设置组件样式
    // setWidgetStyle(); //300ms

    //从ini文件中获取到信息/写入信息
    // getIniText();
}

LocalInfo::~LocalInfo() {}

LocalInfo *LocalInfo::getInstance(QWidget *parent)
{
//    Q_UNUSED(parent);

    static LocalInfo *instanse = nullptr;

    if (nullptr == instanse) {
        instanse = new LocalInfo(parent);
        instanse->setAccessibleName("kylin-ipmsg_LocalInfo_instance");
    }

    return instanse;
}

// 初始化组件
void LocalInfo::setWidgetUi()
{
    this->setFixedHeight(GlobalSizeData::LOCALINFO_HEIGHT);

    // 初始化组件及布局
    m_userAvatarLab = new QLabel();
    m_nicknameLab = new QLabel();
    m_userIpAddrLab = new QLabel();
    m_updateNameBtn = new QPushButton();
    m_openFilePathBtn = new QPushButton();
    m_searchIconLab = new QLabel();
    m_searchLineEdit = new KSearchLineEdit();
    m_localInfoLayout = new QVBoxLayout();
    m_nameLayout = new QHBoxLayout();
    m_ipLayout = new QVBoxLayout();
    m_avatarLayout = new QHBoxLayout();
    m_searchLayout = new QHBoxLayout();
    //设置objectname
    m_userAvatarLab->setAccessibleName("kylin-ipmsg_localinfo_QLabel_muserAvatarLab");
    m_nicknameLab->setAccessibleName("kylin-ipmsg_localinfo_QLabel_mnicknameLab");
    m_userIpAddrLab->setAccessibleName("kylin-ipmsg_localinfo_QLabel_muserIpAddrLab");
    m_updateNameBtn->setAccessibleName("kylin-ipmsg_localinfo_QPushButton_mupdateNameBtn");
    m_openFilePathBtn->setAccessibleName("kylin-ipmsg_localinfo_QPushButton_mopenFilePathBtn");
    m_searchIconLab->setAccessibleName("kylin-ipmsg_localinfo_QLabel_msearchIconLab");
    m_searchLineEdit->setAccessibleName("kylin-ipmsg_localinfo_KSearchLineEdit_msearchLineEdit");
    m_localInfoLayout->setObjectName("kylin-ipmsg_localinfo_QVBoxLayout_mlocalInfoLayout");
    m_nameLayout->setObjectName("kylin-ipmsg_localinfo_QHBoxLayout_mnameLayout");
    m_ipLayout->setObjectName("kylin-ipmsg_localinfo_QVBoxLayout_mipLayout");
    m_avatarLayout->setObjectName("kylin-ipmsg_localinfo_QHBoxLayout_mavatarLayout");
    m_searchLayout->setObjectName("kylin-ipmsg_localinfo_QHBoxLayout_msearchLayout");
    // 对组件进行布局
    m_nameLayout->addWidget(m_nicknameLab, Qt::AlignBottom);
    m_nameLayout->addSpacing(4);
    m_nameLayout->addWidget(m_updateNameBtn, Qt::AlignBottom | Qt::AlignLeft);
    m_nameLayout->addStretch();
    m_nameLayout->addWidget(m_openFilePathBtn, Qt::AlignBottom | Qt::AlignRight);
    m_nameLayout->setMargin(0);
    m_nameLayout->setSpacing(0);

    m_ipLayout->addLayout(m_nameLayout);
    m_ipLayout->addWidget(m_userIpAddrLab, Qt::AlignTop);
    m_ipLayout->setMargin(0);
    m_ipLayout->setSpacing(0);

    m_avatarLayout->addSpacing(5);
    m_avatarLayout->addWidget(m_userAvatarLab);
    m_avatarLayout->addLayout(m_ipLayout);
    m_avatarLayout->addSpacing(5);
    //    m_avatarLayout->setMargin(0);
    //    m_avatarLayout->setSpacing(0);
    
    m_localInfoLayout->addSpacing(-2);
    m_searchLayout->addWidget(m_searchLineEdit);

    m_localInfoLayout->addStretch();
    m_localInfoLayout->addLayout(m_avatarLayout);
    m_localInfoLayout->addStretch();
    m_localInfoLayout->addLayout(m_searchLayout);
    m_localInfoLayout->setSpacing(8);
    this->setLayout(m_localInfoLayout);

    this->installEventFilter(this); //安装过滤器，进行函数过滤
    m_updateNameBtn->installEventFilter(this);
    m_openFilePathBtn->installEventFilter(this);
    m_searchLineEdit->installEventFilter(this);

    // 设置对象名
    m_userAvatarLab->setObjectName("usericon");
    m_userIpAddrLab->setObjectName("userip");
    m_nicknameLab->setObjectName("username");
    m_updateNameBtn->setObjectName("updatename");
    m_openFilePathBtn->setObjectName("uploadfile");

    m_settings = IniSettings::getInstance(); //70ms

    switch (GlobalData::getInstance()->m_currentMode) {
        case CurrentMode::PCMode:
        m_nameWidth = 150;
            break;
        case CurrentMode::HMode:
        m_nameWidth = 550 * 0.5;
            break;
        case CurrentMode::VMode:
        m_nameWidth = 1080 * 0.5;
            break;
    }

    // 槽函数绑定
    connect(m_updateNameBtn, &QPushButton::clicked, this, &LocalInfo::slotUpdateName);
    connect(m_openFilePathBtn, &QPushButton::clicked, this, &LocalInfo::slotOpenFilePath);

    connect(m_searchLineEdit, &QLineEdit::textChanged, FriendListView::getInstance(),
            &FriendListView::filterFriendByReg);
    connect(m_searchLineEdit, &QLineEdit::textChanged, SearchPage::getInstance()->m_msgView,
            &SearchMsgList::filterFriendByReg);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFont, this, &LocalInfo::slotChangeFont);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeHightColor, this, &LocalInfo::setBackgroundColor);
}


// 设置组件样式
void LocalInfo::setWidgetStyle()
{
    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    // 设置空间大小
    m_userAvatarLab->setFixedSize(60, 60);
    m_updateNameBtn->setFixedSize(20, 20);
    m_openFilePathBtn->setFixedSize(20, 20);
    m_searchLineEdit->setMinimumSize(334, 36);
    m_searchLineEdit->setMaximumHeight(40);
    m_searchLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 设置图片
    QFont fontName = GlobalData::getInstance()->getFontSize16px();
    m_nicknameLab->setFont(fontName);
    m_nicknameLab->setAlignment(Qt::AlignBottom); //文字向下对齐
    m_nicknameLab->setMargin(0);
    QFontMetrics fontmts = QFontMetrics(m_nicknameLab->font());
    m_nicknameLab->setFixedWidth(fontmts.width(m_settings->getLocalNickname()));
    m_nicknameLab->setScaledContents(true);
    //    m_nicknameLab->setFixedSize(GlobalSizeData::USER_NAME_LAB_SIZE);

    m_userIpAddrLab->setFixedSize(GlobalSizeData::USER_IP_LAB_SIZE);

    m_userAvatarLab->setAlignment(Qt::AlignCenter);
    m_userAvatarLab->setFixedSize(GlobalSizeData::AVATAR_LAB_SIZE);
    setBackgroundColor();

    m_updateNameBtn->setIcon(QIcon::fromTheme("document-edit-symbolic")); /*修改名字*/
    m_updateNameBtn->setIconSize(GlobalSizeData::OPEN_FOLDER_BTN_ICON);
    m_updateNameBtn->setFixedSize(GlobalSizeData::OPEN_FOLDER_BTN_SIZE);
    m_updateNameBtn->setProperty("isWindowButton", 0x1);
    m_updateNameBtn->setProperty("useIconHighlightEffect", 0x2);
    m_updateNameBtn->setFlat(true);
    m_updateNameBtn->setFocusPolicy(Qt::NoFocus);
    m_updateNameBtn->setToolTip(tr("Modify Name"));

    m_openFilePathBtn->setIcon(QIcon::fromTheme("document-open-symbolic")); /*传输文件*/
    m_openFilePathBtn->setIconSize(GlobalSizeData::OPEN_FOLDER_BTN_ICON);
    m_openFilePathBtn->setFixedSize(GlobalSizeData::OPEN_FOLDER_BTN_SIZE);
    m_openFilePathBtn->setProperty("isWindowButton", 0x1);
    m_openFilePathBtn->setProperty("useIconHighlightEffect", 0x2);
    m_openFilePathBtn->setFlat(true);
    m_openFilePathBtn->setFocusPolicy(Qt::NoFocus);
    m_openFilePathBtn->setToolTip(tr("Open Directory"));

    QFont font = GlobalData::getInstance()->getFontSize14px();
    m_searchLineEdit->setFont(font);
    m_searchLineEdit->setCompleter(nullptr);
    m_searchLineEdit->setClearButtonEnabled(true);
    m_searchLineEdit->setTranslucent(true);
    this->setFocusPolicy(Qt::ClickFocus);

    changeTheme();

    this->update();
}

void LocalInfo::showList()
{
    // 设置组件样式
    setWidgetStyle(); //300ms

    //从ini文件中获取到信息/写入信息
    getIniText();
}

// 跟随主题深浅模式
void LocalInfo::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUIDark) {
        m_backColor = QColor("#000000");
    } else {
        m_backColor = QColor("#FFFFFF");
    }
    m_backColor.setAlpha(255);
    this->update();
}

void LocalInfo::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    QPainterPath rectPath;
    rectPath.addRoundedRect(rect(), 0, 0);
    /* 开启背景模糊效果（毛玻璃） */
    // KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(rectPath.toFillPolygon().toPolygon()));

    QStyleOption opt;
    opt.init(this);
    p.setPen(Qt::NoPen);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Base,QColor(m_backColor));
    this->setPalette(pal);
    QBrush brush = QBrush(m_backColor);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    // p.fillPath(rectPath, brush);
}

bool LocalInfo::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) {
        GlobalData::getInstance()->getSystemTransparency();
        m_backColor.setAlpha(GlobalSizeData::BLUR_TRANSPARENCY);
        m_windowHelper->setBlurEffect(QRegion(), 0, true);

    }
    else if (event->type() == QEvent::WindowDeactivate){
        m_backColor.setAlpha(255);
        m_windowHelper->setBlurEffect(QRegion(), 0, false);
    }

    return QWidget::event(event);
}

bool LocalInfo::eventFilter(QObject *watch, QEvent *e)
{
    return QObject::eventFilter(watch, e);
}

// 修改备注
void LocalInfo::slotUpdateName()
{
    /* sdk功能点：昵称修改 */
    GlobalSizeData::SDKPointNicknameModify();
    
    m_updateNameWid = new LocalUpdateName(LocalUpdateName::LocalName, false, this);
    m_updateNameWid->setAccessibleName("kylin-ipmsg_localinfo_LocalUpdateName_mupdateNameWid");
    m_updateNameWid->show();
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(m_updateNameWid->windowHandle());
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
    ::kabase::WindowManage::setMiddleOfScreen(m_updateNameWid);
    }
}

// 打开文件保存目录
void LocalInfo::slotOpenFilePath()
{
    /* sdk功能点：打开文件保存目录(主界面入口) */
    GlobalSizeData::SDKPointOpenSaveDir();

    QString filePath = m_settings->getFilePath();
    QDir dir(filePath);
    if(!dir.exists()){
        if(!dir.mkpath(filePath)){
            filePath = GlobalUtils::getDefaultPath();
            m_settings->setFilePath(filePath);
            dir.setPath(filePath);
            dir.mkpath(filePath);
        }
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

// 显示本机昵称和IP
void LocalInfo::displayInfo(QString nickname, QString ip)
{
    if (nickname != "") {
        m_userAvatarLab->setText(GlobalUtils::getAvatarByName(nickname));
        // 本机昵称 过长时进行省略显示
        QFontMetrics fontmts = QFontMetrics(m_nicknameLab->font());
        //        int dif = fontmts.width(nickname) - m_nicknameLab->width();
        int dif = fontmts.width(nickname) - m_nameWidth;
        if (dif > 0) {
            m_nicknameLab->setFixedWidth(m_nameWidth);
            m_nicknameLab->setToolTip(nickname);
            nickname = fontmts.elidedText(nickname, Qt::ElideRight, m_nameWidth);
            //            nickname = fontmts.elidedText(nickname, Qt::ElideRight, m_nicknameLab->width());
        }
        else{
            m_nicknameLab->setToolTip("");
        }
        m_nicknameLab->setFixedWidth(fontmts.width(nickname));
        m_nicknameLab->setText(nickname);
    }

    m_userIpAddrLab->setText(ip);

    emit changeLocalName(true);
}

//从ini文件中获取到信息/写入信息
void LocalInfo::getIniText()
{
    AddrSet *m_AddrSet = AddrSet::getInstance(); //获取IP、Mac、主机名称信息
    QString groupLocalHost = m_AddrSet->getLocalHost();
    QString Localip = m_AddrSet->getHostIpAddress();
    QString Localmac = m_AddrSet->getHostMacAddress();

    m_settings->setLocalTcpPort(GlobalUtils::getAvailTcpPort());

    QString m_IPAddress = Localip;
    QString nickname = m_settings->getLocalNickname();
    displayInfo(nickname, m_IPAddress);
}

// 初始化界面后检查本机昵称
void LocalInfo::checkLocalName()
{
    if (m_settings->getLocalNickname().isEmpty()) {
        QTimer::singleShot(100, this, [=](){
            m_updateNameWid = new LocalUpdateName(LocalUpdateName::LocalName, true, this);
            m_updateNameWid->setAccessibleName("kylin-ipmsg_localinfo_LocalUpdateName_mupdateNameWid");
            m_updateNameWid->show();
            UkuiWindowHelper *windowhelper = new UkuiWindowHelper(m_updateNameWid->windowHandle());
            windowhelper->setSkipTaskBar(true);
            windowhelper->setSkipSwitcher(true);
            if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
            ::kabase::WindowManage::setMiddleOfScreen(m_updateNameWid);
            }
        });
    }
}

// 获取本机头像
QString LocalInfo::getLocalAvatar()
{
    return m_userAvatarLab->text();
}

void LocalInfo::changeFontSize()
{
    QFont font14 = GlobalData::getInstance()->getFontSize14px();
    QFont font16 = GlobalData::getInstance()->getFontSize16px();

    m_nicknameLab->setFont(font16);
    QString nickname = IniSettings::getInstance()->getLocalNickname();
    QFontMetrics fontmts = QFontMetrics(m_nicknameLab->font());
    int dif = fontmts.width(nickname) - m_nameWidth;
    if (dif > 0) {
        m_nicknameLab->setToolTip(nickname);
        nickname = fontmts.elidedText(nickname, Qt::ElideRight, m_nameWidth);
    }
    m_nicknameLab->setFixedWidth(fontmts.width(nickname));
    m_nicknameLab->setText(nickname);

    m_userIpAddrLab->setFont(font14);
    m_searchLineEdit->setFont(font14);
}

void LocalInfo::slotChangeFont()
{
    m_userAvatarLab->setFont(GlobalData::getInstance()->m_fontName);
    changeFontSize();
    return;
}

void LocalInfo::setBackgroundColor()
{
    QColor highLightColor = QApplication::palette().highlight().color();
    QString highLightStr = QString("#%1%2%3").arg(highLightColor.red(), 2, 16,QChar('0'))
                                             .arg(highLightColor.green(), 2, 16,QChar('0'))
                                             .arg(highLightColor.blue(), 2, 16,QChar('0'));
    QString str = QString("background-color:%1;border-radius:30px;color:white;font-size:28px;").arg(highLightStr);
    m_userAvatarLab->setStyleSheet(str);
}

void LocalInfo::slotChangePCSize()
{
    // 设置空间大小
    this->setFixedHeight(GlobalSizeData::LOCALINFO_HEIGHT);
    m_nameWidth = 150;
    getIniText();

    return;
}

void LocalInfo::slotChangeFlatHSize()
{
    // 设置空间大小
    this->setFixedHeight(GlobalSizeData::LOCALINFO_MAX_HEIGHT);
    m_nameWidth = 551 * 0.5;
    getIniText();

    return;
}

void LocalInfo::slotChangeFlatVSize()
{
    // 设置空间大小
    this->setFixedHeight(GlobalSizeData::LOCALINFO_MAX_HEIGHT);
    m_nameWidth = 1080 * 0.5;
    getIniText();

    return;
}

