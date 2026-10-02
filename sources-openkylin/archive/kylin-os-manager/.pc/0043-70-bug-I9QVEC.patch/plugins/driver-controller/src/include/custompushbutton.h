#ifndef CUSTOMPUSHBUTTON_H
#define CUSTOMPUSHBUTTON_H

#include <kpushbutton.h>
#include <QString>

class CustomPushButton : public QPushButton
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
#endif // CUSTOMPUSHBUTTON_H
