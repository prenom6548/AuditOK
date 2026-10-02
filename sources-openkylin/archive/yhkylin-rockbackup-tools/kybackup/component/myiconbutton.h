#ifndef MYICONBUTTON_H
#define MYICONBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QPalette>
#include "mylabel.h"

class MyIconButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MyIconButton(QWidget *parent = nullptr);
    ~MyIconButton();

    void setThemeIcon(const QString &themeIconName, const QString &defaultIconName = "", int size = 16);

    void setDesplayText(const QString &text);
    void flashText() {
        setDesplayText(m_originalText);
    }

    QString pluginBtnHoverColor(bool hoverFlag);

public slots:
    void changePalette(bool checked);

private:
    QPushButton *m_iconButton;
    MyLabel *m_textLabel;
    QString m_themeIconName;
    QString m_defaultIconName;
    QString m_originalText;

    QString m_hoverColor;
    QString m_clickColor;
    int m_pixSize;
};

#endif // MYICONBUTTON_H
