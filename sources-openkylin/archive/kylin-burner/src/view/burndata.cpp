/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "burndata.h"
#include "ui_burndata.h"
#include "src/data/datadirectory.h"
#include "src/common/utils.h"
#include "src/po/translation.h"
#include "src/frame/logger.h"
#include "src/frame/signals.h"
#include "src/frame/configuration.h"
#include "src/frame/signals.h"
#include "src/model/menufactory.h"
#include "src/model/logicmain.h"
#include "src/view/execdialog.h"
#include "src/view/filefilterrules.h"
#include "src/view/viewfilterfiles.h"
#include "src/view/mymenu.h"
#include "src/view/mainwindow.h"

#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QToolButton>
#include <QFileSystemWatcher>
#include <QTabBar>
#include <QScrollBar>

#include <KWindowSystem>
#include <KF5/KWindowSystem/kwindowsystem.h>
#include <ktoolbutton.h>

BurnData *BurnData::m_oInstance = nullptr;
QMutex    BurnData::m_oMutex;
bool      BurnData::m_bDirReNameing = false;

#define ICON_SIZE QSize(16, 16)

MyTreeView::MyTreeView(QWidget *parent)
    :   QTreeView(parent)
{
    QScrollBar *vScroll = verticalScrollBar();
    QScrollBar *hScroll = horizontalScrollBar();

    vScroll->installEventFilter(this);
    hScroll->installEventFilter(this);

    // 初始化滚动条状态（避免启动时状态错误）
    emit verticalScrollBarVisibleChanged(verticalScrollBar()->isVisible());
    emit horizontalScrollBarVisibleChanged(horizontalScrollBar()->isVisible());
}

bool MyTreeView::eventFilter(QObject *watched, QEvent *event)
{
    QScrollBar *scrollBar = qobject_cast<QScrollBar*>(watched);
    if (!scrollBar) {
        return QTreeView::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Show || event->type() == QEvent::Hide) {
        bool isVisible = (event->type() == QEvent::Show);
        emitScrollStatusSignal(scrollBar, isVisible);
    }

    return false;
}

void MyTreeView::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void MyTreeView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {

        emit g_user_signal->sigMenuByRightMousePress();
        event->accept();
        return;
    }
    QTreeView::mousePressEvent(event);
}

void MyTreeView::emitScrollStatusSignal(QScrollBar *scrollBar, bool isVisible)
{
    if (scrollBar == verticalScrollBar()) {
        emit verticalScrollBarVisibleChanged(isVisible);
    } else if (scrollBar == horizontalScrollBar()) {
        emit horizontalScrollBarVisibleChanged(isVisible);
    }
}

BurnData::BurnData(QWidget *parent) :
    QWidget(parent),
    m_oModel(new LogicBurnData(this)),
    m_oOpenFile(new ProcessOpenFile(QString(), this)),
    ui(new Ui::BurnData)
{
    this->setObjectName("kylin-burner__BurnData");
    ui->setupUi(this);
    setAccessibleName();

    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        m_bIsWayland = true;
    } else {
        m_bIsWayland = false;
    }

    //this->setWindowFlags(Qt::WindowDoesNotAcceptFocus);
    title = new QLabel(this);
    title->setAccessibleName("kylin-burner_QLabel_BurnData_title");
    title->setFocusPolicy(Qt::NoFocus);
    title->setText(_("BurnData"));
    title->setGeometry(25,10,this->width()-50,40);
    title->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

    bottomLine = new kdk::KHLineFrame();
    bottomLine->setAccessibleName("kylin-burner_KHLineFrame_BurnData_bottomLine");
    ui->layoutBurnData->addWidget(bottomLine);

    mDirectoryCrumb   = new KBreadCrumb(this);
    mDirectoryCrumb->setAccessibleName("kylin-burner_KBreadCrumb_BurnData_mDirectoryCrumb");
    m_go_back = new KToolButton(this);
    m_go_back->setAccessibleName("kylin-burner_KToolButton_BurnData_goback");
    m_go_forward = new KToolButton(this);
    m_go_forward->setAccessibleName("kylin-burner_KToolButton_BurnData_goforward");
    mDirectoryMap = new QMap<int, QModelIndex>;

    m_go_back->setEnabled(false);
    m_go_back->setToolTip(_("Go Back"));
    m_go_back->setIcon(QIcon::fromTheme("go-next-rtl-symbolic"));
    m_go_back->setFixedSize(30, 30);
    m_go_back->setIconSize(QSize(16,16));
    m_go_back->setType(kdk::KToolButtonType::Flat);
    m_go_back->setCursor(Qt::PointingHandCursor);
    m_go_back->setFocusPolicy(Qt::NoFocus);

    m_go_forward->setEnabled(false);
    m_go_forward->setToolTip(_("Go Forward"));
    m_go_forward->setIcon(QIcon::fromTheme("go-next-symbolic"));
    m_go_forward->setFixedSize(30, 30);
    m_go_forward->setIconSize(QSize(16,16));
    m_go_forward->setType(kdk::KToolButtonType::Flat);
    m_go_forward->setCursor(Qt::PointingHandCursor);
    m_go_forward->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,5,0,2);
    hLayout->addWidget(m_go_back);
    hLayout->addWidget(m_go_forward);
    ui->link->insertLayout(0,hLayout);

    mDirectoryCrumb->setFlat(false);


    ui->link->insertWidget(1,mDirectoryCrumb);
    ui->link->addStretch();

    ui->btnAdd->setText(_("Add"));
    ui->btnAdd->setProperty("useIconHighlightEffect", 0x8);
    ui->btnAdd->setCheckable(true);

    connect(ui->btnAdd, &QPushButton::clicked, this, &BurnData::slotsAddFiles);
    connect(g_user_signal, &GlobalUserSignal::secondChangeDiscStatus, this, [&]()
    {
        ui->btnAdd->setEnabled(true);
    });


    ui->btnDelete->setText(_("Delete"));
    ui->btnDelete->setProperty("useIconHighlightEffect", 0x8);
    ui->btnDelete->setCheckable(true);
    connect(ui->btnDelete, &QPushButton::toggled, this, &BurnData::slotsDeleteFiles);

    ui->btnClean->setText(_("Clean"));
    ui->btnClean->setProperty("useIconHighlightEffect", 0x8);
    ui->btnClean->setCheckable(true);
    connect(ui->btnClean, &QPushButton::toggled, this, &BurnData::slotsCleanFiles);

    ui->btnNewFolder->setText(_("NewFolder"));
    ui->btnNewFolder->setProperty("useIconHighlightEffect", 0x8);
    ui->btnNewFolder->setCheckable(true);
    connect(ui->btnNewFolder, &QPushButton::toggled, this, &BurnData::slotsNewFolder);

    ui->linkViewFilterFile->hide();

    ui->labelWelcome->setText(_("Welcome to use KylinBurner."));
    ui->labelWelcome->hide();

    ui->btnTipAddFile->setProperty("isRoundButton",true);
    ui->btnTipAddFile->setIcon(QIcon::fromTheme("list-add-symbolic"));               //TODO... 图片名
    ui->btnTipAddFile->setCheckable(true);
    connect(ui->btnTipAddFile, &QPushButton::clicked, this, &BurnData::slotsAddFiles);
    connect(g_user_signal, &GlobalUserSignal::secondChangeDiscStatus, this, [&]()
    {
        ui->btnTipAddFile->setEnabled(true);
    });


    ui->labelProjectName->setText(_("ProjectSize:"));
    ui->labelProjectSize->setText(_("0.00 MB"));
    ui->labelProjectName->hide();               //隐藏项目大小
    ui->labelProjectSize->hide();

    CustomDelegate *delegate = new CustomDelegate(this);
    ui->treeView->setItemDelegate(delegate);

    ui->treeView->setModel(m_oModel);
    ui->treeView->header()->setFixedHeight(40);

    ui->tip->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->treeView->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->treeView->setProperty("useIconHighlightEffect", 0x8);
    m_oModel->setHeaderData(0, Qt::Horizontal, _("FileName"), Qt::DisplayRole);
    m_oModel->setHeaderData(1, Qt::Horizontal, _("FileType"), Qt::DisplayRole);
    m_oModel->setHeaderData(2, Qt::Horizontal, _("FileSize"), Qt::DisplayRole);
    m_oModel->setHeaderData(3, Qt::Horizontal, _("FilePath"), Qt::DisplayRole);
    m_oModel->setHeaderData(4, Qt::Horizontal, _("FileTime"), Qt::DisplayRole);

    ui->treeView->sortByColumn(4);
    ui->treeView->header()->setSectionHidden(4, true);
    ui->treeView->setSelectionMode(QTreeView::ExtendedSelection);
    ui->treeView->setItemsExpandable(false);
    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setSortingEnabled(false);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setAlternatingRowColors(true);                       //QTreeView行颜色交替

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &BurnData::slotChangeUIFontSize);
    connect(LogicMain::instance(), &LogicMain::systemFontnameChanged, this, &BurnData::slotChangeUIFontname);

    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &BurnData::slotsTreeMenuByRightKey);
    ui->tip_widget->installEventFilter(this);
