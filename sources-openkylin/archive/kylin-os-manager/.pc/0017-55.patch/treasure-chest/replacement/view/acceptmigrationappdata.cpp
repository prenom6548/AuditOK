#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDateTime>
#include <QHeaderView>
#include <QDir>
#include <QSettings>
#include <QDesktopServices>
#include <QTextCodec>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCloseEvent>

#include "acceptmigrationappdata.h"
#include "../globaldata.h"

const QString jsonFileSourPath = "/usr/share/kylin-os-manager/replacement/data/";
const QString jsonFileTargPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/replacement/data/";
const QString fileName = "appblacklist.json";

AcceptMigrationAppData::AcceptMigrationAppData(QWidget *parent)
{
    initUI();
}

AcceptMigrationAppData::~AcceptMigrationAppData()
{

}

void AcceptMigrationAppData::initUI()
{
    this->setFixedSize(800, 500);
    this->windowButtonBar()->minimumButton()->hide();
    this->windowButtonBar()->maximumButton()->hide();
    this->windowButtonBar()->menuButton()->hide();

    m_btnCannel = new QPushButton(this);
    m_btnSure = new QPushButton(this);
    m_label = new K::TextLabel(this);
    m_treeWidget = new QTreeWidget(this);
    m_vlayout = new QVBoxLayout();

    m_label->SetPixelSize(16);
    m_label->SetText(GlobalData::g_textRectChooseApp);

    m_treeWidget->setFixedSize(752, 321);
    // 将表格变为禁止编辑,取消焦点
    m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeWidget->setFocusPolicy(Qt::NoFocus);
    // 设置表格不可选中
    m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    // 开启拖拽功能
    m_treeWidget->setDragEnabled(false);
    // 设置控件内部移动
    m_treeWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    // 设置移动模式
    m_treeWidget->setDefaultDropAction(Qt::IgnoreAction);
    m_treeWidget->header()->setDragEnabled(false);
    m_treeWidget->header()->setAcceptDrops(false);
    m_treeWidget->header()->setDefaultDropAction(Qt::IgnoreAction);
    m_treeWidget->header()->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_treeWidget->header()->setSectionsMovable(false);

//    m_treeWidget->header()->setFixedHeight(48);
    m_treeWidget->header()->setTextElideMode(Qt::ElideRight);
    m_treeWidget->setMouseTracking(true);
    m_treeWidget->setRootIsDecorated(false);
    //设置交替背景色
    m_treeWidget->setAlternatingRowColors(true);
    QStringList topTitle;
    topTitle << "应用名称" << "描述" << "地址" ;
    m_treeWidget->setHeaderLabels(topTitle);


    m_btnCannel->setText(GlobalData::g_textRectMsgCannel);
    m_btnSure->setText(GlobalData::g_textRectMsgSure);
    GlobalData::getInstance()->setHighColorBtn(m_btnSure);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addStretch();
    hlayout->addWidget(m_btnCannel);
    hlayout->addSpacing(8);
    hlayout->addWidget(m_btnSure);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    m_vlayout->addWidget(m_label);
    m_vlayout->addSpacing(16);
    m_vlayout->addWidget(m_treeWidget);
    m_vlayout->addSpacing(24);
    m_vlayout->addLayout(hlayout);
    m_vlayout->addStretch();
    m_vlayout->setSpacing(0);
    m_vlayout->setMargin(0);
    m_vlayout->setContentsMargins(24, 16, 24, 24);

    baseBar()->setLayout(m_vlayout);

    if (createJsonFile()) {
        readJsonFile();
    } else {
        writeJsonFile();
        readJsonFile();
    }
    
    initTreeWidget();
    connect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem *,int)),this,SLOT(treeItemChanged(QTreeWidgetItem*,int)));
    connect(m_btnCannel, &QPushButton::clicked, this, &AcceptMigrationAppData::clickCannelBtn);
    connect(m_btnSure, &QPushButton::clicked, this, &AcceptMigrationAppData::clickSureBtn);

    // connect(m_treeWidget, &QTreeWidget::itemClicked, [=](QTreeWidgetItem *item, int column) {
    //     if (m_currentItem == m_treeWidget->currentItem()) {
    //         m_currentItem = NULL;
    //     } else {
    //         m_currentItem = item;
    //     }
    //     // if (column != 0) {
    //         if (item->checkState(0) == Qt::Checked) {
    //             item->setCheckState(0, Qt::Unchecked);
    //         } else {
    //             item->setCheckState(0, Qt::Checked);
    //         }
    //     // }
    //     qDebug() << "++++++" << item->text(0) << column;
    // });
}

