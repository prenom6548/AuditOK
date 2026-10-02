#include "customlabel.h"
#include <QToolTip>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "globalsignal.h"

CustomLabel::CustomLabel(QWidget *parent) : QLabel(parent), isHovered(false), isElided(false)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMouseTracking(true);
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &CustomLabel::updateDisplayText);
}

QSize CustomLabel::sizeHint() const
{
    return QSize(fontMetrics().width(displayText), fontMetrics().height());
}

void CustomLabel::updateDisplayText()
{
    QFontMetrics fm(font());
    int width = this->width();
    if (fm.width(originalText) > width) {
        isElided = true;
        displayText = fm.elidedText(originalText, Qt::ElideRight, width);
        this->setToolTip(originalText);
    } else {
        isElided = false;
        displayText = originalText;
        this->setToolTip("");
    }
    update();
}

void CustomLabel::SetText(const QString &text)
{
    originalText = text;
    updateDisplayText();
}

void CustomLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawControl(QStyle::CE_ShapedFrame, &opt, &painter, this);

    painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, displayText);
}

void CustomLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit clicked();
}

void CustomLabel::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    isHovered = true;
    update();
}

void CustomLabel::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    isHovered = false;
    update();
}

void CustomLabel::resizeEvent(QEvent *event) {
    Q_UNUSED(event);
    update();
    updateDisplayText();
}
