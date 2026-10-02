#ifndef DELETEBACKUPDIALOG_H
#define DELETEBACKUPDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QMovie>
#include "component/mylabel.h"
#include "component/mypushbutton.h"
#include "component/pixmaplabel.h"
#include "backup_manager_interface.h"
#include "../backup-daemon/parsebackuplist.h"

namespace Ui {
class DeleteBackupDialog;
}

class DeleteBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteBackupDialog(ParseBackupList::BackupPoint backupPonit, QWidget *parent = nullptr);
    ~DeleteBackupDialog();

    void paintEvent(QPaintEvent *event);

signals:
    void deleteFinished(bool result);

public slots:
    void on_checkEnv_end(int result);
    void on_deleteBackup_end(bool result);

private:
    void deleteBackupPoint();

    Ui::DeleteBackupDialog *ui;
    ParseBackupList::BackupPoint m_backupPonit;

    PixmapLabel *m_mTitleIcon;
    MyLabel *m_titleLabel;
    QPushButton *m_buttonClose;
    MyLabel *m_labelMessage;
    QLabel *m_loadingGif;
    QMovie *m_movie;
    MyPushButton *m_buttonOk;

    // dbus接口
    ComKylinBackupManagerInterface *m_pInterface;
};

#endif // DELETEBACKUPDIALOG_H
