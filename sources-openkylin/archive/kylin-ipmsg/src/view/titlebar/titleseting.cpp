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

#include <unistd.h>

#include "titleseting.h"
#include "titlebar.h"
#include "view/kyview.h"
#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
#undef Unsorted
#include <QDir>
#include <QFileDialog>
#include <QDirIterator>
#include <sys/stat.h>

#include <error.h>
#include <windowmanage.hpp>

#include "global/database/chatmsgdb.h"
#include "controller/control.h"
#include "ukuiwindowhelper.h"

TitleSeting::TitleSeting(QWidget *parent) : QWidget(parent)
{
    //初始化
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();
}

void TitleSeting::setWidgetUi()
{
    this->setWindowFlag(Qt::Tool);
    this->setWindowModality(Qt::WindowModal);
    this->setWindowModality(Qt::ApplicationModal);

    /* 适配kysdk的窗管 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    windowhelper->removeTitleBar();

    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    m_labelCatalogue = new QLabel(this);      //目录名称
    m_labelTip = new QLabel(this);            //提示
    m_lineRoute = new QLineEdit(this);        //路径
    m_chooseRouteBtn = new QPushButton(this); //选择路径
    m_clearChatBtn = new QPushButton(this);   //清空聊天记录
    m_clearCacheBtn = new QPushButton(this);  //清空缓存

    m_titleBar = new TitleBar(false, false, false);
    m_titleBar->m_pMinimizeButton->hide();

    m_labelCatalogue->setFixedHeight(36);
    m_lineRoute->setFixedSize(356, 48);
    m_chooseRouteBtn->setFixedSize(356, 36);
    m_clearChatBtn->setFixedSize(356, 36);
    m_clearCacheBtn->setFixedSize(356, 36);

    m_mainLayout = new QVBoxLayout(this); // 设置页面标题布局
    m_vControlLayout = new QVBoxLayout(); // 总体控件纵向布局
    m_hControlLayout = new QHBoxLayout(); // 总体控件横向布局

    //设置objectname
    m_labelCatalogue->setAccessibleName("kylin-ipmsg_titleseting_QLabel_mlabelCatalogue");
    m_labelTip->setAccessibleName("kylin-ipmsg_titleseting_QLabel_mlabelTip");
    m_lineRoute->setAccessibleName("kylin-ipmsg_titleseting_QLineEdit_mlineRoute");
    m_chooseRouteBtn->setAccessibleName("kylin-ipmsg_titleseting_QPushButton_mchooseRouteBtn");
    m_clearChatBtn->setAccessibleName("kylin-ipmsg_titleseting_QPushButton_mclearChatBtn");
    m_clearCacheBtn->setAccessibleName("kylin-ipmsg_titleseting_QPushButton_mclearCacheBtn");
    m_titleBar->setAccessibleName("kylin-ipmsg_titleseting_TitleBar_mtitleBar");
    m_mainLayout->setObjectName("kylin-ipmsg_titleseting_QVBoxLayout_mmainLayout");
    m_vControlLayout->setObjectName("kylin-ipmsg_titleseting_QVBoxLayout_mvControlLayout");
    m_hControlLayout->setObjectName("kylin-ipmsg_titleseting_QHBoxLayout_mhControlLayout");

    // 对组件进行布局
    m_vControlLayout->setSpacing(0);
    m_vControlLayout->addWidget(m_labelCatalogue, Qt::AlignBottom);
    m_vControlLayout->addSpacing(10);
    m_vControlLayout->addWidget(m_lineRoute);
    m_vControlLayout->addSpacing(2);
    m_vControlLayout->addWidget(m_labelTip);
    m_vControlLayout->addSpacing(3);
    m_vControlLayout->addWidget(m_chooseRouteBtn);
    m_vControlLayout->addSpacing(2);
    m_vControlLayout->addWidget(m_clearChatBtn);
    m_vControlLayout->addSpacing(2);
    m_vControlLayout->addWidget(m_clearCacheBtn);
    m_vControlLayout->addSpacing(2);
    m_vControlLayout->setMargin(0);

    m_hControlLayout->addStretch();
    m_hControlLayout->addLayout(m_vControlLayout);
    m_hControlLayout->addStretch();
    //    m_hLayout->setMargin(0);
    //    m_hLayout->setSpacing(0);

    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_titleBar);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addLayout(m_hControlLayout);
    m_mainLayout->addStretch();
    m_mainLayout->setMargin(0);

    connect(m_titleBar->m_pCloseButton, &QPushButton::clicked, this, &TitleSeting::close);
    connect(m_chooseRouteBtn, &QPushButton::clicked, this, &TitleSeting::updateRoute);
    connect(m_clearChatBtn, &QPushButton::clicked, this, &TitleSeting::clearChat);
    connect(m_clearCacheBtn, &QPushButton::clicked, this, &TitleSeting::clearCache);
}

void TitleSeting::setWidgetStyle()
{
    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    this->setFixedSize(420, 320);

    QFont fontLabel = GlobalData::getInstance()->getFontSize16px();
    m_labelCatalogue->setFont(fontLabel);
    m_labelCatalogue->setText(tr("File Save Directory"));

    QFont font = GlobalData::getInstance()->getFontSize14px();

    m_lineRoute->setFont(font);
    m_lineRoute->setFocusPolicy(Qt::ClickFocus);
    m_lineRoute->setReadOnly(true);
    m_lineRoute->setCursor(Qt::IBeamCursor);
    m_lineRoute->setText(IniSettings::getInstance()->getFilePath()); //路径
    m_lineRoute->setTextMargins(8, 6, 8, 6);
    m_lineRoute->setCursorPosition(0);

    m_labelTip->setMargin(0);
    m_labelTip->setStyleSheet("color:#f44e50;font:14px;");
    getPower(m_lineRoute->text());

    m_chooseRouteBtn->setText(tr("Change Directory"));
    m_chooseRouteBtn->setFont(font);
    m_chooseRouteBtn->setFocusPolicy(Qt::NoFocus);
    m_chooseRouteBtn->setProperty("useButtonPalette", true);
    m_chooseRouteBtn->setProperty("isWindowButton", 0x1);
    m_chooseRouteBtn->setProperty("useIconHighlightEffect", 0x2);
    //    m_ChooseRouteBtn->setFlat(true);
    // m_chooseRouteBtn->setStyleSheet("QPushButton{text-align: left;padding-left: 16px;}");

    m_clearChatBtn->setText(tr("Clear All Chat Messages"));
    m_clearChatBtn->setFont(font);
    m_clearChatBtn->setFocusPolicy(Qt::NoFocus);
    m_clearChatBtn->setProperty("useButtonPalette", true);
    m_clearChatBtn->setProperty("isWindowButton", 0x1);
    m_clearChatBtn->setProperty("useIconHighlightEffect", 0x2);
    //    m_ClearChatBtn->setFlat(true);
    // m_clearChatBtn->setStyleSheet("QPushButton{text-align: left;padding-left: 16px;}");

    m_clearCacheBtn->setText(tr("Clear the Cache"));
    m_clearCacheBtn->setFont(font);
    m_clearCacheBtn->setFocusPolicy(Qt::NoFocus);
    m_clearCacheBtn->setProperty("useButtonPalette", true);
    m_clearCacheBtn->setProperty("isWindowButton", 0x1);
    m_clearCacheBtn->setProperty("useIconHighlightEffect", 0x2);
    //    m_ClearCacheBtn->setFlat(true);
    // m_clearCacheBtn->setStyleSheet("QPushButton{text-align: left;padding-left: 16px;}");
    m_clearCacheBtn->setAutoFillBackground(true);
    m_clearCacheBtn->setBackgroundRole(QPalette::Base);


    /* 监听字体变化 */
    connect(Control::getInstance(), &Control::sigFontChange, this, [=]() {
        QFont font14 = GlobalData::getInstance()->getFontSize14px();
        QFont font16 = GlobalData::getInstance()->getFontSize16px();

        m_titleBar->m_pFuncLabel->setFont(font14);
        m_labelCatalogue->setFont(font16);
        m_lineRoute->setFont(font14);
        m_chooseRouteBtn->setFont(font14);
        m_clearChatBtn->setFont(font14);
        m_clearCacheBtn->setFont(font14);
    });
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFont, this, &TitleSeting::slotChangeFont);

    // 设置弹窗应用内居中
    QRect availableGeometry = KyView::getInstance()->geometry();
    this->move(availableGeometry.center() - this->rect().center());
}

