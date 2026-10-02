#include "customwidget.h"

CustomWidget::CustomWidget(QWidget *parent) :
    kdk::KWidget(parent)
{
    setIcon("kylin-camera");
    setWindowIcon(QIcon::fromTheme("kylin-camera"));
}

CustomWidget::~CustomWidget()
{
}

void CustomWidget::setWidgetName(const QString& widgetName)
{
    kdk::KWidget::setWidgetName(widgetName);
    QWidget::setWindowTitle(widgetName);
}