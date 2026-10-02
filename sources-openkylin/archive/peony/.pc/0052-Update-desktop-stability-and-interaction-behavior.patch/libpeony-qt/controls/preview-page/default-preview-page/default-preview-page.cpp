/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "default-preview-page.h"
#include "thumbnail-manager.h"

#include <QLabel>

#include <QFile>
#include <QFileInfo>
#include <QUrl>

#include <QPixmap>
#include <QIcon>
#include <QMimeDatabase>

#include <QVBoxLayout>
#include <QFormLayout>

#include <QLocale>
#include <QDateTime>

#include <QThreadPool>

#include <QResizeEvent>

#include <QImageReader>

#include <QPainter>
#include <QGSettings>
#include <QDBusConnection>
#include <QDBusReply>
#include <QtConcurrent>
#include <memory>
#include <QTextLayout>
#include <QTextLine>

#include "icon-container.h"

#include "file-info.h"
#include "file-info-manager.h"
#include "file-watcher.h"

#include "file-info-job.h"

#include "file-count-operation.h"

#include "FMWindowIface.h"
#include "file-utils.h"
#include "global-settings.h"
#include "idm-dbus-manager.h"
#include "file-launch-manager.h"
#include "directory-view-container.h"
#include "directory-view-widget.h"

#include <kinputdialog.h>
#include <kmessagebox.h>
using namespace Peony;

#define LABEL_MAX_WIDTH       165

DefaultPreviewPage::DefaultPreviewPage(QWidget *parent) : QStackedWidget (parent)
{
    setContentsMargins(10, 20, 2, 40);

    auto label = new QLabel(tr("Select the file you want to preview"), this);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    m_empty_tab_widget = label;

    /*
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    */
    auto previewPage = new FilePreviewPage(this);
    previewPage->installEventFilter(this);
    m_preview_tab_widget = previewPage;
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true); // 允许内部控件自动调整大小
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 水平滚动条按需显示
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);   // 垂直滚动条按需显示
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; }");
    m_scrollArea->viewport()->setStyleSheet("background: transparent;");  // 视口透明
    // 设置透明背景
    // 假设 filePreviewPage 是你已有的 FilePreviewPage 实例
    m_scrollArea->setWidget(m_preview_tab_widget);

    addWidget(m_scrollArea);
    addWidget(m_empty_tab_widget);
    setCurrentWidget(m_empty_tab_widget);

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *gSetting = new QGSettings("org.ukui.style", QByteArray(), this);
        connect(gSetting, &QGSettings::changed, this, [=](const QString &key) {
            if ("systemFontSize" == key) {
                if (m_support && m_preview_tab_widget) {
                    if (m_info) {
                        m_preview_tab_widget->updateInfo(m_info.get());
                    }
                }
            }
        });
    }

    if (QGSettings::isSchemaInstalled("org.ukui.control-center.panel.plugins")) {
        QGSettings* settings = new QGSettings("org.ukui.control-center.panel.plugins", QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if ("hoursystem" == key) {
                if (m_support && m_preview_tab_widget) {
                    if (m_info) {
                        FileInfoJob* infoJob = new FileInfoJob(m_info, this);
                        infoJob->setAutoDelete(true);
                        connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=] {
                            m_preview_tab_widget->updateInfo(m_info.get());
                        });
                        infoJob->queryAsync();
                    }
                }
            }
        });
    }

#ifdef KY_SDK_DATE
    connect(GlobalSettings::getInstance(),
            &GlobalSettings::updateLongDataFormat,
            this,
            &DefaultPreviewPage::updateDateFormat);
#endif
}

DefaultPreviewPage::~DefaultPreviewPage()
{
    cancel();
}

void DefaultPreviewPage::updateDateFormat(QString dateFormat)
{
    //update date and time show format, task #101605
    qDebug() << "sdk format signal:"<<dateFormat<<m_date_format;
    if (m_date_format != dateFormat){
        if (m_support && m_preview_tab_widget) {
            if (m_info) {
                FileInfoJob* infoJob = new FileInfoJob(m_info, this);
                infoJob->setAutoDelete(true);
                connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=] {
                    m_preview_tab_widget->updateInfo(m_info.get());
                });
                infoJob->queryAsync();
            }
        }
        m_date_format = dateFormat;
    }
}

