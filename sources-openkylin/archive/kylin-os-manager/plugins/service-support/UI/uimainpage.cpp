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
#include <QPainter>
#include <QStyleOption>
#include <QLocale>
#include <kborderlessbutton.h>
#include "uiservicesupport.h"
#include "service-support-backend/src/settings.h"
#include "../constant.h"
#include "frame.h"

UIMainPage *UIMainPage::m_instance = nullptr;

UIMainPage::UIMainPage(QWidget *parent) : QWidget(parent)
{
    initMainPageUI();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &UIMainPage::onthemeChange);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, &UIMainPage::onFontSizeChange);

    connect(m_feedback, &UIMainPageItem::onUIMainPageItemClick, this, &UIMainPage::itemIndexChangeFromString);
    connect(m_self, &UIMainPageItem::onUIMainPageItemClick, this, &UIMainPage::itemIndexChangeFromString);
    connect(m_history, &UIMainPageItem::onUIMainPageItemClick, this, &UIMainPage::itemIndexChangeFromString);
    connect(m_faq, &UIMainPageItem::onUIMainPageItemClick, this, &UIMainPage::itemIndexChangeFromString);

    connect(this, &UIMainPage::itemIndexChangeFromString, this, &UIMainPage::onItemClicked);
    connect(this, &UIMainPage::itemIndexChangeFromString, m_serviceSupportUI, &UiServiceSupport::itemIndexChangeFromString);

    //首页点击自动跳转FAQ
    connect(this, &UIMainPage::sigSearchIssue, this, [=]{
        itemIndexChangeFromString(NAME_FAQ);
        onItemClicked("");
    });
    connect(this, &UIMainPage::sigIssueBtnClick, this, [=]{
        itemIndexChangeFromString(NAME_FAQ);
        onItemClicked("");
    });
    connect(this, &UIMainPage::sigUIMainIssueTypeItemClick, this, [=]{
        itemIndexChangeFromString(NAME_FAQ);
        onItemClicked("");
    });
    connect(this, &UIMainPage::sigSearchIssue, m_serviceSupportUI, &UiServiceSupport::sigSearchIssue);
    connect(this, &UIMainPage::sigIssueBtnClick, m_serviceSupportUI, &UiServiceSupport::sigSearchIssue);
    connect(this, &UIMainPage::sigUIMainIssueTypeItemClick, m_serviceSupportUI, &UiServiceSupport::sigUIMainIssueTypeItemClick);

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
    m_serviceSupportUI->show();
    m_mainPagePri->hide();
    if (str == NAME_FAQ) {
        Frame::ChangeBackButtonState(1);
    }
}

void UIMainPage::komCallBack(ClassType classType, QVariant var) {
    switch (classType) {
        case ClassType::CallbackClickTab:
            getInstance()->showMainPage();
            break;
        case ClassType::CallbackClickPluginProblemFeedback:
            getInstance()->m_feedback->clicked();
            break;
        case ClassType::kSwitchToFaqSpecificType: {
            auto type = var.toString();
            if (type == "application_exception") {
                getInstance()->sigUIMainIssueTypeItemClick(tr("Application usage exception"));
            } else if (type == "network_exception") {
                getInstance()->sigUIMainIssueTypeItemClick(tr("Network connection exception"));
            } else if (type == "hardware_exception") {
                getInstance()->sigUIMainIssueTypeItemClick(tr("Hardware exception"));
            } else if (type == "system_exception") {
                getInstance()->sigUIMainIssueTypeItemClick(tr("System exception"));
            }
            break;
        }
        default:
            break;
    }
}

