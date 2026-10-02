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
#include <KWindowEffects>
#include <windowmanage.hpp>
#include <gsettingmonitor.h>
#include <QPainterPath>

#include "friendinfowid.h"
#include "model/friendlistmodel.h"
#include "view/common/globalsizedata.h"
#include "global/utils/global_data.h"
#undef Unsorted
#include "view/kyview.h"
#include "ukuiwindowhelper.h"

FriendInfoWid::FriendInfoWid(QWidget *parent) : QWidget(parent)
{
    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();
}

FriendInfoWid::~FriendInfoWid() {}

void FriendInfoWid::changeFontSize()
{
    QFont font14 = GlobalData::getInstance()->getFontSize14px();

    m_usernameLab->setFont(font14);
    m_ipAddrLab->setFont(font14);
    m_nicknameLab->setFont(font14);
    m_usernameInfo->setFont(font14);
    m_ipAddrInfo->setFont(font14);
    m_nicknameInfo->setFont(font14);
    m_titleLable->setFont(font14);
    if (m_friendId != 0) {
        fillFriendInfo(m_friendId);
    }
    return;
}

// 单例，初始化返回指针
FriendInfoWid *FriendInfoWid::getInstance(QWidget *parent)
{
    static FriendInfoWid *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new FriendInfoWid(parent);
            instance->setAccessibleName("kylin-ipmsg_friendinfowid_FriendInfoWid_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new FriendInfoWid(parent);
        instance->setAccessibleName("kylin-ipmsg_friendinfowid_FriendInfoWid_instance");
#endif
    }

    return instance;
}

