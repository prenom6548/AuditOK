#include "disconnectbutton.h"

DisconnectButton::DisconnectButton(QWidget *parent) : QPushButton(parent)
{
    setFixedSize(48, 48);
}

void DisconnectButton::setTheme(PublicAttributes::Theme theme)
{
    switch (theme) {
        case PublicAttributes::Theme::Light: {
            setStyleSheet(
                "QPushButton "
                "{border:0px;border-radius:8px;background:#FFFFFF;background-image:url(:/view/exit-light-default.svg);}"
                "QPushButton:Hover"
                "{border:0px;border-radius:8px;background:#FFFFFF;background-image:url(:/view/exit-hover.svg);}"
                "QPushButton:Pressed{border:0px;border-radius:8px;background:#FFFFFF;background-image:url(:/"
                "view/exit-click.svg);}");
        } break;
        case PublicAttributes::Theme::Dark: {
            setStyleSheet(
                "QPushButton "
                "{border:0px;border-radius:4px;background:#373737;background-image:url(:/view/exit-dark-default.svg);}"
                "QPushButton:Hover"
                "{border:0px;border-radius:4px;background:#373737;background-image:url(:/view/exit-hover.svg);}"
                "QPushButton:Pressed{border:0px;border-radius:4px;background:#373737;background-image:url(:/"
                "view/exit-click.svg);}");
        } break;
    }
    update();
}