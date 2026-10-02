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
#include "changeuserlogo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QDBusInterface>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QDate>

#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/klineframe.h>

#include "ukcccommon.h"
using namespace ukcc;
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

#include "flowlayout.h"

ChangeUserLogo::ChangeUserLogo(QString realname, QString username, QString currentIcon, QWidget *parent) :
    KWidget(parent),
    m_realname(realname),
    m_username(username),
    m_currentIcon(currentIcon)
{
    this->setWindowFlag(Qt::Dialog);
    this->setWindowModality(Qt::WindowModality::WindowModal);
    this->setFixedSize(760, 580);
    m_pLogosBtnGroup = new QButtonGroup;
    initUI();
    refreshUserLogo(m_currentIcon);
    loadSystemLogo();
    loadCustomLogo();
    setupConnect();
}

ChangeUserLogo::~ChangeUserLogo()
{
}

void ChangeUserLogo::loadSystemPath(QString systemPath)
{
    // 遍历头像目录
    QDir facesDir = QDir(systemPath);
    foreach (QString filename, facesDir.entryList(QDir::Files | QDir::Dirs)) {
        QFileInfo fileInfo(systemPath+filename);
        if(fileInfo.isDir() && filename != "." && filename != "..") {
            loadSystemPath(systemPath + filename + "/");
        }
        QString fullface = QString("%1%2").arg(systemPath).arg(filename);
        // 2107与2203头像资源保留，但不在弹窗显示
        if (systemPath == FACEPATH && (((!UkccCommon::isCommunity() && !UkccCommon::isOpenkylin()) && filename != "01-default-commercial.png")
                                         || ((UkccCommon::isCommunity() || UkccCommon::isOpenkylin()) && filename != "01-default-community.png"))) {
            continue;
        }

        if (filename == "." || filename == "..") {
            continue;
        }

        QPushButton *button = new QPushButton;
        KDK_EXTEND_ALL_INFO_FORMAT(button, "UserInfo", fullface, "system recommend logo button of ChangeUserLogo dailog");
        m_pLogosBtnGroup->addButton(button);
        button->setCheckable(true);
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setFixedSize(QSize(FACEWIDTH, FACEHEIGHT));
        button->setProperty("isRoundButton", true);
        button->setFocusPolicy(Qt::NoFocus);
        CircleLabel *label = new CircleLabel(fullface, button);
        label->setFixedSize(QSize(FACEWIDTH-FACEBODER*2, FACEHEIGHT-FACEBODER*2));
        label->move(button->x()+FACEBODER, button->y()+FACEBODER);

        connect(button, &QPushButton::clicked, [=]{
            // show dialog更新头像
            refreshUserLogo(fullface);
            m_selected = fullface;
            m_systemFaces = true;
        });

        m_pSystemLogosFlowLayout->addWidget(button);
    }
}

void ChangeUserLogo::loadSystemLogo()
{
    loadSystemPath(FACEPATH);

}

bool ChangeUserLogo::isDateValid(const QString &dateString, const QString &format)
{
    QDateTime date;
    if (!date.fromString(dateString, format).isValid()) {
        return false; // 返回false表示解析失败，即格式不正确
    }
    return true; // 返回true表示解析成功，即格式正确
}

