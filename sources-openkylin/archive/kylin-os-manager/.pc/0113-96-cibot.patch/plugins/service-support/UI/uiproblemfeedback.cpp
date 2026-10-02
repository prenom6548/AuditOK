#include "uiproblemfeedback.h"
#include <pwd.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <unistd.h>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QIcon>
#include <QTimer>
#include <QCollator>
#include <QInputDialog>
#include <QDBusConnection>
#include <kpressbutton.h>
#include <gsettingmonitor.h>
#include "feedbackmanager.h"
#include "kom-configure.h"
#include "kom_buriedpoint.h"
#include "src/settings.h"
#include "uiproblemfeedbackdialog.h"
#include <libkydate.h>

#define UPLOAD_SUPPORT_SUFFIX "*.jpg *.png *.zip *.rar *.7z *.mp4"
#define UPLOAD_SUPPORT_SUFFIX_PLUS " *.mkv *.flv"
#define SCREEN_SHOT_GSETTING_PATH "org.ukui.screenshot"
#define SCREEN_SHOT_TMP_PATH "/tmp/kylin-os-manager/service-support/"
#define FORMLAYOUT_ROW_CALSS 0
#define FORMLAYOUT_ROW_CALSS_SECOND 1
#define FORMLAYOUT_ROW_DATETIME 2
#define FORMLAYOUT_ROW_DETAILS 3
#define FORMLAYOUT_ROW_INTERNAL_PROJECT_INFO 4
#define FORMLAYOUT_ROW_UPLOAD_FILE 5
#define FORMLAYOUT_ROW_CONTACT 6
#define FORMLAYOUT_ROW_MAIL 7
#define FORMLAYOUT_ROW_EXPORT_PATH 8
#define FORMLAYOUT_ROW_SYSTEM_INFO 9
#define FORMLAYOUT_ROW_SYSTEM_INFO_SECOND 10
#define FORMLAYOUT_ROW_UPLOAD_LOGS 11
#define FORMLAYOUT_ROW_SUBMIT 12

#define INTERNAL_PLACEHOLDER_TEXT                                                                                      \
    "首行请以【模块名+"                                                                                        \
    "问题现象】格式简要描述遇到的问题，如麒麟管家闪退\n换行后做补充，如问题的复现条件、复现步骤、复现概率等"

UiProblemFeedback::UiProblemFeedback(QWidget *parent) : QWidget(parent)
{
    setObjectName(m_objectName);
    m_customize = Settings::getCustomize();
    if (m_customize == Settings::INTERNAL) {
        m_internalMode = ProjectMode;
    } else if (m_customize == Settings::ICBC) {
        m_internalMode = ProjectMode;
        m_addFileMax = 8;
    }
    initUI();
    initConnect();
    initFinish();
}

UiProblemFeedback::~UiProblemFeedback()
{
    m_tmpDetailsEdit->deleteLater();
    if (!m_systemInfoGroup.isEmpty()) {
        m_systemInfoGroup.first()->deleteLater();
    }
}

void UiProblemFeedback::indexChanged(QString name)
{

    if (name != m_objectName) {
        return;
    }
    onDateFormatChange("");
    if (m_loaded) {
        return;
    }
    kom::Configure cof;
    m_internalUser = cof.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_INTERNAL_USER).toString();
    if (m_internalUser.isEmpty()) {
        return;
    }
    m_loaded = true;
    m_internalPasswd = Settings::getZenTaoPwd();
    bool ok = false;
    m_internalPasswd = QInputDialog::getText(this, "内部模式", "请输入密码", QLineEdit::Password, m_internalPasswd, &ok,
                                             Qt::FramelessWindowHint);
    if (!ok) {
        return;
    }
    showMessageBox("验证中，请稍等", kdk::Info);
    FeedbackManager::getInstance()->getProjectInfo(m_internalUser, m_internalPasswd);
}

void UiProblemFeedback::initUI()
{
    setFixedWidth(800);
    m_formLayoutVerticalSpacing = 24;
    initDialogs();
    initClassUI();
    initDetailsUI();
    initContactUI();
    initSubmitUI();
    initInternalUI();
    initExpertUI();
    resetState();
    onFontSizeChange();
    onthemeChange();
}

void UiProblemFeedback::initDialogs()
{
    m_dialog = new UiProblemFeedbackDialog(this);
}

void UiProblemFeedback::initClassUI()
{
    QWidget *wid = new QWidget(this);
    //    wid->setMinimumWidth(625);
    QHBoxLayout *hlgroup = new QHBoxLayout;
    hlgroup->setMargin(0);
    hlgroup->setSpacing(0);
    for (ChooseClassItem &item : FeedbackManager::getInstance()->getChooseClassList()) {
        QRadioButton *radio = new QRadioButton(item.one, wid);
        if (m_customize == Settings::ICBC) {
            radio->hide();
        }
        m_classTypeGroup.append(radio);
        hlgroup->addWidget(radio);
        hlgroup->addSpacing(24);
        connect(radio, &QRadioButton::pressed, this, [=] {
            m_classType = item.code;
            m_secondClass->clear();
            m_secondClass->addItems(item.two);
            m_detailsEdit->setPlaceholderText(item.guide);
            if (m_customize == Settings::INTERNAL) {
                m_detailsEdit->setPlaceholderText(INTERNAL_PLACEHOLDER_TEXT);
            }
            changeSubmitState();
        });
    }
    m_checkAdvanced = new QCheckBox(tr("Advanced") + " ", wid);
//    if (m_customize == Settings::ICBC) {
//        m_checkAdvanced->setChecked(true);
//    }
    connect(m_checkAdvanced, &QCheckBox::clicked, this, &UiProblemFeedback::onExpertchange);
    hlgroup->addStretch();
    hlgroup->addWidget(m_checkAdvanced);
    setVerticalSpacingLayout(wid, hlgroup, 14);
    if (m_customize == Settings::ICBC) {
        saveFormLayoutPtr(FORMLAYOUT_ROW_CALSS, "", wid);
    } else {
        saveFormLayoutPtr(FORMLAYOUT_ROW_CALSS, creatFormTitle(tr("Type")), wid);
    }
}

