#include <QTimer>
#include <PeonySideBarModel>
#include <QScrollBar>
#include <QHeaderView>
#include <QPainterPath>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolTip>
#include <QScreen>
#include <QWindow>
#include <QDBusInterface>
#include "kyfiledialog.h"
#include "peony-qt/controls/menu/side-bar-menu/side-bar-menu.h"
#include "peony-qt/side-bar-abstract-item.h"
#include "peony-qt/volume-manager.h"
#include "peony-qt/file-enumerator.h"
#include "peony-qt/file-utils.h"
#include "peony-qt/file-info.h"
#include "vfs-plugin-iface.h"
#include "vfs-plugin-manager.h"
#include "side-bar-abstract-item.h"
#include "global-settings.h"
#include "plugin-manager.h"
#include "../libqt5-ukui-style/settings/ukui-style-settings.h"
#include "debug.h"
#include "kyfiledialog.h"
#include "sidebar.h"

using namespace Peony;
using namespace UKUIFileDialog;
FileDialogComboBox::FileDialogComboBox(QWidget *parent):QComboBox(parent)
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();
        connect(settings, &QGSettings::changed, this, [=](const QString &key){
            pDebug << "key changed:" << key << this->size();
                m_styleChanged = true;
        });
    }

    connect(this, &FileDialogComboBox::setStyleChanged, this, [=](bool change){
        pDebug << "setStyleChanged m_styleChanged:" << change;
        m_styleChanged = change;
    });
//    QDBusInterface *interFace = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus());
//    if(interFace->isValid()){
//        connect(interFace, SIGNAL(mode_change_signal(bool)), this,  SLOT(tableModeChanged(bool)));
//    }
    m_minSize = this->minimumSize();
    pDebug << "combobox minsize...." << m_minSize;
}

void FileDialogComboBox::tableModeChanged(bool isTableMode)
{
    m_styleChanged = true;
}

QSize FileDialogComboBox::minimumSizeHint() const
{
    pDebug << "minimumSizeHint...." << m_styleChanged << this->size() << m_minSize;

    QScreen *screen = qApp->screenAt(QCursor::pos());
    if (!screen) {
        if (const QWindow *win = window() ? window()->windowHandle() : nullptr)
            screen = win->screen();
    }
    if (!screen)
        screen = qApp->primaryScreen();

    bool useCompactMinSize = m_styleChanged;
    if (screen) {
        useCompactMinSize = useCompactMinSize
            || (screen->availableSize().width() - 300) < QComboBox::minimumSizeHint().width();
    }

    if (useCompactMinSize) {
        Q_EMIT setStyleChanged(false);
        return m_minSize;//this->size();
    }
    QSize size = QComboBox::minimumSizeHint();

    //fixbug#480595
    return QSize(size.width() > 160 ? 160 : size.width(), size.height());
}


