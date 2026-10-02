/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "backuppointlistdialog.h"
#include "ui_backuppointlistdialog.h"
#include <QHeaderView>
#include <QSizePolicy>
#include <QFile>
#include <QDebug>
#include <ukuistylehelper/ukuistylehelper.h>
#include <windowmanager/windowmanager.h>
#include "component/mypushbutton.h"
#include "../common/mydefine.h"
#include "../common/utils.h"
#include "globalbackupinfo.h"
#include "gsettingswrapper.h"

namespace {
    // 仅在进程生命周期内缓存列宽
    constexpr int kBackupPointHeaderCount = BackupPointListDialog::Column_Index::Prefix_Path + 1; // 得到总列数
    int g_backupPointHeaderWidths[kBackupPointHeaderCount] = {0};
    bool g_headerWidthReady = false;
}

BackupPointListDialog::BackupPointListDialog(QWidget *parent, bool isOnlyShowLocal) :
    QDialog(parent),
    ui(new Ui::BackupPointListDialog),
    m_udector(new UdiskDetector()),
    m_onlyShowLocal(isOnlyShowLocal)
{
    ui->setupUi(this);

    // 为解决贴边问题，设置为固定大小
    this->setFixedSize(QSize(800, 408));
    this->setAutoFillBackground(true);

    // 去除窗管标题栏，传入参数为QWidget *
    kdk::UkuiStyleHelper::self()->removeHeader(this);

    setWindowModality(Qt::WindowModal);

    // 纵向布局
    QVBoxLayout *vlayout = new QVBoxLayout;

    // 设置标题栏
    QHBoxLayout * titleLayout = new QHBoxLayout();
    // logo
    PixmapLabel *titleIcon  = new PixmapLabel;
    titleIcon->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_titleIcon");
    titleIcon->setFixedSize(24, 24);
    titleIcon->setThemeIconSchema(THEME_YHKYLIN_BACKUP_TOOLS, ":/images/yhkylin-backup-tools.png");
    // 标题
    m_titleLabel  = new MyLabel;
    m_titleLabel->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_m_titleLabel");
    m_titleLabel->setFixedHeight(30);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 关闭按钮
    QPushButton *buttonClose = new QPushButton;
    buttonClose->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_buttonClose");
    buttonClose->setAccessibleName("yhkylin-backup-tools_kybackup_QPushButton_buttonClose");
    buttonClose->setProperty("isWindowButton", 0x2);
    buttonClose->setProperty("useIconHighlightEffect", 0x8);
    buttonClose->setFlat(true);
    if (GlobelBackupInfo::instance().isTabletMode())
        buttonClose->setFixedSize(TABLET_TITLE_BAR_SIZE, TABLET_TITLE_BAR_SIZE);
    else
        buttonClose->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    buttonClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    buttonClose->setToolTip(tr("Close"));
    connect(buttonClose, &QPushButton::clicked, this, &BackupPointListDialog::close);

    titleLayout->addWidget(titleIcon);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(buttonClose);
    vlayout->addLayout(titleLayout);

    QStringList headerLabels;
    // 注意：此处的列需要和枚举Column_Index一一对应
    headerLabels << tr("Backup Name") << tr("Backup Type") << tr("UUID") << tr("Backup Time")  << tr("Backup Size") << tr("Position") << QObject::tr("Backup State") << QObject::tr("PrefixPath");
    m_tableWidget = new QTableWidget;
    m_tableWidget->setObjectName("yhkylin-backup-tools_kybackup_QTableWidget_m_tableWidget");
    m_tableWidget->setColumnCount(headerLabels.size());
    m_tableWidget->setHorizontalHeaderLabels(headerLabels);
    auto header = m_tableWidget->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignLeft);
    // m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->resizeSection(Column_Index::Backup_Name, 150);
    // m_tableWidget->horizontalHeader()->resizeSection(Column_Index::UUID, 310);
    header->resizeSection(Column_Index::Backup_Type, 120);
    header->resizeSection(Column_Index::Backup_Time, 150);
    header->resizeSection(Column_Index::Backup_Size, 120);
    header->setSectionResizeMode(Column_Index::Backup_Device, QHeaderView::Stretch);
    header->resizeSection(Column_Index::Backup_State, 120);
    header->setTextElideMode( Qt::ElideRight);
    header->setFixedHeight(36);
    m_tableWidget->verticalHeader()->setHidden(true); // 好像隐藏不掉
    m_tableWidget->verticalHeader()->setVisible(false); //左侧标题不可见
    m_tableWidget->verticalHeader()->setDefaultSectionSize(36); // 设置行高
    header->setHighlightSections(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->hideColumn(Column_Index::UUID);
    m_tableWidget->hideColumn(Column_Index::Prefix_Path);
    m_tableWidget->setSortingEnabled(false); // 等录入完数据后再排序
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableWidget->setFrameShape(QFrame::NoFrame);

    // 首次打开时，写入默认列宽至缓存数组并标记表头宽度已准备完毕
    // 非首次打开时，将更改的表头宽度替换为缓存数组的对应列宽
    if (!g_headerWidthReady) {
        for (int i = 0; i < m_tableWidget->columnCount(); ++i) {
            g_backupPointHeaderWidths[i] = header->sectionSize(i);
        }
        g_headerWidthReady = true;
    } else {
        for (int i = 0; i < m_tableWidget->columnCount(); ++i) {
            if (g_backupPointHeaderWidths[i] > 0) {
                header->resizeSection(i, g_backupPointHeaderWidths[i]);
            }
        }
    }
    // 将新尺寸写入缓存数组对应列宽
    connect(header, &QHeaderView::sectionResized, this,
            [](int logicalIndex, int /*oldSize*/, int newSize) {
        // 防御性编程，确保表头列索引在合适的程度内
        if (logicalIndex < 0 || logicalIndex >= kBackupPointHeaderCount){
            return;
        }
        g_backupPointHeaderWidths[logicalIndex] = newSize;
    });
    // 监听是否点击表头，点击时可视化表头箭头
    connect(header, &QHeaderView::sectionClicked, this, [header]() {
        header->setSortIndicatorShown(true);
    });
    // 根据监听主题变化颜色
    if (g_GSettingWrapper.isDarkTheme()) {
        m_tableWidget->setStyleSheet("QTableWidget { border: 1px solid rgba(217,217,217,0.4);}");
    } else {
        m_tableWidget->setStyleSheet("QTableWidget { border: 1px solid #D9D9D9; }");
    }

    if (g_GSettingWrapper.m_pGsettingThemeData) {
        connect(g_GSettingWrapper.m_pGsettingThemeData, &QGSettings::changed, this, [=](const QString &key) {
            if (key == STYLE_NAME) {
                QString themeStyle = g_GSettingWrapper.m_pGsettingThemeData->get(STYLE_NAME).toString();
                if (STYLE_NAME_VALUE_DARK == themeStyle || STYLE_NAME_VALUE_BLACK == themeStyle) {
                    m_tableWidget->setStyleSheet("QTableWidget { border: 1px solid rgba(217,217,217,0.7);}");
                } else if (STYLE_NAME_VALUE_DEFAULT == themeStyle || STYLE_NAME_VALUE_LIGHT == themeStyle || STYLE_NAME_VALUE_WHITE == themeStyle) {
                    m_tableWidget->setStyleSheet("QTableWidget { border: 1px solid #D9D9D9; }");
                }
            }
        });
    }

    QHBoxLayout *tableLayout = new QHBoxLayout;
    tableLayout->setContentsMargins(10, 0, 10, 0);
    tableLayout->addWidget(m_tableWidget);
    vlayout->addLayout(tableLayout);

    m_bottomLayout = new QHBoxLayout;
    m_bottomLayout->setContentsMargins(10, 0, 10, 0);
    vlayout->addLayout(m_bottomLayout);
    this->setLayout(vlayout);

    // 添加本地默认路径、移动设备目录
    connect(m_udector, &UdiskDetector::udiskListChanged, this, [=](QList<QStorageInfo> diskList) {
        this->m_udiskPaths.clear();
        // 将本地默认备份路径放在第一个
        this->m_udiskPaths << Utils::getSysRootPath();

        /* 2025-5-14 需要展示移动路径
        if (this->m_onlyShowLocal)
            return;
        */
        // 移动设备挂载路径
        for (QStorageInfo& disk : diskList) {
            this->m_udiskPaths << disk.rootPath();
        }

        emit this->udiskChange();
    });
    m_udector->getStorageInfo();

    // 表格中的布局
    // 列表为空时展示图片
    m_labelEmpty = new PixmapLabel(m_tableWidget);
    m_labelEmpty->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_m_labelEmpty");
    m_labelEmpty->setLightAndDarkPixmap(":/images/empty.png", ":/images/empty_dark.png");
    m_labelEmptyText = new QLabel(tr("No Backup"));
    m_labelEmptyText->setObjectName("yhkylin-backup-tools_kybackup_QLabel_m_labelEmptyText");
    m_labelEmptyText->setEnabled(false);
    m_labelEmpty->setVisible(false);
    m_labelEmptyText->setVisible(false);

    QHBoxLayout *layoutEmptyPng = new QHBoxLayout;
    layoutEmptyPng->addStretch();
    layoutEmptyPng->addWidget(m_labelEmpty);
    layoutEmptyPng->addStretch();
    QHBoxLayout *layoutEmptyText = new QHBoxLayout;
    layoutEmptyText->addStretch();
    layoutEmptyText->addWidget(m_labelEmptyText);
    layoutEmptyText->addStretch();

    QVBoxLayout *layoutTable = new QVBoxLayout;
    layoutTable->addStretch();
    layoutTable->addLayout(layoutEmptyPng);
    layoutTable->addLayout(layoutEmptyText);
    layoutTable->addStretch();
    m_tableWidget->setLayout(layoutTable);
}

