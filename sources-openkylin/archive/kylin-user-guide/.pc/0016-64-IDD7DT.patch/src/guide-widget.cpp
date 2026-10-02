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

#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QScrollBar>
#include <QDesktopServices>
#include <QApplication>
#include <QToolTip>
#include <QCompleter>
#include <QPixmap>
#include <QScreen>
#include <QList>
#include <QSysInfo>
#include <QX11Info>

#include <map>
#include <string>
#include <QProcess>

#include "guide-widget.h"
#include "common-tool/comm_func.h"
// #include "xatom-helper.h"
#include "kaboutdialog.h"
#include "kywebview.h"
using namespace kdk;

KAboutDialog *about_widget = nullptr;
#include "ukuistylehelper/ukuistylehelper.h"

GuideWidget::GuideWidget(QWidget *parent) :QWidget(parent)
{
    this->isTopLevel();
    this->setMinimumSize(835,650);

    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        settings = new QGSettings("org.ukui.style");
        connect(settings,&QGSettings::changed,this,&GuideWidget::slot_SettingsChange);
    }

    // 添加窗管协议
    // MotifWmHints hints;
    // hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    // hints.functions = MWM_FUNC_ALL;
    // hints.decorations = MWM_DECOR_BORDER;
    // hints.setWindowBorderRadius(this->winId(),12,12,12,12);
    // hints.decorations =
    // XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    //去掉窗口管理器后设置边框不生效了，所以下面通过背景图标提供边框,并且支持最小化。

    // QPalette palette;
    // if(settings->get("styleName").toString() == "ukui-black"|| settings->get("styleName").toString() == "ukui-dark"){
    //     palette.setBrush(QPalette::Background, QBrush(QColor(29,29,31)));
    //     // palette.setBrush(QPalette::Text, QBrush(QColor(Qt::white)));
    //     palette.setBrush(QPalette::Text, QBrush(QColor(Qt::red)));

    // }else{
    //     //浅色模式 背景灰 字体黑
    //     palette.setBrush(QPalette::Background, QBrush(QColor(Qt::red)));
    //     palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
    // }
    // palette.setBrush(QPalette::Background, QBrush(QColor(Qt::red)));
    // palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
    // this->setPalette(palette);
    // this->setAutoFillBackground(true);
//    desktop = QApplication::desktop();
    // QRect rect = QGuiApplication::screens().at(0)->availableGeometry();
    // this->move((rect.bottomRight().x()-this->width())/2,(rect.bottomRight().y()-this->height())/2);

    initSettings();
    initUI();
    getDirAndPng();
    kdk::UkuiStyleHelper::self()->removeHeader(this);
}

GuideWidget::~GuideWidget()
{
    if (m_pWebEngineView) {
        delete m_pWebEngineView->page();
    }
}

void GuideWidget::paintEvent(QPaintEvent *event)
{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    painter.setBrush(QBrush(Qt::white));
//    painter.setPen(QColor(79,79,79));
//    QRect rect = this->rect();
////    qDebug()<<"====" <<this->rect() << rect.width() << rect.height();
//    rect.setWidth(rect.width()-2);
//    rect.setHeight(rect.height()-2);
//    painter.drawRoundedRect(rect, 10, 10);

//    QStyleOption opt;
//    opt.init(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
//    //也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 15, 15);
//    {
//        QPainterPath painterPath;
//        painterPath.addRoundedRect(rect, 15, 15);
//        p.drawPath(painterPath);
//    }
    QWidget::paintEvent(event);
}

void GuideWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    //调整边距
    auto rect = this->rect();
    QPainterPath blurPath;
    //增加圆角
    blurPath.addRoundedRect(rect, 12, 12);
    //使用QPainterPath的api生成多边形Region
    setProperty("blurRegion", QRegion(blurPath.toFillPolygon().toPolygon()));
    this->update();

    QTimer* timer = new QTimer();
    timer->start(10);           //以毫秒为单位
    connect(timer,&QTimer::timeout,[=]{
        // m_pWebView->page()->mainFrame()->evaluateJavaScript("Refresh_the_content_interface();");
        timer->stop();
    });

    qDebug() << e->oldSize() << e->size() << QGuiApplication::screens().at(0)->availableGeometry();

    QPushButton *button1 = this->findChild<QPushButton *>("maxOffButton");
    QList<QScreen *> list = QGuiApplication::screens();
    if(e->oldSize() == QSize(list.at(0)->availableGeometry().width(),list.at(0)->availableGeometry().height())){

        button1->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        button1->setToolTip(tr("Maximize"));
        windowsflag = true;
    }
    if(e->size() == QSize(list.at(0)->availableGeometry().width(),list.at(0)->availableGeometry().height())){

        button1->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        button1->setToolTip(tr("Reduction"));
        windowsflag = false;
    }
}

