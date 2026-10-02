/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "leftsiderbarwidget.h"
#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include "../common/mydefine.h"
#include "component/myiconbutton.h"
#include "component/doubleclickwidget.h"


#include "../common/mydefine.h"
#include "../common/dynamiccreator.h"
#include "backup_manager_interface.h"
#include "globalbackupinfo.h"
#include "messageboxutils.h"
#include "aboutdialog.h"
#include "gsettingswrapper.h"
#include "component/mywidget.h"
#include "kconf2.h"
#include <libkydatacollect.h>


#define PERSONALSIE_SCHEMA     "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define CONTAIN_PERSONALSIE_TRAN_KEY   "transparency"

LeftsiderbarWidget::LeftsiderbarWidget(QWidget *parent, StartMode mode)
    : QWidget(parent),
      m_navigationBar(nullptr)
{
    m_parent = parent;
    m_mode = mode;

    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setBlurEffect(QRegion(),0,true);

    //设置标记值，准备刷新毛玻璃
    m_active = true;

    this->setAutoFillBackground(true);

    initUi();

    m_personalQgsettings = nullptr;
    if (QGSettings::isSchemaInstalled(PERSONALSIE_SCHEMA)) {
        m_personalQgsettings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);
        m_transparency = m_personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble();
        connect(m_personalQgsettings,&QGSettings::changed,this,[=](QString changedKey) {  //监听透明度变化
            if (changedKey == CONTAIN_PERSONALSIE_TRAN_KEY) {
                m_transparency = m_personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble();
                // 透明度设置转移到paintEvent中
                // this->setWindowOpacity(m_transparency);
                this->repaint();
            }
        });
    }

    installEventFilter(this);

    this->update();
}

