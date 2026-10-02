/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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

#include "basic-properties-page.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFormLayout>
#include <fcntl.h>
#include <sys/types.h>
#include <glib/gstdio.h>
#include <gio/gdesktopappinfo.h>

#include <QDateTime>
#include <QThreadPool>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <file-launch-manager.h>
#include <global-settings.h>

#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

#include <QFileDialog>
#include <QtConcurrent>
#include <QGSettings>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>

#include "file-info.h"
#include "file-meta-info.h"
#include "file-info-job.h"
#include "file-utils.h"
#include "thumbnail-manager.h"
#include "file-operation-utils.h"
#include "file-watcher.h"
#include "file-count-operation.h"
#include "file-operation-manager.h"
#include "file-meta-info.h"
#include "global-settings.h"
#include "generic-thumbnailer.h"
#include "file-operation-manager.h"
#include "open-with-properties-page.h"
#include "file-enumerator.h"
#include "file-properties-operation.h"
#include "linux-pwd-helper.h"

#include <QApplication>

#define DEBUG qDebug() << "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"

using namespace Peony;

static PushButtonStyle *global_instance = nullptr;

PushButtonStyle *PushButtonStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new PushButtonStyle;
    }
    return global_instance;
}

void PushButtonStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, option, painter, widget);
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, option, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);

            return;
        }
        break;
    }

    default:
        break;
    }
    qApp->style()->drawControl(element, option, painter, widget);
}

int PushButtonStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ButtonMargin:
    {
        return 0;
    }
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QRect PushButtonStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_PushButtonContents:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isEmpty();
            QRect rect = option->rect;
            int Margin_Height = 2;
            int ToolButton_MarginWidth = 10;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                rect.adjust(Button_MarginWidth, 0, -Button_MarginWidth, 0);
            } else if (!text && icon && !(button->features & QStyleOptionButton::HasMenu)) {

            } else {
                rect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
            }
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                rect.adjust(dbw, dbw, -dbw, -dbw);
            }
            return rect;
        }
        break;
    }

    default:
        break;
    }

    return QProxyStyle::subElementRect(element, option, widget);
}

BasicPropertiesPage::BasicPropertiesPage(const QStringList &uris, QWidget *parent) : PropertiesWindowTabIface(parent)
{
    m_uris = uris;
    m_timer = new QTimer(this);
    m_timer->setInterval(750);
    connect(m_timer, &QTimer::timeout, this, [=]{
        updateCountInfo(true);
    });
    m_fileType = checkFileType(m_uris);
    initUI();
    // m_futureWatcher = new QFutureWatcher<void>;
    // connect(m_futureWatcher, &QFutureWatcher<void>::finished, this, &BasicPropertiesPage::initUI);

    // m_futureWatcher->setFuture(
    //         QtConcurrent::run([=]() {
    //             m_fileType = checkFileType(m_uris);
    //         })
    // );
}

void BasicPropertiesPage::initUI()
{
    if (m_futureWatcher) {
        delete m_futureWatcher;
        m_futureWatcher = nullptr;
    }

    this->setContentsMargins(0, 0, 0, 0);
    //主布局
    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    this->initFloorOne();
    this->addSeparator();
    this->initFloorTwo();

    m_layout->addStretch(1);

    this->loadData();
}

void BasicPropertiesPage::loadData()
{
    if (m_fileType != BP_MultipleFIle) {
        m_watcher = std::make_shared<FileWatcher>(m_uris.first());
        m_watcher->connect(m_watcher.get(), &FileWatcher::locationChanged, this, &BasicPropertiesPage::onSingleFileChanged);
        m_watcher->startMonitor();

        m_thumbnail_watcher = std::make_shared<FileWatcher>("thumbnail:///");
        connect(m_thumbnail_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri){
            if (!m_info || uri != m_info->uri())
                return;
            auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(uri);
            QIcon icon = QIcon::fromTheme(m_info->iconName(), QIcon::fromTheme("unknown"));
            if (!thumbnail.isNull() && !m_info->customIcon().isEmpty())
                icon = thumbnail;
            // 缩略图尚未生成时勿用空 QIcon 覆盖（否则 .desktop 等会闪成空白或非缩略图样式）
            m_iconButton->setIcon(thumbnail.isNull() ? icon : thumbnail);
        });
    }

    this->loadPartOne();
    this->loadPartTwo();
    this->loadOptionalData();
}

void BasicPropertiesPage::initFloorOne()
{
    QFrame      *baseFrame  = new QFrame(this);
    QHBoxLayout *baseLayout = new QHBoxLayout(baseFrame);
    QGridLayout *gridLayout = new QGridLayout;
    QHBoxLayout *locationLayout = new QHBoxLayout;
    QVBoxLayout *iconLayout = new QVBoxLayout;
    locationLayout->setContentsMargins(0, 0, 0, 0);
    locationLayout->setSpacing(0);
    iconLayout->setSpacing(0);
    iconLayout->setContentsMargins(0, 0, 0, 0);

    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setVerticalSpacing(8);
    gridLayout->setHorizontalSpacing(8);

    baseLayout->setContentsMargins(24, 24, 24, 16);
    baseLayout->setAlignment(Qt::AlignVCenter);

    baseFrame->setLayout(baseLayout);

    m_iconButton      = new QPushButton(baseFrame);
    m_displayNameEdit = new QLineEdit(baseFrame);
    m_locationEdit    = new QLineEdit(baseFrame);
    m_changeIconButton = new QPushButton(tr("Change"), baseFrame);
    m_moveButton = new QPushButton(tr("Move"), baseFrame);
    m_displayNameLabel = new QLabel(tr("Name"), baseFrame);
    m_locationLabel = new QLabel(tr("Location"), baseFrame);

    m_iconButton->setFixedSize(QSize(72, 72));
    m_iconButton->setIconSize(QSize(48, 48));
    m_iconButton->setProperty("isIcon", true);

    m_displayNameEdit->setMinimumHeight(36);
    m_locationEdit->setMinimumHeight(36);
    m_locationEdit->setReadOnly(true);

    m_moveButton->setFixedWidth(70);

    gridLayout->addWidget(m_displayNameLabel, 0, 0, 1, 1);
    gridLayout->addWidget(m_displayNameEdit, 0, 1, 1, 7);

    gridLayout->addWidget(m_locationLabel, 1, 0, 1, 1);
    gridLayout->addWidget(m_locationEdit, 1, 1, 1, 7);
    gridLayout->addWidget(m_moveButton, 1, 8, 1, 1);

    if (m_fileType == BP_Folder) {
        iconLayout->addSpacerItem(new QSpacerItem(0, -28, QSizePolicy::Minimum, QSizePolicy::Fixed));
    }

    iconLayout->addWidget(m_iconButton, 0, Qt::AlignCenter);
    iconLayout->addWidget(m_changeIconButton, 1, Qt::AlignCenter);

    baseLayout->setSpacing(16);
    baseLayout->addLayout(iconLayout, 0);   // stretch=0：图标列只占内容宽度，不参与多余空间分配
    baseLayout->addLayout(gridLayout, 1);  // stretch=1：右侧表格占据剩余水平空间
    m_layout->addWidget(baseFrame);
}

void BasicPropertiesPage::initFloorTwo()
{
    QFrame      *baseFrame  = new QFrame(this);
    QFormLayout *baseLayout = new QFormLayout(baseFrame);

    baseLayout->setObjectName("floorTwoBaseLayout");
    baseLayout->setVerticalSpacing(16);
    baseLayout->setHorizontalSpacing(10);
    baseLayout->setContentsMargins(24, 16, 24, 16);

    baseFrame->setLayout(baseLayout);

    m_fileTypeLabel      = new QLabel(baseFrame);
    m_fileSizeLabel      = new QLabel(baseFrame);
    m_fileTotalSizeLabel = new QLabel(baseFrame);

    m_fileSizeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_fileTotalSizeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    //设置了TextInteractionFlags，内部不会触发高度适配，手动设置高度
    m_fileSizeLabel->setFixedHeight(m_fileSizeLabel->sizeHint().height());
    m_fileTotalSizeLabel->setFixedHeight(m_fileTotalSizeLabel->sizeHint().height());

    baseLayout->addRow(tr("Type:"), m_fileTypeLabel);

    //根据文件类型添加组件 - Add components based on file type
    switch (m_fileType) {
        case BP_Folder:
            m_folderContainLabel = new QLabel(baseFrame);
            m_folderContainLabel->setContentsMargins(0, 0, 0, 0);
            baseLayout->addRow(tr("Include:"), m_folderContainLabel);
            break;
        case BP_File:
            m_openWithLayout = new QHBoxLayout(baseFrame);
            baseLayout->addRow(tr("Open with:"), m_openWithLayout);
            break;
        case BP_Application:
            m_descrptionLabel = new QLabel(baseFrame);
            baseLayout->addRow(tr("Description:"), m_descrptionLabel);
            break;
        case BP_MultipleFIle:
            m_fileTypeLabel->setText(tr("Select multiple files"));
        default:
            break;
    }

    baseLayout->addRow(tr("Size:"), m_fileSizeLabel);
    baseLayout->addRow(tr("Space Usage:"), m_fileTotalSizeLabel);

    if(m_fileType != BP_MultipleFIle ) {
        //===创建时间等===
        switch (m_fileType) {
            case BP_File:
            case BP_Application:
            case BP_Folder:
                m_timeModifiedLabel = new QLabel(baseFrame);
                m_timeAccessLabel = new QLabel(baseFrame);
                m_timeCreatedLabel = new QLabel(baseFrame);
                baseLayout->addRow(tr("Time Create:"), m_timeCreatedLabel);
                baseLayout->addRow(tr("Time Modified:"), m_timeModifiedLabel);
                baseLayout->addRow(tr("Time Access:"), m_timeAccessLabel);
                break;
            case BP_MultipleFIle:
                m_timeModifiedLabel = new QLabel(baseFrame);
                m_timeCreatedLabel = new QLabel(baseFrame);
                baseLayout->addRow(tr("Time Create:"), m_timeCreatedLabel);
                baseLayout->addRow(tr("Time Modified:"), m_timeModifiedLabel);
            default:
                break;
        }
    }

    //===只读和隐藏选择框===
    m_readOnly = new QCheckBox(tr("Readonly"), baseFrame);
    m_hidden = new QCheckBox(tr("Hidden"), baseFrame);
    m_executable = new QCheckBox(tr("Executable"), baseFrame);

    if (m_info->isDir() && m_uris.count() == 1) {
        QString readOnlyText = tr("Readonly (just applied by subfiles)");
        QString elideText = BasicPropertiesPage::elideTextAndToolTip(m_readOnly->font(), 150, readOnlyText, Qt::ElideRight, m_readOnly);
        m_readOnly->setText(elideText);
    }

    QHBoxLayout *checkboxLayout = new QHBoxLayout(baseFrame);
    checkboxLayout->addSpacing(1);
    checkboxLayout->addWidget(m_readOnly, Qt::AlignLeft);
    checkboxLayout->addSpacing(35);
    checkboxLayout->addWidget(m_hidden, Qt::AlignLeft);
    checkboxLayout->addSpacing(35);
    checkboxLayout->addWidget(m_executable, Qt::AlignLeft);
    checkboxLayout->addStretch(1);

    baseLayout->addRow(tr("Property:"), checkboxLayout);

    m_layout->addWidget(baseFrame);
}

