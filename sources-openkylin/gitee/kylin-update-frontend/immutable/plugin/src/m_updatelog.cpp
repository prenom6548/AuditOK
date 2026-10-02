#include "m_updatelog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include "xatom-helper.h"

const QString FIND_UPDATE_LABLE_TYPE = "FIND_UPDATE_LABLE_TYPE";//历史更新模块标签
const QString FIND_DES_LABLE_TYPE = "FIND_DES_LABLE_TYPE";//历史更新描述模块标签
const QString FIND_CLG_LABLE_TYPE = "FIND_CLG_LABLE_TYPE";//历史更新日志模块标签
const int WIDTH = 880;//窗口宽度
const int HEIGHT = 610 -40;//窗口高度   注：QFram实际高度比setFixedSize指定的高度高40像素，故减去40
const int LINE_SPACING = 2;//行间距
const int TOP_MARGIN = 18;//上边距
const int BOTTOM_MARGIN = 24;//下边距
const int LEFT_MARGIN = 32;//左（右）边距

const int LIST_LEFT = 8;//列表对于其背景，左侧边距
const int LIST_TOP = 8;//列表对于其背景，顶（底）部边距
const int LIST_BACKGROUND_WIDTH = 320 + 6;//列表背景宽度
const int SLIDER_WIDTH = 6;//滑块宽度
//const int LIST_WIDTH = LIST_BACKGROUND_WIDTH - LIST_LEFT*2 + SLIDER_WIDTH;//列表宽度
//const int LIST_HEIGHT = 524 - LIST_TOP*2;//列表高度

const int TEXT_TAB_LEFT = 16;//更新详情lable对于其背景，左侧边距
const int TEXT_TAB_RIGHT = 2;//更新详情lable对于其背景，左侧边距
const int TEXT_DSC_LEFT_SPACING = 5;//QTextEdit左侧有边距
const int TEXT_TAB_TOP = 17;//更新详情lable对于其背景，顶部边距
const int TEXT_TAB_SPACING = 18;//更新详情lable对于内容框的间距

const QString OBJECT_NAME = "OBJECT_NAME";

m_updatelog * m_updatelog::m_instance(nullptr);

m_updatelog::m_updatelog(QWidget* parent) : QDialog(parent)
{
    //日志数量
    lognum=0;
    this->setWindowModality(Qt::WindowModal);
    initGsettings();//初始化Gsettings
    initUI();//初始化UI
    dynamicLoadingInit();//动态加载
    updatesql();//更新列表
    if(lognum!=0)
    {
        nocontent->hide();
        content->show();
    }
//    defaultItem();//设置默认选中
    //监听更新完成信号
//    UpdateDbus *uddbus = UpdateDbus::getInstance();
//    connect(uddbus->interface,SIGNAL(update_sqlite_signal(QString,QString)),this,SLOT(historyUpdateNow(QString,QString)));

}

QString m_updatelog::setDefaultDescription(QString str)
{
    if(str == "")
        str = tr("No content.");  //暂无内容
    return str;
}

m_updatelog * m_updatelog::GetInstance(QWidget *parent)
{
    if(m_instance==nullptr)
    {
        m_instance = new m_updatelog(parent);
        return m_instance;
    }
    if(m_instance->isHidden())
    {
        m_instance->deleteLater();
        m_instance = new m_updatelog(parent);
    }
    return m_instance;
}

void m_updatelog::closeUpdateLog()
{
    m_instance->close();
    m_instance->deleteLater();
}

void m_updatelog::initUI()
{
    kdk::UkuiStyleHelper::self()->removeHeader(this);//移除控制面板标题栏
    /* 初始化窗口属性 */
    this->setFixedSize(WIDTH,HEIGHT + 40);//（880，610）
    this->setObjectName("FIND_UPDATE_LABLE_TYPE");

    /* 实例化控件 */
    /* 实例化标题栏 */
    updateTitleWidget();

    QFrame *listBackground = new QFrame;
    listBackground->setFrameStyle(QFrame::Box);
    listBackground->setFixedWidth(326);
    /*mainlistwidget 是frame listbackground 的内部内容*/
    mainListwidget  = new QListWidget;
//    QPalette palette = mainListwidget->palette();
//    palette.setBrush(QPalette::Base, QColor (0, 0 , 0, 0));
//    QPalette palette = qApp->palette();
//    QColor listcolor = palette.button().color();
//    palette.setBrush(QPalette::Base,listcolor);
//    mainListwidget->setPalette(palette);
    mainListwidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);  /* 滑块平滑滚动 */
    mainListwidget->verticalScrollBar()->setProperty("drawScrollBarGroove" , false);
    mainListwidget->setSpacing(2);

    QFrame *desBackground = new QFrame;
    desBackground->setFrameStyle(QFrame::Box);

    /* 更新详情 */
    updateDesTab = new QLabel;
   /*QFont font;
    font.setBold(true);
    updateDesTab->setFont(font);
    updateDesTab->setWordWrap(true);
    updateDesTab->setText(tr("No Contents"));*/

    /* 描述文本框 */
    des = new QTextEdit;
