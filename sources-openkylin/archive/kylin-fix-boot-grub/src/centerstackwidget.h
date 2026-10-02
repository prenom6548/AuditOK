#ifndef CENTERSTACKWIDGET_H
#define CENTERSTACKWIDGET_H

#include <QObject>
#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QPainterPath>

class centerStackWidget : public QWidget
{
    Q_OBJECT
public:
    explicit centerStackWidget(QWidget *parent = nullptr);

public:
    QStackedWidget  *m_pStackWidget = nullptr;
    QHBoxLayout     *m_pHboxWidgetLayout = nullptr;

signals:

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // CENTERSTACKWIDGET_H