bool DefaultPreviewPage::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_preview_tab_widget) {
        if (ev->type() == QEvent::Resize) {
            auto e = static_cast<QResizeEvent*>(ev);
            auto page = qobject_cast<FilePreviewPage*>(m_preview_tab_widget);
            int width = e->size().width();
            width = qMax(width, 96);
            width = qMin(width, 256);
            page->resizeIcon(QSize(width* 2/3, width* 3/4));
            page->updateForm(e->size());
        }
    }
    return QStackedWidget::eventFilter(obj, ev);
}

void DefaultPreviewPage::prepare(const QString &uri, PreviewType type)
{
    if (m_watcher) {
        m_watcher->disconnect();
        m_watcher.reset();
    }

    m_current_uri = uri;
    m_info = FileInfo::fromUri(uri);
    m_current_type = type;
    m_support = (uri.contains("file:///") || uri.contains("mtp://") || uri.contains("gphoto2://") || uri.contains("idm://") || uri.startsWith("smb://") || uri.startsWith("ftp://") || uri.startsWith("sftp://")) ? true : false;
    m_watcher = std::make_shared<FileWatcher>(uri);
    connect(m_watcher.get(), &FileWatcher::locationChanged, [=](const QString &, const QString &newUri) {
        this->prepare(newUri);
        this->startPreview();
    });
    m_watcher->startMonitor();
}

void DefaultPreviewPage::prepare(const QString &uri)
{
    prepare(uri, Other);
}

void DefaultPreviewPage::startPreview()
{
    FMWindowIface *iface = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
    bool locationSupport = true;
    if (iface) {
        QString uri = iface->getCurrentUri();
        QString spaceName;
        if(uri.startsWith("idm:///")){
            spaceName = uri.remove("idm:///");
            spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
        }
        m_preview_tab_widget->setSpaceName(spaceName);
        if (iface->getCurrentUri() == "computer:///" || m_info->isVirtual()) {
            locationSupport = false;
        }
    }

    if (m_support && locationSupport) {
        auto previewPage = qobject_cast<FilePreviewPage*>(m_preview_tab_widget);
        previewPage->updateInfo(m_info.get());
        setCurrentWidget(m_scrollArea);
    } else {
        QLabel *label = qobject_cast<QLabel*>(m_empty_tab_widget);
        label->setText(tr("Can not preview this file"));
    }
}

void DefaultPreviewPage::cancel()
{
    m_preview_tab_widget->cancel();
    if (m_watcher) {
        m_watcher->disconnect();
        m_watcher.reset();
    }
    setCurrentWidget(m_empty_tab_widget);
    QLabel *label = qobject_cast<QLabel*>(m_empty_tab_widget);
    label->setText(tr("Select the file you want to preview"));
}

void DefaultPreviewPage::closePreviewPage()
{
    cancel();
    m_info.reset();
    m_current_uri.clear();
    deleteLater();
}

void DefaultPreviewPage::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(this->rect(), this->palette().base());
    QStackedWidget::paintEvent(e);
}

QMutex FilePreviewPage::m_cacheMutex;
QCache<QString, QPair<QStringList, QString>> FilePreviewPage::m_labelSummaryCache(1000);