QString BasicPropertiesPage::chooseCustomIconFile(QWidget *widget)
{
    QUrl iconPathUrl;
    iconPathUrl.setPath("/usr/share/icons");
    QString picture;
    QFileDialog fileDialog(widget, tr("Choose a custom icon"));
    fileDialog.setDirectory("/usr/share/icons");
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("Images (*.png *.jpg *.jpeg *.svg)"));
    fileDialog.selectNameFilter(tr("Images (*.png *.jpg *.jpeg *.svg)"));
    fileDialog.setWindowModality(Qt::WindowModal);
    if (fileDialog.exec() == QDialog::Accepted) {
        picture = fileDialog.selectedFiles().first();
        QFileInfo fileInfo(picture);
        if (fileInfo.exists()) {
            qint64 fileSize = fileInfo.size();
            double fileSizeMB = fileSize / (1024.0 * 1024.0);
            qDebug() << "fileSize is ：" << fileSizeMB << "MB";
            if (fileSizeMB > 1.0) {
                picture.clear();
                QMessageBox::warning(nullptr, "", tr("Please select a image that is smaller than 1MB."));
            }
        } else {
            qDebug() << "file does not exist ：" << picture;
        }
    }
    return picture;
}

BasicPropertiesPage::~BasicPropertiesPage()
{
    disconnect();
    cancelCount();
}

void BasicPropertiesPage::addSeparator()
{
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    m_layout->addWidget(separator);
}

void BasicPropertiesPage::addOpenWithLayout(QWidget *parent)
{
    if (m_openWithLayout) {
        m_openWithLayout->setContentsMargins(0,0,0,0);
        m_openWithLayout->setAlignment(Qt::AlignVCenter);
        //FIXME:属性窗口重构，修改打开方式为下拉框
        m_moreAppBox = new QComboBox;
        m_moreAppBox->setFixedHeight(36);
        auto allLaunchActions = FileLaunchManager::getRecommendActions(m_info.get()->uri());
        auto defaultAction = FileLaunchManager::getDefaultAction(m_info.get()->uri());
        int currentIndex = 0;
        for (auto action : allLaunchActions)
        {
            action->setParent(m_moreAppBox);
            m_moreAppBox->addItem(!action->icon().isNull()? action->icon(): QIcon::fromTheme("application-x-desktop"),
                                action->text());
            if (defaultAction->getAppInfoName() == action->getAppInfoName()) {
                currentIndex = allLaunchActions.indexOf(action);
            }
        }
        delete defaultAction;
        m_moreAppBox->setCurrentIndex(currentIndex);
        connect(m_moreAppBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
            OpenWithPropertiesPage::setNewLaunchAction(allLaunchActions.at(index), false);
        });
        m_openWithLayout->addWidget(m_moreAppBox);
    }
}

