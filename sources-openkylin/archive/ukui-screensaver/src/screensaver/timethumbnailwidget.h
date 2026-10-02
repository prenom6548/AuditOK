#ifndef TIMETHUMBNAILWIDGET_H
#define TIMETHUMBNAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QLabel>
#include <QDateTime>
#include <QVBoxLayout>

#include <libkydate.h>

enum DateInfo
{
    DATETYPE,
    TIMETYPE,
};

class TimeThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimeThumbnailWidget(QWidget *parent = nullptr, bool isControlSaver = false);
    ~TimeThumbnailWidget();

public Q_SLOTS:
    void update_datatime();
    void timerStart();
    void timerStop();

private:
    void initUI();
    QString getLongFormatDate(int type);

private:
    QTimer *m_timer = nullptr;
    QLabel* m_timeLabel = nullptr;
    QLabel* m_dayLabel = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
    bool m_bControlSaver = false;
};

#endif // TIMETHUMBNAILWIDGET_H