FileDialogSideBar::FileDialogSideBar(QWidget *parent) : QTreeView(parent)
{
    static SideBarStyle *global_style = new SideBarStyle;

    setIconSize(QSize(16, 16));

//    header()->setSectionResizeMode(QHeaderView::Stretch );
    header()->setStretchLastSection(false);
    header()->hide();

    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setFrameStyle(QFrame::NoFrame);
    setSortingEnabled(true);

    setProperty("useIconHighlightEffect", true);
    setProperty("highlightMode", true);

    setDragDropMode(QTreeView::NoDragDrop);
    setProperty("doNotBlur", true);
    viewport()->setProperty("doNotBlur", true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_Disabled, false);

    setExpandsOnDoubleClick(false);
    setMouseTracking(true);//追踪鼠标
    setAutoScrollMargin(0);


    m_delegate = new SideBarItemDelegate(this);
    setItemDelegate(m_delegate);

    m_model = new Peony::SideBarModel(this);
    m_proxyModel = new Peony::SideBarProxyFilterSortModel(this);
    m_proxyModel->setSourceModel(m_model);
    setModel(m_proxyModel);

    Peony::VolumeManager *volumeManager = Peony::VolumeManager::getInstance();
    connect(volumeManager,&Peony::VolumeManager::volumeAdded,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxyModel->invalidate();//display DVD device in real time.
    });
    connect(volumeManager,&Peony::VolumeManager::volumeRemoved,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxyModel->invalidate();//The drive does not display when the DVD device is removed.
        //pDebug << "volumeRemoved:" <<QToolTip::text();
        //fix abnormal pull out usb device tips not hide issue, link to bug#81190
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    });
    connect(volumeManager,&Peony::VolumeManager::driveDisconnected,this,[=](const std::shared_ptr<Peony::Drive> &drive){
        m_proxyModel->invalidate();//Multiple udisk eject display problem
    });
        connect(volumeManager,&Peony::VolumeManager::mountAdded,this,[=](const std::shared_ptr<Peony::Mount> &mount){
        m_proxyModel->invalidate();//display udisk in real time after format it.
    });


    connect(this, &FileDialogSideBar::clicked, this, [=](const QModelIndex &index){
        switch (index.column()) {
        case 0: {
            m_currSelectedItem = m_proxyModel->itemFromIndex(index);

            auto item = m_proxyModel->itemFromIndex(index);
            m_currSelectedItem->connect(m_currSelectedItem, &SideBarAbstractItem::destroyed, this, [=]{
                m_currSelectedItem = nullptr;
            });

            if (! item)
                break;
            //continue to fix crash issue, related to bug#116201,116589
            if(item->isMountable()&&!item->isMounted())
                item->mount();
            else{
                auto uri = item->uri();

                if (!item->uri().isEmpty()){
                    if (uri == "computer:///ukui-data-volume") {
                        uri = "file:///data";
                    }
                    Q_EMIT goToUriRequest(uri);
                }
            }
            break;
        }
        case 1: {
            auto item = m_proxyModel->itemFromIndex(index);
            if (item->isMounted() || item->isEjectable()||item->isStopable()) {
                auto leftIndex = m_proxyModel->index(index.row(), 0, index.parent());
                this->collapse(leftIndex);
                item->ejectOrUnmount();
            } else {
                // if item is not unmountable, just be same with first column.
                // fix #39716
                auto uri = item->uri();

                if (!item->uri().isNull()){
                    if (uri == "computer:///ukui-data-volume") {
                        uri = "file:///data";
                    }
                    Q_EMIT goToUriRequest(uri);
                }
            }
            break;
        }
        default:
            break;
        }
    });

    connect(this, &FileDialogSideBar::expanded, this, [=](const QModelIndex &index){
        auto item = m_proxyModel->itemFromIndex(index);
        /*!
          \bug can not expanded? enumerator can not get prepared signal, why?
          */
        bool isShowNetwork = Peony::GlobalSettings::getInstance()->isExist(SHOW_NETWORK) ?
                    Peony::GlobalSettings::getInstance()->getValue(SHOW_NETWORK).toBool() : true;
        if (item->type() == Peony::SideBarAbstractItem::NetWorkItem && !isShowNetwork) {
            this->setRowHidden(index.row(), index.parent(), true);
            return;
        }

        QStringList disExtensions = Peony::GlobalSettings::getInstance()->getValue(DISABLED_EXTENSIONS).toStringList();
        for (auto extensions : disExtensions) {
            Peony::VFSPluginIface *pIface = dynamic_cast<Peony::VFSPluginIface*>(Peony::PluginManager::getInstance()->getPluginByFileName(extensions));
            if (pIface && pIface->pluginType() == Peony::PluginInterface::VFSPlugin
                    && item->type() == Peony::SideBarAbstractItem::FileSystemItem
                    && !item->uri().contains("computer:///")
                    && item->uri().contains(pIface->uriScheme())) {
                this->setRowHidden(index.row(), index.parent(), true);
                return;
            }

            if (pIface && pIface->pluginType() == PluginInterface::VFSPlugin
                    && item->type() == SideBarAbstractItem::FavoriteItem
                    && pIface->uriScheme() == "kmre://"
                    && item->uri().contains(pIface->uriScheme())) {
                this->setRowHidden(index.row(), index.parent(), true);
                return;
            }
        }

        item->findChildrenAsync();
    });

    connect(this, &FileDialogSideBar::collapsed, this, [=](const QModelIndex &index){
        auto item = m_proxyModel->itemFromIndex(index);
        item->clearChildren();
    });

    connect(this, &QTreeView::customContextMenuRequested, this, [=](const QPoint &pos){
        auto index = indexAt(pos);
        auto item = m_proxyModel->itemFromIndex(index);
        if (item) {
            if (item->type() != Peony::SideBarAbstractItem::SeparatorItem) {
                Peony::SideBarMenu menu(item, nullptr, parent ? parent : this);
                QList<QAction *> actionList;
                if (item->type() == Peony::SideBarAbstractItem::FileSystemItem) {
                    if ((0 != QString::compare(item->uri(), "computer:///")) &&
                        (0 != QString::compare(item->uri(), "filesafe:///"))) {
                        for (const auto &actionItem : actionList) {
                            if(item->isVolume())/* 分区才去需要判断是否已挂载 */
                                actionItem->setEnabled(item->isMounted());
                        }
                    }
                }

                menu.exec(QCursor::pos());
            }
        }
    });

    connect(Peony::GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=](const QString& key){
        if (SHOW_NETWORK == key) {
            for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
                auto index = m_proxyModel->index(i, 0);
                auto item = m_proxyModel->itemFromIndex(index);
                bool isShowNetwork = Peony::GlobalSettings::getInstance()->isExist(SHOW_NETWORK) ?
                            Peony::GlobalSettings::getInstance()->getValue(SHOW_NETWORK).toBool() : true;
                if (item->type() == SideBarAbstractItem::NetWorkItem) {
                    this->setRowHidden(index.row(), index.parent(), !isShowNetwork);
                    if (!isShowNetwork) {
                        item->findChildrenAsync();
                    }
                    return;
                }
            }
            this->viewport()->update();
        }
    });

    connect(Peony::VFSPluginManager::getInstance(), &Peony::VFSPluginManager::updateVFSPlugin, this, [=](VFSPluginIface *vfsPIface, bool enable){
//        qDebug() << __func__ << vfsPIface->name() << enable;
        for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
            auto index = m_proxyModel->index(i, 0);
            auto item = m_proxyModel->itemFromIndex(index);
            if (item->type() == SideBarAbstractItem::VFSItem
                    && item->uri().contains(vfsPIface->uriScheme())
                    && !item->uri().contains("computer:///")
                    && vfsPIface->pluginType() == PluginInterface::VFSPlugin) {
                if (m_currSelectedItem) {
                    if (!m_currSelectedItem->uri().startsWith("file://") && enable) {
                        JumpDirectory("computer:///");
                    }
                }
                this->setRowHidden(index.row(), index.parent(), enable);
                return;
            }
            //hide kmre
            if (item->type() == SideBarAbstractItem::FavoriteItem
                    && vfsPIface->uriScheme() == "kmre://"
                    && vfsPIface->pluginType() == PluginInterface::VFSPlugin) {
                if (m_currSelectedItem) {
                    if (!m_currSelectedItem->uri().startsWith("file:///") && enable) {
                        JumpDirectory("computer:///");
                    }
                }

                for (int j = 0; j < m_proxyModel->rowCount(index); ++j) {
                    auto tIndex = m_proxyModel->index(j, 0, index);
                    auto tItem = m_proxyModel->itemFromIndex(tIndex);
                    if (tItem->uri().startsWith("kmre:///")) {
                        this->setRowHidden(tIndex.row(), tIndex.parent(), enable);
                        return;
                    }
                }
            }
        }
    });

    connect(m_model, &QAbstractItemModel::dataChanged, this, [=](){
        this->viewport()->update();
        m_proxyModel->invalidate();
    });