void LeftsiderbarWidget::initUi()
{
    m_leftSideBarVLayout = new QVBoxLayout;
    m_leftSideBarVLayout->setObjectName("m_leftSideBarVLayout");
    m_leftSideBarVLayout->setContentsMargins(0,0,0,0);
    m_leftSideBarVLayout->setSpacing(5);
    setLayout(m_leftSideBarVLayout);

    DoubleClickWidget *titleWidget = new DoubleClickWidget(this);
    titleWidget->setObjectName("yhkylin-backup-tools_kybackup_DoubleClickWidget_titleWidget");
    m_leftSideBarVLayout->addWidget(titleWidget);
    connect(titleWidget, &DoubleClickWidget::doubleClicked, this, &LeftsiderbarWidget::doubleClicked);

    QHBoxLayout * titleLayout = new QHBoxLayout;
    m_mTitleIcon  = new PixmapLabel;
    m_mTitleIcon->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_m_mTitleIcon");
    m_mTitleIcon->setFixedSize(24, 24);
    m_mTitleIcon->setThemeIconSchema(THEME_YHKYLIN_BACKUP_TOOLS, ":/images/yhkylin-backup-tools.png");
    m_titleLabel  = new MyLabel;
    m_titleLabel->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_m_titleLabel");
    // m_titleLabel->setFixedWidth(this->width() - 40);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleLabel->setElidedText(tr("Backup & Restore"));
    if (m_titleLabel->text() != tr("Backup & Restore"))
        m_titleLabel->setToolTip(tr("Backup & Restore"));
    titleWidget->setLayout(titleLayout);
    titleLayout->addWidget(m_mTitleIcon);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->setAlignment(Qt::AlignLeft);
    m_leftSideBarVLayout->addSpacing(10);

    // 初始化配置
    KConf2 *settings = new KConf2("com:kylin:backup.backupstatus");
    if (NULL == settings) {
        return;
    }

    // 功能列表
    if (StartMode::casper == m_mode) {
        m_deplayFuncs << FuncTypeConverter::FunType::RESTORE_SYSTEM;
        // 读取键值，false表示禁止或隐藏，true表示允许
        if (KConf2::isSettingsAvailable("com:kylin:backup.backupstatus")) {
            bool is_enable_systemrestore = settings->get("systemrestore").toBool();
            if (!is_enable_systemrestore && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::RESTORE_SYSTEM);
        } else {
            qDebug() << "com:kylin:backup.backupstatus not found";
        }
    } else if (StartMode::stdUser == m_mode) {
        m_deplayFuncs << FuncTypeConverter::FunType::BACKUP_DATA
                      << FuncTypeConverter::FunType::RESTORE_DATA;
                      //<< FuncTypeConverter::FunType::OPERATION_LOG;
        // 读取键值，false表示禁止或隐藏，true表示允许
        if (KConf2::isSettingsAvailable("com:kylin:backup.backupstatus")) {
            bool is_enable_databackup =  settings->get("databackup").toBool();
            bool is_enable_datarestore = settings->get("datarestore").toBool();
            if (!is_enable_databackup && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::BACKUP_DATA);
            if (!is_enable_datarestore && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::RESTORE_DATA);
        } else {
            qDebug() << "com:kylin:backup.backupstatus not found";
        }
    } else {
        m_deplayFuncs << FuncTypeConverter::FunType::BACKUP_SYSTEM
                      << FuncTypeConverter::FunType::RESTORE_SYSTEM
                      << FuncTypeConverter::FunType::BACKUP_DATA
                      << FuncTypeConverter::FunType::RESTORE_DATA
                      << FuncTypeConverter::FunType::OPERATION_LOG
                      << FuncTypeConverter::FunType::GHOST_IMAGE;
        // 读取键值，false表示禁止或隐藏，true表示允许
        if (KConf2::isSettingsAvailable("com:kylin:backup.backupstatus")) {
            bool is_enable_systembackup = settings->get("systembackup").toBool();
            bool is_enable_systemrestore = settings->get("systemrestore").toBool();
            bool is_enable_databackup =  settings->get("databackup").toBool();
            bool is_enable_datarestore = settings->get("datarestore").toBool();
            if (!is_enable_systembackup && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::BACKUP_SYSTEM);
            if (!is_enable_systemrestore && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::RESTORE_SYSTEM);
            if (!is_enable_databackup && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::BACKUP_DATA);
            if (!is_enable_datarestore && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::RESTORE_DATA);
            if (!is_enable_datarestore && !m_deplayFuncs.isEmpty())
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::GHOST_IMAGE);
        }else{
            qDebug() << "com:kylin:backup.backupstatus not found";
        }
    }

    // 监听键值变化
    connect(settings, &KConf2::changed, this, [=](const QString &key) {
        bool is_enable = settings->get(key).toBool();
        if (!is_enable) {
            // 需要隐藏
            if (key == "systembackup") {
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::BACKUP_SYSTEM);
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            } else if (key == "systemrestore") {
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::RESTORE_SYSTEM);
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            } else if (key == "databackup") {
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::BACKUP_DATA);
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            } else if (key == "datarestore") {
                m_deplayFuncs.removeOne(FuncTypeConverter::FunType::RESTORE_DATA);
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            }
        } else {
            // 需要显示
            if (key == "systembackup") {
                m_deplayFuncs.append(FuncTypeConverter::FunType::BACKUP_SYSTEM);
                std::sort(m_deplayFuncs.begin(), m_deplayFuncs.end());
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            } else if (key == "systemrestore") {
                m_deplayFuncs.append(FuncTypeConverter::FunType::RESTORE_SYSTEM);
                std::sort(m_deplayFuncs.begin(), m_deplayFuncs.end());
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            } else if (key == "databackup") {
                m_deplayFuncs.append(FuncTypeConverter::FunType::BACKUP_DATA);
                std::sort(m_deplayFuncs.begin(), m_deplayFuncs.end());
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            } else if (key == "datarestore") {
                m_deplayFuncs.append(FuncTypeConverter::FunType::RESTORE_DATA);
                std::sort(m_deplayFuncs.begin(), m_deplayFuncs.end());
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            }else if (key == "ghostimage") {
                m_deplayFuncs.append(FuncTypeConverter::FunType::GHOST_IMAGE);
                std::sort(m_deplayFuncs.begin(), m_deplayFuncs.end());
                qDebug() << m_deplayFuncs;
                emit funcType(m_deplayFuncs);
            }
        }
        m_idxToFunctype.clear();
        m_functypeToIdx.clear();
        if (m_navigationBar != NULL) {
            delete m_navigationBar;
        }

        m_navigationBar = new KNavigationBar(this);
        m_navigationBar->setObjectName("yhkylin-backup-tools_kybackup_KNavigationBar_m_navigationBar");
        m_navigationBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        FuncTypeConverter kvConverter;
        for (int index = 0; index < m_deplayFuncs.size(); ++index) {
            int funcType = m_deplayFuncs.at(index);

            QStandardItem * item = new QStandardItem;

            QString themeIconName = kvConverter.funcTypeToThemeIconName(funcType);
            QString mnamei18nString  = kvConverter.funcTypeToFuncName(funcType); //设置界面显示文本
            QString defaultIconName(":/symbos/");
            defaultIconName += themeIconName;
            QIcon icon = QIcon::fromTheme(themeIconName, QIcon(defaultIconName));
            item->setIcon(icon);
            item->setText(mnamei18nString);
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            // item->setToolTip(mnamei18nString);
            item->setCheckable(true);
            m_navigationBar->addItem(item);

            m_idxToFunctype.insert(index, funcType);
            m_functypeToIdx.insert(funcType, index);
        }
        connect(m_navigationBar->listview(), &QListView::clicked, [=](const QModelIndex &index) {
            int row = index.row();
            int type = this->m_idxToFunctype.value(row, -1);
            emit this->selected(type);
        });

        QHBoxLayout * hbarLayout = new QHBoxLayout;
        hbarLayout->setContentsMargins(10, 0, 10, 0);
        hbarLayout->addWidget(m_navigationBar);
        m_leftSideBarVLayout->addLayout(hbarLayout);