void UiProblemFeedback::initDetailsUI() {
    // 问题描述
    QWidget *wid = new QWidget(this);
    wid->setFixedHeight(150 + m_formLayoutVerticalSpacing);
    kdk::KPressButton *cardBase = new kdk::KPressButton(wid);
    cardBase->setCheckable(false);
    cardBase->setBorderRadius(6);
    cardBase->setFixedHeight(150);

    m_detailsEdit = new QTextEdit(this);
    m_detailsEdit->setAcceptRichText(false);
    m_detailsEdit->setFrameShape(QTextEdit::NoFrame);
    m_detailsEditNumber = new QLabel(this);
    m_detailsEditNumber->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    if (m_customize == Settings::ICBC) {
        m_dateEditText = tr("Please provide a detailed description of the phenomenon you have encountered. "
                            "You can also click the button below to upload images or files for us to quickly understand the usage situation.");
    } else {
        m_dateEditText =
                tr("Please describe the problem in detail and you can upload a photo or file by clicking the button below.");
    }
    m_detailsEditNumberText = tr("Up to 500 characters");
    QString detailsEditNumberTextFirst = tr("Remaining");
    QString detailsEditNumberTextLast = tr("character");
    connect(m_detailsEdit, &QTextEdit::textChanged, this, [=] {
        changeSubmitState();
        int maxTextNum = 500;
        QString text = m_detailsEdit->toPlainText();
        m_detailsEditNumber->setText(detailsEditNumberTextFirst + " " + QString::number(maxTextNum - text.length())
                                     + " " + detailsEditNumberTextLast);
        QPalette palet = m_detailsEditNumber->palette();
        if (text.length() >= maxTextNum) {
            palet.setColor(QPalette::Text, QColor(Qt::red));
            m_detailsEditNumber->setPalette(palet);
        } else {
            palet.setColor(QPalette::Text, QColor(Qt::darkGray));
            m_detailsEditNumber->setPalette(palet);
        }
        if (text.length() > maxTextNum) {
            if (m_tmpDetailsEditText.length() < maxTextNum) {
                m_tmpDetailsEditText = text.mid(0, maxTextNum);
            }
            m_detailsEdit->setText(m_tmpDetailsEditText);
            QTextCursor cursor = m_detailsEdit->textCursor();
            cursor.setPosition(maxTextNum);
            m_detailsEdit->setTextCursor(cursor);
            return;
        }
        m_tmpDetailsEditText = text;
    });
    m_tmpDetailsEdit = new QTextEdit;
    QVBoxLayout *vlEdit = new QVBoxLayout(cardBase);
    vlEdit->setContentsMargins(6, 6, 2, 32);
    vlEdit->addWidget(m_detailsEdit);
    QVBoxLayout *hlDetailsEdit = new QVBoxLayout;
    hlDetailsEdit->setMargin(0);
    hlDetailsEdit->setSpacing(0);
    hlDetailsEdit->addWidget(cardBase);
    m_detailsEditNumber->setContentsMargins(0, 0, 8, 4);
    hlDetailsEdit->addWidget(m_detailsEditNumber);
    setVerticalSpacingLayout(wid, hlDetailsEdit);
    //    saveFormLayoutPtr(FORMLAYOUT_ROW_DETAILS, creatFormTitle(tr("Details")), wid);
    QLabel *label = new QLabel(this);
    if (m_customize == Settings::ICBC) {
        label->setText(creatFormTitle(tr("Situation description")));
    } else {
        label->setText(creatFormTitle(tr("Details")));
    }
    label->setAlignment(Qt::AlignTop);
    m_formLayoutPtrMap.insert(FORMLAYOUT_ROW_DETAILS, FormLayoutRow(label, wid));

    // 上传附件
    QWidget *wid2 = new QWidget(this);
    QHBoxLayout *hlDetails = new QHBoxLayout;
    hlDetails->setMargin(0);
    hlDetails->setSpacing(0);
    hlDetails->setAlignment(Qt::AlignCenter);
    m_screenCut = new QPushButton(wid2);
    m_screenCut->setFixedHeight(36);
    m_screenCut->setText(tr("ScreenShot"));
    connect(m_screenCut, &QPushButton::clicked, this, &UiProblemFeedback::onScreenCutClicked);
    // 麒麟截图
    if (QGSettings::isSchemaInstalled(SCREEN_SHOT_GSETTING_PATH)) {
        m_screenShotSetting = new QGSettings(SCREEN_SHOT_GSETTING_PATH);
        connect(m_screenShotSetting, &QGSettings::changed, this, [=]() {
            if (m_startShot && (m_screenShotSetting->get("isrunning").toString() == "false")) {
                m_startShot = false;
                getShotImage();
            }
        });
    }
    hlDetails->addWidget(m_screenCut);
    hlDetails->addSpacing(10);
    m_addFile = new QPushButton(wid2);
    m_addFile->setFixedHeight(36);
    m_addFile->setText(tr("Add file"));
    connect(m_addFile, &QPushButton::clicked, this, &UiProblemFeedback::onAddFileClicked);
    hlDetails->addWidget(m_addFile);
    hlDetails->addSpacing(24);
    m_uploadText1 = new kom::KomLabel(wid2);
    m_uploadText2 = new kom::KomLabel(wid2);
    m_uploadText1->setFontSize(12);
    m_uploadText2->setFontSize(12);
    m_uploadText1->setAlignment(Qt::AlignTop);
    m_uploadText2->setAlignment(Qt::AlignTop);
    //    m_uploadText1->setAutoFillBackground(true);
    //    m_uploadText2->setAutoFillBackground(true);
    QPalette palUploadText = m_uploadText1->palette();
    palUploadText.setColor(QPalette::Text, QColor(Qt::lightGray));
    palUploadText.setColor(QPalette::Base, QColor(Qt::red));
    m_uploadText1->setPalette(palUploadText);
    m_uploadText2->setPalette(palUploadText);
    m_uploadText1->setMaximumHeight(31);
    m_uploadText2->setMaximumHeight(31);
    m_uploadText = new QWidget(this);
    QVBoxLayout *uploadTextVl = new QVBoxLayout(m_uploadText);
    uploadTextVl->setMargin(0);
    uploadTextVl->setSpacing(0);
    uploadTextVl->addWidget(m_uploadText1);
    uploadTextVl->addWidget(m_uploadText2);
    hlDetails->addWidget(m_uploadText, 9);
    QVBoxLayout *vlDetails = new QVBoxLayout;
    vlDetails->setMargin(0);
    vlDetails->setSpacing(0);
    vlDetails->addLayout(hlDetails);
    m_fileListWidget = new QListWidget(wid2);
    m_fileListWidget->setFixedHeight(74);
    m_fileListWidget->setFlow(QListWidget::LeftToRight);
    m_fileListWidget->setFrameShape(QListWidget::NoFrame);
    m_fileListWidget->hide();
    vlDetails->addWidget(m_fileListWidget);
    setVerticalSpacingLayout(wid2, vlDetails);
    saveFormLayoutPtr(FORMLAYOUT_ROW_UPLOAD_FILE, creatFormTitle(tr("Files"), false), wid2);
}

void UiProblemFeedback::initContactUI()
{
    QWidget *wid = new QWidget(this);
    QHBoxLayout *hlPhoneNumber = new QHBoxLayout;
    hlPhoneNumber->setMargin(0);
    hlPhoneNumber->setSpacing(0);
    QFont ft;
    ft.setPointSize(10);
    QLabel *errMsg = new QLabel(wid);
    errMsg->setFixedHeight(m_formLayoutVerticalSpacing);
    errMsg->setFont(ft);
    QPalette palet = errMsg->palette();
    palet.setColor(QPalette::Text, QColor(Qt::red));
    errMsg->setPalette(palet);
    m_phoneNumberEdit = new QLineEdit(this);
    connect(m_phoneNumberEdit, &QLineEdit::editingFinished, this, [=] {
        changeSubmitState();
        if (m_phoneNumberEdit->text().isEmpty()) {
            errMsg->setText(tr("The phone number cannot be empty"));
        } else if (!checkString(m_phoneNumberEdit->text(), PhoneNumber)) {
            errMsg->setText(tr("The phone number format is incorrect"));
            return;
        }
        errMsg->setText("");
    });
    m_phoneNumberEdit->setPlaceholderText(tr("Please enter your phone number"));
    hlPhoneNumber->addWidget(m_phoneNumberEdit);
    m_nameEdit = new QLineEdit(this);
    connect(m_nameEdit, &QLineEdit::textEdited, this, [=](QString) {
        changeSubmitState();
    });
    m_nameEdit->setPlaceholderText(tr("appellation"));
    hlPhoneNumber->addSpacing(10);
    hlPhoneNumber->addWidget(m_nameEdit);
    QVBoxLayout *vlPhoneNumber = new QVBoxLayout(wid);
    vlPhoneNumber->setMargin(0);
    vlPhoneNumber->setSpacing(0);
    vlPhoneNumber->addLayout(hlPhoneNumber);
    vlPhoneNumber->addWidget(errMsg);
    saveFormLayoutPtr(FORMLAYOUT_ROW_CONTACT, creatFormTitle(tr("Contact")), wid);

    QWidget *wid2 = new QWidget(this);
    m_mailEdit = new QLineEdit(this);
    //    m_mailEdit->setPlaceholderText(tr("预留邮箱可及时收到服务进展"));
    if (m_customize == Settings::OPENKYLIN) {
        m_mailEdit->setPlaceholderText(tr("Email required for anonymous feedback, not required for gitee feedback"));
    }
    m_mailErrorText = new QLabel(wid);
    m_mailErrorText->setFixedHeight(m_formLayoutVerticalSpacing);
    m_mailErrorText->setFont(ft);
    m_mailErrorText->setPalette(palet);
    connect(m_mailEdit, &QLineEdit::textEdited, this, [=] {
        QString tmp = "";
        if (!m_mailEdit->text().isEmpty() && !checkString(m_mailEdit->text(), Mail)) {
            tmp = tr("The mailbox format is incorrect");
        }
        m_mailErrorText->setText(tmp);
        changeSubmitState();
    });

    QVBoxLayout *vlMailEdit = new QVBoxLayout(wid2);
    vlMailEdit->setMargin(0);
    vlMailEdit->setSpacing(0);
    if (m_customize == Settings::OPENKYLIN) {
        m_connectGiteeBtn = new QPushButton(this);
        m_connectGiteeBtn->setText(tr("Log in to gitee"));
        auto *hlMail = new QHBoxLayout;
        hlMail->setMargin(0);
        hlMail->setSpacing(0);
        hlMail->addWidget(m_mailEdit);
        hlMail->addSpacing(10);
        hlMail->addWidget(m_connectGiteeBtn);
        connect(m_connectGiteeBtn, &QPushButton::clicked, this, &UiProblemFeedback::getUserCode);
        vlMailEdit->addLayout(hlMail);
    } else {
        vlMailEdit->addWidget(m_mailEdit);
    }
    vlMailEdit->addWidget(m_mailErrorText);
    saveFormLayoutPtr(FORMLAYOUT_ROW_MAIL, creatFormTitle(tr("Mailbox"), false), wid2);
}

