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
#include "batch-install.h"

Batchinstall::Batchinstall( QString debpath, QWidget *parent) : QWidget(parent)
{
    const QByteArray style_id(ORG_UKUI_STYLE_FONT);
    m_pGsettingFontSize = new QGSettings(style_id);

    // 此处不判断deb包是否存在，因为在调用此类之前已经判断过了
    Package *package = PackageFactory::createPackage(debpath, false);
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO, "Batchinstall");

    m_appname       = package->m_name;
    m_appversion    = package->m_version;
    m_debArch       = package->m_architecture;
    zh_apname       = package->m_zhName;
    m_debpath       = package->m_debPath;

    appiconlabel = new QLabel();
    appiconlabel->adjustSize();

    appversionlabel = new QLabel();
    appversionlabel->setMaximumWidth(240);

    deleteitem = new QToolButton();
    deleteitem->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    deleteitem->setIconSize(QSize(16, 16));
    deleteitem->setFixedSize(36, 36);
    deleteitem->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    deleteitem->setObjectName("delete_app");
    deleteitem->adjustSize();
    deleteitem->setProperty("isWindowButton", 0x1);
    deleteitem->setProperty("useIconHighlightEffect", 0x2);
    deleteitem->setAutoRaise(true);

    appstatus = new QLabel();
    appstatus->setText(tr("Wait"));
    appstatus->setObjectName("appstatus");
    appstatus->adjustSize();

    pProgressBar = new QProgressBar();
    pProgressBar->setOrientation(Qt::Horizontal);  // 水平方向
    pProgressBar->setMinimum(0);  // 最小值
    pProgressBar->setMaximum(100);  // 最大值
    pProgressBar->setValue(0);  // 当前进度
    pProgressBar->setObjectName("pProgressBar");
    pProgressBar->setFixedWidth(80);
    pProgressBar->setFixedHeight(8);
    pProgressBar->setTextVisible(false);
    pProgressBar->hide();

    progress = new QLabel();
    progress->setText("0%");
    progress->adjustSize();
    progress->setObjectName("progress");
    progress->hide();


    m_appiconlabel = new QLabel();
    m_appiconlabel->setFixedSize(25,25);
    m_appiconlabel->setObjectName("app_status");
    m_appiconlabel->hide();

    connect(deleteitem, &QPushButton::clicked, this, &Batchinstall::removelistwidgetitem);

    if (QIcon::fromTheme(m_appname).isNull()) {
        if (getIconPath(m_appname) != nullptr)
            appiconlabel->setPixmap(QIcon(QPixmap(getIconPath(m_appname))).pixmap(QSize(48, 48)));
        else {
            if (package->m_pkgType == packageType::TPYE_KAIMING && !QIcon::fromTheme("kaiming-permission-manager").isNull()) {
                appiconlabel->setPixmap(
                    QIcon::fromTheme("kaiming-permission-manager")
                            .pixmap(48, 48)
                );
            } else {
                appiconlabel->setPixmap(
                    QIcon::fromTheme("application-x-desktop")
                            .pixmap(48, 48)
                );
            }
        }
    } else {
        appiconlabel->setPixmap(
            QIcon::fromTheme(m_appname)
                    .pixmap(48, 48)
        );
    }

    widgetall = new QWidget();
    debiconLayout = new QHBoxLayout();
    debiconLayout->setContentsMargins(0, 0, 16, 0);
    debiconLayout->setSpacing(0);
    debiconLayout->addWidget(appiconlabel);
    debNameAndVersioNLayout = new QVBoxLayout();
    debNameAndVersioNLayout->setContentsMargins(0, 0, 0, 0);
    debNameAndVersioNLayout->setSpacing(0);

    appnamelabel = new QLabel();
    if (zh_apname.isEmpty())
        appnamelabel->setText(m_appname);
    else
        appnamelabel->setText(zh_apname);

    QString debVersion = tr("deb version :");
    debVersion.append(m_appversion);

    appversionlabel->setText(setLabelStringBody(debVersion, appversionlabel));
    appversionlabel->setObjectName("appversion");
    appversionlabel->adjustSize();
    appversionlabel->setAlignment(Qt::AlignTop);

    appnamelabel->adjustSize();
    appnamelabel->setAlignment(Qt::AlignBottom);

    debNameAndVersioNLayout->addWidget(appnamelabel);
    debNameAndVersioNLayout->addSpacing(4);
    debNameAndVersioNLayout->addWidget(appversionlabel);

    int lenth = 0;
    if (appnamelabel->width()>appversionlabel->width())
        lenth = appnamelabel->width();
    else
        lenth = appversionlabel->width();

    statuswidget = new QWidget();

    deleteitemlayout = new QHBoxLayout();
    deleteitemlayout->addStretch();
    deleteitemlayout->addWidget(appstatus);
    deleteitemlayout->addWidget(pProgressBar);
    deleteitemlayout->addSpacing(10);
    deleteitemlayout->addWidget(progress);
    deleteitemlayout->setAlignment(Qt::AlignRight);
    deleteitemlayout->setContentsMargins(0, 0, 0, 0);
    deleteitemlayout->setSpacing(0);


    stauslayout = new QHBoxLayout();
    stauslayout->setContentsMargins(0, 0, 0, 0);
    stauslayout->setSpacing(0);
    stauslayout->addWidget(deleteitem);
    stauslayout->addWidget(m_appiconlabel);

    stauslayoutall = new QHBoxLayout();
    stauslayoutall->setContentsMargins(0, 0, 0, 0);
    stauslayoutall->setSpacing(0);
    stauslayoutall->addLayout(deleteitemlayout);
    stauslayoutall->setSpacing(10);
    stauslayoutall->addLayout(stauslayout);

    statuswidget->setLayout(stauslayoutall);

    Layoutall = new QHBoxLayout();
    Layoutall->addLayout(debiconLayout);
    Layoutall->addLayout(debNameAndVersioNLayout);
    Layoutall->addStretch();
    Layoutall->addWidget(statuswidget);
    Layoutall->addSpacing(16);
    Layoutall->setContentsMargins(0, 0, 0, 0);
    Layoutall->setSpacing(0);

    this->setLayout(Layoutall);
    this->setProperty("useIconHighlightEffect", 0x2);
}

