#ifndef DISKDETAILSMODEL_H
#define DISKDETAILSMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QPalette>

#include "commoninfo.h"
using namespace commom::format;

typedef struct _diskInfo
{
    QString model;
    QString size;
    QString readSpeed;
    QString writeSpeed;
    QString type;

} diskShowInfo;

class DiskDetailsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    DiskDetailsModel(QObject *parent = nullptr);
    ~DiskDetailsModel();

    /**
     * @brief refreshDiskInfo  刷新磁盘信息
     * @param strKey       磁盘名称信息
     */
    void refreshDiskInfo(const QString &strKey);

private:
    diskShowInfo m_showinfo;

protected:
    int rowCount(const QModelIndex &) const
    {
        return 3;
    }

    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::NoItemFlags;
    }

public slots:
    void onModelUpdated();
};

#endif // DISKDETAILSMODEL_H
