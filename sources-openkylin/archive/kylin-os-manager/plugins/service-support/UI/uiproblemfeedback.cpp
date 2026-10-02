#include "uiproblemfeedback.h"
#include <pwd.h>
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
#include <kpressbutton.h>
#include <gsettingmonitor.h>
#include "feedbackmanager.h"
#include "kom-configure.h"
#include "kom_buriedpoint.h"
#include "uiproblemfeedbackdialog.h"

#ifdef HAVE_QT5WEBENGINEWIDGETS
    #include <QtWebEngineWidgets/QWebEngineView>
#endif

#define UPLOAD_SUPPORT_SUFFIX "*.jpg *.png *.zip *.rar *.7z *.mp4"
#define SCREEN_SHOT_GSETTING_PATH "org.ukui.screenshot"
#define SCREEN_SHOT_TMP_PATH "/tmp/kylin-os-manager/service-support/"
#define FORMLAYOUT_ROW_CALSS 0
#define FORMLAYOUT_ROW_CALSS_SECOND 1
#define FORMLAYOUT_ROW_DATETIME 2
#define FORMLAYOUT_ROW_DETAILS 3
#define FORMLAYOUT_ROW_UPLOAD_FILE 4
#define FORMLAYOUT_ROW_CONTACT 5
#define FORMLAYOUT_ROW_MAIL 6
#define FORMLAYOUT_ROW_EXPORT_PATH 7
#define FORMLAYOUT_ROW_SYSTEM_INFO 8
#define FORMLAYOUT_ROW_SYSTEM_INFO_SECOND 9
#define FORMLAYOUT_ROW_UPLOAD_LOGS 10
#define FORMLAYOUT_ROW_SUBMIT 11

UiProblemFeedback::UiProblemFeedback(QWidget *parent) : QWidget(parent)
{
    setObjectName(m_objectName);
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
    if (m_loaded) {
        return;
    }
    if (name != m_objectName) {
        return;
    }
    m_loaded = true;
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
    QHBoxLayout *hlgroup = new QHBoxLayout;
    hlgroup->setMargin(0);
    hlgroup->setSpacing(0);
    for (ChooseClassItem &item : FeedbackManager::getInstance()->getChooseClassList()) {
        QRadioButton *radio = new QRadioButton(item.one, wid);
        m_classTypeGroup.append(radio);
        hlgroup->addWidget(radio);
        hlgroup->addSpacing(24);
        connect(radio, &QRadioButton::pressed, this, [=] {
            m_classType = item.code;
            m_secondClass->clear();
            m_secondClass->addItems(item.two);
            m_detailsEdit->setPlaceholderText(item.guide);
            changeSubmitState();
        });
    }
    m_checkAdvanced = new QCheckBox(tr("Advanced") + " ", wid);
    connect(m_checkAdvanced, &QCheckBox::clicked, this, &UiProblemFeedback::onExpertchange);
    hlgroup->addStretch();
    hlgroup->addWidget(m_checkAdvanced);
    setVerticalSpacingLayout(wid, hlgroup, 14);
    saveFormLayoutPtr(FORMLAYOUT_ROW_CALSS, creatFormTitle(tr("Type")), wid);
}

