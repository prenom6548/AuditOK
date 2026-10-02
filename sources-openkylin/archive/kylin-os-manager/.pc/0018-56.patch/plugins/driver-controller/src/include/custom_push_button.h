#ifndef CUSTOM_PUSH_BUTTON_H
#define CUSTOM_PUSH_BUTTON_H

#include <kpushbutton.h>
#include <QString>

class CustomPushButton : public kdk::KPushButton
{
    Q_OBJECT
public:
    explicit CustomPushButton(QWidget *parent=0);
    explicit CustomPushButton(QString &text, QWidget *parent=0);

    void setText(const QString &text);
    void setFullText(const QString &text);
    void setTextLimitShrink(const QString &text, int width);
    void setTextLimitExpand(const QString &text);
    QString fullText() const;
protected:
    void paintEvent(QPaintEvent *);

private:
    void elideText();
private:
    QString m_fullText;
};
#endif // CUSTOM_PUSH_BUTTON_H