void UiProblemFeedback::initSubmitUI()
{
    QWidget *wid = new QWidget(this);
    QHBoxLayout *hlSubmit = new QHBoxLayout;
    hlSubmit->setMargin(0);
    hlSubmit->setSpacing(0);
    m_agreeAll = new QCheckBox(tr("Agree to take mine "), this);
    connect(m_agreeAll, &QCheckBox::clicked, this, [=](bool checked) {
        for (QCheckBox *item : m_systemInfoGroup) {
            item->setChecked(checked);
        }
        for (InformationClassItem *item : FeedbackManager::getInstance()->getLogFileItems()) {
            item->setSelect(checked);
        }
    });
    hlSubmit->addWidget(m_agreeAll);
    m_systemInfoLabel = new QLabel(tr("System information"), this);
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Text, pal.color(QPalette::Highlight));
    m_systemInfoLabel->setPalette(pal);
    QString toolTipText;
    for (int i = 0; i < FeedbackManager::getInstance()->getLogFileItems().length(); i++) {
        InformationClassItem *item = FeedbackManager::getInstance()->getLogFileItems().at(i);
        toolTipText += item->getItemNameShow();
        if (i + 1 != FeedbackManager::getInstance()->getLogFileItems().length()) {
            toolTipText += "、";
        }
        if ((i + 1) % 4 == 0 && i + 1 != FeedbackManager::getInstance()->getLogFileItems().length()) {
            toolTipText += "\n";
        }
    }
    m_systemInfoLabel->setToolTip(toolTipText);
    hlSubmit->addWidget(m_systemInfoLabel);
    hlSubmit->addStretch(9);
    setVerticalSpacingLayout(wid, hlSubmit, 13);
    saveFormLayoutPtr(FORMLAYOUT_ROW_SYSTEM_INFO, "", wid);

    // 提交按钮
    QWidget *wid2 = new QWidget(this);

    m_generateLogs = new QPushButton(this);
    m_generateLogs->setText(tr("Generate logs only"));
    connect(m_generateLogs, &QPushButton::clicked, this, [this]() {
        onSubmitClicked(true);
    });

    m_submit = new QPushButton(this);
    m_submit->setText(tr("Submit application"));
    m_submit->setProperty("isImportant", true);
    connect(m_submit, &QPushButton::clicked, this, [this]() {
        onSubmitClicked(false);
    });

    QHBoxLayout *hlSubmit2 = new QHBoxLayout;
    hlSubmit2->setMargin(0);
    hlSubmit2->setSpacing(0);
    hlSubmit2->addStretch();
    hlSubmit2->addWidget(m_generateLogs);
    hlSubmit2->addSpacing(16);
    hlSubmit2->addWidget(m_submit);
    wid2->setLayout(hlSubmit2);
    saveFormLayoutPtr(FORMLAYOUT_ROW_SUBMIT, "", wid2);

    if (m_customize != Settings::ICBC) {
        m_generateLogs->hide();
    }
}

void UiProblemFeedback::initExpertUI()
{
    // 详细分类
    QWidget *wid = new QWidget(this);
    m_secondClass = new QComboBox(this);
    QHBoxLayout *hlSecondClass = new QHBoxLayout;
    hlSecondClass->setMargin(0);
    hlSecondClass->setSpacing(0);
    hlSecondClass->addWidget(m_secondClass);
    setVerticalSpacingLayout(wid, hlSecondClass);
    saveFormLayoutPtr(FORMLAYOUT_ROW_CALSS_SECOND, creatFormTitle(tr("Details type"), false), wid);

    // 故障时间段
    QWidget *wid0 = new QWidget(this);
    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setDisplayFormat(QString(kdk_system_get_shortformat()));
    qDebug() << "绑定日期短格式dbus"
             << QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer", "/com/kylin/kysdk/Date",
                                                      "com.kylin.kysdk.DateInterface", "ShortDateSignal", this,
                                                      SLOT(onDateFormatChange(QString)));
    m_dateEditStart = new QTimeEdit(this);
    m_dateEditEnd = new QTimeEdit(this);
    m_dateEditStart->setDisplayFormat("HH:mm");
    m_dateEditEnd->setDisplayFormat("HH:mm");
    QHBoxLayout *hlDateTime = new QHBoxLayout;
    hlDateTime->setMargin(0);
    hlDateTime->setSpacing(4);
    hlDateTime->addWidget(m_dateEdit);
    hlDateTime->addSpacing(6);
    hlDateTime->addWidget(m_dateEditStart);
    hlDateTime->addWidget(new QLabel(" - ", this));
    hlDateTime->addWidget(m_dateEditEnd);
    hlDateTime->addStretch(9);
    setVerticalSpacingLayout(wid0, hlDateTime);
    saveFormLayoutPtr(FORMLAYOUT_ROW_DATETIME, creatFormTitle(tr("Time period"), false), wid0);

    // 系统信息
    QWidget *wid2 = new QWidget(this);
    QGridLayout *blSystemInfo = new QGridLayout;
    blSystemInfo->setMargin(0);
    blSystemInfo->setHorizontalSpacing(0);
    blSystemInfo->setVerticalSpacing(0);
    for (int i = 0; i < FeedbackManager::getInstance()->getLogFileItems().length(); i++) {
        if (i == 0) {
            continue;
        }
        InformationClassItem *item = FeedbackManager::getInstance()->getLogFileItems().at(i);
        QCheckBox *systemInfo = new QCheckBox(this);
        m_systemInfoGroup.append(systemInfo);
        systemInfo->setChecked(true);
        systemInfo->setText(item->getItemNameShow());
        int rowNum = 5;
        int j = 0;
        if (i > rowNum) {
            j++;
        }
        blSystemInfo->addWidget(systemInfo, j, i - 1 - j * rowNum);
        connect(systemInfo, &QCheckBox::clicked, this, [=](bool checked) {
            m_agreeAll->setChecked(checked);
            item->setSelect(checked);
            if (!checked) {
                for (QCheckBox *box : m_systemInfoGroup) {
                    if (box->isChecked()) {
                        m_agreeAll->setChecked(true);
                    }
                }
            }
        });
    }
    setVerticalSpacingLayout(wid2, blSystemInfo);
    saveFormLayoutPtr(FORMLAYOUT_ROW_SYSTEM_INFO_SECOND, creatFormTitle(tr("Information"), false), wid2);

    // 添加日志
    QWidget *wid3 = new QWidget(this);
    QLabel *daysHead = new QLabel(tr("lately"), this);
    QLabel *daysEnd = new QLabel(tr("days"), this);
    m_logDays = new QComboBox(this);
    QStringList daysNum = {"1", "2", "3", "4", "5", "6", "7"};
    m_logDays->addItems(daysNum);
    m_logDays->setFixedWidth(96);
    connect(m_logDays, &QComboBox::currentTextChanged, this, [=](const QString &itemStr) {
        if (!FeedbackManager::getInstance()->getLogFileItems().isEmpty()) {
            FeedbackManager::getInstance()->getLogFileItems().at(0)->setDetailed(itemStr.toInt());
        }
    });
    QCheckBox *log = new QCheckBox;
    m_systemInfoGroup.insert(0, log);
    m_logYes = new QRadioButton(tr("YES"), this);
    connect(m_logYes, &QRadioButton::clicked, this, [=] {
        log->setChecked(true);
        daysHead->show();
        daysEnd->show();
        m_logDays->show();
        if (!FeedbackManager::getInstance()->getLogFileItems().isEmpty()) {
            FeedbackManager::getInstance()->getLogFileItems().first()->setSelect(true);
        }
    });
    QRadioButton *logNo = new QRadioButton(tr("NO"), this);
    connect(logNo, &QRadioButton::clicked, this, [=] {
        log->setChecked(false);
        daysHead->hide();
        daysEnd->hide();
        m_logDays->hide();
        if (!FeedbackManager::getInstance()->getLogFileItems().isEmpty()) {
            FeedbackManager::getInstance()->getLogFileItems().first()->setSelect(false);
        }
    });
    QHBoxLayout *hlLogs = new QHBoxLayout;
    hlLogs->setSpacing(8);
    hlLogs->setMargin(0);
    hlLogs->addWidget(m_logYes);
    hlLogs->addSpacing(16);
    hlLogs->addWidget(logNo);
    hlLogs->addSpacing(29);
    hlLogs->addWidget(daysHead);
    hlLogs->addWidget(m_logDays);
    hlLogs->addWidget(daysEnd);
    hlLogs->addStretch(9);
    setVerticalSpacingLayout(wid3, hlLogs);
    saveFormLayoutPtr(FORMLAYOUT_ROW_UPLOAD_LOGS, creatFormTitle(tr("Upload log"), false), wid3);

    // 导出至
    QWidget *wid4 = new QWidget(this);
    m_exportEdit = new QLineEdit(wid4);
    m_exportEdit->setEnabled(false);
    QPushButton *exportBtn = new QPushButton(wid4);
    exportBtn->setText(tr("Path"));
    connect(exportBtn, &QPushButton::clicked, this, [=] {
        QString path = QFileDialog::getExistingDirectory(
            nullptr, "", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (path.isEmpty()) {
            return;
        }
        if (!QFileInfo(path).isWritable()) {
            errorMessage(tr("所选目录不可用"));
            return;
        }
        m_exportEdit->setText(path);
    });
    QHBoxLayout *hlExport = new QHBoxLayout;
    hlExport->setMargin(0);
    hlExport->addWidget(m_exportEdit);
    hlExport->setSpacing(20);
    hlExport->addWidget(exportBtn);
    setVerticalSpacingLayout(wid4, hlExport);
    saveFormLayoutPtr(FORMLAYOUT_ROW_EXPORT_PATH, creatFormTitle(tr("Export to"), false), wid4);
}

