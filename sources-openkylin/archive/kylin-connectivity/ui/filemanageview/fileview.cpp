#include "fileview.h"

#include <QDebug>
#include <QFileInfo>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include "generatetools.h"
#include "fileviewdelegate.h"

const int TAG_INFO = 100;
const int ICON_MODE_ICON_WIDTH = 64;
const int ICON_MODE_ICON_HEIGHT = 64;
const int LIST_MODE_ICON_WIDTH = 32;
const int LIST_MODE_ICON_HEIGHT = 32;
const int ICON_MODE_ITEM_WIDTH = 96;
const int ICON_MODE_ITEM_HEIGHT = 104;
const int LIST_MODE_ITEM_HEIGHT = 56;
const int ICON_MODE_SPACING = 16;
const int LIST_MODE_SPACING = 4;
const QString CONFIG_PATH = getenv("HOME") + QString("/.connectivitycache/") + QString("setting.ini");
const QString CONFIG_KEY = "LastFilePath";
const QString DEFAULT_DOWNLOAD_PATH = getenv("HOME") + QString("/下载");

FileView::FileView(QWidget *parent) : kdk::KListView(parent)
{
    m_listItemHeight = LIST_MODE_ITEM_HEIGHT;
    setContentsMargins(0, 0, 0, 0);
    setStyleSheet("QListView{background:transparent;}");
    // 自适应布局
    setResizeMode(QListView::Adjust);
    // 列表中的图标不可拖动
    setMovement(QListView::Static);
    // 去边框
    setFrameShape(QListView::NoFrame);
    // 不可编辑
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置选中模式为选中行
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // 设置滑动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // 设置项大小相同
    setUniformItemSizes(true);
    // 设置右键菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    // 设置接收鼠标拖进事件
    setAcceptDrops(true);
    setDragDropMode(QListView::DragDrop);
    // 注册文件信息类型
    FileInfo::registerMetaType();

    m_listModel = new QStandardItemModel(this);
    setModel(m_listModel);
    FileViewDelegate *delegate = new FileViewDelegate(this);
    setItemDelegate(delegate);

    connect(this, &FileView::doubleClicked, this, &FileView::slotDoubleClicked);
    connect(this, &FileView::clicked, this, &FileView::slotClicked);
    connect(this, &FileView::customContextMenuRequested, this, &FileView::slotContextMenu);
}

void FileView::setViewMode(QListView::ViewMode mode)
{
    m_viewModel = mode;
    setIconSize(getIconSize());
    if (m_viewModel == QListView::IconMode) {
        setSpacing(ICON_MODE_SPACING);
    } else {
        setSpacing(LIST_MODE_SPACING);
    }
    updateViewMode();
    kdk::KListView::setViewMode(m_viewModel);
    update();
}

QListView::ViewMode FileView::getViewMode() const
{
    return m_viewModel;
}

void FileView::setModelFlag(PublicAttributes::Model model)
{
    m_model = model;
}

void FileView::setViewInfo(const QMap<QString, FileInfo> &map)
{
    QMap<QString, FileInfo> dirMap;   // 临时存储目录项
    QMap<QString, FileInfo> fileMap;  // 临时存储非目录项
    for (FileInfo info : map) {
        if (info.type() == FileInfo::FileType::Dir) {
            dirMap.insert(info.name(), info);
        } else {
            // 不是文件夹
            fileMap.insert(info.name(), info);
        }
    }
    setInfo(dirMap);
    setInfo(fileMap);

    return;
}

void FileView::setViewInfo(const QList<FileInfo> &list)
{
    QList<FileInfo> dirList;   // 临时存储目录项
    QList<FileInfo> fileList;  // 临时存储非目录项
    for (FileInfo info : list) {
        if (info.type() == FileInfo::FileType::Dir) {
            dirList.append(info);
        } else {
            // 不是文件夹
            fileList.append(info);
        }
    }
    setInfo(dirList);
    setInfo(fileList);

    return;
}

void FileView::setDownloadBtn(bool isShow)
{
    if (isShow) {
        if (m_downloadBtn != nullptr) {
            return;
        }
        m_downloadBtn = new LevitationButton(this);
        connect(m_downloadBtn, &LevitationButton::clicked, this, &FileView::slotDownloadBtnClicked);
        m_downloadBtn->setIcon(":/filemanageview/download.svg");
        m_downloadBtn->setText(tr("Download"));
        m_downloadBtn->move(
            QPoint(this->width() / 2 - m_downloadBtn->width() / 2, this->height() - 30 - m_downloadBtn->height()));
        m_downloadBtn->changeFontSize(m_fontSize);
        m_downloadBtn->show();
    } else {
        if (m_downloadBtn != nullptr) {
            m_downloadBtn->deleteLater();
            m_downloadBtn = nullptr;
        }
    }
}

