/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "operationlog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSettings>
#include <QTextCodec>
#include "globalbackupinfo.h"
#include "gsettingswrapper.h"

IMPLEMENT_DYNCREATE_WITH_ONE_PARAM(OperationLog, QStackedWidget*, QWidget*)

OperationLog::OperationLog(QWidget *parent) :
    QStackedWidget(parent)
{
    // 界面手写代码创建，作为练手
    initHomePage();
}

OperationLog::~OperationLog()
{}

/**
 * @brief 第一页
 */
void OperationLog::initHomePage()
{
    QWidget *homePage = new QWidget;
    homePage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_homePage");

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(40, 20, 40, 40);
    vlayout->setAlignment(Qt::AlignCenter);

    // 列表为空时展示图片
    QHBoxLayout *hlayoutLine1 = new QHBoxLayout;
    labelEmptyLogo = new PixmapLabel(this);
    labelEmptyLogo->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_labelEmptyLogo");
    labelEmptyLogo->setLightAndDarkPixmap(":/images/empty.png", ":/images/empty_dark.png");
    hlayoutLine1->addStretch();
    hlayoutLine1->addWidget(labelEmptyLogo);
    hlayoutLine1->addStretch();
    vlayout->addLayout(hlayoutLine1);

    QHBoxLayout *hlayoutLine2 = new QHBoxLayout;
    labelEmptyText = new QLabel(tr("No operation log"));
    labelEmptyText->setObjectName("yhkylin-backup-tools_kybackup_QLabel_labelEmptyText");
    labelEmptyText->setAlignment(Qt::AlignCenter);
    labelEmptyText->setEnabled(false);
    hlayoutLine2->addStretch();
    hlayoutLine2->addWidget(labelEmptyText);
    hlayoutLine2->addStretch();
    vlayout->addLayout(hlayoutLine2);

    QHBoxLayout *hlayoutLine3 = new QHBoxLayout;
    QStringList headerLabels;
    headerLabels << tr("Backup Name") << tr("UUID") << tr("Operation") << tr("Operation Time");
    m_tableWidget = new QTableWidget;
    m_tableWidget->setObjectName("yhkylin-backup-tools_kybackup_QTableWidget_m_tableWidget");
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->setMinimumSize(680, 530);
    m_tableWidget->setColumnCount(headerLabels.size());
    m_tableWidget->setHorizontalHeaderLabels(headerLabels);
    m_tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    // m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->resizeSection(Column_Index::Backup_Name, 300);
    // m_tableWidget->horizontalHeader()->resizeSection(Column_Index::UUID, 310);
    m_tableWidget->horizontalHeader()->resizeSection(Column_Index::Operation, 190);
    m_tableWidget->horizontalHeader()->resizeSection(Column_Index::Operation_Time, 190);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->horizontalHeader()->setTextElideMode(Qt::ElideRight);
    m_tableWidget->horizontalHeader()->setFixedHeight(36);
    m_tableWidget->horizontalHeader()->setHighlightSections(false);
    m_tableWidget->verticalHeader()->setHidden(true); // 好像隐藏不掉
    m_tableWidget->verticalHeader()->setVisible(false); //左侧标题不可见
    m_tableWidget->verticalHeader()->setDefaultSectionSize(36); // 设置行高
    m_tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    // m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setFocusPolicy(Qt::NoFocus);
    m_tableWidget->setSortingEnabled(false); // 等录入完数据后再排序
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->hideColumn(1); // 隐藏uuid
    m_tableWidget->setFrameShape(QTableWidget::NoFrame);
    hlayoutLine3->addWidget(m_tableWidget);
    vlayout->addLayout(hlayoutLine3);

    homePage->setLayout(vlayout);

    connect(&m_dateUtils, &DateTimeUtils::ShortDateSignal, this, &OperationLog::updateTimeVolumn);

    QList<BackupWrapper> list = Utils::getBackupLogList();
    if (list.isEmpty()) {
        labelEmptyLogo->setVisible(true);
        labelEmptyText->setVisible(true);
        m_tableWidget->setVisible(false);
    } else {
        labelEmptyLogo->setVisible(false);
        labelEmptyText->setVisible(false);
        m_tableWidget->setVisible(true);
        initOperationLogs(list);
    }

    addWidget(homePage);
}

