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

#define PERSONALSIE_SCHEMA     "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define CONTAIN_PERSONALSIE_TRAN_KEY   "transparency"

LeftsiderbarWidget::LeftsiderbarWidget(QWidget *parent, StartMode mode)
    : QWidget(parent),
      m_navigationBar(nullptr)
{
    m_parent = parent;
    m_mode = mode;

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

    // 功能列表
    if (StartMode::casper == m_mode) {
        m_deplayFuncs << FuncTypeConverter::FunType::RESTORE_SYSTEM;
    } else if (StartMode::stdUser == m_mode) {
        m_deplayFuncs << FuncTypeConverter::FunType::BACKUP_DATA
                      << FuncTypeConverter::FunType::RESTORE_DATA;
                      //<< FuncTypeConverter::FunType::OPERATION_LOG;
    } else {
        m_deplayFuncs << FuncTypeConverter::FunType::BACKUP_SYSTEM
                      << FuncTypeConverter::FunType::RESTORE_SYSTEM
                      << FuncTypeConverter::FunType::BACKUP_DATA
                      << FuncTypeConverter::FunType::RESTORE_DATA;
                      //<< FuncTypeConverter::FunType::OPERATION_LOG
                      //<< FuncTypeConverter::FunType::GHOST_IMAGE;
    }

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
    QColor color = palette().color(QPalette::Window);
    if (m_parent != nullptr) {
        color = m_parent->palette().color(QPalette::Window);
    }
    color.setAlphaF(m_transparency);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    m_titleLabel->setElidedText(tr("Backup & Restore"));
    if (m_titleLabel->text() != tr("Backup & Restore"))
        m_titleLabel->setToolTip(tr("Backup & Restore"));
}