void BasicPropertiesPage::loadPartOne()
{
    //icon area
    if (m_fileType != BP_MultipleFIle) {
        if (m_fileType == BP_Folder) {
            m_iconButton->setEnabled(true);
            m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            m_iconButton->setFlat(true);
            m_iconButton->setFocusPolicy(Qt::NoFocus);
            m_iconButton->setCursor(Qt::ArrowCursor);
            m_iconButton->setStyleSheet(R"(
                                        QPushButton {
                                            border: none;
                                            background: transparent;
                                            padding: 0px;
                                        }
                                        QPushButton:hover,
                                        QPushButton:pressed {
                                            background: transparent;
                                            border: none;
                                        }
                                        )");
            connect(m_changeIconButton, &QPushButton::clicked, this, &BasicPropertiesPage::chooseFileIcon);
        } else {
            m_changeIconButton->hide();
            connect(m_iconButton, &QPushButton::clicked, this, &BasicPropertiesPage::chooseFileIcon);
        }
        this->onSingleFileChanged(nullptr, m_uris.first());
    } else {
        m_iconButton->setIcon(QIcon::fromTheme("unknown"));
        m_changeIconButton->hide();
    }

    //select multiplefiles
    if (m_fileType == BP_MultipleFIle || !m_info->canRename())
        m_displayNameEdit->setReadOnly(true);
    //选中多个文件时，使用另外的线程获取文件名称并拼接
    if (m_fileType == BP_MultipleFIle) {
        FileNameThread *getNameThread = new FileNameThread(m_uris);
        connect(getNameThread, &FileNameThread::fileNameReady, this, [=](QString fileName) {
            m_displayNameEdit->setToolTip(fileName);
            fileName = BasicPropertiesPage::elideText(m_displayNameEdit->font(),260,fileName);
            m_displayNameEdit->setText(fileName);
            m_displayNameEdit->setToolTip(fileName);
        });
        connect(getNameThread, &QThread::finished, getNameThread, &QObject::deleteLater);
        getNameThread->start();
    }

    connect(m_displayNameEdit, &QLineEdit::textChanged, [=]() {
        if (isNameChanged()) {
            this->thisPageChanged();
        }
    });

    QUrl url = FileUtils::getParentUri(m_uris.first());
    QString location = url.toDisplayString();
    if (location.startsWith("file://"))
        location = location.split("file://").last();
    m_locationEdit->setText(location);

    if(m_fileType == BP_Folder) {
        //home目录不支持移动和重命名
        //不能改名的文件自然不能移动  标准位置文件夹也不能移动
        if(m_info.get()->uri() == ("file://"+QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first())
            || !m_info->canRename()
            || FileUtils::isStandardPath(m_info->uri())
            || FileUtils::isFuseFileSystemForUri(m_info->uri())
            || FileUtils::isSymbolLinkAndTargetDataPath(m_info->uri())) {
            m_displayNameEdit->setReadOnly(true);
            m_moveButton->setDisabled(true);
        }
        connect(m_moveButton, &QPushButton::clicked, this, &BasicPropertiesPage::moveFile);
    } else {
        m_moveButton->setVisible(false);
    }

    //fix bug#79649,侧边栏保护箱属性的“位置”置灰
    if(m_uris.first() == "filesafe:///" || FileUtils::isSymbolLinkAndTargetDataPath(m_info->uri())) {
        m_locationEdit->setEnabled(false);
    }

    QString fileUri = m_uris.at(0);
    //filesafe插件中保护箱下目录不能移动——fix bug 76864
    //fix show properties in filesafe path crash issue, link to bug#74350
    if(fileUri.startsWith("filesafe:///") && m_moveButton) {
        m_moveButton->setVisible(false);
    }

    if(fileUri.startsWith("filesafe:///") && (fileUri.remove("filesafe:///").indexOf("/") == -1)) {
        disconnect(m_iconButton, &QPushButton::clicked, this, &BasicPropertiesPage::chooseFileIcon);
        m_displayNameEdit->setReadOnly(true);
        m_locationEdit->setDisabled(true);
    }

    if (fileUri == ("file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation))) {
        disconnect(m_iconButton, &QPushButton::clicked, this, &BasicPropertiesPage::chooseFileIcon);
        m_iconButton->setFocusPolicy(Qt::NoFocus);
        m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

void BasicPropertiesPage::loadPartTwo()
{
    if(m_fileType != BP_MultipleFIle) {
        //FIX:目前只是在这里对文件快捷方式进行判断并显示，这并不合适，希望在底层文件fileType直接增加快捷方式。
        //FIX:At present, only the file shortcuts are judged and displayed here. This is not appropriate.
        //I hope to directly add shortcuts to the underlying file file Type.
        m_fileTypeLabel->setText(m_info->isSymbolLink() ? tr("symbolLink") : m_info.get()->fileType());
    }

    if(m_fileType == BP_Folder && m_uris.first() == "filesafe:///"){
        m_fileTypeLabel->setText(tr("Folder"));
    }

    //根据文件类型添加组件 - Add components based on file type
    switch (m_fileType) {
    case BP_Folder:
        break;
    case BP_File:
        this->addOpenWithLayout(this);
        break;
    case BP_Application:
    {
        //fix bug#53504, not show duplicated name issue
        QString displayName = m_info.get()->displayName();
        if (m_info->isDesktopFile())
        {
            displayName = FileUtils::handleDesktopFileName(m_info->uri(), displayName);
        }
        m_descrptionLabel->setText(displayName);
    }

        break;
    case BP_MultipleFIle:
    default:
        break;
    }

    this->queryUserPermission(m_info->uri());
    this->countFilesAsync(m_uris);
}

void BasicPropertiesPage::loadOptionalData()
{
    if (m_fileType != BP_MultipleFIle) {
        this->setSysTimeFormat();
        // set time
        if (QGSettings::isSchemaInstalled("org.ukui.control-center.panel.plugins")) {
            QGSettings* settings = new QGSettings("org.ukui.control-center.panel.plugins", QByteArray(), this);
            connect(settings, &QGSettings::changed, this, [=](const QString &key) {
                if ("hoursystem" == key) {
                    setSysTimeFormat();
                    updateInfo(m_info->uri());
                }
            });
        }

#ifdef KY_SDK_DATE
        connect(GlobalSettings::getInstance(),
                &GlobalSettings::updateLongDataFormat,
                this,
                &BasicPropertiesPage::updateDateFormat);
        //当gsetting的相对日期开启会用日期短格式，所以要监听对应信号
        connect(GlobalSettings::getInstance(),
                &GlobalSettings::updateShortDataFormat,
                this,
                &BasicPropertiesPage::updateDateFormat);
#endif

        updateInfo(m_info.get()->uri());
        connect(m_watcher.get(), &FileWatcher::locationChanged, [=](const QString&, const QString &uri) {
            this->updateInfo(uri);
        });
    }

    if (m_fileType != BP_MultipleFIle && m_fileType != BP_Folder) {
        bool isHidden = m_info.get()->property(G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN).toBool();
        if(m_info.get()->canRead() && !m_info.get()->canWrite())
            m_readOnly->setCheckState(Qt::Checked);

        if(m_info.get()->displayName().startsWith(".") || isHidden)
            m_hidden->setCheckState(Qt::Checked);

        m_readOnly->setDisabled(!m_hasPermission);

        QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        bool isDesktop = FileUtils::isSamePath(m_info->uri(), desktopPath);
        bool isIDM = m_info->uri().startsWith("idm://");
        //fix bug#113890,hiden Desktop folder change desktop show
        m_hidden->setDisabled(!m_info->canRename() || isDesktop || isIDM);
        m_isReadOnly = m_readOnly->isChecked();

        if (FileUtils::isExecutableTypeFile(m_info->uri())) {
            m_executable->setVisible(true);
            if (m_info->canExecute()) {
                m_executable->setChecked(true);
            } else {
                m_executable->setChecked(false);
            }
            m_executableState = m_executable->checkState();
        } else {
            m_executable->setVisible(false);
        }
    } else {
        BatchStatusThread *batchStatusThread = new BatchStatusThread(m_uris);
        batchStatusThread->start();
        connect(batchStatusThread, &BatchStatusThread::updateState, this, [=](Qt::CheckState readOnlyState, Qt::CheckState hiddenState){
            if(m_fileType == BP_Folder){/* 单选一个文件夹的场景，参照windows 只读复选框显示为部分勾选状态；link to bug#265731 一级目录设置只读后再次打开属性界面，只读为未勾选状态  */
                readOnlyState = Qt::CheckState::PartiallyChecked;
            }//end
            m_readOnly->setCheckState(readOnlyState);
            m_hidden->setCheckState(hiddenState);
            m_readOnlyState = readOnlyState;
            m_hiddenState = hiddenState;
        });
        connect(batchStatusThread, &BatchStatusThread::updateDisabled, this, [=](const bool &readOnlyDisable, const bool &hiddenDisable){
            //m_readOnly->setDisabled(readOnlyDisable);
            Q_UNUSED(readOnlyDisable);
            m_readOnly->setDisabled(!m_hasPermission);
            m_hidden->setDisabled(hiddenDisable);
        });
        connect(batchStatusThread, &BatchStatusThread::updateIsAllDir, this, [=](const bool &isAllDir){
            m_isAllDir = isAllDir;
        });
        connect(batchStatusThread, &BatchStatusThread::finished, batchStatusThread, &BatchStatusThread::deleteLater);
        if (m_executable) {
            m_executable->setVisible(false);
        }
    }


    //确认被修改
    connect(m_readOnly, &QCheckBox::stateChanged, this, &BasicPropertiesPage::thisPageChanged);
    connect(m_hidden, &QCheckBox::stateChanged, this, &BasicPropertiesPage::thisPageChanged);
    connect(m_executable, &QCheckBox::stateChanged, this, &BasicPropertiesPage::thisPageChanged);
}

BasicPropertiesPage::FileType BasicPropertiesPage::checkFileType(const QStringList &uris)
{
    getFIleInfo(m_uris.first());

    if(m_uris.count() != 1) {
        return BP_MultipleFIle;
    } else {
        // if (m_info->displayName().isNull() || m_info->displayName().isEmpty()) {
        //     this->getFIleInfo(m_uris.first());
        // }

        if(m_info.get()->isDir())
            return BP_Folder;
        if(m_info.get()->isDesktopFile())
            return BP_Application;
        return BP_File;
    }
}

//异步获取数据
void BasicPropertiesPage::getFIleInfo(QString uri)
{
    //将在收藏夹的文件路径替换为真实路径 - Replace the file path in the favorites with the real path
    //FIX:如果是远程文件夹或者其它非本地文件夹添加到收藏夹呢？ - What if remote folders or other non-local folders are added to favorites?
    if (uri.startsWith("favorite://")) {
        QUrl url(uri);
        uri = "file://" + url.path();
        m_uris.clear();
        m_uris.append(uri);
    }

    std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(uri);
    FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
    fileInfoJob->setAutoDelete();
    fileInfoJob->querySync();
    m_info = fileInfo;
}

QIcon generateThumbnail(const QString &uri)
{
    QUrl url = uri;
    if (! uri.startsWith("file:///")) {
        url = FileUtils::getTargetUri(uri);
    }

    auto _desktop_file = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
    auto _icon_string = g_desktop_app_info_get_string(_desktop_file, "Icon");
    QIcon thumbnail = QIcon::fromTheme(_icon_string);
    QString string = _icon_string;

    //fix desktop file set customer icon issue, link to bug#77638
    auto info = FileInfo::fromUri(uri);
    if (! info->customIcon().isEmpty()){
        thumbnail = GenericThumbnailer::generateThumbnail(info->customIcon(), true);
    }

    if (thumbnail.isNull() && string.startsWith("/")) {
        thumbnail = GenericThumbnailer::generateThumbnail(_icon_string, true);
    }
    DEBUG << "thumbnail" << thumbnail <<_icon_string;
    g_free(_icon_string);
    g_object_unref(_desktop_file);

    return thumbnail;
}

void BasicPropertiesPage::onSingleFileChanged(const QString &oldUri, const QString &newUri)
{
    this->getFIleInfo(newUri);

    ThumbnailManager::getInstance()->createThumbnail(m_info.get()->uri(), m_thumbnail_watcher);
    auto icon = QIcon::fromTheme(m_info.get()->iconName(), QIcon::fromTheme("unknown"));
    // 无自定义图标时优先使用主题图标，避免使用到还原后尚未失效的缩略图缓存。
    // .desktop 无 peony 自定义图标路径时仍应显示解析后的启动图标：仅用 iconName 会得到 application-x-desktop，
    // 且在缩略图异步就绪前后 locationChanged 再次触发时，若此处不取缓存/同步解析则会错误覆盖为通用 MIME 图标。
    QIcon thumbnail;
    if (!m_info->customIcon().isEmpty()) {
        thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(m_info.get()->uri());
    } else if (m_info->isDesktopFile()) {
        thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(m_info.get()->uri());
        if (thumbnail.isNull()) {
            thumbnail = generateThumbnail(m_info->uri());
        }
    }

    m_iconButton->setIcon(thumbnail.isNull() ? icon : thumbnail);
    //fix bug#53504, not show duplicated name issue.
    QString fileName = m_info->displayName();
    if (m_info->isDesktopFile() && !fileName.endsWith(".desktop")) {
        fileName = FileUtils::handleDesktopFileName(m_info->uri(), fileName);
    }
    //fix bug:#82320
    if (QRegExp("^file:///data/usershare(/{,1})$").exactMatch(m_info->uri())) {
        fileName = tr("usershare");
    }

    m_displayNameEdit->setText(fileName);
    m_displayNameEdit->setToolTip(fileName);

    if (thumbnail.isNull()) {
        ThumbnailManager::getInstance()->createThumbnail(m_info.get()->uri(), m_thumbnail_watcher);
    }

    // 文件夹已设置自定义图标时，同步到 m_newFileIconPath，以便修改图标对话框中保留当前图标
    if (checkFileType(m_uris) == BP_Folder && !m_info->customIcon().isEmpty()) {
        m_newFileIconPath = m_info->customIcon();
    }
    QUrl url = FileUtils::getParentUri(m_info.get()->uri());

    QString location = url.toDisplayString();
    if (location.startsWith("file://"))
        location = location.split("file://").last();
    m_locationEdit->setText(location);
}

void BasicPropertiesPage::countFilesAsync(const QStringList &uris)
{
    m_timer->start();

    //old op will delete later
    if (m_countOp) {
        m_countOp->disconnect();
        m_countOp->cancel();
    }
    //clear old data
    m_folderContainFiles   = 0;
    m_folderContainFolders = 0;
    m_fileSizeCount        = 0;
    m_fileTotalSizeCount   = 0;

    if (uris.count() == 1) {
        auto uri = uris.first();
        auto info = FileInfo::fromUri(uri);

        QStringList realUris;
        //判断是不是快捷方式 - Determine if it is a shortcut
        if (info->isSymbolLink()) {
            if (!info.get()->symlinkTarget().isEmpty())
                realUris << ("file://" + info.get()->symlinkTarget());
        }
        m_countOp = new FileCountOperation(realUris.isEmpty() ? uris : realUris);
        DEBUG << "symlinkTarget:" << info.get()->symlinkTarget();
    } else {
        QStringList lists;
        for (auto uri : uris) {
            auto info = FileInfo::fromUri(uri);
            if (info->isSymbolLink()) {
                if (!info.get()->symlinkTarget().isEmpty()) {
                    QString temp = "file://" + info.get()->symlinkTarget();
                    lists.append(temp);
                } else {
                    lists.append(uri);
                }
            } else {
                lists.append(uri);
            }
        }
        m_countOp = new FileCountOperation(lists);
    }

    m_countOp->setAutoDelete(true);

    connect(m_countOp, &FileOperation::operationPreparedOne, this, &BasicPropertiesPage::onFileCountOne, Qt::BlockingQueuedConnection);

    connect(m_countOp, &FileCountOperation::countDone, this, [=](quint64 file_count, quint64 hidden_file_count, quint64 total_size) {
        m_timer->stop();

        m_countOp = nullptr;
        m_folderContainFiles = file_count - m_folderContainFolders;
        m_fileSizeCount = total_size;
        //不统计文件夹本身 - Do not count the folder itself
        if (m_folderContainFolders != 0) {
            m_folderContainFolders--;
        }
        this->updateCountInfo(true);

        //使用du -s 命令查看文件实际占用的磁盘空间。
        for (QString uri : m_uris) {
            if (uri.startsWith("kmre://")) {
                return;
            }
            QUrl url(uri);
            if (url.isLocalFile()) {
                std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(uri);
                FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
                fileInfoJob->setAutoDelete();
                fileInfoJob->querySync();
                //某些带空格的文件名称会导致命令错误，加上引号解决此问题。
                QString path;
                if(uri == "filesafe:///") {
                    path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.box";
                } else {
    //                g_autoptr (GFile) gfile = g_file_new_for_uri(uri.toUtf8().constData());
    //                g_autofree gchar *gpath = g_file_get_path(gfile);
    //                if (gpath) {
    //                    path = gpath;
    //                } else {
                        path = QString("%1%2%3").arg("\"").arg(url.path()).arg("\"");
    //                }
                }


                QProcess process;
                if (fileInfo->isSymbolLink()) {
                    process.start("/usr/bin/du -L " + path);
                } else {
                    process.start("/usr/bin/du -s " + path);
                }
                process.waitForFinished();
                QString result = process.readAllStandardOutput();
                //du -s xxx 输出格式：4	xxx  (大小单位为KB)
                m_fileTotalSizeCount += result.split(QRegExp("\\s+")).first().toLong();
            }
        }
        //转换为 xx Bytes
        m_fileTotalSizeCount *= CELL1K;

        if (m_fileTotalSizeCount == 0) {
            quint64 a = 0;
            quint64 b = 0;
            a = m_fileSizeCount % CELL4K;
            b = m_fileSizeCount / CELL4K;
            quint64 cell4k = (a == 0) ? b : (b + 1);
            m_fileTotalSizeCount = cell4k * CELL4K;
        }
        //gio格式化工具
        char *fileTotalSizeFormat = g_format_size(m_fileTotalSizeCount);
        QString fileTotalSizeFormatString(fileTotalSizeFormat);

        QString fileTotalSizeText(tr("%1 (%2 Bytes)").arg(fileTotalSizeFormatString).arg(m_fileTotalSizeCount));
        g_free(fileTotalSizeFormat);

        fileTotalSizeText = BasicPropertiesPage::elideTextAndToolTip(m_fileTotalSizeLabel->font(), 240, fileTotalSizeText, Qt::ElideMiddle, m_fileTotalSizeLabel);
        m_fileTotalSizeLabel->setText(fileTotalSizeText);
    });

    QThreadPool::globalInstance()->start(m_countOp);
}

void BasicPropertiesPage::onFileCountOne(const QString &uri, quint64 size)
{
    //...FIX:第一版本在当前位置对文件夹进行统计,慢
//    std::shared_ptr<FileInfo> l_fileInfo = FileInfo::fromUri(uri);
//    FileInfoJob *fileInfoJob = new FileInfoJob(l_fileInfo);
//    fileInfoJob->setAutoDelete();
//    fileInfoJob->querySync();
//    bool a = l_fileInfo.get()->isDir();

    bool isDir = false;
    g_autoptr (GFile) file = g_file_new_for_uri(uri.toUtf8().constData());
    g_autoptr (GFileInfo) info = g_file_query_info(file,
                                        "standard::*",
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        nullptr,
                                        nullptr);
    GFileType fileType = g_file_info_get_file_type(info);
    QString contentType = g_file_info_get_content_type (info);
    if (fileType == G_FILE_TYPE_DIRECTORY || contentType == "inode/directory") {
        isDir = true;
    }

    if (isDir)
        m_folderContainFolders ++;
    else
        m_folderContainFiles ++;

    m_fileSizeCount += size;
    m_fileDoneCount ++;
    //500 files update ui
    updateCountInfo((m_fileDoneCount % 500 == 0));
}

void BasicPropertiesPage::cancelCount()
{
    m_timer->stop();
    if (m_countOp)
        m_countOp->cancel();
}

/*!
 * 响应窗口确定按钮
 * \brief BasicPropertiesPage::saveAllChange
 */
void BasicPropertiesPage::saveAllChange()
{
    if (m_watcher)
        m_watcher->stopMonitor();
    if (m_thumbnail_watcher)
        m_thumbnail_watcher->stopMonitor();
    //未发生修改
    if (!this->m_thisPageChanged)
        return;

    if (BP_MultipleFIle != this->checkFileType(m_uris)) {
        //拒绝修改home目录
        if (m_info.get()->uri() == ("file://"+QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first())) {
            return;
        }
        //修改图标
        this->changeFileIcon();

        if (m_info->isDir()) {
            if ((m_readOnly && m_readOnlyState != m_readOnly->checkState())
                    || (m_hidden && m_hiddenState != m_hidden->checkState())) {
                bool isReadOnly = false;
                bool isHidden = false;

                if (Qt::Checked == m_readOnly->checkState()) {
                    isReadOnly = true;
                }
                if (Qt::Checked == m_hidden->checkState()) {
                    isHidden = true;
                }

                PropertiesSetDialog *dialog = new PropertiesSetDialog();
                connect(dialog, &PropertiesSetDialog::sendSelectRadioButton, this, [=](int id){
                    //id == 0 设置当前所选项   id == 1 递归设置
                    bool isRecursive = false;
                    if (id) {
                        isRecursive = true;
                    }
                    auto metainfo = FileMetaInfo::fromUri(m_info->uri());
                    if (metainfo) {
                        metainfo->setMetaInfoInt("peony-is-hidden", isHidden? 1: 2);
                    }
                    FileOperationUtils::setReadOnlyAndHidden(QStringList() << m_info->uri(), isReadOnly, isHidden, isRecursive);
                });
                dialog->exec();
            }
            if (isNameChanged()) {
                FileOperationUtils::rename(m_info.get()->uri(), m_displayNameEdit->text(), true);
            }
        } else {
            if (m_readOnly && m_isReadOnly != m_readOnly->isChecked()) {
                mode_t mod = 0;
                quint32 mode = 0;
                if(m_readOnly->isChecked()) {
        //            mod |= S_IRUSR;
        //            mod |= S_IRGRP;
        //            mod |= S_IROTH;
                    if (getFileUnixMode(m_info->uri(), mode)) {
                        auto metaInfo = FileMetaInfo::fromUri(m_info->uri());
                        if (metaInfo) {
                            metaInfo->setMetaInfoInt(TEMP_PERMISSIONS, mode);
                        }
                    }
                    //去除写权限
                    mode &= ~S_IWUSR;
                    mode &= ~S_IWGRP;
                    mode &= ~S_IWOTH;
                    mod = mode;
                } else {
                    auto metaInfo = FileMetaInfo::fromUri(m_info.get()->uri());
                    if (metaInfo && metaInfo->getMetaInfoInt(TEMP_PERMISSIONS)) {
                        mod = metaInfo->getMetaInfoInt(TEMP_PERMISSIONS);
                    } else {
                        mod |= S_IRUSR;
                        mod |= S_IRGRP;
                        mod |= S_IROTH;

                        mod |= S_IWUSR;
        //                mod |= S_IWGRP;
        //                mod |= S_IWOTH;
                    }
                }
                //FIX:如果该文件之前就是可执行，那么应该保留可执行权限
                if (m_info->canExecute())
                    mod |= S_IXUSR;

                //.desktop文件给予可执行,.desktop文件原本可执行才给可执行权限
                if (((m_info.get()->isDesktopFile()) || m_info.get()->displayName().endsWith(".desktop")) && m_info->canExecute()) {
                    //FIX:可执行范围 目前只给拥有者执行权限
                    mod |= S_IXUSR;
                    //mod |= S_IXGRP;
                    //mod |= S_IXOTH;
                }
//                QUrl url = m_info.get()->uri();
//                g_chmod(url.path().toUtf8(), mod);
                setFileUnixMode(m_info->uri(), (guint32)mod);

            }

            //是否进行文件隐藏操作 - Whether to hide files
            bool existHiddenOpt = false;
            if (m_hidden) {
                QString newName = m_info.get()->displayName();

                if (newName.startsWith("."))
                    newName = newName.mid(1,-1);

                if (isNameChanged()) {
                    newName = m_displayNameEdit->text();
                }

                bool isHidden = m_info.get()->displayName().startsWith(".");

                //以前没隐藏，并且选中隐藏框
                if(!isHidden && m_hidden->isChecked()) {
                    newName = "." + newName;
                    FileOperationUtils::rename(m_info.get()->uri(), newName, true);
                    existHiddenOpt = true;

                } else if(isHidden && !m_hidden->isChecked()) {
                    //以前已经隐藏，并且取消选中隐藏框
                    FileOperationUtils::rename(m_info.get()->uri(), newName, true);
                    existHiddenOpt = true;
                }
            }

            if (!existHiddenOpt) {
                if (isNameChanged()) {
                    FileOperationUtils::rename(m_info.get()->uri(), m_displayNameEdit->text(), true);
                }
            }

            if (m_executable && m_executable->isVisible() && m_executableState != m_executable->checkState()) {
                mode_t mod = 0;
                quint32 mode = 0;
                getFileUnixMode(m_info->uri(), mode);

                if (m_executable->isChecked()) {
                    mode |= S_IXUSR;
                } else {
                    mode &= ~S_IXUSR;
                }

                mod = mode;
                setFileUnixMode(m_info->uri(), (guint32)mod);
            }
        }
    } else {
        if ((m_readOnly && m_readOnlyState != m_readOnly->checkState())
                || (m_hidden && m_hiddenState != m_hidden->checkState())) {

            bool isReadOnly = false;
            bool isHidden = false;

            if (Qt::Checked == m_readOnly->checkState()) {
                isReadOnly = true;
            }
            if (Qt::Checked == m_hidden->checkState()) {
                isHidden = true;
            }

            if (m_isAllDir) {
                PropertiesSetDialog *dialog = new PropertiesSetDialog();
                connect(dialog, &PropertiesSetDialog::sendSelectRadioButton, this, [=](int id){
                    for (auto uri: m_uris) {
                        auto metainfo = FileMetaInfo::fromUri(uri);
                        if (metainfo) {
                            metainfo->setMetaInfoInt("peony-is-hidden", isHidden? 1: 2);
                        }
                    }
                    //id == 0 设置当前所选项   id == 1 递归设置
                    bool isRecursive = false;
                    if (id) {
                        isRecursive = true;
                    }
                    FileOperationUtils::setReadOnlyAndHidden(m_uris, isReadOnly, isHidden, isRecursive);
                });
                dialog->exec();
            } else {
                for (auto uri: m_uris) {
                    auto metainfo = FileMetaInfo::fromUri(uri);
                    if (metainfo) {
                        metainfo->setMetaInfoInt("peony-is-hidden", isHidden? 1: 2);
                    }
                }
                FileOperationUtils::setReadOnlyAndHidden(m_uris, isReadOnly, isHidden, false);
            }
        }
    }

    //FIX:修复桌面快捷方式文件的缩略图改变后需要手动刷新才更新的问题
    //fix the problem that the thumbnails of desktop shortcut files need to be manually refreshed before they are updated after being changed.
    QString desktopPath = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
    //if (m_info.get()->uri().contains(desktopUri) && m_info.get()->isSymbolLink()) {
        QProcess p;
        p.setProgram("/usr/bin/touch");
        p.setArguments(QStringList()<<"-h"<<m_info->filePath());
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        p.startDetached();
    #else
        p.startDetached("/usr/bin/touch", QStringList()<<"-h"<<m_info->filePath());
    #endif
        p.waitForFinished(-1);
    //}
}

void BasicPropertiesPage::chooseFileIcon()
{
    FileType fileType = this->checkFileType(m_uris);
    if (fileType == FileType::BP_Folder) {
        QString currentIcon = m_newFileIconPath.isEmpty() ? m_info->customIcon() : m_newFileIconPath;
        CustomIconDialog *dialog = new CustomIconDialog(m_info.get()->uri(), currentIcon, this);
        connect(dialog, &CustomIconDialog::sendCustomIcon, this, [=](const QString &iconName){
            qDebug() << __func__ << __LINE__ << "iconName:" << iconName;
            if (iconName.isEmpty()) {
                QString icon = FileUtils::updateFileIconName(m_info.get()->uri(), true);
                m_iconButton->setIcon(QIcon::fromTheme(icon));
                this->m_newFileIconPath = "";
                this->thisPageChanged();
            } else {
                QIcon icon(iconName);
                QFileInfo info(iconName);
                if (icon.isNull() || !info.exists() /*|| icon.availableSizes().isEmpty()*/) {
                    icon = QIcon::fromTheme(iconName, QIcon::fromTheme("unknown"));
                }

                m_iconButton->setIcon(icon);
                this->m_newFileIconPath = iconName;
                if (iconName != currentIcon) {
                    this->thisPageChanged();
                }
            }
        });
        dialog->exec();
    } else {
        QString picture = BasicPropertiesPage::chooseCustomIconFile(nullptr);
        if (!picture.isEmpty()) {
            qDebug()<<"chose new file icon:"<< picture;
            m_iconButton->setIcon(QIcon(picture));
            this->m_newFileIconPath = picture;
            this->thisPageChanged();
        }
    }
}

void BasicPropertiesPage::changeFileIcon()
{
    //if (!m_newFileIconPath.isEmpty()) {
        auto metaInfo = FileMetaInfo::fromUri(m_info.get()->uri());
//        QFileInfo fileInfo(m_newFileIconPath);
//        if (!QIcon::fromTheme(fileInfo.baseName()).isNull())
//            metaInfo.get()->setMetaInfoString("custom-icon", fileInfo.baseName());
//        else
            metaInfo.get()->setMetaInfoString("custom-icon", m_newFileIconPath);

        const QString uri = m_info.get()->uri();
        // 还原为默认图标时清除缩略图缓存，避免再次打开属性页时仍显示旧的自定义图标（偶现原因：缓存未失效）
        if (m_newFileIconPath.isEmpty()) {
            ThumbnailManager::getInstance()->releaseThumbnail(uri);
        }
        ThumbnailManager::getInstance()->createThumbnail(uri, m_thumbnail_watcher, true);
    //}
}

void BasicPropertiesPage::updateCountInfo(bool isDone)
{
    if (isDone) {
        QString fileSizeText;

        quint64 a = 0;
        a = m_fileSizeCount / CELL1K;
        //小于1KB
        if (a < 1) {
            fileSizeText = tr("%1 Bytes").arg(m_fileSizeCount);
        } else {
            char *fileSizeFormat = g_format_size(m_fileSizeCount);
            QString fileSizeFormatString(fileSizeFormat);
            //根据设计要求，按照1024字节对数据进行格式化（1GB = 1024MB），同时显示为KB/MB/GB，以便于用户理解。参考windows显示样式。

            fileSizeText = tr("%1 (%2 Bytes)").arg(fileSizeFormatString).arg(m_fileSizeCount);
            g_free(fileSizeFormat);
        }

        fileSizeText = BasicPropertiesPage::elideTextAndToolTip(m_fileSizeLabel->font(), 240, fileSizeText, Qt::ElideMiddle, m_fileSizeLabel);
        m_fileSizeLabel->setText(fileSizeText);

        //在为完成统计前，先显示文件大小而不是占用空间大小
        m_fileTotalSizeLabel->setText(fileSizeText);

        if(m_folderContainLabel)
            m_folderContainLabel->setText(tr("%1 files, %2 folders").arg(m_folderContainFiles).arg(m_folderContainFolders));
    }
}

void BasicPropertiesPage::updateDateFormat(QString dateFormat)
{
    //update date and time show format, task #101605
    qDebug() << "sdk format signal:"<<dateFormat;
    if (m_date_format != dateFormat){
        updateInfo(m_info->uri());
        m_date_format = dateFormat;
    }
}

void BasicPropertiesPage::updateInfo(const QString &uri)
{
    //QT获取文件相关时间 ,
    m_info = FileInfo::fromUri(uri);
    FileInfoJob *fileInfoJob = new FileInfoJob(m_info);
    fileInfoJob->setAutoDelete();
    connect(fileInfoJob, &FileInfoJob::queryAsyncFinished, this, [=](){
        QUrl url(uri);
        //FIXME:暂时不处理除了本地文件外的文件信息,希望添加对其他文件的支持
        //if (url.isLocalFile()) {
        if(m_info->accessTime() != 0 && m_info->modifiedTime() != 0){
//            if(m_timeModifiedLabel) {
//                QDateTime date2 = qFileInfo.lastModified();
//                QString time2 = date2.toString(m_systemTimeFormat);
//                m_timeModifiedLabel->setText(time2);
//            }
//            if(m_timeAccessLabel) {
//                QDateTime date3 = qFileInfo.lastRead();
//                QString time3 = date3.toString(m_systemTimeFormat);
//                m_timeAccessLabel->setText(time3);
//            }

            GFile     *file = g_file_new_for_uri(uri.toUtf8().constData());
            GFileInfo *info = g_file_query_info(file,
                                                "time::*",
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                nullptr,
                                                nullptr);
            g_object_unref(file);


            m_timeModified = g_file_info_get_attribute_uint64(info,"time::modified");
            if(m_timeModifiedLabel) {
//                QDateTime date2 = QDateTime::fromMSecsSinceEpoch(m_timeModified*1000);
//                QString time2 = date2.toString(m_systemTimeFormat);
                QString time2 = GlobalSettings::getInstance()->transToSystemTimeFormat(m_timeModified, true);
                m_timeModifiedLabel->setText(time2);
            }

            if(m_timeAccessLabel) {
                m_timeAccess = g_file_info_get_attribute_uint64(info,"time::access");
//                QDateTime date3 = QDateTime::fromMSecsSinceEpoch(m_timeAccess*1000);
//                QString time3 = date3.toString(m_systemTimeFormat);
                QString time3 = GlobalSettings::getInstance()->transToSystemTimeFormat(m_timeAccess, true);
                m_timeAccessLabel->setText(time3);
            }

            QString path = url.path();
            QFileInfo qFileInfo(path);
            if (/*qFileInfo.isDir() && */m_timeCreatedLabel) {
                m_timeCreated = g_file_info_get_attribute_uint64(info, "time::created");

                // 客户需要必须显示创建时间，因此使用三个时间最小时间戳为创建时间
//                quint64 minTime = m_timeCreated != 0 ? m_timeCreated : m_timeModified;
//                minTime = qMin (minTime, m_timeModified);
//                if (m_timeAccess != 0)
//                    minTime = qMin (minTime, m_timeAccess);
//                m_timeCreated = minTime;
                if (m_timeCreated) {
//                    QDateTime createDate = QDateTime::fromMSecsSinceEpoch(m_timeCreated*1000);
//                    QString createTime = createDate.toString(m_systemTimeFormat);
                    QString createTime = GlobalSettings::getInstance()->transToSystemTimeFormat(m_timeCreated, true);
                    m_timeCreatedLabel->setText(createTime);
                } else {
                    QFormLayout *layout = this->findChild<QFormLayout*>("floorTwoBaseLayout");
                    switch (m_fileType) {
                    case BP_Folder:
                    case BP_File:
                    case BP_Application: {
                        layout->removeRow(m_timeCreatedLabel);
                        break;
                    }
                    default:
                        break;
                    }
                }

//                // FIXME:目前只是文件夹显示创建时间，当创建时间获取失败的时候，将修改时间作为创建时间
//                QDateTime date1 = qFileInfo.birthTime();
//                if (date1.isValid()) {
//                    QString time1 = date1.toString(m_systemTimeFormat);
//                    m_timeCreatedLabel->setText(time1);
//                } else {
//                    m_timeCreated = g_file_info_get_attribute_uint64(info, "time::modified");
//                    QDateTime createDate = QDateTime::fromMSecsSinceEpoch(m_timeCreated*1000);
//                    QString createTime = createDate.toString(m_systemTimeFormat);
//                    m_timeCreatedLabel->setText(createTime);
//                }

            }
            g_object_unref(info);

        } else {
            if (m_timeCreatedLabel){
                if (QGSettings::isSchemaInstalled("org.ukui.style"))
                {
                    //font monitor
                    QGSettings *fontSetting = new QGSettings(FONT_SETTINGS, QByteArray(), this);
                    connect(fontSetting, &QGSettings::changed, this, [=](const QString &key){
                        if (key == "systemFontSize") {
                            QFontMetrics fontWidth(m_timeCreatedLabel->font());
                            QString elideNote = fontWidth.elidedText(tr("Can't get remote file information"),Qt::ElideRight,280);
                            m_timeCreatedLabel->setText(elideNote);
                            m_timeCreatedLabel->setToolTip(tr("Can't get remote file information"));
                        }
                    });
                }
                QFontMetrics fontWidth(m_timeCreatedLabel->font());
                QString elideNote = fontWidth.elidedText(tr("Can't get remote file information"),Qt::ElideRight,280);
                m_timeCreatedLabel->setText(elideNote);
                m_timeCreatedLabel->setToolTip(tr("Can't get remote file information"));
            }
        }

        //FIXME:GVFS底层暂未实现文件创建时间获取API,暂时使用QT获取文件创建时间
        /*
         m_timeCreated = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_TIME_CREATED);
        //m_timeCreated = g_file_info_get_attribute_uint64(info, "time::created");
        QDateTime date1 = QDateTime::fromMSecsSinceEpoch(m_timeCreated*1000);
        QString time1 = date1.toString(m_systemTimeFormat);
        m_timeCreatedLabel->setText(time1);

         if(m_timeModifiedLabel) {
            m_timeModified = g_file_info_get_attribute_uint64(info,
                                                              "time::modified");
            QDateTime date2 = QDateTime::fromMSecsSinceEpoch(m_timeModified*1000);
            QString time2 = date2.toString(m_systemTimeFormat);

            m_timeModifiedLabel->setText(time2);
        }
        if(m_timeAccessLabel) {
            m_timeAccess = g_file_info_get_attribute_uint64(info,
                                                            "time::access");
            QDateTime date3 = QDateTime::fromMSecsSinceEpoch(m_timeAccess*1000);
            QString time3 = date3.toString(m_systemTimeFormat);
            m_timeAccessLabel->setText(time3);
        }

        g_object_unref(info);

         */
    });

    fileInfoJob->queryAsync();
}

QLabel *BasicPropertiesPage::createFixedLabel(quint64 minWidth, quint64 minHeight, QString text, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    if (minWidth != 0)
        label->setMinimumWidth(minWidth);
    if (minHeight != 0)
        label->setMinimumHeight(minHeight);

    label->setText(text);
    return label;
}

QLabel *BasicPropertiesPage::createFixedLabel(quint64 minWidth, quint64 minHeight, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    if (minWidth != 0)
        label->setMinimumWidth(minWidth);
    if (minHeight != 0)
        label->setMinimumHeight(minHeight);
    return label;
}

bool BasicPropertiesPage::isNameChanged()
{
    if (!m_displayNameEdit->isReadOnly() && !m_displayNameEdit->text().isEmpty()) {
        QString fileName(m_info->displayName());
        //桌面文件
        if (m_info->isDesktopFile() && !fileName.endsWith(".desktop")) {
            //做过处理的名称
            QString handledName = FileUtils::handleDesktopFileName(m_info->uri(), fileName);
            if (fileName != handledName) {
                //用户是否手动修改
                if (handledName != m_displayNameEdit->text())
                    return true;
                else
                    return false;
            }
        }
        //文件名称被修改过
        if (fileName != m_displayNameEdit->text())
            return true;
        else
            return false;

    }

    return false;
}

void BasicPropertiesPage::setSysTimeFormat()
{
    this->m_systemTimeFormat = GlobalSettings::getInstance()->getSystemTimeFormat();
}

QString BasicPropertiesPage::elideText(QFont font, int width, QString strInfo)
{
    QFontMetrics fontMetrics(font);

    if(fontMetrics.width(strInfo) > width) {
        strInfo= QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}

QString BasicPropertiesPage::elideTextAndToolTip(QFont font, int width, QString strInfo, Qt::TextElideMode mode, QWidget *widgt)
{
    QFontMetrics fontMetrics(font);

    if (fontMetrics.width(strInfo) > width) {
        widgt->setToolTip(strInfo);
        strInfo = QFontMetrics(font).elidedText(strInfo, mode, width);
    }

    return strInfo;
}

void BasicPropertiesPage::moveFile()
{
    auto newDirPath = QString(QFileDialog::getExistingDirectoryUrl(nullptr, tr("Choose a new folder:"),m_info.get()->uri()).toEncoded());
    DEBUG << "new path:" << newDirPath ;

    if(newDirPath.isEmpty())
        return;

    if(newDirPath == m_info.get()->uri()) {
        QMessageBox::critical(nullptr, tr("Error"), tr("cannot move a folder to itself !"), QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    if(newDirPath.startsWith(m_info.get()->uri())) {
        QMessageBox::critical(nullptr, tr("Error"), tr("cannot move a folder to itself !"), QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    DEBUG << "old Path:" << m_info.get()->uri() << newDirPath;

    QStringList uriList;
    uriList << m_info.get()->uri();

    auto fileOpe = FileOperationUtils::move(uriList,newDirPath,true);

    connect(fileOpe,&FileOperation::operationFinished,[=](){
        if (!fileOpe->hasError()) {
            QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.setProgram("/usr/bin/peony");
            p.setArguments(QStringList()<<"--show-folders" << newDirPath);
            p.startDetached();
#else
                p.startDetached("/usr/bin/peony", QStringList()<<"--show-folders"<< newDirPath);
#endif
            Q_EMIT requestCloseMainWindow();
        }
    });
}

bool BasicPropertiesPage::checkUserPermission(const QString &targetUser)
{
    bool hasPermission = true;
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (!pw) {
        hasPermission = false;
        return hasPermission;
    }

    bool isSelf = false;
    bool isSameGroup = false;
    auto username = pw->pw_name;
    if (targetUser == username) {
        isSelf = true;
    }

    if (!isSelf && !isSameGroup) {
        qDebug()<<"the uid not permit";
        hasPermission = false;
    }

    if (pw->pw_uid == 0) {
        QFileInfo file("/usr/sbin/security-switch");
        if(file.exists() == true) {
            QProcess shProcess;
            shProcess.start("/usr/sbin/security-switch --get");
            if (!shProcess.waitForStarted()) {
                qDebug()<<"wait get security state start timeout";
            } else {
                if (!shProcess.waitForFinished()) {
                    qDebug()<<"wait get security state finshed timeout";
                } else {
                    QString secState = shProcess.readAllStandardOutput();
                    qDebug()<<"security-switch get test "<< secState;
                    if (secState.contains("strict")) {
                        qDebug()<<"now it is in strict mode, so root is not super";
                    } else {
                        qDebug()<<"pw uid is 0, it is super";
                        hasPermission = true;
                    }
                }
            }
        } else {
            qDebug()<<"security-switch is not support, so it is super";
            hasPermission = true;
        }
    }

    return hasPermission;
}

void BasicPropertiesPage::queryUserPermission(const QString &uri)
{
    g_autoptr (GFile) file = g_file_new_for_uri(uri.toUtf8().constData());
    g_autoptr (GFileInfo) info = g_file_query_info(file,
                                                  "owner::*,",
                                                  G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                  nullptr,
                                                  nullptr);
    auto user = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_OWNER_USER);
    QString userString = user;
    m_hasPermission = checkUserPermission(userString);
}

bool BasicPropertiesPage::getFileUnixMode(const QString &uri, quint32 &mode)
{
    mode = 0;
    QByteArray uriBytes = uri.toUtf8();
    g_autoptr(GFile) file = g_file_new_for_uri(uriBytes.constData());
    if (!file) {
        return false;
    }

    g_autoptr(GError) error = NULL;
    g_autoptr(GFileInfo) info = g_file_query_info(file,
                                                  "unix::mode",
                                                  G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                  nullptr,
                                                  &error);
    if (!info) {
        return false;
    }

    if (!g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE)) {
        return false;
    }

    mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);
    return true;
}

void BasicPropertiesPage::setFileUnixMode(const QString &uri, quint32 mode)
{
    QByteArray uriBytes = uri.toUtf8();
    g_autoptr(GFile) file = g_file_new_for_uri(uriBytes.constData());
    if (!file) {
        return;
    }

    g_file_set_attribute_uint32(file,
                                G_FILE_ATTRIBUTE_UNIX_MODE,
                                mode,
                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                nullptr,
                                nullptr);
}

void FileNameThread::run()
{
    QString fileName = "";
    if (m_uris.count() == 1) {
        std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(m_uris.first());
        FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
        fileInfoJob->setAutoDelete();
        fileInfoJob->querySync();
        fileName = fileInfo.get()->displayName();
        //fix bug#53504, not show duplicated name issue
        if (fileInfo->isDesktopFile())
        {
            fileName = FileUtils::handleDesktopFileName(fileInfo->uri(), fileName);
        }
    } else {
        QStringList stringList;
        for (const auto &uri : m_uris) {
            //FIXME: replace BLOCKING api in ui thread.(finish) **
            std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(uri);
            FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
            fileInfoJob->setAutoDelete();
            fileInfoJob->querySync();
            stringList<< fileInfo.get()->displayName();
        }
        auto text = stringList.join(",");
        fileName = QString(text);
    }

    Q_EMIT fileNameReady(fileName);
}

BatchStatusThread::BatchStatusThread(const QStringList &uris)
    : m_uris(uris)
{
    qRegisterMetaType<Qt::CheckState>("Qt::CheckState");
}

void BatchStatusThread::run()
{
    for (const auto &uri : m_uris) {
        QStringList stringList;
        FileEnumerator e;
        e.setEnumerateDirectory(uri);
        e.enumerateSync();
        stringList.append(e.getChildrenUris());

        for (auto u : stringList) {
            queryInfoUpdate(u, false, false);
        }
        queryInfoUpdate(uri, true, true);
    }

    Qt::CheckState readOnlyState = Qt::CheckState::Unchecked;
    Qt::CheckState hiddenState = Qt::CheckState::Unchecked;
    if (m_existReadOnly && !m_existWrite) {
        readOnlyState = Qt::CheckState::Checked;
    } else if (m_existReadOnly && m_existWrite) {
        readOnlyState = Qt::CheckState::PartiallyChecked;
    } else {
        readOnlyState = Qt::CheckState::Unchecked;
    }

    if (m_existHidden && !m_existShow) {
        hiddenState = Qt::CheckState::Checked;
    } else if (m_existHidden && m_existShow) {
        hiddenState = Qt::CheckState::PartiallyChecked;
    } else {
        hiddenState = Qt::CheckState::Unchecked;
    }

    bool hiddenDisable = !m_canRename || m_isDesktop || m_isIDM;

    Q_EMIT updateState(readOnlyState, hiddenState);
    Q_EMIT updateDisabled(!m_canRename, hiddenDisable);
    Q_EMIT updateIsAllDir(m_isAllDir);
}

void BatchStatusThread::queryInfoUpdate(const QString &uri, const bool &queryHidden, const bool &queryIsAllDir)
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    bool canRead = false;
    bool canWrite = false;
    g_autoptr(GFile) file = g_file_new_for_uri(uri.toUtf8().constData());
    g_autoptr(GFileInfo) fileInfo = g_file_query_info(file,
                                                      "standard::*," "access::*,",
                                                      G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                      nullptr,
                                                      nullptr);

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_READ)) {
        canRead = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
    } else {
        canRead = true;
    }

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE)) {
        canWrite = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
    } else {
        canWrite = true;
    }

    if (!g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME)) {
        m_canRename = false;
    }

    if (queryHidden) {
        if (g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN)) {
            m_existHidden = true;
        } else {
            m_existShow = true;
        }
    }

    if (queryIsAllDir) {
        GFileType fileType = g_file_info_get_file_type(fileInfo);
        QString contentType = g_file_info_get_content_type (fileInfo);
        if (fileType == G_FILE_TYPE_DIRECTORY || contentType == "inode/directory") {
            m_isAllDir = true;
        }
    }

    bool isDesktop = FileUtils::isSamePath(uri, desktopPath);
    if (isDesktop) {
        m_isDesktop = isDesktop;
    }

    bool isIDM = uri.startsWith("idm://");
    if (isIDM) {
        m_isIDM = isIDM;
    }

    if(canRead && !canWrite) {
        m_existReadOnly = true;
    } else {
        m_existWrite = true;
    }
}

