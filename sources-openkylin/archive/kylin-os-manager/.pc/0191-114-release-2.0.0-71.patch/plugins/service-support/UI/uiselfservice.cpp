#include "uiselfservice.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QDesktopServices>
#include <kborderlessbutton.h>
#include "../kom/kom-radiuswidget.h"

UiSelfService::UiSelfService(QWidget *parent) : QWidget(parent)
{
    setObjectName(NAME_SELF_SERVICE);
    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(40, 48, 40, 0);
    vl->setSpacing(0);
    vl->addWidget(new QLabel(tr("Contact us"), this));
    vl->addSpacing(16);

    //背景板
    kom::RadiusWidget *cardBase = new kom::RadiusWidget(this);
    cardBase->setFixedHeight(212);
    cardBase->setColorRole(QPalette::Window);
    cardBase->setRadius(6);
    QWidget *card = new QWidget(this);

    int iconSize = 20;

    QHBoxLayout *hlCardLeft2 = new QHBoxLayout();
    hlCardLeft2->setMargin(0);
    hlCardLeft2->setSpacing(12);
    QPushButton *icon2 = new QPushButton(card);
    icon2->setIcon(QIcon::fromTheme("mail-read-symbolic"));
    icon2->setFixedWidth(iconSize);
    icon2->setFlat(true);
    icon2->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    hlCardLeft2->addWidget(icon2);
    hlCardLeft2->addWidget(new QLabel(tr("Mail"), card));
    hlCardLeft2->addSpacing(4);
    hlCardLeft2->addWidget(new QLabel("contact@openkylin.top", card), 9);

    QHBoxLayout *hlCardLeft3 = new QHBoxLayout();
    hlCardLeft3->setMargin(0);
    hlCardLeft3->setSpacing(12);
    QPushButton *icon3 = new QPushButton(card);
    icon3->setIcon(QIcon::fromTheme("ukui-stock-people-symbolic", QIcon(":/res/ukui-stock-people-symbolic.svg")));
    icon3->setFixedWidth(iconSize);
    icon3->setFlat(true);
    icon3->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    hlCardLeft3->addWidget(icon3);
    hlCardLeft3->addWidget(new QLabel(tr("Community"), card));
    hlCardLeft3->addSpacing(4);
    QLabel *label1 = new QLabel(card);
    label1->setText("https://gitee.com/openkylin");
    hlCardLeft3->addWidget(label1, 9);

    QHBoxLayout *hlCardLeft4 = new QHBoxLayout();
    hlCardLeft4->setMargin(0);
    hlCardLeft4->setSpacing(0);
    hlCardLeft4->addWidget(new QLabel(tr("Jump to"), card));
    kdk::KBorderlessButton *lessBtn = new kdk::KBorderlessButton(" openKylin " + tr("website"));
    connect(lessBtn, &kdk::KBorderlessButton::clicked, card, [=] {
        QDesktopServices::openUrl(QUrl("https://www.openkylin.top"));
    });
    hlCardLeft4->addWidget(lessBtn);
    QLabel *label2 = new QLabel(card);
    label2->setMinimumWidth(160);
    label2->setText(tr(" to get more services"));
    hlCardLeft4->addWidget(label2, 9);

    QVBoxLayout *vlCardLeft = new QVBoxLayout();
    vlCardLeft->setMargin(0);
    vlCardLeft->setSpacing(16);
    vlCardLeft->addLayout(hlCardLeft2);
    vlCardLeft->addLayout(hlCardLeft3);
    vlCardLeft->addStretch(9);
    vlCardLeft->addLayout(hlCardLeft4);

    QLabel *rightImage = new QLabel(card);
    rightImage->setAlignment(Qt::AlignCenter);
    rightImage->setFixedSize(118, 118);
    rightImage->setPixmap(QPixmap(":/res/qcpic.png").scaled(118, 118));
    QLabel *rightTextnew = new QLabel(card);
    rightTextnew->setAlignment(Qt::AlignCenter);
    rightTextnew->setText(tr("Kylin technical services"));
    rightTextnew->setFixedHeight(30);

    QHBoxLayout *hlRightImage = new QHBoxLayout;
    hlRightImage->setMargin(0);
    hlRightImage->setSpacing(0);
    hlRightImage->addStretch(9);
    hlRightImage->addWidget(rightImage);
    hlRightImage->addStretch(9);

    QVBoxLayout *vlCardRight = new QVBoxLayout;
    vlCardRight->setMargin(0);
    vlCardRight->setSpacing(0);
    vlCardRight->addLayout(hlRightImage);
    vlCardRight->addStretch(9);
    vlCardRight->addSpacing(8);
    vlCardRight->addWidget(rightTextnew);

    QHBoxLayout *hlCard = new QHBoxLayout(cardBase);
    hlCard->setMargin(32);
    hlCard->addLayout(vlCardLeft, 4);
    hlCard->addLayout(vlCardRight, 1);

    vl->addWidget(cardBase);
    vl->addStretch(9);
}
