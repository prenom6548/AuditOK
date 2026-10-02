#include <QPainter>
#include <QDebug>
#include <QFontMetrics>
#include <QFont>
#include <QVariant>
#include <QBoxLayout>
#include <QPaintDevice>
#include "kom-ukui-gsettings.h"
#include "kom-label.h"
#include "kom-label_p.h"

namespace kom
{

KomLabelPrivate::KomLabelPrivate(KomLabel *q) : q_ptr(q), m_isBold(false), m_fontSize(0), m_label(new QLabel(q_ptr))
{
    QHBoxLayout *bl = new QHBoxLayout(q_ptr);
    bl->setMargin(0);
    bl->addWidget(m_label);
}

KomLabelPrivate::~KomLabelPrivate()
{
    /* nothing to do */
}

void KomLabelPrivate::fitText(void)
{
    Q_Q(KomLabel);

    QFont font;
    font.setBold(m_isBold);
    font.setPointSizeF(adaptFontSize());
    m_label->QLabel::setFont(font);

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
    int heightTotal = fm.height();
    if (q->height() < heightTotal) {
        q->setFixedHeight(heightTotal);
    }
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

double KomLabelPrivate::adaptFontSize(void)
{
    static int defaultSize = 10;

    if (!m_fontSize)
        return UkuiGsettings::getInstance()->getFontSize().toDouble();

    return (m_fontSize / defaultSize) * UkuiGsettings::getInstance()->getFontSize().toDouble();
}

void KomLabelPrivate::setAlignment(Qt::Alignment a)
{
    m_label->setAlignment(a);
}

KomLabel::KomLabel(QWidget *parent) : QLabel(parent), d_ptr(new KomLabelPrivate(this))
{
    connect(UkuiGsettings::getInstance(), &UkuiGsettings::fontSizeChange, this, [=](QVariant fontSize) {
        Q_D(KomLabel);
        d->fitText();
        m_fontSizeChanged = true;
    });
}

KomLabel::~KomLabel() {}

void KomLabel::setAlignment(Qt::Alignment a)
{
    Q_D(KomLabel);
    d->setAlignment(a);
}

void KomLabel::setText(const QString &text)
{
    Q_D(KomLabel);

    d->m_text = text;
    d->fitText();
}

void KomLabel::setBold(bool enable)
{
    Q_D(KomLabel);

    d->m_isBold = enable;
}

void KomLabel::setFontSize(int size)
{
    Q_D(KomLabel);
    double DPI = logicalDpiX();
    if (DPI < 1)
        DPI = 96.0;
    d->m_fontSize = double(size) * 72 / DPI;
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

void KomLabel::paintEvent(QPaintEvent *e)
{
    Q_D(KomLabel);

    d->fitText();
}

} // namespace kom