void UIMainPage::onBackButtonClick() {
    auto instance = m_serviceSupportUI->CurrentDisplayInstance();
    if (instance == nullptr || !instance->GoBack()) {
        showMainPage();
        Frame::ChangeBackButtonState(0);
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
    ft.setBold(true);
    ft.setPixelSize(24 * fontSizeCoefficient);
    m_title->setFont(ft);
}

void UIMainPage::initMainPageUI()
{
    m_serviceSupportUI = new UiServiceSupport(this);
    m_mainPagePri = new QWidget(this);
    m_mainPagePri->setFixedSize(824, 600);

    m_title = new QLabel(m_mainPagePri);
    m_title->setText(tr("Which of the following exceptions have you encountered?"));
    onFontSizeChange();

    m_searchLineEdit = new kdk::KSearchLineEdit(this);
    m_searchLineEdit->setFixedWidth(230);
    m_searchLineEdit->setPlaceholderText(tr("Enter the problem you encountered"));
    m_searchLineEdit->setCompleter(nullptr);
    connect(m_searchLineEdit, &QLineEdit::returnPressed, this, [=](){
        qDebug() << "回车后搜索框内容：" << m_searchLineEdit->text();
        emit sigSearchIssue(m_searchLineEdit->text());
    });

    m_feedback = new UIMainPageItem(m_mainPagePri);
    m_feedback->setItemText(tr("Feedback"));
    m_feedback->setItemText2(tr("Submit problem opinions for feedback"));
    m_feedback->setObjectName(NAME_PROBLEM_FEEDBACK);
    m_self = new UIMainPageItem(m_mainPagePri);
    m_self->setItemText(tr("Self service"));
    m_self->setItemText2(tr("Follow us for updates"));
    m_self->setObjectName(NAME_SELF_SERVICE);
    m_history = new UIMainPageItem(m_mainPagePri);
    m_history->setItemText(tr("History"));
    m_history->setItemText2(tr("View submitted feedback"));
    m_history->setObjectName(NAME_HISTORY_FEEDBACK);
    m_faq = new UIMainPageItem(m_mainPagePri);
    m_faq->setItemText(tr("FAQ"));
    m_faq->setObjectName(NAME_FAQ);
    m_faq->hide();

    m_issueWidget = new QWidget(this);
    m_issueWidget->setFixedWidth(748);

    // 异常区域-应用异常
    m_appUseIssue = new UIMainIssueTypeItem(m_issueWidget);
    m_appUseIssue->setItemText(tr("Application usage exception"));
    QStringList listAppIssue;
    if (QLocale().language() == QLocale::Chinese) {
        listAppIssue << "QQ扫码登录后闪退" << "无法切换输入法" << "软件窗口黑屏" << "无法打开微信" << "无法打开软件商店";
    }
    //listAppIssue << tr("QQ QR code login crashes after scanning")
    //             << tr("Unable to switch input method")
    //             << tr("Software window black screen")
    //             << tr("Unable to open wechat")
    //             << tr("Unable to open software store");
    m_appUseIssue->setItemIssueList(listAppIssue);

    // 异常区域-网络连接异常
    m_netConnIssue = new UIMainIssueTypeItem(m_issueWidget);
    m_netConnIssue->setItemText(tr("Network connection exception"));
    QStringList listNetIssue;
    if (QLocale().language() == QLocale::Chinese) {
        listNetIssue << "网络图标无法显示" << "DNS无法自动同步" << "无法连接外网" << "未检测到网卡" << "WiFi密码输入不显示";
    }
    //listNetIssue << tr("The network icon cannot be displayed")
    //             << tr("DNS cannot automatically synchronize")
    //             << tr("Unable to connect to the external network")
    //             << tr("No network card detected")
    //             << tr("WiFi password input does not display");
    m_netConnIssue->setItemIssueList(listNetIssue);

    // 异常区域-硬件异常
    m_hardwareIssue = new UIMainIssueTypeItem(m_issueWidget);
    m_hardwareIssue->setItemText(tr("Hardware exception"));
    QStringList listHardwareIssue;
    if (QLocale().language() == QLocale::Chinese) {
        listHardwareIssue << "无法打开数据光盘" << "移动硬盘只读" ;
    }
    //listHardwareIssue << tr("Unable to open data disc")
    //                  << tr("Mobile hard drive read-only");
    m_hardwareIssue->setItemIssueList(listHardwareIssue);

    // 异常区域-系统异常
    m_SystemIssue = new UIMainIssueTypeItem(m_issueWidget);
    m_SystemIssue->setItemText(tr("System exception"));
    QStringList listSysIssue;
    if (QLocale().language() == QLocale::Chinese) {
        listSysIssue << "桌面黑屏" << "占用CPU内存较高" ;
    }
    //listSysIssue << tr("Desktop black screen")
    //             << tr("Higt CPU memory usage");
    m_SystemIssue->setItemIssueList(listSysIssue);

    // 自定义布局，确保布局内容可以自动换行
    m_flowLayout = new FlowLayout(m_issueWidget, 0, 16); // 16 两个控件之间的间隙
    m_flowLayout->addWidget(m_appUseIssue);
    m_flowLayout->addWidget(m_netConnIssue);
    m_flowLayout->addWidget(m_hardwareIssue);
    m_flowLayout->addWidget(m_SystemIssue);

    // 滚动区域，防止调整字号大小导致内容截断，滚动显示
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidget(m_issueWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_labelTip = new QLabel(this);
    m_labelTip->setText(tr("More automated services"));
    m_framline = new FrameLine(this);
    m_framline->setMinimumWidth(652);
    m_framline->setFrameStyle(QFrame::VLine | QFrame::Plain);
    m_framline->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *hlCardLeft4 = new QHBoxLayout();
    hlCardLeft4->setMargin(0);
    hlCardLeft4->setSpacing(0);
    hlCardLeft4->addWidget(m_labelTip);
    hlCardLeft4->addSpacing(8);
    hlCardLeft4->addWidget(m_framline);

    QHBoxLayout *hl = new QHBoxLayout();
    hl->setMargin(0);
    hl->setSpacing(24);
    hl->addWidget(m_feedback);
    hl->addWidget(m_history);
    hl->addWidget(m_self);
    hl->addWidget(m_faq);

    QHBoxLayout *hl1 = new QHBoxLayout();
    hl1->addWidget(m_title);
    hl1->addStretch();
    hl1->addWidget(m_searchLineEdit);

    QVBoxLayout *vl = new QVBoxLayout(m_mainPagePri);
    vl->setContentsMargins(40, 10, 40, 30);
    vl->setSpacing(0);
    vl->addLayout(hl1);
    vl->addSpacing(10);
    vl->addWidget(m_scrollArea);
    vl->addSpacing(24);
    vl->addLayout(hlCardLeft4);
    vl->addSpacing(16);
    vl->addLayout(hl);
    vl->addStretch();
    setItemIcons();
}

void UIMainPage::setItemIcons()
{
    QString theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (theme == "ukui-default" || theme == "ukui-light") {
        m_feedback->setIconPath(":/res/feedback-light.png");
        m_self->setIconPath(":/res/self-light.png");
        m_history->setIconPath(":/res/histroy-light.png");
        m_faq->setIconPath(":/res/histroy-light.png");
    } else {
        m_feedback->setIconPath(":/res/feedback-dark.png");
        m_self->setIconPath(":/res/self-dark.png");
        m_history->setIconPath(":/res/histroy-dark.png");
        m_faq->setIconPath(":/res/histroy-dark.png");
    }
}

void UIMainPage::onthemeChange()
{
    setItemIcons();
}

UIMainPageItem::UIMainPageItem(QWidget *parent) : kdk::KPushButton(parent)
{
    setMinimumSize(237, 114);
    setBorderRadius(6);
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
    m_Icon->setFixedSize(22, 22);
    m_Icon->setIconSize(QSize(22, 22));
    m_Icon->setFlat(true);
    m_Icon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_Text = new kom::KomLabel(this);
    m_Text->setBold(true);
    m_Text->setFontSize(16);
    m_Text->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_Text2 = new kom::KomLabel(this);
    m_Text2->setEnabled(false);
    m_Text2->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(24, 16, 24, 16);
    vl->setSpacing(0);
    vl->addWidget(m_Icon);
    vl->addWidget(m_Text);
    vl->addWidget(m_Text2);
    this->setLayout(vl);
}

void UIMainPageItem::setItemText(const QString &str)
{
    m_Text->setText(str);
}

void UIMainPageItem::setItemText2(const QString &str)
{
    m_Text2->setText(str);
}

void UIMainPageItem::setIconPath(const QString &str)
{
    m_Icon->setIcon(QPixmap(str));
}

void UIMainPageItem::onClicked()
{
    emit onUIMainPageItemClick(objectName());
}

UIMainIssueTypeItem::UIMainIssueTypeItem(QWidget *parent) : QWidget(parent)
{
    this->setFixedWidth(364);

    m_textLab = new QLabel(this);

    m_iconBtn = new kdk::KBorderlessButton(this);
    m_iconBtn->setFixedSize(16, 16);
    m_iconBtn->setIconSize(QSize(16, 16));               /*设置ico类型图片大小*/
    m_iconBtn->setIcon(QIcon::fromTheme("ukui-end-symbolic"));
    connect(m_iconBtn, &QPushButton::clicked, this, [=](){
        qDebug() << m_textLab->text();
        emit UIMainPage::getInstance()->sigUIMainIssueTypeItemClick(m_issueTypeStr);
    });
    QHBoxLayout *hl = new QHBoxLayout();
    hl->addWidget(m_textLab);
    hl->addStretch();
    hl->addWidget(m_iconBtn);

    m_widget = new QWidget(this);
    m_widget->setMinimumWidth(316);

    m_flowLayou = new FlowLayout(m_widget, 0, 8);// 8 两个控件之间的间隙

    m_vl = new QVBoxLayout(this);
    m_vl->setSpacing(0);
    m_vl->setMargin(0);
    m_vl->addLayout(hl);
    m_vl->addSpacing(16);
    m_vl->addWidget(m_widget);
    m_vl->setContentsMargins(24, 20, 24, 20);
}

void UIMainIssueTypeItem::setItemText(const QString &str)
{
    m_issueTypeStr = str;
    m_textLab->setText(str);
}

void UIMainIssueTypeItem::setItemIssueList(const QStringList &list)
{
    for (int i = 0; i < list.count(); i++) {
        QPushButton *btn = new QPushButton(list.at(i), this);
        connect(btn, &QPushButton::clicked, this, [=](){
            qDebug() << list.at(i);
            emit UIMainPage::getInstance()->sigIssueBtnClick(list.at(i));
        });
        m_flowLayou->addWidget(btn);
    }
}

void UIMainIssueTypeItem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = qApp->palette().color(QPalette::Window);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setOpacity(0.8);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 8, 8);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);

}