//    QPalette palette2 = des->palette();
//    palette2.setBrush(QPalette::Base, QColor (0, 0 , 0, 0));
    des->verticalScrollBar()->setProperty("drawScrollBarGroove" , false);
//    des->setPalette(palette2);
    des->setReadOnly(true);
    des->setObjectName(FIND_DES_LABLE_TYPE);

    /* 日志文本框 */
    clg = new QTextEdit;
//    QPalette palette3 = clg->palette();
//    palette3.setBrush(QPalette::Base, QColor (0, 0 , 0, 0));
    clg->verticalScrollBar()->setProperty("drawScrollBarGroove" , false);
//    clg->setPalette(palette3);
    clg->setReadOnly(true);
    clg->setObjectName(FIND_CLG_LABLE_TYPE);

    /* 布局 */
    /* 布局外边框 */
    QHBoxLayout *contentlayout = new QHBoxLayout;
    contentlayout->setSpacing(0);
    contentlayout->setMargin(0);
    contentlayout->addSpacing(28);
    contentlayout->addWidget(listBackground);
    contentlayout->addSpacing(2);
    contentlayout->addWidget(desBackground);
    contentlayout->addSpacing(28);
    content = new QWidget(this);
    content->setLayout(contentlayout);

    nocontent=new QLabel(this);
    QVBoxLayout *nocontentlayout = new QVBoxLayout;
    QLabel *nocontentpicture = new QLabel(this);
    QVariant stylename = qtSettings->get("style-name");
    qDebug()<<"style name"<<stylename.toString();
    nocontentpicturepath = ":/img/upgrade/no_content_light.svg";
    int ret = QString::compare(stylename.toString(),"ukui-dark",Qt::CaseInsensitive);
    qDebug()<<"compare result"<<ret;
    if(ret == 0)
    {
        nocontentpicturepath = ":/img/upgrade/no_content_dark.svg";
    }
    nocontentpicture->setPixmap(QPixmap(nocontentpicturepath).scaled(QSize(120,120)));
    nocontentpicture->setAlignment(Qt::AlignCenter);
    QLabel *nocontenttext = new QLabel(this);
    nocontenttext->setText(tr("no content"));
    nocontenttext->setStyleSheet("QLabel{color:grey;}");
    nocontenttext->setAlignment(Qt::AlignCenter);
    nocontentlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
    nocontentlayout->addWidget(nocontentpicture);
    nocontentlayout->addSpacing(10);
    nocontentlayout->addWidget(nocontenttext);
    nocontentlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
    nocontent->setLayout(nocontentlayout);
    nocontent->setAlignment(Qt::AlignCenter);

    QVBoxLayout *Mainlayout = new QVBoxLayout;
    Mainlayout->setContentsMargins(0,0,0,0);
    Mainlayout->addWidget(this->title);
    Mainlayout->addSpacing(18);
    Mainlayout->addWidget(nocontent);
//    Mainlayout->addLayout(contentlayout);
    Mainlayout->addWidget(content);
    content->hide();
    Mainlayout->addSpacing(18);

    this->setLayout(Mainlayout);

    /* 布局列表 */
    hll = new QHBoxLayout;
    hll->setSpacing(0);
    hll->setMargin(0);
    hll->addSpacing(LIST_LEFT);
    hll->addWidget(mainListwidget);
    QVBoxLayout *vll = new QVBoxLayout;
    vll->setSpacing(0);
    vll->setMargin(0);
    vll->addSpacing(LIST_TOP);
    vll->addLayout(hll);
    vll->addSpacing(LIST_TOP);
    listBackground->setLayout(vll);

    /* 布局更新详情标签 */
    QHBoxLayout *hlrt = new QHBoxLayout;
    hlrt->setSpacing(0);
    hlrt->setMargin(0);
    hlrt->addSpacing(TEXT_DSC_LEFT_SPACING);
    hlrt->addWidget(updateDesTab);

    /* 布局标签及文本框 */
    QVBoxLayout *vlr = new QVBoxLayout;
    vlr->setSpacing(0);
    vlr->setMargin(0);
    vlr->addSpacing(TEXT_TAB_TOP);
    vlr->addLayout(hlrt);
    vlr->addSpacing(TEXT_TAB_SPACING);
    vlr->addWidget(des);