void GuideWidget::initUI()
{   
    m_pWebEngineView = new KyWebView(this);
    m_pWebEngineView->installEventFilter(this);
//    注册通道
    m_pWebChannel = new QWebChannel(this);
    bridge = new Bridge();
    m_pWebChannel->registerObject("bridge", bridge);
    m_pWebEngineView->page()->setWebChannel(m_pWebChannel);

//    开发工具的调用
//     QWebEngineView * pv = new QWebEngineView();
//     m_pWebEngineView->page()->setDevToolsPage(pv->page());
//     pv->show();

    this->setObjectName("m_yWidget");
    QPushButton *backOffButton = new QPushButton(this);
    QPushButton *minOffButton = new QPushButton(this);
    QPushButton *maxOffButton = new QPushButton(this);
    QPushButton *closeOffButton = new QPushButton(this);

    m_pIconLabel = new QLabel(this);
    m_pTitleLabel = new QLabel(this);

    QHBoxLayout* queryWidLayout = new QHBoxLayout;
    search_Line = new QLineEdit(this);
    search_Line->setFocusPolicy(Qt::ClickFocus);
    search_Line->installEventFilter(this);
    // 占位防止偏移
    QLabel *m_placeHoldLabel = new QLabel(this);
    m_placeHoldLabel->setFixedSize(30,30);

    // QIcon icon1;
    // icon1.addFile(":/image/icon-search.png");
    QIcon searchIcon = QIcon::fromTheme("edit-find-symbolic");
    m_queryIcon = new QLabel(this);
    m_queryIcon->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(16, 16))));
    m_queryIcon->setProperty("useIconHighlightEffect",0x2);

    queryWidLayout->addWidget(m_queryIcon);
    search_Line->setLayout(queryWidLayout);

    search_Line->setMinimumSize(196,36);
    search_Line->setMaximumSize(196,36);
    search_Line->setAlignment(Qt::AlignVCenter);
    qDebug() << search_Line->alignment();
    backOffButton->setObjectName("backOffButton");

    m_placeHoldLabel->resize(120,30);
    minOffButton->setObjectName("minOffButton");
    maxOffButton->setObjectName("maxOffButton");
    closeOffButton->setObjectName("closeOffButton");

    m_pIconLabel->setFixedSize(24,24);
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setPixmap(QIcon::fromTheme("kylin-user-guide").pixmap(24,24));

    m_pTitleLabel->setText(tr("Manual"));
    m_pTitleLabel->resize(120,30);
    m_pTitleLabel->setIndent(4);

    backOffButton->setIcon(QIcon::fromTheme("go-home-symbolic")); //给按钮添加图标
    backOffButton->setFixedSize(30,30);
    backOffButton->setProperty("isWindowButton", 0x1);
    backOffButton->setProperty("useIconHighlightEffect", 0x2);
    backOffButton->setFlat(true);
    backOffButton->setFocusPolicy(Qt::NoFocus);
    backOffButton->setVisible(false);
    backOffButton->setToolTip(tr("Go back"));
    backOffButton->setCursor(QCursor(Qt::ArrowCursor));
    backOffButton->hide();


    minOffButton->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minOffButton->setFixedSize(30,30);
    minOffButton->setProperty("isWindowButton", 0x1);
    minOffButton->setProperty("useIconHighlightEffect", 0x2);
    minOffButton->setFlat(true);
    minOffButton->setToolTip(tr("Minimize"));
    minOffButton->setCursor(QCursor(Qt::ArrowCursor));

    maxOffButton->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    maxOffButton->setFixedSize(30,30);
    maxOffButton->setProperty("isWindowButton", 0x1);
    maxOffButton->setProperty("useIconHighlightEffect", 0x2);
    maxOffButton->setFlat(true);
    maxOffButton->setToolTip(tr("Maximize"));
    maxOffButton->setCursor(QCursor(Qt::ArrowCursor));

    closeOffButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeOffButton->setFixedSize(30,30);
    closeOffButton->setProperty("isWindowButton", 0x2);
    closeOffButton->setProperty("useIconHighlightEffect", 0x8);
    closeOffButton->setFlat(true);
    closeOffButton->setToolTip(tr("Close"));
    closeOffButton->setCursor(QCursor(Qt::ArrowCursor));

    m_menu = new QMenu(this);
    m_menu->setMinimumWidth(160);
    m_menu->addAction(tr("About"));
    m_menu->addAction(tr("Quit"));

    menuOffButton  = new QToolButton(this);
    menuOffButton->setToolTip(tr("options"));
    menuOffButton->setProperty("isWindowButton", 0x01);
    menuOffButton->setProperty("useIconHighlightEffect", 0x2);
    menuOffButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    menuOffButton->setAutoRaise(true);
    menuOffButton->setFixedSize(30, 30);
    menuOffButton->setMenu(m_menu);
    menuOffButton->setPopupMode(QToolButton::InstantPopup);

    connect(backOffButton,SIGNAL(released()),this,SLOT(slot_backOffButton()));
    connect(closeOffButton,SIGNAL(released()),this,SLOT(slot_onClicked_closeOffButton()));
    connect(minOffButton,SIGNAL(released()),this,SLOT(slot_onClicked_minOffButton()));
    connect(maxOffButton,SIGNAL(released()),this,SLOT(slot_onClicked_maxOffButton()));
    connect(m_menu,SIGNAL(triggered(QAction*)),this,SLOT(slot_MenuActions(QAction*)));
    connect(search_Line,SIGNAL(returnPressed()),this,SLOT(slot_searchLine()));

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    QGridLayout *widget_layout = new QGridLayout(this);
    widget_layout->setHorizontalSpacing(4);

    widget_layout->addWidget(m_pIconLabel,0,2,1,1);
    widget_layout->addWidget(m_pTitleLabel,0,3,1,5);
    widget_layout->addWidget(backOffButton,0,40,1,1);
    widget_layout->addWidget(search_Line,0,45,1,30,Qt::AlignCenter);
    widget_layout->addWidget(m_placeHoldLabel,0,80,1,1);
    widget_layout->addWidget(menuOffButton,0,90,1,4);
    widget_layout->addWidget(minOffButton,0,94,1,4);
    widget_layout->addWidget(maxOffButton,0,98,1,4);
    widget_layout->addWidget(closeOffButton,0,102,1,4);
    widget_layout->setColumnStretch(50,2);
    widget_layout->addWidget(m_pWebEngineView,1,0,1,106);

    QLocale localeNew;
    QStringList word_List;
    QPalette palette;

    QString lang = "en_US";
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        lang = "zh_CN";
    } 

    if(localeNew.language()==QLocale::Chinese || localeNew.language()==QLocale::Tibetan || systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ")
    {
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
            // 设置搜索框背色
            palette.setBrush(QPalette::Background, QBrush(QColor(38,38,38)));
            palette.setBrush(QPalette::Text, QBrush(QColor(245,245,245)));
            this->setPalette(palette);
            m_pWebEngineView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"index-ubuntukylin_black.html"));
        }else{
            palette.setBrush(QPalette::Background, QBrush(QColor(245,245,245)));
            palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
            this->setPalette(palette);
            m_pWebEngineView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"index-ubuntukylin.html"));
        }
        search_Line->setPlaceholderText("搜索");
        for( std::map<std::string,std::string>::iterator iter = this->keywordMap.begin();iter != this->keywordMap.end(); iter++){
           word_List.append(QString::fromStdString(iter->first));
        }
            m_queryIcon->setMargin(60);
            search_Line->setTextMargins(80,0,0,0);
    }
    else
    {
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
            palette.setBrush(QPalette::Background, QBrush(QColor(38,38,38)));
            palette.setBrush(QPalette::Text, QBrush(QColor(245,245,245)));
            this->setPalette(palette);
            m_pWebEngineView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"index-ubuntukylin_en_US_black.html"));
        }else{
            palette.setBrush(QPalette::Background, QBrush(QColor(245,245,245)));
            palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
            this->setPalette(palette);
            m_pWebEngineView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"index-ubuntukylin_en_US.html"));
        }
        //英文情况下修改搜索的placeholder
        search_Line->setPlaceholderText("Search");
        for( std::map<std::string,std::string>::iterator iter = this->keywordMap.begin();iter != this->keywordMap.end(); iter++){
           word_List.append(QString::fromStdString(iter->second));
        }
            m_queryIcon->setMargin(46);
            search_Line->setTextMargins(66,0,0,0);
    }
    QCompleter *m_completer_autoAdd = new QCompleter(word_List);
    m_completer_autoAdd->setFilterMode(Qt::MatchContains);// 搜索候选框内容匹配
    m_completer_autoAdd->setCaseSensitivity(Qt::CaseInsensitive); // 搜索候选忽略大小写
    search_Line->setCompleter(m_completer_autoAdd);
    connect(m_completer_autoAdd,SIGNAL(activated(const QString&)),this,SLOT(slot_onClicked_completerAutoAdd(const QString&)));
