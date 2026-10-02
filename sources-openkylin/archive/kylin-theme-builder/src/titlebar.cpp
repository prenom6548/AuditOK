/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "titlebar.h"

#include <QDesktopServices>
#include <QGSettings>
/**
 * @brief TitleBar类的构造函数
 *
 * TitleBar类的构造函数，用于创建一个带有标题栏的窗口。
 *
 * @param parent 父窗口指针
 */
TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    m_ismaximized = false;
    this->setFixedHeight(30);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
this->setContentsMargins(0,0,5,0);
    iconlabel = new QLabel(this);
    iconlabel->setFixedSize(30, 30);
    m_icon = QIcon::fromTheme("kylin-theme-builder");
    scaledIcon = m_icon.pixmap(QSize(30, 30)).scaled(QSize(30, 30), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap pixmap = scaledIcon.pixmap(QSize(30, 30));
    iconlabel->setPixmap(pixmap);

    QLabel *title = new QLabel(tr("Kylin-Theme-Builder"));

    m_gohomebtn = new QPushButton();
    m_gohomebtn->setFixedSize(30,30);
    m_gohomebtn->setIcon(QIcon::fromTheme("go-home-symbolic"));
    m_gohomebtn->setToolTip(tr("Return"));
    m_gohomebtn->setFlat(true);
    m_gohomebtn->setProperty("isWindowButton", 0x01);

    m_minimumbtn = new QPushButton();
    m_minimumbtn->setFixedSize(30,30);
    m_minimumbtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_minimumbtn->setToolTip(tr("Minimize"));
    m_minimumbtn->setFlat(true);
    m_minimumbtn->setProperty("isWindowButton", 0x01);

    m_maximumbtn = new QPushButton();
    m_maximumbtn->setFixedSize(30,30);
    m_maximumbtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    m_maximumbtn->setToolTip(tr("Maximize"));
    m_maximumbtn->setFlat(true);
    m_maximumbtn->setProperty("isWindowButton", 0x01);


    m_closebtn = new QPushButton();
    m_closebtn->setFixedSize(30, 30);
    m_closebtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closebtn->setToolTip(tr("Close"));
    m_closebtn->setFlat(true);
    m_closebtn->setProperty("isWindowButton", 0x02);

    connect(m_gohomebtn, &QPushButton::clicked, this,[=](){
        emit gohomesignal();
        this->parentWidget()->setFocus();
    });
    connect(m_minimumbtn, &QPushButton::clicked, this, [=](){
        this->parentWidget()->parentWidget()->showMinimized();
        this->parentWidget()->setFocus();
    });
    connect(m_maximumbtn, &QPushButton::clicked, this, &TitleBar::toggleMaximize);
    connect(m_closebtn, &QPushButton::clicked, parent, [=](){
        parent->close();
        kdk_dia_upload_program_exit();
    });

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        iconSettings = new QGSettings("org.ukui.style", QByteArray(), this);
        connect(iconSettings, &QGSettings::changed, this, [=](const QString &key) {
            m_icon.fromTheme("kylin-theme-builder");
            scaledIcon = m_icon.pixmap(QSize(30, 30)).scaled(QSize(30, 30), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap pixmap = scaledIcon.pixmap(QSize(30, 30));
            iconlabel->setPixmap(pixmap);
            });
    }

    m_optionbtn = new QToolButton();
    m_optionbtn->setToolTip(tr("Menu"));
    m_optionbtn->setPopupMode(QToolButton::InstantPopup);
    m_optionbtn->setFixedSize(30,30);
    m_optionbtn->setIcon(QIcon::fromTheme("application-menu"));
    m_optionbtn->setAutoRaise(true);
    m_optionbtn->setProperty("isWindowButton", 0x01);

    QMenu* menu = new QMenu(m_optionbtn);

    QAction* saveAction = new QAction(tr("Save path"), menu);
    menu->addAction(saveAction);
    QAction* aboutAction = new QAction(tr("about"), menu);
    menu->addAction(aboutAction);
    QAction* helpAction = new QAction(tr("help"), menu);
    menu->addAction(helpAction);
    QAction* exitAction = new QAction(tr("exit"), menu);
    menu->addAction(exitAction);
    connect(saveAction,&QAction::triggered,this,[=](){
        m_savepathdlg = new SavePathDialog();
        connect(m_savepathdlg,&SavePathDialog::savePathUpdate,this,[&](const QString& path){

            emit savePathUpdate(path);
        });
        m_savepathdlg->exec();
    });
    connect(aboutAction,&QAction::triggered,this,[=](){
        AboutDialog *dlg = new AboutDialog(this);
        dlg->exec();
    });
    connect(helpAction,&QAction::triggered,this,[=](){
        QUrl Url("https://gitee.com/openkylin/kylin-theme-builder/blob/openkylin/nile/README.md");
        QDesktopServices::openUrl(Url);
    });
    connect(exitAction,&QAction::triggered,m_closebtn,&QPushButton::clicked);
    m_optionbtn->setMenu(menu);

    layout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QSpacerItem *spacer = new QSpacerItem(44, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    buttonLayout->addWidget(iconlabel);
    buttonLayout->addWidget(title);
    buttonLayout->addSpacerItem(spacer);
    buttonLayout->addWidget(m_gohomebtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_optionbtn);
    buttonLayout->addWidget(m_minimumbtn);
    buttonLayout->addWidget(m_maximumbtn);
    buttonLayout->addWidget(m_closebtn);


    layout->addLayout(buttonLayout);
}

/**
 * @brief 切换最大化状态
 *
 * 如果当前窗口不是最大化状态，则将其最大化，并更新相应的按钮图标。
 * 如果当前窗口已经是最大化状态，则将其恢复为正常状态，并更新相应的按钮图标。
 */
void TitleBar::toggleMaximize()
{
    // 如果当前窗口不是最大化状态
    if (!m_ismaximized) {
        // 显示最大化窗口
        parentWidget()->parentWidget()->showMaximized();
        // 设置状态为已最大化
        m_ismaximized = true;
        // 设置最大化按钮的图标为还原图标
        m_maximumbtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_maximumbtn->setToolTip(tr("Restore"));

        emit maximizesignal();
    // 如果当前窗口已经是最大化状态
    } else {
        // 显示普通窗口
        parentWidget()->parentWidget()->showNormal();
        // 设置状态为未最大化
        m_ismaximized = false;
        // 设置最大化按钮的图标为最大化图标
        m_maximumbtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_maximumbtn->setToolTip(tr("Maximize"));
        emit restoresignal();
    }
    this->parentWidget()->setFocus();
}

void TitleBar::setGoHomeBtn(bool isable)
{
    if(isable){
        m_gohomebtn->setDisabled(false);
    }else{
        m_gohomebtn->setDisabled(true);
    }
    this->parentWidget()->setFocus();
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        toggleMaximize();
    }

    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    bool isMaximized = isMaximizedDuringDrag();

    if (isMaximized) {
        m_ismaximized = true;
        m_maximumbtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_maximumbtn->setToolTip(tr("Restore"));
    } else {
        m_ismaximized = false;
        m_maximumbtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_maximumbtn->setToolTip(tr("Maximize"));
    }
    QWidget::mouseMoveEvent(event);
}

bool TitleBar::isMaximizedDuringDrag() const {
    // 检查窗口状态是否包含最大化标志
    return (windowState() & Qt::WindowMaximized) != 0;
}
