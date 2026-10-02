#include "kdrivelabel.h"

KDriveLabel::KDriveLabel(QWidget *parent)
    : QLabel(parent)
{
    this->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->setCursor(Qt::IBeamCursor);
}

void KDriveLabel::contextMenuEvent(QContextMenuEvent *event)
{
    // 不执行任何操作，隐藏上下文菜单事件
    Q_UNUSED(event);
}