//    vlr->addSpacing(TEXT_TAB_SPACING);
//    vlr->addWidget(clg);
    vlr->addSpacing(TEXT_TAB_TOP);

    QHBoxLayout *hlr = new QHBoxLayout;
    hlr->setSpacing(0);
    hlr->setMargin(0);
    hlr->addSpacing(TEXT_TAB_LEFT - TEXT_DSC_LEFT_SPACING);
    hlr->addLayout(vlr);
    hlr->addSpacing(TEXT_TAB_RIGHT);
    desBackground->setLayout(hlr);
    this->installEventFilter(this);
}

void m_updatelog::initGsettings()
{
    timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer,&QTimer::timeout,this,&m_updatelog::changeListWidgetItemHeight);
    const QByteArray iid(THEME_QT_SCHEMA);
    qtSettings = new QGSettings(iid, QByteArray(), this);

    connect(qtSettings,&QGSettings::changed,this,[=] (const QString &key) {
       if(key == "systemFontSize") {
          timer->start(100);
       }
    });
}

void m_updatelog::changeListWidgetItemHeight()
{
    if(mainListwidget->count()<1)
        return;
    int row=0;
    while(row<(mainListwidget->count()))
    {
        QListWidgetItem * item = mainListwidget->item(row);
        HistoryUpdateListWig * hulw = qobject_cast<HistoryUpdateListWig*>(mainListwidget->itemWidget(item));
        item->setSizeHint(hulw->getTrueSize());
        row++;
    }
}

void m_updatelog::updatesql( const int &start,const int &num,const QString &intop)
{
    HistoryUpdateListWig *firstWidget = NULL;
    bool flag = true;

    //sql 拼接
    QString sqlCmd = "SELECT * FROM updateinfos";
    if(intop!="")
        sqlCmd+=" where `date` = '"+intop+"'";
    else if(start>0)
        sqlCmd+=" where `id` < "+QString::number(start);
    sqlCmd+=" order by `id` desc limit ";
    sqlCmd+=QString::number(num);
    //载入数据库数据
    QSqlQuery query(QSqlDatabase::database("A"));
    query.exec(sqlCmd);
    while(query.next()){
        QString statusType = query.value("keyword").toString();
        if(statusType!=""&&statusType!="1")
            continue;
        HistoryUpdateListWig *hulw = new HistoryUpdateListWig(updateDesTab);
        lognum++;
        if (flag == true) {
            firstWidget = hulw;
            flag = false;
        }
        
        QString appname(translationVirtualPackage(query.value("appname").toString()));
        QString appname_cn=query.value("appname_cn").toString();
        QString locale = QLocale::system().name();
        if (locale == "zh_CN"&&!appname_cn.isEmpty())
        {
            appname=appname_cn;
        }
        if (appname.isEmpty() || appname.isNull()) {
            appname = query.value("appname").toString();
        }

        hulw->setAttribute(appname +" "+query.value("version").toString(),
                           query.value("status").toString(),
                           query.value("date").toString(),
                           setDefaultDescription(query.value("description").toString()),
                           setDefaultDescription(query.value("changelog").toString()),
                           query.value("errorcode").toString(),
                           query.value("id").toInt());
        QListWidgetItem *item = new QListWidgetItem();
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(hulw->getTrueSize());
        if(intop!="")
        {
            if(hulw->id<=firstCode)
            {
                hulw->deleteLater();
                delete item;
                item = nullptr;
                return;
            }
            firstCode=hulw->id;
            mainListwidget->insertItem(0,item);
        }
        else
        {
            loadingCode = hulw->id;//记录加载到哪个位置
            mainListwidget->addItem(item);
        }
        mainListwidget->setItemWidget(item,hulw);
        if(intop!="")
            hulw->selectStyle();//设置选中样式
    }

    /* 选中第一条记录 */
    if (firstWidget != NULL && start == 0) {
        firstWidget->selectStyle();
    }
}