void UiProblemFeedback::initInternalUI()
{
    QWidget *wid = new QWidget(this);
    m_internalTitle = new QLineEdit(this);
    m_internalTitle->setPlaceholderText("【必填】BUG标题");
    connect(m_internalTitle, &QLineEdit::textEdited, this, [=](QString) {
        changeSubmitState();
    });
    m_internalSeverity = new QComboBox(this);
    m_internalSeverity->addItem("致命", "1");
    m_internalSeverity->addItem("高", "2");
    m_internalSeverity->addItem("中等", "3");
    m_internalSeverity->addItem("低", "4");
    m_internalSeverity->addItem("建议", "5");
    m_internalSeverity->addItem("不确定", "6");
    m_internalPri = new QComboBox(this);
    m_internalPri->addItem("紧急", "1");
    m_internalPri->addItem("重要", "2");
    m_internalPri->addItem("一般", "3");
    m_internalPri->addItem("暂缓", "4");
    m_internalDetailsEdit = new QTextEdit(this);
    connect(m_internalDetailsEdit, &QTextEdit::textChanged, this, [=] {
        changeSubmitState();
    });
    m_internalCodeError = new QComboBox(this);
    m_internalCodeError->addItem("代码错误", "codeerror");
    m_internalCodeError->addItem("配置相关", "config");
    m_internalCodeError->addItem("安装部署", "install");
    m_internalCodeError->addItem("安全相关", "security");
    m_internalCodeError->addItem("性能问题", "performance");
    m_internalCodeError->addItem("标准规范", "standard");
    m_internalCodeError->addItem("测试脚本", "automation");
    m_internalCodeError->addItem("设计缺陷", "designdefect");
    m_internalCodeError->addItem("其他", "others");
    m_internalExecute = new QComboBox(this);
    m_internalProduct = new QLineEdit(this);
    m_internalProduct->setEnabled(false);
    m_internalBuild = new QComboBox(this);
    connect(m_internalBuild, &QComboBox::currentTextChanged, this, &UiProblemFeedback::internalonBuildChange);
    m_internalAssign = new QLineEdit(this);
    connect(m_internalAssign, &QLineEdit::textEdited, this, [=](QString) {
        changeSubmitState();
    });
    m_internalAssign->setPlaceholderText("【必填】填禅道ID（拼音）");
    m_internalUsecase = new QLineEdit(this);
    m_internalUsecase->setPlaceholderText("用例ID");
    m_internalCutoff = new QDateEdit(this);
    QDate nowDate = QDateTime::currentDateTime().date();
    m_internalCutoff->setMinimumDate(nowDate);
    m_internalCutoff->setDisplayFormat("yyyy/MM/dd");
    m_internalModule1 = new QComboBox(this);
    connect(m_internalModule1, &QComboBox::currentTextChanged, this, &UiProblemFeedback::internalSetModule2);
    m_internalModule2 = new QComboBox(this);
    connect(m_internalModule2, &QComboBox::currentTextChanged, this, &UiProblemFeedback::internalSetModule3);
    m_internalModule3 = new QComboBox(this);
    connect(m_internalModule3, &QComboBox::currentTextChanged, this, &UiProblemFeedback::internalSetModule4);
    m_internalModule4 = new QComboBox(this);
    m_internalBranch = new QComboBox(this);
    m_internalCPUArch = new QComboBox(this);
    m_internalCPUArch->addItem("所有", "所有");
    m_internalCPUArch->addItem("x86", "x86");
    m_internalCPUArch->addItem("arm64", "arm64");
    m_internalCPUArch->addItem("龙芯", "龙芯");
    m_internalCPUArch->addItem("申威", "申威");
    m_internalCPUArch->addItem("龙芯3a5000", "龙芯3a5000");
    m_internalCPUArch->addItem("龙芯3c5000", "龙芯3c5000");


    QHBoxLayout *hl1 = new QHBoxLayout;
    hl1->setMargin(0);
    hl1->addWidget(new QLabel("版本", this));
    hl1->addWidget(m_internalBuild);
    hl1->addWidget(new QLabel("执行", this));
    hl1->addWidget(m_internalExecute);
    hl1->addStretch();
    QHBoxLayout *hl2 = new QHBoxLayout;
    hl2->setMargin(0);
    hl2->addWidget(new QLabel("产品", this));
    hl2->addWidget(m_internalProduct);
    hl2->addWidget(new QLabel("分支", this));
    hl2->addWidget(m_internalBranch);
    QHBoxLayout *hl3 = new QHBoxLayout;
    hl3->setMargin(0);
    hl3->addWidget(m_internalSeverity);
    hl3->addWidget(m_internalPri);
    hl3->addWidget(m_internalCodeError);
    QHBoxLayout *hl4 = new QHBoxLayout;
    hl4->setMargin(0);
    hl4->addWidget(new QLabel("指派给", this));
    hl4->addWidget(m_internalAssign);
    hl4->addWidget(new QLabel("关联用例", this));
    hl4->addWidget(m_internalUsecase);
    QHBoxLayout *hl5 = new QHBoxLayout;
    hl5->setMargin(0);
    hl5->addWidget(new QLabel("模块", this));
    hl5->addWidget(m_internalModule1);
    hl5->addWidget(m_internalModule2);
    hl5->addWidget(m_internalModule3);
    hl5->addWidget(m_internalModule4);
    hl5->addStretch();
    QHBoxLayout *hl6 = new QHBoxLayout;
    hl6->setMargin(0);
    hl6->addWidget(new QLabel("体系架构", this));
    hl6->addWidget(m_internalCPUArch);
    hl6->addWidget(new QLabel("截止日期", this));
    hl6->addWidget(m_internalCutoff);
    hl6->addStretch();



    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->setSpacing(16);
    vl->addLayout(hl1);
    vl->addLayout(hl2);
    vl->addLayout(hl5);
    vl->addLayout(hl3);
    vl->addLayout(hl4);
    vl->addLayout(hl6);
    vl->addWidget(m_internalTitle);
    vl->addWidget(m_internalDetailsEdit);

    setVerticalSpacingLayout(wid, vl);
    QLabel *label = new QLabel(this);
    label->setText(creatFormTitle("禅道信息", false));
    label->setAlignment(Qt::AlignTop);
    m_formLayoutPtrMap.insert(FORMLAYOUT_ROW_INTERNAL_PROJECT_INFO, FormLayoutRow(label, wid));
}

void UiProblemFeedback::errorMessage(const QString &msg)
{
    //    qDebug() << "反馈失败：" << msg;
    showMessageBox(msg, kdk::Error);
}

