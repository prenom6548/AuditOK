#ifndef DEVICECODEITEM_H
#define DEVICECODEITEM_H

#include <QLineEdit>
#include <QKeyEvent>
#include "publicattributes.hpp"

class DeviceCodeItem : public QLineEdit
{
    Q_OBJECT
public:
    DeviceCodeItem(QWidget *parent = nullptr);

    void setTheme(PublicAttributes::Theme theme);
    void setInput();
    void exitInput();
    void clearText();
    void setHead();
    void setEnd();

protected:
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

Q_SIGNALS:
    void sigTextInput(QString);
    void sigBackspace();

private Q_SLOTS:
    void slotTextChanged();
    void slotCursorPositionChanged();

private:
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light;
    bool m_isActiveDelete = false;
    bool m_isHead = false;
    bool m_isEnd = false;
};

#endif // DEVICECODEITEM_H
