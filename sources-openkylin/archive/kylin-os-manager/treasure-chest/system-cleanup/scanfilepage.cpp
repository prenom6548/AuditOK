#include "scanfilepage.h"
#include <QVBoxLayout>

namespace systemcleanup
{

ScanFilePage::ScanFilePage(QWidget *parent)
    : QWidget(parent)
    , m_label(new QLabel(this))
    , m_progress(new kdk::KProgressBar(this))
    , m_btnCannelScan(new QPushButton(this))
    , m_timer(new QTimer(this))
    , m_dialog(new ScanCannelDialog(this))
{
    this->setBackgroundRole(QPalette::Base);
    m_dialog->hide();

    m_strLabel = tr("Scanning disk...");
    m_strBtn = tr("Cancel Scan");

    m_label->setText(m_strLabel);

    m_progress->setRange(0,100);
    m_progress->setFixedSize(568, 22);

    m_btnCannelScan->setText(m_strBtn);
    m_btnCannelScan->setProperty("useButtonPalette", true);
    m_btnCannelScan->setProperty("isWindowButton", 0x1);
    m_btnCannelScan->setProperty("useIconHighlightEffect", 0x2);

    m_timer->setInterval(100);

    QVBoxLayout *vLayoutProgress = new QVBoxLayout;
    vLayoutProgress->setSpacing(0);
    vLayoutProgress->setMargin(0);
    vLayoutProgress->addWidget(m_label);
    vLayoutProgress->addSpacing(11);
    vLayoutProgress->addWidget(m_progress, 0, Qt::AlignCenter);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addLayout(vLayoutProgress);
    hLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addSpacing(218);
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(115);
    vLayout->addWidget(m_btnCannelScan, 0, Qt::AlignCenter);
    vLayout->addSpacing(105);

    this->setLayout(vLayout);

    connect(m_btnCannelScan, &QPushButton::clicked, this, &ScanFilePage::slotStopScan);
    connect(m_dialog, &ScanCannelDialog::signalCannelScan, this, &ScanFilePage::slotCannelScan);
    connect(m_dialog, &ScanCannelDialog::signalContinueScan, this, &ScanFilePage::slotContinueScan);
    connect(m_timer, &QTimer::timeout, this, [=](){
        if (m_progress->value() != m_progress->maximum()) {
            timeNum++;
            if (timeNum < 5) {
                m_progress->setValue(m_progress->value() + 5);
            } else if (timeNum < 15) {
                m_progress->setValue(m_progress->value() + 3);
            } else if (timeNum < 40) {
                m_progress->setValue(m_progress->value() + 1);
            } else if (timeNum < 60) {
                if ( timeNum % 3 == 0) {
                    m_progress->setValue(m_progress->value() + 1);
                }
            } else if (timeNum < 100) {
                if ( timeNum % 5 == 0) {
                    m_progress->setValue(m_progress->value() + 1);
                }
            } else {
                if (isProcessFinish) {
                    m_progress->setValue(m_progress->maximum());
                }
            }

        } else {
            m_progress->setValue(m_progress->minimum());
            timeNum = 0;
            isProcessFinish = false;
        }
    });
    connect(m_progress, &kdk::KProgressBar::valueChanged, this, &ScanFilePage::slotProcessValue);
}

ScanFilePage::~ScanFilePage()
{
}

void ScanFilePage::setProgressValue(int value, bool isScanSameFile)
{
    m_isScanSameFile = isScanSameFile;
    m_progress->setValue(0);
    m_progress->setRange(0,100);
    if (isScanSameFile) {
        return;
    }
    if (!m_timer->isActive()) {
        m_timer->start();
    }
    if (value == 0) {
        timeNum = 0;
        isProcessFinish = false;
    } else {
        m_progress->setValue(value);
    }
}

void ScanFilePage::slotContinueScan()
{
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void ScanFilePage::slotCannelScan()
{
    m_progress->setValue(0);
    m_timer->stop();
    Q_EMIT signalCannelScan();
    return;
}

void ScanFilePage::slotStopScan()
{
    m_timer->stop();
    m_dialog->exec();
}

void ScanFilePage::slotProcessValue(int value)
{
    if (value == m_progress->maximum()) {
        m_progress->setState(ProgressBarState::NormalProgress);
        Q_EMIT signalScanSuccess();
        m_progress->setValue(0);
        m_timer->stop();
    }
}

void ScanFilePage::slotProcessFinish()
{
    isProcessFinish = true;
}

void ScanFilePage::slotProcessValueChanged(int current, int total)
{
    // qDebug() << "current:" << current << "total:" << total;
    m_progress->setValue(current);
    m_progress->setMaximum(total);
}
void ScanFilePage::slotProcessValueChangedMax()
{
    if (m_isScanSameFile) {
        int n = m_progress->maximum() - m_progress->value();
        if (n != 5) {
            return;
        }
    }
    m_progress->setValue(m_progress->maximum());
}
}