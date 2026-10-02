#ifndef WIDGET_H
#define WIDGET_H

#include "klunardateedit.h"
#include <QCheckBox>
#include <QWidget>

using namespace kdk;

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_checkBox_clicked(bool checked);

private:
    QCheckBox *m_pLunarCheckBox;
    KLunarDateEdit *m_pLunarDateEdit;
};

#endif // WIDGET_H