FilePreviewPage::FilePreviewPage(QWidget *parent) : QFrame(parent)
{
    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);

    m_icon = new IconContainer(this);
    //fix bug#201023,200758, 200393 pure picture preview issue
    m_icon->setProperty("useIconHighlightEffect", 0x0);
    m_icon->setIconSize(QSize(96, 96));
    m_icon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_layout->addWidget(m_icon);
    m_layout->addSpacing(24);

    m_form = new QFormLayout(this);
    m_form->setSpacing(3);
    m_form->setContentsMargins(0, 0, 0, 0);

    m_display_name_label = new QLabel(this);
    this->setLabelProperties(m_display_name_label);
    QFont font;
    font.setBold(true);
    m_display_name_label->setFont(font);
    m_layout->addWidget(m_display_name_label);
    m_form_label_map.insert(m_display_name_label, "");
    m_layout->addSpacing(6);

    m_lableContainer = new QWidget(this);
    m_intelligentLabelLayout = new labelFlowLayout(m_lableContainer);
    m_lableContainer->setLayout(m_intelligentLabelLayout);
    m_form->insertRow(1, m_lableContainer);
    m_form->setAlignment(m_lableContainer,Qt::AlignLeft);

    m_file_summery_label = new QLabel(this);
    this->setLabelProperties(m_file_summery_label);
    m_file_summery_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_form_label_map.insert(m_file_summery_label, "");
    m_form->insertRow(2, m_file_summery_label);
    m_file_summery_label->setWordWrap(true);
    m_file_summery_label->setContentsMargins(0,6,0,0);
    addSeparator();

    m_location_label = new QLabel(this);
    this->setLabelProperties(m_location_label);
    m_form->addRow(getLabel(tr("Location")), m_location_label);
    m_location_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_location_label, "");

    addSeparator();

    m_type_label = new QLabel(this);
    this->setLabelProperties(m_type_label);
    m_form->addRow(getLabel(tr("File Type")), m_type_label);
    m_type_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_type_label, "");

    addSeparator();

    m_total_size_label = new QLabel(this);
    this->setLabelProperties(m_total_size_label);
    m_form->addRow(getLabel(tr("Size")), m_total_size_label);
    m_total_size_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_total_size_label, "");

    addSeparator();

    m_time_create_label = new QLabel(this);
    this->setLabelProperties(m_time_create_label);
    m_form->addRow(getLabel(tr("Time Created")), m_time_create_label);
    m_time_create_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_time_create_label, "");

    addSeparator();

    m_time_modified_label = new QLabel(this);
    this->setLabelProperties(m_time_modified_label);
    m_form->addRow(getLabel(tr("Time Modified")), m_time_modified_label);
    m_time_modified_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_time_modified_label, "");

    addSeparator();

    m_time_access_label = new QLabel(this);
    this->setLabelProperties(m_time_access_label);
    m_form->addRow(getLabel(tr("Time Access")), m_time_access_label);
    m_time_access_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_time_access_label, "");

    addSeparator();

    m_file_count_label = new QLabel(this);
    this->setLabelProperties(m_file_count_label);
    QLabel *children_label = new QLabel(this);
    children_label->setAlignment(Qt::AlignTop);
    children_label->setText(tr("Children Count"));
    m_form->addRow(children_label, m_file_count_label);
    m_file_count_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_file_count_label, "");

    addSeparator();

    //image
    m_image_size = new QLabel(this);
    this->setLabelProperties(m_image_size);
    m_form->addRow(getLabel(tr("Image resolution")), m_image_size);
    m_image_size->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_image_size, "");

    addSeparator();

    m_image_format = new QLabel(this);
    this->setLabelProperties(m_image_format);
    m_form->addRow(getLabel(tr("color model")), m_image_format);
    m_image_format->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_image_format, "");

    addSeparator();

    m_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    m_form->setFormAlignment(Qt::AlignHCenter);

    QWidget *form = new QWidget(this);
    form->setLayout(m_form);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(form);

    QLabel* openFileLabel = new QLabel(tr("<a href='#' style = 'text-decoration:none'>Open File</a>"));
    QLabel* openDirLabel = new QLabel(tr("<a href='#' style = 'text-decoration:none'>Open File Directory</a>"));
    m_removeLabel = new QLabel(tr("<a href='#' style = 'text-decoration:none'>Remove From Space</a>"));
    openFileLabel->setOpenExternalLinks(false); // 禁止自动打开外部链接
    openDirLabel->setOpenExternalLinks(false);
    m_removeLabel->setOpenExternalLinks(false);

    openFileLabel->setTextFormat(Qt::RichText);
    openDirLabel->setTextFormat(Qt::RichText);
    m_removeLabel->setTextFormat(Qt::RichText);

    // 绑定点击事件
    connect(openFileLabel, &QLabel::linkActivated, this, [this](const QString&) {
        if (m_isDir) {
            this->openInNewWindow(m_currentFileUri);
        } else {
            // 执行打开文件逻辑
            Peony::FileLaunchManager::openAsync(m_currentFileUri);
        }
    });
    connect(openDirLabel, &QLabel::linkActivated, this, [this](const QString&) {
        QString targetUri = m_currentFileUri;
        QString parentUri = FileUtils::getParentUri(targetUri);
        FMWindowIface *window = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
        if (window) {
            auto windowIface = window->create(parentUri);
            auto newWindow = dynamic_cast<QWidget *>(windowIface);
            if (newWindow) {
                newWindow->setAttribute(Qt::WA_DeleteOnClose);
                newWindow->show();

                QTimer::singleShot(500, this, [targetUri, windowIface]() {
                    if (windowIface) {
                        auto page = windowIface->getCurrentPage();
                        if (page && page->getView()) {
                            page->getView()->setSelections(QStringList() << targetUri);
                            page->getView()->scrollToSelection(targetUri);
                            page->getView()->setFocus();
                        }
                    }
                });
            }
        }
    });
    connect(m_removeLabel, &QLabel::linkActivated, this, [this](const QString&) {
        // 执行移除逻辑
        if (!qApp->property("skipAskRemoveFile").toBool()) {
            kdk::KMessageBox questionBox;
            questionBox.setIcon(kdk::KMessageBox::Icon::Question);
            auto cancelButton = new QPushButton(tr("Cancel"));
            auto okButton = new QPushButton(tr("OK"));
            questionBox.addButton(cancelButton, kdk::KMessageBox::RejectRole);
            questionBox.addButton(okButton, kdk::KMessageBox::AcceptRole);
            okButton->setProperty("isImportant", true);
            okButton->setDefault(true);
            okButton->setFocus();
            QString text = tr("<font color='gray'>After removal,when files in the space are aggregated and updated,these files will be no longer be displayed</font>");
            QString filetext = tr("<font color='gray'>After removal,when files in the space are aggregated and updated,this files will be no longer be displayed</font>");

            questionBox.setText(tr("Do you want to remove this file?<br>")+"<br>"+"<br>"+filetext);
            auto checkbox = new QCheckBox;
            checkbox->setText(tr("Do not ask again"));
            questionBox.setCheckBox(checkbox);
            questionBox.exec();
            auto resultButton = questionBox.clickedButton();

            if (resultButton != okButton)
                return;
            if (checkbox->isChecked()) {
                qApp->setProperty("skipAskRemoveFile", true);
            }
        }

        if (!m_currentFileUri.isEmpty()) {
            QUrl url(m_currentFileUri);
            QString path = url.toLocalFile();
            QStringList filelist;
            filelist.append(path);
            IDMDBusManager::getInstance()->removeFilesFromSpace(m_spaceName, filelist);
        }
    });

    // 添加到布局
    m_layout->addWidget(openFileLabel);
    m_layout->addWidget(openDirLabel);
    m_layout->addWidget(m_removeLabel);
    //removeLabel->setVisible(false);
    // m_layout->addSpacing(20);
    m_layout->addStretch(1);
}

