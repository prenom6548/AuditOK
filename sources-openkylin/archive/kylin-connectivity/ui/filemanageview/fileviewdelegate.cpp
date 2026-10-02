#include "fileviewdelegate.h"

#include <QString>
#include <QPainter>
#include <QFont>
#include <QApplication>
#include <QFontMetrics>
#include <QLabel>
#include <QDebug>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QListView>
#include <QHBoxLayout>
#include <QPainterPath>

#include "fileview.h"

FileViewDelegate::FileViewDelegate(QAbstractItemView *parent) : kdk::KListViewDelegate(parent)
{
}

FileViewDelegate::~FileViewDelegate()
{
}

void FileViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto *view = qobject_cast<const FileView *>(option.widget);
    double fontSize = view->getFontSize();
    QFont font;
    font.setPointSizeF(fontSize);
    painter->setFont(font);

    if (view->getItemMode() == FileView::ItemSelectMode::Default) {
        kdk::KListViewDelegate::paint(painter, option, index);
    } else {
        editModePaint(painter, option, index);
    }
}

QSize FileViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto *listView = qobject_cast<const FileView *>(option.widget);
    return QStyledItemDelegate::sizeHint(option, index);
}

void FileViewDelegate::editModePaint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    kdk::KListViewDelegate::paint(painter, option, index);
    auto *view = qobject_cast<const FileView *>(option.widget);
    if (!view) {
        return;
    }
    QRectF rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y());
    rect.setWidth(option.rect.width());
    rect.setHeight(option.rect.height());
    QRect selectIconRect;
    // 绘制可选图标的位置
    if (view->viewMode() == QListView::ListMode) {
        selectIconRect = QRect(rect.right() - 24, rect.top() + 21, 14, 14);
    } else if (view->viewMode() == QListView::IconMode) {
        selectIconRect = QRect(rect.right() - 15, rect.top(), 14, 14);
    }
    QIcon selectIcon;
    if (option.state.testFlag(QStyle::State_Selected)) {
        selectIcon = QIcon(":/filemanageview/select.svg");
    } else {
        selectIcon = QIcon(":/filemanageview/rectangle.svg");
    }
    QPixmap selectPixmap = selectIcon.pixmap(14, 14);
    painter->drawPixmap(selectIconRect, selectPixmap);
}
