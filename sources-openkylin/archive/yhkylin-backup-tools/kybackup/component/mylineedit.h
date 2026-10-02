#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>

class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    MyLineEdit(QWidget *parent = nullptr);
    virtual ~MyLineEdit();

private:
};

#endif // MYLINEEDIT_H
