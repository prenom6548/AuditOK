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
// #include <X11/Xlib.h>
#include <map>
#include <string>
#include <QProcess>
#include <QStringListModel>
#include <sys/utsname.h>
#include <kysdk/kysdk-system/libkysysinfo.h>

#include "guide-widget.h"
#include "common-tool/comm_func.h"
#include "xatom-helper.h"
#include "kaboutdialog.h"
#include "kwidget.h"
#include "kywebview.h"
#include "libkypackages.h"
#include "libkydatacollect.h"
#include "kmenubutton.h"

GuideWidget::GuideWidget(QWidget *parent) :QWidget(parent)
{
    this->isTopLevel();
    this->setMinimumSize(835,650);

    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        settings = new QGSettings("org.ukui.style");
        connect(settings,&QGSettings::changed,this,&GuideWidget::slot_SettingsChange);
    }

    char **path = kdk_package_get_standard_path("KYLIN_USER_GUIDE_DATA_PATH");
    size_t num = 0;
    if (path) {
        while (path[num]) {
            printf("sdk guide path = %s\n", path[num]);
            pathList.append(QString::fromUtf8(path[num]));
            num++;
        }
    } else {
        qDebug()<<"kdk::kdk_package_get_standard_path not get path";
    }

    // 添加窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    desktop = QApplication::desktop();
    QRect rect = QGuiApplication::screens().at(0)->availableGeometry();
    this->move((rect.bottomRight().x()-this->width())/2,(rect.bottomRight().y()-this->height())/2);

    initSettings();
    initUI();
    getDirAndPng();
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
        m_pWebView->page()->mainFrame()->evaluateJavaScript("Refresh_the_content_interface();");
        timer->stop();
    });

    qDebug() << e->oldSize() << e->size() << QGuiApplication::screens().at(0)->availableGeometry();
}

GuideWidget::~GuideWidget()
{

}

