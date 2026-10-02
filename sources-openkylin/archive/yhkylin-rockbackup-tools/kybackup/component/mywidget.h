#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QObject>

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = nullptr);
    ~MyWidget();

    void paintEvent(QPaintEvent *event);
};

#endif // MYWIDGET_H