void UiProblemFeedback::prijectInfo(FeedBackInternalVerifyRes res, const QList<ProjectInfo> &list)
{
    if (res == LoginError) {
        bool ok = false;
        m_internalPasswd = QInputDialog::getText(this, "密码错误", "请重新输入密码", QLineEdit::Password, "", &ok,
                                                 Qt::FramelessWindowHint);
        if (!ok) {
            return;
        }
        showMessageBox("验证中，请稍等", kdk::Info);
        FeedbackManager::getInstance()->getProjectInfo(m_internalUser, m_internalPasswd);
        return;
    } else if (res == ConnectFaild) {
        showMessageBox("连接失败", kdk::Error);
        return;
    } else if (res == Nodata) {
        showMessageBox("未匹配到buildid", kdk::Error);
        return;
    }
    Settings::setZenTaoPwd(m_internalPasswd);
    m_projectInfo = list;
    m_internalMode = InZentao;
    onExpertchange(false);
    m_systemDeviceInfo = Settings::getSystemDeviceInfo();
    if (m_systemDeviceInfo.isEmpty()) {
        m_systemDeviceInfo = "\n";
    }
    for (const ProjectInfo info : m_projectInfo) {
        m_internalBuild->addItem(info.zentaoBuildName, info.zentaoBuildId);
    }
    resetState();
}

void UiProblemFeedback::internalonBuildChange(const QString &str)
{
    for (ProjectInfo &info : m_projectInfo) {
        if (info.zentaoBuildName == str) {
            m_internalExecute->clear();
            m_internalExecute->addItem(info.executionName, info.executionId);
            m_internalProduct->setText(info.productName);
            m_nowInfo.productId = info.productId;
            m_nowInfo.projectId = info.projectId;
            m_internalBranch->clear();
            m_internalModule1->clear();
            m_internalModule2->clear();
            m_internalModule3->clear();
            m_internalModule4->clear();
            for (const QString key : info.branchs.keys()) {
                m_internalBranch->addItem(info.branchs.value(key), key);
            }
            //模块
            m_internalModulesMap = info.modules;
            m_internalModulesList = sortQStringList(info.modules.values());
            //一级模块
            QStringList tmpList;
            for (const QString &key : m_internalModulesList) {
                tmpList.append(key.mid(0, key.indexOf('/', 1)));
            }
            tmpList = sortQStringList(tmpList.toSet().toList());
            m_internalModule1->addItems(tmpList);
            return;
        }
    }
}

void UiProblemFeedback::onDateFormatChange(QString str)
{
    qDebug() << "日期短格式改变：" << str;
    m_dateEdit->setDisplayFormat(QString(kdk_system_get_shortformat()));
}

void UiProblemFeedback::showMessageBox(const QString &msg, kdk::TipType type)
{
    kdk::KBallonTip *ballonTip = new kdk::KBallonTip();
    ballonTip->setAttribute(Qt::WA_DeleteOnClose);
    ballonTip->setFixedHeight(56);
    ballonTip->setWindowFlag(Qt::FramelessWindowHint);
    ballonTip->setAttribute(Qt::WA_TranslucentBackground);
    ballonTip->setText(msg);
    QPoint globalPoint = mapToGlobal(QPoint(geometry().x(), geometry().y()));
    int tmpWidth = QFontMetrics(ballonTip->font()).width(msg) + 46;
    int tmpHeight = 50;
    ballonTip->move(globalPoint.x() + (width() - tmpWidth) / 2, globalPoint.y() + (height() - tmpHeight) / 2 - 50);
    ballonTip->setTipType(type);
    ballonTip->showInfo();
}

void UiProblemFeedback::onFontSizeChange()
{
    double fontSizeCoefficient = kdk::GsettingMonitor::getSystemFontSize().toDouble() / 10.0;
    if (fontSizeCoefficient == 0) {
        qCritical() << "获取不到字号大小的值！";
        return;
    }
    double coefficient = fontSizeCoefficient * fontSizeCoefficient * fontSizeCoefficient;
    m_formLayoutHorizontalSpacing = 48 / coefficient;
    //    m_formLayoutContentsMarginsLeft = 40 / coefficient;
    m_formLayoutContentsMarginsLeft = 40;
    m_uploadText->setFixedHeight(42 * fontSizeCoefficient);
    onExpertchange(m_checkAdvanced->isChecked());
}

void UiProblemFeedback::onthemeChange()
{
    //主题切换时，主题色尚未切换，获取到的是切换主题之前的颜色，此为规避方案    
    QTimer::singleShot(5, [=](){ 
        QPalette pal = m_detailsEdit->palette();
        QColor col = qApp->palette().color(QPalette::Button);
        pal.setColor(QPalette::Base, col);
        m_detailsEdit->setPalette(pal);
        m_detailsEdit->update();
     }) ;
}

void UiProblemFeedback::onHighLightChange()
{
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Text, pal.color(QPalette::Highlight));
    m_systemInfoLabel->setPalette(pal);
}

void UiProblemFeedback::initConnect()
{
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this,
            &UiProblemFeedback::onFontSizeChange);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this,
            &UiProblemFeedback::onthemeChange);
    connect(FeedbackManager::getInstance(), &FeedbackManager::creatFinish, this, [=](FeedBackFinishType type) {
        switch (type) {
        case Success:
        case Oversize:
        case ExportLogSuccess:
            resetState();
            break;
        default:
            break;
        }
        if (m_customize == Settings::OPENKYLIN) {
            m_mailEdit->setEnabled(true);
            getUserCode();
        }
        changeSubmitState();
    });
    connect(FeedbackManager::getInstance(), &FeedbackManager::creatFinish, m_dialog,
            &UiProblemFeedbackDialog::showDialog);
    connect(FeedbackManager::getInstance(), &FeedbackManager::creatProgress, m_dialog,
            &UiProblemFeedbackDialog::showProgress);

    constexpr char THEMESCHEMA[] = "org.ukui.style";
    constexpr char HIGHTCOLORKEY1[] = "themeColor";
    constexpr char HIGHTCOLORKEY2[] = "theme-color";
    if (QGSettings::isSchemaInstalled(THEMESCHEMA)) {
        QGSettings *m_theme = new QGSettings(THEMESCHEMA, QByteArray(), this);
        connect(m_theme, &QGSettings::changed, this, [=](QString key) {
            if (key == HIGHTCOLORKEY1 || key == HIGHTCOLORKEY2) {
                onHighLightChange();
            }
        });
    }

    connect(FeedbackManager::getInstance(), &FeedbackManager::prijectInfo, this, &UiProblemFeedback::prijectInfo);
}

void UiProblemFeedback::initFinish()
{
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=] {
        m_uploadText1->setText(tr("No more than 5 files and total capacity not exceed %1MB").arg(m_addFileMax));
        m_uploadText2->setText(tr("Supported formats: ") + QString(UPLOAD_SUPPORT_SUFFIX));
    });
    connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    timer->start(10);
}

void UiProblemFeedback::resetState() {
    m_generateLogs->setEnabled(false);
    m_submit->setEnabled(false);

    for (QRadioButton *item : m_classTypeGroup) {
        item->setCheckable(true);
    }
    // 默认选中最后一个
    if (m_customize == Settings::ICBC) {
        if (!m_classTypeGroup.isEmpty()) {
            m_classTypeGroup.back()->setChecked(true);
        }
    }

    m_secondClass->clear();
    QDateTime now = QDateTime::currentDateTime();
    m_dateEdit->setDate(now.date());
    m_dateEditStart->setTime(now.time());
    m_dateEditEnd->setTime(now.time());
    setDateTime();

    m_detailsEdit->clear();
    m_detailsEdit->setPlaceholderText(m_dateEditText);
    if (m_customize == Settings::INTERNAL) {
        m_detailsEdit->setPlaceholderText(INTERNAL_PLACEHOLDER_TEXT);
    }
    m_detailsEditNumber->setText(m_detailsEditNumberText);

    m_uploadFileNameList.clear();
    m_fileListWidget->clear();
    m_fileListWidget->hide();
    m_screenCut->setEnabled(true);
    m_addFile->setEnabled(true);
    m_agreeAll->setChecked(true);
    for (QCheckBox *item : m_systemInfoGroup) {
        item->setChecked(true);
    }
    for (InformationClassItem *item : FeedbackManager::getInstance()->getLogFileItems()) {
        item->setSelect(true);
    }
    m_logYes->setChecked(true);
    m_logDays->show();

    // 重置姓名、邮箱、手机号
    kom::Configure conf;
    m_nameEdit->setText(
        conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_NAME_ADDRESS).toString());
    m_mailEdit->setText(conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL).toString());
    m_phoneNumberEdit->setText(
        conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT).toString());

    // 重置导出路径
    QString path = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH).toString();
    if (path.isEmpty() || m_customize == Settings::ICBC) {
        path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    m_exportEdit->setText(path);


    m_internalDetailsEdit->setText("[硬件信息]\n" + m_systemDeviceInfo
                                   + "[软件版本]\n\n[步骤]\n\n[结果]\n\n[期望]\n\n");
    m_internalAssign->setText(
        conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_INTERNAL_ASSIGNED_TO).toString());
}