void UiProblemFeedback::initDetailsUI()
{
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
    m_dateEditText =
        tr("Please describe the problem in detail and you can upload a photo or file by clicking the button below.");
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
    label->setText(creatFormTitle(tr("Details")));
    label->setAlignment(Qt::AlignTop);
    m_formLayoutPtrMap.insert(FORMLAYOUT_ROW_DETAILS, FormLayoutRow(label, wid));


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
                QTimer::singleShot(300, this, [=] () {
                    getShotImage();
                });
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
    saveFormLayoutPtr(FORMLAYOUT_ROW_CONTACT, creatFormTitle(tr("Contact"), false), wid);

    QWidget *wid2 = new QWidget(this);
    m_mailEdit = new QLineEdit(this);
    m_mailEdit->setPlaceholderText(tr("Email required for anonymous feedback, not required for gitee feedback"));
    m_mailErrorText = new QLabel(wid);
    m_mailErrorText->setFixedHeight(m_formLayoutVerticalSpacing);
    m_mailErrorText->setFont(ft);
    m_mailErrorText->setPalette(palet);
    connect(m_mailEdit, &QLineEdit::textEdited, this, [=](QString str) {
        QString tmp = "";
        if (!m_mailEdit->text().isEmpty() && !checkString(str, Mail)) {
            tmp = tr("The mailbox format is incorrect");
        }
        m_mailErrorText->setText(tmp);
        changeSubmitState();
    });

    m_connectGiteeBtn = new QPushButton(this);
    m_connectGiteeBtn->setText(tr("Log in to gitee"));
    m_connectGiteeBtn->hide();
#ifdef HAVE_QT5WEBENGINEWIDGETS
    m_connectGiteeBtn->show();
#endif
    QHBoxLayout *hlMail = new QHBoxLayout;
    hlMail->setMargin(0);
    hlMail->addWidget(m_mailEdit);
    hlMail->addSpacing(10);
    hlMail->addWidget(m_connectGiteeBtn);
    connect(m_connectGiteeBtn, &QPushButton::clicked, this, &UiProblemFeedback::getUserCode);

    QVBoxLayout *vlMailEdit = new QVBoxLayout(wid2);
    vlMailEdit->setMargin(0);
    vlMailEdit->setSpacing(0);
    vlMailEdit->addLayout(hlMail);
    vlMailEdit->addWidget(m_mailErrorText);
    saveFormLayoutPtr(FORMLAYOUT_ROW_MAIL, creatFormTitle(tr("Mailbox")), wid2);
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

    QWidget *wid2 = new QWidget(this);
    QHBoxLayout *hlSubmit2 = new QHBoxLayout(wid2);
    hlSubmit2->setMargin(0);
    hlSubmit2->setSpacing(0);
    m_submit = new QPushButton(this);
    m_submit->setText(tr("Submit"));
    m_submit->setProperty("isImportant", true);
    connect(m_submit, &QPushButton::clicked, this, &UiProblemFeedback::onSubmitClicked);
    hlSubmit2->addWidget(m_submit);
    hlSubmit2->addStretch(9);
    saveFormLayoutPtr(FORMLAYOUT_ROW_SUBMIT, "", wid2);
}

void UiProblemFeedback::initExpertUI()
{
    QWidget *wid = new QWidget(this);
    m_secondClass = new QComboBox(this);
    QHBoxLayout *hlSecondClass = new QHBoxLayout;
    hlSecondClass->setMargin(0);
    hlSecondClass->setSpacing(0);
    hlSecondClass->addWidget(m_secondClass);
    setVerticalSpacingLayout(wid, hlSecondClass);
    saveFormLayoutPtr(FORMLAYOUT_ROW_CALSS_SECOND, creatFormTitle(tr("Details type"), false), wid);

    QWidget *wid0 = new QWidget(this);
    m_dateEdit = new QDateEdit(this);
    m_dateEditStart = new QTimeEdit(this);
    m_dateEditEnd = new QTimeEdit(this);
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

    QWidget *wid4 = new QWidget(this);
    m_exportEdit = new QLineEdit(wid4);
    m_exportEdit->setEnabled(false);
    QPushButton *exportBtn = new QPushButton(wid4);
    exportBtn->setText(tr("Path"));
    connect(exportBtn, &QPushButton::clicked, this, [=] {
        QString path = QFileDialog::getExistingDirectory(
            nullptr, "", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!path.isEmpty()) {
            m_exportEdit->setText(path);
        }
    });
    QHBoxLayout *hlExport = new QHBoxLayout;
    hlExport->setMargin(0);
    hlExport->addWidget(m_exportEdit);
    hlExport->setSpacing(10);
    hlExport->addWidget(exportBtn);
    setVerticalSpacingLayout(wid4, hlExport);
    saveFormLayoutPtr(FORMLAYOUT_ROW_EXPORT_PATH, creatFormTitle(tr("Export to"), false), wid4);
}

void UiProblemFeedback::errorMessage(const QString &msg)
{
    //    qDebug() << "反馈失败：" << msg;
    showMessageBox(msg, kdk::Error);
}

