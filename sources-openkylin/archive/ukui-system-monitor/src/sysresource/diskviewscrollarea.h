#ifndef DISKVIEWSCROLLAREA_H
#define DISKVIEWSCROLLAREA_H

#include <QMap>
#include <QScrollArea>

class DiskItemWidget;
class DiskViewScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit DiskViewScrollArea(QWidget *parent = nullptr);

    QString getCurrentDiskName();

signals:
    void changeInfo(const QString &deviceName);

protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void updateWidgetGeometry();
    void onUpdateData();
    void fontChanged(const QFont &font);
    void onSetItemStatus(const QString &deviceName);

private:
    void showItemOne();
    void showItemTwo();
    void showItemLgTwo(int count);

private:
    QStringList m_listDevice;
    QList<DiskItemWidget *> m_listBlockItemWidget;
    QMap<QString, DiskItemWidget *> m_mapDeviceItemWidget;
    QWidget *m_pCentralWidget;
    QFont m_font;
    QString m_diskName;
};

#endif // DISKVIEWSCROLLAREA_H