//    connect(m_completer_autoAdd, SIGNAL(activated(const QString&)), search_Line->clear, Qt::QueuedConnection)
    QObject::connect(bridge,SIGNAL(showBackbutton()),this,SLOT(slot_showBackOffButton()));
    QObject::connect(m_pWebEngineView,SIGNAL(loadFinished(bool)),this,SLOT(slot_loadFinished(bool)));
    widget_layout->setContentsMargins(0, 0, 0, 0);
    widget_layout->setVerticalSpacing(4);

    main_layout->addLayout(widget_layout);

    main_layout->setContentsMargins(2,4,4,10);

    this->setLayout(main_layout);

    this->setMouseTracking(true);

    about_widget = new KAboutDialog(this);
    about_widget->setAppIcon(QIcon::fromTheme("kylin-user-guide"));
    about_widget->setAppName(tr("Manual"));
    about_widget->setAppVersion(tr("Version：")+qApp->applicationVersion());
}

void GuideWidget::slot_webGoto(QUrl url)
{
    qDebug() << Q_FUNC_INFO << url;
    if(url.toString().contains("http")||url.toString().contains("mailto"))    // 添加mailto跳转
        QDesktopServices::openUrl(url);
}

void GuideWidget::slot_MenuActions(QAction *action)
{
    if(action->text() == tr("About")){
        about_widget->setModal(true);
        about_widget->show();
    }else if(action->text() == tr("Quit")){
        slot_onClicked_closeOffButton();
    }else{

    }
}

