#ifndef SELECTRESTOREPOINT_H
#define SELECTRESTOREPOINT_H

#include "../backuppointlistdialog.h"
#include "../datetimeutils.h"

class SelectRestorePoint : public BackupPointListDialog
{
    Q_OBJECT
public:
    enum BackupPointType {
        DATA,
        SYSTEM
    };

    explicit SelectRestorePoint(QWidget *parent = nullptr, BackupPointType backupType = BackupPointType::SYSTEM, bool isOnlyShowLocal = false,bool isghost = false);
    virtual ~SelectRestorePoint();

    void setNeedConfirm(bool needConfirm) {
        m_needConfirm = needConfirm;
    }

public slots:
    void initTableWidget();

private:
    void insertLines(const QList<ParseBackupList::BackupPoint> &backupPoints);

private:
    BackupPointType m_backupType;
    bool m_needConfirm = true;
    DateTimeUtils m_dateUtils;
    bool m_isghost;
};

#endif // SELECTRESTOREPOINT_H