FilePreviewPage::~FilePreviewPage()
{
    m_destroyed = true;
}

void FilePreviewPage::wrapData(QLabel *p_label, const QString &text)
{
    QString wrapText;
    QTextLayout textLayout(text, p_label->font());
    QTextOption option = textLayout.textOption();
    option.setWrapMode(QTextOption::WrapAnywhere);
    textLayout.setTextOption(option);

    int width = p_label->width() == 0 ? LABEL_MAX_WIDTH : p_label->width();
    textLayout.beginLayout();
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(width);
    }
    textLayout.endLayout();

    QStringList lines;
    for (int i = 0; i < textLayout.lineCount(); ++i) {
        QTextLine line = textLayout.lineAt(i);
        lines.append(text.mid(line.textStart(), line.textLength()));
    }

    wrapText = lines.join("\n");
    p_label->setText(wrapText);
}

void FilePreviewPage::updateInfo(FileInfo *info)
{
    if (info->displayName().isEmpty() ||
            info->fileType().isEmpty() ||
            info->accessTime() == 0 ||
            info->modifiedTime() == 0) {
        FileInfoJob j(info->uri());
        j.querySync();
    }

    auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(info->uri());
    if (!thumbnail.isNull()) {
        QUrl url = info->uri();
        thumbnail.addFile(url.path());
    }
    auto icon = QIcon::fromTheme(info->iconName(), QIcon::fromTheme("unknown"));
    m_icon->setIcon(thumbnail.isNull()? icon: thumbnail);
  
    //fix bug:#82320
    QString displayName = info->displayName();
    if (QRegExp("^file:///data/usershare(/{,1})$").exactMatch(info->uri())) {
        displayName = tr("usershare");
    }

    QUrl url = FileUtils::getParentUri(info->uri());
    QString location = url.toDisplayString();
    if (location.startsWith("file://")) {
        location = location.split("file://").last();
    }

    QFont font;
    font.setBold(true);
    m_display_name_label->setFont(font);

    wrapData(m_display_name_label, displayName);
    m_form_label_map[m_display_name_label] = displayName;

    wrapData(m_location_label, location);
    m_form_label_map[m_location_label] = location;

    wrapData(m_type_label, info->fileType());
    m_form_label_map[m_type_label] = info->fileType();

    quint64 timeNum = FileUtils::getCreateTimeOfMicro (info->uri());
    quint64 modifiedTime = info->modifiedTime();
    // 客户需要必须显示创建时间，因此使用时间最小时间戳为创建时间
    quint64 minTime = timeNum != 0 ? timeNum : modifiedTime;
    // minTime = qMin (minTime, modifiedTime);
    // if (info->accessTime() != 0) {
    //     minTime = qMin (minTime, info->accessTime());
    // }
    QDateTime createDate = QDateTime::fromMSecsSinceEpoch(minTime*1000);
    QString createTime = createDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
    QString accessDate = info->accessDate();
    QString modifyDate = info->modifiedDate();
#ifdef KY_SDK_DATE
    createTime = GlobalSettings::getInstance()->transToSystemTimeFormat(minTime, true);
    accessDate = GlobalSettings::getInstance()->transToSystemTimeFormat(info->accessTime(), true);
    modifyDate = GlobalSettings::getInstance()->transToSystemTimeFormat(info->modifiedTime(), true);
#endif

    wrapData(m_time_create_label, createTime);
    m_form_label_map[m_time_create_label] = createTime;

    wrapData(m_time_access_label, accessDate);
    m_form_label_map[m_time_access_label] = accessDate;

    wrapData(m_time_modified_label, modifyDate);
    m_form_label_map[m_time_modified_label] = modifyDate;

    m_file_count_label->setText(tr(""));
    m_form_label_map[m_file_count_label] = "";

    m_currentFileUri = info->uri();
    QUrl pathUrl = m_currentFileUri;
    if (!m_currentFileUri.startsWith("file:///")) {
        m_currentFileUri = info->targetUri();
        pathUrl = info->filePath();
        if (pathUrl.isEmpty()) {
            pathUrl = FileUtils::getTargetUri(info->uri());
        }
        if (m_currentFileUri.isEmpty()) {
            m_currentFileUri = "file://" + info->filePath();
        }
        qDebug() << __func__ << __LINE__ << "target uri:" << pathUrl.path() << "m_currentFileUri:" << m_currentFileUri;
    }

    m_intelligentLabelLayout->clear();
    m_lableContainer->setVisible(false);
    m_file_summery_label->setVisible(false);

    QString path = pathUrl.path();
    fetchLabelsAndSummaryAsync(path);

    if (info->isDir()) {
        m_isDir = true;
        m_form->labelForField(m_file_count_label)->setVisible(true);
        m_file_count_label->setVisible(true);
        m_form->itemAt(14, QFormLayout::SpanningRole)->widget()->setVisible(true);
    } else {
        m_isDir = false;
        m_form->labelForField(m_file_count_label)->setVisible(false);
        m_file_count_label->setVisible(false);
        m_form->itemAt(14, QFormLayout::SpanningRole)->widget()->setVisible(false);
    }

    if (info->mimeType().startsWith("image/")) {
        QImageReader r;
        if (info->uri().startsWith("idm://")) {
            QString targetUri = info->targetUri();
            r.setFileName(targetUri.remove("file://"));
        } else {
            r.setFileName(info->filePath());
        }

        auto image_size_row_left = m_form->labelForField(m_image_size);
        image_size_row_left->setVisible(true);

        m_image_size->setText(tr("%1x%2").arg(r.size().width()).arg(r.size().height()));
        m_form_label_map[m_image_size] = tr("%1x%2").arg(r.size().width()).arg(r.size().height());

        auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(info->uri());
        bool rgba = thumbnail.pixmap(r.size()).hasAlphaChannel();
        m_image_size->setVisible(true);

        auto image_format_row_left = m_form->labelForField(m_image_format);
        image_format_row_left->setVisible(true);
        m_image_format->setText(rgba? "RGBA": "RGB");
        m_form_label_map[m_image_format] = rgba? "RGBA": "RGB";
        m_image_format->setVisible(true);
        m_form->itemAt(16, QFormLayout::SpanningRole)->widget()->setVisible(true);
        m_form->itemAt(18, QFormLayout::SpanningRole)->widget()->setVisible(true);
    } else {
        auto image_size_row_left = m_form->labelForField(m_image_size);
        image_size_row_left->setVisible(false);
        m_image_size->setVisible(false);
        auto image_format_row_left = m_form->labelForField(m_image_format);
        image_format_row_left->setVisible(false);
        m_image_format->setVisible(false);
        m_form->itemAt(16, QFormLayout::SpanningRole)->widget()->setVisible(false);
        m_form->itemAt(18, QFormLayout::SpanningRole)->widget()->setVisible(false);
    }
    if (info->fileType().startsWith("video/")) {

    }
    if (info->fileType().startsWith("audio/")) {

    }

    if (info->isSymbolLink()&&!info->symlinkTarget().isEmpty()){
        countAsync("file:///" + info->symlinkTarget());
    } else {
        countAsync(info->uri());
    }
}