void GuideWidget::slot_SettingsChange(const QString &key)
{
    qDebug() << Q_FUNC_INFO << key << settings->get("styleName").toString();
    if(key == "styleName"){
        QPalette palette;
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
            palette.setBrush(QPalette::Background, QBrush(QColor(38,38,38)));
            palette.setBrush(QPalette::Text, QBrush(QColor(Qt::white)));
            this->setPalette(palette);
            m_pWebEngineView->page()->runJavaScript("SwitchStyle(2);");
            m_pWebEngineView->page()->runJavaScript("changebuttonicon(2);");
        }else{
            palette.setBrush(QPalette::Background, QBrush(QColor(Qt::white)));
            palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
            this->setPalette(palette);
            m_pWebEngineView->page()->runJavaScript("SwitchStyle(1);");
            m_pWebEngineView->page()->runJavaScript("changebuttonicon(1);");
        }
    }
    if ("iconThemeName" == key) {
        m_pIconLabel->setPixmap(QIcon::fromTheme("kylin-user-guide").pixmap(24,24));
        QString iconTheme = settings->get("iconThemeName").toString();
        QString themeJs = QString("switchTheme(\"%1\");").arg(iconTheme);
         if(!iconTheme.isNull()){
            m_pWebEngineView->page()->runJavaScript(themeJs);
         }
    }

    if("systemFont" == key || "systemFontSize" == key ){
        QFont font = this->font();
        for (auto widget: qApp->allWidgets()) {
            qDebug() << Q_FUNC_INFO;
            widget->setFont(font);
        }
        m_pTitleLabel->adjustSize();
    }

    if("themeColor" == key){
        js_changeNavHighlight();
    }
}

void GuideWidget::js_changeNavHighlight(){
    settings->get("themeColor").toString();
    QPalette pal = qApp->palette();
    QColor highlight = pal.color(QPalette::Active, QPalette::Highlight);
    QString highlightJs = QString("changeNavHighlight(\"%1\");").arg(converRGB2HexStr(highlight));
    m_pWebEngineView->page()->runJavaScript(highlightJs);
}

bool GuideWidget::js_getCpuArchitecture()
{
    if (QSysInfo::currentCpuArchitecture() == "mips" || QSysInfo::currentCpuArchitecture() == "mips64")
        return true;
    else
        return false;
}

void GuideWidget::jump_app(QString appName)
{
    qDebug() << Q_FUNC_INFO << appName;
    QTimer::singleShot(1000,this,[=]{
        QString cmd = QString("qt_jumpApp(\"%1\");").arg(appName);
        m_pWebEngineView->page()->runJavaScript(cmd);
    });
}

void GuideWidget::js_executeCommand(QString cmd)
{
    qDebug() << Q_FUNC_INFO << cmd;
    QProcess::startDetached(cmd,QStringList());
}

void GuideWidget::slot_javaScriptFromWinObject()
{
    qDebug() << Q_FUNC_INFO;
}

// 点击联想，进行搜索跳转
void GuideWidget::slot_onClicked_completerAutoAdd(const QString& appName){
    search_Line->setText(appName);
    slot_searchLine();
//    QCompleter信号触发后清理
     QTimer::singleShot(10,this,[=]{
         search_Line->clear();
     });
}

void GuideWidget::slot_searchLine()
{
    qDebug() << Q_FUNC_INFO;
    QLocale localeNew;
    QString appName =search_Line->text();
    std::string keyword = "";

    // 判断是否为能够跳转的页面
    for( std::map<std::string,std::string>::iterator iter = this->keywordMap.begin();iter != this->keywordMap.end(); iter++){
        // 判断是否为英文环境下
        if(localeNew.language()==QLocale::Chinese)
        {
            // 中文
            if(iter->first==appName.trimmed().toStdString()){
                keyword = iter->second;
                break;
            }
        }else{
            // 英文
            if(iter->second==appName.trimmed().toStdString()){
                keyword = iter->second;
                break;
            }
        }

    }

    if(""!=keyword)
        jump_app(QString::fromStdString(keyword));

    search_Line->clear();
}

void GuideWidget::slot_backOffButton()
{
    qDebug() << Q_FUNC_INFO;
     // 判断当前主题模式，返回主页时选择对应主题模式的页面
    QString style = "1";
    if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
        style = "2";
    }
    m_pWebEngineView->page()->runJavaScript("goBackMainUI_ubuntu("+style+");");
    m_pWebEngineView->page()->runJavaScript("loadhandle("+style+");");
    QPushButton *button = this->findChild<QPushButton *>("backOffButton");
    button->hide();
}

