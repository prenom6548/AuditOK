#ifndef CUSTOMCALENDAR_H
#define CUSTOMCALENDAR_H

#include <QSpinBox>
#include <QCalendarWidget>
#include <QToolButton>

class QPushButton;
class QLabel;
class CustomCalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    CustomCalendarWidget(QWidget *parent = nullptr);
    ~CustomCalendarWidget();
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
    QPixmap loadSvg(const QString &path, int size);
    bool eventFilter(QObject *watched, QEvent *event);
    void resetYearEditWidth();

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *event);

private:
    QToolButton *yearButton   = nullptr;
    QToolButton *monthButton  = nullptr;
    QSpinBox    *yearEdit     = nullptr;
    QToolButton *left_button  = nullptr;
    QToolButton *right_button = nullptr;

};




#endif // CUSTOMCALENDAR_H
