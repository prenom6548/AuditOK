#ifndef LEVITATIONBUTTON_H
#define LEVITATIONBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QFileDialog>

#include "fileinfo.h"

class LevitationButton : public QPushButton
{
    Q_OBJECT
public:
    LevitationButton(QWidget *parent = nullptr);

    void setIcon(QString iconName);
    void setText(QString text);
    void changeFontSize(double fontSize);

private:
    QLabel *m_icon = nullptr;
    QLabel *m_text = nullptr;
};

#endif // LEVITATIONBUTTON_H