const QString UiProblemFeedback::creatFormTitle(const QString &text, bool required) const
{
    if (!required) {
        return text;
    }
    return "<p style='white-space:pre;'><span style='color:#aa0000;'>*</span> " + text + "</p>";
}

void UiProblemFeedback::removeUploadFile(const QString &fileName)
{
    m_uploadFileNameList.removeAll(fileName);
    m_fileListWidget->clear();
    if (m_uploadFileNameList.isEmpty()) {
        m_fileListWidget->hide();
        onExpertchange(m_checkAdvanced->isChecked());
    }
    for (const QString &file : m_uploadFileNameList) {
        addUploadFileItem(file);
    }
    if (!(m_uploadFileNameList.length() > 4)) {
        m_screenCut->setEnabled(true);
        m_addFile->setEnabled(true);
    }
}

void UiProblemFeedback::addUploadFile(const QString &filename)
{
    if (m_uploadFileNameList.contains(filename)) {
        errorMessage(tr("Repeat addition"));
        return;
    }
    if (filename.contains("&")) {
        errorMessage(tr("文件名包含特殊字符"));
        return;
    }
    qint64 fileSize = 0;
    QFileInfo info;
    if (!m_uploadFileNameList.isEmpty()) {
        for (QString &file : m_uploadFileNameList) {
            info.setFile(file);
            fileSize += info.size();
        }
    }
    info.setFile(filename);
    fileSize += info.size();

    if (fileSize > 1024 * 1024 * m_addFileMax) {
        qDebug() << "附件大小超出限制" << fileSize;
        errorMessage(tr("Attachment size out of limit"));
        return;
    }
    m_uploadFileNameList.append(filename);
    addUploadFileItem(filename);
}

void UiProblemFeedback::addUploadFileItem(const QString &filename)
{
    UploadFileItem *itemWidget = new UploadFileItem(filename);
    connect(itemWidget, &UploadFileItem::removeUploadFile, this, &UiProblemFeedback::removeUploadFile);
    m_fileListWidget->addItem(itemWidget->getWidgetItem());
    m_fileListWidget->setItemWidget(itemWidget->getWidgetItem(), itemWidget);
    if (m_fileListWidget->isHidden()) {
        m_fileListWidget->show();
        onExpertchange(m_checkAdvanced->isChecked());
    }
    if (m_uploadFileNameList.length() > 4) {
        m_screenCut->setEnabled(false);
        m_addFile->setEnabled(false);
    }
}

void UiProblemFeedback::setVerticalSpacingLayout(QWidget *parent, QLayout *bl, int vSpacing)
{
    QVBoxLayout *vlVerticalSpacing = new QVBoxLayout(parent);
    vlVerticalSpacing->setSpacing(0);
    vlVerticalSpacing->setMargin(0);
    vlVerticalSpacing->addLayout(bl);
    if (vSpacing == 0) {
        vSpacing = m_formLayoutVerticalSpacing;
    }
    parent->setWhatsThis(QString::number(vSpacing));
    vlVerticalSpacing->addSpacing(vSpacing);
}

void UiProblemFeedback::saveFormLayoutPtr(int row, const QString &labelText, QWidget *wid)
{
    QLabel *label = new QLabel(this);
    label->setText(labelText);
    int bottomSpacing = wid->whatsThis().toInt();
    if (bottomSpacing == 0) {
        bottomSpacing = m_formLayoutVerticalSpacing;
    }
    label->setContentsMargins(0, 0, 0, bottomSpacing);
    m_formLayoutPtrMap.insert(row, FormLayoutRow(label, wid));
}

bool UiProblemFeedback::checkString(const QString &str, UiProblemFeedback::CheckStringType type)
{
    QRegExp exp;
    if (type == PhoneNumber) {
        exp = QRegExp("^1[0-9]{10}$");
    } else if (type == Mail) {
        exp = QRegExp("^[\\w-]+(\\.[\\w-]+)*@[\\w-]+(\\.[\\w-]+)+");
    }
    if (exp.exactMatch(str)) {
        return true;
    }
    return false;
}

QStringList UiProblemFeedback::sortQStringList(const QStringList &list)
{
    QStringList tmpList = list;
    QCollator collator;
    std::sort(tmpList.begin(), tmpList.end(), [&](const QString &a, const QString &b) {
        return collator.compare(a, b) < 0;
    });
    return tmpList;
}

void UiProblemFeedback::internalSetModule2(const QString &str)
{
    m_internalModule2->clear();
    if (str.length() < 2) {
        return;
    }
    QStringList tmpList;
    for (const QString &item : m_internalModulesList) {
        if (item.startsWith(str)) {
            tmpList.append(item.mid(str.length(), item.indexOf('/', str.length() + 1) - str.length()));
        }
    }
    tmpList = sortQStringList(tmpList.toSet().toList());
    m_internalModule2->addItems(tmpList);
}

void UiProblemFeedback::internalSetModule3(const QString &str)
{
    m_internalModule3->clear();
    if (str.length() < 2) {
        return;
    }
    QString first = m_internalModule1->currentText() + str;
    QStringList tmpList;
    for (const QString &item : m_internalModulesList) {
        if (item.startsWith(first)) {
            tmpList.append(item.mid(first.length(), item.indexOf('/', first.length() + 1) - first.length()));
        }
    }
    tmpList = sortQStringList(tmpList.toSet().toList());
    m_internalModule3->addItems(tmpList);
}

void UiProblemFeedback::internalSetModule4(const QString &str)
{
    m_internalModule4->clear();
    if (str.length() < 2) {
        return;
    }
    QString first = m_internalModule1->currentText() + m_internalModule2->currentText() + str;
    QStringList tmpList;
    for (const QString &item : m_internalModulesList) {
        if (item.startsWith(first)) {
            tmpList.append(item.mid(first.length()));
        }
    }
    tmpList = sortQStringList(tmpList.toSet().toList());
    m_internalModule4->addItems(tmpList);
}

void UiProblemFeedback::changeSubmitState() {
    m_generateLogs->setEnabled(false);
    m_submit->setEnabled(false);

    if (m_internalMode == DefaultMode || m_internalMode == ProjectMode) {
        if (m_detailsEdit->toPlainText().isEmpty()) {
            return;
        }
        if (m_customize == Settings::ICBC) {
            m_generateLogs->setEnabled(true);
            // 工行仅允许 软件开发中心 组织机构下的用户进行提交
            kom::Configure conf;
            auto enable_organization = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_SUBMIT_FILTER, "软件开发中心").toString();
            auto current_organization = FeedbackManager::getInstance()->getGroup();
            if (current_organization.contains(enable_organization)) {
                m_submit->setEnabled(true);
            }
            return;
        }
        if (m_secondClass->count() == 0) {
            return;
        }
        if (m_customize == Settings::OPENKYLIN) {
            if (m_mailEdit->text().isEmpty()) {
                return;
            }
            if (!m_mailErrorText->text().isEmpty()) {
                return;
            }
            m_submit->setEnabled(true);
            return;
        }
        if (m_phoneNumberEdit->text().isEmpty()) {
            return;
        }
        if (m_nameEdit->text().isEmpty()) {
            return;
        }
        if (!checkString(m_phoneNumberEdit->text(), PhoneNumber)) {
            return;
        }
        m_submit->setEnabled(true);
    } else if (m_internalMode == InZentao) {
        if (m_internalTitle->text().isEmpty()) {
            return;
        }
        if (m_internalDetailsEdit->toPlainText().isEmpty()) {
            return;
        }
        if (m_internalAssign->text().isEmpty()) {
            return;
        }
        m_submit->setEnabled(true);
    } 
}

