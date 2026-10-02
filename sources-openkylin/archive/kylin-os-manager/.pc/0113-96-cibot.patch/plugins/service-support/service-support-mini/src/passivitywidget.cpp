#include "passivitywidget.h"
#include <glib.h>
#include <QIcon>
#include <QLabel>
#include <QBoxLayout>
#include <QDebug>
#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QTimer>
#include <QApplication>
#include <QTextEdit>
#include <QSizePolicy>
#include <kborderlessbutton.h>
#include <gsettingmonitor.h>
#include "kom-radiuswidget.h"
#include "kom-label.h"
#include "kom_buriedpoint.h"
#include "blacklist.h"

#define CALL_400 "400-089-1870"
#define MAIL_400 "support@kylinos.cn"

PassivityWidget::PassivityWidget(QWidget *parent)
    : QWidget(parent)
{
    exec_desktop_map_ = {
        {"kybackup", "yhkylin-backup-tools"},
        {"kylin-log-viewer", "logview"}
    };
}

void PassivityWidget::run() {
    if (m_isInit) {
        return;
    }
    m_isInit = true;

    if (m_mode == "2") {
        initUI();
        connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this,
                &PassivityWidget::onthemeChange);
        connect(FeedbackManager::getInstance(),&FeedbackManager::creatFinish,this,&PassivityWidget::creatFinish);
        onthemeChange();
    } else if (m_mode == "3") {
        connect(FeedbackManager::getInstance(),&FeedbackManager::creatFinish,this,&PassivityWidget::creatFinishNoUI);
        startCollecting(true);
    }
}

void PassivityWidget::onButtonClick() {
    startCollecting(m_logCheckbox->isChecked());
    m_submit->setEnabled(false);
    m_submit->setText(tr("Submitting"));
}

void PassivityWidget::onDialogButtonClick() {
    if (m_dialogSubmit->text() == tr("Ok")) {
        m_dialog->accept();
        m_submit->setEnabled(true);
        m_submit->setText(tr("Submit"));
    }
    if (m_dialogSubmit->text() == tr("Retry")) {
        m_dialog->accept();
        m_submit->setEnabled(false);
        m_submit->setText(tr("Submitting"));
        FeedbackManager::getInstance()->retryUpload();
    }
}

void PassivityWidget::creatFinish(FeedBackFinishType type, QString str) {
    switch (type) {
    case Success: {
        qDebug()<<"提交成功";
        showMessageBox(tr("Submitted successfully"),kdk::Normal);
        if (m_restartApp->isChecked()) {
            QProcess::startDetached(m_resart);
        }
        m_submit->setText(tr("Submitted"));
        break;
    }
    case Oversize: {
        qDebug()<<"压缩包总大小超过限制";
        m_dialogSubmit->setText(tr("Ok"));
        m_dialog->show();
        break;
    }
    case UploadFail: {
        qDebug()<<"提交失败";
        m_dialogSubmit->setText(tr("Retry"));
        m_dialog->show();
        break;
    }
    default:
        break;
    }
}

void PassivityWidget::creatFinishNoUI(FeedBackFinishType type, QString str)
{
    switch (type) {
        case Success: {
            QMap<QString, QString> custom_property = {
                {"applicationName", m_exe},
                {"errorCode",       m_code},
                {"submitTime",      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")},
                {"taskNumber",      str}
            };
            kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kServiceSupportMini,
                                            kom::BuriedPoint::EventCode::kFaultDiagnosisReport,
                                            custom_property);

            qDebug() << "提交成功";
            exit(0);
            break;
        }
        case Oversize:
            qDebug() << "压缩包总大小超过限制";
            exit(-2);
            break;
        case UploadFail:
            qDebug() << "提交失败";
            exit(-3);
            break;
        default:
            break;
    }
}

void PassivityWidget::exportLogsFinish(int code)
{
    QString text = tr("Export to desktop successfully:") + "\nkom-corefiles-"+ m_datetime +".7z";
    showMessageBox(text,kdk::Normal,m_dialog);
    m_exportLogsProc->deleteLater();
    m_exportLogsProc = nullptr;
}

