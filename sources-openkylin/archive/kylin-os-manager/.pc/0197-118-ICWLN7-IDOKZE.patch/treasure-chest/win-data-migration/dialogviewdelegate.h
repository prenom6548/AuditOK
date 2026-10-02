#ifndef FONTLISTDELEGATE_H
#define FONTLISTDELEGATE_H

#include <QAbstractItemDelegate>
#include <QPoint>
#include <QPainter>
#include <QStandardItemModel>

class DialogViewDelegate : public QAbstractItemDelegate
{
public:
    DialogViewDelegate();
    ~DialogViewDelegate();
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // FONTLISTDELEGATE_H