void AcceptMigrationAppData::initTreeWidget()
{
    readSenderDesktopAllowMigrationFile();

    QDir* dirinfo = new QDir(desktopFilePath);
    if (!dirinfo->exists()) {
        delete dirinfo, dirinfo = nullptr;
    }
    dirinfo->setNameFilters(QStringList("*"));
    QStringList fileList = dirinfo->entryList(QDir::Files);

    fileList.removeOne(".");
    fileList.removeOne("..");

    m_treeWidget->headerItem()->setFlags(m_treeWidget->headerItem()->flags()  | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    m_treeWidget->headerItem()->setCheckState(0, Qt::Unchecked);

    for (int i = 0; i < fileList.size(); i++ ) {
        if (!m_listAllowDesktopFile.contains(fileList[i])) {
            continue;
        }

        QString file = desktopFilePath + fileList[i];

        QSettings desktopFileSetting(file, QSettings::IniFormat);
        desktopFileSetting.setIniCodec(QTextCodec::codecForName("UTF-8"));
        desktopFileSetting.beginGroup(DesktopGroup);
        QString name = desktopFileSetting.value(DesktopKeyNameZH).toString();
        QString comm = desktopFileSetting.value(DesktopKeyCommentZH).toString();
        QString Exec = desktopFileSetting.value("Exec").toString();
        if (name == "") {
            name = desktopFileSetting.value(DesktopKeyNameEN).toString();
        } 
        if (comm == "") {
            comm = desktopFileSetting.value(DesktopKeyCommentEN).toString();
        }
        if (name == "") {
            continue;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << comm << Exec);
        item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        
        bool incompatibleBool = false;
        if (m_listIncompatible.size() != 0) {
            for (int j = 0 ; j < m_listIncompatible.size(); j++) {
                if (fileList[i].contains(m_listIncompatible[j])) {
                    incompatibleBool = true;
                }
            }
        }
        if (incompatibleBool) {
            continue;
        }

        bool containBool = false;
        if (m_listApp.size() != 0) {
            if (m_listApp.contains(fileList[i])) {
                containBool = true;
            }
        }
        if (m_listNameContains.size() != 0) {
            for (int j = 0 ; j < m_listNameContains.size(); j++) {
                if (fileList[i].contains(m_listNameContains[j])) {
                    containBool = true;
                }
            }
        }
        
        if (containBool) {
            item->setCheckState(0, Qt::Unchecked);
        } else {
            qDebug() << fileList[i];
            item->setCheckState(0, Qt::Checked);
            // m_list.append(file);
            m_list.append(fileList[i]);
        }
        m_treeWidget->addTopLevelItem(item);
        // m_itemHash.insert(item, file);
        m_itemHash.insert(item, fileList[i]);

    }
}

void AcceptMigrationAppData::clickSureBtn()
{
    this->close();
}

void AcceptMigrationAppData::clickCannelBtn()
{
    GlobalData::getInstance()->g_chooseAppData = false;
    this->close();
}

void AcceptMigrationAppData::treeItemChanged(QTreeWidgetItem *item, int column)
{
    QString itemText = item->text(0);
    QString filePath = m_itemHash.value(item);
    if (m_list.contains(filePath)) {
        m_list.removeOne(filePath);
    } else {
        m_list.append(filePath);
    }
    qDebug() << itemText;
}

void AcceptMigrationAppData::initTreewidgetStyle()
{
    m_itemHash.clear();
    m_list.clear();
    m_treeWidget->clear();
    if (createJsonFile()) {
        readJsonFile();
    } else {
        writeJsonFile();
        readJsonFile();
    }
    initTreeWidget();
}

bool AcceptMigrationAppData::createJsonFile()
{
    QFile jsonFileSour(jsonFileSourPath + fileName);
    QFile jsonFileTarg(jsonFileTargPath + fileName);

    QString confDirPath = jsonFileTargPath;

    QDir confDir = QDir(confDirPath);
    if (!confDir.exists()) {
        confDir.mkpath(confDirPath);
    }

    if (!(jsonFileTarg.exists())) {
        if (jsonFileSour.exists()) {
            if (!jsonFileSour.copy(jsonFileTargPath + fileName)) {
                qDebug() << "文件 copy 创建失败！";
                return false;
            } else {
                qDebug() << "文件 copy 创建成功！";
                return true;
	        }
        } else {
            return false;
        }
    } else {
        return true;
    }
}

void AcceptMigrationAppData::writeJsonFile()
{
    QJsonObject jsonObject;
    QJsonArray jsonArrayNameContains;
    jsonArrayNameContains.insert(0, "kylin");
    jsonArrayNameContains.insert(1, "ukui");
    jsonArrayNameContains.insert(2, "gnome");
    jsonArrayNameContains.insert(3, "mate");
    jsonObject.insert("nameContain",jsonArrayNameContains);

    QJsonArray jsonArrayV10;
    jsonObject.insert("v10App",jsonArrayV10);

    QJsonArray jsonArraySp1;
    jsonObject.insert("sp1App",jsonArraySp1);

    QJsonArray jsonArrayOther;
    jsonObject.insert("otherApp",jsonArrayOther);

    /// Json文件格式可以使用 ".json",也可以使用 ".jdb"
    QFile file(jsonFileTargPath + fileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug()<<"Open json file failed";
        return;
    }
    QJsonDocument jsonDocument;
    jsonDocument.setObject(jsonObject);
    file.write(jsonDocument.toJson());
    file.close();

    return;
}

void AcceptMigrationAppData::readSenderDesktopAllowMigrationFile()
{
    m_listAllowDesktopFile.clear();
    QFile file(desktopListAllowFilePath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug()<<" Open json file failed ";
        return;
    }
    // 读取文件内容并判断每一行的开头
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        m_listAllowDesktopFile.append(line);
    }
}