void PassivityWidget::exportLogs(){
    if (m_exportLogsProc) {
        return;
    }
    m_exportLogsProc = new QProcess(this);
    connect(m_exportLogsProc,SIGNAL(finished(int)),this,SLOT(exportLogsFinish(int)));
    QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/kom-corefiles-" + m_datetime + ".7z";
    QString cmd = "/usr/lib/p7zip/7za";
    QStringList args;
    args.append("a");
    args.append("-y");
    args.append("-l");
    args.append("-bsp1");
    args.append("-m0=lzma2");
    args.append("-mx=9");
    args.append(fileName);
    args.append(m_path);
#ifdef DEBUG_MODE
    QString tmpCmd = cmd;
    for (const QString &str : args) {
        tmpCmd += " " + str;
    }
    qDebug() << tmpCmd;
#endif
    m_exportLogsProc->start(cmd, args);
}

void PassivityWidget::showMessageBox(const QString &msg, kdk::TipType type ,QWidget *parent)
{
    int num = -60;
    kdk::KBallonTip *ballonTip = new kdk::KBallonTip();
    if (parent == nullptr) {
        parent = this;
        num = 10;
    } else {
        ballonTip->setTipTime(5000);
    }
    ballonTip->setAttribute(Qt::WA_DeleteOnClose);
    ballonTip->setWindowFlag(Qt::FramelessWindowHint);
    ballonTip->setAttribute(Qt::WA_TranslucentBackground);
    ballonTip->setText(msg);
    QPoint globalPoint = QPoint(parent->geometry().x(), parent->geometry().y());
    int tmpWidth = QFontMetrics(ballonTip->font()).width(msg) + 46;
    int tmpHeight = 50;
    ballonTip->move(globalPoint.x() + (parent->width() - tmpWidth) / 2 - num, globalPoint.y() + (parent->height() - tmpHeight) / 2 - 20);
    ballonTip->setTipType(type);
    ballonTip->showInfo();
}

void PassivityWidget::onthemeChange()
{
    // 主题切换时，主题色尚未切换，获取到的是切换主题之前的颜色，此为规避方案
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=] {
        QPalette palet = qApp->palette();
        palet.setColor(QPalette::Window, qApp->palette().color(QPalette::Base));
        setPalette(palet);

        timer->deleteLater();
    });
    timer->start(10);
}