void GuideWidget::initUI()
{
    // QSslConfiguration sslConf=QSslConfiguration::defaultConfiguration();
    // sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    // QSslConfiguration::setDefaultConfiguration(sslConf);
//    m_pWebView = new QWebView(this);
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    m_pWebView = new KyWebView(this);
    m_pWebView->installEventFilter(this);
    qDebug()<< m_pWebView->width() << m_pWebView->height();

    this->setObjectName("m_yWidget");
    QPushButton *backOffButton = new QPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(backOffButton, "GuideWidget", "initUI", "User manual return to main interface button");
    m_iconBar = new KIconBar(this);
    m_iconBar->setIcon("kylin-user-guide");
    m_iconBar->setWidgetName(tr("Manual"));
    m_iconBar->setMinimumWidth(140);

    m_windowButtonBar = new KWindowButtonBar(this);

    search_Line = new KSearchLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(search_Line, "GuideWidget", "initUI", "User Manual Search Bar");
    search_Line->setFocusPolicy(Qt::ClickFocus);
    search_Line->setClearButtonEnabled(true);
    search_Line->installEventFilter(this);
    search_Line->completer()->setFilterMode(Qt::MatchContains);
    // 占位防止偏移
    QLabel *m_placeHoldLabel = new QLabel(this);
    m_placeHoldLabel->setFixedSize(30,30);

    search_Line->setMinimumSize(240,36);
    search_Line->setMaximumSize(240,36);
    search_Line->setAlignment(Qt::AlignVCenter);
    qDebug() << search_Line->alignment();
    backOffButton->setObjectName("backOffButton");
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

    m_windowButtonBar->menuButton()->menu()->removeAction(m_windowButtonBar->menuButton()->settingAction());
    m_windowButtonBar->menuButton()->menu()->removeAction(m_windowButtonBar->menuButton()->themeAction());
    m_windowButtonBar->menuButton()->menu()->removeAction(m_windowButtonBar->menuButton()->assistAction());
    m_windowButtonBar->setMaximumButtonState(MaximumButtonState::Maximum);

    connect(backOffButton,SIGNAL(released()),this,SLOT(slot_backOffButton()));
    connect(m_windowButtonBar->closeButton(),SIGNAL(released()),this,SLOT(slot_onClicked_closeOffButton()));
    connect(m_windowButtonBar->minimumButton(),SIGNAL(released()),this,SLOT(slot_onClicked_minOffButton()));
    connect(m_windowButtonBar->maximumButton(),SIGNAL(released()),this,SLOT(slot_onClicked_maxOffButton()));
    connect(m_iconBar,SIGNAL(doubleClick()),this,SLOT(slot_onClicked_maxOffButton()));
    connect(m_windowButtonBar->menuButton()->quitAction(), &QAction::triggered, this, [=] () {
        slot_onClicked_closeOffButton();
    });
    connect(m_windowButtonBar->menuButton()->aboutAction(), &QAction::triggered, this, [=] () {
        slot_AboutActions();
    });
    connect(search_Line,SIGNAL(returnPressed()),this,SLOT(slot_searchLine()));

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    QGridLayout *widget_layout = new QGridLayout(this);
    widget_layout->setHorizontalSpacing(4);
    widget_layout->addWidget(m_iconBar,0,0,1,1);
    widget_layout->addWidget(backOffButton,0,22,1,1);
    widget_layout->addWidget(search_Line,0,39,1,30,Qt::AlignCenter);
    widget_layout->addWidget(m_placeHoldLabel,0,80,1,1);
    widget_layout->addWidget(m_windowButtonBar,0,102,1,4);
    widget_layout->setColumnStretch(55,2);
    widget_layout->addWidget(m_pWebView,1,0,1,106);


//    QString name = system_name();
    QLocale localeNew;
    QStringList word_List;
    if(localeNew.language()==QLocale::Chinese || localeNew.language()==QLocale::Tibetan ||  localeNew.language() != QLocale::English)
    {
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
            m_pWebView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"sw/index-ubuntukylin_black_sw.html"));
        }else{
            m_pWebView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"sw/index-ubuntukylin_sw.html"));
        }
        // search_Line->setPlaceholderText("搜索");
        for( std::map<std::string,std::string>::iterator iter = this->keywordMap.begin();iter != this->keywordMap.end(); iter++){
           word_List.append(QString::fromStdString(iter->first));
        }
            // m_queryIcon->setMargin(60);
            search_Line->setTextMargins(80,0,0,0);
    }else{
        gLang = "en_US";
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
            m_pWebView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"sw/index-ubuntukylin_en_US_black_sw.html"));
        }else{
            m_pWebView->load(QUrl(QString(LOCAL_URL_PATH_UBUNTUKYLIN)+"sw/index-ubuntukylin_en_US_sw.html"));
        }
        
        //英文情况下修改搜索的placeholder
        // search_Line->setPlaceholderText("Search");
        for( std::map<std::string,std::string>::iterator iter = this->keywordMap.begin();iter != this->keywordMap.end(); iter++){
           word_List.append(QString::fromStdString(iter->first));
        }
            // m_queryIcon->setMargin(46);
            search_Line->setTextMargins(66,0,0,0);
    }
    QCompleter *m_completer_autoAdd = new QCompleter(word_List);
    m_completer_autoAdd->setFilterMode(Qt::MatchContains);// 搜索候选框内容匹配
    m_completer_autoAdd->setCaseSensitivity(Qt::CaseInsensitive); // 搜索候选忽略大小写
    m_completerModel = new QStringListModel(word_List);
    search_Line->completer()->setModel(m_completerModel);
    // sdk搜索框需要重载一下样式
    search_Line->reloadStyle();
    //search_Line->setCompleter(m_completer_autoAdd);
    connect(search_Line->completer(),SIGNAL(activated(const QString&)),this,SLOT(slot_onClicked_completerAutoAdd(const QString&)));
//    connect(m_completer_autoAdd, SIGNAL(activated(const QString&)), search_Line->clear, Qt::QueuedConnection)

    //m_pWebView->load(QUrl(QString(LOCAL_URL_PATH)+"index.html"));
    m_pWebView->setStyleSheet("border-radius:10px");
    m_pWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);//外链
    m_pWebView->settings()->setObjectCacheCapacities(0,0,0);
