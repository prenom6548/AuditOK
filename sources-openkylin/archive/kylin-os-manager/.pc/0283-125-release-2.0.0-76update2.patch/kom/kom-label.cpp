#include <QPainter>
#include <QDebug>
#include <QFontMetrics>
#include <QFont>
#include <QVariant>
#include <QBoxLayout>
#include <QPaintDevice>
#include <QApplication>
#include "kom-ukui-gsettings.h"
#include "kom-utils.h"
#include "kom-label_p.h"
#include "kom-label.h"

namespace kom
{

KomLabelPrivate::KomLabelPrivate(KomLabel *q)
    : q_ptr(q),
      m_label(new QLabel(q_ptr)),
      m_isBold(false),       // 默认不是粗体
      m_fontPixelSize(14),   // 默认 14 pixel size
      m_transparency(1)      // 默认不透明
{
    QHBoxLayout *hbox_layout = new QHBoxLayout(q_ptr);
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(m_label);
}

KomLabelPrivate::~KomLabelPrivate()
{
    /* nothing to do */
}

void KomLabelPrivate::fitText(void) {
    Q_Q(KomLabel);

    // 设置字体
    QFont font;
    font.setBold(m_isBold);
    font.setPointSizeF(kom::KomUtils::adaptFontSize(m_fontPixelSize));
    m_label->QLabel::setFont(font);

    // 设置调色板
    QPalette pal = qApp->palette();
    QColor col = pal.color(QPalette::Foreground);
    col.setAlphaF(m_transparency);
    pal.setColor(QPalette::Foreground , col);
    m_label->setPalette(pal);

    QString showText;
    QFontMetrics fm(font);
    if (fm.width(m_text) - q->width() >= 0) {
        showText = fm.elidedText(m_text, Qt::ElideRight, q->width());
        if (showText == m_text)
            m_label->QLabel::setToolTip("");
        else
            m_label->setToolTip(m_text);
    } else {
        showText = m_text;
        m_label->setToolTip("");
    }

    QString html = transHtml(showText);
    m_label->QLabel::setText(html);
}

QString KomLabelPrivate::transHtml(QString &text)
{
    QString html;
    html += "<html><head/><body><p>";

    /* 处理数字 */
    if (m_firstNumColor.isValid() || m_secondNumColor.isValid()) {
        int count = 0;
        bool isStart = false;
        bool needClose = false;

        for (const QChar &item : text) {
            if (item.isNumber() && isStart == false) {
                count++;
                isStart = true;

                switch (count) {
                case 1:
                    if (m_firstNumColor.isValid()) {
                        html += QString("<span style=\"color:%1;\">").arg(m_firstNumColor.name());
                        needClose = true;
                    }
                    break;
                case 2:
                    if (m_secondNumColor.isValid()) {
                        html += QString("<span style=\"color:%1;\">").arg(m_secondNumColor.name());
                        needClose = true;
                    }
                    break;
                case 3:
                    if (m_thirdNumColor.isValid()) {
                        html += QString("<span style=\"color:%1;\">").arg(m_thirdNumColor.name());
                        needClose = true;
                    }
                    break;
                default:
                    break;
                }

                html += item;
            } else if (item.isNumber()) {
                html += item;
            } else {
                isStart = false;
                if (needClose) {
                    html += "</span>";
                    needClose = false;
                }
                html += item;
            }
        }
    } else {
        html += text;
    }

    /* 处理字符串 */
    /* todo */

    html += "</p></body></html>";

    return html;
}

void KomLabelPrivate::setAlignment(Qt::Alignment a)
{
    m_label->setAlignment(a);
}

KomLabel::KomLabel(QWidget *parent) : QLabel(parent), d_ptr(new KomLabelPrivate(this)) {
    connect(UkuiGsettings::getInstance(), &UkuiGsettings::fontSizeChange, this, [=](QVariant fontSize) {
        Q_D(KomLabel);
        setText(d->m_text);
    });
}

KomLabel::~KomLabel() {}

void KomLabel::setAlignment(Qt::Alignment a) {
    Q_D(KomLabel);

    QLabel::setAlignment(a);
    d->setAlignment(a);
}

void KomLabel::setBold(bool enable)
{
    Q_D(KomLabel);
    d->m_isBold = enable;
}

void KomLabel::setFontSize(int pixelSize)
{
    Q_D(KomLabel);
    d->m_fontPixelSize = pixelSize;
}

void KomLabel::setFirstNumColor(QColor color)
{
    Q_D(KomLabel);
    d->m_firstNumColor = color;
}

void KomLabel::setSecondNumColor(QColor color)
{
    Q_D(KomLabel);
    d->m_secondNumColor = color;
}

void KomLabel::setThirdNumColor(QColor color)
{
    Q_D(KomLabel);
    d->m_thirdNumColor = color;
}

void KomLabel::setTransparency(float t)
{
    Q_D(KomLabel);
    d->m_transparency = t;
}

void KomLabel::setText(const QString &text) {
    Q_D(KomLabel);

    d->m_text = text;

    // 设置底层 label 的属性，使底层和上层 label 的属性保持一致
    // 设置字体
    QFont font;
    font.setBold(d->m_isBold);
    font.setPointSizeF(kom::KomUtils::adaptFontSize(d->m_fontPixelSize));
    QLabel::setFont(font);

    // 设置调色板
    auto palette = qApp->palette();
    auto color = palette.color(QPalette::Foreground);
    color.setAlphaF(d->m_transparency);
    palette.setColor(QPalette::Foreground, color);
    QLabel::setPalette(palette);

    // 设置文本
    QLabel::setText(text);
    d->fitText();
}

void KomLabel::paintEvent(QPaintEvent *e) {
    Q_D(KomLabel);
    d->fitText();
}

} // namespace kom
