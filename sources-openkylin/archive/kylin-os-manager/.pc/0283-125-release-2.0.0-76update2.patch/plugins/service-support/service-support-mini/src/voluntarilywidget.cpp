#include "voluntarilywidget.h"
#include <QBoxLayout>
#include <QIcon>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QClipboard>
#include <QMimeData>
#include <QDateTime>
#include <QFileDialog>
#include <QLineEdit>
#include <QStandardPaths>
#include <QSettings>
#include <QGSettings>
#include <QStyleOptionButton>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <kpressbutton.h>
#include <gsettingmonitor.h>
#include "kom-label.h"
#include "kom-configure.h"

#define DETAIL_HEIGHT 150
#define NOMAL_SPACING 16
#define SMALL_SPACING 8
#define WINDOW_WIDTH 452
#define WINDOW_HEIGHT 686
#define HIGH_SCALE_WIDTH 452
#define HIGH_SCALE_HEIGHT 600
#define WINDOW_HEIGHT_BIG 770

#define UPLOAD_SUPPORT_SUFFIX "*.jpg *.png *.zip *.rar *.7z *.mp4 *.mkv"
#define UPLOAD_SUPPORT_SUFFIX_PLUS " *.mkv *.flv"
#define SCREEN_SHOT_GSETTING_PATH "org.ukui.screenshot"
#define SCREEN_SHOT_TMP_PATH "/tmp/kylin-os-manager/service-support/"
#define SCREEN_RECORD_EXE "/usr/bin/vokoscreen"

VoluntarilyWidget::VoluntarilyWidget(QWidget *parent)
    :kdk::KWidget(parent)
{

}

void VoluntarilyWidget::run()
{
    if (m_isInit) {
        return;
    }
    m_isInit = true;

    initUI();

    m_videoConfigPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/vokoscreen/vokoscreen.conf";
    connect(&m_fileWatcher,&QFileSystemWatcher::fileChanged,this,&VoluntarilyWidget::videoFileChanged);
    connect(&m_fileWatcher,&QFileSystemWatcher::directoryChanged,this,&VoluntarilyWidget::videoFileChanged);
    m_fileWatcher.addPaths(QStringList()<<m_videoConfigPath<<getVideoSavePath());
    connect(FeedbackManager::getInstance(),&FeedbackManager::creatFinish,this,&VoluntarilyWidget::creatFinish);

    addLogFile();
}

void VoluntarilyWidget::onButtonClick()
{
    m_submit->setEnabled(false);
    m_submit->setText(tr("Submitting"));
    if (m_submit->text() == tr("Retry")) {
        //FeedbackManager::getInstance()->retryUpload();
        return;
    }
    QString phoneNumber;
    if (m_phoneNumberErrMsg->text().isEmpty()) {
        phoneNumber = m_phoneNumberEdit->text();
    }
    startCollecting(m_agreeAll->isChecked(),m_detailsEdit->toPlainText(),phoneNumber,m_scoreWidget->getScore());
}

void VoluntarilyWidget::creatFinish(FeedBackFinishType type, QString str)
{
    QString failed = tr("Submission failed");
    switch (type) {
    case Success:
        qDebug()<<"提交成功";
        showMessageBox(tr("Submitted successfully"), kdk::Normal);
        m_submit->setText(tr("Submitted"));
        break;
    case Oversize:
        qDebug()<<"压缩包总大小超过限制";
        errorMessage(failed);
        m_submit->setEnabled(true);
        m_submit->setText(tr("Retry"));
        break;
    case UploadFail:
        qDebug()<<"提交失败";
        errorMessage(failed);
        m_submit->setEnabled(true);
        m_submit->setText(tr("Retry"));
        break;
    default:
        break;
    }
}