QString Batchinstall::getDebInfoFromDeb(QString m_debPath, QString type)
{
    QStorageInfo SD;
    SD.setPath("/");
    SD.refresh();
    int disk_space = SD.bytesAvailable()/1024/1024;
    if (disk_space <=10)
    {
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",tr("Insufficient disk space!!!!"),"",this);
        msgBox->exec();
        msgBox->setContextMenuPolicy(Qt::NoContextMenu);
        exit(0);
    }
    QProcess terminal;
    QStringList cmdList;
    m_debPath = QDir::toNativeSeparators(m_debPath);

    QRegExp rx("[^a-zA-Z0-9]");
    int pos = rx.indexIn(m_debPath);
    if (pos > -1)
        m_debPath.toHtmlEscaped();

    QFileInfo file(m_debPath);

    cmdList << "-c" << QString("dpkg -I " + m_debPath);
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString debInfo = terminal.readAll();

    QString debinfoerro = terminal.readAllStandardError().data();
    if ((debInfo.contains(type) || debInfo.contains(type.toLower())) && debinfoerro=="" ){
        cmdList.clear();
        terminal.close();
        cmdList << "-c" << QString("dpkg -I " + m_debPath + "| grep " + type);
        terminal.start("/bin/bash",cmdList);
        terminal.waitForFinished();

        debInfo = terminal.readAll();

        if ( debInfo!="" && debInfo.contains(type)){
            QStringList list = debInfo.split(":");
            return list[list.count()-1].simplified();
        }
    }
    else if (debInfo!="" && debinfoerro.contains("压缩数据已损坏"))
    {
        //deb包已损坏，无法解析
        return "The software package is damaged";
    }
    else if (file.exists() && debInfo=="")
    {
        //路径包含特殊字符，需要debfile解析
        //QApt::DebFile::isValid();
        QApt::DebFile debfile(m_debPath);
        if (debfile.isValid()){
            if (type == "Package"){
                return debfile.packageName();}
            else if (type == "Version"){
                return debfile.version();
            }
            else{
                return debfile.architecture();
            }
        }
        else{
            return "The software package is damaged";
        }
    }

}

QString Batchinstall::getAppCnName(QString appName)
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/uksc.db");
    if (!db.open()) {
        qDebug()<<"db open failed!";
    }
    /**************************使用QSqlQuery操作数据库**************************/
    QSqlQuery query;	//执行操作类对象

    bool ret = query.exec(QString("SELECT * FROM application WHERE app_name IS '%1'").arg(appName));	//执行
    qDebug()<<"ret:"<<ret;
    while(query.next()) {
        qDebug()<<query.size();
        qDebug()<<query.value(4).toString();
        return query.value(4).toString();
    }
    db.close();
    return nullptr;
}

QString Batchinstall::getIconPath(QString appName)
{
    QString iconFilePath;
    /*cr判断~/.cache/icons下是否有icon*/
    iconFilePath.clear();
    iconFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/icons/"+appName + ".png";
    qDebug()<<iconFilePath;
    if (QFile::exists(iconFilePath)) {
        return iconFilePath;
    }
    /*最后判断/usr/share/ubuntu-kylin-software-center/data/icons下面是否有*/
    iconFilePath.clear();
    iconFilePath = "/usr/share/kylin-software-center/data/icons/" + appName + ".png";
    qDebug()<<iconFilePath;
    if (QFile::exists(iconFilePath)) {
        return iconFilePath;
    }
    return nullptr;
}

/********发送删除item信号********/
void Batchinstall::removelistwidgetitem()
{
     emit deleteitemstring(m_debpath);
}

QString Batchinstall::setLabelStringBody(QString text, QLabel *lable)
{
    QFontMetrics fontMetrics(lable->font());
    int LableWidth = lable->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if(fontSize > (LableWidth - 10)) {
        //说明只存在一行，在最后面加...就行
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
        lable->setToolTip(text);
        return formatBody;
    }
    lable->setToolTip(text);
    return formatBody;
}

QFont Batchinstall::getSystemFont(int size)
{
    double fontsize = (m_pGsettingFontSize->get(GSETTING_FONT_KEY).toFloat());

//    double lableBaseFontSize = size * 9/12;//魔鬼数字，自行处理
//    double nowFontSize = lableBaseFontSize * double(fontsize) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    if (size == 18){
        font.setFamily("Noto Sans CJK SC");
        font.setWeight(500);

    }
    font.setBold(false);
    font.setPointSizeF(fontsize);

    return font;
}
