#ifndef TRAYLISTVIEWDELEGATE_H
#define TRAYLISTVIEWDELEGATE_H

#include <QStyledItemDelegate>

class TrayListViewDelegate : public QStyledItemDelegate
{
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // TRAYLISTVIEWDELEGATE_H