void FileView::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    update();
}

PublicAttributes::Theme FileView::getTheme() const
{
    return m_theme;
}

void FileView::changeFontSize(double fontSize)
{
    m_listItemHeight = m_listItemHeight + ((fontSize - m_fontSize) * 2);
    m_fontSize = fontSize;
    if (m_downloadBtn != nullptr) {
        m_downloadBtn->changeFontSize(m_fontSize);
    }
    QFont font;
    font.setPointSizeF(fontSize);
    setFont(font);
    updateViewMode();
}

double FileView::getFontSize() const
{
    return m_fontSize;
}

void FileView::setItemMode(FileView::ItemSelectMode mode, bool isClear)
{
    m_itemMode = mode;
    if (isClear) {
        clearSelection();
        setDownloadBtn(false);
    }
    if (m_itemMode == ItemSelectMode::Default) {
        QListView::setSelectionMode(QAbstractItemView::ExtendedSelection);
    } else {
        QListView::setSelectionMode(QAbstractItemView::MultiSelection);
    }
    update();
}

FileView::ItemSelectMode FileView::getItemMode() const
{
    return m_itemMode;
}
void FileView::setThumbnailType(ThumbnailType type)
{
    m_thumbnailType = type;
}

void FileView::setItemThumbnail(QString filePath)
{
    if (m_itemMap.contains(filePath)) {
        QPixmap thumbnail = QPixmap(filePath);
        thumbnail = thumbnail.scaled(getIconSize(), Qt::KeepAspectRatio);
        if (m_thumbnailType == ThumbnailType::Image) {
            QIcon icon;
            if (thumbnail.isNull()) {
                icon = QIcon::fromTheme("image-x-generic");
            } else {
                icon = QIcon(thumbnail);
            }
            m_itemMap.find(filePath).value()->setIcon(icon);
        } else if (m_thumbnailType == ThumbnailType::Video) {
            QIcon icon;
            if (thumbnail.isNull()) {
                icon = QIcon(":/filemanageview/videodefault.svg");
            } else {
                QPixmap playPixmap(":/filemanageview/play.svg");
                if (m_viewModel == QListView::IconMode) {
                    playPixmap = playPixmap.scaled(QSize(36, 36), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                } else {
                    playPixmap = playPixmap.scaled(QSize(24, 24), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                QPainter imagepainter(&thumbnail);
                imagepainter.setCompositionMode(QPainter::CompositionMode_SourceOver);  // 设置重叠效果
                imagepainter.drawPixmap((thumbnail.width() - playPixmap.width()) / 2,
                                        (thumbnail.height() - playPixmap.height()) / 2, playPixmap);
                imagepainter.end();
                icon = QIcon(thumbnail);
            }
            m_itemMap.find(filePath).value()->setIcon(icon);
        }
    }
}

void FileView::setAllowDrag(bool isAllowDrag)
{
    m_isAllowDrag = isAllowDrag;
}

void FileView::mousePressEvent(QMouseEvent *event)
{
    if (!indexAt(event->pos()).isValid()) {
        if (QApplication::keyboardModifiers() != Qt::ControlModifier ||
            event->source() != Qt::MouseEventSynthesizedByQt) {
            clearSelection();
            setDownloadBtn(false);
        }
    }
    kdk::KListView::mousePressEvent(event);
}

void FileView::mouseReleaseEvent(QMouseEvent *event)
{
    QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.size() >= 2) {
        if (list.size() == m_listModel->rowCount()) {
            Q_EMIT sigItemSelectAll(true);
        } else {
            Q_EMIT sigItemSelectAll(false);
        }
        setItemMode(FileView::ItemSelectMode::Edit);
        setDownloadBtn(true);
    } else {
        if (m_itemMode == ItemSelectMode::Edit) {
            Q_EMIT sigItemSelectAll(false);
        }
    }
    kdk::KListView::mouseReleaseEvent(event);
}

void FileView::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_A)) {
        Q_EMIT sigItemSelectAll(true);
        setItemMode(FileView::ItemSelectMode::Edit);
        setDownloadBtn(true);
        Q_EMIT sigCtrlAClicked();
    }
    kdk::KListView::keyPressEvent(event);
}

void FileView::dragEnterEvent(QDragEnterEvent *event)
{
    if (m_isAllowDrag && m_itemMode == ItemSelectMode::Default && event->mimeData()->hasUrls()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void FileView::dragMoveEvent(QDragMoveEvent *event)
{
    auto index = indexAt(event->pos());
    if (index.isValid()) {
        QHoverEvent he(QHoverEvent::HoverMove, event->posF(), event->posF());
        viewportEvent(&he);
    } else {
        QHoverEvent he(QHoverEvent::HoverLeave, event->posF(), event->posF());
        viewportEvent(&he);
    }
    event->accept();
}

void FileView::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();
    qInfo() << "drag urls" << urls;
    if (urls.isEmpty()) {
        return;
    }

    QStringList localpath;
    for (auto &url : urls) {
        localpath << url.toLocalFile();
    }
    QString dirName = "";
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        FileInfo fileInfo = index.data(Qt::DisplayRole + TAG_INFO).value<FileInfo>();
        if (fileInfo.type() == FileInfo::FileType::Dir) {
            dirName = fileInfo.name();
        }
    }
    Q_EMIT sigUploadFileInfo(dirName, localpath);
}

