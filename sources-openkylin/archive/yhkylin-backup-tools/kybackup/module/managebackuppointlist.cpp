/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "managebackuppointlist.h"
#include <QSettings>
#include <QTextCodec>
#include <unistd.h>
#include "component/mylabel.h"
#include "../../common/utils.h"
#include "../globalbackupinfo.h"
#include "../deletebackupdialog.h"
#include "messageboxutils.h"

ManageBackupPointList::ManageBackupPointList(QWidget *parent, BackupPointType backupType) :
    BackupPointListDialog(parent),
    m_backupType(backupType)
{
    // 【窗口】焦点不在窗口上时不显示tooltips
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    if (BackupPointType::SYSTEM == backupType) {
        this->setWindowTitle(tr("System Backup Management"));
        m_titleLabel->setDeplayText(tr("System Backup Management"));
    } else {
        this->setWindowTitle(tr("Data Backup Management"));
        m_titleLabel->setDeplayText(tr("Data Backup Management"));
    }

    // 隐藏备份点大小Backup Size字段
    // m_tableWidget->hideColumn(Column_Index::Backup_Size);
    initTableWidget();

    // 您可以删除不需要的备份，更多细节请参考“操作日志”
    MyLabel *labelTips = new MyLabel;
    labelTips->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelTips");
    labelTips->setIsOriginal(true);
    labelTips->setWordWrap(true);
    labelTips->setMinimumWidth(600);
    labelTips->setDeplayText(tr("You can delete the backup that does not need, refer operation logs for more details"));

    m_buttonDelete = new MyPushButton;
    m_buttonDelete->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_m_buttonDelete");
    m_buttonDelete->setText(tr("Delete"));
    m_buttonDelete->setProperty("isImportant", true);
    m_buttonDelete->setEnabled(false);

    m_bottomLayout->addWidget(labelTips);
    m_bottomLayout->addStretch();
    m_bottomLayout->addWidget(m_buttonDelete);

    connect(m_tableWidget, &QTableWidget::clicked, this, [=]() {
        this->m_buttonDelete->setEnabled(true);
    });

    connect(m_buttonDelete, &MyPushButton::clicked, this, [=](){
        // 判断是否已经选中备份点
        QList<QTableWidgetItem *> selectList = this->m_tableWidget->selectedItems();
        if (selectList.isEmpty()) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), QObject::tr("Please select one backup to continue."), QObject::tr("OK"));
            return;
        }

        // 是否确认删除此备份
        if (MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(this, QObject::tr("Information"), QObject::tr("Are you sure to delete the backup ?"), QObject::tr("OK"), QObject::tr("Cancel"))) {
            int curRow = this->m_tableWidget->currentRow();
            ParseBackupList::BackupPoint backupPoint;
            backupPoint.m_backupName = this->text(curRow, Column_Index::Backup_Name);
            backupPoint.m_uuid = this->text(curRow, Column_Index::UUID);
            backupPoint.m_time = this->text(curRow, Column_Index::Backup_Time);
            backupPoint.m_path = this->text(curRow, Column_Index::Prefix_Path);
            QString position = this->text(curRow, Column_Index::Backup_Device);
            backupPoint.m_iPosition = position.startsWith(QObject::tr("Customize:")) ? BackupPosition::CUSTOMIZE :
                                                                                       (position.startsWith(QObject::tr("Local Disk:")) ? BackupPosition::LOCAL : BackupPosition::UDISK);
            m_deleteRow = curRow;

            emit selected(backupPoint);
        }
    });

    connect(this, &ManageBackupPointList::selected, this, &ManageBackupPointList::deleteBackupPoint);
    connect(this, &ManageBackupPointList::udiskChange, this, &ManageBackupPointList::initTableWidget);
    connect(&m_dateUtils, &DateTimeUtils::ShortDateSignal, this, &ManageBackupPointList::initTableWidget);
}

ManageBackupPointList::~ManageBackupPointList()
{

}

void ManageBackupPointList::initTableWidget()
{
    m_tableWidget->clearContents();
    m_tableWidget->setRowCount(0);
    m_tableWidget->setSortingEnabled(false); // 等录入完数据后再排序
    insertLines(getBackupPointList());
}

