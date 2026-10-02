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

#include "viewfilterfiles.h"
#include "ui_viewfilterfiles.h"
#include "mainwindow.h"
#include "src/frame/signals.h"
#include "src/frame/xatom-helper.h"
#include "src/frame/logger.h"
#include "src/frame/configuration.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/data/datadirectory.h"
#include "src/view/burndata.h"
#include "src/view/filefilterrules.h"
#include "src/view/burndata.h"
#include "src/common/utils.h"
#include "v11sdk.h"

#include <QComboBox>
#include <QDesktopWidget>


using namespace kdk;

ViewFilterFiles *ViewFilterFiles::m_oInstance = nullptr;
QMutex           ViewFilterFiles::m_oMutex;

ViewFilterFiles::ViewFilterFiles(QWidget *parent) :
    QDialog(parent)
{
    m_oModel = FileFilterModel::instance();
    this->setObjectName("kylin-burner_ViewFilterFiles");
    initDialog();
    initMember();
    initRootLayout();
    initTreeWidget();
    initSignalConnect();

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &ViewFilterFiles::slotChangeUIFontSize);
}

ViewFilterFiles *ViewFilterFiles::instance(QWidget *parent)
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new ViewFilterFiles(parent);
        }
    }
    return m_oInstance;
}

bool ViewFilterFiles::filtered()
{
    return m_oModel->filtered() > 0;
}

int ViewFilterFiles::currentIndex()
{
    return mComboBoxDisk->currentIndex();
}

QList<void *> ViewFilterFiles::recoveryHiddenData(int idx, int level)
{
    if (level < 0)
        level = 0;
    return m_oModel->GetFilteredHiddenFiles(idx, level);
}

QList<void *> ViewFilterFiles::recoveryBrokenData(int idx, int level)
{
    if (level < 0)
        level = 0;
    return m_oModel->GetFilteredBadLinkFiles(idx, level);
}

QList<void *> ViewFilterFiles::recoveryRepleaceData(int idx, int level)
{
    if (level < 0)
        level = 0;
    return m_oModel->GetFilteredReplacedLinkfiles(idx, level);
}

void ViewFilterFiles::updatePosition(QPoint p)
{
    m_oPosition.setX(p.x() - width() / 2);
    m_oPosition.setY(p.y() - height() / 2);
}

void ViewFilterFiles::show()
{
    move(m_oPosition);
    if (!SystemInfo::m_isWayland) {
        QPoint position = MainWindow::Instance()->frameGeometry().topLeft();
        move(position.x()+(MainWindow::Instance()->width()-width())/2, position.y()+(MainWindow::Instance()->height()-height())/2);
        setGeometry(geometry());
    }
    QDialog::exec();
}

void ViewFilterFiles::newFilterRoot(void *d)
{
    m_oModel->addFilterNode(d);
}

void ViewFilterFiles::filterHidden(int idx, void *data)
{
    m_oModel->addFilterData(idx, data);
    update();
}

void ViewFilterFiles::filterBroken(int idx, void *data)
{
    m_oModel->addFilterData(idx, data);
    update();
}

void ViewFilterFiles::repleace(int idx, void *data)
{
    m_oModel->addFilterData(idx, data);
    update();
}

void ViewFilterFiles::updateIndex(int idx)
{
    m_oModel->updateIndex(idx);

    DataDirectory *root = m_oModel->getCurrentRoot();
    if (root) {
        m_fileFilterRules->init(root);
    }

    emit g_user_signal->sigChangeComboxBurnerIndex(idx);
}

void ViewFilterFiles::deleteNode(int idx)
{
    mCDROMList.removeAt(idx);
    mComboBoxDisk->removeItem(idx);
    m_oModel->deleteFilterNode(idx);
}


void ViewFilterFiles::slotRestoreFiles()
{
    m_oModel->restoreFiles();
}

void ViewFilterFiles::slotsChangeComboDiskIndex(void *cdrom)
{
    for (int i = 0; i < mCDROMList.size(); i++)
    {
        if (cdrom == mCDROMList.at(i))
        {
            emit mComboBoxDisk->currentIndexChanged(i);
            break;
        }
    }
}

void ViewFilterFiles::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);

    QFont font;
    font.setPixelSize(16 + scaleRatio);
    font.setFamily("Noto Sans CJK SC Medium");
    mTitleLabel->setFont(font);

    font.setPixelSize(14 + scaleRatio);
    mTreeView->setFont(font);

    mTreeView->setStyleSheet(QString("QHeaderView::section {"
                                "font-size:%1px;"
                                     "}").arg(14+scaleRatio));
}

