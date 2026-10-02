#ifndef FILEVIEWDELEGATE_H
#define FILEVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <kysdk/applications/klistviewdelegate.h>

class FileViewDelegate : public kdk::KListViewDelegate
{
public:
    explicit FileViewDelegate(QAbstractItemView *parent = nullptr);
    ~FileViewDelegate();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override; //重写绘制事件
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void editModePaint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // FILEVIEWDELEGATE_H
