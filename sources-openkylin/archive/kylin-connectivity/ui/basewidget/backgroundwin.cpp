#include "backgroundwin.h"
#include <QPainter>
#include <QStyleOption>
#include <QBitmap>

BackgroundWin::BackgroundWin(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground); //设置窗口背景透明
    this->setWindowFlags(Qt::FramelessWindowHint);    //设置无边框窗口
}

void BackgroundWin::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    update();
}

void BackgroundWin::setBackground(bool isDefault)
{
    m_isDefaultColor = isDefault;
}

void BackgroundWin::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this); //绘制样式

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    QColor mainColor;
    if (m_isDefaultColor) {
        if (m_theme == PublicAttributes::Theme::Light) {
            mainColor = QColor("#FFFEFD");
        } else {
            mainColor = QColor("#1D1D1D");
        }
    } else {
        if (m_theme == PublicAttributes::Theme::Light) {
            mainColor = QColor("#F6F6F6");
        } else {
            mainColor = QColor("#232426");
        }
    }
    painter.setBrush(QBrush(mainColor));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width());
    rect.setHeight(rect.height());
    painter.drawRoundedRect(rect, 8, 8);
    QWidget::paintEvent(event);

    return;
}