void ViewFilterFiles::slotShowFilterRules()
{
#ifndef UNIT_TEST
    //filter dates
    m_fileFilterRules->disconnect();
    connect(m_fileFilterRules, &FileFilterRules::hidden, BurnData::instance(), &BurnData::slotsFilterHidden);
    connect(m_fileFilterRules, &FileFilterRules::broken, BurnData::instance(), &BurnData::slotsFilterBroken);
    connect(m_fileFilterRules, &FileFilterRules::replace, BurnData::instance(), &BurnData::slotsRepleaceSymbolLinks);

    DataDirectory *root = m_oModel->getCurrentRoot();
    if (root)
    {
        m_fileFilterRules->init(root);
        if (!m_fileFilterRules->isVisible()){
            m_fileFilterRules->showDlg();
        }else{
            m_fileFilterRules->activateWindow();
        }
    }
#endif
}

void ViewFilterFiles::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();
    if ("ukui-black" == LogicMain::instance()->theme())
    {
        pal.setBrush(QPalette::Background,QColor("#1D1D1D"));
    }
    else
    {
        pal.setBrush(QPalette::Background, QColor("#FFFFFF"));
    }
    setPalette(pal);
    QDialog::paintEvent(event);
}

ViewFilterFiles::~ViewFilterFiles()
{

}

void ViewFilterFiles::initDialog()
{
    setWindowIcon(QIcon::fromTheme("burner"));   //窗口左上角图标
    setWindowTitle(_("Burner"));            //窗口左上角标题
    setFixedSize(700,530);

    //判断是否是wayland环境
    if(SystemInfo::m_isWayland) {
        ukuiWaylandWindowStyleChange(this);
    } else {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
    }
    setAttribute(Qt::WA_ShowModal, true);

}

