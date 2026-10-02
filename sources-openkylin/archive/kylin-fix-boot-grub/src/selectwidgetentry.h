#ifndef SELECTWIDGETENTRY_H
#define SELECTWIDGETENTRY_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
#include <QTimer>
#include <QPainterPath>

class selectWidgetEntry : public QWidget
{
    Q_OBJECT
public:
    explicit selectWidgetEntry(QWidget *parent = nullptr);
    ~selectWidgetEntry();
    void initUI();
    void setDiskName(QString diskName);

public:
    QLabel *m_pDiskNameLabel;
    QLabel *m_pIconLabel;
    QHBoxLayout *m_pMainHBoxLayout = nullptr;

    enum TaskWidgetStatus{NORMAL, HOVER, PRESS};
    TaskWidgetStatus status;
    QString m_distNameSelectEntry = nullptr;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void updateDiskSelectEntry(QWidget *widget);
};

#endif // SELECTWIDGETENTRY_H