BackupPointListDialog::~BackupPointListDialog()
{
    delete m_udector;
    delete ui;
}

/**
 * @brief 获取本地和移动设备中的备份点
 * @return
 */
QList<ParseBackupList::BackupPoint> BackupPointListDialog::getBackupPointList()
{
    QList<ParseBackupList::BackupPoint> list;

    // 获取用户手动备份点信息
    qDebug()<<"get backup point list";
    QString backupPath = Utils::getBackupRootPath() + SYS_BACKUP_XML_PATH;
    backupPath.replace("//", "/");
    QFile xmlFile(backupPath);
    ParseBackupList parse(backupPath);
    list.append(parse.getBackupPointList());

    // 获取用户数据备份点信息
    QString xmlPath = Utils::getBackupRootPath() + USER_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse1(xmlPath);
    list.append(parse1.getBackupPointList());

    // 获取出厂备份点信息
    QString factoryXmlPath = Utils::getBackupRootPath() + FACTORY_BACKUP_XML_PATH;
    factoryXmlPath.replace("//", "/");
    ParseBackupList parse2(factoryXmlPath);
    list.append(parse2.getBackupPointList());

    //获取USB 移动设备数据备份点信息
    foreach (const QString &str, this->m_udiskPaths)
    {
        if(!str.contains("media"))
            continue;

        qDebug() << "Checking USB device: " << str;
        // 构建USB路径
        QString tmpDisk = str + "/.backup/snapshots/";
        QDir targetDir(tmpDisk);
        if (!targetDir.exists()) {
            qDebug() << "Directory not found:" << targetDir.path();
            continue;
        }
        QString xmlPath = tmpDisk + UDISK_BACKUP_XML_PATH;
        xmlPath.replace("//", "/");

        qDebug()<<"xml path is "<< xmlPath;
        QFileInfo fileInfo(xmlPath);
        if (!fileInfo.exists())
        {
            qWarning() << "backup point file not exist:" << xmlPath;
        }
        else
        {
            ParseBackupList parse1(xmlPath);
            qDebug()<< "list size is "<< parse1.getBackupPointList().size();
            list.append(parse1.getBackupPointList());
        }


        QString udiskSysXmlPath = tmpDisk + UDISK_SYSBACKUP_XML_PATH;
        udiskSysXmlPath.replace("//", "/");

        qDebug()<<"xml path is "<< udiskSysXmlPath;
        QFileInfo fileInfo1(udiskSysXmlPath);
        if (!fileInfo1.exists()) {
            qWarning() << "backup point file not exist:" << udiskSysXmlPath;
        }
        else
        {
            ParseBackupList parse2(udiskSysXmlPath);
            qDebug()<< "list size is "<< parse2.getBackupPointList().size();
            list.append(parse2.getBackupPointList());
        }

    }
    return list;
}

