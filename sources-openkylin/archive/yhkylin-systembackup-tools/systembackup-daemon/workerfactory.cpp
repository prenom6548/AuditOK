/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

 #include "workerfactory.h"
 #include <QString>
 
 Worker::Worker() :
     QObject(nullptr),
     m_bCancel(false)
 {}
 
 Worker::~Worker()
 {}
 
 //
 /**
  * @brief 环境检测
  */
 void Worker::checkEnv()
 {
     checkEnvEx();
 }
 
 /**
  * @brief 任务处理
  */
 void Worker::doWork()
 {
     doWorkEx();
 }
 
 // 环境检测，个性化部分派生类去实现
 bool Worker::checkEnvEx()
 {
     return true;
 }
 
 // 任务处理，个性化部分派生类去实现
 void Worker::doWorkEx()
 {}
 
 // 任务取消，个性化部分派生类去实现
 void Worker::cancelEx()
 {}
 
 /**
  * @brief 根据操作类型和位置创建相应的处理类
  * @param type 操作类型，如：系统备份, 系统还原
  * @param position 本地备份还是U盘备份: 0-本地备份；1-移动设备备份
  * @return Worker派生类对象指针，需调用者释放此对象
  */
 Worker * WorkerFactory::createWorker(int type, int position, int user )
 {
     QString className;
     switch (type)
     {
     case BackupType::BACKUP_SYSTEM:
         className = "SystemBackupProxy";
         break;
     case BackupType::INC_BACKUP_SYSTEM:
         if (BackupPosition::UDISK == position) {
             className = "UDiskSystemBackupProxy";
         } else if (BackupPosition::CUSTOMIZE == position) {
             className = "CustomizeSystemBackupProxy";
         } else {
             className = "SystemBackupProxy";
         }
         break;
     case BackupType::AUTO_BACKUP_SYSTEM:
         if (BackupPosition::CUSTOMIZE == position) {
             className = "AutoSystemBackupProxy";
         }
         break;
     case BackupType::RESTORE_SYSTEM:
     case BackupType::AUTO_RESTORE_SYSTEM:
     case BackupType::RESTORE_SYSTEM_WITH_DATA:
         /*if (BackupPosition::UDISK == position || BackupPosition::OTHER == position) {
             className = "UDiskSystemRestoreProxy";
         } else */
         if (BackupUser::AUTO == user) {
             className = "AutoSystemRestoreProxy";
         } else if (BackupUser::USER == user){
             className = "SystemRestoreProxy";
         }
         break;
     case BackupType::BACKUP_DATA:
         className = "DataBackupProxy";
         break;
     case BackupType::INC_BACKUP_DATA:
         if (BackupPosition::UDISK == position) {
             className = "UDiskDataBackupProxy";
         } else if (BackupPosition::CUSTOMIZE == position) {
             className = "CustomizeDataBackupProxy";
         } else {
             className = "DataBackupProxy";
         }
         break;
     case BackupType::RESTORE_DATA:
         className = "DataRestoreProxy";
         /*
         if (BackupPosition::UDISK == position) {
             className = "UDiskDataRestoreProxy";
         } else {
             className = "DataRestoreProxy";
         }*/
         break;
     case BackupType::DELETE_BACKUP:
         className = "DeleteBackupProxy";
         break;
     case BackupType::GHOST_IMAGE:
         className = "GhostImageProxy";
         /*
         if (BackupPosition::UDISK == position) {
             className = "UDiskGhostImageProxy";
         } else if (BackupPosition::CUSTOMIZE == position) {
             className = "CustomizeGhostImageProxy";
         } else {
             className = "GhostImageProxy";
         }*/
         break;
     default:
         break;
     }
 
     if (className.isEmpty()) {
         return nullptr;
     }
 
     Worker * p = qobject_cast<Worker *>(Reflect::createObject(className));
     return p;
 }
 
