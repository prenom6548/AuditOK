#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopServices>
#include "titlebar.h"
#include "mainwindow.h"
#include "settingpage.h"

TitleBar::TitleBar(QWidget *parent)
  : QWidget(parent)
{
    // 按钮初始化
    gsettingItem = new DefaultGetSet;
    gsettingItem->setAccessibleName("kylin-camera_titlebar_DefaultGetSet_gsettingItem");
    funcListButton = new QToolButton(this);
    funcListButton->setAccessibleName("kylin-camera_titlebar_QToolButton_funcListButton");
    m_pFuncLabel = new QLabel(this);
    m_pFuncLabel->setAccessibleName("kylin-camera_QLabel_m_pFuncLabel");
    m_pMinimizeButton = new QPushButton(this);
    m_pMinimizeButton->setAccessibleName("kylin-camera_QPushButton_m_pMinimizeButton");
    m_pMaximizeButton = new QPushButton;     //不指定this
    m_pMaximizeButton->setAccessibleName("kylin-camera_QPushButton_m_pMaximizeButton");
    m_pCloseButton = new QPushButton(this);
    m_pCloseButton->setAccessibleName("kylin-camera_QPushButton_m_pCloseButton");
    setWid = new Setting(this);
    setWid->hide();
    funcListButton->setProperty("isWindowButton", 0x1);
    funcListButton->setProperty("useIconHighlightEffect",0x2);
    funcListButton->setAutoRaise(true);
    funcListButton->setMenu(setWid->menu);
    m_pMinimizeButton->setProperty("isWindowButton", 0x1);
    m_pMinimizeButton->setProperty("useIconHighlightEffect",0x2);
    m_pMinimizeButton->setFlat(true);
    m_maxButtonPressed = false;
    m_pMaximizeButton->setProperty("isWindowButton", 0x1);
    m_pMaximizeButton->setProperty("useIconHighlightEffect",0x2);
    m_pMaximizeButton->setFlat(true);
    m_pCloseButton->setProperty("isWindowButton", 0x2);
    m_pCloseButton->setProperty("useIconHighlightEffect",0x8);
    m_pCloseButton->setFlat(true);
    // 设置空间大小
    m_pFuncLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pMaximizeButton->setFixedSize(30,30);
    m_pMinimizeButton->setFixedSize(30,30);
    m_pCloseButton->setFixedSize(30,30);
    funcListButton->setFixedSize(30,30);

    m_pIconBtn = new QPushButton();
    m_pIconBtn->setAccessibleName("kylin-camera_QPushButton_m_pIconBtn");
    m_pIconBtn->setIconSize(QSize(30, 30));
    m_pIconBtn->setFixedSize(QSize(30, 30));
    m_pIconBtn->setIcon(QIcon::fromTheme("kylin-camera"));
    m_pIconBtn->setFlat(true);
    m_pIconBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);

#ifndef __V10__
    funcListButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_pMinimizeButton->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_pCloseButton->setIcon(QIcon::fromTheme("window-close-symbolic")); //lhl debug 更改title关闭图标
    m_pMaximizeButton->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
#else
    funcListButton->setIcon(QIcon(":/image/funclist.png"));
    m_pMinimizeButton->setIcon(QIcon(":/image/min.png"));
    m_pCloseButton->setIcon(QIcon(":/image/close.png"));
    m_pMaximizeButton->setIcon(QIcon(":/image/max.png"));
#endif
    m_pCloseButton->setProperty("isWindowButton",0x2);
    m_pCloseButton->setProperty("useIconHighlightEffect",0x8);

    funcListButton->setProperty("isWindowButton", 0x1);
    funcListButton->setProperty("useIconHighlightEffect", 0x2);
    funcListButton->setPopupMode(QToolButton::InstantPopup);
    funcListButton->setAutoRaise(true);
    funcListButton->setToolTip(tr("Options"));

    m_pMinimizeButton->setProperty("isWindowButton", 0x1);
    m_pMinimizeButton->setProperty("useIconHighlightEffect", 0x2);
    m_pMinimizeButton->setFlat(true);
    m_pMinimizeButton->setChecked(false);
    m_pMinimizeButton->setFocusPolicy(Qt::NoFocus);
    m_pMinimizeButton->setToolTip(tr("Minimise"));

    m_pCloseButton->setProperty("isWindowButton", 0x2);
    m_pCloseButton->setProperty("useIconHighlightEffect", 0x8);
    m_pCloseButton->setFlat(true);
    m_pCloseButton->setFocusPolicy(Qt::NoFocus);
    m_pCloseButton->setToolTip(tr("Close"));

    m_pMaximizeButton->setProperty("isWindowButton", 0x1);
    m_pMaximizeButton->setProperty("useIconHighlightEffect", 0x2);
    m_pMaximizeButton->setFlat(true);
    m_pMaximizeButton->setFocusPolicy(Qt::NoFocus);
    m_pMaximizeButton->setToolTip(tr("Maximize"));

    // 设置对象名
    m_pFuncLabel->setObjectName("whiteLabel");
    funcListButton->setObjectName("funcListButton");
    m_pMinimizeButton->setObjectName("minimizeButton");
    m_pMaximizeButton->setObjectName("maximizeButton");
    m_pCloseButton->setObjectName("closeButton");
    // 麒麟摄像头显示
    m_pFuncLabel->setText(tr("Camera"));

