#include "pictureiconlabel.h"
#include <QPainter>
#include <QGridLayout>
#include <QDebug>

static const int EXTERNAL_RADIUS = 10;
static const int INTERNAL_RADIUS = EXTERNAL_RADIUS - 2;

PictureIconLabel::PictureIconLabel(QWidget *parent)
    : QLabel(parent)
    , m_radius(EXTERNAL_RADIUS)
    , m_pictrueLabel(new DrawIconLabel(INTERNAL_RADIUS, this))
{
    this->setScaledContents(true);
    QHBoxLayout *hLyt = new QHBoxLayout(this);
    hLyt->setContentsMargins(0, 0, 0, 0);
    hLyt->addWidget(m_pictrueLabel);
}

PictureIconLabel::PictureIconLabel(const QPixmap &pixmap, QWidget *parent)
    : QLabel(parent)
    , m_radius(EXTERNAL_RADIUS)
    , m_pictrueLabel(new DrawIconLabel(pixmap, INTERNAL_RADIUS, this))
{
    this->setScaledContents(true);
    QHBoxLayout *hLyt = new QHBoxLayout(this);
    hLyt->setContentsMargins(0, 0, 0, 0);
    hLyt->addWidget(m_pictrueLabel);
}

PictureIconLabel::PictureIconLabel(QStringList iStringList, QWidget *parent)
    : QLabel(parent)
    , m_radius(EXTERNAL_RADIUS)
    , m_iconList(iStringList)

{
    QGridLayout *gLyt = new QGridLayout(this);
    gLyt->setContentsMargins(0, 0, 0, 0);
    gLyt->setSpacing(10);
    for (int i = 0; i < iStringList.count(); i++) {
        QLabel * label = new QLabel(this);
        label->setFixedSize(32, 32);
        label->setPixmap(pixmapToRound(iStringList.at(i), QSize(32, 32)));
        gLyt->addWidget(label, i / 3 , i % 3);
    }
}

PictureIconLabel::PictureIconLabel(QList<QPixmap> iconList, QWidget *parent)
    : QLabel(parent)
{
    QGridLayout *gLyt = new QGridLayout(this);
    gLyt->setContentsMargins(0, 0, 0, 0);
    gLyt->setSpacing(10);
    for (int i = 0; i < iconList.count(); i++) {
        QLabel * label = new QLabel(this);
        label->setFixedSize(24, 24);
        label->setPixmap(iconList.at(i));
        gLyt->addWidget(label, i / 3 , i % 3);
    }
}

QPixmap PictureIconLabel::pixmapToRound(const QString &filePath, const QSize &scaledSize) const
{
    QPixmap pixmap(filePath);

    qreal dpi = devicePixelRatioF();

    QSize realSize = scaledSize * dpi;
    QPixmap resultPixmap(realSize);

    resultPixmap.fill(Qt::transparent);
    QPainter painter(&resultPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPoint drawLeftTop(0, 0);
    QSize drawSize = realSize;

    drawSize.setWidth(realSize.width());
    drawSize.setHeight(realSize.width() * scaledSize.height() / scaledSize.width());
    drawLeftTop.setX(0);
    drawLeftTop.setY((realSize.height() - drawSize.height()) / 2);

    // scaled draw
    painter.drawPixmap(drawLeftTop.x(), drawLeftTop.y(), drawSize.width(), drawSize.height(), pixmap.scaled(drawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    resultPixmap.setDevicePixelRatio(dpi);
    return resultPixmap;
}

QPixmap PictureIconLabel::pixmapToRound(const QPixmap &pixmap, const QSize &scaledSize) const
{
    qreal dpi = devicePixelRatioF();

    QSize realSize = scaledSize * dpi;
    QPixmap resultPixmap(realSize);

    resultPixmap.fill(Qt::transparent);
    QPainter painter(&resultPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPoint drawLeftTop(0, 0);
    QSize drawSize = realSize;

    drawSize.setWidth(realSize.width());
    drawSize.setHeight(realSize.width() * scaledSize.height() / scaledSize.width());
    drawLeftTop.setX(0);
    drawLeftTop.setY((realSize.height() - drawSize.height()) / 2);

    // scaled draw
    painter.drawPixmap(drawLeftTop.x(), drawLeftTop.y(), drawSize.width(), drawSize.height(), pixmap.scaled(drawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    resultPixmap.setDevicePixelRatio(dpi);
    return resultPixmap;
}

void PictureIconLabel::setRadius(int radius)
{
    m_radius = radius - 2;
}

void PictureIconLabel::setIcon(const QPixmap &pixmap)
{
    if (m_pictrueLabel != nullptr)
        m_pictrueLabel->setIcon(pixmap);
}

void PictureIconLabel::leaveEvent(QEvent *event)
{
    Q_EMIT leaveWidget();
    QLabel::leaveEvent(event);
}

void PictureIconLabel::enterEvent(QEvent *event)
{
    Q_EMIT enterWidget();
    QLabel::enterEvent(event);
}