void ChangeUserLogo::loadCustomLogo()
{
    QWidget *addWidget = new QWidget(this);
    addWidget->setFixedSize(FACEWIDTH + CHECKBOXWIDTH/2, FACEHEIGHT + CHECKBOXHEIGHT/2);
    m_pAddCustomLogoBtn->setParent(addWidget);
    m_pCustomLogosFlowLayout->addWidget(addWidget);
    m_pAddCustomLogoBtn->move(addWidget->x()+(addWidget->width() - m_pAddCustomLogoBtn->width())/2,
                              addWidget->y()+(addWidget->height() - m_pAddCustomLogoBtn->height())/2);

    // 遍历头像目录
    QDir facesDir = QDir(CUSTOMFACEPATH);
    foreach (QString filename, facesDir.entryList(QDir::Files)) {
        QString fullface = QString("%1%2").arg(CUSTOMFACEPATH).arg(filename);
        QStringList tmpList = fullface.split("-");
        QString username;
        if (tmpList.length() > 2) {
            // 判断字符串是否符合时间戳格式，从后往前读用户名，判断是否是当前用户头像
            QString time = tmpList.at(tmpList.count() - 1);
            if (time.split(".").length() > 1) {
                qDebug() << "fullface = " << fullface << ";remove sufix" << time.split(".").at(0) << isDateValid(time.split(".").at(0), "yyyyMMddhhmmss");
                if (isDateValid(time.split(".").at(0), "yyyyMMddhhmmss")) {
                    QString tmpPic = fullface;
                    QString removeStr =  tmpPic.remove("-" + time); //移除时间戳后缀
                    int n = m_username.length(); // 实际用户名长度
                    username = removeStr.right(n); // 从图片名获取的用户名
                    qDebug() << fullface << "removedstr = " << removeStr << ";length of m_username = "<< m_username.length() << ";got username from pic name = " << username;
                }
            }
        }

        // 非当前用户历史头像不显示在当前用户弹窗界面
        if (username != m_username) {
            continue;
        }

        QWidget *logoWidgets = new QWidget(this);
        logoWidgets->setFixedSize(FACEWIDTH + CHECKBOXWIDTH/2, FACEHEIGHT + CHECKBOXHEIGHT/2);
        // setFixedSize不管设置多少，复选框可见大小都是16，setFixedSize设置的是实际QCheckBox控件的大小，复选框可见大小不变
        QCheckBox *checkbox = new QCheckBox(logoWidgets);
        checkbox->setFixedSize(CHECKBOXWIDTH, CHECKBOXHEIGHT);
        checkbox->setProperty("logoName", fullface);
        checkbox->hide();
        checkbox->setContentsMargins(0, 0, 0, 0);
        QPushButton *button = new QPushButton(logoWidgets);
        KDK_EXTEND_ALL_INFO_FORMAT(button, "UserInfo", fullface, "custom logo button of ChangeUserLogo dailog");
        m_pLogosBtnGroup->addButton(button); // 添加到 buttongroup 实现按钮单选
        button->setProperty("logoName", fullface);
        button->setCheckable(true);
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setFixedSize(QSize(FACEWIDTH, FACEHEIGHT));
        button->setProperty("isRoundButton", true);
        button->setFocusPolicy(Qt::NoFocus);
        CircleLabel *label = new CircleLabel(UkccCommon::makeRoundLogo(fullface, button->width(), button->height(), button->width()/2, button->devicePixelRatioF()), button);
        label->setFixedSize(QSize(FACEWIDTH-FACEBODER*2, FACEHEIGHT-FACEBODER*2));
        label->move(button->x()+FACEBODER, button->y()+FACEBODER);
        button->move(logoWidgets->x()+(logoWidgets->width() -button->width())/2, logoWidgets->y()+(logoWidgets->height() -button->height())/2);
        checkbox->move(label->x() + FACEWIDTH/2 + FACEWIDTH/2/1.414, logoWidgets->y());

        connect(button, &QPushButton::clicked, [=](bool checked) {
            // 未显示复选框，点击图片按钮，设置头像
            if (!checkbox->isVisible()) {
                // show dialog更新头像
                refreshUserLogo(fullface);
                m_selected = fullface;
                m_systemFaces = false;
            } else {
                // 显示复选框，根据复选框状态设置选中状态
                if (checkbox->isChecked()) {
                    if (m_selectedLogos.contains(fullface)) {
                        m_selectedLogos.removeOne(fullface);
                    }
                    checkbox->setChecked(false);
                } else {
                    if (!m_selectedLogos.contains(fullface)) {
                        m_selectedLogos << fullface;
                    }
                    checkbox->setChecked(true);
                }
            }
        });

        connect(checkbox, &QCheckBox::stateChanged, this, [=](int checked){
            if (checked == Qt::Checked) {
                button->setChecked(true);
                if (!m_selectedLogos.contains(fullface)) {
                    m_selectedLogos << fullface;
                }
            } else if (checked == Qt::Unchecked){
                if (m_selectedLogos.contains(fullface)) {
                    m_selectedLogos.removeOne(fullface);
                }
                button->setChecked(false);
            }
            refreshDeleteStatus();
        });

        m_pCustomLogoCheckboxList << checkbox;
        m_pCustomLogoButtonList << button;
        m_pCustomLogosFlowLayout->addWidget(logoWidgets);
    }
}