void GuideWidget::slot_loadFinished(bool f)
{
    qDebug() << Q_FUNC_INFO << f ;

    QStringList info = js_getIntoFilename();
    qDebug() << info.join(",") ;

    QTimer::singleShot(1000,this,[=]{
        QPalette palette;
        if(settings->get("styleName").toString() == "ukui-black"  || settings->get("styleName").toString() == "ukui-dark"){
            palette.setBrush(QPalette::Background, QBrush(QColor(38,38,38)));
            palette.setBrush(QPalette::Text, QBrush(QColor(Qt::white)));
            this->setPalette(palette);
            QString cmd = QString("SwitchStyle(\"%1\");").arg(2);
            m_pWebEngineView->page()->runJavaScript(cmd);
        }else{
            palette.setBrush(QPalette::Background, QBrush(QColor(Qt::white)));
            palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
            this->setPalette(palette);
            QString cmd = QString("SwitchStyle(\"%1\");").arg(1);
            m_pWebEngineView->page()->runJavaScript(cmd);
        }
        QString iconTheme = settings->get("iconThemeName").toString();
        QString themeJs = QString("switchTheme(\"%1\");").arg(iconTheme);
        if(!iconTheme.isNull()){
            m_pWebEngineView->page()->runJavaScript(themeJs);
        }

        // 调用函数自动添加搜索参数
        slot_getSarchKeyword();
        m_pWebEngineView->page()->runJavaScript("addhtmlapp(\""+info.join(",")+"\")");
        //监听平板模式切换
        m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                                "/",
                                                "com.kylin.statusmanager.interface",
                                                QDBusConnection::sessionBus(),this);
        if (m_statusSessionDbus->isValid()) {
            is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
            tabletStatusChanged(is_tabletmode);
            connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(tabletStatusChanged(bool)));
        } else {
            qDebug() << "Create statusmanager Interface Failed When : " << QDBusConnection::systemBus().lastError();
        }
    });
}

void GuideWidget::slot_onClicked_minOffButton()
{
    QWidget *m_pWindow = this->window();
    if(m_pWindow->isTopLevel())
    {
        m_pWindow->showMinimized();
    }
}

void GuideWidget::slot_onClicked_maxOffButton()
{
    QWidget *m_pWindow = this->window();
    if(m_pWindow->isTopLevel())
    {
        QPushButton *button = this->findChild<QPushButton *>("maxOffButton");
        m_pWindow->isMaximized() ? m_pWindow->showNormal() : m_pWindow->showMaximized();
        m_pWindow->isMaximized() ? button->setIcon(QIcon::fromTheme("window-restore-symbolic")) :button->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_pWindow->isMaximized() ? button->setToolTip(tr("Reduction")) : button->setToolTip(tr("Maximize"));
        if(m_pWindow->isMaximized())
            windowsflag = false;
        else
            windowsflag = true;
    }
}

void GuideWidget::slot_onClicked_closeOffButton()
{
    QWidget *m_pWindow = this->window();
    if(m_pWindow->isTopLevel())
    {
        m_pWindow->close();
    }
}

QString GuideWidget::system_name()
{
    QFile system (SYSTEM_FILE);
    if(!system.exists())
    {
        return "";
    }
    if (!system.open(QIODevice::ReadOnly))
    {
        return "";
    }
    QString str = system.readLine();
    QString name = str.section("\"",1,1);
    return name;
    //return "Ubuntu Kylin";
}

QString GuideWidget::system_version()
{
    QFile system (SYSTEM_FILE);
    if(!system.exists())
    {
        return "";
    }
    if (!system.open(QIODevice::ReadOnly))
    {
        return "";
    }

    QString version = "";
    QTextStream aStream(&system);
    //aStream.setAutoDetectUnicode(true);
    for(int i = 0;!aStream.atEnd();i++){
        if(0==i){
            QString str = system.readLine();
            continue;
        }
       QString str = system.readLine();
       version = str.section("\"",1,1);
       break;
    }
    system.close();

    return version;
}

QStringList GuideWidget::getDirAndPng()
{
    QStringList list;
    QString PathForSystem;

    PathForSystem = LOCAL_FILE_PATH_UBUNTUKYLIN;

    QDir path(PathForSystem);
    if(!path.exists())
    {
        qDebug()<< PathForSystem << "is not exists !!!";
    }
    QStringList dirList = path.entryList();
    dirList.sort();
    for(int i=0;i<dirList.size();++i)
    {
        QDir dirname(PathForSystem);
        if(dirList.at(i) == "."||dirList.at(i)=="..")
        {
            continue;
        }
        else
        {
            dirname.cd(dirList.at(i));
            qDebug() << dirname.entryList();
            QStringList namelist=dirname.entryList();
            QStringList Pngname = namelist.filter(".png");
            if(namelist.filter(".png").size() && namelist.filter("zh_CN").size() && namelist.filter("en_US").size())
                list << dirList.at(i)+"|"+Pngname.at(0);
        }
    }
    return list;
}