void PassivityWidget::initUI() {
    setFixedSize(452, 490);
    setWindowFlags(Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon::fromTheme("kylin-os-manager"));
    setWindowTitle(tr("Kylin Os Manager"));

    // 提示语
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setFixedSize(24, 24);
    iconLabel->setPixmap(QPixmap::fromImage(QImage(":/res/dialog-warning.png").scaled(24, 24, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation)));
    auto *text_edit = new QTextEdit(this);
    text_edit->setReadOnly(true);
    text_edit->setFrameShape(QFrame::Shape::NoFrame);
    text_edit->setText(tr("Sorry, %1 encountered a problem while running, "
                          "we deeply apologize and you may need to report the issue to Kylin operations personnel.")
                          .arg(ExecName(m_exe)));
    // 在驱动阶段调整 QTextEdit 的高度与文本高度一致，只有在界面完全显示结束进入事件循环后，才能获取到正确的 QTextDocument 的大小
    auto adjust_text_edit_heigth = [this, text_edit]() {
        QTimer::singleShot(0, this, [text_edit]() {
            auto height = text_edit->document()->size().toSize().height();
            if (height > 84) {
                height = 84;
            }
            text_edit->setFixedHeight(height);
        });
    };
    QTimer::singleShot(0, this, adjust_text_edit_heigth);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, adjust_text_edit_heigth);

    QHBoxLayout *hl1 = new QHBoxLayout;
    hl1->setContentsMargins(0, 0, 0, 0);
    hl1->setSpacing(0);
    hl1->addWidget(iconLabel);
    hl1->addSpacing(8);
    hl1->addWidget(text_edit);

    // 发送错误报告
    m_logCheckbox = new QCheckBox(this);
    m_logCheckbox->setChecked(true);
    auto *error_report_label = new kom::KomLabel(this);
    error_report_label->setText(tr("Send"));
    kdk::KBorderlessButton *lessSendBtn = new kdk::KBorderlessButton(tr(" an error report"), this);
    connect(lessSendBtn, &kdk::KBorderlessButton::clicked, this, [=] {
        QProcess::startDetached("/usr/bin/peony " + m_path);
    });
    auto *error_report_hbox_layout = new QHBoxLayout;
    error_report_hbox_layout->setContentsMargins(0, 0, 0, 0);
    error_report_hbox_layout->setSpacing(0);
    error_report_hbox_layout->addWidget(m_logCheckbox);
    error_report_hbox_layout->addWidget(error_report_label);
    error_report_hbox_layout->addWidget(lessSendBtn);
    error_report_hbox_layout->addStretch();

    // 重新启动程序
    m_restartApp = new QCheckBox(this);
    m_restartApp->setChecked(true);
    auto *restart_label = new kom::KomLabel(this);
    restart_label->setText(tr("Restart program"));
    auto *restart_hbox_layout = new QHBoxLayout;
    restart_hbox_layout->setContentsMargins(0, 0, 0, 0);
    restart_hbox_layout->setSpacing(0);
    restart_hbox_layout->addWidget(m_restartApp);
    restart_hbox_layout->addWidget(restart_label);
    restart_hbox_layout->addStretch();
    if (m_resart.isEmpty()) {
        qInfo() << "Restart command is empty.";
        m_restartApp->setChecked(false);
        m_restartApp->hide();
        restart_label->hide();
    }

    // 7 天内该程序崩溃不再弹出此窗口
    auto *prohibit_popup_check_box = new QCheckBox(this);
    prohibit_popup_check_box->setChecked(false);
    auto *prohibit_popup_label = new kom::KomLabel(this);
    prohibit_popup_label->setText(tr("The program crashes within 7 days and no longer pops up this window"));
    auto *prohibit_popup_hbox_layout = new QHBoxLayout;
    prohibit_popup_hbox_layout->setContentsMargins(0, 0, 0, 0);
    prohibit_popup_hbox_layout->setSpacing(0);
    prohibit_popup_hbox_layout->addWidget(prohibit_popup_check_box);
    prohibit_popup_hbox_layout->addWidget(prohibit_popup_label);
    prohibit_popup_hbox_layout->addStretch();
    connect(prohibit_popup_check_box, &QCheckBox::toggled, this, &PassivityWidget::on_UpdateBlacklist);

    // 卡片
    auto *cardBase = new kom::RadiusWidget(this);
    cardBase->setColorRole(QPalette::Window);
    cardBase->setRadius(6);

    // 联系我们
    kom::KomLabel *cardLabel1 = new kom::KomLabel(cardBase);
    cardLabel1->setBold(true);
    cardLabel1->setText(tr("Or contact us"));
    QHBoxLayout *hlCardLeft0 = new QHBoxLayout;
    hlCardLeft0->setMargin(0);
    hlCardLeft0->setSpacing(0);
    hlCardLeft0->addWidget(cardLabel1);
    hlCardLeft0->addStretch();

    // 电话
    auto *telephone_key_label = new kom::KomLabel(cardBase);
    telephone_key_label->setText(tr("Telephone"));
    auto *telephone_value_label = new kom::KomLabel(cardBase);
    telephone_value_label->setText(CALL_400);
    QHBoxLayout *hlCardLeft1 = new QHBoxLayout;
    hlCardLeft1->setMargin(0);
    hlCardLeft1->setSpacing(0);
    hlCardLeft1->addWidget(telephone_key_label);
    hlCardLeft1->addSpacing(16);
    hlCardLeft1->addWidget(telephone_value_label);
    hlCardLeft1->addStretch();

    // 邮件
    auto *mail_key_label = new kom::KomLabel(cardBase);
    mail_key_label->setText(tr("Mail"));
    auto *mail_value_label = new kom::KomLabel(cardBase);
    mail_value_label->setText(MAIL_400);
    QHBoxLayout *hlCardLeft2 = new QHBoxLayout;
    hlCardLeft2->setMargin(0);
    hlCardLeft2->setSpacing(0);
    hlCardLeft2->addWidget(mail_key_label);
    hlCardLeft2->addSpacing(16);
    hlCardLeft2->addWidget(mail_value_label);
    hlCardLeft2->addStretch();

    // 常见问题
    auto *faq_label = new kom::KomLabel(cardBase);
    faq_label->setText(tr("FAQ found here"));
    auto *lessQuestionBtn = new kdk::KBorderlessButton(tr("KylinOS website"), cardBase);
    connect(lessQuestionBtn, &kdk::KBorderlessButton::clicked, this, [=] {
        QDesktopServices::openUrl(QUrl("https://wx.kylinos.cn/zsk"));
    });
    QHBoxLayout *hlCardLeft4 = new QHBoxLayout;
    hlCardLeft4->setMargin(0);
    hlCardLeft4->setSpacing(0);
    hlCardLeft4->addWidget(faq_label);
    hlCardLeft4->addSpacing(4);
    hlCardLeft4->addWidget(lessQuestionBtn);
    hlCardLeft4->addStretch();

    QVBoxLayout *vlCardLeft = new QVBoxLayout;
    vlCardLeft->setMargin(0);
    vlCardLeft->setSpacing(0);
    vlCardLeft->addStretch();
    vlCardLeft->addLayout(hlCardLeft0);
    vlCardLeft->addSpacing(8);
    vlCardLeft->addLayout(hlCardLeft1);
    vlCardLeft->addSpacing(8);
    vlCardLeft->addLayout(hlCardLeft2);
    vlCardLeft->addSpacing(8);
    vlCardLeft->addLayout(hlCardLeft4);
    vlCardLeft->addStretch();

    // 二维码
    auto *rightImage = new QLabel(cardBase);
    rightImage->setFixedSize(110, 110);
    rightImage->setPixmap(QPixmap(":/res/qcpic.png").scaled(110, 110, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    QVBoxLayout *vlCardRight = new QVBoxLayout;
    vlCardRight->addStretch();
    vlCardRight->addWidget(rightImage);
    vlCardRight->addStretch();

    QHBoxLayout *hlCard = new QHBoxLayout(cardBase);
    hlCard->setContentsMargins(16, 16, 16, 16);
    hlCard->setSpacing(0);
    hlCard->addLayout(vlCardLeft);
    hlCard->addSpacing(8);
    hlCard->addLayout(vlCardRight);

    // 按钮
    auto *cancelBtn = new QPushButton(this);
    cancelBtn->setText(tr("Cancel"));
    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        FeedbackManager::getInstance()->cancel();
        exit(0);
    });
    m_submit = new QPushButton(this);
    m_submit->setText(tr("Submit"));
    m_submit->setProperty("isImportant", true);
    connect(m_submit, &QPushButton::clicked, this, &PassivityWidget::onButtonClick);
    QHBoxLayout *hlSubmit2 = new QHBoxLayout;
    hlSubmit2->setMargin(0);
    hlSubmit2->setSpacing(0);
    hlSubmit2->addStretch();
    hlSubmit2->addWidget(cancelBtn);
    hlSubmit2->addSpacing(8);
    hlSubmit2->addWidget(m_submit);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(24, 16, 24, 24);
    vl->setSpacing(0);
    vl->addLayout(hl1);
    vl->addSpacing(12);
    vl->addLayout(error_report_hbox_layout);
    vl->addLayout(restart_hbox_layout);
    vl->addLayout(prohibit_popup_hbox_layout);
    vl->addSpacing(16);
    vl->addWidget(cardBase);
    vl->addStretch(1);
    vl->addSpacing(4);
    vl->addLayout(hlSubmit2);

    initDialog();
}