/**
 * @brief 屏蔽部分键盘
 * @param event
 * @note 没有限制住QTableWidget子控件接收按键输入
 */
void BackupPointListDialog::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        this->close();
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
        //忽略，什么都不做
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

/**
 * @brief 设置QTableWidget的单元格内容
 * @param row
 * @param column
 * @param text
 */
void BackupPointListDialog::setItem(int row, int column, const QString& text, int alignFlag)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(text);
    item->setToolTip(text);
    item->setTextAlignment(alignFlag);
    m_tableWidget->setItem(row, column, item);
}

/**
 * @brief 获取单元格的内容
 * @param row
 * @param column
 * @return QString，单元格的内容
 */
QString BackupPointListDialog::text(int row, int column)
{
    QTableWidgetItem *item = m_tableWidget->item(row, column);
    if (item)
        return item->text();
    else
        return "";
}

/**
 * @brief 检测是否是U盘
 * @param mountPath
 * @return bool，是U盘
 */
bool BackupPointListDialog::isUsbDevice(const QString &mountPath)
{
    return mountPath.startsWith("/media/") ||
           mountPath.startsWith("/run/media/") ||
           mountPath.startsWith("/mnt/");
}

void BackupPointListDialog::paintEvent(QPaintEvent *event)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, palette.color(QPalette::Base));
    this->setPalette(palette);
    QDialog::paintEvent(event);
}

void BackupPointListDialog::clearSortIndicator()
{
    auto header = m_tableWidget ? m_tableWidget->horizontalHeader() : nullptr;
    if (!header || !header->isSortIndicatorShown()){
        return;
    }
    header->setSortIndicatorShown(false);
    header->update();
}

void BackupPointListDialog::mousePressEvent(QMouseEvent *event)
{
    auto header = m_tableWidget ? m_tableWidget->horizontalHeader() : nullptr;
    if (header) {
        const QPoint localPos = header->mapFrom(this, event->pos());
        if (!header->rect().contains(localPos)){
            clearSortIndicator();
        }
    }
    QDialog::mousePressEvent(event);
}
