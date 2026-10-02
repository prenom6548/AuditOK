#include "uiselfservice.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <QDesktopServices>
#include <QApplication>
#include <kborderlessbutton.h>
#include "../kom/kom-radiuswidget.h"
#include "kom-label.h"
#include "service-support-backend/src/settings.h"

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
    //cardBase->setFixedHeight(212);
    cardBase->setColorRole(QPalette::Window);
    cardBase->setRadius(6);
    QWidget *card = new QWidget(this);

    int iconSize = 20;

    QHBoxLayout *hlCardLeft1 = new QHBoxLayout();
    hlCardLeft1->setMargin(0);
    hlCardLeft1->setSpacing(12);
    QPushButton *icon1 = new QPushButton(card);
    icon1->setIcon(QIcon::fromTheme("smartphone-symbolic"));
    icon1->setFixedWidth(iconSize);
    icon1->setFlat(true);
    icon1->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    hlCardLeft1->addWidget(icon1);
    hlCardLeft1->addWidget(new QLabel(tr("Telephone"), card));
    hlCardLeft1->addSpacing(4);
    //获取定制配置
    QString call400 = "400-089-1870";
    QString mail = "support@kylinos.cn";
    if (Settings::getCustomize() == Settings::ICBC) {
        call400 = "400-011-1870";
        mail = "kossup@sdc.icbc.com.cn";
    }
    QString callLabelName = "call400Label";
    QLabel *labelCall = new QLabel(card);
    labelCall->setObjectName(callLabelName);
    labelCall->setText(call400);
    labelCall->adjustSize();
    labelCall->setTextInteractionFlags(Qt::TextSelectableByMouse);// label内容可选中
    labelCall->setMouseTracking(true); // 开启鼠标跟踪
    labelCall->installEventFilter(this); // 安装事件过滤器
    m_objectNameStrList.append(callLabelName); 
    hlCardLeft1->addWidget(labelCall);
    hlCardLeft1->addStretch(9);

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
    QString mailLabelName = "mailLabel";
    QLabel *labelMail = new QLabel(mail, card);
    labelMail->setObjectName(mailLabelName);
    labelMail->adjustSize();
    labelMail->setTextInteractionFlags(Qt::TextSelectableByMouse);// label内容可选中
    labelMail->setMouseTracking(true); // 开启鼠标跟踪
    labelMail->installEventFilter(this); // 安装事件过滤器
    m_objectNameStrList.append(mailLabelName); 
    hlCardLeft2->addWidget(labelMail);
    hlCardLeft2->addStretch(9);

    QHBoxLayout *hlCardLeft3 = new QHBoxLayout();
    hlCardLeft3->setMargin(0);
    hlCardLeft3->setSpacing(12);
    QPushButton *icon3 = new QPushButton(card);
    icon3->setIcon(QIcon::fromTheme("ukui-stock-people-symbolic", QIcon(":/res/ukui-stock-people-symbolic.svg")));
    icon3->setFixedWidth(iconSize);
    icon3->setFlat(true);
    icon3->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    hlCardLeft3->addWidget(icon3);
    hlCardLeft3->addWidget(new QLabel(tr("Team"), card));
    hlCardLeft3->addSpacing(4);
    QLabel *label1 = new QLabel(card);
    label1->setText("KylinOS Support Team");
    hlCardLeft3->addWidget(label1, 9);

    QHBoxLayout *hlCardLeft4 = new QHBoxLayout();
    hlCardLeft4->setMargin(0);
    hlCardLeft4->setSpacing(0);
    QLabel *jump = new QLabel(tr("Jump to"), card);
    hlCardLeft4->addWidget(jump);
    kdk::KBorderlessButton *lessQuestionBtn = new kdk::KBorderlessButton(tr(" asked questions, ")); // 常见问题解答
    connect(lessQuestionBtn, &kdk::KBorderlessButton::clicked, card, [=] {
        QDesktopServices::openUrl(QUrl("https://wx.kylinos.cn/zsk"));
    });
    kdk::KBorderlessButton *lessAdapterBtn = new kdk::KBorderlessButton(tr("Adaptation Center ")); // 生态适配中心
    connect(lessAdapterBtn,  &kdk::KBorderlessButton::clicked, card, [=] {
        QDesktopServices::openUrl(QUrl("https://eco.kylinos.cn/"));
    });
    hlCardLeft4->addWidget(lessQuestionBtn);
    hlCardLeft4->addWidget(lessAdapterBtn);
    kom::KomLabel *label2 = new kom::KomLabel(card);
    label2->setText(tr("to get more services"));
    hlCardLeft4->addWidget(label2, 9);

    QVBoxLayout *vlCardLeft = new QVBoxLayout();
    vlCardLeft->setMargin(0);
    vlCardLeft->setSpacing(8);
    vlCardLeft->addLayout(hlCardLeft1);
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

    if (Settings::getCustomize() == Settings::ICBC) {
        jump->hide();
        lessQuestionBtn->hide();
        lessAdapterBtn->hide();
        label2->hide();
    }
}

bool UiSelfService::eventFilter(QObject *watched, QEvent *event)
{
    if (m_objectNameStrList.contains(watched->objectName()) && event->type() == QEvent::Enter) {
        // 鼠标进入区域，改变鼠标形状
        QApplication::setOverrideCursor(Qt::IBeamCursor);
    } else if (m_objectNameStrList.contains(watched->objectName()) && event->type() == QEvent::Leave) {
        // 鼠标离开区域，恢复鼠标形状
        QApplication::restoreOverrideCursor();
    }
    return QObject::eventFilter(watched, event);
}