void ChangeUserLogo::initUI()
{
    this->setWidgetName(tr("User Logo"));
    this->setWindowTitle(tr("User Logo"));
    this->setAccessibleName(tr("User Logo"));
    this->setIcon("ukui-control-center");
    this->setLayoutType(HorizontalType);
    this->windowButtonBar()->menuButton()->hide();
    this->windowButtonBar()->minimumButton()->hide();
    this->windowButtonBar()->maximumButton()->hide();

    // 左侧导航
    m_pCurrentLogoLabel = new CircleLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pCurrentLogoLabel, "UserInfo", m_username, "preview logo of ChangeUserLogo dailog");
    m_pCurrentLogoLabel->setAccessibleName(tr("User Logo"));
    m_pCurrentLogoLabel->setAccessibleDescription(QString());
    m_pCurrentLogoLabel->setFixedSize(QSize(96, 96));

    m_pNickNameLabel = new KLabel(this);
    m_pNickNameLabel->setText(m_realname);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNickNameLabel, "UserInfo", m_username, "nickname label of ChangeUserLogo dailog");
    m_pNickNameLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *logoAndNameVLayout = new QVBoxLayout();
    logoAndNameVLayout->setAlignment(Qt::AlignCenter);
    logoAndNameVLayout->addSpacing(8);
    logoAndNameVLayout->addWidget(m_pCurrentLogoLabel);
    logoAndNameVLayout->addWidget(m_pNickNameLabel); 
    m_pLeftBarScrollArea = new KNavigationBar(this);
    m_pSystemLogoItem = new QStandardItem(tr("System Recommend"));
    m_pSystemLogoItem->setAccessibleDescription("System Recommend QStandardItem");
    m_pSystemLogoItem->setAccessibleText("ukui-control-center_QStandardItem_m_pSystemLogoItem_ChangeUserLogo");
    m_pCustomLogoItem = new QStandardItem(tr("Customize"));
    m_pCustomLogoItem->setAccessibleDescription("Customize QStandardItem");
    m_pCustomLogoItem->setAccessibleText("ukui-control-center_QStandardItem_m_pCustomLogoItem_ChangeUserLogo");
    m_pLeftBarScrollArea->addItem(m_pSystemLogoItem);
    m_pLeftBarScrollArea->addItem(m_pCustomLogoItem);
    m_pLeftBarScrollArea->listview()->setCurrentIndex(m_pSystemLogoItem->index());

    QVBoxLayout *leftBarVLayout = new QVBoxLayout(this);
    leftBarVLayout->setContentsMargins(8, 8, 0, 0);
    leftBarVLayout->addLayout(logoAndNameVLayout);
    leftBarVLayout->addSpacing(16);
    leftBarVLayout->addWidget(m_pLeftBarScrollArea);
    sideBar()->setLayout(leftBarVLayout);

    // 右侧头像列表
    QVBoxLayout *logoVLayout = new QVBoxLayout();
    logoVLayout->setContentsMargins(24, 0, 0, 0);

    QWidget *titleWidget = new QWidget(this);
    titleWidget->setMinimumHeight(36);
    m_pTitleLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pTitleLabel, "UserInfo", m_username, "title label of ChangeUserLogo dailog");
    m_pTitleLabel->setText(tr("System Recommend"));
    m_pEditButton =  new KBorderlessButton(tr("Edit"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pEditButton, "UserInfo", m_username, "edit button of ChangeUserLogo dailog");
    m_pEditButton->setAccessibleName(tr("Edit"));
    m_pEditButton->setAccessibleDescription(QString());
    m_pDeleteButton = new KBorderlessButton(tr("Delete"));;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pDeleteButton, "UserInfo", m_username, " delete button of ChangeUserLogo dailog");
    m_pDeleteButton->setAccessibleName(tr("Delete"));
    m_pDeleteButton->setAccessibleDescription(QString());
    m_pCancelButton = new KBorderlessButton(tr("Cancel"));;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pCancelButton, "UserInfo", m_username, "cancel buttton of ChangeUserLogo dailog");
    m_pCancelButton->setAccessibleName(tr("Cancel"));
    m_pCancelButton->setAccessibleDescription(QString());
    m_pSelectAllCheckBox = new QCheckBox(tr("All"));;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSelectAllCheckBox, "UserInfo", m_username, "select all label of ChangeUserLogo dailog");
    m_pSelectAllCheckBox->setAccessibleName(tr("All"));
    m_pSelectAllCheckBox->setAccessibleDescription(QString());
    setButtonVisible(false);
    m_pEditButton->setVisible(false);
    QHBoxLayout *titleHLyout = new QHBoxLayout();
    titleHLyout->setContentsMargins(0, 0, 16, 0);
    titleHLyout->addWidget(m_pTitleLabel);
    titleHLyout->addStretch();
    titleHLyout->addWidget(m_pEditButton);
    titleHLyout->addWidget(m_pSelectAllCheckBox);
    titleHLyout->addWidget(m_pDeleteButton);
    titleHLyout->addWidget(m_pCancelButton);
    titleWidget->setLayout(titleHLyout);

    QScrollArea *systemLogosScrollArea = new QScrollArea;
    systemLogosScrollArea->setWidgetResizable(true);
    systemLogosScrollArea->setFrameShape(QFrame::Box);
    systemLogosScrollArea->setFrameShadow(QFrame::Plain);
    m_pSystemLogosFlowLayout = new FlowLayout(0, 8, 8);
    m_pSystemLogosFlowLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *systemLogoWidget = new QWidget(systemLogosScrollArea);
    systemLogoWidget->setLayout(m_pSystemLogosFlowLayout);
    systemLogosScrollArea->setWidget(systemLogoWidget);

    QScrollArea *customLogosScrollArea = new QScrollArea;
    customLogosScrollArea->setWidgetResizable(true);
    customLogosScrollArea->setFrameShape(QFrame::Box);
    customLogosScrollArea->setFrameShadow(QFrame::Plain);
    m_pCustomLogosFlowLayout = new FlowLayout(0, 0, 0);
    m_pCustomLogosFlowLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *customLogoWidget = new QWidget(customLogosScrollArea);
    customLogoWidget->setLayout(m_pCustomLogosFlowLayout);
    customLogosScrollArea->setWidget(customLogoWidget);

    m_pLogosWidget = new QStackedWidget();
    m_pLogosWidget->addWidget(systemLogosScrollArea);
    m_pLogosWidget->addWidget(customLogosScrollArea);

    logoVLayout->addWidget(titleWidget);
    logoVLayout->addWidget(m_pLogosWidget);
    logoVLayout->addStretch();

    m_pAddCustomLogoBtn = new QPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pAddCustomLogoBtn, "UserInfo", m_username, "add custom logo button of ChangeUserLogo dailog");
    m_pAddCustomLogoBtn->setFixedSize(QSize(FACEWIDTH, FACEHEIGHT));
    m_pAddCustomLogoBtn->setProperty("isRoundButton", true);
    m_pAddCustomLogoBtn->setFocusPolicy(Qt::NoFocus);
    m_pAddCustomLogoBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));

    // 取消保存按钮
    m_pCancelBtn = new KPushButton;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pCancelBtn, "UserInfo", m_username, "cancel button of ChangeUserLogo dailog");
    m_pCancelBtn->setText(tr("Cancel"));
    m_pCancelBtn->setAccessibleName(tr("Cancel"));
    m_pCancelBtn->setAccessibleDescription(QString());
    m_pConfirmBtn = new KPushButton;
    KDK_EXTEND_ALL_INFO_FORMAT(m_pConfirmBtn, "UserInfo", m_username, "confirm button of ChangeUserLogo dailog");
    m_pConfirmBtn->setText(tr("Save"));
    m_pConfirmBtn->setAccessibleName(tr("Save"));
    m_pConfirmBtn->setAccessibleDescription(QString());
    m_pConfirmBtn->setBackgroundColorHighlight(true);
    m_pBottomBtnsHorLayout = new QHBoxLayout;
    m_pBottomBtnsHorLayout->setSpacing(16);
    m_pBottomBtnsHorLayout->setContentsMargins(0, 8, 24, 24);
    m_pBottomBtnsHorLayout->addStretch();
    m_pBottomBtnsHorLayout->addWidget(m_pCancelBtn);
    m_pBottomBtnsHorLayout->addWidget(m_pConfirmBtn);

    KHLineFrame *hline = new KHLineFrame();
    KDK_EXTEND_ALL_INFO_FORMAT(hline, "UserInfo", m_username, "horizonal line of ChangeUserLogo dailog");
    QVBoxLayout *logosButtomVlayout = new QVBoxLayout();
    logosButtomVlayout->setContentsMargins(0, 0, 0, 0);
    logosButtomVlayout->addLayout(logoVLayout);
    logosButtomVlayout->addWidget(hline);
    logosButtomVlayout->addLayout(m_pBottomBtnsHorLayout);
    baseBar()->setLayout(logosButtomVlayout);
}