void UiProblemFeedback::onSubmitClicked(bool onlyGenerateLogs)
{
    m_submit->setEnabled(false);
    QString titleLevel = "3";
    QString color = "red";
    InformationUserStruct info;
    info.mail = m_mailEdit->text();
    info.module = m_internalMode;
    kom::Configure conf;
    QString buriedPointAdvanced = "NormalModeOnSubmit";
    if (m_checkAdvanced->isChecked()) {
        buriedPointAdvanced = "AdvancedModeOnSubmit";
    }

    QString jobNum, userName, group;
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        jobNum = QString(pw->pw_name);
        userName = QString(pw->pw_gecos);
    }
    group = FeedbackManager::getInstance()->getGroup();


    info.jobNumber = jobNum;
    info.userName = userName;
    info.userGroup = group;
    info.user = m_nameEdit->text();
    info.phoneNumber = m_phoneNumberEdit->text();
    info.serviceNumber = Settings::getServiceNumber();

    QString details;
    creatDetails(details, titleLevel, color, "故障发生时间", m_dateTime);
    if (m_internalMode == ProjectMode) {
        creatDetails(details, titleLevel, color, "称谓", m_nameEdit->text());
        creatDetails(details, titleLevel, color, "手机号", m_phoneNumberEdit->text());
        creatDetails(details, titleLevel, color, "邮箱", m_mailEdit->text());
        creatDetails(details, titleLevel, color, "工号(本机)", jobNum);
        creatDetails(details, titleLevel, color, "称谓（本机）", userName);
        creatDetails(details, titleLevel, color, "组织（本机）", group);
    } else {
        creatDetails(details, titleLevel, color, "服务序列号", info.serviceNumber);
    }

    info.user = m_nameEdit->text();
    info.phoneNumber = m_phoneNumberEdit->text();
    info.serviceNumber = Settings::getServiceNumber();
    auto item = FeedbackManager::getInstance()->getChooseClassItem(m_classType);
    QString typeStr = item.oneCN;
    if (m_secondClass->currentIndex() > 0 && m_secondClass->currentIndex() < item.twoCN.length() - 1) {
        typeStr += "," + item.twoCN.at(m_secondClass->currentIndex());
    }
    info.priviteTypeStr = typeStr;

    if (m_internalMode == DefaultMode || m_internalMode == ProjectMode) {
        info.title = creatTitle();
        info.priviteType = QString::number(m_classType) + "-" + QString::number(m_secondClass->currentIndex());
        info.originalDetails = m_detailsEdit->toPlainText();
        m_tmpDetailsEdit->clear();
        m_tmpDetailsEdit->setText(details + m_detailsEdit->toHtml());
        info.details = m_tmpDetailsEdit->toHtml();
    } else if (m_internalMode == InZentao) {
        info.buildId = m_internalBuild->currentData().toString();
        info.title = m_internalTitle->text();
        info.details = m_internalDetailsEdit->toHtml();
        info.execution = m_internalExecute->currentData().toString();
        info.severity = m_internalSeverity->currentData().toString();
        info.pri = m_internalPri->currentData().toString();
        info.bugType = m_internalCodeError->currentData().toString();
        if (m_internalCutoff->date() != QDate::currentDate()) {
            info.deadline = m_internalCutoff->date().toString("yyyy-MM-dd");
        }
        info.productId = m_nowInfo.productId;
        info.projectId = m_nowInfo.projectId;
        info.assignedTo = m_internalAssign->text();
        info.usecase = m_internalUsecase->text();
        info.zentaoName = m_internalUser;
        info.zentaoPwd = m_internalPasswd;
        QString module = m_internalModule1->currentText() + m_internalModule2->currentText()
                         + m_internalModule3->currentText() + m_internalModule4->currentText();
        info.zentaoModule = m_internalModulesMap.key(module);
        info.zentaoBranch = m_internalBranch->currentData().toString();
        info.CPUArch = m_internalCPUArch->currentData().toString();
        conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_INTERNAL_ASSIGNED_TO,
                      m_internalAssign->text());
        buriedPointAdvanced = "InternalModeOnSubmit";
    }

    info.accessory = m_uploadFileNameList;

    conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_NAME_ADDRESS, m_nameEdit->text());
    if (m_mailErrorText->text().isEmpty()) {
        conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL, m_mailEdit->text());
    }
    conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT, m_phoneNumberEdit->text());
    conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH, m_exportEdit->text());

    FeedbackManager::getInstance()->setOnlyGenerateLogs(onlyGenerateLogs);
    if (onlyGenerateLogs) {
        auto desktopPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DesktopLocation);
        FeedbackManager::getInstance()->setSavePath(desktopPath);
    } else {
        FeedbackManager::getInstance()->setSavePath(m_exportEdit->text());
    }
    FeedbackManager::getInstance()->setUserData(info);
    FeedbackManager::getInstance()->startCollecting();

    kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kServiceSupport,
                                    kom::BuriedPoint::EventCode::kSubmit, {{"submitType", buriedPointAdvanced}});
}

void UiProblemFeedback::creatDetails(QString &orig, QString level, QString color, QString title, QString value)
{
    if (value.isEmpty()) {
        return;
    }
    orig += "<h" + level + " style='color:" + color + "'>" + title + ": " + value + "</h" + level + ">";
}

QString UiProblemFeedback::creatTitle()
{
    ChooseClassItem item = FeedbackManager::getInstance()->getChooseClassItem(m_classType);
    QString title = "【管家反馈】" + item.oneCN;

    if (m_secondClass->currentIndex() > 0) {
        title += "|" + item.twoCN.at(m_secondClass->currentIndex());
    }
    title += "|" + m_detailsEdit->toPlainText().split("\n").first().mid(0, 50);
    return title;
}

void UiProblemFeedback::creatFormLayout()
{
    if (m_formLayout) {
        delete m_formLayout;
    }
    m_formLayout = new QFormLayout(this);
    //    m_formLayout->setSizeConstraint(QLayout::SetMaximumSize);
    m_formLayout->setContentsMargins(m_formLayoutContentsMarginsLeft, 0, m_formLayoutContentsMarginsLeft / 3, 24);
    m_formLayout->setVerticalSpacing(0);
    m_formLayout->setHorizontalSpacing(m_formLayoutHorizontalSpacing);
    m_formLayout->setLabelAlignment(Qt::AlignRight);
}

void UiProblemFeedback::onScreenCutClicked()
{
    // 开始截图前，如果剪切板中存在图片的话，则清空剪切板
    // 避免本次启动截图而点叉关掉并没有进行实际截图时，读取到之前缓存的图片数据
    auto *clipboard = QApplication::clipboard();
    if (clipboard->mimeData()->hasImage()) {
        clipboard->clear();
    }

    QProcess::execute("/usr/bin/kylin-screenshot osmanager");
    m_startShot = true;
}

void UiProblemFeedback::onAddFileClicked()
{
    QString suffix = QString(UPLOAD_SUPPORT_SUFFIX);
    if (m_internalMode == InZentao) {
        suffix += QString(UPLOAD_SUPPORT_SUFFIX_PLUS);
    }
    QString file = QFileDialog::getOpenFileName(this, tr("Add attachment"),
                                                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                                                "(" + suffix + ")");
    if (file.isEmpty()) {
        return;
    }
    addUploadFile(file);
}

void UiProblemFeedback::onExpertchange(bool b)
{
    m_modelExpert.clear();
    if (m_internalMode == DefaultMode || m_internalMode == ProjectMode) {
        if (b) {
            m_modelExpert.append(FORMLAYOUT_ROW_CALSS);
            m_modelExpert.append(FORMLAYOUT_ROW_CALSS_SECOND);
            m_modelExpert.append(FORMLAYOUT_ROW_DATETIME);
            m_modelExpert.append(FORMLAYOUT_ROW_DETAILS);
            m_modelExpert.append(FORMLAYOUT_ROW_UPLOAD_FILE);
            m_modelExpert.append(FORMLAYOUT_ROW_CONTACT);
            if (m_customize == Settings::ICBC) {
                // Task: 221695 需求30000 【麒麟管家】使用反馈上报工单系统
                // 在工行模式下，如果开启了上传功能则移除掉“导出至”控件，默认导出至 /tmp，如果没有开启上传功能则显示“导出至”控件
                if (!Settings::isUpload()) {
                    m_modelExpert.append(FORMLAYOUT_ROW_EXPORT_PATH);
                }
            }
            m_modelExpert.append(FORMLAYOUT_ROW_MAIL);
            m_modelExpert.append(FORMLAYOUT_ROW_SYSTEM_INFO_SECOND);
            m_modelExpert.append(FORMLAYOUT_ROW_UPLOAD_LOGS);
            m_modelExpert.append(FORMLAYOUT_ROW_SUBMIT);
        } else {
            m_modelExpert.append(FORMLAYOUT_ROW_CALSS);
            m_modelExpert.append(FORMLAYOUT_ROW_DETAILS);
            m_modelExpert.append(FORMLAYOUT_ROW_UPLOAD_FILE);
            m_modelExpert.append(FORMLAYOUT_ROW_CONTACT);
            m_modelExpert.append(FORMLAYOUT_ROW_MAIL);
            m_modelExpert.append(FORMLAYOUT_ROW_SYSTEM_INFO);
            m_modelExpert.append(FORMLAYOUT_ROW_SUBMIT);
        }
        if (m_customize == Settings::ICBC) {
            m_modelExpert.removeAll(FORMLAYOUT_ROW_CONTACT);
            m_modelExpert.removeAll(FORMLAYOUT_ROW_MAIL);
            // Task: 226824 麒麟管家工行版本6月版优化
            // 移除掉“导出至”控件，不在显示
            m_modelExpert.removeAll(FORMLAYOUT_ROW_EXPORT_PATH);
            m_modelExpert.removeAll(FORMLAYOUT_ROW_CALSS_SECOND);
            m_modelExpert.removeAll(FORMLAYOUT_ROW_DATETIME);
        }
    } else if (m_internalMode == InZentao) {
        m_modelExpert.append(FORMLAYOUT_ROW_INTERNAL_PROJECT_INFO);
        m_modelExpert.append(FORMLAYOUT_ROW_UPLOAD_FILE);
        m_modelExpert.append(FORMLAYOUT_ROW_SUBMIT);
    }
    creatFormLayout();
    for (int i = 0; i < m_formLayoutPtrMap.count(); i++) {
        if (m_modelExpert.contains(i)) {
            showRow(i);
            auto res = m_formLayoutPtrMap.value(i);
            m_formLayout->addRow(res.label, res.widget);
        } else {
            hideRow(i);
        }
    }
    m_expertchange = true;
}

