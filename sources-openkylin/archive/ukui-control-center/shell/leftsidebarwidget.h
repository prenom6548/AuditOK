#ifndef LEFTSIDEBARWIDGET_H
#define LEFTSIDEBARWIDGET_H

#include <QWidget>
#include <QObject>
#include <QGSettings>
#include <kwidget.h>

class LeftsidebarWidget : public QWidget
{
    Q_OBJECT
public:
    LeftsidebarWidget(QWidget *parent = nullptr);
    ~LeftsidebarWidget();
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);
private:
    int transparency = 0;

    QGSettings *personalQgsettings = nullptr;
};

#endif // LEFTSIDEBARWIDGET_H
