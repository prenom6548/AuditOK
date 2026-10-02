#ifndef DEVICECODEITEM_H
#define DEVICECODEITEM_H

#include <QObject>
#include <QLineEdit>
#include <QKeyEvent>

class DeviceCodeItem : public QLineEdit
{
    Q_OBJECT
public:
    DeviceCodeItem(QWidget *parent = nullptr);

    void setInput();
    void exitInput();
    void clearText();
    void setHead();
    void setEnd();

protected:
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;

Q_SIGNALS:
    void sigTextInput(QString);
    void sigBackspace();

private Q_SLOTS:
    void slotTextChanged();
    void slotCursorPositionChanged();

private:
    bool m_isActiveDelete = false;
    bool m_isHead = false;
    bool m_isEnd = false;
    bool m_isInput = false;
};

#endif  // DEVICECODEITEM_H