/**
 * @brief 初始化操作日志表格
 */
void OperationLog::initOperationLogs(const QList<BackupWrapper>& list)
{
    m_tableWidget->clearContents();
    m_tableWidget->setRowCount(0);
    m_tableWidget->setSortingEnabled(false); // 等录入完数据后再排序

    int indexOfRow = 0;
    for (const BackupWrapper& backupPoint : list) {
        // 管理员可看自己用户进行的操作和无用户信息的数据备份（旧备份数据）
        if (GlobelBackupInfo::instance().isManager() && (backupPoint.m_frontUid != -1) && backupPoint.m_frontUid != GlobelBackupInfo::instance().curUserId().toInt())
            continue ;

        // 标准用户只能看自己进行的操作
        if (!GlobelBackupInfo::instance().isManager() && backupPoint.m_frontUid != GlobelBackupInfo::instance().curUserId().toInt())
            continue ;

        m_tableWidget->insertRow(indexOfRow);
        setItem(indexOfRow, 0, backupPoint.m_backupName);
        setItem(indexOfRow, 1, backupPoint.m_uuid);
        setItem(indexOfRow, 2, castTypeToString(backupPoint.m_type));
        setItem(indexOfRow, 3, m_dateUtils.TranslateDateFormat(backupPoint.m_time));
        ++indexOfRow;
    }

    m_tableWidget->setSortingEnabled(true); // 等录入完数据后再排序
    m_tableWidget->sortItems(3, Qt::DescendingOrder);
}

/**
 * @brief 设置QTableWidget的单元格内容
 * @param row
 * @param column
 * @param text
 */
void OperationLog::setItem(int row, int column, const QString& text, int alignFlag)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(text);
    item->setToolTip(text);
    item->setTextAlignment(alignFlag);
    m_tableWidget->setItem(row, column, item);
}

/**
 * @brief 翻译操作类型
 * @param type
 * @return
 */
QString OperationLog::castTypeToString(int type)
{
    QString name;
    switch (type) {
    case BackupType::BACKUP_SYSTEM :
        name = tr("new system backup");

        break;
    case BackupType::INC_BACKUP_SYSTEM :
        name = tr("udpate system backup");

        break;
    case BackupType::BACKUP_DATA :
        name = tr("new data backup");

        break;
    case BackupType::INC_BACKUP_DATA :
        name = tr("update data backup");

        break;
    case BackupType::RESTORE_SYSTEM :
        name = tr("restore system");

        break;
    case BackupType::RESTORE_SYSTEM_WITH_DATA :
        name = tr("restore retaining user data");

        break;
    case BackupType::RESTORE_DATA :
        name = tr("restore data");

        break;
    case BackupType::DELETE_BACKUP :
        name = tr("delete backup");

        break;
    case BackupType::GHOST_IMAGE :
        name = tr("make ghost image");

        break;
    case BackupType::AUTO_BACKUP_SYSTEM :
        name = tr("auto backup system");

        break;
    case BackupType::AUTO_RESTORE_SYSTEM :
        name = tr("auto restore system");

        break;
    default:
        break;
    }

    return name;
}

void OperationLog::updateTimeVolumn()
{
    QList<BackupWrapper> list = Utils::getBackupLogList();
    if (list.isEmpty()){
        labelEmptyLogo->setVisible(true);
        labelEmptyText->setVisible(true);
        m_tableWidget->setVisible(false);
    }else{
        labelEmptyLogo->setVisible(false);
        labelEmptyText->setVisible(false);
        m_tableWidget->setVisible(true);
        initOperationLogs(list);
    }
}

void OperationLog::showEvent(QShowEvent *event)
{
    QStackedWidget::showEvent(event);
    updateTimeVolumn();
}


