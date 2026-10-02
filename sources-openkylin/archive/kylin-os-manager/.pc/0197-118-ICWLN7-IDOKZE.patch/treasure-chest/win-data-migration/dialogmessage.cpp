#include "dialogmessage.h"
#include "dialogmigrationlogs.h"
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "kom_buriedpoint.h"
#include "kom-configure.h"
#include <QIcon>
#include <QScreen>
#include <QGuiApplication>

DialogMessage::DialogMessage(QWidget *parent) : DialogCommon(parent)
{
    initUI();
    initWidgetErrList();
    initInteractionBox();
}

DialogMessage::~DialogMessage()
{
    if (m_interactionWidget2 != nullptr) {
        m_interactionWidget2->deleteLater();
    }
}

void DialogMessage::connetcResult(int errCode, QVariant var)
{
    switch (errCode) {
    case SMB_START_SAVE:
    case SMB_START_BACKPOINT_RESUME: {
        m_labelMessage->SetText(Constant::g_startSaveText);
        m_cancel->setText(Constant::g_cancelText);
        m_progressBar->setState(kdk::NormalProgress);
        m_progressBar->show();
        m_progressBar->setValue(0);
        //获取定制配置
        kom::Configure conf;
        QString customize = conf.value(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_CUSTOMIZE).toString();
        if (customize == "icbc") {
            if (!m_interactionWidget2->isHidden()) {
                m_interactionWidget2->isStarMigration = true;
                m_interactionWidget2->hide();
            }
            clearAndShow();
        }
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
            m_labelMessage->SetText(Constant::g_migrationSuccessText);
        } else {
            m_icon->show();
            m_viewDetails->show();
            m_labelMessage->SetText(QString::number(err) + Constant::g_saveFailedText);
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
    m_userRejected = false;
}

void DialogMessage::saveProgressValue(int num)
{
    m_progressBar->setValue(num);
}

void DialogMessage::showTimeOutTip() 
{
    if(m_messageBoxTip == nullptr) {
        m_messageBoxTip = new QMessageBox(QMessageBox::Critical, "", tr("Migration timeout, please wait…"),QMessageBox::Ok, nullptr);//迁移超时，请稍后…
    }
    m_messageBoxTip->show();
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
    m_labelMessage->SetText(str);
}

void DialogMessage::onCancelClick()
{
    if (m_cancel->text() == Constant::g_cancelText) {
        *m_cancelHandle = true;
//        m_labelMessage->setText(Constant::g_cancellingText);
//        m_cancel->hide();
//        m_progressBar->hide();
        m_cancel->setText(Constant::g_okText);
        kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kWinDataMigration, kom::BuriedPoint::EventCode::kCancelMigration);
        emit signalCannelGetData();
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
    emit resave(true);
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
    // m_labelMessage = new QLabel(this);
    m_labelMessage = new K::TextLabel(this);
    hl1->addWidget(m_labelMessage);
    hl1->addStretch(9);
    vl->addLayout(hl1);

    m_progressBar = new kdk::KProgressBar(this);
    m_progressBar->setFixedHeight(16);
    connect(m_progressBar, &kdk::KProgressBar::valueChanged, this, [=]() {
        if (m_progressBar->value() == m_progressBar->maximum())
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

void DialogMessage::initInteractionBox()
{
    QWidget *parent = dynamic_cast<QWidget *>(this->parent());
    m_interactionBox1 = new QMessageBox(QMessageBox::Question, Constant::g_toolbarText,
                                        tr("Can we quickly build a migration queue and generate bat scripts and related configuration files on the win side? Do you agree? (Click No to migrate data by default)")
                                        , QMessageBox::Yes | QMessageBox::No, parent);
                                        // 可快速构建迁移队列，将在win端生成bat脚本和相关配置文件，是否同意？（点击取消，则以默认方式迁移数据）
    m_interactionBox2 = new QMessageBox(Constant::g_toolbarText, "", QMessageBox::Information, QMessageBox::Cancel,
                                        QMessageBox::NoButton, QMessageBox::NoButton, parent);
    connect(m_interactionBox2, &QMessageBox::rejected, this, &DialogMessage::onBox2rejected);

    m_interactionWidget2 = new DialogMessageBox();
    m_interactionWidget2->hide();
    connect(m_interactionWidget2, &DialogMessageBox::signalWidgetClose, this, &DialogMessage::onBox2rejected);
}

void DialogMessage::onBox2rejected()
{
    if (m_userRejected) {
        // onCancelClick();
        *m_cancelHandle = true;
        kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kWinDataMigration, kom::BuriedPoint::EventCode::kCancelMigration);
    }
}

void DialogMessage::showInteractionBox(int type, QString str)
{
    if (type == 1) {
        //this->hide();
        int res = m_interactionBox1->exec();
        if (res == QMessageBox::Yes) {
            emit selectiveSource(1);
        } else {
            emit selectiveSource(0);
        }
    } else if (type == 2) {
        m_userRejected = true;
        m_interactionWidget2->setText(str);
        QScreen *screen = QGuiApplication::primaryScreen();
        m_interactionWidget2->move(screen->geometry().center() - m_interactionWidget2->rect().center());
        m_interactionWidget2->show();
    }
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
    m_labelMessage->SetText(err);
    m_progressBar->hide();
    m_viewDetails->hide();
}

void DialogMessage::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    if (m_cancel->isHidden()) {
        event->ignore();
    } else if (m_cancel->text() == Constant::g_cancelText) {
        onCancelClick();
        event->ignore();
    }
    emit resave(false);
}

DialogMessageBox::DialogMessageBox(QWidget *parent) : kdk::KWidget(parent)
    , m_iconBtn(new QPushButton(this))
    , m_cannelBtn(new QPushButton(this))
    , m_browser(new QTextBrowser(this))
{
    this->setFixedSize(425, 260);
    this->windowButtonBar()->maximumButton()->hide();
    this->windowButtonBar()->menuButton()->hide();
    this->windowButtonBar()->minimumButton()->hide();
    this->setWindowFlags(Qt::Tool | Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowModality(Qt::WindowModal);
    this->setAttribute(Qt::WA_ShowModal, true);
    // this->setAttribute(Qt::WA_DeleteOnClose); 

    m_cannelBtn->setText(tr("Cannel"));

    m_browser->setOpenLinks(false);
    m_browser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_browser->setFrameShape(QFrame::NoFrame); // 去除边框
    m_browser->moveCursor(QTextCursor::Start); // 顶部显示
    m_browser->setReadOnly(true);              // 不可编辑
    m_browser->setFixedSize(350,150);

    m_iconBtn->setFixedSize(25, 25);
    m_iconBtn->setIconSize(QSize(25, 25));
    m_iconBtn->setIcon(QIcon::fromTheme("ukui-dialog-success"));
    m_iconBtn->setFlat(true);
    m_iconBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QHBoxLayout *hl = new QHBoxLayout;
    QVBoxLayout *vl = new QVBoxLayout;
    QVBoxLayout *vlIcon = new QVBoxLayout;

    vlIcon->addSpacing(5);
    vlIcon->addWidget(m_iconBtn, 0, Qt::AlignTop);
    vlIcon->addStretch();

    vl->addWidget(m_browser, 0, Qt::AlignRight | Qt::AlignTop);
    vl->addSpacing(20);
    vl->addWidget(m_cannelBtn, 0, Qt::AlignRight);
    vl->addStretch();
    
    hl->setMargin(0);
    hl->setSpacing(0);
    hl->addStretch();
    hl->addLayout(vlIcon);
    hl->addSpacing(5);
    hl->addLayout(vl);
    hl->addStretch();

    this->baseBar()->setLayout(hl);
    connect(m_cannelBtn, &QPushButton::clicked, this, &kdk::KWidget::close);
}

void DialogMessageBox::setText(QString strText)
{
    QString str = "<div>" + tr("The bat batch processing script has been generated. Please double-click to execute:") + strText + "</div>" +
                  "<div>" + tr("Note 1: The script only obtains the directory structure and file names to be migrated, and will not infringe on your privacy. Please use it with confidence") + "</div>" +
                  "<div>" + tr("Note 2: When the script execution is completed, the generated script and temporary files will be automatically destroyed, and the Kirin end will automatically start migration")+ "</div>";
                  // 已生成bat批处理脚本，请双击执行： 注1：脚本仅获取待迁移目录结构和文件名，不会侵犯您的隐私，请放心使用  注2：脚本执行完成时，会自动销毁本次生成的脚本和临时文件，同时麒麟端会自动开始迁移 
    m_browser->append(str);
    m_browser->moveCursor(QTextCursor::Start); // 顶部显示
}

void DialogMessageBox::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    event->ignore();
    this->hide();
    if (!isStarMigration) {
        emit signalWidgetClose();
    } else {
        isStarMigration = false;
    }
    
}