#ifndef UNIT_TEST
    connect(m_oModel, &LogicBurnData::rootIndex, [=](QModelIndex idx)
    {
        if (!idx.isValid())
        {
            refresh();
            return;
        }

        ui->treeView->setRootIndex(idx);
        m_oAddIndex = idx;
        refresh();

    });

    connect(m_oModel, &LogicBurnData::rootIndex, [=](QModelIndex idx){
        slotsEntry(idx);
        slotsBurnerIndexChanged();
        updateDisplayList(m_oModel->level(idx)+1);
        autoResetCrumb();
    });

    connect(m_oModel,&LogicBurnData::rootIndexChanged,[=](QModelIndex idx){
        slotsEntry(idx);
        updateDisplayList(m_oModel->level(idx)+1);
        autoResetCrumb();
        refresh();
    });
#endif
    connect(m_oModel, &LogicBurnData::updateDisk, this, &BurnData::updateDisk);
    connect(this, &BurnData::delData, m_oModel, &LogicBurnData::deleteRoot);
    connect(this, &BurnData::changeCurrentIndex, m_oModel, &LogicBurnData::changeIndex);

    connect(ui->treeView, &MyTreeView::verticalScrollBarVisibleChanged, this, &BurnData::slotsVerticalScrollVisible);
    connect(ui->treeView, &MyTreeView::doubleClicked, [=](QModelIndex idx){
        slotsEntry(idx);
        updateDisplayList(m_oModel->level(idx)+1);
        autoResetCrumb();
    });

    //面包屑点击事件
    connect(mDirectoryCrumb, &KBreadCrumb::currentChanged, this, &BurnData::directoryChangeSlot);
    connect(m_go_back, &QToolButton::clicked, this, [=]()
    {
        goBack();
    });
    connect(m_go_forward, &QToolButton::clicked, this, [=]()
    {
        goForward();
    });


    connect(m_oModel, &LogicBurnData::sizeChanged, this, &BurnData::slotsSize);
    connect(m_oModel, &LogicBurnData::defaultAddPageEnabled, this, &BurnData::slotSetDefaultAddWindowVisible);
    connect(m_oModel, &LogicBurnData::addEnabled, ui->btnAdd, &QPushButton::setEnabled);
    connect(m_oModel, &LogicBurnData::delEnabled, ui->btnDelete, &QPushButton::setEnabled);
    connect(m_oModel, &LogicBurnData::clsEnabled, ui->btnClean, &QPushButton::setEnabled);
    connect(m_oModel, &LogicBurnData::folderEnabled, ui->btnNewFolder, &QPushButton::setEnabled);

    connect(m_oModel, &LogicBurnData::hiddenFileFiltered, ViewFilterFiles::instance(), &ViewFilterFiles::filterHidden);
    connect(m_oModel, &LogicBurnData::brokenLinkfileFiltered, ViewFilterFiles::instance(), &ViewFilterFiles::filterBroken);
    connect(m_oModel, &LogicBurnData::replaceLinkFileFiltered, ViewFilterFiles::instance(), &ViewFilterFiles::repleace);


    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this, SLOT(slotsSelectionChanged()));

    connect(LogicMain::instance(), SIGNAL(burnDataSetting()), m_oModel, SLOT(slotsSetting()));
    connect(LogicMain::instance(), SIGNAL(createISO(void*)), m_oModel, SLOT(slotsCreateISO(void*)));
    connect(LogicMain::instance(), SIGNAL(emptyMedium(void*)), m_oModel, SLOT(slotsCreateISO(void*)));
    connect(LogicMain::instance(), SIGNAL(appendableMedium(void*)), m_oModel, SLOT(slotsCreateISO(void*)));
    connect(LogicMain::instance(), SIGNAL(finishedMedium(void*)), m_oModel, SLOT(slotsCreateISO(void*)));


    connect(LogicMain::instance(), SIGNAL(createISO(void*)), this, SIGNAL(newNode(void*)));
    connect(LogicMain::instance(), SIGNAL(emptyMedium(void*)), this, SIGNAL(newNode(void*)));
    connect(LogicMain::instance(), SIGNAL(appendableMedium(void*)), this, SIGNAL(newNode(void*)));
    connect(LogicMain::instance(), SIGNAL(finishedMedium(void*)), this, SIGNAL(newNode(void*)));
    connect(this, SIGNAL(newNode(void*)), ViewFilterFiles::instance(), SLOT(newFilterRoot(void*)));
    connect(LogicMain::instance(), SIGNAL(createISO()), m_oModel, SLOT(slotsCreateISO()));
    connect(LogicMain::instance(), SIGNAL(burnData()), m_oModel, SLOT(slotsBurnData()));
    connect(LogicMain::instance(), SIGNAL(addBurn()), m_oModel, SLOT(slotsBurnData()));

    connect(MenuFactory::instance(), &MenuFactory::fileFilter, [=]()
    {
        if (!FileFilterRules::instance()->isHidden()){
            FileFilterRules::instance()->close();
        }
        m_oModel->resetFileterConfigToModel();
        FileFilterRules::instance()->show();
    });

    connect(this, &BurnData::viewFileterFile, MenuFactory::instance(), &MenuFactory::setViewFilterFile);
    connect(MenuFactory::instance(), SIGNAL(viewFilterFile(bool)), ViewFilterFiles::instance(), SLOT(show()));

    //filter dates
    connect(FileFilterRules::instance(), &FileFilterRules::hidden, this, &BurnData::slotsFilterHidden,Qt::UniqueConnection);
    connect(FileFilterRules::instance(), &FileFilterRules::broken, this, &BurnData::slotsFilterBroken,Qt::UniqueConnection);
    connect(FileFilterRules::instance(), &FileFilterRules::replace, this, &BurnData::slotsRepleaceSymbolLinks,Qt::UniqueConnection);
    connect(ViewFilterFiles::instance(), &ViewFilterFiles::allFilesRestored,m_oModel, &LogicBurnData::slotRestoreAllFiles,Qt::UniqueConnection);

    ui->lineEdit->hide();

    initAction();

    connect(g_user_signal, &GlobalUserSignal::iconChange, [=](QIcon icon){
        setWindowIcon(icon);
    });
    connect(g_user_signal, &GlobalUserSignal::sigDeleteByKeyPress, this, &BurnData::slotsDeleteFilesByKeyPress);
    connect(g_user_signal, &GlobalUserSignal::sigShiftPlusNByKeyPress, this, &BurnData::slotsNewFolder);
    connect(g_user_signal, &GlobalUserSignal::sigMenuByRightMousePress, this, &BurnData::slotsTreeMenuByShortcutKey);
    connect(g_user_signal, &GlobalUserSignal::sigDeleteAllByKeyPress, this, &BurnData::slotsCleanFiles);
    connect(g_user_signal, &GlobalUserSignal::sigAddFileByKeyPress, this, &BurnData::slotsAddFiles);
    refresh();
}

