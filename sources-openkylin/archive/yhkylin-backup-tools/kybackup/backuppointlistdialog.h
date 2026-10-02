#ifndef BackupPointListDialog_H
#define BackupPointListDialog_H

#include <QDialog>
#include <QHeaderView>
#include <QTableWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "../backup-daemon/parsebackuplist.h"
#include "module/udiskdetector.h"
#include "component/pixmaplabel.h"
#include "component/mylabel.h"

namespace Ui {
class BackupPointListDialog;
}

class BackupPointListDialog : public QDialog
{
    Q_OBJECT
public:
    enum Column_Index {
        Backup_Name = 0,
        Backup_Type,
        UUID,
        Backup_Time,
        Backup_Size,
        Backup_Device, // 即备份位置Position
        Backup_State,
        Prefix_Path
    };

    explicit BackupPointListDialog(QWidget *parent = nullptr, bool isOnlyShowLocal = false);
    virtual ~BackupPointListDialog();

    QList<ParseBackupList::BackupPoint> getBackupPointList();

    void setIsOnlyShowLocal(bool onlyShowLocal) { m_onlyShowLocal = onlyShowLocal; }
    bool isOnlyShowLocal() const { return m_onlyShowLocal; }
    bool isUsbDevice(const QString &mountPath);
    void paintEvent(QPaintEvent *event);

signals:
    void udiskChange();
    void selected(ParseBackupList::BackupPoint);

protected:
    void keyPressEvent(QKeyEvent* event);

    void setItem(int row, int column, const QString& text, int alignFlag = Qt::AlignLeft | Qt::AlignVCenter);
    QString text(int row, int column);

    QTableWidget *m_tableWidget;
    QHBoxLayout *m_bottomLayout = nullptr;
    PixmapLabel *m_labelEmpty; // 空记录图片
    QLabel *m_labelEmptyText; // 空记录文本
    MyLabel *m_titleLabel; // 窗口标题
    void mousePressEvent(QMouseEvent *event) override;
private:
    Ui::BackupPointListDialog *ui;

    // U盘探测
    UdiskDetector* m_udector;
    // U盘挂载路径列表
    QStringList m_udiskPaths;
    // 是否只展示本地备份
    bool m_onlyShowLocal;
    void clearSortIndicator();
};

#endif // BackupPointListDialog_H