void UiProblemFeedback::paintEvent(QPaintEvent *e)
{
    if (m_expertchange) {
        m_expertchange = false;
        setFixedHeight(m_formLayout->sizeHint().height());
    }
}

void UiProblemFeedback::getShotImage() {
    const QMimeData *newData = QApplication::clipboard()->mimeData();
    if (!newData->hasImage()) {
        qDebug() << "剪贴板中不存在图片数据";
        return;
    }

    // 创建缓存目录
    auto cache_dir = QString("/tmp/problem_feedback_%1").arg(QString(getenv("USER")));
    QDir dir(cache_dir);
    if (!dir.exists()) {
        if (!dir.mkdir(cache_dir)) {
            qCritical() << "Collect screenshot annex create cache directory fail.";
            return;
        }
    }

    auto screenshot = QString("%1/%2.jpg").arg(cache_dir, QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    QImage img = qvariant_cast<QImage>(newData->imageData());
    if (m_lastMimeImage == img) {
        qDebug() << "重复截图，跳过";
        return;
    }
    if (!img.save(screenshot)) {
        qDebug() << "保存截图失败" << img << screenshot;
        return;
    }
    m_lastMimeImage = img;
    addUploadFile(screenshot);
}

void UiProblemFeedback::setDateTime()
{
    m_dateTime = m_dateEdit->text() + "  " + m_dateEditStart->text() + " ~ " + m_dateEditEnd->text();
}

void UiProblemFeedback::hideRow(int i)
{
    auto tmp = m_formLayoutPtrMap.value(i);
    auto *labelPtr = tmp.label;
    if (labelPtr != nullptr) {
        labelPtr->hide();
    }
    auto *fieldPtr = tmp.widget;
    if (fieldPtr != nullptr) {
        fieldPtr->hide();
    }
}

void UiProblemFeedback::showRow(int i)
{
    auto tmp = m_formLayoutPtrMap.value(i);
    auto *labelPtr = tmp.label;
    if (labelPtr != nullptr) {
        labelPtr->show();
    }
    auto *fieldPtr = tmp.widget;
    if (fieldPtr != nullptr) {
        fieldPtr->show();
    }
}

void UiProblemFeedback::getUserCode() {
    if (checkString(m_mailEdit->text(), Mail)) {
        return;
    }
    if (!m_mailEdit->isEnabled()) {
        m_connectGiteeBtn->setText(tr("Log in to gitee"));
        m_giteeUserCode.clear();
        m_mailEdit->setEnabled(true);
        m_mailEdit->clear();
        return;
    }
    m_webView->deleteLater();
    m_getCodeFinish = false;
    m_webView = new QWebEngineView;
    m_webView->resize(1250, 720);
    connect(m_webView, &QWebEngineView::urlChanged, this, [=](QUrl url) {
        QString str = url.toString();
        if (str.contains("https://gitee.com/organizations/openkylin?code=")) {
            m_giteeUserCode = str.split("=").last();
            m_getCodeFinish = true;
            m_webView->deleteLater();
            m_webView = nullptr;
            m_connectGiteeBtn->setText(tr("Log out of gitee"));
            m_mailEdit->setText(tr("gitee has been associated"));
            m_mailEdit->setEnabled(false);
            m_mailErrorText->setText("");
            changeSubmitState();
        }
    });
    if (!m_giteeUserCode.isEmpty()) {
        //连续提交不重复授权
        m_webView->setUrl(
            QUrl("https://gitee.com/oauth/"
                 "authorize?client_id=b0bb8c1cc58afa8a5c6959e382511cc20be2869e1b24ba1f7301d4535ec960a3&redirect_uri="
                 "https://gitee.com/organizations/openkylin&response_type=code&scope=user_info+issues+emails+notes"));
        return;
    }
    //启动后首次提交需授权
    m_webView->setUrl(
        QUrl("https://gitee.com/oauth/"
             "authorize?client_id=b0bb8c1cc58afa8a5c6959e382511cc20be2869e1b24ba1f7301d4535ec960a3&redirect_uri="
             "https://gitee.com/organizations/openkylin&response_type=code"));
    m_webView->show();
}

UploadFileItem::UploadFileItem(const QString &filename) : m_filename(filename)
{
    getType();
    setFixedSize(74, 74);
    m_widgetItem = new QListWidgetItem;
    m_widgetItem->setFlags(Qt::NoItemFlags);
    m_widgetItem->setToolTip(m_filename);
    m_widgetItem->setSizeHint(size());
    m_item = new QLabel(this);
    m_item->setFixedSize(64, 64);
    m_item->move(0, height() - m_item->height());
    m_closeBtn = new QPushButton(this);
    m_closeBtn->hide();
    m_closeBtn->setFixedSize(16, 16);
    m_closeBtn->setIcon(QIcon(":/res/file-item-close.png"));
    m_closeBtn->move(width() - m_closeBtn->width(), 0);
    connect(m_closeBtn, &QPushButton::clicked, this, [=] {
        emit removeUploadFile(m_filename);
    });
    if (m_fileType == Image) {
        QPixmap pix(m_filename);
        if (pix.isNull()) {
            pix.load(":/res/damaged_img.png");
        }
        m_item->setPixmap(pix.scaled(m_item->width(), m_item->height()));
        return;
    }
    m_icon = new QLabel(m_item);
    m_icon->setFixedSize(32, 32);
    if (m_fileType == Video) {
        m_icon->setPixmap(QIcon::fromTheme("video").pixmap(m_icon->size()));
    } else if (m_fileType == Package) {
        m_icon->setPixmap(QIcon::fromTheme("application-archive").pixmap(m_icon->size()));
    } else {
        m_icon->setPixmap(QIcon::fromTheme("application-octet-stream").pixmap(m_icon->size()));
    }

    m_icon->move(m_item->width() / 2 - m_icon->width() / 2, 0);
    m_name = new QLabel(m_item);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this,
            &UploadFileItem::textUpdate);
    m_name->move(0, m_item->height() - m_name->height());
    m_name->setFixedWidth(m_item->width());
    textUpdate();
}

void UploadFileItem::textUpdate()
{
    QString showText = QFileInfo(m_filename).fileName();
    QString tmp = showText;
    QFont ft = m_name->font();
    ft.setPointSizeF(kdk::GsettingMonitor::getSystemFontSize().toDouble());
    while (QFontMetrics(ft).width(showText) > m_name->width()) {
        tmp.chop(1);
        showText = tmp + "...";
    }
    m_name->setText(showText);
}

QListWidgetItem *UploadFileItem::getWidgetItem()
{
    return m_widgetItem;
}

bool UploadFileItem::event(QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        m_closeBtn->show();
    } else if (event->type() == QEvent::Leave) {
        m_closeBtn->hide();
    }
    return QWidget::event(event);
}

void UploadFileItem::getType()
{
    QString suffix = QFileInfo(m_filename).suffix().toLower();
    if (suffix == "jpg" || suffix == "png") {
        m_fileType = Image;
    } else if (suffix == "mp4") {
        m_fileType = Video;
    } else if (suffix == "zip" || suffix == "7z" || suffix == "tar.gz") {
        m_fileType = Package;
    }
}
