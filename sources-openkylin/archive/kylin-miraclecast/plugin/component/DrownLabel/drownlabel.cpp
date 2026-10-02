#include "drownlabel.h"
#include "projection.h"

#define ICONSIZE 18,18
DrownLabel::DrownLabel(QWidget * parent) : QLabel(parent)
{
    setFixedSize(36,36);
    loadPixmap(isChecked);
    this->setProperty("useIconHighlightEffect", 0x2);
}

DrownLabel::~DrownLabel()
{

}
void DrownLabel::setDropDownStatus(bool status)
{
    isChecked = status;
    loadPixmap(isChecked);
}

void DrownLabel::loadPixmap(bool isChecked)
{
    if (isChecked) {
        setPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(ICONSIZE));
    } else {
        setPixmap(QIcon::fromTheme("ukui-up-symbolic").pixmap(ICONSIZE));
    }
}

void DrownLabel::mouseReleaseEvent(QMouseEvent *event)
{
    emit labelClicked();
    QWidget::mouseReleaseEvent(event);
}
