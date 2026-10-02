#ifndef MANAGEBACKUPPOINTLIST_H
#define MANAGEBACKUPPOINTLIST_H

#include "../backuppointlistdialog.h"
#include "component/mypushbutton.h"
#include "../datetimeutils.h"

class ManageBackupPointList : public BackupPointListDialog
{
    Q_OBJECT
public:
    enum BackupPointType {
        DATA,
        SYSTEM
    };

    explicit ManageBackupPointList(QWidget *parent = nullptr, BackupPointType backupType = BackupPointType::SYSTEM);
    virtual ~ManageBackupPointList();

public slots:
    void initTableWidget();
    void deleteBackupPoint(ParseBackupList::BackupPoint backupPonit);

private:
    void insertLines(const QList<ParseBackupList::BackupPoint> &backupPoints);

private:
    // 备份点类型，如系统备份、数据备份
    BackupPointType m_backupType;
    // 删除的行索引
    int m_deleteRow = -1;
    // 删除按钮
    MyPushButton * m_buttonDelete;
    DateTimeUtils m_dateUtils;
};

#endif // MANAGEBACKUPPOINTLIST_H
