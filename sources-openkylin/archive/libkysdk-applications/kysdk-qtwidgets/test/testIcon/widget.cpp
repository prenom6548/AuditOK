#include "widget.h"
#include "kwidget.h"
#include "themeController.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

// using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QPushButton *btn = new QPushButton();
    QIcon icon1 = QIcon::fromTheme("kylin-music");
    QIcon icon2 = QIcon::fromTheme("kylin-video");
    btn->setIcon(ThemeController::overlayIcons(icon1, icon2, 16, 16, 8, 8));
    QLabel *label = new QLabel();
    label->setPixmap(ThemeController::overlayPixmaps(icon2.pixmap(24, 24), icon1.pixmap(16, 16), 8, 8));
    layout->addWidget(btn);
    layout->addWidget(label);
}

Widget::~Widget()
{
}