//    QDBusInterface *interFace = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus());
//    if(interFace->isValid()){
//        connect(interFace, SIGNAL(mode_change_signal(bool)), this,  SLOT(tableModeChanged(bool)));
//    }

//    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
//        auto settings = UKUIStyleSettings::globalInstance();
//        connect(settings, &QGSettings::changed, this, [=](const QString &key){
//            pDebug << "key changed:" << key << width();
//            if (key == "styleName") {
//                m_siderWidth = this->width();
//            }
//        });
//    }
//    expandToDepth(1);
    connect(VFSPluginManager::getInstance(), &VFSPluginManager::updateVFSPlugin, this, [=](VFSPluginIface *vfsPIface, bool enable){
//        qDebug() << __func__ << vfsPIface->name() << enable;
        for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
            auto index = m_proxyModel->index(i, 0);
            auto item = m_proxyModel->itemFromIndex(index);
            if (item->type() == SideBarAbstractItem::VFSItem
                    && item->uri().contains(vfsPIface->uriScheme())
                    && !item->uri().contains("computer:///")
                    && vfsPIface->pluginType() == PluginInterface::VFSPlugin) {
                if (m_currSelectedItem) {
                    if (!m_currSelectedItem->uri().startsWith("file://") && enable) {
                        JumpDirectory("computer:///");
                    }
                }
                this->setRowHidden(index.row(), index.parent(), enable);
                return;
            }
            //hide kmre
            if (item->type() == SideBarAbstractItem::FavoriteItem
                    && vfsPIface->uriScheme() == "kmre://"
                    && vfsPIface->pluginType() == PluginInterface::VFSPlugin) {
                if (m_currSelectedItem) {
                    if (!m_currSelectedItem->uri().startsWith("file:///") && enable) {
                        JumpDirectory("computer:///");
                    }
                }

                for (int j = 0; j < m_proxyModel->rowCount(index); ++j) {
                    auto tIndex = m_proxyModel->index(j, 0, index);
                    auto tItem = m_proxyModel->itemFromIndex(tIndex);
                    if (tItem->uri().startsWith("kmre:///")) {
                        this->setRowHidden(tIndex.row(), tIndex.parent(), enable);
                        return;
                    }
                }
            }
        }
    });

    connect(m_model, &SideBarModel::signal_collapsedChildren, this, [=](const QModelIndex &index){
        QModelIndex modelIndex = m_proxyModel->mapFromSource(index);
        collapse(modelIndex);
    });

    //expandToDepth(1);/* 快速访问、计算机、网络 各模块往下展开一层 */
    for(int row =0; row < model()->rowCount();row++)
    {
        auto index = model()->index(row,0);
        auto srcIndex = m_proxyModel->mapToSource(index);
        auto item = m_model->itemFromIndex(srcIndex);
        auto type = item->type();
        if (item->type() != SideBarAbstractItem::VFSItem && item->type() != SideBarAbstractItem::SeparatorItem) {
            doExpand(index);
        }

        QStringList disExtensions = GlobalSettings::getInstance()->getValue(DISABLED_EXTENSIONS).toStringList();
        for (auto extensions : disExtensions) {
            VFSPluginIface *pIface = dynamic_cast<VFSPluginIface*>(PluginManager::getInstance()->getPluginByFileName(extensions));
            if (pIface && pIface->pluginType() == PluginInterface::VFSPlugin
                    && item->type() == SideBarAbstractItem::VFSItem
                    && !item->uri().contains("computer:///")
                    && item->uri().contains(pIface->uriScheme())) {
                this->setRowHidden(index.row(), index.parent(), true);
            }

            if (pIface && pIface->pluginType() == PluginInterface::VFSPlugin
                    && item->type() == SideBarAbstractItem::FavoriteItem
                    && pIface->uriScheme() == "kmre://"
                    && item->uri().contains(pIface->uriScheme())) {
                this->setRowHidden(index.row(), index.parent(), true);
            }
        }

//        if(item->uri()=="filesafe:///")/* 文件保护箱默认不展开 */
//            continue;
//        expand(index);
    }

    setStyle(global_style);
    setViewportMargins(8, 4, 4, 4);

    pDebug << "columndffffffffffffffff";
