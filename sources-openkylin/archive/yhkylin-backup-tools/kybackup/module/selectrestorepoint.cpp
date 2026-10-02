/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "selectrestorepoint.h"
#include <QSettings>
#include <QTextCodec>
#include "component/mypushbutton.h"
#include "../../common/utils.h"
#include "messageboxutils.h"
#include "../globalbackupinfo.h"
#include <QDebug>

SelectRestorePoint::SelectRestorePoint(QWidget *parent, BackupPointType backupType, bool isOnlyShowLocal,bool isghost) :
    BackupPointListDialog(parent, isOnlyShowLocal),
    m_backupType(backupType)
{
    m_isghost=isghost;
    if (BackupPointType::SYSTEM == backupType) {
        this->setWindowTitle(tr("System Backup Management"));
        m_titleLabel->setDeplayText(tr("System Backup Management"));
    } else {
        this->setWindowTitle(tr("Data Backup Management"));
        m_titleLabel->setDeplayText(tr("Data Backup Management"));
    }

    // 隐藏备份点状态Backup State字段
    m_tableWidget->hideColumn(Column_Index::Backup_State);
    initTableWidget();

    // 刷新
//    MyPushButton * buttonRefresh = new MyPushButton;
//    buttonRefresh->setText(tr("Refresh"));

    // 确定按钮
    MyPushButton * buttonOk = new MyPushButton;
    buttonOk->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_buttonOk");
    buttonOk->setText(tr("OK"));
    this->setResult(QDialog::Rejected);
    buttonOk->setProperty("isImportant", true);
    buttonOk->setEnabled(false);

    m_bottomLayout->addStretch();
//    m_bottomLayout->addWidget(buttonRefresh);
    m_bottomLayout->addSpacing(10);
    m_bottomLayout->addWidget(buttonOk);

    connect(m_tableWidget, &QTableWidget::clicked, this, [=]() {
        buttonOk->setEnabled(true);
    });
//    connect(buttonRefresh, &MyPushButton::clicked, this, &SelectRestorePoint::initTableWidget);
    connect(this, &SelectRestorePoint::udiskChange, this, &SelectRestorePoint::initTableWidget);
    connect(&m_dateUtils, &DateTimeUtils::ShortDateSignal, this, &SelectRestorePoint::initTableWidget);

    connect(buttonOk, &MyPushButton::clicked, this, [=](){
        // 判断是否已经选中备份点
        QList<QTableWidgetItem *> selectList = this->m_tableWidget->selectedItems();
        if (selectList.isEmpty()) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), QObject::tr("Please select one backup to continue."), QObject::tr("OK"));
            // this->reject();
            return;
        }

        // 还原相应备份点
        if (!this->m_needConfirm || MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(this, QObject::tr("Information"), QObject::tr("Do you want to continue?"), QObject::tr("Continue"), QObject::tr("Cancel"))) {
            int curRow = this->m_tableWidget->currentRow();
            ParseBackupList::BackupPoint backupPoint;
            backupPoint.m_backupName = this->text(curRow, Column_Index::Backup_Name);
            backupPoint.m_uuid = this->text(curRow, Column_Index::UUID);
            backupPoint.m_time = this->text(curRow, Column_Index::Backup_Time);
            backupPoint.m_path = this->text(curRow, Column_Index::Prefix_Path);
            backupPoint.m_size = this->text(curRow, Column_Index::Backup_Size);
            QString dev = this->text(curRow, Column_Index::Backup_Device);

            if(dev.contains("media")){
                backupPoint.m_iPosition = BackupPosition::UDISK;
            } else if(dev.contains("/data/.backup")){
                backupPoint.m_iPosition = BackupPosition::LOCAL;
            } else{
                backupPoint.m_iPosition = BackupPosition::CUSTOMIZE;
            }

            qDebug()<<"position is "<< backupPoint.m_iPosition;
            qDebug()<<"dev is "<< dev;
            emit this->selected(backupPoint);
            this->accept();
        }
    });
}