void m_updatelog::defaultItem()
{
    //默认选中第一个
    HistoryUpdateListWig *first = mainListwidget->findChild<HistoryUpdateListWig *>();
    if(first!=nullptr)
    {
        first->selectStyle();//设置选中样式
        firstCode = first->id;//记录id
    }
}

QString m_updatelog::translationVirtualPackage(QString str)
{
    QString retStr = str;
    /*判断json文件是否存在*/
    QString filename = QString("/usr/share/kylin-update-desktop-config/config/") +str +".json";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "JSON file open failed! ";
    } else {
        QByteArray jsonData = file.readAll();
        QJsonParseError err_rpt;
        QJsonDocument  root_Doc = QJsonDocument::fromJson(jsonData, &err_rpt); // 字符串格式化为JSON

        if (!root_Doc.isNull() && (err_rpt.error == QJsonParseError::NoError)) {  // 解析未发生错误
            if (root_Doc.isObject()) { // JSON 文档为对象
                QJsonObject object = root_Doc.object();  // 转化为对象
                if (QLocale::system().name() == "zh_CN"){
                    QString name  = object.value("name").toObject().value("zh_CN").toString();
                    if (!name.isNull()) {
                        retStr =  name;
                        return retStr;
                    }
                }else {
                    QString name  = object.value("name").toObject().value("en_US").toString();
                    if (!name.isNull()) {
                        retStr = name;
                        return retStr;
                    }
                }
            }
        }else{
            qDebug() << "JSON文件格式错误！";
        }
    }

    if(QLocale::system().name()!="zh_CN")
        return str;
    if(str == "kylin-update-desktop-app")
        return "基本应用";
    if(str == "kylin-update-desktop-security")
        return "安全更新";
    if(str == "kylin-update-desktop-support")
        return "系统基础组件";
    if(str == "kylin-update-desktop-ukui")
        return "桌面环境组件";
    if(str == "linux-generic")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kernel")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kernel-3a4000")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kydroid")
        return "kydroid补丁包";
    if(str == "kylin-update-desktop")
        return "麒麟系统升级";

    /* 从软件商店数据库根据包名获取应用中文名 */
    QString dst;
    dst.clear();

    QSqlQuery query(QSqlDatabase::database("B"));
    bool ret = query.exec(QString("SELECT display_name_cn FROM application WHERE app_name IS '%1'").arg(str));    //执行
    if (ret == false) {
        qDebug() << "Error : exec select sql fail , switch chinese pkg name fail";
        return retStr;
    }

    while (query.next()) {
        dst = query.value(0).toString();
        qDebug() << "Info : switch chinese pkg name is [" << dst << "]";
    }

    if (!dst.isEmpty()) {
        retStr = dst;
    }
    return retStr;
}


void m_updatelog::dynamicLoadingInit()
{
    //绑定信号和槽
    connect(mainListwidget->verticalScrollBar(),&QScrollBar::valueChanged, this,&m_updatelog::dynamicLoading );
}

void m_updatelog::dynamicLoading(int i)
{
    if(mainListwidget->verticalScrollBar()->maximum()==i)
    {
        qDebug()<<"动态加载";
        updatesql(loadingCode);
    }
}

void m_updatelog::historyUpdateNow(QString str1,QString str2)
{
    qDebug()<<"动态更新:"<<str1;
    updatesql(0,1,str2);
}

