#ifndef DELAYWDIGET_H
#define DELAYWDIGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainterPath>
#include "util/custombuttonstyle.h"
class DelayWdiget : public QWidget
{
    Q_OBJECT
public:
    DelayWdiget();
    ~DelayWdiget();
    CustomButtonStyle *close = nullptr;
    CustomButtonStyle *delay1 = nullptr;
    CustomButtonStyle *delay2 = nullptr;
    CustomButtonStyle *delay3 = nullptr;
    QWidget *mainWin = nullptr;
    QHBoxLayout *mainWinLayout = nullptr;
protected:
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void delay_emit(int);

private:
    QHBoxLayout *delayLayout = nullptr;
    int width;
    int height;
    void time0SecSlot();
    void time3SecSlot();
    void time6SecSlot();
    void time9SecSlot();
};

#endif // DELAYWDIGET_H