//    m_pWebView->settings()->setAttribute(QWebSettings::WebSecurityEnabled, false);//关闭浏览器安全
//    m_pWebView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);//开发模式  关闭开发模式，检查功能对普通用户无太大意义，且怀疑安全会杀掉用户手册
    m_pWebView->settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
    m_pWebView->settings()->setAttribute(QWebSettings::JavaEnabled,true);
    m_pWebView->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    m_pWebView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard,true);
    m_pWebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    m_pWebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);//设置能访问本地文件
    m_pWebView->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    m_pWebView->settings()->setAttribute(QWebSettings::AutoLoadImages,true);
    m_pWebView->setAttribute(Qt::WA_InputMethodEnabled,false);
//    m_pWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);//关闭插件，开启插件可能会在宝德鲲鹏机器上崩溃#69666
    // m_pWebView->setContextMenuPolicy(Qt::NoContextMenu); //关闭右键菜单  

    QObject::connect(m_pWebView,SIGNAL(loadFinished(bool)),this,SLOT(slot_loadFinished(bool)));
    QObject::connect(m_pWebView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(slot_javaScriptFromWinObject()));
    QObject::connect(m_pWebView->page(),SIGNAL(linkClicked(QUrl)),this,SLOT(slot_webGoto(QUrl)));
    widget_layout->setContentsMargins(0, 0, 0, 0);
    widget_layout->setVerticalSpacing(4);
    main_layout->addLayout(widget_layout);
    main_layout->setContentsMargins(2, 2, 2, 10);
    this->setLayout(main_layout);
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

void GuideWidget::slot_AboutActions()
{
    if (!about_widget) {
        about_widget = new KAboutDialog(this);
    }
    about_widget->setAppIcon(QIcon::fromTheme("kylin-user-guide"));
    about_widget->setAppName(tr("Manual"));
    about_widget->setAppVersion(tr("Version：") + get_current_version());
    about_widget->setModal(true);
    about_widget->exec();
}

QString GuideWidget::get_current_version()
{
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = "1.0.1-1kylin1~43";
    pp = popen("dpkg -l  kylin-user-guide | grep  kylin-user-guide", "r");
    if (NULL == pp)
        return version;
    while ((read = getline(&line, &len, pp)) != -1) {
        q = strrchr(line, '\n');
        *q = '\0';
        QString content = line;
        QStringList list = content.split(" ");
        list.removeAll("");
        if (list.size() >= 3)
            version = list.at(2);
    }
    free(line);
    pclose(pp);
    return version;
}

void GuideWidget::slot_SettingsChange(const QString &key)
{
    qDebug() << Q_FUNC_INFO << key << settings->get("styleName").toString();
    if(key == "styleName"){
        QPalette palette;
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
            palette.setBrush(QPalette::Background, QBrush(QColor("#1d1d1d")));
            palette.setBrush(QPalette::Text, QBrush(QColor(Qt::white)));
            this->setPalette(palette);

            m_pWebView->page()->mainFrame()->evaluateJavaScript("SwitchStyle(2);");
            m_pWebView->page()->mainFrame()->evaluateJavaScript("changebuttonicon(2);");
        }else{
            palette.setBrush(QPalette::Background, QBrush(QColor(Qt::white)));
            palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
            this->setPalette(palette);

            m_pWebView->page()->mainFrame()->evaluateJavaScript("SwitchStyle(1);");
            m_pWebView->page()->mainFrame()->evaluateJavaScript("changebuttonicon(1);");
        }
    }
    if ("iconThemeName" == key) {
        QString iconTheme = settings->get("iconThemeName").toString();
        QString themeJs = QString("switchTheme(\"%1\");").arg(iconTheme);
//         m_pWebView->page()->mainFrame()->evaluateJavaScript("alert()");
         if(!iconTheme.isNull()){
             m_pWebView->page()->mainFrame()->evaluateJavaScript(themeJs);
         }
    }

    if("systemFont" == key || "systemFontSize" == key ){
        QFont font = this->font();
        for (auto widget: qApp->allWidgets()) {
            qDebug() << Q_FUNC_INFO;
            widget->setFont(font);
        }
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
    m_pWebView->page()->mainFrame()->evaluateJavaScript(highlightJs);
}

