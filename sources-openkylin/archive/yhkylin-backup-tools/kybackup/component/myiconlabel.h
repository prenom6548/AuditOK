#ifndef MYICONLABEL_H
#define MYICONLABEL_H

#include "circlelabel.h"

class MyIconLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyIconLabel(QWidget *parent = nullptr);
    virtual ~MyIconLabel();

    void setThemeIcon(const QString &themeIconName, const QString &defaultIconName = "", int size = 16);

    void setDesplayText(const QString &text);

public slots:
    // void changePalette(bool checked);

private:
    CircleLable *m_iconLabel;
    QLabel *m_textLabel;
    QString m_themeIconName;
    QString m_defaultIconName;
    QString m_originalText;
    int m_iconSize;
};

#endif // MYICONLABEL_H
