#include "uiserviceonline.h"
#include <QDebug>
#include <QBoxLayout>
#include <gsettingmonitor.h>
#include <QDesktopServices>
#include <QUrl>
#include "kom-configure.h"

UiServiceOnline::UiServiceOnline(QWidget *parent) : QWidget(parent)
{
    setObjectName(m_objectName);

    m_cardBase = new QWidget(this);
    m_cardBase->setObjectName("QWebEngineViewPage");
    m_cardBase->setStyleSheet("QWidget#QWebEngineViewPage{border:1px solid #EBEBEB;border-radius:6px;}");
    m_webEngineViewWiget = new QWidget(m_cardBase);
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setContentsMargins(40, 0, 40, 24);
    hl->addWidget(m_cardBase);
    QVBoxLayout *vl = new QVBoxLayout(m_cardBase);
    vl->addWidget(m_webEngineViewWiget);
    vl->setMargin(2);
    m_cardBase->hide();

    // 状态占位图和文字描述模块
    m_iconWidget = new QWidget(this);
    m_iconWidget->setFixedSize(824, 528);
    m_labelText = new QLabel(m_iconWidget);
    QPalette pal = m_labelText->palette();
    pal.setColor(QPalette::Text, Qt::lightGray);
    m_labelText->setPalette(pal);
    m_labelIcon = new QPushButton(m_iconWidget);
    m_labelIcon->setFixedSize(96, 96);
    m_labelIcon->setIconSize(QSize(96, 96));
    m_labelIcon->setFlat(true);
    m_labelIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_retryBtn = new QPushButton(m_iconWidget);
    m_retryBtn->setFixedHeight(36);
    m_retryBtn->setMinimumWidth(96);
    QString loadingText = tr("Loading, please wait");
    QVBoxLayout *vlIconWidget = new QVBoxLayout(m_iconWidget);
    vlIconWidget->addStretch(9);
    vlIconWidget->addWidget(m_labelIcon);
    vlIconWidget->addWidget(m_labelText);
    vlIconWidget->addWidget(m_retryBtn);
    vlIconWidget->addStretch(9);
    vlIconWidget->addSpacing(50);
    vlIconWidget->setAlignment(m_labelIcon, Qt::AlignHCenter);
    vlIconWidget->setAlignment(m_labelText, Qt::AlignHCenter);
    vlIconWidget->setAlignment(m_retryBtn, Qt::AlignHCenter);

#ifdef HAS_WEBENGINE
    // sw架构没有WEBWNGINE，弹出浏览器在线客服
    m_labelText->setText(loadingText);
    m_retryBtn->hide();
    m_retryBtn->setText(tr("retry"));
    connect(m_retryBtn, &QPushButton::clicked, this, [=] {
        m_retryBtn->hide();
        m_iconType = Loading;
        m_labelText->setText(loadingText);
        setIconPix();
        creatWebPage();
    });
#else
    m_iconType = NoWebEngine;
    m_labelText->setText(tr("Go to your browser"));
    m_retryBtn->setText(tr("Go"));
    connect(m_retryBtn, &QPushButton::clicked, this, [=] {
        QDesktopServices::openUrl(QUrl(m_onlineUrl));
    });
#endif

    setIconPix();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &UiServiceOnline::setIconPix);

    kom::Configure conf;
    m_onlineUrl = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_SERVICE_ONLINE).toString();
    if (m_onlineUrl.isEmpty()) {
        m_onlineUrl = "https://webchat-bj.clink.cn/chat.html?accessId=8b626ce9-8d56-4b68-b6f9-de357a088258&language=zh_CN";
    }
}

UiServiceOnline::~UiServiceOnline() {}

void UiServiceOnline::creatWebPage() {
    qInfo() << "加载在线客服页面";
    m_loaded = true;

#ifdef HAS_WEBENGINE
    if (m_webView == nullptr) {
        qInfo() << "Service support online service init WebEngineView start...";

        m_webView = new QWebEngineView(m_webEngineViewWiget);
        m_webView->setFixedWidth(740);
        // 如果是默认页面，隐藏蓝色上边框
        m_webView->move(0, -60);
        m_webView->setFixedHeight(557 + 60);
        auto *page = m_webView->page();
        page->setAudioMuted(true);
        connect(page, &QWebEnginePage::loadFinished, this, [this](bool ok) {
            if (ok) {
                m_iconWidget->hide();
                m_cardBase->show();
            } else {
                m_labelText->setText(tr("There is a network problem, please try again later"));
                m_retryBtn->show();
                m_iconType = IconType::Error;
                setIconPix();
            }
        });
        m_webView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
        m_webView->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);
        m_webView->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);

        qInfo() << "Service support online service init WebEngineView end";
    }

    m_webView->setUrl(QUrl(m_onlineUrl));
#endif
}

void UiServiceOnline::setIconPix()
{
    QString theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (m_iconType == Loading) {
        if (theme == "ukui-default" || theme == "ukui-light") {
            m_labelIcon->setIcon(QIcon(":/res/loding-light.png"));
        } else {
            m_labelIcon->setIcon(QIcon(":/res/loding-dark.png"));
        }
    } else if (m_iconType == Error) {
        if (theme == "ukui-default" || theme == "ukui-light") {
            m_labelIcon->setIcon(QIcon(":/res/lodingerr-light.png"));
        } else {
            m_labelIcon->setIcon(QIcon(":/res/lodingerr-dark.png"));
        }
    } else if (m_iconType == NoWebEngine) {
        if (theme == "ukui-default" || theme == "ukui-light") {
            m_labelIcon->setIcon(QIcon(":/res/online-light.png"));
        } else {
            m_labelIcon->setIcon(QIcon(":/res/online-dark.png"));
        }
    }
}

void UiServiceOnline::indexChanged(QString name)
{
    if (m_loaded) {
        return;
    }
    if (name == m_objectName) {
        creatWebPage();
    }
}
