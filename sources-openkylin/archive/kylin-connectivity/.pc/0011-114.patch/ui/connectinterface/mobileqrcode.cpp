#include "mobileqrcode.h"

#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include "generatetools.h"

QString CODE_TXT = "https://kylinos.cn/?connectip=%1&version=%2&verify=ky";
const QString APP_CODE_TXT = "https://www.openkylin.top/public/software/ky_connectivity-1.0.0.1.apk";

MobileQRcode::MobileQRcode(QWidget *parent) : QWidget(parent)
{
    initWidget();
}

void MobileQRcode::setQRCode(QString codeInfo)
{
    if (codeInfo.isEmpty()) {
        QImage image(":/connectinterface/defaultCode.png");

        QPainter painter(&image);
        painter.setPen(QPalette::Base);
        QFont font;
        font.setPointSize(12);
        painter.setFont(font);
        painter.drawText(image.rect(), Qt::AlignCenter, tr("No network"));

        m_pixmap = QPixmap::fromImage(image);
        m_qrCodeLabel->setPixmap(m_pixmap);
        m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    } else {
        m_pixmap = GenerateTools::getCode(CODE_TXT.arg(codeInfo).arg(qApp->applicationVersion()));
        m_qrCodeLabel->setPixmap(m_pixmap);
    }
}

void MobileQRcode::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    if (m_theme == PublicAttributes::Theme::Light) {
        if (m_appDownLab != nullptr) {
            m_appDownLab->setText("<a href=\" \""
                                  "style=\"color:#262626;text-decoration:underline\">" +
                                  tr("No app installed? Install Now") + "</a>");
        }
        if (m_descLab1 != nullptr) {
            m_descLab1->setText("<a href=\"https://www.openkylin.top/assistant.html\""
                                "style=\"color:#262626;text-decoration:underline\">" +
                                tr("view supported phone types>>") + "</a>");
        }
    } else {
        if (m_appDownLab != nullptr) {
            m_appDownLab->setText("<a href=\" \""
                                  "style=\"color:#D9D9D9;text-decoration:underline\">" +
                                  tr("No app installed? Install Now") + "</a>");
        }
        if (m_descLab1 != nullptr) {
            m_descLab1->setText("<a href=\"https://www.openkylin.top/assistant.html\""
                                "style=\"color:#D9D9D9;text-decoration:underline\">" +
                                tr("view supported phone types>>") + "</a>");
        }
    }
}

