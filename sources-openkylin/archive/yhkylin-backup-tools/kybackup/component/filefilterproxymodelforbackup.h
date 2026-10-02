#ifndef FILEFILTERPROXYMODELFORBACKUP_H
#define FILEFILTERPROXYMODELFORBACKUP_H

#include <QSortFilterProxyModel>

class FileFilterProxyModeForBackup : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    FileFilterProxyModeForBackup(QObject *parent = nullptr);
    ~FileFilterProxyModeForBackup();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};

#endif // FILEFILTERPROXYMODELFORBACKUP_H
