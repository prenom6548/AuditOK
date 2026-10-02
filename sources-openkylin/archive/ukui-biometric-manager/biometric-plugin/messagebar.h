#ifndef MESSAGEBAR_H
#define MESSAGEBAR_H

#include <QWidget>

class MessageBar : public QWidget
{
    Q_OBJECT
public:
    explicit MessageBar(QWidget *parent = nullptr);
    ~MessageBar();

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // MESSAGEBAR_H