BurnData *BurnData::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new BurnData();
        }
    }
    return m_oInstance;
}

void BurnData::setAccessibleName()
{
    ui->btnAdd->setAccessibleName("kylin-burner_QPushButton_BurnData_btnAdd");
    ui->btnClean->setAccessibleName("kylin-burner_QPushButton_BurnData_btnClean");
    ui->btnDelete->setAccessibleName("kylin-burner_QPushButton_BurnData_btnDelete");
    ui->btnNewFolder->setAccessibleName("kylin-burner_QPushButton_BurnData_btnNewFolder");
    ui->btnTipAddFile->setAccessibleName("kylin-burner_QPushButton_BurnData_btnTipAddFile");
    ui->frame_6->setAccessibleName("kylin-burner_MyFrame_BurnData_frame_6");

    ui->labelProjectName->setAccessibleName("kylin-burner_QLabel_BurnData_labelProjectName");
    ui->labelProjectSize->setAccessibleName("kylin-burner_QLabel_BurnData_labelProjectSize");
    ui->labelWelcome->setAccessibleName("kylin-burner_QLabel_BurnData_labelWelcome");
    ui->lineEdit->setAccessibleName("kylin-burner_QLineEdit_BurnData_lineEdit");
    ui->linkViewFilterFile->setAccessibleName("kylin-burner_QLabel_BurnData_linkViewFilterFile");

    ui->tip->setAccessibleName("kylin-burner_QLabel_BurnData_tip");
    ui->tip_widget->setAccessibleName("kylin-burner_QWidget_BurnData_tipWidget");
    ui->treeView->setAccessibleName("kylin-burner_MyTreeView_BurnData_treeView");
    ui->widget->setAccessibleName("kylin-burner_QWidget_BurnData_widget");
    ui->widget_2->setAccessibleName("kylin-burner_QWidget_BurnData_widget2");
    ui->widget_3->setAccessibleName("kylin-burner_QWidget_BurnData_widget3");
}

void BurnData::resetTreeViewSize(bool scrollBarVisible)
{
    int scrollBarWidth = scrollBarVisible ? ui->treeView->verticalScrollBar()->width() : 0;
    int headerWidth = this->width() - 40;

    if (scrollBarVisible) {
        headerWidth -= scrollBarWidth;
    }

    ui->treeView->header()->setFixedWidth(headerWidth);
    qreal basic = ui->treeView->header()->width() / 10;
    ui->treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    ui->treeView->header()->resizeSection(0, basic * 4);
    ui->treeView->header()->resizeSection(1, basic * 1.5);
    ui->treeView->header()->resizeSection(2, basic * 1.5);
    ui->treeView->header()->resizeSection(3, basic * 3);
}


void BurnData::refresh()
{
    m_oAddIndex = ui->treeView->rootIndex();

    //check main menu
    m_oNewFolder->setEnabled(m_oModel->checkAdd(m_oAddIndex));
    m_oDelete->setEnabled(m_oModel->checkDel(ui->treeView->selectionModel()->selectedRows()));
    m_oRename->setEnabled(m_oModel->checkDel(ui->treeView->selectionModel()->selectedRows()));

    if (ui->treeView->selectionModel()->selectedRows().size() != 1)
        m_oRename->setEnabled(false);

    m_oOpen->setEnabled(m_oModel->checkOpen(ui->treeView->selectionModel()->selectedRows()));
    m_oOpenLocal->setEnabled(m_oModel->checkOpenLocal(ui->treeView->selectionModel()->selectedRows()));
    m_oProperties->setEnabled(m_oModel->checkProperties(ui->treeView->selectionModel()->selectedRows()));
    m_oModel->checkCls(ui->treeView->rootIndex());

    emit viewFileterFile(ViewFilterFiles::instance()->filtered());

    if (!(m_oModel->checkDefaultAddpage(ui->treeView->rootIndex())))
    {
        int selecteds = ui->treeView->selectionModel()->selectedRows().count();

        if (selecteds > 1) {
            ui->tip->setText(_("Selected %1 files of total %2 files")
                             .arg(selecteds).arg(m_oModel->rowCount(ui->treeView->rootIndex())));
        } else {
            ui->tip->setText(_("Selected %1 file of total %2 file(s)")
                             .arg(selecteds).arg(m_oModel->rowCount(ui->treeView->rootIndex())));
        }

    }

    emit burnDataToDiscEnabled(m_oModel->checkSize());
}


bool BurnData::canGoBack()
{
    return !m_back_list.isEmpty();
}

bool BurnData::canGoForward()
{
    return !m_forward_list.isEmpty();
}

