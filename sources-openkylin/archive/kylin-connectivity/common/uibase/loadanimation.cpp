#include "loadanimation.h"
#include <QHBoxLayout>
#include <QDebug>

#include "highlight-effect.h"

const int ICON_MAX_NUM = 8;

LoadAnimation::LoadAnimation(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_loadLab = new QLabel(this);
    m_timer = new QTimer(this);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_loadLab);
    hLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    vLayout->addStretch();

    setLayout(vLayout);

    m_loadLab->setPixmap(
        QIcon::fromTheme(QString("ukui-loading-%1-symbolic").arg(QString::number(m_count))).pixmap(48, 48));

    connect(m_timer, &QTimer::timeout, this, &LoadAnimation::timeout);
    m_timer->start(100);
}

void LoadAnimation::setHighLight()
{
    m_loadLab->setProperty("useIconHighlightEffect", 0x2);
}

void LoadAnimation::timeout()
{
    m_loadLab->setPixmap(
        QIcon::fromTheme(QString("ukui-loading-%1-symbolic").arg(QString::number(m_count))).pixmap(48, 48));
    m_count++;
    if (m_count == ICON_MAX_NUM) {
        m_count = 1;
    }
}