QString GuideWidget::JudgmentSystem()
{
    QFile system_file (OS_RELEASE);
    QFile system_file1 (LSB_RELEASE);
    QString value;
    if(system_file.exists()){
        QString SystemName = system_name();
        qDebug() << Q_FUNC_INFO << SystemName << SystemName.compare("kylin",Qt::CaseInsensitive)<< __LINE__;
        if(SystemName.compare("debian",Qt::CaseInsensitive) == 0){
            value = "Debian";
        }else if(SystemName.compare("ubuntu kylin",Qt::CaseInsensitive) == 0){
            value = "Ubuntu Kylin";
        }else if(SystemName.compare("kylin",Qt::CaseInsensitive) == 0){
            value = "Kylin";
        }else if(SystemName.compare("ubuntu",Qt::CaseInsensitive) == 0){
            if(QFileInfo("/usr/bin/ukui-panel").exists()){
                if(QFileInfo("/usr/bin/ukui-about").exists())
                    value = "Kylin";
                else
                    value = "Ubuntu Kylin";
            }else{
                value = "Ubuntu";
            }
        }else{
            value = "";
            return value;
        }
    }else{
        if(!system_file1.exists())
        {
            value = "";
            return value;
        }
        if (!system_file1.open(QIODevice::ReadOnly))
        {
            value = "";
            return value;
        }
        QString str = system_file1.readLine();
        QString name = str.section("=",-1);
        name = name.trimmed();
        qDebug() << Q_FUNC_INFO << name.compare("debian",Qt::CaseInsensitive) << __LINE__;
        if(name.compare("debian",Qt::CaseInsensitive) == 0){
            value = "Debian";
        }else if(name.compare("kylin",Qt::CaseInsensitive) == 0){
            value = "Kylin";
        }else if(name.compare("ubuntu",Qt::CaseInsensitive) == 0){
            qDebug() << __LINE__;
            if(QFileInfo("/usr/bin/ukui-panel").exists()){
                if(QFileInfo("/usr/bin/ukui-about").exists())
                    value = "Kylin";
                else
                    value = "Ubuntu Kylin";
            }else{
                value = "Ubuntu";
            }
        }else{
            value = "";
            return value;
        }
    }
    return value;
}

QString GuideWidget::js_getFileSystemInformation(QString filePath)
{
    if (filePath.isNull())
        return "";

     QFileInfo fileinfo(filePath);
    if (fileinfo.exists()) {
        return fileinfo.lastModified().toString("yyyy年MM月dd日");
    } else {
        qDebug() << Q_FUNC_INFO << "This File is not exist !!!";
        return "";
    }
}

bool GuideWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 添加搜索栏事件
    if (watched == search_Line) {
//        FocusOut
        if (event->type() == 9) {
            if (search_Line->text().isEmpty()) {
                QLocale localeNew;
                if(localeNew.language()==QLocale::Chinese)
                {
                    search_Line->setTextMargins(80,0,0,0);
                    m_queryIcon->setMargin(60);
                }else{
                    m_queryIcon->setMargin(46);
                    search_Line->setTextMargins(66,0,0,0);
                }
            }else{

            }
        }else if (event->type() == QEvent::MouseButtonPress) {
            search_Line->setTextMargins(26,0,0,0);
            m_queryIcon->setMargin(0);
        }
    }

   if (watched == m_pWebEngineView) {
        if (event->type() == QEvent::ShortcutOverride) {
            // 快捷键ctrl+E 搜索
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_E && keyEvent->modifiers() == Qt::ControlModifier) {
                search_Line->setFocus();
            }
            // 快捷键ctrl+A 全选
            if (keyEvent->key() == Qt::Key_A && keyEvent->modifiers() == Qt::ControlModifier) {
                m_pWebEngineView->page()->triggerAction(QWebEnginePage::SelectAll);
            }
        }

   }

    return false;
}

QStringList GuideWidget::js_getIntoFilename()
{
    QStringList test=getDirAndPng();
    return test;
}

QString GuideWidget::js_getIndexMdFilePath(QString appName)
{
    qDebug() << Q_FUNC_INFO << appName;
    QString IndexMdFilePath;
//    QString name = system_name();
//    if(name == "kylin")
//    {
//        IndexMdFilePath = LOCAL_FILE_PATH + appName + "/" +  gLang + "/index.md";
//    }
//    else if (name == "Ubuntu Kylin"){
    QString systemName = system_version();
//    if(strstr(systemName, "国防") == NULL || strstr(systemName, "GF") == NULL){
//        IndexMdFilePath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" +  gLang + "/index.md";
//    }

    IndexMdFilePath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" +  gLang + "/index.md";
    if(systemName.contains("国防",Qt::CaseSensitive) || systemName.contains("GF",Qt::CaseSensitive)){
//       if(systemName == QString::fromLocal8Bit("ukui")){
//            IndexMdFilePath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" +  gLang + "/index_GF.md";
//       }
       if(appName.compare(QString::fromLocal8Bit("ukui")) == 0){
           IndexMdFilePath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" +  gLang + "/index_GF.md";
       }
    }


//    }
    //QString IndexMdFilePath = LOCAL_FILE_PATH + appName + "/" +  gLang + "/index.md";
//    QLabel *label = this->findChild<QLabel *>("m_placeHoldLabel");
//    label->hide();
    QPushButton *button = this->findChild<QPushButton *>("backOffButton");
    button->show();
    return IndexMdFilePath;
}