//        m_leftSideBarVLayout->addSpacing(8);
        m_leftSideBarVLayout->setAlignment(Qt::AlignTop);
        if (!m_deplayFuncs.isEmpty()) {
            this->setCheckedFunc(m_deplayFuncs[0]);
        }
    });

    m_idxToFunctype.clear();
    m_functypeToIdx.clear();

    m_navigationBar = new KNavigationBar(this);
    m_navigationBar->setObjectName("yhkylin-backup-tools_kybackup_KNavigationBar_m_navigationBar");
    m_navigationBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    FuncTypeConverter kvConverter;
    for (int index = 0; index < m_deplayFuncs.size(); ++index) {
        int funcType = m_deplayFuncs.at(index);

        QStandardItem * item = new QStandardItem;

        QString themeIconName = kvConverter.funcTypeToThemeIconName(funcType);
        QString mnamei18nString  = kvConverter.funcTypeToFuncName(funcType); //设置界面显示文本
        QString defaultIconName(":/symbos/");
        defaultIconName += themeIconName;
        QIcon icon = QIcon::fromTheme(themeIconName, QIcon(defaultIconName));
        item->setIcon(icon);
        item->setText(mnamei18nString);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        // item->setToolTip(mnamei18nString);
        item->setCheckable(true);
        m_navigationBar->addItem(item);

        m_idxToFunctype.insert(index, funcType);
        m_functypeToIdx.insert(funcType, index);
    }
    connect(m_navigationBar->listview(), &QListView::clicked, [=](const QModelIndex &index) {
        int row = index.row();
        int type = this->m_idxToFunctype.value(row, -1);
        emit this->selected(type);
    });

    QHBoxLayout * hbarLayout = new QHBoxLayout;
    hbarLayout->setContentsMargins(10, 0, 10, 0);
    hbarLayout->addWidget(m_navigationBar);
    m_leftSideBarVLayout->addLayout(hbarLayout);

    m_leftSideBarVLayout->addSpacing(8);
    m_leftSideBarVLayout->setAlignment(Qt::AlignTop);
}

LeftsiderbarWidget::~LeftsiderbarWidget()
{

}

void LeftsiderbarWidget::setBusy(bool isBusy)
{
    bool enabled = !isBusy;
    this->m_navigationBar->setEnabled(enabled);
}

void LeftsiderbarWidget::setCheckedFunc(int funcType)
{
    int index = m_functypeToIdx.value(funcType, -1);
    if (index > -1)
        m_navigationBar->listview()->setCurrentIndex(m_navigationBar->model()->index(index, 0));
}


void LeftsiderbarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
//    QColor color = palette().color(QPalette::Window);
//    if (m_parent != nullptr) {
//        color = m_parent->palette().color(QPalette::Window);
//    }

    QColor color;
     QPalette leftSiderPalette;
     if(m_active)
     {
         //获取毛玻璃阶段配色
         color = ThemeController::getPaletteColorFromDT("kcontain-secondary-normal");
         color.setAlphaF(m_transparency);
         leftSiderPalette.setColor(QPalette::Window, color);      // 窗口背景
     }
     else
     {
         //获取毛玻璃关闭时候配色
         color = ThemeController::getPaletteColorFromDT("kcontain-secondary-inactive");
         //取消阶段不透明
         color.setAlphaF(1);
         leftSiderPalette.setColor(QPalette::Window, color);      // 窗口背景
     }
     this->setPalette(leftSiderPalette);
     this->setAutoFillBackground(true);

    //color.setAlphaF(m_transparency);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(color));
    this->setPalette(pal);
//    QBrush brush = QBrush(color);
//    p.setBrush(brush);
//    p.drawRoundedRect(opt.rect,0,0);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);


    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(),color);
    QWidget::paintEvent(event);

    m_titleLabel->setElidedText(tr("Backup & Restore"));
    if (m_titleLabel->text() != tr("Backup & Restore"))
        m_titleLabel->setToolTip(tr("Backup & Restore"));
}

bool LeftsiderbarWidget::eventFilter(QObject *obj, QEvent *event)
{
    // 传递给标题文本框
    if (event->type() == QEvent::WindowActivate)
    {   
        if (g_GSettingWrapper.isDarkTheme())
        {
            // 窗口激活
        m_titleLabel->setStyleSheet("color:white;");
        }
        else
        {
            // 窗口激活
        m_titleLabel->setStyleSheet("color:black;");
        }
        
        qDebug()  <<  "左侧边栏激活";
        m_windowHelper->setBlurEffect(QRegion(),0,true);
        m_active = true;

        this->update();
    }
    else if(event->type() == QEvent::WindowDeactivate)
    {
        if (g_GSettingWrapper.isDarkTheme())
        {
            // 窗口未激活
        m_titleLabel->setStyleSheet("color:gray;");
        }
        else
        {
            // 窗口未激活
        m_titleLabel->setStyleSheet("color:gray;");
        }

        qDebug()  <<  "左侧边栏失去焦点";
        m_windowHelper->setBlurEffect(QRegion(),0,false);
        m_active = false;
        this->update();
    }

    return QWidget::eventFilter(obj, event);
}
