#include "clicklabel.h"

#include <QDebug>

ClickLabel::ClickLabel(QWidget *parent) : QLabel(parent)
{

}

void ClickLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event != nullptr && event->button() == Qt::LeftButton) {
        Q_EMIT clicked();
    }
}