void ViewFilterFiles::initMember()
{
    mMainLayout = new QVBoxLayout;
    mTitleLayout = new QHBoxLayout;
    mButtonLayout = new QHBoxLayout;
    mFileTreeLayout = new QHBoxLayout;

    mComboBoxDisk = new QComboBox;
    mComboBoxDisk->setAccessibleName("kylin-burner_QComboBox_ViewFilterFiles_mComboBoxDisk");
    mWindowTitle = new WindowTitle(this);
    mWindowTitle->setAccessibleName("kylin-burner_WindowTitle_ViewFilterFiles_mWindowTitle");
    mMainWdiget = new QWidget(this);
    mMainWdiget->setAccessibleName("kylin-burner_QWidget_ViewFilterFiles_mMainWdiget");
    mTitleLabel = new QLabel(_("ViewFilterFile(s)"));
    mTitleLabel->setAccessibleName("kylin-burner_QLabel_ViewFilterFiles_mTitleLabel");
    mFilterOptionBtn = new KPushButton;
    mFilterOptionBtn->setAccessibleName("kylin-burner_KPushButton_ViewFilterFiles_mFilterOptionBtn");
    mRecoverAllBtn = new KPushButton;
    mRecoverAllBtn->setAccessibleName("kylin-burner_KPushButton_ViewFilterFiles_mRecoverAllBtn");

    mFilterOptionBtn->setText(_("FilterOptionSetting"));
    mRecoverAllBtn->setText(_("RecoverAll"));
    mButtonSpacer1 = new QSpacerItem(8,1,QSizePolicy::Fixed,QSizePolicy::Fixed);
    mButtonSpacer2 = new QSpacerItem(0,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
    mTreeView = new QTreeView;
    mTreeView->setAccessibleName("kylin-burner_QTreeView_ViewFilterFiles_mTreeView");

    //按照Button中内容长度设置button的长度
    QFontMetrics btn1Font(mFilterOptionBtn->font());
    btn1Width = btn1Font.width(mFilterOptionBtn->text());   //获取mFilterOptionBtn组件文本内容的长度
    QFontMetrics btn2Font(mRecoverAllBtn->font());
    btn2Width = btn2Font.width(mRecoverAllBtn->text());   //获取mFilterOptionBtn组件文本内容的长度

    mWindowTitle->setFixedSize(700,40);
    mWindowTitle->move(0,0);
    mWindowTitle->onlyShowInDialog();
    mWindowTitle->setIconAndTitle(QIcon::fromTheme("burner"), _("Burner"));
    mMainWdiget->setFixedSize(700,490);
    mMainWdiget->move(0, 40);

    m_fileFilterRules = new FileFilterRules(this);
    m_fileFilterRules->setWindowFlags(Qt::Dialog);
}

void ViewFilterFiles::initRootLayout()
{
    initTitleLayout();
    mFilterOptionBtn->setFixedWidth(btn1Width+32);
    mRecoverAllBtn->setFixedWidth(btn2Width+32);
    mButtonLayout->addWidget(mFilterOptionBtn);
    mButtonLayout->addItem(mButtonSpacer1);
    mButtonLayout->addWidget(mRecoverAllBtn);
    mButtonLayout->addItem(mButtonSpacer2);

    mFileTreeLayout->addWidget(mTreeView);

    mMainLayout->addLayout(mTitleLayout);
    mMainLayout->addLayout(mButtonLayout);
    mMainLayout->addLayout(mFileTreeLayout);


    mMainWdiget->setLayout(mMainLayout);

    mMainLayout->setSpacing(0);
    mButtonLayout->setSpacing(0);
    mFileTreeLayout->setSpacing(0);
    mMainLayout->setContentsMargins(24,0,24,0);
    mButtonLayout->setContentsMargins(0,16,0,0);
    mFileTreeLayout->setContentsMargins(0,12,0,24);
}

void ViewFilterFiles::initTitleLayout()
{
    //设置QLabel字体颜色
    QFont font;
    font.setPixelSize(16);
    font.setFamily("Noto Sans CJK SC Medium");
    mTitleLabel->setFont(font);

    mTitleLayout->addWidget(mTitleLabel);
    mTitleLayout->setSpacing(0);
    mTitleLayout->setContentsMargins(0,25,0,0);
}

void ViewFilterFiles::initSignalConnect()
{
    connect(mWindowTitle, &WindowTitle::close, this, &ViewFilterFiles::close);
    connect(this, SIGNAL(windowIconChanged(QIcon)), mWindowTitle, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), mWindowTitle, SLOT(onWindowTitleChange(QString)));

    connect(mRecoverAllBtn, &KPushButton::clicked, this, &ViewFilterFiles::slotRestoreFiles);


    connect(mFilterOptionBtn, &KPushButton::clicked, this, &ViewFilterFiles::slotShowFilterRules);

    connect(mComboBoxDisk, SIGNAL(currentIndexChanged(int)), this, SLOT(updateIndex(int)));
    connect(this, SIGNAL(delNode(int)), this, SLOT(deleteNode(int)));
    connect(m_oModel, &FileFilterModel::node, [=](void *cdrom, QIcon icon, QString dis){
        mCDROMList << cdrom;
        mComboBoxDisk->addItem(icon, dis);
        updateIndex(mCDROMList.size()-1);
    });
    connect(m_oModel, &FileFilterModel::root, [=](QModelIndex idx){
        mTreeView->setRootIndex(idx);
    });
    connect(m_oModel, &FileFilterModel::allFilesRestored,this, &ViewFilterFiles::allFilesRestored);
}

void ViewFilterFiles::initTreeWidget()
{
    QFont font;
    font = mRecoverAllBtn->font();
    font.setPixelSize(14);

    mTreeView->setModel(m_oModel);
    mTreeView->setFont(font);
    mTreeView->setAlternatingRowColors(true);
    mTreeView->header()->setFixedHeight(40);
    mTreeView->setAttribute(Qt::WA_TranslucentBackground, true);
    m_oModel->setHeaderData(0, Qt::Horizontal, _("FileName"), Qt::DisplayRole);
    m_oModel->setHeaderData(1, Qt::Horizontal, _("FilePath"), Qt::DisplayRole);
    m_oModel->setHeaderData(2, Qt::Horizontal, _("FileTime"), Qt::DisplayRole);
    mTreeView->sortByColumn(2);
    mTreeView->header()->setSectionHidden(2, true);
    mTreeView->setSelectionMode(QTreeView::ExtendedSelection);
    mTreeView->setItemsExpandable(false);
    mTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);

    qreal basic = width() / 10;
    mTreeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    mTreeView->header()->resizeSection(0, basic * 4);
    mTreeView->header()->resizeSection(1, basic * 5);
}