PropertiesSetDialog::PropertiesSetDialog(QWidget *parent)
    : QDialog(parent)
{
    this->initUI();
    connect(m_okBtn, &QPushButton::clicked, this, [=](){
        int id = m_group->checkedId();
        Q_EMIT sendSelectRadioButton(id);
        accept();
    });
    connect(m_cancelBtn, &QPushButton::clicked, this, &PropertiesSetDialog::reject);
}

PropertiesSetDialog::~PropertiesSetDialog()
{

}

void PropertiesSetDialog::initUI()
{
    this->setAutoFillBackground(true);
    this->setWindowTitle(tr("Confirming property settings"));
    this->setFixedSize(460, 300);
    this->setContentsMargins(0, 0, 0, 0);

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    m_label = new QLabel(tr("Whether to apply to the current selected option or to selected options and subfolders and subfiles."));
    m_label->setContentsMargins(22, 10, 22, 0);
    m_label->setWordWrap(true);
    m_label->setAutoFillBackground(true);
    m_layout->addWidget(m_label);
    m_layout->addStretch(1);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(22, 0, 22, 0);
    gridLayout->setSpacing(0);

    QLabel *currentSelectLabel = new QLabel(tr("Apply the current selection"), this);
    QLabel *recursiveLabel = new QLabel(tr("Apply the current selection as well as subfolders and subfiles"), this);
    currentSelectLabel->setWordWrap(true);
    recursiveLabel->setWordWrap(true);

    m_currentSelectBtn = new QRadioButton(this);
    m_recursiveBtn = new QRadioButton(this);
    m_recursiveBtn->setChecked(true);

    m_group = new QButtonGroup;
    m_group->setExclusive(true);
    m_group->addButton(m_currentSelectBtn, 0);
    m_group->addButton(m_recursiveBtn, 1);
    gridLayout->addWidget(m_currentSelectBtn, 0, 0, 1, 1);
    gridLayout->addWidget(currentSelectLabel, 0, 1, 1, 15);
    gridLayout->addWidget(m_recursiveBtn, 1, 0, 1, 1);
    gridLayout->addWidget(recursiveLabel, 1, 1, 1, 15);
    m_layout->addLayout(gridLayout);
    m_layout->addSpacing(10);
    m_layout->addStretch(1);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(22, 0, 22, 0);
    hBoxLayout->setSpacing(0);
    m_okBtn = new QPushButton(tr("Ok"));
    m_cancelBtn = new QPushButton(tr("Cancel"));
    hBoxLayout->addStretch(1);
    hBoxLayout->addWidget(m_cancelBtn);
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(m_okBtn);
    m_layout->addLayout(hBoxLayout);
    m_layout->addSpacing(16);
}