void VoluntarilyWidget::initUI() {
    if (GetSystemScale() > 1.3) {
        setFixedSize(HIGH_SCALE_WIDTH, HIGH_SCALE_HEIGHT);
    } else {
        setFixedSize(WINDOW_WIDTH,WINDOW_HEIGHT);
    }

    setAttribute(Qt::WA_AlwaysShowToolTips);
    setIcon(QIcon::fromTheme(ExecIcon()));
    setWindowIcon(QIcon::fromTheme(ExecIcon()));
    setWidgetName(ExecName());
    setWindowTitle(ExecName());
    windowButtonBar()->menuButton()->hide();
    windowButtonBar()->minimumButton()->hide();
    windowButtonBar()->maximumButton()->hide();

    // 隐藏任务栏图标
    const KWindowInfo window_info(winId(), NET::WMState);
    if (!window_info.hasState(NET::SkipTaskbar) || !window_info.hasState(NET::SkipPager)) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    }

    // 标题
    kom::KomLabel *label1 = new kom::KomLabel(this);
    label1->setFontSize(16);
    label1->setBold(true);
    label1->setText(tr("FeedBack"));

    // 问题描述
    QLabel *label2 = new QLabel(this);
    label2->setText(tr("Problem description"));

    m_cardBase = new kdk::KPressButton(this);
    m_cardBase->setCheckable(false);
    m_cardBase->setBorderRadius(6);
    m_cardBase->setFixedHeight(DETAIL_HEIGHT);

    QString dateEditText = tr("Please describe the problem in detail and you can upload a photo or file by clicking the button below.");
    QString detailsEditNumberText = tr("Up to %1 characters").arg(500);
    m_detailsEdit = new QTextEdit(this);
    m_detailsEdit->setAcceptRichText(false);
    m_detailsEdit->setFrameShape(QTextEdit::NoFrame);
    m_detailsEdit->setPlaceholderText(dateEditText);
    auto details_edit_palette = m_detailsEdit->palette();
    details_edit_palette.setColor(QPalette::ColorRole::Base, m_cardBase->palette().color(QPalette::Button));
    m_detailsEdit->setPalette(details_edit_palette);

    m_detailsEditNumber = new QLabel(m_detailsEdit);
    m_detailsEditNumber->hide();
    m_detailsEditNumber->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_detailsEditNumber->setText(detailsEditNumberText);
    QString detailsEditNumberTextFirst = tr("Remaining");
    QString detailsEditNumberTextLast = tr("character");
    connect(m_detailsEdit, &QTextEdit::textChanged, this, [=] {
        int maxTextNum = 500;
        QString text = m_detailsEdit->toPlainText();
        m_detailsEditNumber->setText(detailsEditNumberTextFirst + " " + QString::number(maxTextNum - text.length())
                                     + " " + detailsEditNumberTextLast);
        QPalette palet = qApp->palette();
        if (text.length() >= maxTextNum) {
            palet.setColor(QPalette::Foreground, QColor(Qt::red));
            m_detailsEditNumber->setPalette(palet);
        } else {
            palet.setColor(QPalette::Foreground, QColor(Qt::darkGray));
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

    QVBoxLayout *vlEdit = new QVBoxLayout;
    vlEdit->setContentsMargins(6, 6, 6, 32);
    vlEdit->addWidget(m_detailsEdit);
    m_cardBase->setLayout(vlEdit);

    // 上传附件
    QLabel *label3 = new QLabel(this);
    label3->setText(tr("Files"));

    // 截图
    m_screenCut = new QPushButton(this);
    m_screenCut->setText(tr("ScreenShot"));
    connect(m_screenCut, &QPushButton::clicked, this, &VoluntarilyWidget::onScreenCutClicked);
    if (QGSettings::isSchemaInstalled(SCREEN_SHOT_GSETTING_PATH)) {
        m_screenShotSetting = new QGSettings(SCREEN_SHOT_GSETTING_PATH);
        connect(m_screenShotSetting, &QGSettings::changed, this, [=]() {
            if (m_startShot && (m_screenShotSetting->get("isrunning").toString() == "false")) {
                m_startShot = false;
                getShotImage();
            }
        });
    }

    // 录屏
    m_video = new QPushButton(this);
    m_video->setText(tr("Screen recording"));
    if (!QFile(SCREEN_RECORD_EXE).exists()) {
        m_video->hide();
    }
    connect(m_video, &QPushButton::clicked, this, &VoluntarilyWidget::onscreenRecordClicked);

    // 添加文件
    m_addFile = new QPushButton(this);
    m_addFile->setText(tr("Add file"));
    connect(m_addFile, &QPushButton::clicked, this, &VoluntarilyWidget::onAddFileClicked);

    QHBoxLayout *filesButtonHL = new QHBoxLayout;
    filesButtonHL->setSpacing(SMALL_SPACING);
    filesButtonHL->addWidget(m_screenCut);
    filesButtonHL->addWidget(m_video);
    filesButtonHL->addWidget(m_addFile);
    filesButtonHL->addStretch();

    // 说明文案
    kom::KomLabel *uploadText1 = new kom::KomLabel(this);
    uploadText1->setFixedWidth(380);
    uploadText1->setFontSize(12);
    uploadText1->setTransparency(0.35);
    uploadText1->setText(tr("No more than 5 files and total capacity not exceed %1MB").arg(m_addFileMax));
    kom::KomLabel *uploadText2 = new kom::KomLabel(this);
    uploadText2->setFixedWidth(380);
    uploadText2->setFontSize(12);
    uploadText2->setTransparency(0.35);
    uploadText2->setText(tr("Supported formats: ") + QString(UPLOAD_SUPPORT_SUFFIX));

    // 添加文件列表
    m_fileListWidget = new QListWidget(this);
    m_fileListWidget->setFixedHeight(74);
    m_fileListWidget->setFlow(QListWidget::LeftToRight);
    m_fileListWidget->setFrameShape(QListWidget::NoFrame);
    m_fileListWidget->hide();

    // 评分
    QLabel *label4 = new QLabel(this);
    label4->setText(tr("My score"));
    m_scoreWidget= new kom::ScoreWidget(this);
    m_scoreWidget->setStarPix(QImage(":/res/star-l.png"),QImage(":/res/star-d.png"));

    // 联系方式
    QLabel *label5 = new QLabel(this);
    label5->setText(tr("Contact"));
    m_phoneNumberEdit = new QLineEdit(this);
    m_phoneNumberEdit->setPlaceholderText(tr("Please enter your phone number"));
    kom::Configure conf;
    m_phoneNumberEdit->setText(conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT).toString());

    // 手机号错误提示
    m_phoneNumberErrMsg = new QLabel(this);
    QPalette errMsgPalet = m_phoneNumberErrMsg->palette();
    errMsgPalet.setColor(QPalette::Text, Qt::red);
    m_phoneNumberErrMsg->setPalette(errMsgPalet);
    connect(m_phoneNumberEdit, &QLineEdit::textChanged, this, [=] {
        QRegExp exp("^1[0-9]{10}$");
        if (m_phoneNumberEdit->text().isEmpty()) {
            m_phoneNumberErrMsg->setText("");
            return;
        }
        if (!exp.exactMatch(m_phoneNumberEdit->text())) {
            m_phoneNumberErrMsg->setText(tr("The phone number format is incorrect"));
            return;
        }
        m_phoneNumberErrMsg->setText("");
    });

    // 同意获取系统信息
    m_agreeAll = new QCheckBox(tr("Agree to take mine "), this);
    m_agreeAll->setChecked(true);
    system_information_label_ = new QLabel(tr("System information"), this);
    QPalette pal = system_information_label_->palette();
    pal.setColor(QPalette::Text, qApp->palette().color(QPalette::Highlight));
    system_information_label_->setPalette(pal);

    QString tooltip;
    auto categories = FeedbackManager::getInstance()->getLogFileItems();
    for (const auto c: categories) {
        const auto name = c->getItemNameShow();
        tooltip.push_back(name);
        tooltip.push_back("、");
    }
    // 移除最后一个多余的 "、"符号
    if (!tooltip.isEmpty()) {
        tooltip.chop(1);
    }
    system_information_label_->setToolTip(tooltip);

    QHBoxLayout *hlSubmit = new QHBoxLayout;
    hlSubmit->setMargin(0);
    hlSubmit->setSpacing(0);
    hlSubmit->addWidget(m_agreeAll);
    hlSubmit->addWidget(system_information_label_);
    hlSubmit->addStretch();

    // 提交
    QPushButton *cancelBtn = new QPushButton(this);
    cancelBtn->setText(tr("Cancel"));
    connect(cancelBtn,&QPushButton::clicked,this,[=]{
        FeedbackManager::getInstance()->cancel();
        exit(0);
    });
    m_submit = new QPushButton(this);
    m_submit->setText(tr("Submit"));
    m_submit->setProperty("isImportant", true);
    connect(m_submit, &QPushButton::clicked, this, &VoluntarilyWidget::onButtonClick);

    QHBoxLayout *hlSubmit2 = new QHBoxLayout;
    hlSubmit2->setMargin(0);
    hlSubmit2->setSpacing(SMALL_SPACING);
    hlSubmit2->addStretch();
    hlSubmit2->addWidget(cancelBtn);
    hlSubmit2->addWidget(m_submit);

    // 总布局
    QVBoxLayout *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(24, 0, 24, 24);
    vbox_layout->addSpacing(NOMAL_SPACING);
    vbox_layout->addWidget(label1);
    vbox_layout->addSpacing(NOMAL_SPACING);
    vbox_layout->addWidget(label2);
    vbox_layout->addSpacing(SMALL_SPACING);
    vbox_layout->addWidget(m_cardBase);
    vbox_layout->addSpacing(NOMAL_SPACING);
    vbox_layout->addWidget(label3);
    vbox_layout->addSpacing(SMALL_SPACING);
    vbox_layout->addLayout(filesButtonHL);
    vbox_layout->addSpacing(SMALL_SPACING);
    vbox_layout->addWidget(uploadText1);
    vbox_layout->addWidget(uploadText2);
    vbox_layout->addSpacing(SMALL_SPACING);
    vbox_layout->addWidget(m_fileListWidget);
    vbox_layout->addSpacing(SMALL_SPACING);
    vbox_layout->addWidget(label4);
    vbox_layout->addSpacing(SMALL_SPACING);
    vbox_layout->addWidget(m_scoreWidget);
    vbox_layout->addSpacing(NOMAL_SPACING);
    vbox_layout->addWidget(label5);
    vbox_layout->addSpacing(SMALL_SPACING);
    vbox_layout->addWidget(m_phoneNumberEdit);
    vbox_layout->addWidget(m_phoneNumberErrMsg);
    vbox_layout->addLayout(hlSubmit);
    vbox_layout->addStretch();
    vbox_layout->addLayout(hlSubmit2);

    // 将总布局放入滚动区域中
    QWidget *widget = new QWidget(this);
    widget->setLayout(vbox_layout);
    m_baseScroll = new QScrollArea(this);
    m_baseScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_baseScroll->setFrameShape(QScrollArea::NoFrame);
    m_baseScroll->setWidget(widget);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(m_baseScroll);
    baseBar()->setLayout(hbox_layout);

    m_baseScroll->setWidgetResizable(true);

    auto *theme_gsettings = new QGSettings("org.ukui.style", "", this);
    connect(theme_gsettings, &QGSettings::changed, this, [this](QString key) {
        QStyleOptionButton option;
        if (key == "style-name" || key == "styleName") {
            auto details_edit_palette = m_detailsEdit->palette();
            details_edit_palette.setColor(QPalette::Base, option.palette.color(QPalette::Button));
            m_detailsEdit->setPalette(details_edit_palette);
        }
        if (key == "theme-color" || key == "themeColor") {
            // 高亮色改变
            auto palette = system_information_label_->palette();
            palette.setColor(QPalette::Text, option.palette.color(QPalette::Highlight));
            system_information_label_->setPalette(palette);
        }
    });
}

void VoluntarilyWidget::onScreenCutClicked()
{
    // 开始截图前，如果剪切板中存在图片的话，则清空剪切板
    // 避免本次启动截图而点叉关掉并没有进行实际截图时，读取到之前缓存的图片数据
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard->mimeData()->hasImage()) {
        clipboard->clear();
    }
    QProcess::execute("/usr/bin/kylin-screenshot osmanager");
    m_startShot = true;
}