void ManageBackupPointList::insertLines(const QList<ParseBackupList::BackupPoint> &backupPoints)
{
    QString settingPath = Utils::getSysRootPath() + UDISK_UNIQUE_SETTINGS;
    settingPath.replace("//", "/");
    QSettings udisk_setting(settingPath, QSettings::IniFormat);
    udisk_setting.setIniCodec(QTextCodec::codecForName("utf-8"));

    int indexOfRow = 0;
    QString preDevPath(Utils::getSysRootPath());
    preDevPath += "/media";
    preDevPath.replace("//", "/");

    for (const ParseBackupList::BackupPoint& backupPoint : backupPoints) {
        // 系统备份排除非系统备份且非增量系统备份
        if (BackupPointType::SYSTEM == m_backupType &&
            int(BackupType::BACKUP_SYSTEM) != backupPoint.m_type &&
            int(BackupType::INC_BACKUP_SYSTEM) != backupPoint.m_type)
            continue;
        else if (BackupPointType::DATA == m_backupType &&
                 int(BackupType::BACKUP_DATA) != backupPoint.m_type &&
                 int(BackupType::INC_BACKUP_DATA) != backupPoint.m_type)
            continue;
        else if (backupPoint.m_state != BACKUP_PARSE_STATE_SUCCESS_STRTING)
            continue;

        // 数据备份需要用户隔离
        if (BackupPointType::DATA == m_backupType) {
            // 管理员可看自己用户进行的数据备份和无用户信息的数据备份（旧备份数据）
            if (GlobelBackupInfo::instance().isManager() && backupPoint.m_userId.isEmpty())
                continue ;

            // 标准用户只能看自己进行的操作
            if (!GlobelBackupInfo::instance().isManager() && backupPoint.m_userId != qgetenv("USER"))
                continue ;
        }

        m_tableWidget->insertRow(indexOfRow);
        if(backupPoint.m_uuid == "00000000-0000-0000-0000-000000000000")
        {
            setItem(indexOfRow, Column_Index::Backup_Type,tr("factory backup"));
        }
        else
        {
            if(backupPoint.m_iPosition == 1){
                qDebug()<<"begin to check posion it's 1";
                setItem(indexOfRow, Column_Index::Backup_Type,tr("Removable Device"));
            } else {
                setItem(indexOfRow, Column_Index::Backup_Type,tr("Manual Backup"));
            }
        }

        setItem(indexOfRow, Column_Index::Backup_Name, backupPoint.m_backupName);
        setItem(indexOfRow, Column_Index::UUID, backupPoint.m_uuid);
        setItem(indexOfRow, Column_Index::Backup_Time, m_dateUtils.TranslateDateFormat(backupPoint.m_time));
        setItem(indexOfRow, Column_Index::Backup_Size, backupPoint.m_size);

        QString prefixPath_to_device;
        if (backupPoint.m_iPosition == BackupPosition::CUSTOMIZE) {
            prefixPath_to_device = QObject::tr("Customize:") + LOCAL_DEFAULT_PATH;
        } else if (backupPoint.m_iPosition == BackupPosition::UDISK) {
            QStorageInfo storage(backupPoint.m_path);
            QString udiskName = storage.rootPath();
            prefixPath_to_device = QObject::tr("Udisk Device:") + " " + udiskName + BACKUP_SNAPSHOTS_PATH;
        } else {
            prefixPath_to_device = QObject::tr("Local Disk:") + " " + BACKUP_SNAPSHOTS_PATH;
        }
        prefixPath_to_device.replace("//", "/");
        setItem(indexOfRow, Column_Index::Backup_Device, prefixPath_to_device);
        if (backupPoint.m_state == BACKUP_PARSE_STATE_SUCCESS_STRTING) {
            setItem(indexOfRow, Column_Index::Backup_State, tr("backup finished"));
        } else {
            setItem(indexOfRow, Column_Index::Backup_State, tr("backup unfinished"));
        }
        setItem(indexOfRow, Column_Index::Prefix_Path, backupPoint.m_path);

        ++indexOfRow;
    }

    if (0 < indexOfRow) {
        m_tableWidget->setSortingEnabled(true); // 等录入完数据后再排序
        m_tableWidget->sortItems(Column_Index::Backup_Time, Qt::DescendingOrder);
        m_labelEmpty->setVisible(false);
        m_labelEmptyText->setVisible(false);
        m_tableWidget->horizontalHeader()->setVisible(true);
    } else {
        m_labelEmpty->setVisible(true);
        m_labelEmptyText->setVisible(true);
        m_tableWidget->horizontalHeader()->setVisible(false);
    }
}

/**
 * @brief 删除备份点
 * @param backupPonit
 */
void ManageBackupPointList::deleteBackupPoint(ParseBackupList::BackupPoint backupPonit)
{
    qDebug()<<"m_backupType value "<< m_backupType;
    DeleteBackupDialog *deleteDialog = new DeleteBackupDialog(backupPonit, this,m_backupType);
    deleteDialog->setObjectName("yhkylin-backup-tools_kybackup_DeleteBackupDialog_deleteDialog");
    connect(deleteDialog, &DeleteBackupDialog::deleteFinished, this, [=](bool result) {
        if (result)
            this->m_tableWidget->removeRow(m_deleteRow);
        m_deleteRow = -1;
        if (this->m_tableWidget->rowCount() == 0) {
            m_labelEmpty->setVisible(true);
            m_labelEmptyText->setVisible(true);
            m_tableWidget->horizontalHeader()->setVisible(false);
            this->m_buttonDelete->setEnabled(false);
        }
    });
    deleteDialog->exec();
    deleteDialog->deleteLater();
}


