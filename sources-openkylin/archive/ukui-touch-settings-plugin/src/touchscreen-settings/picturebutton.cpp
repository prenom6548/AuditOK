#include "picturebutton.h"
#include "pictureiconlabel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPainter>

PictureButton::PictureButton(const QString &name, QWidget *parent)
    : QPushButton(parent)
    , m_themeName(name)
    , m_iconLabel(new PictureIconLabel(this))
{
    initUi();
    setConnect();
}

PictureButton::PictureButton(const QString &name, const QPixmap &pixmap, QWidget *parent)
    : QPushButton(parent)
    , m_themeName(name)
    , m_iconLabel(new PictureIconLabel(pixmap, this))
{
    initUi();
    setConnect();
}


PictureButton::PictureButton(QString name, QStringList iStringList, QWidget *parent)
    : QPushButton(parent)
    , m_themeName(name)
    , m_iconLabel(new PictureIconLabel(iStringList, this))
{
    initUi();
    setConnect();
}

PictureButton::PictureButton(QString name, QList<QPixmap> iconList, QWidget *parent)
    : QPushButton(parent)
    , m_themeName(name)
    , m_iconLabel(new PictureIconLabel(iconList, this))
{
    initUi();
    setConnect();
}

void PictureButton::initUi()
{
    m_clickedBtnStyle = getStyle(2, m_radius);
    m_enterBtnStyle = getStyle(1, m_radius);
    m_leaveBtnStyle = getStyle(0, m_radius);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setCheckable(true);
    setFixedWidth(m_width);
    QVBoxLayout *baseVerLayout = new QVBoxLayout(this);
    baseVerLayout->setSpacing(8);
    baseVerLayout->setMargin(0);

    m_iconLabel->setFixedSize(QSize(m_width, m_heigh - 32));
    m_iconLabel->setAttribute(Qt::WA_DeleteOnClose);
    m_iconLabel->setStyleSheet(m_leaveBtnStyle);

    QHBoxLayout *bottomHorLayout = new QHBoxLayout;
    bottomHorLayout->setSpacing(8);
    bottomHorLayout->setMargin(0);

    bottomHorLayout->addStretch();
    bottomHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomHorLayout->addStretch();

    baseVerLayout->addWidget(m_iconLabel);
    baseVerLayout->addLayout(bottomHorLayout);

    this->setLayout(baseVerLayout);
}

void PictureButton::setStatus(bool status)
{
    this->setChecked(status);

    if (status) {
        m_iconLabel->setStyleSheet(m_clickedBtnStyle);
    } else {
        m_iconLabel->setStyleSheet(m_leaveBtnStyle);
    }
}

void PictureButton::setConnect()
{
    connect(m_iconLabel, &PictureIconLabel::enterWidget, this, [this](){
        if (!this->isChecked()) {
            m_iconLabel->setStyleSheet(m_enterBtnStyle);
        }
    });

    connect(m_iconLabel,&PictureIconLabel::leaveWidget, this, [this](){
        if (!this->isChecked()) {
            m_iconLabel->setStyleSheet(m_leaveBtnStyle);
        }
    });
}

void PictureButton::setRect(QSize size)
{
    m_width = size.width();
    m_heigh = size.height();
    setFixedWidth(m_width);
    m_iconLabel->setFixedSize(QSize(m_width, m_heigh - 32));
}

void PictureButton::setPixmap(const QPixmap &pixmap)
{    
    m_iconLabel->setIcon(pixmap);
}

void PictureButton::setBtnClicked(bool status)
{
    this->setChecked(status);
    if (!this->isChecked()) {
        m_iconLabel->setStyleSheet(m_leaveBtnStyle);
    } else {
        m_iconLabel->setStyleSheet(m_clickedBtnStyle);
    }
}

QString PictureButton::getStyle(int border, int radius) const
{
    return QString("PictureIconLabel{background: palette(window); border-color: palette(highlight);"
                   "border-width: %1px;border-style: solid;border-radius: %2px}").arg(border).arg(radius);
}

void PictureButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    painter.drawRoundedRect(this->rect(), m_radius, m_radius);


    if (height() != m_iconLabel->height() + 8 /*+ mNameLabel->height()*/)
        this->setFixedHeight(m_iconLabel->height() + 8 /*+ mNameLabel->height()*/);

}

QString PictureButton::getThemeName() const
{
    return m_themeName;
}