void AcceptMigrationAppData::readJsonFile()
{
    m_listApp.clear();
    m_listNameContains.clear();
    m_listIncompatible.clear();
    QFile file(jsonFileTargPath + fileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug()<<" Open json file failed ";
        return;
    }

    QByteArray allData = file.readAll();
    file.close();

    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "json error 格式错误!";
    }
    QJsonObject rootObj = jsonDoc.object();
 
    QJsonArray jsonArrayIncompatible = rootObj.value("incompatible").toArray();
    for(int i=0; i < jsonArrayIncompatible.size(); i++) {
        m_listIncompatible.append(jsonArrayIncompatible.at(i).toString());
        qDebug() << "incompatible" << jsonArrayIncompatible.at(i).toString();
    }
 
    QJsonArray jsonArrayNameContain = rootObj.value("nameContain").toArray();
    for(int i=0; i < jsonArrayNameContain.size(); i++) {
        m_listNameContains.append(jsonArrayNameContain.at(i).toString());
        qDebug() << "name" << jsonArrayNameContain.at(i).toString();
    }

    QJsonArray jsonArrayV10 = rootObj.value("v10App").toArray();
    for(int i=0; i < jsonArrayV10.size(); i++) {
        m_listApp.append(jsonArrayV10.at(i).toString());
        qDebug() << "v10" << jsonArrayV10.at(i).toString();
    }

    QJsonArray jsonArraySp1 = rootObj.value("sp1App").toArray();
    for(int i=0; i < jsonArraySp1.size(); i++) {
        m_listApp.append(jsonArraySp1.at(i).toString());
        qDebug() << "sp1" << jsonArraySp1.at(i).toString();
    }

    QJsonArray jsonArrayOther = rootObj.value("otherApp").toArray();
    for(int i=0; i < jsonArrayOther.size(); i++) {
        m_listApp.append(jsonArrayOther.at(i).toString());
        qDebug() << "other" << jsonArrayOther.at(i).toString();
    }

    return;
}

// 窗口关闭事件
void AcceptMigrationAppData::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    if ( GlobalData::getInstance()->g_chooseAppData == true) {
        emit sigAppData(m_list);
    }
    GlobalData::getInstance()->g_chooseAppData = true;
}