#ifdef __V10__
    m_pFuncLabel->setStyleSheet("color:#A6A6A6;font-size:14px;font-family:SourceHanSansCN-Regular;");
    QString btnStyle = \
                     "QPushButton{border-radius:4px;}"
                     "QPushButton:hover{background-color:rgba(255,255,255,0.1);}"
                     "QPushButton:pressed{background-color:rgba(255,255,255,0.15);}";
    funcListButton->setStyleSheet(btnStyle);
    m_pMinimizeButton->setStyleSheet(btnStyle);
    m_pMaximizeButton->setStyleSheet(btnStyle);
    m_pCloseButton->setStyleSheet("QPushButton{border-radius:4px;}QPushButton:hover{background-color:#F86457;}");
#endif
    //设置背景色白色
    nowThemeStyle = gsettingItem->getStyleName();
    this->setGeometry(0,0,300,100);
    QPalette pal(this->palette());
    //深色模式下
    if (nowThemeStyle == "ukui-dark" || nowThemeStyle == "ukui-black"){
        //设置背景色黑色
        pal.setColor(QPalette::Background,Qt::black);
    }
    else{
        //设置背景色白色
        pal.setColor(QPalette::Background,Qt::white);
    }
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();

    // 设置按钮布局
    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addSpacing(8);
    pLayout->addWidget(m_pIconBtn);
    pLayout->addWidget(m_pFuncLabel);
    pLayout->addStretch(-4);
    pLayout->addWidget(funcListButton);
    pLayout->addSpacing(-4);
    pLayout->addWidget(m_pMinimizeButton);
    pLayout->addSpacing(-4);
    pLayout->addWidget(m_pMaximizeButton);
    pLayout->addSpacing(-4);
    pLayout->addWidget(m_pCloseButton);
    pLayout->addSpacing(4);
    pLayout->setContentsMargins(0, 0, 0, 0);
    this->setFixedHeight(42);
    this->setLayout(pLayout);
    this->setObjectName("title");
    // 设置信号和槽函数
    connect(m_pMinimizeButton,&QPushButton::clicked,this,&TitleBar::onClicked);
    connect(m_pCloseButton,&QPushButton::clicked,this,&TitleBar::onClicked);
    connect(m_pMaximizeButton,&QPushButton::clicked,this,&TitleBar::onClicked);
    connect(funcListButton,&QToolButton::clicked,this,&TitleBar::onClicked);
    connect(gsettingItem,&DefaultGetSet::sigThemeChange,this,&TitleBar::dealSystemGsettingChange);
}


TitleBar::~TitleBar()
{

}

//按钮功能设置，和槽链接
void TitleBar::onClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel()){
        if (pButton == m_pMinimizeButton)//小化到托盘
        {
            pWindow->showMinimized();
        }
        else if(pButton == m_pMaximizeButton){
            //当前是最大化，设置还原
            if(pWindow->windowState() == Qt::WindowMaximized){
                qDebug() << "resize:: maxmized";
                pWindow->showNormal();
                m_pMaximizeButton->setToolTip(tr("maximize"));
                m_pMaximizeButton->setIcon(QIcon(":/image/window-maximize-symbolic.svg"));

            }//当前是非最大化，设置最大化
            else if(pWindow->windowState() == Qt::WindowNoState){
                pWindow->showMaximized();
                m_pMaximizeButton->setToolTip(tr("normal"));
                m_pMaximizeButton->setIcon(QIcon(":/image/window-restore-symbolic.svg"));
                qDebug() << "reisze: change normal";
            }
        }
        else if (pButton == m_pCloseButton){//mainwidow开始析构
            pWindow->close();
        }
        else{
            qDebug() << "error!";
        }
    }
}

//冗余代码 需优化
void TitleBar::dealSystemGsettingChange(const QString themeNow){
    this->setGeometry(0,0,300,100);
    QPalette pal(this->palette());
    //深色模式下
    if (themeNow == "ukui-dark" || themeNow == "ukui-black"){
        //设置背景色黑色
        pal.setColor(QPalette::Background,Qt::black);
    }
    else{
        //设置背景色白色
        pal.setColor(QPalette::Background,Qt::white);
    }
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}

void TitleBar::titleBarButtonShowOrHide(bool sig){
    if(sig){
        m_pMaximizeButton->show();
    }else{
        m_pMaximizeButton->hide();
    }
}
