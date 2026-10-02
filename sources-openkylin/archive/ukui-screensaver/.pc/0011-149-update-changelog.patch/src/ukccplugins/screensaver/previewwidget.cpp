#include "previewwidget.h"
#include <QProcess>

PreviewWidget::PreviewWidget(QWidget *parent):QWidget(parent)
{

}

PreviewWidget::~PreviewWidget()
{

}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRect(rect());
}

void PreviewWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton) {
        previewScreensaver();
    }
}

void PreviewWidget::previewScreensaver()
{
    static QProcess *viewProcess = new QProcess();
    viewProcess->start("ukui-screensaver-command --screensaver");
}
