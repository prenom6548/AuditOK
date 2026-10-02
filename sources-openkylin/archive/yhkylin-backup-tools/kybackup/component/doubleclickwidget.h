#ifndef DOUBLECLICKWIDGET_H
#define DOUBLECLICKWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class DoubleClickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DoubleClickWidget(QWidget *parent = nullptr);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void doubleClicked();

};

#endif // DOUBLECLICKWIDGET_H
