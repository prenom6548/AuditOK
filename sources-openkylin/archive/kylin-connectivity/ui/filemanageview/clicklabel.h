#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QMouseEvent>

class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QWidget *parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent *event);

Q_SIGNALS:
    void clicked();
};

#endif // CLICKLABEL_H