QString GuideWidget::js_getIndexMdFilePathOther(QString appName)
{
    qDebug() << Q_FUNC_INFO << appName;
    QString IndexMdFilePath;
//    QString name = system_name();
//    if(name == "kylin")
//    {
//        IndexMdFilePath = LOCAL_FILE_PATH + appName + "/" +  gLang + "/index.md";
//    }
//    else if (name == "Ubuntu Kylin"){
    IndexMdFilePath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" +  gLang + "/index.md";
//    }
    return IndexMdFilePath;
}

QString GuideWidget::js_getIndexMdFileContent(QString IndexMdFilePath)
{
    qDebug() << Q_FUNC_INFO << IndexMdFilePath;
    QFile file (IndexMdFilePath);
    if(!file.exists())
        return "";
    if(!file.open(QIODevice::ReadOnly))
        return "";
    return file.readAll();
}

QString GuideWidget::js_getIndexMdFileTitle(QString IndexMdFilePath)
{
    qDebug() << Q_FUNC_INFO << IndexMdFilePath;
    QFile file (IndexMdFilePath);
    if(!file.exists())
        return "";
    if(!file.open(QIODevice::ReadOnly))
        return "";
    QString title = file.readLine();
    title = title.mid(1,title.length());
    return title.trimmed();
}

QStringList GuideWidget::js_getBigPngSize(QString filePath)
{
    QStringList stringlist;
    qDebug() << Q_FUNC_INFO << filePath<<"0-size:";
    QFile file (filePath);
    if(file.exists())
    {
        QImage image(filePath);
        stringlist.append(QString::number(image.width()));
        stringlist.append(QString::number(image.height()));
        qDebug() << Q_FUNC_INFO << filePath<<"1-size:"<<stringlist;
        return stringlist;
    };

    stringlist.append(QString::number(0));
    stringlist.append(QString::number(0));
    return stringlist;
}

void GuideWidget::initSettings()
{
    slot_getSarchKeyword();
}

void GuideWidget::tabletStatusChanged(bool tabletmode){
    QPushButton *button = this->findChild<QPushButton *>("maxOffButton");
    if(tabletmode){
        // 平板模式调整控件大小 
        // div.system-app div.system-app img 
        QString appSizeJs = QString("appSizeChange(\"%1\");").arg(tabletmode);
        m_pWebEngineView->page()->runJavaScript(appSizeJs);
        button->hide();
    }else{
        QString appSizeJs = QString("appSizeChange(\"%1\");").arg(tabletmode);
        m_pWebEngineView->page()->runJavaScript(appSizeJs);
        button->show();
    }
}