const QStringList CustomIconDialog::s_iconList = CustomIconDialog::initializeIconList();

CustomIconDialog::CustomIconDialog(const QString &uri, const QString &currentIconPath, QWidget *parent)
    : m_uri(uri), QDialog(parent)
{
    initUI();
    loadData();
    // 若传入当前已设置的自定义图标路径，作为初始选中，避免用户选择无效图标后点击确定时丢失原图标
    if (!currentIconPath.isEmpty()) {
        m_selectIconName = currentIconPath;
        if (currentIconPath.contains(QLatin1String("/"))) {
            m_customIconRadio->setChecked(true);
            m_customEdit->setText(currentIconPath);
            m_customEdit->setToolTip(currentIconPath);
            updatePreviewIcon();
        } else {
            for (int i = 0; i < m_iconList->count(); ++i) {
                QListWidgetItem *item = m_iconList->item(i);
                if (item && item->icon().name() == currentIconPath) {
                    m_iconList->setCurrentRow(i);
                    updatePreviewIcon();
                    break;
                }
            }
        }
    }
    initConnects();
}

CustomIconDialog::~CustomIconDialog()
{

}

const QStringList &CustomIconDialog::getAllIcons()
{
    return s_iconList;
}

void CustomIconDialog::updateSystemBtnStatus(const QListWidgetItem *current, const QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) {
        qWarning() << __func__ << "select item is null";
        return;
    }

    m_systemIconRadio->setChecked(true);
    qDebug() << __func__ << "icon name:" << current->icon().name();
    m_selectIconName = current->icon().name();
    updatePreviewIcon();
}