//    setColumnHidden(1, true);
}

FileDialogSideBar::~FileDialogSideBar()
{
    if(m_proxyModel){
        m_proxyModel->deleteLater();
        m_proxyModel = nullptr;
    }
    if(m_model){
        m_model->deleteLater();
        m_model = nullptr;
    }
    if(m_delegate){
        m_delegate->deleteLater();
        m_delegate = nullptr;
    }
}

void FileDialogSideBar::tableModeChanged(bool isTableMode)
{
    pDebug << "tableModeChanged........";
//    setStyle(SideBarStyle::getStyle());
}

void FileDialogSideBar::resizeEvent(QResizeEvent *e)
{
    setViewportMargins(8, 4, 4, 4);

//    this->setGeometry(this->x(), this->y(), m_siderWidth, this->height());
    QTreeView::resizeEvent(e);
    pDebug << "FileDialogSideBar resizeEvent header count......:" << header()->count() << this->viewport()->width() << this->columnWidth(0) << this->columnWidth(1);
    if (header()->count() > 0) {
        this->setColumnWidth(1, 20);
        header()->resizeSection(0, this->viewport()->width() - this->columnWidth(1));
    }
}

QSize FileDialogSideBar::sizeHint() const
{
    return QTreeView::sizeHint();
}

void FileDialogSideBar::menuRequest(const QPoint &pos)
{
//    auto index = indexAt(pos);
//    auto item = proxyModel->itemFromIndex(index);
//    if (item) {
//        pDebug << "sider bar menu00000" << item->type() << index;
//        if (item->type() != Peony::SideBarAbstractItem::SeparatorItem) {
//            pDebug << "sider bar menu11111";
////            Peony::SideBarMenu *menu = new Peony::SideBarMenu(item, this);
//            pDebug << "sider bar menu22222222";
////            menu->exec(QCursor::pos());
//            pDebug << "sider bar menu3333333";
//        }
//    }
}

