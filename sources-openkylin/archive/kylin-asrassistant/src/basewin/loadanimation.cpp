#include "loadanimation.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QPalette>
#include <QPixmap>
#include <QEvent>

const int ICON_MAX_NUM = 49;
const int DEF_PNG_SIZE = 58;
const int INIT_SIZE = 84;
const int TIME_COUNT = 40;

LoadAnimation::LoadAnimation(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background: transparent;");
    m_size = INIT_SIZE;
    m_loadLab = new QLabel(this);
    m_loadLab->setFocusPolicy(Qt::NoFocus);
    m_loadLab->setStyleSheet("background: transparent;");
    m_loadLab->resize(INIT_SIZE, INIT_SIZE);
    m_loadLab->setPixmap(QPixmap(QString(":/res/animation/%1.png").arg(QString::number(m_count)))
                             .scaled(INIT_SIZE, INIT_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    m_loadLab->installEventFilter(this);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &LoadAnimation::timeout);
    m_timer->start(TIME_COUNT);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_loadLab);
    hLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    vLayout->addStretch();

    setLayout(vLayout);
}

void LoadAnimation::start()
{
    qInfo() << "animation start ...";
    m_isPlay = true;
    m_loadLab->setPixmap(QPixmap(QString(":/res/animation/%1.png").arg(QString::number(m_count)))
                             .scaled(INIT_SIZE, INIT_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    m_timer->start(TIME_COUNT);
}

void LoadAnimation::stop()
{
    qInfo() << "animation stop";
    m_isPlay = false;
    m_timer->stop();
    m_loadLab->setPixmap(QPixmap(":/res/default.png")
                             .scaled(DEF_PNG_SIZE, DEF_PNG_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

bool LoadAnimation::eventFilter(QObject *watch, QEvent *event)
{
    if (!m_isPlay && watch == m_loadLab && event->type() == QEvent::MouseButtonPress) {
        m_loadLab->setPixmap(QPixmap(QString(":/res/animation/%1.png").arg(QString::number(m_count)))
                                 .scaled(INIT_SIZE, INIT_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_timer->start(TIME_COUNT);
        m_isPlay = true;
        Q_EMIT cliecked();
    }

    return QWidget::eventFilter(watch, event);
}

void LoadAnimation::timeout()
{
    m_count++;
    if (m_count == ICON_MAX_NUM) {
        m_count = 0;
    }
    m_loadLab->setPixmap(QPixmap(QString(":/res/animation/%1.png").arg(QString::number(m_count)))
                             .scaled(m_size, m_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void LoadAnimation::onUpdateLevel(double level)
{
    m_size = int(level * double(INIT_SIZE) / 4) + INIT_SIZE;
}