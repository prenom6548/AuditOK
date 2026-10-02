#ifndef FONTUTIL_H
#define FONTUTIL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QGSettings>

class FontUtil
{
//    Q_OBJECT
public:
    FontUtil();
};

class FontListFilter: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FontListFilter(QSortFilterProxyModel *parent = nullptr);
    ~FontListFilter();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    bool filterAcceptsRow(int source_row , const QModelIndex &source_parent) const;
    bool compareString(QString rightStr, QString leftStr) const;

private:
    QString m_installPath;
};

#endif // FONTUTIL_H