QString GuideWidget::js_getIconThemeName(){
    QString iconTheme = settings->get("iconThemeName").toString();
    QString themeJs = QString("switchTheme(\"%1\");").arg(iconTheme);
    if(!iconTheme.isNull()){
        return iconTheme;
    }
    return "";
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

    const QVariant& result = m_pWebView->page()->mainFrame()->evaluateJavaScript("typeof qt_jumpApp !== 'undefined'");
    qDebug() << " load result = " << result;
    if (result.toBool()) {
        // JavaScript函数已就绪，执行跳转
        QString cmd = QString("qt_jumpApp(\"%1\");").arg(appName);
        m_pWebView->page()->mainFrame()->evaluateJavaScript(cmd);
    } else {
        QTimer::singleShot(50, [this, appName]() {
            jump_app(appName);
        });
    }
}

void GuideWidget::js_buriedPoint(QString appName)
{
    qDebug() << Q_FUNC_INFO;
    // 埋点数据

    QString app = "kylin-user-guide";
    char *appCstr = strdup(app.toUtf8().constData());

    QString action = "user_guide_search_event";
    char *actionCstr = strdup(action.toUtf8().constData());

    KCustomProperty property[1];
    QString appKey= "application";
    char *keyCstr = strdup(appKey.toUtf8().constData());
    property[0].key = keyCstr;
    char *valueCstr = strdup(appName.toUtf8().constData());
    property[0].value = valueCstr;

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    kdk_dia_append_custom_property(node,property,1);
    kdk_dia_upload_default(node,actionCstr,appCstr);
    kdk_dia_data_free(node);
}

void GuideWidget::js_executeCommand(QString cmd ,QStringList args)
{
    qDebug() << Q_FUNC_INFO << cmd;
    qDebug() << Q_FUNC_INFO << args;
    QProcess::startDetached(cmd,args);
}

void GuideWidget::js_repaint()
{
    qDebug() << Q_FUNC_INFO;
    repaint();
}

void GuideWidget::slot_javaScriptFromWinObject()
{
    qDebug() << Q_FUNC_INFO;
    m_pWebView->page()->mainFrame()->addToJavaScriptWindowObject("guideWebkit",this);
//    m_pWebView->page()->mainFrame()->evaluateJavaScript("mywebkit.sig_backOff2js.connect(goBackMainUI());");
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
    QString appName = search_Line->text();
    std::string keyword = "";

    //fprintf(stderr,"text = %s\n",appName.toStdString().c_str());

    // 判断是否为能够跳转的页面
    if("" != appName ){
        for( std::map<std::string,std::string>::iterator iter = this->keywordMap.begin();iter != this->keywordMap.end(); iter++){
            // 判断是否为英文环境下
            QString qm_name = QLocale::system().name();
            if(localeNew.language()==QLocale::Chinese || "bo_CN" == qm_name)
            {
                // 中文
                if(iter->first==appName.trimmed().toStdString()){
                    keyword = iter->second;
                    break;
                }
            }else{
                // 英文
                if(iter->first==appName.trimmed().toStdString()){
                    keyword = iter->second;
                    break;
                }
            }

        }    
        if("" != keyword){
            jump_app(QString::fromStdString(keyword));
        }
    }

    search_Line->clear();
//    std::string str = keyWord.toStdString();
//    const char* p =str.c_str();
//    fprintf(stderr," %s ",str.c_str());
//    qWarning() << s;
//    m_pWebView->page()->mainFrame()->evaluateJavaScript("goBackMainUI_ubuntu();");
//    QPushButton *button = this->findChild<QPushButton *>("backOffButton");
//    button->hide();
}

void GuideWidget::slot_backOffButton()
{
    qDebug() << Q_FUNC_INFO;
     // 判断当前主题模式，返回主页时选择对应主题模式的页面
    QString style = "1";
    if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
        style = "2";
    }
    m_pWebView->page()->mainFrame()->evaluateJavaScript("goBackMainUI_ubuntu("+style+");");
    // m_pWebView->page()->mainFrame()->evaluateJavaScript("goBackMainUI_ubuntu();");
    QPushButton *button = this->findChild<QPushButton *>("backOffButton");
    button->hide();
}

