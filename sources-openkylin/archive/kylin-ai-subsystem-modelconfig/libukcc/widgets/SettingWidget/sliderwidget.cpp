#include "sliderwidget.h"

#include <QDebug>

SliderWidget::SliderWidget(QString title, bool hasNode, QWidget *parent, UkccFrame::BorderRadiusStyle style, Qt::Orientation orientation):
    UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_slider = new KSlider(orientation, this);
    m_slider->setNodeVisible(hasNode);
    init();
}

SliderWidget::~SliderWidget()
{
}

void SliderWidget::init()
{
    mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_leftLabel = new QLabel(this);
    m_leftLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_rightLabel = new QLabel(this);
    m_rightLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_settingName->setFixedWidth(214);
    mainLayout->addWidget(m_settingName);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_leftLabel);
    mainLayout->addWidget(m_slider);
    mainLayout->addWidget(m_rightLabel);
    this->setLayout(mainLayout);
    connect(m_slider, &KSlider::valueChanged, this, &SliderWidget::valueChanged);
}

void SliderWidget::setEnabled(const bool checked)
{
    if (m_slider) {
        m_slider->setEnabled(checked);
    }
}

bool SliderWidget::isEnabled() const
{
    return m_slider->isEnabled();
}

void SliderWidget::setValue(const int value)
{
    m_slider->setValue(value);
}

void SliderWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
    m_slider->setAccessibleName(title);
}

void SliderWidget::setLeftText(const QString text)
{
    m_leftLabel->setText(text);
}

void SliderWidget::setRightText(const QString text)
{
    m_rightLabel->setText(text);
}

QString SliderWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void SliderWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

int SliderWidget::value() const
{
    return m_slider->value();
}

KSlider *SliderWidget::slider() const
{
    return m_slider;
}

QLabel *SliderWidget::leftLabel() const
{
    return m_leftLabel;
}

QLabel *SliderWidget::rightLabel() const
{
    return m_rightLabel;
}

void SliderWidget::insertWidget(int index, QWidget *widget, int stretch, Qt::Alignment alignment)
{
    mainLayout->insertWidget(index, widget, stretch, alignment);
}

void SliderWidget::setLeftPixmap(const QPixmap &pixmap)
{
    m_leftLabel->setPixmap(pixmap);
}

void SliderWidget::setRightPixmap(const QPixmap &pixmap)
{
    m_rightLabel->setPixmap(pixmap);
}