void CustomIconDialog::updatePreviewIcon()
{
    if (m_systemIconRadio->isChecked()) {
        QListWidgetItem *item = m_iconList->currentItem();
        m_previewIconLabel->setPixmap(item->icon().pixmap(QSize(90, 90)));
    } else if (m_customIconRadio->isChecked()) {
        QString iconPath = m_customEdit->text();
        QPixmap pixmap(iconPath);
        if (!pixmap.isNull()) {
            m_previewIconLabel->setPixmap(pixmap.scaled(90, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void CustomIconDialog::updateCustomStatus()
{
    QString iconName = BasicPropertiesPage::chooseCustomIconFile(nullptr);
    qDebug() << __func__ << "iconName:" << iconName;
    // 仅当用户选择了有效图标（如未超过1M限制）时更新选中项，避免选择无效图标时清空之前的图标
    if (!iconName.isEmpty()) {
        m_selectIconName = iconName;
        m_customIconRadio->setChecked(true);
        m_customEdit->setText(m_selectIconName);
        m_customEdit->setToolTip(m_selectIconName);
        updatePreviewIcon();
    }
}

void CustomIconDialog::onClicked()
{
    Q_EMIT sendCustomIcon(m_selectIconName);
    this->close();
}

void CustomIconDialog::onDefaultIcon()
{
    m_selectIconName = "";
    QString iconName = FileUtils::updateFileIconName(m_uri, true);
    m_previewIconLabel->setPixmap(QIcon::fromTheme(iconName).pixmap(QSize(90, 90)));
}

void CustomIconDialog::initUI()
{
    this->setWindowTitle(tr("Change folder icon"));
    this->setFixedSize(410, 577);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setContentsMargins(0, 10, 0, 0);
    this->setWindowFlags(this->windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(24, 24, 24, 24);
    m_layout->setSpacing(0);

    QHBoxLayout *iconLayout = new QHBoxLayout;
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->setSpacing(0);

    m_previewIconLabel = new QLabel(this);
    m_previewIconLabel->setFixedSize(96, 96);

    iconLayout->addStretch();
    iconLayout->addWidget(m_previewIconLabel);
    iconLayout->addStretch();

    QButtonGroup buttonGroup(this);
    buttonGroup.setExclusive(true);
    m_systemIconRadio = new QRadioButton(tr("Select an icon from the system icons:"), this);
    m_customIconRadio = new QRadioButton(tr("Custom Icons:"), this);
    m_systemIconRadio->setChecked(true);
    buttonGroup.addButton(m_systemIconRadio);
    buttonGroup.addButton(m_customIconRadio);

    m_iconList = new QListWidget(this);
    m_iconList->setFixedSize(362, 180);
    m_iconList->setSpacing(0);
    m_iconList->setViewMode(QListWidget::IconMode);
    m_iconList->setIconSize(QSize(40, 40));
    m_iconList->setUniformItemSizes(true);
    m_iconList->setMovement(QListView::Static);

    QHBoxLayout *customLayout = new QHBoxLayout;
    customLayout->setContentsMargins(0, 0, 0, 0);
    customLayout->setSpacing(0);

    m_customEdit = new QLineEdit(this);
    m_customEdit->setPlaceholderText(tr("Click Browse to load a custom icon"));
    m_customEdit->setReadOnly(true);
    m_customIconBtn = new QPushButton(tr("Browse"), this);
    customLayout->addWidget(m_customEdit, 2);
    customLayout->addSpacing(12);
    customLayout->addWidget(m_customIconBtn, 1);

    QHBoxLayout *lastLayout = new QHBoxLayout;
    lastLayout->setContentsMargins(0, 0, 0, 0);
    lastLayout->setSpacing(0);

    m_defaultIconBtn = new QPushButton(tr("Restore default"), this);
    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    m_okBtn = new QPushButton(tr("Ok"), this);

    lastLayout->addWidget(m_defaultIconBtn, 0, Qt::AlignHCenter);
    lastLayout->addSpacing(12);
    lastLayout->addStretch();
    lastLayout->addWidget(m_cancelBtn);
    lastLayout->addSpacing(12);
    lastLayout->addWidget(m_okBtn);

    m_layout->addLayout(iconLayout);
    m_layout->addSpacing(30);
    m_layout->addWidget(m_systemIconRadio);
    m_layout->addSpacing(8);
    m_layout->addWidget(m_iconList);
    m_layout->addSpacing(24);
    m_layout->addWidget(m_customIconRadio);
    m_layout->addSpacing(8);
    m_layout->addLayout(customLayout);
    m_layout->addStretch();
    m_layout->addLayout(lastLayout);
}

void CustomIconDialog::initConnects()
{
    connect(m_iconList, &QListWidget::currentItemChanged, this, &CustomIconDialog::updateSystemBtnStatus);
    connect(m_customIconBtn, &QPushButton::clicked, this, &CustomIconDialog::updateCustomStatus);
    connect(m_defaultIconBtn, &QPushButton::clicked, this, &CustomIconDialog::onDefaultIcon);
    connect(m_cancelBtn, &QPushButton::clicked, this, &CustomIconDialog::close);
    connect(m_okBtn, &QPushButton::clicked, this, &CustomIconDialog::onClicked);
}

void CustomIconDialog::loadData()
{
    QStringList iconNames = getAllIcons();

    for (auto iconName : iconNames) {
        QIcon icon = QIcon::fromTheme(iconName);
        if (icon.isNull() || icon.availableSizes().isEmpty())
            continue;

        QListWidgetItem *item = new QListWidgetItem(icon, nullptr);
        item->setSizeHint(QSize(40, 40));
        if (m_iconList)
            m_iconList->addItem(item);
    }

    if (m_iconList->count() > 0) {
        m_iconList->setCurrentRow(0);
        updatePreviewIcon();
    }
}

QStringList CustomIconDialog::getThemeIcon() const
{
    QStringList iconNames;

    QStringList directories;
    directories << "/usr/share/icons/ukui-icon-theme-default/64x64/devices"
                << "/usr/share/icons/ukui-icon-theme-default/64x64/places";

    QStringList iconExtensions;
    iconExtensions << "*.png" << "*.svg" << "*.xpm" << "*.svgz"
                   << "*.ico" << "*.bmp" << "*.jpg" << "*.jpeg";

    for (const QString &directory : directories) {
        QDir dir(directory);

        if (!dir.exists()) {
            qWarning() << "Directory does not exist:" << directory;
            continue;
        }

        // 设置名称过滤器，获取所有图标文件
        dir.setNameFilters(iconExtensions);
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);

        QStringList files = dir.entryList();

        for (const QString &file : files) {
            iconNames.append(QFileInfo(file).baseName());
        }
    }

    return iconNames;
}

QStringList CustomIconDialog::initializeIconList()
{
    return QStringList()
            << "audio-headphones" << "audio-headset" << "audio-card"
            << "blueman-device" << "block-device" << "camera" << "computer"
            << "drive-harddisk" << "drive-harddisk-ieee1394" << "drive-harddisk-usb"
            << "drive-multidisk" << "drive-optical" << "drive-removable-media-usb"
            << "input-gaming" << "input-keyboard" << "input-mouse"
            << "media-flash" << "media-floppy" << "media-optical"
            << "media-optical-cd-audio" << "media-optical-dvd" << "multimedia-player"
            << "phone" << "printer-network" << "cupsprinter"
            << "folder" << "folder-documents" << "folder-download"
            << "folder-dropbox" << "folder-music" << "folder-pictures"
            << "folder-publicshare" << "document-open-recent" << "folder-remote"
            << "folder-templates" << "folder-user" << "folder-videos"
            << "network-server" << "network-workgroup" << "kylin-startmenu"
            << "user-desktop" << "user-home" << "user-trash"
            << "user-trash-full" << "ukui-lock" << "ukui-tounlock" << "ukui-unlock";
}