void GuideWidget::slot_loadFinished(bool f)
{
    qDebug() << Q_FUNC_INFO << f ;
    QTimer::singleShot(10,this,[=]{
        QPalette palette;
        if(settings->get("styleName").toString() == "ukui-black"  || settings->get("styleName").toString() == "ukui-dark"){
            palette.setBrush(QPalette::Background, QBrush(QColor("#1d1d1d")));
            palette.setBrush(QPalette::Text, QBrush(QColor(Qt::white)));
            this->setPalette(palette);
            m_pWebView->page()->mainFrame()->evaluateJavaScript("SwitchStyle(2);");
        }else{
            palette.setBrush(QPalette::Background, QBrush(QColor(Qt::white)));
            palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
            this->setPalette(palette);
            m_pWebView->page()->mainFrame()->evaluateJavaScript("SwitchStyle(1);");
        }
        QString iconTheme = settings->get("iconThemeName").toString();
        QString themeJs = QString("switchTheme(\"%1\");").arg(iconTheme);
//         m_pWebView->page()->mainFrame()->evaluateJavaScript("alert()");
        if(!iconTheme.isNull()){
            m_pWebView->page()->mainFrame()->evaluateJavaScript(themeJs);
        }
    });
    // 调用函数自动添加搜索参数
    slot_getSarchKeyword();
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
        m_pWindow->isMaximized() ? m_pWindow->showNormal() : m_pWindow->showMaximized();
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
//    QString str = system();
//    QString version = "";
//    int lineNum = 0;
//    while (system.atEnd() == false) {
//        if(0==lineNum){
//            lineNum++;
//            continue;
//        }
//       QString str = system.readLine();
//       version = str.section("\"",1,5);
//       break;
//    }

    return version;
//    return "Ubuntu Kylin";
}

bool GuideWidget::isOpenkylin()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

QStringList GuideWidget::getDirAndPng()
{
    QStringList resultList;
    QStringList validPaths;

    // 处理路径列表（外部变量 pathList）
    if (pathList.isEmpty()) {
        validPaths << LOCAL_FILE_PATH_UBUNTUKYLIN; // 默认路径
    } else {
        // 标准 for 循环遍历外部路径列表
        for (int i = 0; i < pathList.size(); ++i) {
            QDir currentDir(pathList.at(i));
            if (currentDir.exists()) {
                validPaths << pathList.at(i);
            }
        }
    }

    // 遍历所有有效路径
    for (int pathIdx = 0; pathIdx < validPaths.size(); ++pathIdx) {
        QDir parentDir(validPaths[pathIdx]);

        // 获取子目录列表（自动过滤 . 和 ..）
        QStringList subDirs = parentDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        subDirs.sort();

        // 遍历子目录
        for (int dirIdx = 0; dirIdx < subDirs.size(); ++dirIdx) {
            QDir subDir(parentDir);
            if (!subDir.cd(subDirs.at(dirIdx))) continue;

            // 检查包含 zh_CN 和 en_US 子目录
            if (!subDir.exists("zh_CN") || !subDir.exists("en_US")) continue;

            // 获取 PNG 文件列表
            QStringList pngFiles = subDir.entryList(QStringList() << "*.png", QDir::Files);

            if (!pngFiles.isEmpty()) {
                // 组合目录名和第一个 PNG 文件名
                resultList << QString("%1|%2").arg(subDirs.at(dirIdx), pngFiles.first());
            }
        }
    }

    return resultList;
}

bool GuideWidget::js_judgmentHW()
{
    QFile file("/proc/hardware"); 
    if(file.exists()){
        return true;
    }else{
        return false;
    }
	// Close the file 
	file.close(); 
    return false;
}


bool GuideWidget::js_judgmentSW()
{
    struct utsname sysinfo;
    if (uname(&sysinfo) == 0) {
        if (strstr(sysinfo.machine, "sw") != NULL) {
            return true;
        } else {    
            return false;
        }
    } else {
        perror("uname");
    }
    return false;
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

    QLocale localeNew;
    QFileInfo fileinfo(filePath);
    if (fileinfo.exists()) {
        if(localeNew.language()==QLocale::Chinese || localeNew.language()==QLocale::Tibetan){
            return fileinfo.lastModified().toString("yyyy年MM月dd日");
        }else{
            return fileinfo.lastModified().toString("yyyy-MM-dd");
        }
    } else {
        qDebug() << Q_FUNC_INFO << "This File is not exist !!!";
        return "";
    }
}

