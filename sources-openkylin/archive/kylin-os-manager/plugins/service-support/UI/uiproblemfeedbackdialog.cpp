#include "uiproblemfeedbackdialog.h"
#include <QBoxLayout>
#include <QDebug>
#include <gsettingmonitor.h>
#include "kom_buriedpoint.h"
UiProblemFeedbackDialog::UiProblemFeedbackDialog(QWidget *parent) : m_parent(parent)
{
    setFixedWidth(424);
    connect(this, &UiProblemFeedbackDialog::rejected, this, [=] {
        if (m_progressBar->isHidden()) {
            return;
        }
        kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kServiceSupport,
                                        kom::BuriedPoint::EventCode::kCancelSubmission);
        FeedbackManager::getInstance()->cancel();
    });
    m_progressBar = new kdk::KProgressBar(this);
    m_progressBar->setBodyWidth(16);
    m_titleText = new QLabel(this);
    m_titleText->setWordWrap(true);
    m_titleText->setMinimumHeight(24);
    m_secondText = new QLabel(this);
    m_secondText->setWordWrap(true);
    m_secondText->setContentsMargins(32, 0, 0, 0);
    QPalette pt = m_secondText->palette();
    pt.setColor(QPalette::WindowText, QColor(Qt::lightGray));
    m_secondText->setPalette(pt);
    m_icon = new QLabel(this);
    QHBoxLayout *hlText = new QHBoxLayout;
    hlText->setMargin(0);
    hlText->setSpacing(8);
    hlText->addWidget(m_icon);
    hlText->addWidget(m_titleText);
    hlText->addStretch(9);
    m_OKBtn = new QPushButton(this);
    m_OKBtn->setFixedHeight(36);
    m_OKBtn->setText(tr("OK"));
    m_OKBtn->setProperty("useButtonPalette", true);
    m_CancelBtn = new QPushButton(this);
    m_CancelBtn->setText(tr("Cancel"));
    m_CancelBtn->setFixedHeight(36);
    m_CancelBtn->setProperty("useButtonPalette", true);
    m_retryBtn = new QPushButton(this);
    m_retryBtn->setFixedHeight(36);
    m_retryBtn->setText(tr("Retry"));
    m_retryBtn->setProperty("isImportant", true);
    connect(m_OKBtn, &QPushButton::clicked, this, [=] {
        close();
    });
    connect(m_CancelBtn, &QPushButton::clicked, this, [=] {
        close();
    });
    connect(m_retryBtn, &QPushButton::clicked, this, [=] {
        emit retryUpload();
        close();
    });
    QHBoxLayout *hlBtn = new QHBoxLayout;
    hlBtn->setMargin(0);
    hlBtn->setSpacing(8);
    hlBtn->addStretch(9);
    hlBtn->addWidget(m_OKBtn);
    hlBtn->addWidget(m_CancelBtn);
    hlBtn->addWidget(m_retryBtn);
    QVBoxLayout *vl = new QVBoxLayout(mainWidget());
    vl->setSpacing(8);
    vl->setContentsMargins(24, 0, 24, 0);
    vl->addLayout(hlText);
    vl->addWidget(m_progressBar);
    vl->addWidget(m_secondText);
    vl->addSpacing(24);
    vl->addLayout(hlBtn);
    vl->addStretch(9);

    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=] {
        if (!isHidden()) {
            showDialogPri();
        }
    });
}

void UiProblemFeedbackDialog::showDialog(FeedBackFinishType type, QString str)
{
    m_icon->show();
    m_OKBtn->show();
    m_progressBar->setValue(0);
    m_progressBar->hide();
    m_secondText->hide();
    m_retryBtn->hide();
    m_CancelBtn->hide();
    switch (type) {
    case Success:
        m_icon->setPixmap(QIcon::fromTheme("ukui-dialog-success", QIcon(":/res/finish.png")).pixmap(24, 24));
        m_titleText->setText(tr("Submitted successfully"));
        break;
    case Cancel:
        m_icon->setPixmap(QIcon::fromTheme("ukui-dialog-success", QIcon(":/res/finish.png")).pixmap(24, 24));
        m_titleText->setText(tr("Cancel successfully"));
        break;
    case Oversize:
        m_icon->setPixmap(QIcon::fromTheme("dialog-error", QIcon(":/res/fail.png")).pixmap(24, 24));
        m_titleText->setText(tr("System is abnormal, contact technical support"));
        m_secondText->setText(tr("Log and submission is packed, please go") + " " + str + " " + tr("acquire"));
        m_secondText->show();
        break;
    case UploadFail:
        m_icon->setPixmap(QIcon::fromTheme("dialog-error", QIcon(":/res/fail.png")).pixmap(24, 24));
        m_titleText->setText(tr("Submission failed"));
        m_secondText->setText(tr("Click 'Retry' to upload again, or contact us directly.")
                              + tr("Log and submission is packed, please go") + " " + str + " " + tr("acquire"));
        m_secondText->show();
        m_retryBtn->show();
        m_CancelBtn->show();
        m_OKBtn->hide();
        break;
    default:
        break;
    }
    showDialogPri();
}

void UiProblemFeedbackDialog::showProgress(int i)
{
    m_progressBar->setValue(i);
    if (isHidden()) {
        m_secondText->hide();
        m_icon->hide();
        m_OKBtn->hide();
        m_CancelBtn->hide();
        m_retryBtn->hide();
        m_titleText->setText(tr("Under submission..."));
        m_progressBar->show();
        showDialogPri();
    }
}

void UiProblemFeedbackDialog::paintEvent(QPaintEvent *event)
{
    if (!m_isChanged) {
        return;
    }
    m_isChanged = false;
    resize(424, 1080);
    int heightNum = 40 + 24;
    heightNum += m_titleText->height();
    if (!m_secondText->isHidden()) {
        heightNum += 8;
        heightNum += m_secondText->height();
    }
    if (!m_progressBar->isHidden()) {
        heightNum += 8;
        heightNum += m_progressBar->height();
    }
    if (!m_OKBtn->isHidden() || !m_CancelBtn->isHidden()) {
        heightNum += 32;
        heightNum += m_OKBtn->height();
    }
    resize(424, heightNum);
    QPoint globalPoint = m_parent->mapToGlobal(QPoint(m_parent->geometry().x(), m_parent->geometry().y()));
    move(globalPoint.x() + (824 - width()) / 2 - 50, globalPoint.y() + (520 - height()) / 2 - 50);
}

void UiProblemFeedbackDialog::showDialogPri()
{
    m_isChanged = true;
    exec();
}
