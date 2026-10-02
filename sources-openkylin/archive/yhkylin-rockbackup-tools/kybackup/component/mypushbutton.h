#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>

class MyPushButton : public QPushButton
{
    Q_OBJECT
public:
    MyPushButton(QWidget* parent = nullptr);
    virtual ~MyPushButton();

    void setFontSize(int size);
};

#endif // MYPUSHBUTTON_H