// 初始化弹窗
void TitleSeting::setTipWid(QString text)
{
    m_tipWidget->setWindowFlag(Qt::Tool);
    m_tipWidget->setWindowModality(Qt::WindowModal);
    m_tipWidget->setWindowModality(Qt::ApplicationModal);
    m_tipWidget->setAccessibleName("kylin-ipmsg_titleseting_KBallonTip_mtipWidget");

    /* 适配kysdk的窗管 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(m_tipWidget);
    windowhelper->removeTitleBar();

    m_tipWidget->setAutoFillBackground(true);
    m_tipWidget->setBackgroundRole(QPalette::Base);

    m_tipWidText = new QLabel(m_tipWidget); //提示内容
    m_tipWidIcon = new QLabel(m_tipWidget); //提示图标

    m_vTipWidLayout = new QVBoxLayout(m_tipWidget); // 提示弹窗纵向布局
    m_hTipWidLayout = new QHBoxLayout();            // 提示弹窗横向布局

    m_hTipWidLayout->setSpacing(0);
    m_hTipWidLayout->addStretch();
    m_hTipWidLayout->addWidget(m_tipWidIcon);
    m_hTipWidLayout->addSpacing(9);
    m_hTipWidLayout->addWidget(m_tipWidText);
    m_hTipWidLayout->addStretch();
    m_hTipWidLayout->setMargin(0);

    m_vTipWidLayout->setContentsMargins(0, 0, 0, 0);
    m_vTipWidLayout->setSpacing(0);
    m_vTipWidLayout->addStretch();
    m_vTipWidLayout->addLayout(m_hTipWidLayout);
    m_vTipWidLayout->addStretch();
    m_tipWidget->setLayout(m_vTipWidLayout);

    //设置图标
    QIcon searchIcon = QIcon::fromTheme("ukui-dialog-success");
    m_tipWidIcon->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(22, 22))));
    m_tipWidIcon->setProperty("isWindowButton", 0x1);
    m_tipWidIcon->setProperty("useIconHighlightEffect", 0x2);
    m_tipWidIcon->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_tipWidIcon->setAttribute(Qt::WA_TranslucentBackground, true);

    // text为要显示的信息
    m_tipWidText->setText(text);

    int w = m_tipWidText->width() + m_tipWidIcon->width();
    //    m_tipWidget->setFixedSize(120 , 60);
    m_tipWidget->setFixedWidth(w);
    m_tipWidget->setFixedHeight(60);

    // 关于弹窗应用内居中
    QRect availableGeometry = this->geometry();
    m_tipWidget->move(availableGeometry.center() - m_tipWidget->rect().center());

    //设置定时器，到时自我销毁
    QTimer *timer = new QTimer(this);
    timer->start(1500);         //时间1.5秒
    timer->setSingleShot(true); //仅触发一次

    connect(timer, SIGNAL(timeout()), this, SLOT(timeDestroy()));
    connect(Control::getInstance(), &Control::sigFontChange, this, [=]() {
        QFont font14 = GlobalData::getInstance()->getFontSize14px();

        m_tipWidText->setFont(font14);
    });
}

/* 修改目录 */
void TitleSeting::updateRoute()
{
    /* SDK功能点：更改目录 */
    GlobalSizeData::SDKPointChangeDir();

    QString SavePath = QFileDialog::getExistingDirectory(this, tr("Change Directory"), IniSettings::getInstance()->getFilePath());

    if (!SavePath.isEmpty()) {
        if (!SavePath.endsWith("ipmsgReceivedFiles")) {
            if (!SavePath.endsWith("/")) {
                SavePath += "/";
            }
            SavePath += "ipmsgReceivedFiles";
        }

        SavePath = QDir::toNativeSeparators(SavePath);

        m_lineRoute->setText(SavePath);
        m_lineRoute->setCursorPosition(0);

        // 确保目录存在
        QDir dir(SavePath);
        if (!dir.exists()) {
            dir.mkpath(SavePath);
        }

        if (!this->checkSavePath(SavePath)) {
            this->m_labelTip->setText(tr("The save path can only be a dir under the home dir!"));

            return;
        }

        if (!getPower(SavePath)) {
            return;
        }

        IniSettings::getInstance()->setFilePath(SavePath);

        m_tipWidget = new QDialog(); //提示框

        setTipWid(tr("Modified Successfully"));

        m_tipWidget->exec();
    }

    return;
}

bool TitleSeting::checkSavePath(QString path)
{
    if (path.isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.isDir()) {
        return false;
    }

    QString baseName = fileInfo.baseName();
    if (baseName.isEmpty() || baseName.at(0) == QChar('.')) {
        return false;
    }


    QString homePath = getenv("HOME");
    if (!path.contains(homePath) || path == homePath || path == (homePath + '/')) {
        return false;
    }

    return true;
}

/* 清空聊天信息 */
void TitleSeting::clearChat()
{
    /* sdk功能点：清空聊天记录 */
    GlobalSizeData::SDKPointClearChatRecord();

    QMessageBox *msg = new QMessageBox(this);
    msg->setAccessibleName("kylin-ipmsg_titleseting_QMessageBox_msg");
    msg->setWindowTitle(tr(""));
    msg->setText(tr("Clear all messages？"));
    msg->setIcon(QMessageBox::Question);
    msg->addButton(tr("No"), QMessageBox::RejectRole);
    QPushButton *sureBtn = msg->addButton(tr("Yes"), QMessageBox::AcceptRole);
    msg->setDefaultButton(sureBtn);
    msg->exec();

    if ((QPushButton *)msg->clickedButton() == sureBtn) {

        ChatMsgDB::getInstance()->clearAllMsg();
        FriendListModel::getInstance()->clearAllMessage();
        FriendListView::getInstance()->clearAllMessages();

        // 设置弹窗
        m_tipWidget = new QDialog(); //提示框
        setTipWid(tr("Cleared"));
        m_tipWidget->exec();
    }
    return;
}

/* 清空缓存 */
void TitleSeting::clearCache()
{ 
    /* SDK功能点：清空缓存 */
    GlobalSizeData::SDKPointCleanCache();
    
    QMessageBox *msg = new QMessageBox(this);
    msg->setAccessibleName("kylin-ipmsg_titleseting_QMessageBox_msg");
    msg->setWindowTitle(tr(""));
    msg->setText(tr("Clean the cache information such as images/videos/documents?"));
    msg->setIcon(QMessageBox::Question);
    msg->addButton(tr("No"), QMessageBox::RejectRole);
    QPushButton *sureBtn = msg->addButton(tr("Yes"), QMessageBox::AcceptRole);
    msg->setDefaultButton(sureBtn);
    msg->exec();

    if ((QPushButton *)msg->clickedButton() != sureBtn) {
        return;
    }

    QString path = IniSettings::getInstance()->getFilePath();
    if (!path.endsWith("ipmsgReceivedFiles") && !path.endsWith(QString("ipmsgReceivedFiles") + QDir::separator())) {
        if (!path.endsWith(QDir::separator())) {
            path += QDir::separator();
        }
        path += "ipmsgReceivedFiles";
    }

    QDir pathDir(path);
    if (!pathDir.exists()) {
        // 如果目录不存在，说明没有缓存需要清理
        m_tipWidget = new QDialog(); //提示框
        setTipWid(tr("Clean Up Complete"));
        m_tipWidget->exec();
        return;
    }

    if (pathDir.isEmpty()) {
        // 设置弹窗
        m_tipWidget = new QDialog(); //提示框
        setTipWid(tr("Clean Up Complete"));
        m_tipWidget->exec();

        return;
    }

    // 只删除 ipmsgReceivedFiles 目录下的内容
    QDirIterator DirsIterator(path, QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden,
                              QDirIterator::NoIteratorFlags);

    while (DirsIterator.hasNext()) {
        QString itemPath = DirsIterator.next();
        QFileInfo fileInfo(itemPath);
        if (fileInfo.isDir()) {
            QDir(itemPath).removeRecursively();
        } else {
            QFile::remove(itemPath);
        }
    }

    if (path == GlobalUtils::getDefaultPath()) {
        pathDir.mkdir("sendFiles");
    }

    // 设置弹窗
    m_tipWidget = new QDialog(); //提示框
    setTipWid(tr("Clean Up Complete"));
    m_tipWidget->exec();

    return;
}

/* 获取目录权限 */
bool TitleSeting::getPower(QString SavePath)
{
    struct stat buf;
    std::string stdStr = SavePath.toStdString();
    const char *str = stdStr.c_str();

    memset(&buf, 0x00, sizeof(struct stat));

    int ret = stat(str, &buf);
    if (ret) {
        printf("Waring : TitleSeting , getPower , get file stat fail . errno = %d , errstr = %s\n", errno,
               strerror(errno));
        return false;
    }

    if (getuid() != buf.st_uid) {
        if (getgid() != buf.st_uid) {
            if (buf.st_mode & S_IROTH && buf.st_mode & S_IWOTH && buf.st_mode & S_IXOTH) {
                m_labelTip->setText(tr(""));
            } else {
                m_labelTip->setText(tr("Please do not save the file in this directory"));
                return false;
            }
        } else {
            if (buf.st_mode & S_IRGRP && buf.st_mode & S_IWGRP && buf.st_mode & S_IXGRP) {
                m_labelTip->setText(tr(""));
            } else {
                m_labelTip->setText(tr("Please do not save the file in this directory"));
                return false;
            }
        }
    } else {
        if (buf.st_mode & S_IRUSR && buf.st_mode & S_IWUSR && buf.st_mode & S_IXUSR) {
            m_labelTip->setText(tr(""));
        } else {
            m_labelTip->setText(tr("Please do not save the file in this directory"));
            return false;
        }
    }

    return true;
}

/* 销毁弹窗 */
void TitleSeting::timeDestroy()
{
    m_tipWidget->hide();
}

void TitleSeting::slotChangeFont()
{
    m_chooseRouteBtn->setFont(GlobalData::getInstance()->m_fontName);
    m_clearChatBtn->setFont(GlobalData::getInstance()->m_fontName);
    m_clearCacheBtn->setFont(GlobalData::getInstance()->m_fontName);
    m_labelTip->setFont(GlobalData::getInstance()->m_fontName);
    return;
}

//弹窗显示后按Esc关闭弹窗
void TitleSeting::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Escape){
        this->window()->close();
    }
}
