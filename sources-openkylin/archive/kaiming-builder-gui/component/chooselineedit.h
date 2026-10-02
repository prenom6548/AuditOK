#ifndef CHOOSELINEEDIT_H
#define CHOOSELINEEDIT_H

#include <QLineEdit>
#include "clicklabel.h"

class ChooseLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    ChooseLineEdit(QWidget *parent = nullptr);
    virtual ~ChooseLineEdit();

signals:
    void clearText();
    void choose();

private:
    ClickLabel * m_clear;
    ClickLabel * m_choose;
};

#endif // CHOOSELINEEDIT_H