void FilePreviewPage::countAsync(const QString &uri)
{
    cancel();
    m_file_count = 0;
    m_hidden_count = 0;
    m_total_size = 0;
    QString tempStr;
    bool isIdm = false;
    if (QString(uri).startsWith("idm://")) {
        tempStr = Peony::FileUtils::getRealUriFromIDMUri(uri);
        isIdm = true;
    }
    QStringList uris;
    if(isIdm && !tempStr.isEmpty()){
        uris<<tempStr;
    }else{
        uris<<uri;
    }
    //FIXME: replace BLOCKING api in ui thread.
    auto info = isIdm ? FileInfo::fromUri(tempStr) : FileInfo::fromUri(uri);
    m_count_op = new FileCountOperation(uris, !info->isDir());
    connect(m_count_op, &FileOperation::operationStarted, this, &FilePreviewPage::resetCount, Qt::BlockingQueuedConnection);
    connect(m_count_op, &FileOperation::operationPreparedOne, this, &FilePreviewPage::onPreparedOne, Qt::BlockingQueuedConnection);
    connect(m_count_op, &FileCountOperation::countDone, this, &FilePreviewPage::onCountDone, Qt::BlockingQueuedConnection);
    QThreadPool::globalInstance()->start(m_count_op);
}

void FilePreviewPage::updateCount()
{
    wrapData(m_file_count_label, tr("%1 total, %2 hidden").arg(m_file_count).arg(m_hidden_count));
    m_form_label_map[m_file_count_label] = tr("%1 total, %2 hidden").arg(m_file_count).arg(m_hidden_count);

    auto format = g_format_size(m_total_size);
    QString fileSize(format);
    m_total_size_label->setText(fileSize);
    m_form_label_map[m_total_size_label] = fileSize;
    g_free(format);
    updateForm(this->size());
}

