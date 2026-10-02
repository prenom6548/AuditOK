#include "dialogmessage.h"
#include "dialogmigrationlogs.h"
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "kom-buriedpoint.h"

DialogMessage::DialogMessage(QWidget *parent) : DialogCommon(parent)
{
    initUI();
    initWidgetErrList();
}

void DialogMessage::connetcResult(int errCode, QVariant var)
{
    switch (errCode) {
    case SMB_START_SAVE: {
        m_icon->hide();
        m_labelMessage->setText(Constant::g_startSaveText);
        m_cancel->setText(Constant::g_cancelText);
        m_progressBar->setState(kdk::NormalProgress);
        m_progressBar->show();
        m_progressBar->setValue(0);
        break;
    }
    case SMB_CREAT_LOG_FINISH: {
        DialogMigrationLogs::getInstance()->updateView();
        break;
    }
    case SMB_ERROR_3: {
        errorDialog(Constant::g_notChooseText);
	m_cancel->setText(Constant::g_okText);
        break;
    }
    case SMB_ERROR_4: {
        errorDialog(Constant::g_saveFileFailedText);
	m_cancel->setText(Constant::g_okText);
        break;
    }
    case SMB_ERROR_5: {
        m_errList = var.value<SmbUrlInfoList>();
        int err = m_errList.length();
        if (!err) {
            m_viewDetails->hide();
            m_labelMessage->setText(Constant::g_migrationSuccessText);
        } else {
            m_icon->show();
            m_viewDetails->show();
            m_labelMessage->setText(QString::number(err) + Constant::g_saveFailedText);
            m_progressBar->hide();
        }
        m_cancel->setText(Constant::g_okText);
        break;
    }
    case SMB_ERROR_6: {
        errorDialog(Constant::g_creatMigrationLogFailedText);
	m_cancel->setText(Constant::g_okText);
        break;
    }
    case SMB_ERROR_7: {
        errorDialog(Constant::g_smbDisconnectText);
	m_cancel->setText(Constant::g_okText);
        exec();
        break;
    }
    }
}

void DialogMessage::saveProgressValue(int num)
{
    m_progressBar->setValue(num);
}

void DialogMessage::saveProgressMax(int num)
{
    m_icon->hide();
    minimumButton()->hide();
    m_viewDetails->hide();
    m_progressBar->show();
    m_progressBar->setMaximum(num);
    m_cancel->setText(Constant::g_cancelText);
}

void DialogMessage::downloadingFile(const QString &str)
{
    m_labelMessage->setText(str);
}

void DialogMessage::onCancelClick()
{
    if (m_cancel->text() == Constant::g_cancelText) {
        *m_cancelHandle = true;
        m_labelMessage->setText(Constant::g_cancellingText);
        m_cancel->hide();
        m_progressBar->hide();
        kom::BuriedPoint::uploadMessage(kom::PluginsWinDataMigration, "startmigration");
        return;
    }
    this->close();
}

void DialogMessage::onViewDetailsClick()
{
    m_listViewModel->clear();
    for (SmbUrlInfo errInfo : m_errList) {
        QStandardItem *item = new QStandardItem;
        item->setSizeHint(QSize(0, 36));
        item->setText(errInfo.url);
        item->setEditable(false);
        item->setSelectable(false);
        if (errInfo.type == 8) { // libsmbclient的type值，8代表普通文件
            item->setIcon(QIcon::fromTheme("application-octet-stream"));
        } else {
            item->setIcon(QIcon::fromTheme("folder-open"));
        }
        m_listViewModel->appendRow(item);
    }
    m_dialogErrList->exec();
}

void DialogMessage::onResaveClick()
{
    emit resave();
    m_dialogErrList->hide();
    m_listViewModel->clear();
    m_progressBar->setState(kdk::NormalProgress);
    m_progressBar->setValue(0);
    m_progressBar->show();
}

void DialogMessage::setCancelHandle(bool *cancelHandle)
{
    m_cancelHandle = cancelHandle;
}

void DialogMessage::clearAndShow()
{
    m_icon->hide();
    m_viewDetails->hide();
    exec();
}