void ChangeUserLogo::refreshUserLogo(QString logo)
{
    m_pCurrentLogoLabel->setPixmap(UkccCommon::makeRoundLogo(logo, m_pCurrentLogoLabel->width(), m_pCurrentLogoLabel->height(), m_pCurrentLogoLabel->width()/2, m_pCurrentLogoLabel->devicePixelRatioF()));
}

void ChangeUserLogo::setupConnect()
{

    connect(m_pCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(m_pConfirmBtn, &QPushButton::clicked, this, [=]{
        if (m_selected != "") {
            qDebug() << "selected:" << m_selected << ";" << __LINE__;
            emit confirmChanged(m_selected, m_systemFaces);
        }
        close();
    });

    connect(m_pEditButton, &KBorderlessButton::clicked, this, &ChangeUserLogo::editCustomLogo);
    connect(m_pSelectAllCheckBox, &QCheckBox::stateChanged, this, &ChangeUserLogo::selectAllCustomLogo);
    connect(m_pDeleteButton, &KBorderlessButton::clicked, this, &ChangeUserLogo::deleteCustomLogo);
    connect(m_pCancelButton, &KBorderlessButton::clicked, this, &ChangeUserLogo::cancelEditCustomLogo);

    connect(m_pAddCustomLogoBtn, &QPushButton::clicked, this, &ChangeUserLogo::showLocalFaceDialog);

    connect(m_pLeftBarScrollArea->listview(), &QListView::clicked, this, &ChangeUserLogo::refreshUI);

}

void ChangeUserLogo::editCustomLogo()
{
    m_selected = "";
    m_pEditButton->setVisible(false);
    setButtonVisible(true);
    m_pAddCustomLogoBtn->setEnabled(false);
    int shownCheckboxCount = 0;
    foreach (QCheckBox *checkbox, m_pCustomLogoCheckboxList) {
        if (checkbox != nullptr && checkbox->property("logoName") != m_currentIcon) {
            shownCheckboxCount++;
            checkbox->show();
        }
        if (checkbox->property("logoName") == m_currentIcon) {
            checkbox->setEnabled(false);
            checkbox->setCheckable(false);
        }
    }

    qDebug() << shownCheckboxCount << m_pCustomLogoCheckboxList.count();
    // 存在可选中的复选框，全选按钮启用，否则禁用
    if (shownCheckboxCount > 0) {
        m_pSelectAllCheckBox->setEnabled(true);
    } else {
        m_pSelectAllCheckBox->setEnabled(false);
    }

    foreach (QPushButton *button, m_pCustomLogoButtonList) {
        m_pLogosBtnGroup->removeButton(button);
        button->setChecked(false);
        if (button->property("logoName") == m_currentIcon) {
            button->setEnabled(false);
            button->setCheckable(false);
        }
    }

    refreshDeleteStatus();
}

void ChangeUserLogo::selectAllCustomLogo(int checked)
{
    foreach (QCheckBox *checkbox, m_pCustomLogoCheckboxList) {
        if (checkbox != nullptr) {
            if (checked == Qt::Checked) {
                checkbox->setChecked(true);
            } else if (checked == Qt::Unchecked){
                checkbox->setChecked(false);
            }
        }
    }
    refreshDeleteStatus();
}

void ChangeUserLogo::refreshDeleteStatus()
{
    int checkedCount = 0;
    foreach (QCheckBox *checkbox, m_pCustomLogoCheckboxList) {
        if (checkbox != nullptr) {
            if (checkbox->checkState() == Qt::Checked) {
                checkedCount++;
            }
        }
    }

    qDebug() << checkedCount << m_pCustomLogoCheckboxList.count();

    // 选中复选框，删除按钮启用，否则禁用
    if (checkedCount > 0) {
        m_pDeleteButton->setEnabled(true);
    } else {
        m_pDeleteButton->setEnabled(false);
    }
}

void ChangeUserLogo::deleteCustomLogo()
{
    m_pEditButton->setVisible(true);
    setButtonVisible(false);
    m_pAddCustomLogoBtn->setEnabled(true);

    foreach (QPushButton *button, m_pCustomLogoButtonList) {
        if (button->property("logoName") == m_currentIcon) {
            button->setEnabled(true);
            button->setCheckable(true);
        }
        if (button->isChecked()) {
            m_pCustomLogoButtonList.removeOne(button);
            continue;
        }
        button->setChecked(false);
        m_pLogosBtnGroup->addButton(button);
    }

    foreach (QCheckBox *checkbox, m_pCustomLogoCheckboxList) {
        if (checkbox->isChecked()) {
            m_pCustomLogoCheckboxList.removeOne(checkbox);
            delete checkbox->parent();
            continue;
        }
        checkbox->hide();
    }

    Q_EMIT deleteLogos(m_selectedLogos);
}

void ChangeUserLogo::cancelEditCustomLogo()
{
    m_pEditButton->setVisible(true);
    setButtonVisible(false);
    m_pAddCustomLogoBtn->setEnabled(true);
    m_pSelectAllCheckBox->setChecked(false);
    foreach (QPushButton *button, m_pCustomLogoButtonList) {
        button->setChecked(false);
        m_pLogosBtnGroup->addButton(button);
        if (button->property("logoName") == m_currentIcon) {
            button->setEnabled(true);
            button->setCheckable(true);
        }
    }
    foreach (QCheckBox *checkbox, m_pCustomLogoCheckboxList) {
        checkbox->setChecked(false);
        checkbox->hide();
    }
}

void ChangeUserLogo::setButtonVisible(bool visible)
{
    m_pSelectAllCheckBox->setVisible(visible);
    m_pDeleteButton->setVisible(visible);
    m_pCancelButton->setVisible(visible);
}

void ChangeUserLogo::refreshUI(const QModelIndex &index)
{
    if (index.row() == 1) {
        m_pEditButton->setVisible(true);
        setButtonVisible(false);
        m_pLogosWidget->setCurrentIndex(1);
        m_pTitleLabel->setText(tr("Customize Logos"));
    } else {
        cancelEditCustomLogo();
        m_pEditButton->setVisible(false);
        setButtonVisible(false);
        m_pLogosWidget->setCurrentIndex(0);
        m_pTitleLabel->setText(tr("System Recommend"));
    }
}

void ChangeUserLogo::showLocalFaceDialog()
{
    QString filters = tr("Face files%1").arg("(*.jpg *.jpeg *.png *.svg *.gif)");
    QFileDialog fd;
    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8;// 最大添加U盘数，可以自己定义
    QString home_path = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home_path + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList file_list = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
        QFileInfo fi = file_list.at(i);
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher m_fileSystemWatcher(&fd);
    m_fileSystemWatcher.addPath("/media/" + home_path + "/");
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
        QDir m_wmntDir(path);
        m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
            QFileInfo m_fi = m_wfilist.at(i);
            mntUrlList << QUrl("file://" + m_fi.filePath());
        }
        fd.setSidebarUrls(usb_list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
        fd.setSidebarUrls(usb_list);
    });

    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select custom face file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    fd.setSidebarUrls(usb_list + mntUrlList);

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    qDebug() << "select file path: " << selectedfile;
    if (selectedfile.startsWith(QString("/run/user/%1/gvfs").arg(QString::number(getuid())))) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Non-local images are not supported for configuration"));
        return;
    }

    QFile pic(selectedfile);
    int size = pic.size();

    qDebug() << "size is"  << size;
    if (size >= 1048576) {
        QMessageBox::warning(this, "", tr("Please select images smaller than 1M"));
        return;
    }

    refreshUserLogo(selectedfile);

    m_selected = selectedfile;
    m_systemFaces = false;
}
