#include "webdriverinstallwidget.h"

class WebDriverInstallWidgetPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(WebDriverInstallWidget)

public:
    WebDriverInstallWidgetPrivate(WebDriverInstallWidget* parent);

private:
    WebDriverInstallWidget *q_ptr;
    QString m_type;
    QString m_token = "";
    QStringList m_tokenlist;
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
    QLabel *m_linkLabel;
    QPushButton *m_okButton;

    void setupUI();
    void setupConnections();
};

WebDriverInstallWidget::WebDriverInstallWidget(const QStringList& token, QWidget* parent)
        : kdk::KDialog(parent),
          d_ptr(new WebDriverInstallWidgetPrivate(this))
{
    Q_D(WebDriverInstallWidget);

    d->m_tokenlist = token;

    d->setupUI();
    d->setupConnections();
    setFixedSize(500, 270);

    d->m_textLabel->setText(tr("The current device cannot obtain the driver online. To obtain the driver, please use a machine that can access the public network to access this link."));
    d->m_okButton->setText(tr("OK"));
}

WebDriverInstallWidget::WebDriverInstallWidget(const QString& type, const QString& token, QWidget* parent)
        : kdk::KDialog(parent),
          d_ptr(new WebDriverInstallWidgetPrivate(this))
{
    Q_D(WebDriverInstallWidget);

    d->m_type = type;
    d->m_token = token;
    
    d->setupUI();
    d->setupConnections();
    setFixedSize(500, 270);

    d->m_textLabel->setText(tr("The current device cannot obtain the driver online. To obtain the driver, please use a machine that can access the public network to access this link."));
    d->m_okButton->setText(tr("OK"));
}

WebDriverInstallWidget::~WebDriverInstallWidget()
{

}

WebDriverInstallWidgetPrivate::WebDriverInstallWidgetPrivate(WebDriverInstallWidget* parent)
    :q_ptr(parent)
{
    Q_Q(WebDriverInstallWidget);
}

void WebDriverInstallWidgetPrivate::setupUI()
{
    Q_Q(WebDriverInstallWidget);

    // 顶部：图标 + 文字
    m_iconLabel = new QLabel(q);
    m_iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(32, 32));  // 图标资源

    m_textLabel = new QLabel(q);
    m_textLabel->setWordWrap(true);

    QHBoxLayout *topLayout = new QHBoxLayout(q);
    topLayout->addWidget(m_iconLabel, 0, Qt::AlignTop);
    topLayout->addWidget(m_textLabel);
    topLayout->addStretch();

    // 中部：超链接
    m_linkLabel = new QLabel(q);
    m_linkLabel->setTextFormat(Qt::RichText);
    QString url = "https://kom.kylinos.cn/d/";
    if(m_token != ""){
        url += m_token;
    }else{
       url = url + m_tokenlist.join("-");
    }
    
    m_linkLabel->setText(QString("<a href=\"%1\">\%2\</a>").arg(url).arg(url));
    m_linkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_linkLabel->setOpenExternalLinks(true);

    QHBoxLayout *linkLayout = new QHBoxLayout(q);
    linkLayout->addSpacing(40);
    linkLayout->addWidget(m_linkLabel);

    // 底部：确认按钮
    m_okButton = new QPushButton(q);
    m_okButton->setProperty("isImportant", QVariant::fromValue(true));

    QHBoxLayout *buttonLayout = new QHBoxLayout(q);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addSpacing(25);

    // 总布局
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(linkLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 15);

    q->mainWidget()->setLayout(mainLayout);
}

void WebDriverInstallWidgetPrivate::setupConnections()
{
    Q_Q(WebDriverInstallWidget);

    connect(m_okButton, &QPushButton::clicked, q, &WebDriverInstallWidget::accept);
}

#include "webdriverinstallwidget.moc"
#include "moc_webdriverinstallwidget.cpp"
