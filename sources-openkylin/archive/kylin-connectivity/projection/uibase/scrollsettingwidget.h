#ifndef SCROLLSETTINGWIDGET_H
#define SCROLLSETTINGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QSlider>
#include "config.h"

class ScrollSettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollSettingWidget(QWidget *parent = 0);

private:
    QLineEdit *m_line = nullptr;
    QSlider *m_slider = nullptr;
};

#endif // SCROLLSETTINGWIDGET_H