/* 历史更新界面搜索功能 */
void m_updatelog::slotSearch(QString packageName)
{
    HistoryUpdateListWig *firstWidget = NULL;
    bool flag = true;
    /* 取消原历史界面动态加载功能 */
    cacheDynamicLoad();

    /* 转换包名 */
    QString dstPackageName = conversionPackageName(packageName);

    /* 清空列表 */
    clearList();

    /* 拼接sql */
    QString sql = "SELECT `appname` ,`version` , `status` , `date` , `description` , `changelog` , `errorcode` , `id` , `keyword` , `appname_cn` , `status_cn` FROM updateinfos WHERE (`appname` LIKE '%" + dstPackageName + "%' OR `status` LIKE '%" + dstPackageName + "%' OR `date` LIKE '%" + dstPackageName + "%' OR `appname_cn` LIKE '%" + dstPackageName + "%' OR `status_cn` LIKE '%" + dstPackageName + "%' OR `version` LIKE '%" + dstPackageName + "%') ORDER BY date desc";

    qDebug() << "Info : sql is [ " << sql << " ]";

    /* 查询数据库 , 获取数据 */
    QSqlQuery query(QSqlDatabase::database("A"));
    if (!query.exec(sql)) {
        qDebug() << "Error : search sql exec fail";
        return;
    }
    while (query.next()) {
        QString appName = query.value(0).toString();
        QString version = query.value(1).toString();
        QString status = query.value(2).toString();
        QString time = query.value(3).toString();
        QString description = setDefaultDescription(query.value(4).toString());
        QString changelog = setDefaultDescription(query.value(5).toString());
        QString errorcode = query.value(6).toString();
        int id = query.value(7).toInt();
        QString keyword = query.value(8).toString();
        QString appname_cn = query.value(9).toString();
        QString status_cn = query.value(10).toString();

        if (keyword != "" && keyword != "1") {
            continue;
        }

        /* 展示搜索内容 */
        HistoryUpdateListWig *updateItem = new HistoryUpdateListWig(updateDesTab);
        if (flag == true) {
            firstWidget = updateItem;
            flag = false;
        }
         
        QString appname(translationVirtualPackage(appName));
        QString locale = QLocale::system().name();
        if (locale == "zh_CN"&&!appname_cn.isEmpty())
        {
            appname=appname_cn;
        }
        if (appname.isEmpty() || appname.isNull()) {
            appname = appName;
        }

        updateItem->setAttribute(appname + " " + version , status , time , description , changelog , errorcode , id);
        QListWidgetItem *item = new QListWidgetItem();
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(updateItem->getTrueSize());

        mainListwidget->addItem(item);
        mainListwidget->setItemWidget(item , updateItem);

        appName.clear();
        version.clear();
        status.clear();
        time.clear();
        description.clear();
        changelog.clear();
        errorcode.clear();
        id = 0;
        keyword.clear();
        appname_cn.clear();
        status_cn.clear();
    }

    /* 选中第一条记录 */
    if (firstWidget != NULL) {
        firstWidget->selectStyle();
    } else {
        if (this->des != NULL) {
            des->setText(QString(""));
        }
        if (this->updateDesTab != NULL) {
            updateDesTab->setText(QString(""));
        }
        if (this->clg != NULL) {
            clg->setText(QString(""));
        }
    }
    return;
}

void m_updatelog::cacheDynamicLoad(void)
{
     disconnect(mainListwidget->verticalScrollBar() , &QScrollBar::valueChanged , this , &m_updatelog::dynamicLoading);
}

void m_updatelog::clearList(void)
{
   int sum = mainListwidget->count();

   for (int i = sum ; i >= 0 ; i--) {
       QListWidgetItem *item = mainListwidget->takeItem(i);
       delete item;
   }

   return;
}

QString m_updatelog::conversionPackageName(QString package)
{
    if (QLocale::system().name() != "zh_CN")
        return package;
    if (package == "基本应用")
        return "kylin-update-desktop-app";
    if (package == "安全更新")
        return "kylin-update-desktop-security";
    if (package == "系统基础组件")
        return "kylin-update-desktop-support";
    if (package == "桌面环境组件")
        return "kylin-update-desktop-ukui";
    if (package == "系统内核组件")
        return "linux-generic";
    if (package == "系统内核组件")
        return "kylin-update-desktop-kernel";
    if (package == "系统内核组件")
        return "kylin-update-desktop-kernel-3a4000";
    if (package == "kydroid补丁包")
        return "kylin-update-desktop-kydroid";

    /* 从软件商店数据库根据包名获取应用英文名 */
    QString dst;
    dst.clear();

    QSqlQuery query(QSqlDatabase::database("B"));
    bool ret = query.exec(QString("SELECT `app_name` FROM application WHERE `display_name_cn` = '%1'").arg(package));    //执行
    if (ret == false) {
        qDebug() << "Error : exec select sql fail , switch pkg name fail";
        return package;
    }

    while (query.next()) {
        dst = query.value(0).toString();
        qDebug() << "Info : switch chinese pkg name is [" << dst << "]";
    }

    if (dst.isEmpty()) {
        return package;
    } else {
        return dst;
    }

    return package;
}