void UiProblemFeedback::retryUpload()
{
    if (!m_getCodeFinish) {
        m_dialog->exec();
    }
    FeedbackManager::getInstance()->retryUpload(m_giteeUserCode);
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
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=] {
        QPalette pal = qApp->palette();
        QColor col = qApp->palette().color(QPalette::Button);
        pal.setColor(QPalette::Base, col);
        m_detailsEdit->setPalette(pal);
        timer->deleteLater();
    });
    timer->start(1);
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
            resetState();
            break;
        default:
            break;
        }
        m_mailEdit->setEnabled(true);
        getUserCode();
        changeSubmitState();
    });
    connect(FeedbackManager::getInstance(), &FeedbackManager::creatFinish, m_dialog,
            &UiProblemFeedbackDialog::showDialog);
    connect(FeedbackManager::getInstance(), &FeedbackManager::creatProgress, m_dialog,
            &UiProblemFeedbackDialog::showProgress);
    connect(m_dialog, &UiProblemFeedbackDialog::retryUpload, this, &UiProblemFeedback::retryUpload);

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
}

void UiProblemFeedback::initFinish()
{
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=] {
        m_uploadText1->setText(tr("No more than 5 files and total capacity not exceed 10MB"));
        m_uploadText2->setText(tr("Supported formats: ") + QString(UPLOAD_SUPPORT_SUFFIX));
    });
    connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    timer->start(10);
}

void UiProblemFeedback::resetState()
{
    m_submit->setEnabled(false);
    for (QRadioButton *item : m_classTypeGroup) {
        item->setCheckable(false);
        item->setCheckable(true);
    }
    m_secondClass->clear();
    QDateTime now = QDateTime::currentDateTime();
    m_dateEdit->setDate(now.date());
    m_dateEditStart->setTime(now.time());
    m_dateEditEnd->setTime(now.time());
    setDateTime();
    m_detailsEdit->clear();
    m_detailsEditNumber->setText(m_detailsEditNumberText);
    m_detailsEdit->setPlaceholderText(m_dateEditText);
    m_uploadFileNameList.clear();
    m_fileListWidget->clear();
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
    m_logDays->setCurrentIndex(0);
    m_mailEdit->setEnabled(true);
    kom::Configure conf;
    m_nameEdit->setText(
        conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_NAME_ADDRESS).toString());
    QString configMail = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL).toString();
    if (checkString(configMail, Mail)) {
        m_mailEdit->setText(configMail);
    }
    m_phoneNumberEdit->setText(
        conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT).toString());
    QString path = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH).toString();
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    m_exportEdit->setText(path);
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

    if (fileSize > 1024 * 1024 * 10) {
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

void UiProblemFeedback::changeSubmitState()
{
    m_submit->setEnabled(false);
    if (m_secondClass->count() == 0) {
        return;
    }
    if (m_detailsEdit->toPlainText().isEmpty()) {
        return;
    }
    // if (m_phoneNumberEdit->text().isEmpty()) {
    //     return;
    // }
    //    if (m_nameEdit->text().isEmpty()) {
    //        return;
    //    }
    //    if (!checkString(m_phoneNumberEdit->text(), PhoneNumber)) {
    //        return;
    //    }

    if (m_mailEdit->text().isEmpty()) {
        return;
    }
    if (!m_mailErrorText->text().isEmpty()) {
        return;
    }
    m_submit->setEnabled(true);
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

void UiProblemFeedback::getUserCode()
{
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

#ifdef HAVE_QT5WEBENGINEWIDGETS
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
#endif
}

void UiProblemFeedback::onSubmitClicked()
{
    m_submit->setEnabled(false);
    QString titleLevel = "3";
    QString color = "red";
    InformationUserStruct info;
    info.title = creatTitle();
    info.mail = m_mailEdit->text();
    info.phoneNumber = m_phoneNumberEdit->text();
    info.name = m_nameEdit->text();
    info.module = m_classType;
    info.priviteType = QString::number(m_classType) + "-" + QString::number(m_secondClass->currentIndex());
    info.originalDetails = m_detailsEdit->toPlainText();
    info.giteecode = m_giteeUserCode;
    m_tmpDetailsEdit->clear();
    QString details;
    creatDetails(details, titleLevel, color, "故障发生时间", m_dateTime);

    m_tmpDetailsEdit->setText(details + m_detailsEdit->toHtml());
    info.details = m_tmpDetailsEdit->toPlainText();
    info.accessory = m_uploadFileNameList;

    kom::Configure conf;
    conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_NAME_ADDRESS, m_nameEdit->text());
    if (checkString(m_mailEdit->text(), Mail)) {
        conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL, m_mailEdit->text());
    }
    conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT, m_phoneNumberEdit->text());
    conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH, m_exportEdit->text());

    FeedbackManager::getInstance()->setSavePath(m_exportEdit->text());
    FeedbackManager::getInstance()->setUserData(info);
    FeedbackManager::getInstance()->startCollecting();
    QString buriedPointAdvanced = "NormalModeOnSubmit";
    if (m_checkAdvanced->isChecked()) {
        buriedPointAdvanced = "AdvancedModeOnSubmit";
    }
    kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kServiceSupport,
                                    kom::BuriedPoint::EventCode::kSubmit,
                                    {{"submitType", buriedPointAdvanced}});
}

