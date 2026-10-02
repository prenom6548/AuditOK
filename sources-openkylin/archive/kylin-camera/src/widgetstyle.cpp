#include "widgetstyle.h"
#include <QPainterPath>
void widgetstyle::paintEvent(QPaintEvent *event)    //重绘窗口
{
    //设置字体字号
    QFont ft = font;
    ft.setPixelSize(14);
    text->setFont(ft);

    //上半部分阴影
    QPainterPath painterPath;
    painterPath.setFillRule(Qt::WindingFill);       //设置填充方式
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(painterPath, QBrush(Qt::white));
    //下半部分阴影
    QPainterPath painterPath2;
    painterPath2.setFillRule(Qt::WindingFill);

    QColor color = Qt::black;
    for(int i=0; i<SHADOW; i++)
    {
        int alpha=SHADOWALPHA*255/4;
        color.setAlpha(alpha - (alpha/SHADOW)*(i+1));
        painter.setPen(color);

        QRect rec(SHADOW-i,SHADOW-i,this->width()-(SHADOW-i)*2,TITLEH+WIDGETRADIUS*2 );
        painterPath.addRoundedRect(rec,WIDGETRADIUS,WIDGETRADIUS);
        painter.setClipRegion(QRegion(0,0,this->width(),TITLEH+WIDGETRADIUS)); //裁剪区域（标题栏）
        painter.drawPath(painterPath);

        QRect rec2(rec.left(),rec.top()+TITLEH,this->width()-(SHADOW-i)*2,this->height()-TITLEH-WIDGETRADIUS);
        painterPath2.addRect(rec2);
        painter.setClipRegion(QRegion(0,TITLEH+WIDGETRADIUS,this->width(),this->height())); //裁剪区域（窗体）
        painter.drawPath(painterPath2);
    }

    QWidget::paintEvent(event);
}
