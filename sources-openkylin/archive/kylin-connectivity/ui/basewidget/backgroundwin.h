#ifndef BACKGROUNDWIN_H
#define BACKGROUNDWIN_H

#include <QWidget>
#include "publicattributes.hpp"

class BackgroundWin : public QWidget
{
    Q_OBJECT
public:
    BackgroundWin(QWidget *parent = nullptr);

    void setTheme(PublicAttributes::Theme theme);
    void setBackground(bool isDefault);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light;
    bool m_isDefaultColor = true;

Q_SIGNALS:
};

#endif // BACKGROUNDWIN_H