void FilePreviewPage::cancel()
{
    if (m_count_op) {
        m_count_op->blockSignals(true);
        m_count_op->cancel();
        onCountDone();
    }
    m_count_op = nullptr;
}

void FilePreviewPage::resizeIcon(QSize size)
{
    m_icon->setIconSize(size);
}

void FilePreviewPage::onLabelsAndSummaryFetched(const QStringList &labels, const QString &summary)
{
    updateLabelsAndSummary(labels, summary);
}

void FilePreviewPage::openInNewWindow(const QString &uri)
{
    FMWindowIface *window = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
    if (window) {
        auto windowIface = window->create(uri);
        auto newWindow = dynamic_cast<QWidget *>(windowIface);
        newWindow->setAttribute(Qt::WA_DeleteOnClose);
        newWindow->show();
    }
}

void FilePreviewPage::resetCount()
{
    m_file_count = 0;
    m_hidden_count = 0;
    m_total_size = 0;
    updateCount();
}

void FilePreviewPage::onCountDone()
{
    if (!m_count_op)
        return;
    m_count_op->getInfo(m_file_count, m_hidden_count, m_total_size);
    // Do not count the folder itself
    if (m_file_count > 0) {
        m_file_count--;
    }
    this->updateCount();
    m_count_op = nullptr;
    m_file_count = 0;
    m_hidden_count = 0;
    m_total_size = 0;
}

