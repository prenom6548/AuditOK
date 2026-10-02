#ifndef DISCONNECTBUTTON_H
#define DISCONNECTBUTTON_H

#include <QObject>
#include <QPushButton>
#include "publicattributes.hpp"

class DisconnectButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DisconnectButton(QWidget *parent = nullptr);
    void setTheme(PublicAttributes::Theme theme);
};

#endif  // DISCONNECTBUTTON_H