void DialogMessage::cancelFileMigrate()
{
    qDebug() << " === === == 取消迁移 界面关闭 == === === ";
    if (!m_cancel->isHidden()) {
        return;
    }
    m_cancel->setText(Constant::g_okText);
    m_progressBar->show();
    m_progressBar->setState(kdk::NormalProgress);
    m_progressBar->setValue(0);
    m_cancel->show();
    this->setEnabled(true);
    this->close();
    return;
}

void DialogMessage::initUI()
{
    setFixedWidth(424);
    setFixedHeight(200);

    QVBoxLayout *vl = new QVBoxLayout;
    vl->setContentsMargins(32, 24, 32, 24);

    QHBoxLayout *hl1 = new QHBoxLayout;
    m_icon = new QLabel(this);
    m_icon->setFixedSize(24, 24);
    m_icon->setPixmap(QPixmap(":/res/warning.svg"));
    m_icon->hide();
    hl1->addWidget(m_icon);
    m_labelMessage = new QLabel(this);
    hl1->addWidget(m_labelMessage);
    hl1->addStretch(9);
    vl->addLayout(hl1);

    m_progressBar = new kdk::KProgressBar(this);
    m_progressBar->setFixedHeight(16);
    connect(m_progressBar, &kdk::KProgressBar::valueChanged, this, [=](){
        if(m_progressBar->value()==m_progressBar->maximum())
            m_progressBar->setState(kdk::NormalProgress);
    });
    vl->addWidget(m_progressBar);

    vl->addStretch(9);

    QHBoxLayout *hl2 = new QHBoxLayout;
    hl2->addStretch(9);
    m_viewDetails = new QPushButton(this);
    m_viewDetails->setFixedWidth(96);
    m_viewDetails->setText(Constant::g_viewText); // 查看按钮
    connect(m_viewDetails, &QPushButton::clicked, this, &DialogMessage::onViewDetailsClick);
    hl2->addWidget(m_viewDetails);
    m_cancel = new QPushButton(this);
    m_cancel->setFixedWidth(96);
    m_cancel->setText(Constant::g_cancelText); // 取消按钮
    connect(m_cancel, &QPushButton::clicked, this, &DialogMessage::onCancelClick);
    hl2->addWidget(m_cancel);
    vl->addLayout(hl2);

    mainWidget()->setLayout(vl);
}

void DialogMessage::initWidgetErrList()
{
    if (m_dialogErrList) {
        return;
    }
    m_dialogErrList = new DialogCommon;
    m_dialogErrList->setFixedWidth(700);
    m_dialogErrList->setFixedHeight(441);

    QVBoxLayout *vl = new QVBoxLayout;
    vl->setContentsMargins(24, 16, 24, 22);
    QHBoxLayout *hl = new QHBoxLayout;
    QLabel *label = new QLabel(m_dialogErrList);
    label->setText(Constant::g_migrationErrorText);
    hl->addWidget(label);
    hl->addStretch(9);
    QPushButton *resave = new QPushButton(m_dialogErrList);
    resave->setFixedSize(80, 32);
    resave->setText(Constant::g_retryText); // 重试按钮
    connect(resave, &QPushButton::clicked, this, &DialogMessage::onResaveClick);
    hl->addWidget(resave);
    vl->addLayout(hl);
    QListView *listView = new QListView;
    listView->setAlternatingRowColors(true);
    m_listViewModel = new QStandardItemModel;
    listView->setModel(m_listViewModel);
    vl->addWidget(listView);
    m_dialogErrList->mainWidget()->setLayout(vl);
}

void DialogMessage::errorDialog(const QString &err)
{
    m_icon->show();
    m_labelMessage->setText(err);
    m_progressBar->hide();
    m_viewDetails->hide();
}

void DialogMessage::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    if (m_cancel->isHidden()){
        event->ignore();
    } else if (m_cancel->text() == Constant::g_cancelText) {
        *m_cancelHandle = true;
        m_labelMessage->setText(Constant::g_cancellingText);
        m_cancel->hide();
        m_progressBar->hide();
        event->ignore();
    }
}