// 初始化组件
void FriendInfoWid::setWidgetUi()
{
    this->setWindowFlag(Qt::Tool);
    this->setWindowModality(Qt::WindowModal);
    this->setWindowModality(Qt::ApplicationModal);
    
    // 读取gsettings
    initGsetting();

    // 标题栏
    m_titleIcon = new QPushButton(this);
    m_titleLable = new QLabel(this);
    m_titleClose = new QPushButton(this);

    // 好友信息
    m_avatarLab = new QLabel(this);
    m_usernameLab = new QLabel(this);
    m_usernameInfo = new QLabel(this);
    m_ipAddrLab = new QLabel(this);
    m_ipAddrInfo = new QLabel(this);
    m_nicknameLab = new QLabel(this);
    m_nicknameInfo = new QLabel(this);

    // 修改好友备注按钮
    m_changeNicknameBtn = new QPushButton(this);

    // 对界面组件进行布局
    m_mainLayout = new QVBoxLayout(this);
    m_infoLayout = new QGridLayout();
    m_avatarLayout = new QHBoxLayout();
    m_nicknameLayout = new QHBoxLayout();
    m_titleHLayout = new QHBoxLayout();
    //设置objectname
    m_titleIcon->setAccessibleName("kylin-ipmsg_friendinfowid_QPushButton_mtitleIcon");
    m_titleLable->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_mtitleLable");
    m_titleClose->setAccessibleName("kylin-ipmsg_friendinfowid_QPushButton_mtitleClose");
    m_avatarLab->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_mavatarLab");
    m_usernameLab->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_musernameLab");
    m_usernameInfo->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_musernameInfo");
    m_ipAddrLab->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_mipAddrLab");
    m_ipAddrInfo->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_mipAddrInfo");
    m_nicknameLab->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_mnicknameLab");
    m_nicknameInfo->setAccessibleName("kylin-ipmsg_friendinfowid_QLabel_mnicknameInfo");
    m_changeNicknameBtn->setAccessibleName("kylin-ipmsg_friendinfowid_QPushButton_mchangeNicknameBtn");
    m_mainLayout->setObjectName("kylin-ipmsg_friendinfowid_QVBoxLayout_mmainLayout");
    m_infoLayout->setObjectName("kylin-ipmsg_friendinfowid_QPushButton_minfoLayout");
    m_avatarLayout->setObjectName("kylin-ipmsg_friendinfowid_QHBoxLayout_mavatarLayout");
    m_nicknameLayout->setObjectName("kylin-ipmsg_friendinfowid_QHBoxLayout_mnicknameLayout");
    m_titleHLayout->setObjectName("kylin-ipmsg_friendinfowid_QHBoxLayout_mtitleHLayout");

    // 好友信息布局
    // m_infoLayout->addWidget(m_avatarLab,    0, 0, 3, 3);
    m_infoLayout->addWidget(m_usernameLab, 0, 1, 1, 1);
    m_infoLayout->addWidget(m_usernameInfo, 0, 2, 1, 2);
    m_infoLayout->addWidget(m_ipAddrLab, 1, 1, 1, 1);
    m_infoLayout->addWidget(m_ipAddrInfo, 1, 2, 1, 2);
    m_infoLayout->addWidget(m_nicknameLab, 2, 1, 1, 1);
    m_infoLayout->addLayout(m_nicknameLayout, 2, 2, 1, 1);
    m_infoLayout->setMargin(0);
    m_infoLayout->setSpacing(10);

    m_nicknameLayout->addWidget(m_nicknameInfo);
    m_nicknameLayout->addWidget(m_changeNicknameBtn);
    m_nicknameLayout->setAlignment(Qt::AlignLeft);
    m_nicknameLayout->setMargin(0);
    m_nicknameLayout->setSpacing(5);

    m_avatarLayout->setMargin(16);
    m_avatarLayout->setSpacing(35);
    m_avatarLayout->addStretch();
    m_avatarLayout->addWidget(m_avatarLab);
    m_avatarLayout->addLayout(m_infoLayout);
    m_avatarLayout->addStretch();

    m_titleHLayout->setContentsMargins(4, 4, 4, 4);
    m_titleHLayout->setSpacing(0);
    m_titleHLayout->addSpacing(2);
    m_titleHLayout->addWidget(m_titleIcon);
    m_titleHLayout->addSpacing(8);
    m_titleHLayout->addWidget(m_titleLable);
    m_titleHLayout->addStretch(); /*表示加了弹簧*/
    m_titleHLayout->addWidget(m_titleClose);

    // 界面总体布局
    m_mainLayout->addLayout(m_titleHLayout);
    m_mainLayout->addLayout(m_avatarLayout);
    m_mainLayout->addStretch();
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    connect(m_titleClose, &QPushButton::clicked, this, &FriendInfoWid::hide);
    connect(m_changeNicknameBtn, &QPushButton::clicked, this, &FriendInfoWid::changeNickname);
    connect(FriendListModel::getInstance(), &FriendListModel::updateFriendInfo, this,
            &FriendInfoWid::slotUpdateFriendInfo);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFont, this, &FriendInfoWid::slotChangeFont);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFontSize, this, &FriendInfoWid::changeFontSize);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeHightColor, this, &FriendInfoWid::setBackgroundColor);
}

