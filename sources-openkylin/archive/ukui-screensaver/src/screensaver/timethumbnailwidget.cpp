#include "timethumbnailwidget.h"

TimeThumbnailWidget::TimeThumbnailWidget(QWidget *parent, bool isControlSaver)
    : QWidget{parent},
      m_bControlSaver(isControlSaver)
{
    initUI();
}

TimeThumbnailWidget::~TimeThumbnailWidget()
{
    m_timer->deleteLater();
}

void TimeThumbnailWidget::initUI()
{
    if (!m_bControlSaver ) {
        setFixedSize(100, 40);
    } else {
        setFixedSize(100, 30);
    }
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(2);

    m_timeLabel = new QLabel(this);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet("QLabel{color: white;}");
    QFont timeFont = m_timeLabel->font();
    if (!m_bControlSaver ) {
        timeFont.setPointSize(14);
    } else {
        timeFont.setPointSize(12);
    }
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setText(getLongFormatDate(TIMETYPE));
    m_mainLayout->addWidget(m_timeLabel);

    m_dayLabel = new QLabel(this);
    m_dayLabel->setAlignment(Qt::AlignCenter);
    m_dayLabel->setStyleSheet("QLabel{color: white;}");
    QFont dayFont = m_dayLabel->font();
    if (!m_bControlSaver ) {
        dayFont.setPointSize(6);
    } else {
        dayFont.setPointSize(5);
    }
    m_dayLabel->setFont(dayFont);
    m_dayLabel->setText(getLongFormatDate(TIMETYPE));
    m_mainLayout->addWidget(m_dayLabel);
    setLayout(m_mainLayout);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update_datatime()));
    this->timerStart();
}

void TimeThumbnailWidget::timerStart()
{
    m_timer->start(1000);
}

void TimeThumbnailWidget::timerStop()
{
    m_timer->stop();
}

QString TimeThumbnailWidget::getLongFormatDate(int type)
{
    kdk_logn_dateinfo *dateInfo = kdk_system_login_lock_dateinfo(getenv("USER"));
    if (type == DATETYPE) {
        QString date = dateInfo->date;
        QString week = dateInfo->week;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        return date + " " + week;
    } else if (type == TIMETYPE) {
        QString time = dateInfo->time;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        return time;
    }
}

void TimeThumbnailWidget::update_datatime()
{
    m_timeLabel->setText(getLongFormatDate(TIMETYPE));
    m_dayLabel->setText(getLongFormatDate(DATETYPE));
}
