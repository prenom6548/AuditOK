#ifndef MYCHECKBOX_H
#define MYCHECKBOX_H

#include <QCheckBox>

class MyCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit MyCheckBox(const QString &text, QWidget *parent = nullptr);
    virtual ~MyCheckBox();

    void setFontSize(int size);
};

#endif // MYCHECKBOX_H