void FilePreviewPage::updateForm(QSize size)
{
    int labelWidth = 0;
    QMap<QLabel*, QString>::const_iterator i = m_form_label_map.constBegin();
    int iLongTextWidth = 0;
    while (i != m_form_label_map.constEnd()) {
        if (i.key() == m_display_name_label) {
            iLongTextWidth = qMax(fontMetrics().width(i.value()), iLongTextWidth);
            ++i;
            continue;
        }
        iLongTextWidth = qMax(fontMetrics().width(i.value()), iLongTextWidth);
        QLabel *label =  ((QLabel*)m_form->labelForField(i.key()));
        if(label == nullptr){
            ++i;
            continue;
        }
        QString text = label->text();
        labelWidth = qMax(fontMetrics().width(text), labelWidth);
        ++i;
    }
    i = m_form_label_map.constBegin();

    int fieldWidth = size.width() - 3*m_form->spacing() - labelWidth - 30;
    fieldWidth = qMin(fieldWidth,iLongTextWidth);

    while (i != m_form_label_map.constEnd()) {
        auto label = i.key();
        if (label) {
            if (fieldWidth == label->width()) {
                return;
            }
            if (m_display_name_label == label) {
                label->setFixedWidth(fieldWidth + labelWidth);
            } else if(m_file_summery_label == label){
                label->setFixedWidth(fieldWidth + labelWidth - 3);
            }else {
                label->setFixedWidth(fieldWidth);
            }
            wrapData(i.key(), i.value());
        }
        ++i;
    }
     m_intelligentLabelLayout->update(); // 显式更新布局
}

void FilePreviewPage::addSeparator()
{
    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    m_form->addRow(separator);
}

void FilePreviewPage::setLabelProperties(QLabel *label)
{
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setCursor(Qt::IBeamCursor);
}

void FilePreviewPage::setSpaceName(const QString &spaceName)
{
    m_spaceName = spaceName;
    m_removeLabel->setVisible(!m_spaceName.isEmpty());
}

void FilePreviewPage::fetchLabelsAndSummaryAsync(const QString &path)
{
    QPointer<FilePreviewPage> weakThis(this);
    QtConcurrent::run([weakThis, path]{
        if (!weakThis || weakThis->m_destroyed)
            return;

        QStringList labels;
        QString summary;
        bool foundInCache = false;
        {
            QMutexLocker locker(&m_cacheMutex);
            if (m_labelSummaryCache.contains(path)) {
                auto pair = m_labelSummaryCache.object(path);
                labels = pair->first;
                summary = pair->second;
                foundInCache = true;
            }
        }

        if (!foundInCache) {
            labels = IDMDBusManager::getInstance()->getFileLabels(path);
            summary = IDMDBusManager::getInstance()->getFileSummery(path);

            std::unique_ptr<QPair<QStringList, QString>> pair(new QPair<QStringList, QString>(labels, summary));
            {
                QMutexLocker locker(&m_cacheMutex);
                m_labelSummaryCache.insert(path, pair.release());
            }
        }

        QMetaObject::invokeMethod(qApp, [weakThis, labels, summary]{
            if (weakThis && !weakThis->m_destroyed) {
                weakThis->onLabelsAndSummaryFetched(labels, summary);
            }
        });
    });
}

void FilePreviewPage::updateLabelsAndSummary(const QStringList &labels, const QString &summary)
{
    if (labels.isEmpty() && summary.isEmpty()) {
        return;
    }

    // 更新标签
    int count = qMin(labels.size(), 3);
    m_intelligentLabelLayout->clear();
    for (int i = 0; i < count; i++) {
        auto labelBtn = new IntelligentLabelButton(labels[i]);
        labelBtn->setMaximumWidth(210);
        m_intelligentLabelLayout->addWidget(labelBtn);
    }
    m_lableContainer->setVisible(count > 0);

    // 更新摘要
    wrapData(m_file_summery_label, summary);
    m_form_label_map[m_file_summery_label] = summary;
    m_file_summery_label->setVisible(!summary.isEmpty());

    updateForm(size());
}