void PassivityWidget::initDialog() {
    m_dialog = new kdk::KDialog(this);
    m_dialog->setFixedSize(424,230);
    m_dialog->setModal(true);

    QHBoxLayout *hl0 = new QHBoxLayout(m_dialog->mainWidget());
    hl0->setContentsMargins(24, 0, 24, 24);
    QVBoxLayout *vl0 = new QVBoxLayout();
    QLabel *iconLabel = new QLabel(m_dialog->mainWidget());
    iconLabel->setPixmap(QPixmap::fromImage(QImage(":/res/fail.png").scaled(24,24)));
    vl0->addWidget(iconLabel);
    vl0->addStretch();
    hl0->addLayout(vl0);

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setSpacing(0);
    QLabel *titleLabel = new QLabel(m_dialog->mainWidget());
    titleLabel->setText(tr("Submission failed"));
    QFont font;
    font.setBold(true);
    titleLabel->QLabel::setFont(font);
    vl->addWidget(titleLabel);

    QString call400 = CALL_400;
    QString mail = MAIL_400;
    QLabel *textLabel = new QLabel(this);
    textLabel->setWordWrap(true);
    QString text = tr("Please check the network or contact the O&M personnel.\nPhone: %1\nMail: %2").arg(call400).arg(mail);
    textLabel->setText(text);
    vl->addSpacing(8);
    vl->addWidget(textLabel);

    kdk::KBorderlessButton *lessExportBtn = new kdk::KBorderlessButton(tr("Export error report"),m_dialog->mainWidget());
    connect(lessExportBtn, &kdk::KBorderlessButton::clicked, this, [=] {
        exportLogs();
    });
    vl->addSpacing(8);
    vl->addWidget(lessExportBtn);

    connect(m_dialog->closeButton(), &QPushButton::clicked, this, [this]() {
        m_submit->setEnabled(true);
        m_submit->setText(tr("Submit"));

    });
    QPushButton *cancelBtn = new QPushButton(m_dialog->mainWidget());
    cancelBtn->setText(tr("Cancel"));
    connect(cancelBtn,&QPushButton::clicked,this,[this]{
        FeedbackManager::getInstance()->cancel();
        m_dialog->accept();
        m_submit->setEnabled(true);
        m_submit->setText(tr("Submit"));
    });
    m_dialogSubmit = new QPushButton(this);
    m_dialogSubmit->setProperty("isImportant", true);
    connect(m_dialogSubmit, &QPushButton::clicked, this, &PassivityWidget::onDialogButtonClick);
    QHBoxLayout *hlSubmit2 = new QHBoxLayout;
    hlSubmit2->setMargin(0);
    hlSubmit2->setSpacing(8);
    hlSubmit2->addStretch();
    hlSubmit2->addWidget(cancelBtn);
    hlSubmit2->addWidget(m_dialogSubmit);
    vl->addStretch();
    vl->addLayout(hlSubmit2);
    hl0->addLayout(vl);
}

void PassivityWidget::on_UpdateBlacklist(bool ok) {
    if (ok) {
        Blacklist::Add(m_exe);
    } else {
        Blacklist::Remove(m_exe);
    }
}

QString PassivityWidget::ExecName(const QString &exec) {
    QString desktop;
    const auto iter = exec_desktop_map_.find(exec);
    if (iter != exec_desktop_map_.end()) {
        desktop = QString("/usr/share/applications/%1.desktop").arg(iter.value());
    } else {
        desktop = QString("/usr/share/applications/%1.desktop").arg(exec);
    }

    g_autoptr(GKeyFile) key_file = g_key_file_new();
    g_autoptr(GError) error = NULL;
    if (!g_key_file_load_from_file(key_file, desktop.toStdString().c_str(), G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
        qCritical() << "Get exec name fail: load key file fail, " << error->message;
        return exec;
    }

    g_autofree gchar *name = g_key_file_get_locale_string(key_file, "Desktop Entry", "Name", NULL, &error);
    if (name != NULL) {
        return QString(name);
    }

    return exec;
}
