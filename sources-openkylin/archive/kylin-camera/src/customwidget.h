
#ifndef CUSTOM_WIDGET_H
#define CUSTOM_WIDGET_H

#include <kysdk/applications/kwidget.h>

class CustomWidget : public kdk::KWidget
{
    Q_OBJECT
public:
    explicit CustomWidget(QWidget *parent = nullptr);
    ~CustomWidget() override;
    void setWidgetName(const QString& widgetName);
};

#endif // CUSTOM_WIDGET_H
