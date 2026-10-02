#ifndef FIXPUSHBUTTON_H
#define FIXPUSHBUTTON_H

#include <QPushButton>
#include <QString>

class FixPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FixPushButton(QWidget *parent = 0);

public:
    void setButtonText(QString text);
    QString getText();

private Q_SLOTS:
    void changedLabelSlot();
private:
    QString mStr;

};
#endif // FIXPUSHBUTTON_H