// 设置组件样式
void FriendInfoWid::setWidgetStyle()
{
    this->setWindowTitle(tr("Messages"));

    //毛玻璃
    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    /* 适配kysdk的窗管 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    windowhelper->removeTitleBar();

    // 设置背景色
    // this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    this->setFixedSize(QSize(376, 180));


    // 设置标题栏文字
    // 设置应用图标样式
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_titleIcon->setStyleSheet(btnStyle);
    m_titleIcon->setIconSize(QSize(24, 24));               /*设置ico类型图片大小*/
    m_titleIcon->setIcon(QIcon::fromTheme("kylin-ipmsg")); /*使用fromTheme函数调用库中的图片*/

    QFont font = GlobalData::getInstance()->getFontSize14px();
    m_titleLable->setFont(font);
    m_titleLable->setText(tr("Messages")); /*给label设置信息*/

    m_titleClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_titleClose->setIconSize(QSize(16, 16));
    m_titleClose->setFixedSize(QSize(30, 30));
    m_titleClose->setProperty("isWindowButton", 0x2);
    m_titleClose->setProperty("useIconHighlightEffect", 0x8);
    m_titleClose->setFlat(true);
    m_titleClose->setFocusPolicy(Qt::NoFocus);
    m_titleClose->setToolTip(tr("Close"));

    // 设置标签字号及颜色
    QFont labFont = GlobalData::getInstance()->getFontSize14px();

    QString labColorStr = "#8C8C8C";
    QString labGrayStyle = "color:#8C8C8C;";

    QPalette labFontPe;
    labFontPe.setColor(QPalette::WindowText, QColor(labColorStr)); //设置颜色

    // 设置好友信息字号
    QFont infoFont = GlobalData::getInstance()->getFontSize14px();

    // 好友头像
    m_avatarLab->setFixedSize(GlobalSizeData::AVATAR_LAB_SIZE);
    m_avatarLab->setAlignment(Qt::AlignCenter);
    // m_avatarLab->setStyleSheet("background-color:#3790FA;border-radius:30px;color:white;font:28px;");
    setBackgroundColor();

    QSize leftLabSize = QSize(70, 20);
    QString locale = QLocale::system().name();
    if (locale == "zh_CN") {
        leftLabSize.setWidth(50);
    }

    // 好友用户名
    // m_usernameLab->setFixedSize(leftLabSize);
    m_usernameLab->setFont(labFont);
    m_usernameLab->setPalette(labFontPe);
    m_usernameLab->setText(tr("Username"));
    m_usernameLab->setStyleSheet(labGrayStyle);

    // m_usernameInfo->setFixedSize(QSize(140, 20));
    m_usernameInfo->setFont(infoFont);

    // 好友IP地址
    // m_ipAddrLab->setFixedSize(leftLabSize);
    m_ipAddrLab->setFont(labFont);
    m_ipAddrLab->setText(tr("IP Address"));
    m_ipAddrLab->setStyleSheet(labGrayStyle);

    // m_ipAddrInfo->setFixedSize(QSize(140, 20));
    m_ipAddrInfo->setFont(infoFont);

    // 好友备注
    // m_nicknameLab->setFixedSize(leftLabSize);
    m_nicknameLab->setFont(labFont);
    m_nicknameLab->setPalette(labFontPe);
    m_nicknameLab->setText(tr("Nickname"));
    m_nicknameLab->setStyleSheet(labGrayStyle);

    m_nicknameInfo->setFont(infoFont);
    m_nicknameInfo->setMargin(0);

    // 修改好友备注按钮
    m_changeNicknameBtn->setIcon(QIcon::fromTheme("document-edit-symbolic"));
    m_changeNicknameBtn->setFixedSize(QSize(20, 20));
    m_changeNicknameBtn->setIconSize(GlobalSizeData::CHANGE_NAME_BTN_ICON);
    m_changeNicknameBtn->setProperty("isWindowButton", 0x1);
    m_changeNicknameBtn->setProperty("useIconHighlightEffect", 0x2);
    m_changeNicknameBtn->setFlat(true);
    m_changeNicknameBtn->setFocusPolicy(Qt::NoFocus);

    // 应用内居中
    this->move(KyView::getInstance()->geometry().center() - this->rect().center());

    this->update();
}

void FriendInfoWid::initGsetting()
{
    // 透明度
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, this, [=]() {
        //GlobalData::getInstance()->getSystemTransparency();
        this->update();
    });

    return;
}

void FriendInfoWid::paintEvent(QPaintEvent *event)
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
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(GlobalSizeData::BLUR_TRANSPARENCY);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.fillPath(rectPath, brush);
}

// 更新好友信息
void FriendInfoWid::slotUpdateFriendInfo(int friendId)
{
    if (m_friendId == friendId && !(this->isHidden())) {
        fillFriendInfo(friendId);
    }
}

