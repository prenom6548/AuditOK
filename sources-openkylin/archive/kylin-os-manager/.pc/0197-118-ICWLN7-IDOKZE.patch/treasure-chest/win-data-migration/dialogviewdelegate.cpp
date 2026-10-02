#include <QFont>
#include <QPixmap>
#include <QRectF>
#include <QRect>
#include <QDebug>
#include <QApplication>

#include "dialogviewdelegate.h"

DialogViewDelegate::DialogViewDelegate() {}

DialogViewDelegate::~DialogViewDelegate() {}

QSize DialogViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    return QSize(650,40);
}

void DialogViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        // 反锯齿
        painter->setRenderHint(QPainter::Antialiasing);

        QString text = index.data(0).toString();

        QRectF itemRect;
        itemRect.setX(option.rect.x());
        itemRect.setY(option.rect.y());
        itemRect.setWidth(option.rect.width() - 1);
        itemRect.setHeight(option.rect.height() - 1);

        QColor highLightColor = QApplication::palette().highlight().color();
        QColor buttonColor = QApplication::palette().buttonText().color();

        /* 内容样式 */
        QPoint textPoint(itemRect.left() + 24, itemRect.top() + 6);
        QSize textSize(option.rect.width() * 0.6, 35);
        QRectF textRect = QRect(textPoint, textSize);
        QTextOption textOption;

        painter->save();
        // 鼠标悬停/选中，改变背景颜色
        if (option.state.testFlag(QStyle::State_MouseOver)) {
            painter->setPen(QPen(highLightColor));
        } else {
            painter->setPen(QPen(buttonColor));
        }
        textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        painter->setFont(QFont(painter->fontInfo().family()));
        painter->drawText(textRect, text, textOption);
        painter->restore();
    }
    return;
}