void BurnData::delModelIndex(QModelIndex idx)
{
    QModelIndexList idxes;
    idxes << idx;

    slotsFilterHidden(false);
    slotsFilterBroken(false);
    slotsRepleaceSymbolLinks(false);

    m_oModel->deleteDatasFromIndex(idxes, ui->treeView->rootIndex());
    m_oAddIndex = ui->treeView->rootIndex();
    ui->btnDelete->setChecked(false);

    ui->treeView->setCurrentIndex(QModelIndex());
    refresh();

}

bool BurnData::event(QEvent *event)
{
    return QWidget::event(event);
}

bool BurnData::eventFilter(QObject *watched, QEvent *event)
{
    if (ui->tip_widget == watched)
    {

        QString url;
        QFileInfo  fileInfo;
        QList<QUrl> urls;
        QDropEvent *dropEvent;
        QDragEnterEvent *dragEvent;
        QStringList list;
        switch (event->type())
        {
        case QEvent::DragEnter:
            dragEvent = static_cast<QDragEnterEvent *>(event);
            urls = dragEvent->mimeData()->urls();

            while (urls.size())
            {
                url = LogicBurnData::convertTrashUrlToFileUrl(urls.takeFirst()).path();
                fileInfo.setFile(url);
                if (!fileInfo.exists()) {
                    event->ignore();
                    break;
                }
            }

            if (!fileInfo.exists())
                break;

            if (m_oModel->checkAdd(m_oAddIndex))
            {
                event->accept();
            }
            else
            {
                event->ignore();
            }
            break;

        case QEvent::DragLeave:
            event->ignore();
            break;

        case QEvent::Drop:
            dropEvent = static_cast<QDropEvent *>(event);
            urls = dropEvent->mimeData()->urls();
            while (urls.size()){
                QUrl indexUrl = LogicBurnData::convertTrashUrlToFileUrl(urls.takeFirst());
                list << indexUrl.path();
            }


            m_oModel->addDatasToIndex(list, m_oAddIndex);

            refresh();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void BurnData::paintEvent(QPaintEvent *event)
{

    QWidget::paintEvent(event);
}

void BurnData::resizeEvent(QResizeEvent *event)
{
    bool scrollBarVisible = ui->treeView->verticalScrollBar()->isVisible();
    resetTreeViewSize(scrollBarVisible);
    QWidget::resizeEvent(event);
}

void BurnData::keyPressEvent(QKeyEvent *event)
{
    QModelIndexList idxes = ui->treeView->selectionModel()->selectedRows();

    // 快捷键Shift+F10 ： 展示/隐藏上下文菜单
    if ((event->modifiers() == Qt::ShiftModifier) && (event->key() == Qt::Key_F10))
    {
        slotsTreeMenuByShortcutKey();
        event->accept();
        return;
    }

    // 快捷键F2 ：重命名
    if (event->key() == Qt::Key_F2)
    {
        if (m_oRename->isEnabled() && m_oModel->checkSelectModelItem(idxes)) {
            slotsRename();
        }
        event->accept();
        return;
    }

    // 快捷键F5 ：属性
    if (event->key() == Qt::Key_F5)
    {
        if (m_oProperties->isEnabled() && m_oModel->checkSelectModelItem(idxes)) {
            slotsProperties();
        }
        event->accept();
        return;
    }

    // 快捷键Ctrl+回车键return : 本地应用打开 (Key_Enter是小键盘的确认键，Key_Return是大键盘的回车键)
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Return) )
    {

        if (m_oOpenLocal->isEnabled() && m_oModel->checkSelectModelItem(idxes)) {
            slotsOpenLocal();
        }
        event->accept();
        return;
    }

    // 快捷键return回车键 : 打开选中文件夹
    if (event->key() == Qt::Key_Return)
    {
        if (m_oOpen->isEnabled() && m_oModel->checkSelectModelItem(idxes) && !m_bDirReNameing)
        {
            slotsOpen();
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void BurnData::initAction()
{
    m_oMenu = new MyMenu(ui->treeView);
    m_oMenu->setAccessibleName("kylin-burner_MyMenu_BurnData_oMenu");
    m_oOpen = new QAction(_("Open"), ui->treeView);
    m_oOpenLocal = new QAction(_("OpenInLocal"), ui->treeView);
    m_oRename = new QAction(_("Rename"), ui->treeView);
    m_oDelete = new QAction( _("Delete"), ui->treeView);
    m_oNewFolder = new QAction(_("NewFolder"), ui->treeView);
    m_oProperties = new QAction(_("Properties"), ui->treeView);

    m_oMenu->addAction(m_oOpen);
    m_oMenu->addAction(m_oOpenLocal);
    m_oMenu->addSeparator();
    m_oMenu->addAction(m_oRename);
    m_oMenu->addAction(m_oDelete);
    m_oMenu->addAction(m_oNewFolder);
    m_oMenu->addSeparator();
    m_oMenu->addAction(m_oProperties);

    connect(m_oOpen, &QAction::triggered, this, &BurnData::slotsOpen);
    connect(m_oOpenLocal, &QAction::triggered, this, &BurnData::slotsOpenLocal);
    connect(m_oRename, &QAction::triggered, this, &BurnData::slotsRename);
    connect(m_oDelete, &QAction::triggered, this, &BurnData::slotsDeleteFiles);
    connect(m_oNewFolder, &QAction::triggered, this, &BurnData::slotsNewFolder);
    connect(m_oProperties, &QAction::triggered, this, &BurnData::slotsProperties);
}

void BurnData::linkEntry(QModelIndex idx)
{
    if (!idx.isValid())
        return;

    int level = m_oModel->level(idx);
    int tabCount = mDirectoryCrumb->count();                        //面包屑里tab的个数
    if (0 == level)
    {
        return;
    }

#ifndef UNIT_TEST
    //前进
    if (tabCount <= level)
    {
        tabCount = mDirectoryCrumb->count();    //面包屑里tab的个数
        mDirectoryMap->insert(level, idx);      //逻辑3),存储子目录对应的QModelIndex，方便QTreeView做目录回退操作
        mDirectoryCrumb->show();                //通过QTreeView进入子目录后，面包屑需要show出来，因为后面可能需要返回父目录
        m_go_back->setEnabled(canGoBack());
        m_go_forward->setEnabled(canGoForward());
        return;
    }

    //其他
    //后退/减少 隐藏
    m_go_back->setEnabled(canGoBack());
    m_go_forward->setEnabled(canGoForward());
#endif
}

void BurnData::dataViewChanged(QModelIndex &idx)
{
    if(!idx.isValid()){
        return;
    }

    DataFile *file = static_cast<DataFile *>(idx.internalPointer());
    QModelIndex freshIndex = m_oModel->modelIndexForDataFile(file);
    if (freshIndex.isValid()) {
        idx = freshIndex.siblingAtColumn(0);
    }

    QString tabDisplay = m_oModel->data(idx).toString();

    ui->treeView->setRootIndex(idx);
    QModelIndex currentIndex =  ui->treeView->rootIndex();
    
    int tabcount = 0;
    QModelIndex index = currentIndex;
    
    while (index != QModelIndex())
    {
        index = index.parent();
        tabcount++;
    }
    
    QList<QModelIndex> indexList;
    tabcount = tabcount - 1;
    for(int i=1; i<tabcount; ++i)
    {
        if(currentIndex != m_rootIndex)
        {
            if(i==1)
            {
                indexList.prepend(currentIndex);
            }
            else
            {
                auto parentIndex = currentIndex.parent();
                indexList.prepend(parentIndex.siblingAtColumn(0));
                currentIndex = parentIndex;
            }
        }
    }

    //map更新
    for(int i=1;i<tabcount-1;i++)
    {
        mDirectoryMap->insert(i,indexList.at(i-1));
    }

    refresh();

    //切换路径时，清空按钮的状态也需要调整
    updateDisplayList(m_oModel->level(idx) + 1);
    autoResetCrumb();

    m_go_back->setEnabled(canGoBack());
    m_go_forward->setEnabled(canGoForward());
}

void BurnData::autoResetCrumb()
{
    m_offsetRemovedTabCount = 0;
    //calc all the tab length
    QFontMetrics fm(mDirectoryCrumb->font());
    int currentWidth = 30;
    for(int i=0;i<m_textList.count();i++)
    {
        if(i==0)
        {
            currentWidth += (fm.width(_("All files"))*1.2+ 40);
        }
        else
        {
            currentWidth += (fm.width(m_textList.at(i))*1.2+ 40);
        }
    }
    int crumbMaxWidth = ui->tip_widget->width()-m_go_back->width()*2;


    QStringList list = m_textList;

    //replace the tab[1] to ...
    //and removte the last tab to fit
    if(currentWidth < crumbMaxWidth)
    {
        m_isAutoReset = false;
    }
    else
    {
        m_isAutoReset = true;
        QString str = list.at(1);
        currentWidth = currentWidth - fm.width(str)*1.2+ fm.width("...")*1.2;
        list[1] = "...";
        for (int i = 2; i < m_textList.count(); i++)
        {
            if (currentWidth < crumbMaxWidth)
            {
                break;
            }
            str = m_textList.at(i);
            list.removeAt(2);
            m_offsetRemovedTabCount++;
            currentWidth = currentWidth - fm.width(str)*1.2 - 40;
        }
    }

    mDirectoryCrumb->setFixedWidth(currentWidth);

    clearCrumb();
    int index = 0;
    for (QString text : list)
    {
        mDirectoryCrumb->blockSignals(true);
        mDirectoryCrumb->addTab(text);
        mDirectoryCrumb->blockSignals(false);
        QString str = m_oModel->data(mDirectoryMap->value(index+m_offsetRemovedTabCount)).toString();
        if (index != 0 && text != str)
        {
            if(index==1 && m_isAutoReset)
            {
                //规避省略显示,"..."不可点击
                mDirectoryCrumb->setTabEnabled(1,false);
            }
            else
            {
                mDirectoryCrumb->setTabToolTip(index, str);
            }
        }
        index++;
    }
    mDirectoryCrumb->setCurrentIndex(mDirectoryCrumb->count()-1);               //逻辑2),双击进入子目录后，更新面包屑的索引
}

void BurnData::updateDisplayList(int count)
{
    m_textList.clear();
    QFontMetrics fm(mDirectoryCrumb->font());

    for(int i = 0; i < count; i++)
    {
        QString str;
        if(i==0)
        {
            str = _("All files");
        }
        else
        {
            str = m_oModel->data(mDirectoryMap->value(i)).toString();
        }
        QString text = fm.elidedText(str, Qt::ElideRight, 120);
        m_textList.append(text);
    }
}

void BurnData::clearCrumb()
{
    mDirectoryCrumb->blockSignals(true);
    for (int i = mDirectoryCrumb->count() - 1; i >= 0; i--)
    {
        mDirectoryCrumb->removeTab(i);
    }
    mDirectoryCrumb->blockSignals(false);
}

void BurnData::resetText(QPushButton* item)
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString item_name = item->text();
    if (item->toolTip() != ""
            && -1 != item_name.indexOf("…")
            && -1 != item->toolTip().indexOf(item_name.mid(0, item_name.length() - 3)))
    {
        item_name = item->toolTip();
    }
    QString show_name = fontWidth.elidedText(item_name, Qt::ElideRight, item->width() - 50);
    if(item_name == "Add"){
        item->setIcon(QIcon::fromTheme("list-add-symbolic"));
    }else if(item_name == "Delete"){
        item->setIcon(QIcon::fromTheme("edit-clear-symbolic"));
    }else if(item_name == "Clean"){
        item->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    }else if(item_name == "NewFolder"){
        item->setIcon(QIcon::fromTheme("folder-new-symbolic"));
    }

    item->setIconSize(ICON_SIZE);
    item->setText(show_name);

    if (show_name.endsWith("…"))
        item->setToolTip(item_name);
    else
        item->setToolTip("");
}

void BurnData::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);

    QFont font = title->font();
    font.setPixelSize(24 + scaleRatio);
    title->setFont(font);

    font = ui->labelWelcome->font();
    font.setPixelSize(14 + scaleRatio);
    ui->labelWelcome->setFont(font);
    ui->tip->setFont(font);

    font = ui->btnAdd->font();
    font.setPixelSize(14 + scaleRatio);
    font.setWeight(400);
    ui->btnAdd->setFont(font);
    ui->btnDelete->setFont(font);
    ui->btnClean->setFont(font);
    ui->btnNewFolder->setFont(font);
    mDirectoryCrumb->setFont(font);

    ui->labelProjectSize->setFont(font);
    ui->labelProjectName->setFont(font);
    ui->linkViewFilterFile->setFont(font);
    ui->treeView->setFont(font);
    ui->treeView->setStyleSheet(QString("QHeaderView::section {"
                                "font-size:%1px;"
                                        "}").arg(14+scaleRatio));
    resetText(ui->btnAdd);
    resetText(ui->btnDelete);
    resetText(ui->btnClean);
    resetText(ui->btnNewFolder);
}

void BurnData::slotChangeUIFontname(QString fontName)
{
    QFont font = ui->treeView->font();
    font.setFamily(fontName);
    ui->treeView->setFont(font);
    ui->treeView->setStyleSheet(QString("QHeaderView::section {"
                                "font-family:%1;"
                                        "}").arg(fontName));
}

void BurnData::slotStopLoadingData()
{
    m_oModel->stopLoadingData();
}

void BurnData::slotSetDefaultAddWindowVisible(bool isVisible)
{
    if (isVisible)
    {
        ui->layoutBurnData->setStretch(2, 1);
        ui->layoutBurnData->setStretch(3, 100);
        ui->tip->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        ui->tip->setText(_("Click the \"+\" icon\n"
                                     "Or drag the files here to add the files"));
        ui->widget->hide();
        ui->widget_2->hide();
        ui->tip_widget->show();
        ui->treeView->header()->setVisible(false);
        ui->treeView->setVisible(false);
        title->show();
        bottomLine->show();

        ui->treeView->setFrameShape(QFrame::NoFrame);
    }
    else
    {
        ui->layoutBurnData->setStretch(2, 20);
        ui->layoutBurnData->setStretch(3, 1);
        ui->tip->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tip_widget->hide();
        ui->widget->show();
        ui->widget_2->show();
        ui->treeView->header()->setVisible(true);
        ui->treeView->setVisible(true);
        title->hide();
        bottomLine->hide();

        ui->treeView->setFrameShape(QFrame::StyledPanel);
    }
}

void BurnData::slotsEntry(QModelIndex idx)
{
    idx = idx.siblingAtColumn(0);

    FileFilterRules::instance()->blockSignals(true);

    if (m_oModel->isUp(idx))
    {
        //后退
        QModelIndex brotherIndex_back = ui->treeView->rootIndex().siblingAtColumn(0);
        m_back_list.append(brotherIndex_back);

        ui->treeView->setRootIndex(idx.parent().parent());        
        linkEntry(idx.parent().parent());

        m_oModel->filter(idx.parent().parent());
        m_oAddIndex = idx.parent().parent();
    }
    else if (m_oModel->isFolder(idx))
    {
        //前进
        QModelIndex brotherIndex_back = ui->treeView->rootIndex().siblingAtColumn(0);
        if(mDirectoryCrumb->count()==0)
        {
            mDirectoryMap->insert(0,brotherIndex_back);
            m_rootIndex = brotherIndex_back;
        }
        else
        {
            m_back_list.append(brotherIndex_back);
        }
        ui->treeView->setRootIndex(idx);

        linkEntry(idx);
        m_oModel->filter(idx);
        m_oAddIndex = idx;
    }

    ui->treeView->setCurrentIndex(QModelIndex());
    FileFilterRules::instance()->blockSignals(false);
}


void BurnData::slotsSize(unsigned long size)
{
    QPalette pal = ui->labelProjectSize->palette();
    if (m_oModel->checkSize())
        pal.setColor(QPalette::WindowText, Qt::green);
    else
        pal.setColor(QPalette::WindowText, Qt::red);
    ui->labelProjectSize->setPalette(pal);
    ui->labelProjectSize->setText(m_oModel->stringSize(size));
}

void BurnData::slotsSelectionChanged()
{
    QModelIndexList idxes = ui->treeView->selectionModel()->selectedRows();
    if (idxes.count() == 1)
    {
        if (m_oModel->checkAdd(idxes))
            m_oAddIndex = idxes.first();
    }
    else
    {
        m_oAddIndex = ui->treeView->rootIndex();
    }
    refresh();
}

void BurnData::slotsAddFiles(bool b)
{
    //when btn is disabled can't add file
    if(!ui->btnAdd->isEnabled()){
        return;
    }
    slotsShowAddFilesDlg(b);
}

void BurnData::slotsDeleteFiles(bool istoggled)
{
    if (istoggled || m_oDelete == sender())
    {
        //使用QMessageBox代替EXEC删除提示弹窗
        if (m_bIsWayland) {
            QScopedPointer<TipsChange> p(new TipsChange(this));
            if(p->question(_("Delete"), _("Delete file(s) selected?")))
            {
                ui->btnDelete->setChecked(false);
                return;
            }
        } else {
            QScopedPointer<TipsChange> p(new TipsChange());
            //show center in parent widget
            QRect parentRect = MainWindow::Instance()->geometry();
            QRect childRect = p->geometry();

            int x = parentRect.x() + (parentRect.width() - childRect.width())/2;
            int y = parentRect.y() + (parentRect.height() - childRect.height())/2;
            p->move(x,y);

            if(p->question(_("Delete"), _("Delete file(s) selected?")))
            {
                ui->btnDelete->setChecked(false);
                return;
            }
        }

        QModelIndexList idxes = ui->treeView->selectionModel()->selectedRows();
        m_oModel->deleteDatasFromIndex(idxes, ui->treeView->rootIndex());
        m_oAddIndex = ui->treeView->rootIndex();
        ui->btnDelete->setChecked(false);
    }
    ui->treeView->setCurrentIndex(QModelIndex());
    refresh();
}

void BurnData::slotsCleanFiles(bool enabled)
{
    if (enabled)
    {
        if (m_bIsWayland) {
            QScopedPointer<TipsChange> tipChange(new TipsChange(this));
            if(!tipChange->question(_("Clean"), _("Clean all files(s)?")))
            {
                slotsRemoveAllFiles();
            }
        } else {
            QScopedPointer<TipsChange> tipChange(new TipsChange());
            //show center in parent widget
            QRect parentRect = MainWindow::Instance()->geometry();
            QRect childRect = tipChange->geometry();

            int x = parentRect.x() + (parentRect.width() - childRect.width())/2;
            int y = parentRect.y() + (parentRect.height() - childRect.height())/2;
            tipChange->move(x,y);

            if(!tipChange->question(_("Clean"), _("Clean all files(s)?")))
            {
                slotsRemoveAllFiles();
            }
        }
        ui->btnClean->setChecked(false);
    }
    update();
    refresh();
}

void BurnData::slotsRemoveAllFiles()
{
#ifndef UNIT_TEST
    QModelIndex index = QModelIndex();
    QList<void *> hiddenDatas = ViewFilterFiles::instance()->recoveryHiddenData(m_oModel->index(), m_oModel->level());
    m_oModel->doRecoveryHidden(hiddenDatas, index);

    QList<void *> brokenDatas = ViewFilterFiles::instance()->recoveryBrokenData(m_oModel->index(), m_oModel->level());
    m_oModel->doRecoveryBroken(brokenDatas, index);

    QList<void *> replaceLinkfileDatas = ViewFilterFiles::instance()->recoveryRepleaceData(m_oModel->index(), m_oModel->level());
    m_oModel->doRecoveryRepleace(replaceLinkfileDatas,index);

    update();
    refresh();
    QModelIndex rootIndex = ui->treeView->rootIndex();
    m_oModel->clearDatasUnderIndex(rootIndex);

    m_back_list.clear();
    m_forward_list.clear();
    m_go_back->setEnabled(false);
    m_go_forward->setEnabled(false);
#endif
}

void BurnData::slotsNewFolder(bool b)
{
#ifndef UNIT_TEST
    QString folderName;
    bool inputed;

    if (b || m_oNewFolder == sender())
    {
        do
        {
            if (SystemInfo::m_isWayland) {
                QScopedPointer<InputChange> inputchange(new InputChange(this));
                //KWindowSystem::setState(inputchange->winId(), NET::SkipPager | NET::SkipSwitcher);
                inputed=inputchange->input(folderName, _("NewFolder"), _("Please input new folder name"), _("NewFolder"));
            } else {
                QScopedPointer<InputChange> inputchange(new InputChange());
                KWindowSystem::setState(inputchange->winId(), NET::SkipPager | NET::SkipSwitcher | NET::SkipTaskbar);
                inputchange->move(MainWindow::Instance()->frameGeometry().center() - inputchange->rect().center());
                inputed=inputchange->input(folderName, _("NewFolder"), _("Please input new folder name"), _("NewFolder"));
            }
            if (!inputed) break;

            ui->btnNewFolder->setChecked(false);
            if (folderName.trimmed().isEmpty())
            {
                EXEC->error(_("NewFolder") + _("Empty Name"),
                            _("New Folder cannot be named with empty."));
                continue;
            }
            if (folderName.contains("/"))
            {
                EXEC->error(_("NewFolder") + _("Empty Name"),
                            _("New Folder cannot be named with ""/""."));
                continue;
            }

            if (folderName.toUtf8().size() > 103)
            {
                EXEC->error(_("NewFolder") + _("Name Too Long"),
                            _("Need less than 103 unicode charset for naming new folder."));
                continue;
            }
            if (m_oModel->duplicatedName(folderName, m_oAddIndex))
            {
                EXEC->error(_("NewFolder") + _("Duplicate Name"),
                            _("There is a item named by %1").arg(folderName));
                continue;
            }
            if (inputed)
                break;
        } while (true);
        if (inputed)
            m_oModel->addEmptyFolder(folderName.trimmed(), m_oAddIndex);
        ui->btnNewFolder->setChecked(false);
    }
    refresh();
#endif
}

void BurnData::slotsFilterHidden(bool isEnabled)
{
    // 获取当前设备索引
    int currentDeviceIndex = m_oModel->index();
    if (currentDeviceIndex < 0) return;

    // 获取当前设备的根节点
    DataDirectory *currentRoot = static_cast<DataDirectory*>(m_oModel->m_oRoots.at(currentDeviceIndex));
    if (!currentRoot) return;

    // 设置当前设备的节点特定过滤规则
    currentRoot->setNodeFilterHidden(isEnabled);

    // 应用过滤规则到当前设备
    if (isEnabled) {
        currentRoot->execHidden(true);
    } else {
        // 恢复当前设备的隐藏文件
        QList<void *> datas = ViewFilterFiles::instance()->recoveryHiddenData(currentDeviceIndex, m_oModel->level());
        m_oModel->recoverHiddenFilesByDeviceIndex(datas, currentDeviceIndex);
    }

    update();
    refresh();
}

void BurnData::slotsFilterBroken(bool isEnabled)
{
    // 获取当前设备索引
    int currentDeviceIndex = m_oModel->index();
    if (currentDeviceIndex < 0) return;

    // 获取当前设备的根节点
    DataDirectory *currentRoot = static_cast<DataDirectory*>(m_oModel->m_oRoots.at(currentDeviceIndex));
    if (!currentRoot) return;

    // 设置当前设备的节点特定过滤规则
    currentRoot->setNodeFilterBrokenSymbolLinks(isEnabled);

    // 应用过滤规则到当前设备
    if (isEnabled) {
        currentRoot->execBroken(true);
    } else {
        // 恢复当前设备的损坏链接文件
        QList<void *> datas = ViewFilterFiles::instance()->recoveryBrokenData(currentDeviceIndex, m_oModel->level());
        m_oModel->recoverBrokenFilesByDeviceIndex(datas, currentDeviceIndex);
    }

    update();
    refresh();
}

void BurnData::slotsRepleaceSymbolLinks(bool isEnabled)
{
    // 获取当前设备索引
    int currentDeviceIndex = m_oModel->index();
    if (currentDeviceIndex < 0) return;

    // 获取当前设备的根节点
    DataDirectory *currentRoot = static_cast<DataDirectory*>(m_oModel->m_oRoots.at(currentDeviceIndex));
    if (!currentRoot) return;

    // 设置当前设备的节点特定过滤规则
    currentRoot->setNodeRepleaceSymbolLinks(isEnabled);

    // 应用过滤规则到当前设备
    if (isEnabled) {
        currentRoot->execRepleace(true);
    } else {
        // 恢复当前设备的替换链接文件
        QList<void *> datas = ViewFilterFiles::instance()->recoveryRepleaceData(currentDeviceIndex, m_oModel->level());
        m_oModel->recoverReplaceLinkFilesByDeviceIndex(datas, currentDeviceIndex);
    }
    update();
    refresh();
}

void BurnData::slotsParent()
{
    if (ui->treeView->rootIndex().parent().isValid())
    {
        slotsEntry(ui->treeView->rootIndex().parent());
        updateDisplayList(m_oModel->level(ui->treeView->rootIndex().parent())+2);
        autoResetCrumb();
    }
    refresh();
}

void BurnData::slotsOpen()
{
#ifndef UNIT_TEST
    if(ui->treeView->selectionModel()->selectedIndexes().size() <= 0)
    {
        return;
    }

    if (".." == ui->treeView->selectionModel()->selectedIndexes().first().siblingAtColumn(0).data().toString())
    {
        slotsParent();
    }
    else
    {
        QModelIndexList modelIndexList = ui->treeView->selectionModel()->selectedIndexes();
        QModelIndex modeIndex = modelIndexList.first();
        slotsEntry(modeIndex);
        updateDisplayList(m_oModel->level(modeIndex) +1);
        autoResetCrumb();
    }
#endif
}

void BurnData::slotsOpenLocal()
{
#ifndef UNIT_TEST
    QModelIndex idx;

    if(ui->treeView->selectionModel()->selectedIndexes().size() <= 0){
        return;
    }

    if (".." == ui->treeView->selectionModel()->selectedIndexes().first().siblingAtColumn(0).data().toString())
        idx = ui->treeView->rootIndex();
    else
        idx = ui->treeView->selectionModel()->selectedIndexes().first();
    m_oOpenFile->setFile(idx.siblingAtColumn(3).data().toString());
    m_oOpenFile->exec();
#endif
}

void BurnData::slotsRename()
{
#ifndef UNIT_TEST
    if(ui->treeView->selectionModel()->selectedIndexes().size() <= 0){
        return;
    }

    QModelIndex idx = ui->treeView->selectionModel()->selectedIndexes().first().siblingAtColumn(0);
    m_bDirReNameing = true;
    ui->treeView->edit(idx);
#endif
}

void BurnData::slotsProperties()
{ 
#ifndef UNIT_TEST
    QString cmd;
    QModelIndex idx;

    if(ui->treeView->selectionModel()->selectedIndexes().size() <= 0){
        return;
    }

    idx = ui->treeView->selectionModel()->selectedIndexes().first();
    cmd = QString("/usr/bin/peony -p \"%1\"").arg(idx.siblingAtColumn(3).data().toString());
    ::system(cmd.toUtf8().constData());
#endif
}

/**
 * @brief 鼠标右键展现上下文菜单
 * @param pos 右键点击的位置
 */
void BurnData::slotsTreeMenuByRightKey(const QPoint &pos)
{
    QModelIndex curIndex = ui->treeView->indexAt(pos);      //当前点击的元素的index
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //该行的第1列元素的index

    if (index.isValid())
    {
        ui->treeView->setCurrentIndex(index);
        m_oMenu->popup(QCursor::pos());//显示菜单
    }
}

/**
 * @brief 使用快捷键去展现上下文菜单
 */
void BurnData::slotsTreeMenuByShortcutKey()
{
    m_oMenu->popup(QCursor::pos());
}

/**
 * @brief 使用快捷键Del去删除指定文件
 * @param enabled
 */
void BurnData::slotsDeleteFilesByKeyPress(bool enabled)
{
    if (m_oDelete->isEnabled()) {
        slotsDeleteFiles(enabled);
    }
}

void BurnData::slotsVerticalScrollVisible(bool visible)
{
    resetTreeViewSize(visible);
}

void BurnData::goBack()
{
    if (!canGoBack())
        return;
    auto uri = m_back_list.takeLast();
    m_forward_list.prepend(ui->treeView->rootIndex());
    dataViewChanged(uri);
}

void BurnData::goForward()
{
    if (!canGoForward())
        return;
    auto uri = m_forward_list.takeFirst();
    m_back_list.append(ui->treeView->rootIndex());
    dataViewChanged(uri);
}

void BurnData::slotsBurnerIndexChanged()
{
    m_rootIndex = ui->treeView->rootIndex().siblingAtColumn(0);
    mDirectoryMap->insert(0,ui->treeView->rootIndex().siblingAtColumn(0));
    m_back_list.clear();
    m_forward_list.clear();
    m_go_back->setEnabled(canGoBack());
    m_go_forward->setEnabled(canGoForward());
    autoResetCrumb();
    refresh();
}

void BurnData::slotsShowAddFilesDlg(bool enable)
{
#ifndef UNIT_TEST
    MultiSelectionFileDialog fd;
    QList<QUrl> list = fd.sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        if (fi.fileName() == "2691-6AB8")
            continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(&fd);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path)
    {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
            QFileInfo fi = wfilist.at(i);
            if (fi.fileName() == "2691-6AB8")
                continue;
            mntUrlList << QUrl("file://" + fi.filePath());
        }
        fd.setSidebarUrls(list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd]() {
        fd.setSidebarUrls(list);
    });

    if (enable)
    {
        fd.reset();
        fd.setExtendedSelection();
        fd.setDirectory(QDir(last_path));
        fd.setWindowTitle(_("Burner") + " - " + _("Add File(s)"));
        fd.setWindowIcon(QIcon::fromTheme("burner"));
        fd.setFileMode(QFileDialog::ExistingFiles);
        fd.setSidebarUrls(list + mntUrlList);
        fd.move(BurnData::instance()->geometry().center()-fd.geometry().center());
        if (fd.exec()) {
            QStringList files = fd.selectedFiles();
            if (files.size() <= 0) {
                return;
            }
            last_path = files.first().left(files.first().lastIndexOf('/') + 1);;
            m_oModel->addDatasToIndex(files, m_oAddIndex);
        }
        ui->btnAdd->setChecked(false);
        ui->btnTipAddFile->setChecked(false);
    }
    refresh();