// 填充好友信息
void FriendInfoWid::fillFriendInfo(int friendId)
{
    m_friendId = friendId;

    // 获取好友信息
    QStandardItem *item = FriendListModel::getInstance()->getFriendById(m_friendId);
    QString avatarStr = item->data(FriendListModel::Avatar).toString();
    QString usernameStr = item->data(FriendListModel::Username).toString();
    QString ipAddrStr = item->data(FriendListModel::Ip).toString();
    QString nicknameStr = item->data(FriendListModel::Nickname).toString();

    QString labGrayStyle = "color:#8C8C8C;";
    QString labBlackStyle = "color:#1F2022;";

    m_avatarLab->setText(avatarStr);
    m_ipAddrInfo->setText(ipAddrStr);

    // 好友用户名 过长时进行省略显示
    QFontMetrics fontmts = QFontMetrics(m_usernameInfo->font());
    QString displayUserName = usernameStr;

    // int dif = fontmts.width(usernameStr) - m_usernameInfo->width();
    int dif = fontmts.width(usernameStr) - 125;
    if (dif > 0) {
        m_usernameInfo->setToolTip(usernameStr);
        displayUserName = fontmts.elidedText(usernameStr, Qt::ElideRight, 125);
    }
    m_usernameInfo->setText(displayUserName);

    // 好友备注 过长时进行省略显示
    if (nicknameStr.isEmpty()) {

        m_nicknameInfo->setStyleSheet(labGrayStyle);
        m_nicknameInfo->setText(tr("Add"));
        m_nicknameInfo->setMargin(0);

        m_nicknameInfo->setFixedWidth(fontmts.width(m_nicknameInfo->text()));
        m_nicknameInfo->setScaledContents(true);
        m_nicknameInfo->setAlignment(Qt::AlignBottom);

    } else {

        // m_nicknameInfo->setFixedSize(QSize(100, 20));
        m_nicknameInfo->setStyleSheet("");

        QString displayNickName = nicknameStr;
        int dif = fontmts.width(nicknameStr) - 100;
        if (dif > 0) {
            m_nicknameLab->setToolTip(nicknameStr);
            displayNickName = fontmts.elidedText(nicknameStr, Qt::ElideRight, 100);
        }
        m_nicknameInfo->setText(displayNickName);
        m_nicknameInfo->setFixedWidth(fontmts.width(m_nicknameInfo->text()));
    }

    // this->show();
}

// 窗口关闭事件
void FriendInfoWid::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    this->hide();
    event->ignore();
}

// 修改好友备注
void FriendInfoWid::changeNickname()
{
    /* SDK功能点：修改好友备注 */
    GlobalSizeData::SDKPointModifyFriendNotes();

    LocalUpdateName *updateNameWid = new LocalUpdateName(LocalUpdateName::FriendName, m_friendId, this);
    updateNameWid->setAccessibleName("kylin-ipmsg_friendinfowid_LocalUpdateName_updateNameWid");
    updateNameWid->show();
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(updateNameWid->windowHandle());
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
    ::kabase::WindowManage::setMiddleOfScreen(updateNameWid);
    }
}

void FriendInfoWid::slotChangeFont()
{
    m_avatarLab->setFont(GlobalData::getInstance()->m_fontName);
    m_usernameLab->setFont(GlobalData::getInstance()->m_fontName);
    m_ipAddrLab->setFont(GlobalData::getInstance()->m_fontName);
    m_nicknameLab->setFont(GlobalData::getInstance()->m_fontName);
    m_nicknameInfo->setFont(GlobalData::getInstance()->m_fontName);
    return;
}

void FriendInfoWid::setBackgroundColor()
{
    QColor highLightColor = QApplication::palette().highlight().color();
    QString highLightStr = QString("#%1%2%3").arg(highLightColor.red(), 2, 16,QChar('0'))
                                             .arg(highLightColor.green(), 2, 16,QChar('0'))
                                             .arg(highLightColor.blue(), 2, 16,QChar('0'));
    QString str = QString("background-color:%1;border-radius:30px;color:white;font-size:28px;").arg(highLightStr);
    m_avatarLab->setStyleSheet(str);
}

//弹窗显示后按Esc关闭弹窗
void FriendInfoWid::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Escape){
        this->window()->close();
    }
}