void FileDialogSideBar::paintEvent(QPaintEvent *event)
{
    setViewportMargins(8, 4, 4, 4);

    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Base, QColor(Qt::transparent));
    setPalette(pal);
    viewport()->setPalette(pal);
    QTreeView::paintEvent(event);
}

void FileDialogSideBar::updateModel()
{
    Peony::GlobalSettings::getInstance()->forceSync();
    QMap <QString, QVariant> remoteServer = Peony::GlobalSettings::getInstance()->getValue(REMOTE_SERVER_REMOTE_IP).toMap ();

    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        auto index = m_proxyModel->index(i, 0);
        auto item = m_proxyModel->itemFromIndex(index);
        bool isShowNetwork = Peony::GlobalSettings::getInstance()->isExist(SHOW_NETWORK) ?
                    Peony::GlobalSettings::getInstance()->getValue(SHOW_NETWORK).toBool() : true;
        if (item->type() == SideBarAbstractItem::NetWorkItem) {
            this->setRowHidden(index.row(), index.parent(), !isShowNetwork);
            if (!isShowNetwork) {
                item->findChildrenAsync();
            }
            if(this->isExpanded(index))
            {
                this->collapse(index);
                this->expand(index);
            }
        }
    }
    this->viewport()->update();
}

void FileDialogSideBar::JumpDirectory(const QString &uri)
{
    if((uri=="" || uri.startsWith("computer://")) && m_currSelectedItem && m_currSelectedItem->getDevice().startsWith("/dev/sd"))
    {/* 异常U盘 */
        QMessageBox::information(nullptr, tr("Tips"), tr("This is an abnormal Udisk, please fix it or format it"));
        return;
    }

    auto info = Peony::FileInfo::fromUri(uri);

    // try fix #174128, peony stucked when click sidebar network item sometimes.
//    if (!uri.startsWith("network:///")) {
//        if (info.get()->isEmptyInfo()) {
//            Peony::FileInfoJob j(info);
//            j.querySync();
//        }
//    }

    auto targetUri = Peony::FileUtils::getTargetUri(uri);
    if (targetUri == "" && uri== "burn://")
    {
//        qDebug() << "empty drive"<<uri;
        QMessageBox::information(nullptr, tr("Tips"), tr("This is an empty drive, please insert a Disc."));
        return;
    }

    if (!targetUri.isEmpty()) {
        Q_EMIT this->goToUriRequest(targetUri);
        return;
    }

    // try fixing #133429.
    if (m_currSelectedItem->getDevice().startsWith("/dev/sr") && uri.startsWith("computer://")) {
        return;
    }

    //some side bar item doesn't have a uri.
    //do not emit signal with a null uri to window.
    if (!uri.isNull())
        Q_EMIT this->goToUriRequest(uri);
}