QLabel *FilePreviewPage::getLabel(const QString& text)
{
    QLabel *label = new QLabel(this);
    label->setAlignment(Qt::AlignTop);
    label->setText(text);
    return label;
}

labelFlowLayout::labelFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

labelFlowLayout::labelFlowLayout(int margin, int hSpacing, int vSpacing)
    : QLayout(nullptr), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

labelFlowLayout::~labelFlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void labelFlowLayout::addItem(QLayoutItem *item)
{
    itemList.append(item);
}

int labelFlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    }
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int labelFlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    }
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

Qt::Orientations labelFlowLayout::expandingDirections() const
{
    return Qt::Horizontal;
}

bool labelFlowLayout::hasHeightForWidth() const
{
    return true;
}

int labelFlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

int labelFlowLayout::count() const
{
    return itemList.size();
}

QLayoutItem *labelFlowLayout::itemAt(int index) const
{
    if (index >= 0 && index < itemList.size())
        return itemList.at(index);
    return nullptr;
}

QSize labelFlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem *item : qAsConst(itemList))
        size = size.expandedTo(item->minimumSize());

    int left = 0, top = 0, right = 0, bottom = 0;
    getContentsMargins(&left, &top, &right, &bottom);
    size += QSize(left + right, top + bottom);
    return size;
}

void labelFlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize labelFlowLayout::sizeHint() const
{
    int maxLineWidth = 0;
    int currentLineWidth = 0;
    int totalHeight = 0;
    int lineHeight = 0;

    for (QLayoutItem *item : itemList) {
        QSize itemSize = item->sizeHint();
        if (currentLineWidth + itemSize.width() > maxLineWidth) {
            currentLineWidth += itemSize.width() + horizontalSpacing();
        } else {
            maxLineWidth = qMax(maxLineWidth, currentLineWidth);
            currentLineWidth = itemSize.width() + horizontalSpacing();
            totalHeight += lineHeight + verticalSpacing();
            lineHeight = 0;
        }
        lineHeight = qMax(lineHeight, itemSize.height());
    }
    maxLineWidth = qMax(maxLineWidth, currentLineWidth - horizontalSpacing());
    totalHeight += lineHeight;

    // 添加边距
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    return QSize(maxLineWidth + left + right, totalHeight + top + bottom);
}
QLayoutItem *labelFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    return nullptr;
}

int labelFlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    // 关键调整1：减少右侧边距对可用宽度的影响
    int availableWidth = rect.width() - left - right + horizontalSpacing() - 8;

    int x = left;
    int y = top;
    int lineHeight = 0;
    bool isFirstInLine = true; // 标记行首元素

    for (QLayoutItem *item : itemList) {
        int itemWidth = item->sizeHint().width();
        int itemHeight = item->sizeHint().height();

        // 关键调整2：动态计算换行条件（行首元素不加间距）
        int requiredWidth = isFirstInLine ? itemWidth : (x + horizontalSpacing() + itemWidth);

        if (requiredWidth > availableWidth && !isFirstInLine) {
            x = left;
            y += lineHeight + verticalSpacing();
            lineHeight = 0;
            isFirstInLine = true;
        }

        // 关键调整3：行首元素不添加左边距
        int itemX = isFirstInLine ? x : (x + horizontalSpacing());
        if (!testOnly) {
            item->setGeometry(QRect(QPoint(itemX, y), QSize(itemWidth, itemHeight)));
        }

        x = itemX + itemWidth;
        lineHeight = qMax(lineHeight, itemHeight);
        isFirstInLine = false;
    }

    // 关键调整4：返回高度时补偿最后一行间距
    return y + lineHeight + bottom + verticalSpacing();
}

int labelFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

void labelFlowLayout::clear() {
    QLayoutItem *item;
    while ((item = this->takeAt(0)) != nullptr) {
        if (QWidget *widget = item->widget()) {
            delete widget;
        }
        delete item;
    }
}
