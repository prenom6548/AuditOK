#include "scalablelabel.h"
#include <QFontMetrics>
#include <QResizeEvent>

ScalableLabel::ScalableLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(1, 1);
    originalFont = font();  // 保存原始字体
}

void ScalableLabel::setFixedHeight(int h)
{
    QLabel::setFixedHeight(h);
    adjustFont();
}

void ScalableLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    adjustFont();
}

void ScalableLabel::setText(const QString &text)
{
    QLabel::setText(text);
    adjustFont();
}

void ScalableLabel::adjustFont()
{
    if (height() <= 0) return;  // 避免无效高度

    QFont font = originalFont;  // 从原始字体开始调整
    QFontMetrics metrics(font);
    int textHeight = metrics.height();

    // 如果文本高度超过标签高度，按比例缩小字体
    if (textHeight > height()) {
        double scaleFactor = static_cast<double>(height()) / textHeight;
        int newSize = qRound(font.pointSize() * scaleFactor);

        // 确保字体不会过小
        if (newSize >= minFontSize) {
            font.setPointSize(newSize);
            setFont(font);
        }
    } else {
        // 文本高度未超过，使用原始字体
        setFont(originalFont);
    }
}
