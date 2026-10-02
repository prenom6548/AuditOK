#include "uimainpage.h"
#include <QDebug>
#include <QLabel>
#include <QBoxLayout>
#include <QPalette>
#include <QApplication>
#include <QDesktopServices>
#include <gsettingmonitor.h>
#include <QTimer>
#include <QUrl>
#include <kborderlessbutton.h>
#include "uiservicesupport.h"
#include "service-support-backend/src/settings.h"
#include "../constant.h"

UIMainPage *UIMainPage::m_instance = nullptr;

UIMainPage::UIMainPage(QWidget *parent) : QWidget(parent)
{
    initMainPageUI();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this,
            &UIMainPage::onthemeChange);
    connect(m_feedback, &UIMainPageItem::onUIMainPageItemClick, this, &UIMainPage::itemIndexChangeFromString);
    connect(m_self, &UIMainPageItem::onUIMainPageItemClick, this, &UIMainPage::itemIndexChangeFromString);
    connect(m_history, &UIMainPageItem::onUIMainPageItemClick, this, &UIMainPage::itemIndexChangeFromString);
    connect(this, &UIMainPage::itemIndexChangeFromString, this, &UIMainPage::onItemClicked);
    connect(this, &UIMainPage::itemIndexChangeFromString, m_serviceSupportUI,
            &UiServiceSupport::itemIndexChangeFromString);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this,
            &UIMainPage::onFontSizeChange);
    showMainPage();
}

UIMainPage *UIMainPage::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new UIMainPage();
    }
    return m_instance;
}

void UIMainPage::showMainPage()
{
    m_serviceSupportUI->hide();
    m_mainPagePri->show();
}

void UIMainPage::onItemClicked(const QString &str)
{
    if (str == NAME_PROBLEM_FEEDBACK) {}
    m_serviceSupportUI->show();
    m_mainPagePri->hide();
}

void UIMainPage::komCallBack(ClassType classType, QVariant var)
{
    if (classType == CallbackClickTab) {
        getInstance()->showMainPage();
    }
}

void UIMainPage::onFontSizeChange()
{
    double fontSizeCoefficient = kdk::GsettingMonitor::getSystemFontSize().toDouble() / 10.0;
    if (fontSizeCoefficient == 0) {
        qCritical() << "获取不到字号大小的值！";
        return;
    }
    QFont ft;
    ft.setPixelSize(24 * fontSizeCoefficient);
    m_title->setFont(ft);
}

void UIMainPage::initMainPageUI()
{
    m_serviceSupportUI = new UiServiceSupport(this);
    m_mainPagePri = new QWidget(this);
    m_mainPagePri->setFixedSize(824, 600);

    m_title = new QLabel(m_mainPagePri);
    m_title->setText(tr("ServiceSupport"));
    onFontSizeChange();
    QLabel *secondTitle = new QLabel(m_mainPagePri);
    secondTitle->setText(tr("Multi-channel technical support services"));

    m_feedback = new UIMainPageItem(m_mainPagePri);
    m_feedback->setItemText(tr("Feedback"));
    m_feedback->setObjectName(NAME_PROBLEM_FEEDBACK);
    m_self = new UIMainPageItem(m_mainPagePri);
    m_self->setItemText(tr("Self service"));
    m_self->setObjectName(NAME_SELF_SERVICE);
    m_history = new UIMainPageItem(m_mainPagePri);
    m_history->setItemText(tr("History"));
    m_history->setObjectName(NAME_HISTORY_FEEDBACK);

    QLabel *pic = new QLabel(m_mainPagePri);
    pic->setPixmap(QPixmap(":/res/pic.png").scaled(744, 264));
    QHBoxLayout *hlCardLeft4 = new QHBoxLayout();
    hlCardLeft4->setMargin(0);
    hlCardLeft4->setSpacing(0);
    hlCardLeft4->addStretch(9);
    hlCardLeft4->addWidget(new QLabel(tr("Jump to"), m_mainPagePri));
    kdk::KBorderlessButton *lessBtn = new kdk::KBorderlessButton(" openKylin " + tr("website"));
    connect(lessBtn, &kdk::KBorderlessButton::clicked, m_mainPagePri, [=] {
        QDesktopServices::openUrl(QUrl("https://www.openkylin.top"));
    });
    hlCardLeft4->addWidget(lessBtn);
    QLabel *label2 = new QLabel(m_mainPagePri);
    label2->setMinimumWidth(160);
    label2->setText(tr(" to get more services"));
    hlCardLeft4->addWidget(label2);

    QHBoxLayout *hl = new QHBoxLayout();
    hl->setMargin(0);
    hl->setSpacing(24);
    hl->addWidget(m_feedback);
    hl->addWidget(m_self);
    hl->addWidget(m_history);
    hl->addStretch(9);
    QVBoxLayout *vl = new QVBoxLayout(m_mainPagePri);
    vl->setContentsMargins(40, 24, 40, 0);
    vl->setSpacing(0);
    vl->addWidget(m_title);
    vl->addWidget(secondTitle);
    vl->addSpacing(24);
    vl->addWidget(pic);
    vl->addSpacing(8);
    vl->addLayout(hlCardLeft4);
    vl->addSpacing(24);
    vl->addLayout(hl);
    vl->addStretch(9);
    setItemIcons();
}

void UIMainPage::setItemIcons()
{
    QString theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (theme == "ukui-default" || theme == "ukui-light") {
        m_feedback->setIconPath(":/res/feedback-light.png");
        m_self->setIconPath(":/res/self-light.png");
        m_history->setIconPath(":/res/histroy-light.png");
    } else {
        m_feedback->setIconPath(":/res/feedback-dark.png");
        m_self->setIconPath(":/res/self-dark.png");
        m_history->setIconPath(":/res/histroy-dark.png");
    }
}

void UIMainPage::onthemeChange()
{
    setItemIcons();
}

UIMainPageItem::UIMainPageItem(QWidget *parent) : kdk::KPushButton(parent)
{
    setBorderRadius(6);
    setFixedSize(168, 72);
    setBackgroundColor(qApp->palette().color(QPalette::Window));

    connect(this, &UIMainPageItem::clicked, this, &UIMainPageItem::onClicked);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=] {
        //适配深浅色，临时方案，待修改
        QTimer *timer = new QTimer;
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [=] {
            setBackgroundColor(qApp->palette().color(QPalette::Window));
            timer->deleteLater();
        });
        timer->start(50);
    });

    m_Icon = new QPushButton(this);
    m_Icon->setFixedSize(40, 40);
    m_Icon->setIconSize(QSize(40, 40));
    m_Icon->setFlat(true);
    m_Icon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_Text = new kom::KomLabel(this);
    m_Text->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_Text->setFixedWidth(96);
    QVBoxLayout *vlIcon = new QVBoxLayout;
    vlIcon->setContentsMargins(0, 16, 0, 16);
    vlIcon->addWidget(m_Icon);
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setContentsMargins(20, 0, 0, 0);
    hl->addLayout(vlIcon);
    //    hl->addSpacing(12);
    hl->addWidget(m_Text);
    hl->addStretch(9);
    this->setLayout(hl);
}

void UIMainPageItem::setItemText(const QString &str)
{
    m_Text->setText(str);
}

void UIMainPageItem::setIconPath(const QString &str)
{
    m_Icon->setIcon(QPixmap(str));
}

void UIMainPageItem::onClicked()
{
    emit onUIMainPageItemClick(objectName());
}