bool GuideWidget::eventFilter(QObject *watched, QEvent *event)
{
//    bool m_isSearching;
//    m_isSearching = false;

    // 添加搜索栏事件
    if (watched == search_Line) {
//        FocusOut
        if (event->type() == 9) {
            if (search_Line->text().isEmpty()) {
                QLocale localeNew;
                if(localeNew.language()==QLocale::Chinese)
                {
                    search_Line->setTextMargins(80,0,0,0);
                    // m_queryIcon->setMargin(60);
                }else{
                    // m_queryIcon->setMargin(46);
                    search_Line->setTextMargins(66,0,0,0);
                }


                // m_animation->stop();
                // m_animation->setStartValue(QRect((m_searchWidget->width()-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                //                                  m_queryIcon->width()+m_queryText->width()+30,(m_searchWidget->height()+32)/2));
                // m_animation->setEndValue(QRect(8, 0, m_queryIcon->width() + 5,(m_searchWidget->height()+32)/2));
                // m_animation->setEasingCurve(QEasingCurve::OutQuad);
                // m_animation->start();
                // m_searchWidget->setTextMargins(30, 1, 0, 1);
            }else{

            }
//            m_isSearching = true;

        }else if (event->type() == QEvent::MouseButtonPress) {
            search_Line->setTextMargins(26,0,0,0);
            // m_queryIcon->setMargin(0);
//            search_Line->clear();
//            if (search_Line->text().isEmpty()) {
//                 search_Line->addAction(QIcon::fromTheme("search-symbolic"),QLineEdit::LeadingPosition);
//            }
//            m_isSearching=false;
        }
    }

    if (watched == m_pWebView) {
        if (event->type() == QEVENT_KEYPRESS) {
            // 快捷键ctrl+E 搜索
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_E && keyEvent->modifiers() == Qt::ControlModifier) {
                search_Line->setFocus();
            }
            // 快捷键ctrl+A 全选
            if (keyEvent->key() == Qt::Key_A && keyEvent->modifiers() == Qt::ControlModifier) {
                m_pWebView->page()->triggerAction(QWebPage::SelectAll);
            }
        }
    }

    if (watched == m_pWebView && event->type() == QEvent::Resize) {
        QPainterPath path;
        path.addRoundedRect(0, 0, this->width()-3, this->height()-47, 5, 5);
        m_pWebView->setMask(path.toFillPolygon().toPolygon());
    }

    if (watched == m_pWebView && event->type() == QEvent::Leave) {
        return true;
    }

//    if (watched == m_pWebView && event->type() == QEvent::Leave) {
//        return true;
//    }

