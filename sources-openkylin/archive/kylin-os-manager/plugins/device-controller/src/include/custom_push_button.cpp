#include "custom_push_button.h"
#include <QDebug>

CustomPushButton::CustomPushButton(QWidget *parent)
    : QPushButton(parent)
{

}

CustomPushButton::CustomPushButton(QString &text, QWidget *parent)
    : QPushButton(parent), m_fullText(text)
{

}

void CustomPushButton::setText(const QString &text)
{
    setFullText(text);
}

void CustomPushButton::setFullText(const QString &text)
{
    m_fullText = text;
    update();
}

void CustomPushButton::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void CustomPushButton::setTextLimitExpand(const QString &text)
{
    int textWidth = this->fontMetrics().width(text);
    this->setMaximumWidth(textWidth);
    setFullText(text);
}

QString CustomPushButton::fullText() const
{
    return m_fullText;
}

void CustomPushButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    elideText();
}

void CustomPushButton::elideText()
{
    int margin = 32;
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) + margin - this->width() ;
    if (dif > 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width() - margin);
        QPushButton::setText(showText);
        if (showText != m_fullText) {
            QString str = m_fullText;
            this->setToolTip(str);
        } else {
            this->setToolTip("");
        }
    } else {
        QPushButton::setText(m_fullText);
        this->setToolTip("");
    }
}