void FileView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_downloadBtn != nullptr) {
        m_downloadBtn->move(
            QPoint(this->width() / 2 - m_downloadBtn->width() / 2, this->height() - 30 - m_downloadBtn->height()));
    }

    return kdk::KListView::resizeEvent(event);
}

void FileView::setInfo(const QMap<QString, FileInfo> &map)
{
    for (FileInfo info : map) {
        QStandardItem *item = new QStandardItem;
        if (info.type() == FileInfo::FileType::Txt) {
            item->setIcon(QIcon::fromTheme("text"));
        } else if (info.type() == FileInfo::FileType::Video) {
            item->setIcon(QIcon::fromTheme("video"));
        } else if (info.type() == FileInfo::FileType::Image) {
            item->setIcon(QIcon::fromTheme("image-x-generic"));
        } else if (info.type() == FileInfo::FileType::Audio) {
            item->setIcon(QIcon::fromTheme("audio-x-generic"));
        } else if (info.type() == FileInfo::FileType::Dir) {
            item->setIcon(QIcon::fromTheme("folder"));
        } else {
            item->setIcon(QIcon::fromTheme(info.iconName(info.getContentType())));
        }
        // 设置文件名
        item->setData(info.name(), Qt::DisplayRole);
        // 标记文件信息
        item->setData(info, Qt::DisplayRole + TAG_INFO);
        item->setToolTip(info.name());
        m_itemMap.insert(info.getThumbnailPath(), item);
        m_listModel->appendRow(item);
    }
}

void FileView::setInfo(const QList<FileInfo> &list)
{
    for (FileInfo info : list) {
        QStandardItem *item = new QStandardItem;
        if (info.type() == FileInfo::FileType::Txt) {
            item->setIcon(QIcon::fromTheme("text"));
        } else if (info.type() == FileInfo::FileType::Video) {
            setVideoIcon(item, info.getThumbnailPath());
        } else if (info.type() == FileInfo::FileType::Image) {
            setPictureIcon(item, info.getThumbnailPath());
        } else if (info.type() == FileInfo::FileType::Audio) {
            item->setIcon(QIcon::fromTheme("audio-x-generic"));
        } else if (info.type() == FileInfo::FileType::Dir) {
            item->setIcon(QIcon::fromTheme("folder"));
        } else {
            item->setIcon(QIcon::fromTheme(info.iconName(info.getContentType())));
        }
        // 设置文件名
        item->setData(info.name(), Qt::DisplayRole);
        // 标记文件信息
        item->setData(info, Qt::DisplayRole + TAG_INFO);
        item->setToolTip(info.name());
        m_itemMap.insert(info.getThumbnailPath(), item);
        m_listModel->appendRow(item);
    }
}

void FileView::openFile(const QModelIndex &index)
{
    FileInfo fileInfo = index.data(Qt::DisplayRole + TAG_INFO).value<FileInfo>();
    Q_EMIT sigOpenFile(fileInfo.type(), fileInfo.getPath());
}

void FileView::sendDownloadInfo()
{
    QSettings setting(CONFIG_PATH, QSettings::IniFormat);
    QString lastPath = setting.value(CONFIG_KEY).toString();
    if (lastPath.isEmpty()) {
        lastPath = DEFAULT_DOWNLOAD_PATH;
    }
    QFileDialog fileDialog;
    QString filePath = fileDialog.getExistingDirectory(this, tr("Choose folder"), lastPath);  // Choose folder选择文件夹
    if (!filePath.isEmpty()) {
        setting.setValue(CONFIG_KEY, filePath);
    } else {
        return;
    }

    QList<FileInfo> fileList;
    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        FileInfo fileInfo = index.data(Qt::DisplayRole + TAG_INFO).value<FileInfo>();
        fileList.append(fileInfo);
    }

    Q_EMIT sigDownFileInfo(filePath, fileList);
}