#endif
}

void BurnData::directoryChangeSlot(int directoryIndex)
{
    int index = 0;

    if(directoryIndex ==0)
    {
        index = 0;
    }
    else
    {
        index = directoryIndex + m_offsetRemovedTabCount;
    }

    int recoredDirCount = mDirectoryCrumb->count();
    if (directoryIndex == (recoredDirCount - 1) || (m_isAutoReset && directoryIndex == 1))
    {
        return;
    }

    mDirectoryCrumb->blockSignals(true);
    for(int index=recoredDirCount-1; index > directoryIndex; --index)
    {
         mDirectoryCrumb->removeTab(index);
    }
    mDirectoryCrumb->blockSignals(false);

    slotsEntry(mDirectoryMap->value(index));
    updateDisplayList(index+1);
    autoResetCrumb();
    refresh();
}


BurnData::~BurnData()
{
    delete ui;
    if (m_oMenu) {
        delete m_oMenu;
        m_oMenu = nullptr;
    }
    if (m_oOpen) {
        delete m_oOpen;
        m_oOpen = nullptr;
    }
    if (m_oOpenLocal) {
        delete m_oOpenLocal;
        m_oOpenLocal = nullptr;
    }
    if (m_oRename) {
        delete m_oRename;
        m_oRename = nullptr;
    }
    if (m_oDelete) {
        delete m_oDelete;
        m_oDelete = nullptr;
    }
    if (m_oNewFolder) {
        delete m_oNewFolder;
        m_oNewFolder = nullptr;
    }
    if (m_oProperties) {
        delete m_oProperties;
        m_oProperties = nullptr;
    }
    if (m_go_back) {
        delete m_go_back;
        m_go_back = nullptr;
    }
    if (m_go_forward) {
        delete m_go_forward;
        m_go_forward = nullptr;
    }
    if (bottomLine) {
        delete bottomLine;
        bottomLine = nullptr;
    }

    mDirectoryMap->clear();
    delete mDirectoryMap;
    mDirectoryMap = nullptr;

    ui->link->removeWidget(mDirectoryCrumb);
    connect(mDirectoryCrumb,&KBreadCrumb::destroyed,this,[&]{mDirectoryCrumb = nullptr;});
    mDirectoryCrumb->deleteLater();

    clearHistory();
    delete title;
}

CustomDelegate::CustomDelegate(QObject *parent) : QStyledItemDelegate(parent)
{}

CustomDelegate::~CustomDelegate(){}

QWidget *CustomDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    CustomSelectionLineEdit *lineEdit = new CustomSelectionLineEdit(parent);
    return lineEdit;
}

void CustomDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    CustomSelectionLineEdit* lineEdit = qobject_cast<CustomSelectionLineEdit*>(editor);
    if(lineEdit){
        DataFile *file = static_cast<DataFile *>(index.internalPointer());
        if(file){
            lineEdit->setText(file->display());
            QFileInfo fileInfo(file->display());
            QString baseName = fileInfo.baseName();
            if(!baseName.isEmpty()){
               lineEdit->setBasename_pos(baseName.length());
            }
        }
    }
}

void CustomDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    CustomSelectionLineEdit* lineEdit = qobject_cast<CustomSelectionLineEdit*>(editor);
    if(lineEdit){
        model->setData(index,lineEdit->text());
    }
}

