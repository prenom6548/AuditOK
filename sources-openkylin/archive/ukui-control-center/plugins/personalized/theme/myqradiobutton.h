#ifndef MYQRADIOBUTTON_H
#define MYQRADIOBUTTON_H

#include <QObject>
#include <QWidget>
#include <QRadioButton>

class MyQRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    MyQRadioButton(QWidget *parent = nullptr);

    // Tab 只落在当前选中项；方向键可程序化移焦
    void refreshGroupKeyboardNav();

protected:
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    // 同父级强调色单选间左右键只移焦点，不改选中
    void moveFocusInGroup(int key);

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();
};

#endif // MYQRADIOBUTTON_H