SelectRestorePoint::~SelectRestorePoint()
{}

void SelectRestorePoint::initTableWidget()
{
    m_tableWidget->clearContents();
    m_tableWidget->setRowCount(0);
    m_tableWidget->setSortingEnabled(false); // 等录入完数据后再排序
    insertLines(getBackupPointList());
}

void SelectRestorePoint::insertLines(const QList<ParseBackupList::BackupPoint> &backupPoints)
{
    QString settingPath = Utils::getSysRootPath() + UDISK_UNIQUE_SETTINGS;
    settingPath.replace("//", "/");
    QSettings udisk_setting(settingPath, QSettings::IniFormat);
    udisk_setting.setIniCodec(QTextCodec::codecForName("utf-8"));

    int indexOfRow = 0;
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

        // 数据备份需要用户隔离
        if (BackupPointType::DATA == m_backupType)
        {
            qDebug() << "backupPoint.m_userId : " << backupPoint.m_userId ;
            qDebug() << "qgetenv : " << qgetenv("USER") ;
            // 管理员可看自己用户进行的数据备份和无用户信息的数据备份（旧备份数据）
            if (GlobelBackupInfo::instance().isManager() && backupPoint.m_userId.isEmpty())
                continue ;

            // 标准用户只能看自己进行的操作
            if (!GlobelBackupInfo::instance().isManager() && backupPoint.m_userId != qgetenv("USER"))
                continue ;
        }

        // 不展示未完成的备份点
        if (BACKUP_PARSE_STATE_SUCCESS_STRTING != backupPoint.m_state)
            continue;

        //hide factory backup
        // 华为机型向主线SP1看齐，出厂备份相关的保持和主线一致
//        if (backupPoint.m_uuid == FACTORY_BACKUP_UUID && Utils::isHuawei990())
//            continue;

        bool isOther = false;
        QString preDevPath(Utils::getSysRootPath());
        preDevPath += "/media";
        preDevPath.replace("//", "/");

        if (m_isghost == true){
            if (backupPoint.m_uuid == AUTO_BACKUP_UUID || backupPoint.m_uuid == CUSTOMIZEKEYBOARD_SYSTEM_BACK_UP_UUID){
                continue;
            }
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

        qDebug()<<"mpath is "<<backupPoint.m_path;
        QString prefixPath_to_device;
        if (backupPoint.m_iPosition == BackupPosition::CUSTOMIZE) {
            prefixPath_to_device = QObject::tr("Customize:") + backupPoint.m_path + DATA_BACKUP_PATH;
        } else if (backupPoint.m_path.contains(preDevPath)) {
            QStorageInfo storage(backupPoint.m_path);
            QString udiskName = storage.rootPath();
            if (isOther){
                prefixPath_to_device = QObject::tr("Other machine:") + " " + udiskName + DATA_BACKUP_PATH;
            }
            else{
                qDebug()<<"shows udisk device";
                prefixPath_to_device = QObject::tr("Udisk Device:") + " " + udiskName + DATA_BACKUP_PATH;
            }
        } else {
            qDebug()<<"shows local disk";
            prefixPath_to_device = QObject::tr("Local Disk:") + " " + LOCAL_DEFAULT_PATH;
        }
        prefixPath_to_device.replace("//", "/");
        setItem(indexOfRow, Column_Index::Backup_Device, prefixPath_to_device);
        setItem(indexOfRow, Column_Index::Backup_State, backupPoint.m_state);
        setItem(indexOfRow, Column_Index::Prefix_Path, backupPoint.m_path);

        ++indexOfRow;
    }

    if (0 < indexOfRow) {
        m_tableWidget->setSortingEnabled(true); // 等录入完数据后再排序
        m_tableWidget->sortItems(Column_Index::Backup_Time, Qt::DescendingOrder);
        m_tableWidget->horizontalHeader()->setVisible(true);
    } else {
        m_labelEmpty->setVisible(true);
        m_labelEmptyText->setVisible(true);
        m_tableWidget->horizontalHeader()->setVisible(false);
    }
}
