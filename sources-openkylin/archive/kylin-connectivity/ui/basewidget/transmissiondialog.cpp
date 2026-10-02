#include "transmissiondialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <windowmanage.hpp>
#include "highlight-effect.h"

TransmissionDialog::TransmissionDialog(QWidget *parent) : QDialog(parent)
{
    setFixedSize(400, 100);
    m_titleLab = new QLabel(this);
    m_progressBar = new QProgressBar(this);
    m_closeBtn = new QPushButton(this);

    m_titleLab->setText(tr("Current progress"));
    m_progressBar->setFixedSize(350, 15);
    m_closeBtn->setFixedSize(30, 30);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFlat(true);
    connect(m_closeBtn, &QPushButton::clicked, [=]() {
        close();
        Q_EMIT sigCancelAbort();
    });

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addWidget(m_titleLab);
    hLayout1->addStretch();
    hLayout1->addWidget(m_closeBtn);
    hLayout1->setContentsMargins(4, 4, 4, 4);

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addStretch();
    hLayout2->addWidget(m_progressBar);
    hLayout2->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(hLayout1);
    layout->addSpacing(5);
    layout->addLayout(hLayout2);
    layout->addStretch();

    setLayout(layout);
    setWindowModality(Qt::ApplicationModal); // 模态窗口

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    kabase::WindowManage::removeHeader(this);
}

void TransmissionDialog::setProgressBarValue(qint64 value)
{
    m_progressBar->setValue(value);
}

void TransmissionDialog::setProgressBarRange(qint64 min, qint64 max)
{
    m_progressBar->setRange(min, max);
}