void FileDialogSideBar::doExpand(const QModelIndex &index)
{
    if (isExpanded(index))
        return;
    auto item = m_proxyModel->itemFromIndex(index);
    connect(item, &SideBarAbstractItem::findChildrenFinished, this, [=]{
        auto currentIndex = m_proxyModel->mapFromSource(item->firstColumnIndex());
        this->expand(currentIndex);
    });
    item->findChildrenAsync();

    auto globalSettings = Peony::GlobalSettings::getInstance();
    bool isShowNetwork = globalSettings->isExist(SHOW_NETWORK) ? globalSettings->getValue(SHOW_NETWORK).toBool() : true;
    QStringList disExtensions = globalSettings->getValue(DISABLED_EXTENSIONS).toStringList();
    // 获取索引项并缓存到局部变量
    auto itemType = item->type();
    auto itemUri = item->uri();

    if (itemType == SideBarAbstractItem::NetWorkItem && !isShowNetwork) {
        // 如果不显示网络项目，则隐藏该行并退出
        this->setRowHidden(index.row(), index.parent(), true);
        return;
    }

    // 使用 QMap 缓存插件实例
    PluginManager* pluginManager = PluginManager::getInstance();
    QMap<QString, VFSPluginIface*> cachedPlugins;

    // 遍历禁用的扩展列表
    for (auto extension : disExtensions) {
        // 如果插件未缓存，则动态转换并缓存
        if (!cachedPlugins.contains(extension)) {
            VFSPluginIface* pIface = dynamic_cast<VFSPluginIface*>(pluginManager->getPluginByFileName(extension));
            cachedPlugins.insert(extension, pIface);
        } else {
            auto pIface = cachedPlugins[extension];

            if (pIface && pIface->pluginType() == PluginInterface::VFSPlugin) {
                bool isFileSystemItem = (itemType == SideBarAbstractItem::FileSystemItem);
                bool isFavoriteItem = (itemType == SideBarAbstractItem::FavoriteItem);

                if ((isFileSystemItem && !itemUri.contains("computer:///") && itemUri.contains(pIface->uriScheme())) ||
                    (isFavoriteItem && pIface->uriScheme() == "kmre://" && itemUri.contains(pIface->uriScheme()))) {
                    this->setRowHidden(index.row(), index.parent(), true);
                    return;
                }
            }
        }
    }
}

void FileDialogSideBar::doCollapse(const QModelIndex &index)
{
    if (!isExpanded(index))
        return;
    auto item = m_proxyModel->itemFromIndex(index);
    this->collapse(index);
    QTimer::singleShot(qApp->style()->styleHint(QStyle::SH_Widget_Animation_Duration), item, [=]{
        item->clearChildren();
    });
}

void FileDialogSideBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        auto index = this->indexAt(event->pos());
        if (index.isValid()) {
            // 计算展开按钮位置
            auto rect = visualRect(index);
            int identation = indentation();
            rect.setWidth(identation);
            rect.translate(-identation, 0);
            if (rect.contains(event->pos())) {
                // 展开/收起
                if (isExpanded(index)) {
                    doCollapse(index);
                } else {
                    doExpand(index);
                }
                event->ignore();
                return;
            }
        }
    }

    return QTreeView::mousePressEvent(event);
}

void FileDialogSideBar::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left||event->key()==Qt::Key_Right)
    {
        auto index = currentIndex();
        if (event->key() == Qt::Key_Left) {
            doCollapse(index);
        } else {
            doExpand(index);
        }
        return;
    }

    QTreeView::keyPressEvent(event);

//    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
//        if (!selectedIndexes().isEmpty()) {
//            auto index = selectedIndexes().first();
//            auto uri = index.data(Qt::UserRole).toString();
//            Q_EMIT this->updateWindowLocationRequest(uri, true);
//        }
//    }

    if(event->key() == Qt::Key_Left||event->key()==Qt::Key_Right)
    {/* 按下左右键不可使侧边栏内容左右平移显示 */
        horizontalScrollBar()->setValue(0);/* hotfix bug#93557 */
    }
}

//void FileDialogSideBar::paintEvent(QPaintEvent *e)
//{
//    QColor color = this->palette().window().color();
//    QColor colorBase = QColor(Qt::red); //this->palette().base().color();

//    int R1 = color.red();
//    int G1 = color.green();
//    int B1 = color.blue();
//    qreal a1 = 0.3;

//    int R2 = colorBase.red();
//    int G2 = colorBase.green();
//    int B2 = colorBase.blue();
//    qreal a2 = 1;

//    qreal a = 1 - (1 - a1)*(1 - a2);

//    qreal R = (a1*R1 + (1 - a1)*a2*R2) / a;
//    qreal G = (a1*G1 + (1 - a1)*a2*G2) / a;
//    qreal B = (a1*B1 + (1 - a1)*a2*B2) / a;

