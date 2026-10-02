#include "cleanupfinishpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

namespace systemcleanup
{

CleanupFinishPage::CleanupFinishPage(QWidget *parent)
    : QWidget(parent)
    , m_labelIcon(new QLabel(this))
    , m_labelType(new K::TextLabel(this))
    , m_labelInfo1(new K::TextLabel(this))
    , m_labelInfo2(new K::TextLabel(this))
    , m_labelInfo3(new K::TextLabel(this))
    , m_btn(new QPushButton(this))
    , m_btnBack(new QPushButton(this))
    , m_btnRetry(new QPushButton(this))
{
    this->setBackgroundRole(QPalette::Base);

    m_strTypeSucc = tr("Cleaning completed");
    m_strTypeFail = tr("Cleaning failed");
    m_strInfo1 = tr("Together, we have freed up"); // 共帮您释放了
    m_strInfo2 = tr("disk space for you"); // 磁盘空间
    m_strInfo3 = tr(" files do not have permission to clean. It is recommended to obtain root permission before trying again");
    m_strBtn = tr("OK"); // 好的
    m_strBtnBack = tr("Back"); // 返回
    m_strBtnRetry = tr("Retry"); // 重试


    m_labelType->SetPixelSize(24);

    m_labelInfo1->SetFontColor(QColor("#8C8C8C"));
    m_labelInfo1->SetPixelSize(14);

    m_labelInfo2->SetFontColor(QColor("#3790FA"));
    m_labelInfo2->SetPixelSize(14);

    m_labelInfo3->SetFontColor(QColor("#8C8C8C"));
    m_labelInfo3->SetPixelSize(14);

    m_btn->setText(m_strBtn);
    m_btn->setProperty("isImportant", true);
    m_btnBack->setText(m_strBtnBack);
    m_btnBack->setProperty("isImportant", false);
    m_btnRetry->setText(m_strBtnRetry);
    m_btnRetry->setProperty("isImportant", true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->addStretch();
    hLayout->addWidget(m_labelInfo1);
    hLayout->addWidget(m_labelInfo2);
    hLayout->addWidget(m_labelInfo3);
    hLayout->addStretch();

    QHBoxLayout *hLayoutBtn = new QHBoxLayout;
    hLayoutBtn->setSpacing(0);
    hLayoutBtn->setMargin(0);
    hLayoutBtn->addStretch();
    hLayoutBtn->addWidget(m_btnBack);
    hLayoutBtn->addSpacing(10);
    hLayoutBtn->addWidget(m_btnRetry);
    hLayoutBtn->addSpacing(10);
    hLayoutBtn->addWidget(m_btn);
    hLayoutBtn->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addStretch();
    vLayout->addWidget(m_labelIcon, 0, Qt::AlignCenter);
    vLayout->addSpacing(24);
    vLayout->addWidget(m_labelType, 0, Qt::AlignCenter);
    vLayout->addSpacing(8);
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(40);
    vLayout->addLayout(hLayoutBtn);
    vLayout->addStretch();

    this->setLayout(vLayout);

    connect(m_btn, &QPushButton::clicked, this, &CleanupFinishPage::slotClickBtn);
    connect(m_btnBack, &QPushButton::clicked, this, &CleanupFinishPage::signalBtnBack);
    connect(m_btnRetry, &QPushButton::clicked, this, &CleanupFinishPage::signalBtnRetry);
}

CleanupFinishPage::~CleanupFinishPage()
{
}

void CleanupFinishPage::setCleanUpSucc(QString size)
{
    QIcon icon = QIcon::fromTheme("ukui-dialog-success");
    m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(QSize(64, 64))));
    m_labelIcon->setAttribute(Qt::WA_TranslucentBackground, true);

    m_labelType->SetText(m_strTypeSucc);
    m_labelInfo1->SetText(m_strInfo1);
    m_labelInfo2->SetText(size);
    m_labelInfo2->setToolTip("");
    m_labelInfo3->SetText(m_strInfo2);

    m_btnBack->hide();
    m_btnRetry->hide();
    m_btn->show();
}

void CleanupFinishPage::setCleanUpFail(QStringList list)
{
    // 设置图标
    QIcon icon = QIcon::fromTheme("dialog-error");
    m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(QSize(64, 64))));
    m_labelIcon->setAttribute(Qt::WA_TranslucentBackground, true);

    // 设置空间的内容
    m_labelType->SetText(m_strTypeFail);
    m_labelInfo1->hide();
    m_labelInfo2->SetText(QString::number(list.size()));
    QString fileList = "";
    for (int i = 0; i < list.size(); i++) {
        if (i == (list.size() - 1)) {
            fileList = fileList + list.at(i);
        } else {
            fileList = fileList + list.at(i) + "\n";
        }
    }
    m_labelInfo2->setToolTip(fileList);
    m_labelInfo3->SetText(m_strInfo3);

    // 设置显示按钮
    m_btnBack->show();
    m_btnRetry->show();
    m_btn->hide();
}

void CleanupFinishPage::slotClickBtn()
{
    Q_EMIT signalNextPage();
    return;
}


}