void FileView::updateViewMode()
{
    if (m_listModel->rowCount() == 0) {
        return;
    }
    for (int i = 0; i < m_listModel->rowCount(); i++) {
        QStandardItem *item = m_listModel->item(i, 0);
        QModelIndex index = m_listModel->index(i, 0, QModelIndex());
        if (m_viewModel == QListView::IconMode) {
            item->setSizeHint(QSize(ICON_MODE_ITEM_WIDTH, ICON_MODE_ITEM_HEIGHT));
        } else {
            item->setSizeHint(QSize(width(), m_listItemHeight));
            FileInfo fileInfo = index.data(Qt::DisplayRole + TAG_INFO).value<FileInfo>();
            if (fileInfo.type() != FileInfo::FileType::Dir) {
                QString sizeStr = GenerateTools::intSizeToString(fileInfo.size());
                QVariant metaSize;
                metaSize.setValue(sizeStr);
                m_listModel->setData(index, metaSize, Qt::UserRole);
            }
        }
    }
}

QSize FileView::getIconSize()
{
    if (m_viewModel == QListView::IconMode) {
        return QSize(ICON_MODE_ICON_WIDTH, ICON_MODE_ICON_HEIGHT);
    } else {
        return QSize(LIST_MODE_ICON_WIDTH, LIST_MODE_ICON_HEIGHT);
    }
}

void FileView::setPictureIcon(QStandardItem *item, QString path)
{
    QIcon icon;
    QPixmap thumbnail = QPixmap(path);
    if (path.isEmpty()) {
        icon = QIcon::fromTheme("image-x-generic");
    } else {
        QFileInfo info(path);
        if (!info.exists()) {
            icon = QIcon::fromTheme("image-x-generic");
        } else {
            thumbnail = thumbnail.scaled(getIconSize(), Qt::KeepAspectRatio);
            if (thumbnail.isNull()) {
                icon = QIcon::fromTheme("image-x-generic");
            } else {
                icon = QIcon(thumbnail);
            }
        }
    }

    item->setIcon(icon);
}

void FileView::setVideoIcon(QStandardItem *item, QString path)
{
    QIcon icon;
    QPixmap thumbnail = QPixmap(path);
    QPixmap playPixmap(":/filemanageview/play.svg");
    if (path.isEmpty()) {
        icon = QIcon::fromTheme(":/filemanageview/videodefault.svg");
    } else {
        QFileInfo info(path);
        if (!info.exists()) {
            icon = QIcon::fromTheme(":/filemanageview/videodefault.svg");
        } else {
            thumbnail = thumbnail.scaled(getIconSize(), Qt::KeepAspectRatio);
            if (thumbnail.isNull()) {
                icon = QIcon::fromTheme(":/filemanageview/videodefault.svg");
            } else {
                if (m_viewModel == QListView::IconMode) {
                    playPixmap = playPixmap.scaled(QSize(36, 36), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                } else {
                    playPixmap = playPixmap.scaled(QSize(24, 24), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                QPainter imagepainter(&thumbnail);                                      // 新建画板
                imagepainter.setCompositionMode(QPainter::CompositionMode_SourceOver);  // 设置重叠效果
                imagepainter.drawPixmap((thumbnail.width() - playPixmap.width()) / 2,
                                        (thumbnail.height() - playPixmap.height()) / 2, playPixmap);
                imagepainter.end();
                icon = QIcon(thumbnail);
            }
        }
    }
    item->setIcon(icon);
}

void FileView::slotDoubleClicked(const QModelIndex &index)
{
    openFile(index);
}

void FileView::slotClicked(const QModelIndex &index)
{
    if (m_model == PublicAttributes::Model::Table && m_itemMode == ItemSelectMode::Default && m_isClicked) {
        FileInfo fileInfo = index.data(Qt::DisplayRole + TAG_INFO).value<FileInfo>();
        if (fileInfo.type() != FileInfo::FileType::Dir) {
            m_isClicked = true;
        }
        openFile(index);
    } else {
        setDownloadBtn(true);
    }
}

void FileView::slotDownloadBtnClicked()
{
    sendDownloadInfo();
}

void FileView::slotContextMenu(const QPoint &pos)
{
    setDownloadBtn(false);
    QModelIndexList list = selectionModel()->selectedIndexes();
    if (!list.empty()) {
        QMenu *menu = new QMenu(this);
        QAction *openAction = new QAction(QIcon::fromTheme("folder-open-symbolic"), tr("Open"), this);
        connect(openAction, &QAction::triggered, this, &FileView::slotOpenTriggered);
        QAction *downloadAction = new QAction(QIcon::fromTheme("folder-download-symbolic"), tr("Download"), this);
        connect(downloadAction, &QAction::triggered, this, &FileView::slotDownloadTriggered);
        if (list.size() == 1) {
            menu->addAction(openAction);
        }
        menu->addAction(downloadAction);
        menu->exec(QCursor::pos());  // 在当前鼠标位置显示
        clearSelection();

        delete menu;
        delete openAction;
        delete downloadAction;
    }
}

void FileView::slotOpenTriggered()
{
    QModelIndex index = currentIndex();
    openFile(index);
}

void FileView::slotDownloadTriggered()
{
    sendDownloadInfo();
}