void UiProblemFeedback::creatDetails(QString &orig, QString level, QString color, QString title, QString value)
{
    if (value.isEmpty()) {
        return;
    }
    orig += "<h" + level + " style='color:" + color + "'>" + title + ":" + value + "</h" + level + ">";
}

QString UiProblemFeedback::creatTitle()
{
    ChooseClassItem item = FeedbackManager::getInstance()->getChooseClassItem(m_classType);
    QString title = item.oneCN;

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
    const QMimeData *mimeData = clipboard->mimeData();
    bool hasImage = false;

    if (mimeData->hasImage()) {
        hasImage = true;
    }

    if (!hasImage && mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            if (filePath.isEmpty()) {
                filePath = url.toString();
            }
            QString urlLower = filePath.toLower();
            if (urlLower.endsWith(".jpg") || urlLower.endsWith(".jpeg") ||
                urlLower.endsWith(".png") || urlLower.endsWith(".gif") ||
                urlLower.endsWith(".bmp") || urlLower.endsWith(".webp")) {
                hasImage = true;
                break;
            }
        }
    }

    if (hasImage) {
        QMimeData *emptyData = new QMimeData();
        clipboard->setMimeData(emptyData);
    }
    QProcess::execute("/usr/bin/kylin-screenshot osmanager");
    m_startShot = true;
}

void UiProblemFeedback::onAddFileClicked()
{
    QString file = QFileDialog::getOpenFileName(nullptr, tr("Add attachment"),
                                                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                                                "(" + QString(UPLOAD_SUPPORT_SUFFIX) + ")");
    if (file.isEmpty()) {
        return;
    }
    addUploadFile(file);
}

void UiProblemFeedback::onExpertchange(bool b)
{
    m_modelExpert.clear();
    if (b) {
        m_modelExpert.append(FORMLAYOUT_ROW_CALSS);
        m_modelExpert.append(FORMLAYOUT_ROW_CALSS_SECOND);
        m_modelExpert.append(FORMLAYOUT_ROW_DATETIME);
        m_modelExpert.append(FORMLAYOUT_ROW_DETAILS);
        m_modelExpert.append(FORMLAYOUT_ROW_UPLOAD_FILE);
        m_modelExpert.append(FORMLAYOUT_ROW_CONTACT);
        m_modelExpert.append(FORMLAYOUT_ROW_MAIL);
        m_modelExpert.append(FORMLAYOUT_ROW_EXPORT_PATH);
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

void UiProblemFeedback::getShotImage()
{
    const QMimeData *newData = QApplication::clipboard()->mimeData();
    QImage image;
    if (newData->hasUrls()) {
        if (newData->urls().size() == 1) {
            image = QImage(newData->urls().at(0).path());
        }
    }
    if (image.isNull()) {
        if (newData->hasImage()) {
            image = qvariant_cast<QImage>(newData->imageData());
        }
    }
    if (image.isNull()) {
        qDebug() << "剪贴板中不存在图片数据";
        return;
    }

    if (!QDir().mkpath(SCREEN_SHOT_TMP_PATH)) {
        qDebug() << "创建截图临时目录失败";
        return;
    }
    QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + QString(".jpg");
    QString filePath = SCREEN_SHOT_TMP_PATH + fileName;
    if (m_lastMimeImage == image) {
        return;
    }
    if (!image.save(filePath)) {
        qDebug() << "保存截图失败";
        return;
    }
    m_lastMimeImage = image;
    addUploadFile(filePath);
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