void GuideWidget::set_Cursor(QPoint &currentPoint)
{
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());

    int x = currentPoint.x();
    int y = currentPoint.y();

    if (tl.x()+Padding >= x && tl.x() <= x && tl.y()+Padding >= y && tl.y() <= y) {
        site_flag = left_top;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    else if (rb.x()-Padding<= x && rb.x() >= x && tl.y()+Padding >= y && tl.y() <= y) {
        site_flag = right_top;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    }
    else if (tl.x()+Padding >= x && tl.x() <= x && rb.y()-Padding <= y && rb.y() >= y) {
        site_flag = left_bottm;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    }
    else if (rb.x()-Padding <= x && rb.x() >= x && rb.y()-Padding <= y && rb.y() >= y) {
        site_flag = right_botm;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    else if (tl.x()+Padding >= x && tl.x() <= x && tl.y()+Padding <= y && rb.y()-Padding >= y) {
        site_flag = left;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }
    else if (rb.x()-Padding <= x && rb.x() >= x && rb.y()-Padding >= y && tl.y()+Padding <= y) {
        site_flag = right;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }
    else if (tl.x()+Padding <= x && rb.x()-Padding >= x && tl.y()+Padding >= y && tl.y() <= y) {
        site_flag = top;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }
    else if (tl.x()+Padding <= x && rb.x()-Padding >= x && rb.y()-Padding <= y && rb.y() >= y) {
        site_flag = bottom;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }
    else {
        site_flag = middle;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void GuideWidget::mousePressEvent(QMouseEvent *event)
{
    mouseinwidget = false; //避免在其他控件上按下鼠标移动出现位置不正确问题
    if(event->pos().y()>=3&&event->pos().y()<=30)
        mCanDrag = true;
    else
        mCanDrag = false;
    if (event->button() == Qt::LeftButton) {
        mouseinwidget = true;
        this->setCursor(QCursor(Qt::OpenHandCursor));
        dragPos = event->globalPos() - frameGeometry().topLeft();
    }
    event->accept();
}

//void GuideWidget::mouseMoveEvent(QMouseEvent *event)
//{
    // if(mCanDrag && mouseinwidget){
    //     qreal  dpiRatio = qApp->devicePixelRatio();
    //     if (QX11Info::isPlatformX11()) {
    //         Display *display = QX11Info::display();
    //         Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
    //         XEvent xEvent;
    //         const auto pos = QCursor::pos();

    //         memset(&xEvent, 0, sizeof(XEvent));
    //         xEvent.xclient.type = ClientMessage;
    //         xEvent.xclient.message_type = netMoveResize;
    //         xEvent.xclient.display = display;
    //         xEvent.xclient.window = this->winId();
    //         xEvent.xclient.format = 32;
    //         xEvent.xclient.data.l[0] = pos.x() * dpiRatio;
    //         xEvent.xclient.data.l[1] = pos.y() * dpiRatio;
    //         xEvent.xclient.data.l[2] = 8;
    //         xEvent.xclient.data.l[3] = Button1;
    //         xEvent.xclient.data.l[4] = 0;

    //         XUngrabPointer(display, CurrentTime);
    //         XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()),
    //                    False, SubstructureNotifyMask | SubstructureRedirectMask,
    //                    &xEvent);
    //         //XFlush(display);

    //         XEvent xevent;
    //         memset(&xevent, 0, sizeof(XEvent));

    //         xevent.type = ButtonRelease;
    //         xevent.xbutton.button = Button1;
    //         xevent.xbutton.window = this->winId();
    //         xevent.xbutton.x = event->pos().x() * dpiRatio;
    //         xevent.xbutton.y = event->pos().y() * dpiRatio;
    //         xevent.xbutton.x_root = pos.x() * dpiRatio;
    //         xevent.xbutton.y_root = pos.y() * dpiRatio;
    //         xevent.xbutton.display = display;

    //         XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xevent);
    //         XFlush(display);

    //         if (event->source() == Qt::MouseEventSynthesizedByQt) {
    //             if (!this->mouseGrabber()) {
    //                 this->grabMouse();
    //                 this->releaseMouse();
    //             }
    //         }

    //         mouseinwidget = false;
    //     } else {
    //         this->move((QCursor::pos() - dragPos) * dpiRatio);
    //     }
    // }
    // event->accept();
//}

void GuideWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons()&Qt::LeftButton)
    {
        QWidget *m_yWindow = this->window();
        if(m_yWindow->isTopLevel())
        {
            QPushButton *button1 = this->findChild<QPushButton *>("maxOffButton");
            m_yWindow->isMaximized() ? m_yWindow->showNormal() : m_yWindow->showMaximized();
            m_yWindow->isMaximized() ? button1->setIcon(QIcon::fromTheme("window-restore-symbolic")) :button1->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
            m_yWindow->isMaximized() ? button1->setToolTip(tr("Reduction")) : button1->setToolTip(tr("Maximize"));
            if(m_yWindow->isMaximized())
                windowsflag = false;
            else
                windowsflag = true;
        }
    }
    event->accept();
}

void GuideWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mouseinwidget = false;
        this->setCursor(Qt::ArrowCursor);
    }
    event->accept();
}

void GuideWidget::closeEvent(QCloseEvent *)
{
}

void GuideWidget::slot_getSarchKeyword()
{
    this->keywordMap.clear();
    // 获取当前语言环境
    QLocale localeNew;
    QString PathForSystem;
    PathForSystem = LOCAL_FILE_PATH_UBUNTUKYLIN;
    QDir path(PathForSystem);
    QString lang = "en_US";

    if(localeNew.language()==QLocale::Chinese)
    {
        lang = "zh_CN";
    }

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        lang = "zh_CN";
    } 

    if(!path.exists())
    {
        qDebug()<< PathForSystem << "is not exists !!!";
    }else{
        QStringList dirList = path.entryList();
        dirList.sort();
        for(int i=0;i<dirList.size();++i)
        {
            QDir dirname(PathForSystem);
            if(dirList.at(i) == "."||dirList.at(i)=="..")
            {
                continue;
            }
            else
            {
                dirname.cd(dirList.at(i));
                //进入文件夹读取标题
                QString IndexMdFilePath;
                IndexMdFilePath = js_getIndexMdFilePathOther(dirList.at(i));
                QString keyword = js_getIndexMdFileTitle(IndexMdFilePath);
                QString value = dirList.at(i);
                if ("银河麒麟最终用户使用许可协议和隐私政策声明" == keyword) {
                    keyword = "隐私与声明";
                }
                this->keywordMap.insert(make_pair(keyword.toStdString(),value.toStdString()));
            }
        }
        // 添加开机引导
        if(localeNew.language()==QLocale::Chinese)
        {
            this->keywordMap.insert(std::make_pair("新功能介绍","kylin-new-function-introduction"));
        }else{
            this->keywordMap.insert(std::make_pair("kylin-new-function-introduction","kylin-new-function-introduction"));
        }
    }
}

QString GuideWidget::converRGB2HexStr(QColor _rgb)
{
	QString redStr = QString("%1").arg(_rgb.red(),2,16,QChar('0'));
	QString greenStr = QString("%1").arg(_rgb.green(),2,16,QChar('0'));
	QString blueStr = QString("%1").arg(_rgb.blue(),2,16,QChar('0'));
    QString hexStr = "#"+redStr+greenStr+blueStr;
	return hexStr;
}

void GuideWidget::slot_showBackOffButton()
{
    qDebug() << Q_FUNC_INFO;
    QPushButton *button = this->findChild<QPushButton *>("backOffButton");
    qDebug() << button;
    button->show();
}