void MobileQRcode::initWidget()
{
    m_qrCodeLabel = new QLabel(this);
    m_illustrate = new QLabel(this);
    m_scanQRCode = new QLabel(this);
    m_appDownLab = new QLabel(this);

    // 二维码
    m_qrCodeLabel->setFixedSize(174, 174);
    m_qrCodeLabel->setStyleSheet("background-color:#FFFFFF;");

    // 二维码扫描
    // 说明
    m_illustrate = new QLabel(this);
    m_illustrate->setFixedWidth(640);
    m_illustrate->setAlignment(Qt::AlignCenter);
    m_illustrate->setWordWrap(true);
    m_illustrate->setText(tr(
        "Connect the mobile phone and computer to the same network,open the mobile phone app and scan the QR code."));

    m_scanQRCode->setText(tr("Use the mobile app to scan this code"));
    QPalette pal;
    pal.setColor(QPalette::Text, QColor("#8F9399"));
    m_scanQRCode->setPalette(pal);

    if (m_theme == PublicAttributes::Theme::Light) {
        if (m_appDownLab != nullptr) {
            m_appDownLab->setText("<a href=\" \""
                                  "style=\"color:#262626;text-decoration:underline\">" +
                                  tr("No app installed? Install Now") + "</a>");
        }
    } else {
        if (m_appDownLab != nullptr) {
            m_appDownLab->setText("<a href=\" \""
                                  "style=\"color:#D9D9D9;text-decoration:underline\">" +
                                  tr("No app installed? Install Now") + "</a>");
        }
    }
    connect(m_appDownLab, &QLabel::linkActivated, this, &MobileQRcode::onLinkActivated);

    QHBoxLayout *codeLayout = new QHBoxLayout();
    codeLayout->setMargin(0);
    codeLayout->setSpacing(0);
    codeLayout->addStretch();
    codeLayout->addWidget(m_qrCodeLabel);
    codeLayout->addStretch();

    QHBoxLayout *illustrateLayout = new QHBoxLayout();
    illustrateLayout->setMargin(0);
    illustrateLayout->setSpacing(0);
    illustrateLayout->addStretch();
    illustrateLayout->addWidget(m_illustrate);
    illustrateLayout->addStretch();

    QHBoxLayout *scanQRCodeLayout = new QHBoxLayout();
    scanQRCodeLayout->setMargin(0);
    scanQRCodeLayout->setSpacing(0);
    scanQRCodeLayout->addStretch();
    scanQRCodeLayout->addWidget(m_scanQRCode);
    scanQRCodeLayout->addStretch();

    QHBoxLayout *appDownLayout = new QHBoxLayout();
    appDownLayout->setMargin(0);
    appDownLayout->setSpacing(0);
    appDownLayout->addStretch();
    appDownLayout->addWidget(m_appDownLab);
    appDownLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(102);
    mainLayout->addLayout(codeLayout);
    mainLayout->addSpacing(24);
    mainLayout->addLayout(illustrateLayout);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(scanQRCodeLayout);
    mainLayout->addSpacing(24);
    mainLayout->addLayout(appDownLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    setQRCode();
}

void MobileQRcode::onLinkActivated(const QString url)
{
    m_downAppWin = new kdk::KDialog(this);
    m_downAppWin->setAutoFillBackground(true);
    m_downAppWin->setBackgroundRole(QPalette::Base);

    QLabel *codeLabel = new QLabel(m_downAppWin);
    m_descLab1 = new QLabel(m_downAppWin);
    QLabel *descLab2 = new QLabel(m_downAppWin);

    codeLabel->setFixedSize(174, 174);
    codeLabel->setStyleSheet("background-color:#FFFFFF;");

    QPixmap pixmap = GenerateTools::getCode(APP_CODE_TXT);
    codeLabel->setPixmap(pixmap);
    codeLabel->setAlignment(Qt::AlignCenter);

    if (m_theme == PublicAttributes::Theme::Light) {
        m_descLab1->setText("<a href=\"https://www.openkylin.top/assistant.html\""
                            "style=\"color:#262626;text-decoration:underline\">" +
                            tr("view supported phone types>>") + "</a>");

    } else {
        m_descLab1->setText("<a href=\"https://www.openkylin.top/assistant.html\""
                            "style=\"color:#D9D9D9;text-decoration:underline\">" +
                            tr("view supported phone types>>") + "</a>");
    }
    connect(m_descLab1, &QLabel::linkActivated, this, [=](const QString url) { QDesktopServices::openUrl(QUrl(url)); });
    m_descLab1->setMargin(0);

    descLab2->setText(tr("Please scan this QR code with your mobile browser to download the app"));
    descLab2->setMargin(0);
    descLab2->adjustSize();
    descLab2->setWordWrap(true);
    descLab2->setFixedWidth(280);
    descLab2->setAlignment(Qt::AlignCenter);

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addStretch();
    hlayout1->addWidget(codeLabel);
    hlayout1->addStretch();
    hlayout1->setMargin(0);

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addStretch();
    hlayout2->addWidget(m_descLab1);
    hlayout2->addStretch();
    hlayout2->setMargin(0);

    QHBoxLayout *hlayout3 = new QHBoxLayout;
    hlayout3->addStretch();
    hlayout3->addWidget(descLab2);
    hlayout3->addStretch();
    hlayout3->setMargin(0);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->addSpacing(21);
    layout->addLayout(hlayout1);
    layout->addSpacing(6);
    layout->addLayout(hlayout3);
    layout->addSpacing(16);
    layout->addLayout(hlayout2);
    layout->addStretch();
    layout->setContentsMargins(24, 0, 24, 24);

    m_downAppWin->mainWidget()->setLayout(layout);
    m_downAppWin->layout()->setSizeConstraint(QLayout::SetFixedSize);
    m_downAppWin->setWindowModality(Qt::WindowModal);
    m_downAppWin->setWindowModality(Qt::ApplicationModal);
    m_downAppWin->setAttribute(Qt::WA_DeleteOnClose);
    m_downAppWin->show();
    m_downAppWin->exec();
    m_descLab1 = nullptr;
}