bool m_updatelog::eventFilter(QObject *watch , QEvent *e)
{
    if ((e->type() == QEvent::MouseButtonPress && watch != searchBox)) {
        if (this->searchBox->text() == "") {
            qDebug()<<"QEvent::MouseButtonPress && watch != searchBox, searchBox->text() == NULL";
            this->searchBox->setAlignment(Qt::AlignLeft);
//            this->searchBox->setPlaceholderText(tr("Search content"));
//            this->searchBox->setTextMargins(this->searchBox->width()/ 2 - 60 , 0 , 0 , 1);
//            this->searchBox->setFocus();
            this->searchBox->clearFocus();
        }
    }

    if (e->type() == QEvent::MouseButtonPress && watch == searchBox) {
        qDebug()<<"QEvent::MouseButtonPress && watch == searchBox";
        this->searchBox->setAlignment(Qt::AlignLeft);
//        this->searchBox->setPlaceholderText(tr(""));
//        this->searchBox->setTextMargins(this->searchBox->width()/ 2 - 60 , 0 , 0 , 1);
    }

    if (e->type() == 6 && watch == searchBox) {
        QKeyEvent *key = static_cast<QKeyEvent *>(e);
        if (key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return) {
            if (this->searchBox->text() != "") {
                QString appName = this->searchBox->text();
                slotSearch(appName);
            }
            if (this->searchBox->text() == "") {
                clearList();
                connect(mainListwidget->verticalScrollBar() , &QScrollBar::valueChanged , this , &m_updatelog::dynamicLoading);
                updatesql();
            }
        }
    }

    return QObject::eventFilter(watch , e);
}

void m_updatelog::updateTitleWidget(void)
{
    this->title = new QWidget(this);
    this->title->setFixedHeight(40);
    this->title->setObjectName("titleWidget");
    titlewidget = new QWidget(this);
    titlewidget->setFixedHeight(40);

    /* 标题栏图标 */
    this->titleIcon = new QLabel(this);
    this->titleIcon->setFixedSize(24 , 24);
    this->titleIcon->setPixmap(QIcon::fromTheme("ukui-control-center").pixmap(QSize(24 , 24)));
    /* 标题栏名字 */
    this->titleName = new QLabel(this);
    this->titleName->resize(56 , 20);
    //this->titleName->setText(tr("历史更新"));
    this->titleName->setText(tr("History Log"));
    /*标题整体*/
    titlewidgetlayout = new QHBoxLayout(titlewidget);
    titlewidgetlayout->setContentsMargins(3,8,0,8);
    titlewidgetlayout->addWidget(titleIcon);
    titlewidgetlayout->addSpacing(4);
    titlewidgetlayout->addWidget(titleName);
    titlewidget->setLayout(titlewidgetlayout);
    /* 搜索框 */
    this->searchBox = new KSearchLineEdit(this);
    this->searchBox->setFixedSize(300 , 31);
    this->searchBox->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    this->searchBox->installEventFilter(this);
//    this->searchBox->setTextMargins(this->searchBox->width()/ 2 - 60 , 0 , 0 , 1);
    /* 关闭按钮 */
    this->titleClose = new QPushButton(this);
    titleClose->setToolTip(tr("Close"));
    this->titleClose->setFixedSize(30 , 30);
    this->titleClose->setFlat(true);
    this->titleClose->setProperty("isWindowButton" , 0x2);
    this->titleClose->setProperty("useIconHighlightEffect" , 0x8);
    this->titleClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    this->titleClose->setFocusPolicy(Qt::NoFocus);
    this->titleClose->setToolTip(tr("Close"));
    connect(this->titleClose , &QPushButton::clicked , this , &m_updatelog::slotClose);

    /* 布局 */
    titlelayout = new QHBoxLayout(title);
    titlelayout->setContentsMargins(5,0,5,0);
    title->setLayout(titlelayout);
    titlelayout->addWidget(titlewidget);
    titlelayout->addSpacing(4);
    titlelayout->addStretch();
    titlelayout->addWidget(searchBox);
    titlelayout->addStretch();
    titlelayout->addSpacing(this->titlewidget->width()+4);
    titlelayout->addWidget(titleClose);
}

void m_updatelog::slotClose(void)
{
    this->close();
}