//    colorBase.setRed(R);
//    colorBase.setGreen(G);
//    colorBase.setBlue(B);
//    colorBase.setAlphaF(1);

////    QPainter p(this);
////    QPainterPath sidebarPath;
////    sidebarPath.addRoundedRect(this->geometry(), 6, 6);
////    p.fillPath(sidebarPath, colorBase);
////    viewport()->setObjectName("viewport");
////    viewport()->setStyleSheet("QWidget#viewport{background-color:rgba(255,255,255,0.5)");////" + QString::number(colorBase.red()) + "," + QString::number(colorBase.green()) + "," + QString::number(colorBase.blue()) + "," + QString::number(colorBase.alpha()) + "}");
//    QTreeView::paintEvent(e);
//}


SideBarItemDelegate::SideBarItemDelegate(QObject *parent)
{
//    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
//        auto settings = UKUIStyleSettings::globalInstance();
//        connect(settings, &QGSettings::changed, this, [=](const QString &key){
//            pDebug << "key changed:" << key;
//            if (key == "styleName") {
//                pDebug << "key changed....";
//                this->paint();
//                this->update();
//            }
//        });
//    }
}

QSize SideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(36);
    return size;
}


void SideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    painter->save();
//    if (index.column() == 1) {
//        QPainterPath rightRoundedRegion;
//        rightRoundedRegion.setFillRule(Qt::WindingFill);
//        auto rect = option.rect;
//        auto view = qobject_cast<const QAbstractItemView *>(option.widget);
//        if (view) {
//            rect.setRight(view->viewport()->rect().right());
//        }
//        rightRoundedRegion.addRoundedRect(rect, 4, 4);
//        rightRoundedRegion.addRect(rect.adjusted(0, 0, -4, 0));
//        painter->setClipPath(rightRoundedRegion);
//    }

//    painter->setRenderHint(QPainter::Antialiasing);
//    QStyledItemDelegate::paint(painter, option, index);
//    painter->restore();

    QStyledItemDelegate::paint(painter, option, index);
}

SideBarStyle::SideBarStyle()
{

}

void SideBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    switch (element) {
    case QStyle::PE_IndicatorItemViewItemDrop: {
        painter->setRenderHint(QPainter::Antialiasing, true);/* 反锯齿 */
        /* 按设计要求，边框颜色为调色板highlight值，圆角为6px */
        QColor color = option->palette.color(QPalette::Highlight);
        painter->setPen(color);
        painter->drawRoundedRect(option->rect, 6, 6);
        painter->restore();
        return;
    }

    case QStyle::PE_IndicatorBranch: {
        if (option->rect.x() == 0) {
            QPainterPath leftRoundedRegion;
            leftRoundedRegion.setFillRule(Qt::WindingFill);
            leftRoundedRegion.addRoundedRect(option->rect, 4, 4);
            leftRoundedRegion.addRect(option->rect.adjusted(4, 0, 0, 0));
            painter->setClipPath(leftRoundedRegion);
        }
        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
//                opt.palette.setColor(QPalette::Highlight, Qt::red);
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
//                opt.palette.setColor(QPalette::Highlight, Qt::green);
            }
        }
        qApp->style()->drawPrimitive(element, &opt, painter, widget);
        painter->restore();
        return;
    }
    case QStyle::PE_PanelItemViewRow: {
        painter->restore();
        return;
        break;
    }
    case QStyle::PE_PanelItemViewItem: {
        /*
        const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *vi;
        painter->save();
        painter->setPen(Qt::NoPen);
        QPainterPath path;
        if (qobject_cast<const QTreeView*>(widget) || qobject_cast<const QTreeWidget*>(widget)) {
            path.addRoundedRect(vi->rect, 6, 6);
        }
        else {
            path.addRect(vi->rect);
        }
//                pDebug << "vi->rect:" << vi->rect;

        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
            }
        }
        painter->drawPath(path);
        */
        break;
    }

    default:
        break;
    }

    qApp->style()->drawPrimitive(element, option, painter, widget);
    painter->restore();
}

void SideBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::CE_ItemViewItem) {
        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
            }
        }
        return qApp->style()->drawControl(element, &opt, painter, widget);
    }
}

