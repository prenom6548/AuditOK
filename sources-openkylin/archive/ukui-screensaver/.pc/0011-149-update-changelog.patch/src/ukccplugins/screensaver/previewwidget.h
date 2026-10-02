#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();
    void previewScreensaver();

protected:
    void paintEvent(QPaintEvent *e);

private:
    void mousePressEvent(QMouseEvent *e);
};

#endif // PREVIEWWIDGET_H
