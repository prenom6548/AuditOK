#include "scancanneldialog.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGuiApplication>
#include <QScreen>

namespace systemcleanup
{

ScanCannelDialog::ScanCannelDialog(QWidget *parent)
    : KDialog(parent)
    , m_labelSummary(new K::TextLabel(this))
    , m_labelInfo(new K::TextLabel(this))
    , m_btnIcon(new QPushButton(this))
    , m_btnContinu(new QPushButton(this))
    , m_btnCannelScan(new QPushButton(this))
{
    setFixedSize(424, 186);

    m_strCannel = tr("Cancel Scan");
    m_strContinue = tr("Continue");
    m_strInfo = tr("To ensure the scanning effect, we suggest that you choose to continue the analysis !");
    m_strTitle = tr("Are you sure you want to cancel this scan ?");

    m_labelSummary->SetText(m_strTitle);
    m_labelSummary->SetPixelSize(14);

    m_labelInfo->SetText(m_strInfo);
    m_labelInfo->SetPixelSize(14);
    m_labelInfo->SetFontColor(QColor("#8C8C8C"));

    m_btnIcon->setFixedSize(25, 25);
    m_btnIcon->setIconSize(QSize(24, 24));
    m_btnIcon->setIcon(QIcon::fromTheme("dialog-warning")); /*使用fromTheme函数调用库中的图片*/
    m_btnIcon->setFlat(true);
    m_btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_btnContinu->setText(m_strContinue);
    m_btnContinu->setProperty("useButtonPalette", true);
    m_btnContinu->setProperty("isWindowButton", 0x1);
    m_btnContinu->setProperty("useIconHighlightEffect", 0x2);
    
    m_btnCannelScan->setText(m_strCannel);
    m_btnCannelScan->setProperty("isImportant", true);

    QGridLayout *gridLyout = new QGridLayout;
    gridLyout->addWidget(m_btnIcon, 0, 0);
    gridLyout->addWidget(m_labelSummary, 0, 1);
    gridLyout->addWidget(m_labelInfo, 1, 1);
    gridLyout->setHorizontalSpacing(8);
    gridLyout->setVerticalSpacing(8);

    QHBoxLayout *hLayoutBtn = new QHBoxLayout;
    hLayoutBtn->setSpacing(0);
    hLayoutBtn->setMargin(0);
    hLayoutBtn->addStretch();
    hLayoutBtn->addWidget(m_btnContinu);
    hLayoutBtn->addSpacing(10);
    hLayoutBtn->addWidget(m_btnCannelScan);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addLayout(gridLyout);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutBtn);
    vLayout->setContentsMargins(24, 5, 24, 24);
    mainWidget()->setLayout(vLayout);

    connect(m_btnCannelScan, &QPushButton::clicked, this, &ScanCannelDialog::slotBtnCannelScan);
    connect(m_btnContinu, &QPushButton::clicked, this, &ScanCannelDialog::slotBtnContinue);
    connect(closeButton(), &QPushButton::clicked, this, &ScanCannelDialog::slotBtnContinue);
}

ScanCannelDialog::~ScanCannelDialog()
{
}

void systemcleanup::ScanCannelDialog::slotBtnContinue()
{
    Q_EMIT signalContinueScan();
    this->close();
}

void systemcleanup::ScanCannelDialog::slotBtnCannelScan()
{
    Q_EMIT signalCannelScan();
    this->close();
}

}