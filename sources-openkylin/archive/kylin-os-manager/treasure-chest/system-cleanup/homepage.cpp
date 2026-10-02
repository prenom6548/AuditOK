#include "homepage.h"
#include <QVBoxLayout>

namespace systemcleanup
{

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , m_labelSummary(new K::TextLabel(this))
    , m_labelInfo(new K::TextLabel(this))
    , m_radioBigFile(new RadioWidget(this))
    , m_radioSameFile(new RadioWidget(this))
    , m_btnScan(new QPushButton(this))
{
    this->setBackgroundRole(QPalette::Base);

    m_strSummary = tr("System Cleanup");
    m_strInfo = tr("Regularly delete useless large files and duplicate files that occupy disk space, freeing up more disk space");
    m_strBtnScan = tr("Start Scan");
    m_strBigFile = tr("Scan Large File");
    m_strSameFile = tr("Scan Same File");

    m_labelSummary->SetText(m_strSummary);
    m_labelSummary->SetPixelSize(24);

    m_labelInfo->SetText(m_strInfo);
    m_labelInfo->SetFontColor(QColor("#8C8C8C"));
    m_labelInfo->SetPixelSize(14);

    QIcon icon = QIcon(":/image/homeimage_light.png");
    m_radioBigFile->setLabelText(m_strBigFile);
    m_radioBigFile->setLabelIcon(icon);

    QIcon iconSameFile = QIcon(":/image/homeimage_samefile_light.png");
    m_radioSameFile->setLabelText(m_strSameFile);
    m_radioSameFile->setLabelIcon(iconSameFile);

    m_btnScan->setText(m_strBtnScan);
    m_btnScan->setProperty("isImportant", true);
    m_btnScan->setEnabled(false);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->addStretch();
    hLayout->addWidget(m_radioBigFile);
    hLayout->addSpacing(42);
    hLayout->addWidget(m_radioSameFile);
    hLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addSpacing(24);
    vLayout->addWidget(m_labelSummary, 0, Qt::AlignCenter);
    vLayout->addWidget(m_labelInfo, 0, Qt::AlignCenter);
    vLayout->addSpacing(51);
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(69);
    vLayout->addWidget(m_btnScan, 0, Qt::AlignCenter);
    vLayout->addStretch();

    this->setLayout(vLayout);

    connect(m_btnScan, &QPushButton::clicked, this, &HomePage::slotClickBtn);
    connect(m_radioBigFile , &RadioWidget::sigClicked, this, &HomePage::slotClickBigFileRadioBtn);
    connect(m_radioBigFile , &RadioWidget::sigCannel, this, &HomePage::slotCanncelClickBigFileRadioBtn);
    connect(m_radioSameFile , &RadioWidget::sigClicked, this, &HomePage::slotClickSameFileRadioBtn);
    connect(m_radioSameFile , &RadioWidget::sigCannel, this, &HomePage::slotCanncelClickSameFileRadioBtn);
}

HomePage::~HomePage()
{
}

void HomePage::slotClickBigFileRadioBtn()
{
    if (m_radioSameFile->getCilckStyle()) {
        m_radioSameFile->setClickStyle();
    }
    if (m_boolChooseSameFile) {
        m_boolChooseSameFile = false;
    }
    m_boolChooseBigFile = true;
    m_btnScan->setEnabled(true);
}

void HomePage::slotClickSameFileRadioBtn()
{
    if (m_radioBigFile->getCilckStyle()) {
        m_radioBigFile->setClickStyle();
    }
    if (m_boolChooseBigFile) {
        m_boolChooseBigFile = false;
    }
    m_boolChooseSameFile = true;
    m_btnScan->setEnabled(true);
 }

void HomePage::slotCanncelClickSameFileRadioBtn()
{
    if (m_boolChooseSameFile) {
        m_boolChooseSameFile = false;
        m_btnScan->setEnabled(false);
    } else {
        if (!m_boolChooseBigFile) {
            m_btnScan->setEnabled(false);
        }
    }
    return;
}

void HomePage::slotCanncelClickBigFileRadioBtn()
{
    if (m_boolChooseBigFile) {
        m_boolChooseBigFile = false;
        m_btnScan->setEnabled(false);
    } else {
        if (!m_boolChooseSameFile) {
            m_btnScan->setEnabled(false);
        }
    }
    return;
}

void HomePage::slotClickBtn()
{
    if (m_radioSameFile->getCilckStyle()) {
        Q_EMIT signalStartScanSameFile();
    } else if (m_radioBigFile->getCilckStyle()) {
        Q_EMIT signalStartScanBigFile();
    }
    return;
}


}