void VoluntarilyWidget::getShotImage() {
    const QMimeData *newData = QApplication::clipboard()->mimeData();
    if (!newData->hasImage()) {
        qWarning() << "剪贴板中不存在图片数据";
        return;
    }

    // 创建缓存目录
    const auto cache_dir = QString("/tmp/problem_feedback_%1").arg(QString(getenv("USER")));
    const QDir dir(cache_dir);
    if (!dir.exists()) {
        if (!dir.mkdir(cache_dir)) {
            qCritical() << "Collect screenshot annex create cache directory fail.";
            return;
        }
    }

    const auto screenshot = QString("%1/%2.jpg").arg(cache_dir, QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    const QImage img = qvariant_cast<QImage>(newData->imageData());
    if (m_lastMimeImage == img) {
        qWarning() << "重复截图，跳过";
        return;
    }
    if (!img.save(screenshot)) {
        qCritical() << "保存截图失败" << img << screenshot;
        return;
    }
    m_lastMimeImage = img;
    addUploadFile(screenshot);
}

void VoluntarilyWidget::addUploadFile(const QString &filename)
{
    if (m_uploadFileNameList.contains(filename)) {
        errorMessage(tr("Repeat addition"));
        return;
    }
    if (filename.contains("&")) {
        errorMessage(tr("The file name contains special characters"));
        return;
    }

    // 计算所有文件的总大小
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

    if (fileSize > annex_size_max_limit_) {
        qDebug() << "附件大小超出限制" << fileSize;
        errorMessage(tr("Attachment size out of limit"));
        return;
    }

    m_uploadFileNameList.append(filename);
    addUploadFileItem(filename);
}

void VoluntarilyWidget::addUploadFileItem(const QString &filename)
{
    UploadFileItem *itemWidget = new UploadFileItem(filename);
    connect(itemWidget, &UploadFileItem::removeUploadFile, this, &VoluntarilyWidget::removeUploadFile);
    m_fileListWidget->addItem(itemWidget->getWidgetItem());
    m_fileListWidget->setItemWidget(itemWidget->getWidgetItem(), itemWidget);
    if (m_fileListWidget->isHidden()) {
        m_fileListWidget->show();
        if (GetSystemScale() < 1.3) {
            setFixedHeight(WINDOW_HEIGHT_BIG);
        }
    }
    if (m_uploadFileNameList.length() > 4) {
        m_screenCut->setEnabled(false);
        m_addFile->setEnabled(false);
    }
}

void VoluntarilyWidget::onAddFileClicked()
{
    QString suffix = QString(UPLOAD_SUPPORT_SUFFIX);
//    if (m_internalMode == InZentao) {
//        suffix += QString(UPLOAD_SUPPORT_SUFFIX_PLUS);
//    }
    QString file = QFileDialog::getOpenFileName(this, tr("Add file"),
                                                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                                                "(" + suffix + ")");
    if (file.isEmpty()) {
        return;
    }
    addUploadFile(file);
}

void VoluntarilyWidget::onscreenRecordClicked()
{
    if (m_screenRecord != nullptr) {
        m_screenRecord->deleteLater();
        m_screenRecord = nullptr;
    }
    m_video->setEnabled(false);
    m_screenRecord = new QProcess(this);
    connect(m_screenRecord,SIGNAL(finished(int)),this,SLOT(onScreenRecordFinish(int)));
    isWatcher = true;
    m_historyVideoFileList = QDir(getVideoSavePath()).entryList(QDir::NoDotAndDotDot | QDir::Files , QDir::Time);
    m_screenRecord->start(SCREEN_RECORD_EXE);
}

void VoluntarilyWidget::onScreenRecordFinish(int exit)
{
    isWatcher = false;
    m_video->setEnabled(true);
    qDebug()<< "录屏进程结束"<<exit;
}

void VoluntarilyWidget::videoFileChanged(const QString &filename)
{
    if (filename == m_videoConfigPath) {
        m_fileWatcher.addPaths(QStringList()<<m_videoConfigPath<<getVideoSavePath());
        return;
    }
    if (!isWatcher) {
        return;
    }
    QStringList nowVideoList = QDir(getVideoSavePath()).entryList(QDir::NoDotAndDotDot | QDir::Files , QDir::Time);
    QString newFile = nowVideoList.first();
    if (!m_historyVideoFileList.contains(newFile)) {
        if (newFile.startsWith("vokoscreen")) {
            qDebug()<<"自动添加录屏文件"<<newFile;
            addUploadFile(newFile);
        }
    }
    m_historyVideoFileList = nowVideoList;
}

QString VoluntarilyWidget::getVideoSavePath()
{
    QSettings setting(m_videoConfigPath , QSettings::IniFormat);
    setting.beginGroup("Miscellaneous");
    QString filePath = setting.value("VideoPath").toString();
    setting.endGroup();
    if (filePath.isEmpty()) {
        filePath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    }
    return filePath;
}

void VoluntarilyWidget::showMessageBox(const QString &msg, kdk::TipType type) {
    auto *ballonTip = new kdk::KBallonTip;
    ballonTip->setWindowFlag(Qt::FramelessWindowHint);
    ballonTip->setAttribute(Qt::WA_TranslucentBackground);
    ballonTip->setAttribute(Qt::WA_DeleteOnClose);
    ballonTip->setText(msg);
    ballonTip->setTipType(type);
    ballonTip->showInfo();

    // 移动到界面中间
    const auto p = pos();
    const auto x = p.x() + width() / 2 - ballonTip->width() / 2;
    const auto y = p.y() + height() / 2 - ballonTip->height() / 2;
    ballonTip->move(x, y);
}

void VoluntarilyWidget::errorMessage(const QString &msg)
{
    showMessageBox(msg, kdk::Error);
}

void VoluntarilyWidget::removeUploadFile(const QString &fileName)
{
    m_uploadFileNameList.removeAll(fileName);
    m_fileListWidget->clear();
    if (m_uploadFileNameList.isEmpty()) {
        m_fileListWidget->hide();
        if (GetSystemScale() < 1.3) {
            setFixedHeight(WINDOW_HEIGHT);
        }
    }

    for (const QString &file : m_uploadFileNameList) {
        addUploadFileItem(file);
    }
    if (!(m_uploadFileNameList.length() > 4)) {
        m_screenCut->setEnabled(true);
        m_addFile->setEnabled(true);
    }
}

void VoluntarilyWidget::addLogFile()
{
    QString logFile = getAppLogFile();
    if (!logFile.isEmpty()) {
        addUploadFile(logFile);
    }
}

double VoluntarilyWidget::GetSystemScale() {
    if (QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.xsettings")) {
        const QGSettings gsettings("org.ukui.SettingsDaemon.plugins.xsettings");
        const auto keys = gsettings.keys();
        if (keys.contains("scaling-factor") || keys.contains("scalingFactor")) {
            return gsettings.get("scalingFactor").toDouble();
        }
    }
    return 0;
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