//    if (event->type() == QEvent::MouseButtonPress) {
//        QMouseEvent *mEvent = static_cast<QMouseEvent*>(event);
//        QWidget *searchParentWid = static_cast<QWidget*>(search_Line);
//        QPoint  searchPoint      = searchParentWid->mapFromGlobal(mEvent->globalPos());
////        //qDebug()<<m_searchWidget->geometry()<<  mWindowGlobalPoint << mouseGlobalPoint << tPoint;
//        if (!search_Line->geometry().contains(searchPoint)) {
////            if (search_Line->text().isEmpty()) {
//            search_Line->setFocus();
//                search_Line->clearFocus();
////            }
//        }
//    }
    if (event->type() == QEvent::WindowStateChange) {
        if (watched == this) {
            if (this->windowState() == Qt::WindowMaximized) {
                m_windowButtonBar->setMaximumButtonState(MaximumButtonState::Restore);
            } else {
                m_windowButtonBar->setMaximumButtonState(MaximumButtonState::Maximum);
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

    if (!isOpenkylin() && appName == "kylin-os") {
        IndexMdFilePath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" +  gLang + "/index_openkylin.md";
    }


//    }
    //QString IndexMdFilePath = LOCAL_FILE_PATH + appName + "/" +  gLang + "/index.md";
//    QLabel *label = this->findChild<QLabel *>("m_placeHoldLabel");
//    label->hide();

    return IndexMdFilePath;
}

QString GuideWidget::js_getIndexMdFilePathOther(QString appName)
{
    qDebug() << Q_FUNC_INFO << appName;
    // 若 pathList 为空，使用默认路径
    if (pathList.isEmpty()) {
        QString defaultPath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" + gLang + "/index.md";
        return defaultPath;
    }

    // 遍历 pathList，生成并验证路径
    for (QStringList::const_iterator it = pathList.begin(); it != pathList.end(); ++it) {
        QString fullPath;
        if (isOpenkylin() && appName == "kylin-os") {
            fullPath = *it + "/" + appName + "/" + gLang +  "/index_openkylin.md";
        } else {
            fullPath = *it + "/" + appName + "/" + gLang + "/index.md";
        }
        if (QFile::exists(fullPath)) {
            return fullPath;
        }
    }

    return LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" + gLang + "/index.md";
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
        dragPos = event->globalPos() - frameGeometry().topLeft();
    }
    event->accept();
}

void GuideWidget::mouseMoveEvent(QMouseEvent *event)
{
//    QPoint gloPoint = event->globalPos();
//    QRect rect = this->rect();
//    QPoint tl = mapToGlobal(rect.topLeft());
//    QPoint rb = mapToGlobal(rect.bottomRight());
//    if(windowsflag){
//        if (!mouseinwidget)
//        {
//            set_Cursor(gloPoint);
//        }
//        else
//        {
//            if (site_flag != middle )
//            {
//                QRect rMove(tl, rb);
//                switch (site_flag)
//                {
//                    case left_top:
//                        if(rb.x()-gloPoint.x() >= this->minimumWidth())
//                            rMove.setLeft(gloPoint.x());

//                        if(rb.y()-gloPoint.y() >= this->minimumHeight())
//                            rMove.setTop(gloPoint.y());
//                        break;
//                    case right_top:
//                        if(gloPoint.x()-tl.x() >= this->minimumWidth())
//                            rMove.setRight(gloPoint.x());

//                        if(rb.y()-gloPoint.y() >= this->minimumHeight())
//                            rMove.setTop(gloPoint.y());
//                        break;
//                    case left_bottm:
//                        if(rb.x()-gloPoint.x() >= this->minimumWidth())
//                            rMove.setLeft(gloPoint.x());

//                        if(gloPoint.y()-tl.y() >= this->minimumHeight())
//                            rMove.setBottom(gloPoint.y());
//                        break;
//                    case right_botm:
//                        if(gloPoint.x()-tl.x() >= this->minimumWidth())
//                            rMove.setRight(gloPoint.x());

//                        if(gloPoint.y()-tl.y() >= this->minimumHeight())
//                            rMove.setBottom(gloPoint.y());
//                        break;
//                    case top:
//                        if(rb.y()-gloPoint.y() >= this->minimumHeight())
//                            rMove.setTop(gloPoint.y());
//                        break;
//                    case right:
//                        if(gloPoint.x()-tl.y() >= this->minimumWidth())
//                            rMove.setRight(gloPoint.x());
//                        break;
//                    case bottom:
//                        if(gloPoint.y()-tl.y() >= this->minimumHeight())
//                            rMove.setBottom(gloPoint.y());
//                        break;
//                    case left:
//                        if(rb.x()-gloPoint.x() >= this->minimumWidth())
//                            rMove.setLeft(gloPoint.x());
//                        break;
//                    default:
//                        break;
//                }
//                this->setGeometry(rMove);
//                QTimer* timer = new QTimer();
//                timer->start(10);           //以毫秒为单位
//                connect(timer,&QTimer::timeout,[=]{
//                    m_pWebView->page()->mainFrame()->evaluateJavaScript("Refresh_the_content_interface();");
//                    timer->stop();
//                });

//            }
//        }
//    }

    if(mCanDrag && mouseinwidget){
        qreal  dpiRatio = qApp->devicePixelRatio();
        if (QX11Info::isPlatformX11()) {
            Display *display = QX11Info::display();
            Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
            XEvent xEvent;
            const auto pos = QCursor::pos();

            memset(&xEvent, 0, sizeof(XEvent));
            xEvent.xclient.type = ClientMessage;
            xEvent.xclient.message_type = netMoveResize;
            xEvent.xclient.display = display;
            xEvent.xclient.window = this->winId();
            xEvent.xclient.format = 32;
            xEvent.xclient.data.l[0] = pos.x() * dpiRatio;
            xEvent.xclient.data.l[1] = pos.y() * dpiRatio;
            xEvent.xclient.data.l[2] = 8;
            xEvent.xclient.data.l[3] = Button1;
            xEvent.xclient.data.l[4] = 0;

            XUngrabPointer(display, CurrentTime);
            XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()),
                       False, SubstructureNotifyMask | SubstructureRedirectMask,
                       &xEvent);
            //XFlush(display);

            XEvent xevent;
            memset(&xevent, 0, sizeof(XEvent));

            xevent.type = ButtonRelease;
            xevent.xbutton.button = Button1;
            xevent.xbutton.window = this->winId();
            xevent.xbutton.x = event->pos().x() * dpiRatio;
            xevent.xbutton.y = event->pos().y() * dpiRatio;
            xevent.xbutton.x_root = pos.x() * dpiRatio;
            xevent.xbutton.y_root = pos.y() * dpiRatio;
            xevent.xbutton.display = display;

            XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xevent);
            XFlush(display);

            if (event->source() == Qt::MouseEventSynthesizedByQt) {
                if (!this->mouseGrabber()) {
                    this->grabMouse();
                    this->releaseMouse();
                }
            }

            mouseinwidget = false;
        } else {
            // wlcom环境无需手动处理鼠标拖拽窗口位置移动
//            this->move((QCursor::pos() - dragPos) * dpiRatio);
        }
    }
    event->accept();
}

void GuideWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons()&Qt::LeftButton)
    {
        QWidget *m_yWindow = this->window();
        if(m_yWindow->isTopLevel())
        {
            m_yWindow->isMaximized() ? m_yWindow->showNormal() : m_yWindow->showMaximized();
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
//    QApplication* app;
//    app->quit();
}

void GuideWidget::slot_getSarchKeyword()
{
    this->keywordMap.clear();
    QLocale localeNew;

    // ========== 新增路径处理逻辑 ==========
    QStringList validPaths;
    if (pathList.isEmpty()) {
        // 默认路径处理
        QDir defaultDir(LOCAL_FILE_PATH_UBUNTUKYLIN);
        if (defaultDir.exists()) {
            validPaths << LOCAL_FILE_PATH_UBUNTUKYLIN;
        }
    } else {
        // 使用标准for循环遍历路径列表
        for (int i = 0; i < pathList.size(); ++i) {
            QDir currentDir(pathList.at(i));
            if (currentDir.exists()) {
                validPaths << pathList.at(i);
            }
        }
    }

    // ========== 目录遍历逻辑 ==========
    for (int pathIdx = 0; pathIdx < validPaths.size(); ++pathIdx) {
        QDir path(validPaths[pathIdx]);
        QStringList dirList = path.entryList();
        dirList.sort();

        for (int i = 0; i < dirList.size(); ++i) {
            if (dirList.at(i) == "." || dirList.at(i) == "..")
                continue;

            QDir subDir(path);
            if (subDir.cd(dirList.at(i))) {
                // 生成索引文件路径（复用原有函数）
                QString IndexMdFilePath = js_getIndexMdFilePathOther(dirList.at(i));
                if (QFile::exists(IndexMdFilePath)) { // 新增存在性检查
                    QString keyword = js_getIndexMdFileTitle(IndexMdFilePath);
                    QString value = dirList.at(i);
                    // 特殊关键字替换
                    if ("银河麒麟最终用户使用许可协议和隐私政策声明" == keyword) {
                        keyword = "隐私与声明";
                    }
                    this->keywordMap.insert(std::make_pair(keyword.toStdString(),
                                                           value.toStdString()));
                }
            }
        }
        // 添加开机引导
        // if(localeNew.language()==QLocale::Chinese)
        // {
        //     this->keywordMap.insert(std::make_pair("新功能介绍","kylin-new-function-introduction"));
        // }
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

void GuideWidget::js_showBackbtn()
{
    QPushButton *button = this->findChild<QPushButton *>("backOffButton");
    button->show();
}
