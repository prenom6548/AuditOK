/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "systembackup.h"
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMovie>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QtGlobal>
#include <unistd.h>
#include <ksoundeffects.h>

#include "../component/backuppositionselectdialog.h"
#include "../component/clicklabel.h"
#include "../component/circlelabel.h"
#include "../component/myiconlabel.h"
#include "../component/mylabel.h"
#include "../component/mylineedit.h"
#include "../component/mypushbutton.h"
#include "../component/linelabel.h"
#include "../component/ringsprogressbar.h"
#include "../component/pixmaplabel.h"
#include "../component/inputvalidator.h"
#include "../../common/utils.h"
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"
#include "managebackuppointlist.h"
#include "messageboxutils.h"
#include "qpushbutton.h"
#include "kprogresscircle.h"
using namespace kdk;

#include <libkydatacollect.h>

// bigTitle控件前面的宽度：24+spacing
#define LEN_TITLE_PRE 50
// labelCheck1、labelCheck2控件前面的宽度：10+6+5
#define LEN_DOT_PRE 50
#define PICTURE_SIZE 256

IMPLEMENT_DYNCREATE_WITH_ONE_PARAM(SystemBackup, QStackedWidget*, QWidget*)

SystemBackup::SystemBackup(QWidget *parent /*= nullptr*/) :
    QStackedWidget(parent),
    m_udector(new UdiskDetector()),
    m_isLock(false),
    m_isLocal(true),
    m_systemBackupState(SystemBackupState::IDEL),
    m_pInterface(nullptr)
{
    // 界面手写代码创建，作为练手
    initHomePage();
    initSetBackupPathsPage();
    initCheckPage();
    initNameBackupPage();
    initBackupingPage();
    initLastPage();
}

SystemBackup::~SystemBackup()
{
    delete m_udector;
    m_udector = nullptr;

    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 初始化第一个页面
 */
void SystemBackup::initHomePage()
{
    QWidget *homePage = new QWidget;
    homePage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_homePage");
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addStretch();

    // 中部布局包含一个独立的窗口，用于大小变化时局部相对不变
    QWidget *center = new QWidget;
    center->setObjectName("yhkylin-backup-tools_kybackup_QWidget_center");
    center->setFixedSize(760, 540);
    QHBoxLayout *centerHBoxLayout = new QHBoxLayout;
    centerHBoxLayout->addStretch();
    centerHBoxLayout->addWidget(center);
    centerHBoxLayout->addStretch();
    vLayout->addLayout(centerHBoxLayout);
    // 图片
    PixmapLabel *imageBackup = new PixmapLabel(center);
    imageBackup->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_imageBackup");
    imageBackup->setGeometry(center->width() - 40 - PICTURE_SIZE, 145, PICTURE_SIZE, PICTURE_SIZE);
    imageBackup->setScaledContents(true);
    imageBackup->setLightAndDarkPixmap(":/images/system_backup.png", ":/images/system_backup_dark.png");
    imageBackup->setAttribute(Qt::WA_DeleteOnClose,true);

    // 系统备份大字提示
    MyLabel *labelBackup = new MyLabel(center);
    labelBackup->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelBackup");
    labelBackup->setDeplayText(tr("System Backup"));
    labelBackup->setFixedWidth(center->width() - 200);
    labelBackup->setFixedHeight(48);
    labelBackup->move(41, 80);
    // 默认水平左对齐，上下居中对齐；故不需要设置
    // labelBackup->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QFont font;
    font.setBold(true);
    font.setPixelSize(36);
    labelBackup->setFont(font);
    // labelBackup->setAttribute(Qt::WA_TranslucentBackground);
    labelBackup->setScaledContents(true);
    labelBackup->adjustSize();
    labelBackup->setAttribute(Qt::WA_DeleteOnClose,true);

    // 系统备份说明
    MyLabel *labelNote = new MyLabel(center);
    labelNote->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelNote");
    labelNote->setFixedWidth(center->width() - 45);
    labelNote->setFixedHeight(24);
    labelNote->move(41, 140);
    labelNote->setDeplayText(tr("Can be restored when files are damaged or lost"));
    font.setBold(false);
    font.setPixelSize(18);
    labelNote->setFont(font);
    labelNote->setScaledContents(true);
    labelNote->adjustSize();
    labelNote->setAttribute(Qt::WA_DeleteOnClose,true);

    // 多点还原
    MyIconLabel *iconMultiBackup = new MyIconLabel(center);
    iconMultiBackup->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconMultiBackup");
    iconMultiBackup->setGeometry(41, 214, 210, 36);
    iconMultiBackup->setThemeIcon("ukui-multipoint-symbolic", ":/symbos/ukui-multipoint-symbolic");
    iconMultiBackup->setDesplayText(tr("Multi-Spot"));
    iconMultiBackup->setEnabled(false);
    iconMultiBackup->setAttribute(Qt::WA_DeleteOnClose,true);

    // 体积小
    MyIconLabel *iconSmallSize = new MyIconLabel(center);
    iconSmallSize->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSmallSize");
    iconSmallSize->setGeometry(231, 214, 210, 36);
    iconSmallSize->setThemeIcon("ukui-volume-symbolic", ":/symbos/ukui-volume-symbolic");
    iconSmallSize->setDesplayText(tr("Low Disk Usage"));
    iconSmallSize->setEnabled(false);
    iconSmallSize->setAttribute(Qt::WA_DeleteOnClose,true);

    // 安全
    MyIconLabel *iconSecurity = new MyIconLabel(center);
    iconSecurity->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSecurity");
    iconSecurity->setGeometry(41, 266, 180, 36);
    iconSecurity->setThemeIcon("ukui-bf-security-symbolic", ":/symbos/ukui-bf-security-symbolic");
    iconSecurity->setDesplayText(tr("Security"));
    iconSecurity->setEnabled(false);
    iconSecurity->setAttribute(Qt::WA_DeleteOnClose,true);

    // 操作简单
    MyIconLabel *iconSimple = new MyIconLabel(center);
    iconSimple->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSimple");
    iconSimple->setGeometry(231, 266, 180, 36);
    iconSimple->setThemeIcon("ukui-bf-simple-symbolic", ":/symbos/ukui-bf-simple-symbolic");
    iconSimple->setDesplayText(tr("Simple"));
    iconSimple->setEnabled(false);
    iconSimple->setAttribute(Qt::WA_DeleteOnClose,true);

    // 开始备份按钮
    MyPushButton *beginBackup = new MyPushButton(center);
    beginBackup->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_beginBackup");
    beginBackup->setGeometry(41, 342, 180, 52);
    beginBackup->setText(tr("Start Backup"));
    beginBackup->setEnabled(true);
    // beginBackup->setAutoRepeat(true);
    beginBackup->setProperty("isImportant", true);
    font.setPixelSize(24);
    beginBackup->setFont(font);
    beginBackup->setAttribute(Qt::WA_DeleteOnClose,true);
    if (beginBackup->text() == "Start Backup" || beginBackup->text() == "开始备份")
        beginBackup->setFixedWidth(180);
    else {
        QFontMetrics fontMetrics(font);
        int fontSize = fontMetrics.width(beginBackup->text());
        if (180 > fontSize * 1.2)
            beginBackup->setFixedWidth(180);
        else
            beginBackup->setFixedWidth(fontSize * 1.2);
    }
    connect(beginBackup, &MyPushButton::clicked, this, [=](bool clicked) {
        this->m_prefixDestPath = "";
        this->m_backupName = "";
        this->on_next_clicked(clicked);
    });

    // 底部控件布局
    QHBoxLayout *bottomHBoxLayout = new QHBoxLayout;
    bottomHBoxLayout->addStretch();

    // 备份管理
    backupPointManage = new KBorderlessButton("borderless",this);
    backupPointManage->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_backupPointManage");
    backupPointManage->setText(tr("System Backup Management >>"));
    backupPointManage->setFlat(true);
    backupPointManage->setProperty("useButtonPalette", true);
    QPalette pal(backupPointManage->palette());
    pal.setColor(QPalette::ButtonText, this->palette().highlight().color());
    pal.setColor(QPalette::Text, this->palette().highlight().color());
    pal.setColor(QPalette::Button, this->palette().base().color());
    backupPointManage->setPalette(pal);
    bottomHBoxLayout->addWidget(backupPointManage);
    bottomHBoxLayout->addSpacing(20);
    bottomHBoxLayout->setAlignment(Qt::AlignRight);

    vLayout->addStretch();
    vLayout->addLayout(bottomHBoxLayout);
    vLayout->addSpacing(20);
    homePage->setLayout(vLayout);

    connect(backupPointManage, &MyPushButton::clicked, this, [=]() {
        ManageBackupPointList backupManager(this, ManageBackupPointList::SYSTEM);
        backupManager.setModal(true);
        QPointF position = parentWidget()->mapToGlobal(QPoint(0,0));
        backupManager.move(position.x()-100+parentWidget()->width()/2-backupManager.width()/2,position.y()-20+parentWidget()->height()/2-backupManager.height()/2);
        backupManager.exec();
    });
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=](bool isDark) {
        Q_UNUSED(isDark)
        // 深浅主题切换时，因为调色板已经更换，高亮等颜色已经改变，所以要重新加载图标。
        // from席博文：因为按钮的文字颜色换了一个类型，因为设计非要什么半透明透明效果的；Text就是半透明的，buttontext就是不半透明。所以将ButtonText改为Text
        QPalette pal(backupPointManage->palette());
        pal.setColor(QPalette::ButtonText, this->palette().highlight().color());
        pal.setColor(QPalette::Text, this->palette().highlight().color());
        pal.setColor(QPalette::Button, this->palette().base().color());
        backupPointManage->setPalette(pal);
    });
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeColorChanged, this, [=]() {
        // 强调色更换，高亮等颜色已经改变，所以要重新加载图标。
        // from席博文：因为按钮的文字颜色换了一个类型，因为设计非要什么半透明透明效果的；Text就是半透明的，buttontext就是不半透明。所以将ButtonText改为Text
        QPalette pal(backupPointManage->palette());
        pal.setColor(QPalette::ButtonText, this->palette().highlight().color());
        pal.setColor(QPalette::Text, this->palette().highlight().color());
        pal.setColor(QPalette::Button, this->palette().base().color());
        backupPointManage->setPalette(pal);
    });

    addWidget(homePage);


    // 系统备份初始界面打开的时候，通过dbus接口唤醒后台daemon进程
    qDebug() << "wake up daemon";
    GlobelBackupInfo::instance().setIsBusy(true);
    ComKylinBackupManagerInterface *m_pInterface;
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.systembackup", "/update", QDBusConnection::systemBus());

    BackupWrapper backupWrapper;
    backupWrapper.m_frontUserName = qgetenv("USER");
    m_pInterface->wakeUpDaemon(backupWrapper);

    GlobelBackupInfo::instance().setIsBusy(false);
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief “上一步”按钮响应槽
 * @param checked
 */
void SystemBackup::on_pre_clicked(bool checked)
{
    Q_UNUSED(checked)
    int index = this->currentIndex() - 1;
    if (index >= 0) {
        this->setCurrentIndex(index);
    }
}

/**
 * @brief “开始备份”“下一步”按钮响应槽
 * @param checked
 */
void SystemBackup::on_next_clicked(bool checked)
{
    Q_UNUSED(checked)
    int index = this->currentIndex() + 1;
    if (index < this->count()) {
        this->setCurrentIndex(index);
    }
}

/**
 * @brief 初始化选择备份文件页面
 */
void SystemBackup::initSetBackupPathsPage()
{
    QWidget *selectPathPage = new QWidget;
    selectPathPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_selectPathPage");
    // 纵向布局
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);

    // 第一行
    QHBoxLayout * hlayoutLine1 = new QHBoxLayout;
    hlayoutLine1->addSpacing(40);
    // 备份路径选择提示
    MyLabel* labelSelect = new MyLabel(selectPathPage);
    labelSelect->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelSelect");
    labelSelect->setDeplayText(tr("Please select backup position"));
    labelSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QFont font;
    font.setBold(true);
    font.setPixelSize(18);
    labelSelect->setFont(font);
    hlayoutLine1->addWidget(labelSelect);
    hlayoutLine1->setAlignment(Qt::AlignLeft);
    vlayout->addLayout(hlayoutLine1);

    // 第二行
    vlayout->addSpacing(5);
    QHBoxLayout * hlayoutLine2 = new QHBoxLayout;
    hlayoutLine2->addSpacing(40);
    // 备份路径选择框
    QComboBox* comboSelect = new QComboBox(selectPathPage);
    comboSelect->setObjectName("yhkylin-backup-tools_kybackup_QComboBox_comboSelect");
    comboSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QPushButton *buttonBrowse = new QPushButton;
    buttonBrowse->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_buttonBrowse");
    buttonBrowse->setText(tr("Browse"));
    buttonBrowse->setEnabled(true);
    hlayoutLine2->addWidget(comboSelect);
    hlayoutLine2->addWidget(buttonBrowse);
    buttonBrowse->setVisible(false);
    hlayoutLine2->addSpacing(40);
    vlayout->addLayout(hlayoutLine2);
    vlayout->addSpacing(56);

    // 第三行
    QHBoxLayout * hlayoutLine3 = new QHBoxLayout;
    hlayoutLine3->setSpacing(16);
    hlayoutLine3->addStretch();
    // 上一步按钮
    QPushButton *preStep = new QPushButton(selectPathPage);
    preStep->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_preStep");
    preStep->setText(tr("Back"));
    preStep->setEnabled(true);
    // preStep->setAutoRepeat(true);
    connect(preStep, &QPushButton::clicked, preStep, &QPushButton::clearFocus);
    connect(preStep, &MyPushButton::clicked, this, &SystemBackup::on_pre_clicked);
    hlayoutLine3->addWidget(preStep);

    // 下一步按钮
    MyPushButton *nextStep = new MyPushButton(selectPathPage);
    nextStep->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_nextStep");
    nextStep->setText(tr("Next"));
    nextStep->setEnabled(false);
    // nextStep->setAutoRepeat(true);
    nextStep->setProperty("isImportant", true);
    hlayoutLine3->addWidget(nextStep);
    hlayoutLine3->addStretch();
    hlayoutLine3->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutLine3);

    vlayout->addStretch();
    selectPathPage->setLayout(vlayout);

    addWidget(selectPathPage);

    // 添加本地默认路径、移动设备目录
    connect(m_udector, &UdiskDetector::udiskListChanged, this, [=](QList<QStorageInfo> diskList) {
        m_isLock = true;
        int oldItemCount = comboSelect->count();

        comboSelect->clear();
        this->m_udiskPaths.clear();
        QIcon iconFolder = QIcon::fromTheme("insync-folder", QIcon(":/images/insync-folder.png"));

        // 如果有备份分区，则将本地默认分区备份路径放在第一个
        if (GlobelBackupInfo::instance().hasBackupPartition()) {
            QString qsLocalDefaultPath = Utils::getBackupRootPath() + BACKUP_SNAPSHOTS_PATH;
            qsLocalDefaultPath.replace("//", "/");
            this->m_udiskPaths << Utils::getBackupRootPath();
            comboSelect->addItem(iconFolder, tr("local default path : ") + qsLocalDefaultPath);
        }

        QString qsPreDevPath(tr("removable devices path : "));
        for (QStorageInfo& disk : diskList) {
            this->m_udiskPaths << disk.rootPath();
            qDebug()<<" directory is " << disk.rootPath() + UDISK_BACKUP_SNAPSHOTS_PATH;
            comboSelect->addItem(iconFolder, qsPreDevPath + disk.rootPath() + UDISK_BACKUP_SNAPSHOTS_PATH);
        }

        if (!this->m_customizePath.isEmpty()) {
            comboSelect->setCurrentIndex(-1);
            QLineEdit * edit = new QLineEdit;
            edit->setObjectName("yhkylin-backup-tools_kybackup_QLineEdit_edit");
            comboSelect->setLineEdit(edit);
            edit->setText(tr("customize path : ") + this->m_customizePath + BACKUP_SNAPSHOTS_PATH);
            edit->setReadOnly(true);
        } else if (comboSelect->count() > 0) {
            nextStep->setEnabled(true);
        } else {
            nextStep->setEnabled(false);
        }

        int newItemCount = comboSelect->count();
        if (newItemCount < oldItemCount) {
            // 拔出U盘，取消备份
            if (m_pInterface)
                m_pInterface->cancel();
        }

        m_isLock = false;
    });
    connect(comboSelect, QOverload<int>::of(&QComboBox::activated), this, [=]() {
        if (!m_isLock) {
            this->m_customizePath = "";
            nextStep->setEnabled(true);
        }
    });
    m_udector->getStorageInfo();
    connect(buttonBrowse, &QPushButton::clicked, this, [=](){
        buttonBrowse->clearFocus();
        // 是否自定义路径？自定义路径备份文件不受保护，可能导致备份文件丢失或损坏
        if (!MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(GlobelBackupInfo::instance().getMainWidget(), QObject::tr("Information"),
                                                          QObject::tr("Are you sure to continue customizing the path?\nThe custom path backup file is not protected, which may cause the backup file to be lost or damaged"),
                                                          QObject::tr("OK"), QObject::tr("Cancel")))
        {
            return ;
        }

        BackupPositionSelectDialog fileDialog(this);
        if (fileDialog.exec() == QDialog::Accepted) {
            QStringList selectFiles = fileDialog.selectedFiles();
            if (!selectFiles.isEmpty()) {
                QString fileName = selectFiles.at(0);

                if (Utils::isSubPath(this->m_udiskPaths, fileName)) {
                    // 选择了移动设备挂载目录的子孙目录
                    // 移动设备只能选择挂载目录本身
                    MessageBoxUtils::QMESSAGE_BOX_WARNING(GlobelBackupInfo::instance().getMainWidget(), QObject::tr("Information"),
                                                          QObject::tr("Mobile devices can only select the mount directory itself."),
                                                          QObject::tr("OK"));
                    return ;
                }

                if (GlobelBackupInfo::instance().hasBackupPartition() && fileName == Utils::getSysRootPath()) {
                    comboSelect->setCurrentIndex(0);
                    this->m_customizePath = "";
                } else if (this->m_udiskPaths.contains(fileName)) {
                    comboSelect->setCurrentIndex(this->m_udiskPaths.indexOf(fileName));
                    this->m_customizePath = "";
                } else {
                    comboSelect->setCurrentIndex(-1);
                    this->m_customizePath = fileName;

                    QLineEdit * edit = new QLineEdit;
                    edit->setObjectName("yhkylin-backup-tools_kybackup_QLineEdit_edit");
                    comboSelect->setLineEdit(edit);
                    edit->setText(tr("customize path : ") + QString(fileName + BACKUP_SNAPSHOTS_PATH).replace("//", "/"));
                    edit->setReadOnly(true);
                    nextStep->setEnabled(true);
                }
            }
        }
    });
    connect(nextStep, &MyPushButton::clicked, this, [=](bool checked) {
        // 备份路径选择索引
        int index = comboSelect->currentIndex();
        if (index >= 0) {
            // 第一个选项是本地系统备份
            this->m_isLocal = GlobelBackupInfo::instance().hasBackupPartition() && (index == 0);
            this->m_prefixDestPath = this->m_udiskPaths.at(index);
        } else if (this->m_customizePath.isEmpty()) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(GlobelBackupInfo::instance().getMainWidget(), QObject::tr("Warning"),
                                                  QObject::tr("Please select backup position"),
                                                  QObject::tr("OK"));
            return ;
        } else {
            // 自定义备份位置的场景
            this->m_isLocal = true;
            this->m_prefixDestPath = this->m_customizePath;
        }
        this->on_next_clicked(checked);
        emit this->startCheckEnv();
    });
}

/**
 * @brief 初始化检测中页面
 */
void SystemBackup::initCheckPage()
{
    QWidget *checkPage = new QWidget;
    checkPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_checkPage");
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", checkPage, 24, QColor(COLOR_BLUE));
    one->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_one");
    LineLabel *line1 = new LineLabel(checkPage, QColor(COLOR_BLUE));
    line1->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line1");
    CircleLable *two = new CircleLable("2", checkPage);
    two->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_two");
    LineLabel *line2 = new LineLabel(checkPage);
    line2->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line2");
    CircleLable *three = new CircleLable("3", checkPage);
    three->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_three");
    LineLabel *line3 = new LineLabel(checkPage);
    line3->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line3");
    CircleLable *four = new CircleLable("4", checkPage);
    four->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_four");

    QHBoxLayout *hlayoutTopLine1 = new QHBoxLayout;
    hlayoutTopLine1->addSpacing(80);
    hlayoutTopLine1->addWidget(one);
    hlayoutTopLine1->addWidget(line1);
    hlayoutTopLine1->addWidget(two);
    hlayoutTopLine1->addWidget(line2);
    hlayoutTopLine1->addWidget(three);
    hlayoutTopLine1->addWidget(line3);
    hlayoutTopLine1->addWidget(four);
    hlayoutTopLine1->addSpacing(80);
    vlayout->addLayout(hlayoutTopLine1);

    MyLabel *label1 = new MyLabel(tr("checking"), checkPage);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("preparing"), checkPage);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setFixedWidth(164);
    label2->setEnabled(false);
    MyLabel *label3 = new MyLabel(tr("backuping"), checkPage);
    label3->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label3");
    label3->setFixedWidth(164);
    label3->setEnabled(false);
    MyLabel *label4 = new MyLabel(tr("finished"), checkPage);
    label4->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label4");
    label4->setFixedWidth(164);
    label4->setEnabled(false);

    QHBoxLayout *hlayoutTopLine2 = new QHBoxLayout;
    hlayoutTopLine2->addSpacing(11);
    hlayoutTopLine2->addWidget(label1);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label2);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label3);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label4);
    hlayoutTopLine2->addSpacing(11);
    vlayout->addLayout(hlayoutTopLine2);

    vlayout->addSpacing(80);

    // ------------ 中部布局begin-------------
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addSpacing(20);
    QWidget *centerFont = new QWidget(checkPage);
    centerFont->setObjectName("yhkylin-backup-tools_kybackup_QWidget_centerFont");
    centerFont->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *vlayoutCenterFont = new QVBoxLayout;

    // 第一行
    QHBoxLayout *hlayoutCenterFont1 = new QHBoxLayout;
    // 检测等待图标
    QLabel *loadingGif = new QLabel(centerFont);
    loadingGif->setObjectName("yhkylin-backup-tools_kybackup_QLabel_loadingGif");
    loadingGif->setFixedSize(24,24);
    // 环境检测等待动画
    QMovie *movie = new QMovie(":/images/loading.gif", QByteArray(), centerFont);
    movie->setObjectName("yhkylin-backup-tools_kybackup_QMovie_movie");
    loadingGif->setMovie(movie);
    hlayoutCenterFont1->addWidget(loadingGif);
    // 检测结果对错图标
    PixmapLabel *resultLogo = new PixmapLabel(centerFont);
    resultLogo->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_resultLogo");
    resultLogo->setFixedSize(24,24);
    hlayoutCenterFont1->addWidget(resultLogo);
    // 检测中大标题
    MyLabel *bigTitle = new MyLabel(centerFont);
    bigTitle->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_bigTitle");
    bigTitle->setFontSize(24);
    bigTitle->setIsOriginal(true);
    bigTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont1->addWidget(bigTitle);
    hlayoutCenterFont1->setAlignment(Qt::AlignLeft);
    vlayoutCenterFont->addLayout(hlayoutCenterFont1);

    // 第二行
    vlayoutCenterFont->addSpacing(16);
    QHBoxLayout *hlayoutCenterFont2 = new QHBoxLayout;
    hlayoutCenterFont2->addSpacing(10);
    // 检测中的记录：黑点1和文字1
    CircleLable *dot1 = new CircleLable(QString(""), centerFont, 6, Qt::black);
    dot1->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_dot1");
    hlayoutCenterFont2->addWidget(dot1);
    hlayoutCenterFont2->addSpacing(5);
    MyLabel *labelCheck1 = new MyLabel(centerFont);
    labelCheck1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelCheck1");
    labelCheck1->setIsOriginal(true);
    labelCheck1->setWordWrap(true);
    labelCheck1->adjustSize();
    labelCheck1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont2->addWidget(labelCheck1);
    hlayoutCenterFont2->setAlignment(Qt::AlignLeft);
    vlayoutCenterFont->addLayout(hlayoutCenterFont2);

    // 第三行
    QHBoxLayout *hlayoutCenterFont3 = new QHBoxLayout;
    hlayoutCenterFont3->addSpacing(10);
    // 检测中的记录：黑点2和文字2
    CircleLable *dot2 = new CircleLable(QString(""), centerFont, 6, Qt::black);
    dot2->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_dot2");
    hlayoutCenterFont3->addWidget(dot2);
    hlayoutCenterFont3->addSpacing(5);
    MyLabel *labelCheck2 = new MyLabel(centerFont);
    labelCheck2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelCheck2");
    labelCheck2->setIsOriginal(true);
    labelCheck2->setWordWrap(true);
    labelCheck2->adjustSize();
    labelCheck2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont3->addWidget(labelCheck2);
    hlayoutCenterFont3->setAlignment(Qt::AlignLeft);
    vlayoutCenterFont->addLayout(hlayoutCenterFont3);

    // 第四行
    vlayoutCenterFont->addSpacing(56);

    // 第五行
    QHBoxLayout *hlayoutCenterFont5 = new QHBoxLayout;
    hlayoutCenterFont5->setSpacing(16);
    hlayoutCenterFont5->addStretch();
    // 上一步按钮
    QPushButton *preStep = new QPushButton(centerFont);
    preStep->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_preStep");
    preStep->setText(tr("Back"));
    preStep->setEnabled(true);
    // preStep->setAutoRepeat(true);
    connect(preStep, &QPushButton::clicked, preStep, &QPushButton::clearFocus);
    connect(preStep, &QPushButton::clicked, this, &SystemBackup::on_pre_clicked);
    hlayoutCenterFont5->addWidget(preStep);
    // 下一步按钮
    MyPushButton *nextStep = new MyPushButton(centerFont);
    nextStep->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_nextStep");
    nextStep->setText(tr("Next"));
    nextStep->setEnabled(true);
    // nextStep->setAutoRepeat(true);
    nextStep->setProperty("isImportant", true);
    connect(nextStep, &MyPushButton::clicked, this, [=](bool checked) {
        this->on_next_clicked(checked);
        emit this->clearBackupName();
    });
    hlayoutCenterFont5->addWidget(nextStep);
    // 重新检测按钮
    MyPushButton *recheck = new MyPushButton(centerFont);
    recheck->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_recheck");
    recheck->setText(tr("Recheck"));
    recheck->setEnabled(true);
    // recheck->setAutoRepeat(true);
    recheck->setProperty("isImportant", true);
    connect(recheck, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        emit this->startCheckEnv();
    });
    hlayoutCenterFont5->addWidget(recheck);
    hlayoutCenterFont5->addStretch();
    hlayoutCenterFont5->setAlignment(Qt::AlignCenter);
    vlayoutCenterFont->addLayout(hlayoutCenterFont5);

    centerFont->setLayout(vlayoutCenterFont);
    hlayout->addWidget(centerFont);
    hlayout->addSpacing(20);
    hlayout->addStretch();
    hlayout->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayout);

    vlayout->addStretch();
    checkPage->setLayout(vlayout);
    // ------------ 中部布局end-------------

    // 开始检测
    connect(this, &SystemBackup::startCheckEnv, this, [=]() {
        this->m_systemBackupState = SystemBackupState::CHECKING;
        loadingGif->setVisible(true);
        movie->start();
        resultLogo->setVisible(false);
        // 环境检测中，请等待
        bigTitle->setDeplayText(tr("Checking, wait a moment ..."));
        dot1->setBackgroundColor(Qt::black);
        dot2->setBackgroundColor(Qt::black);
        labelCheck1->setFontColor(Qt::black);
        labelCheck2->setFontColor(Qt::black);
        // 备份过程中不要做其它操作，以防数据丢失
        labelCheck1->setDeplayText(tr("Do not use computer during backup in case of data loss"));
        if (this->m_isLocal) {
            // 检测备份分区空间是否充足···
            labelCheck2->setDeplayText(tr("Check whether the remaining capacity of the backup partition is sufficient"));
        } else {
            // 检测移动设备空间是否充足···
            labelCheck2->setDeplayText(tr("Check whether the remaining capacity of the removable device is sufficient"));
        }
        preStep->setVisible(false);
        nextStep->setVisible(false);
        recheck->setVisible(false);

        this->on_checkEnv_start();
        emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();
    });

    // 检测结果
    connect(this, &SystemBackup::checkEnvResult, this, [=](bool result, const QString &errMsg, const QString &errTip) {
        loadingGif->setVisible(false);
        movie->stop();

        if (result) {
            resultLogo->setThemeIconSchema("ukui-dialog-success", ":/symbos/ukui-dialog-success");
            resultLogo->setVisible(true);
            // 检测成功
            bigTitle->setDeplayText(tr("Check success"));
            // 备份空间充足
            labelCheck1->setDeplayText(tr("The storage for backup is enough"));
            dot2->setBackgroundColor(COLOR_YELLOW);
            labelCheck2->setFontColor(COLOR_YELLOW);
            labelCheck2->setFontWordWrap(true);
            // 请确保电脑已连接电源或电量超过60%
            labelCheck2->setDeplayText(tr("Make sure the computer is plugged in or the battery level is above 60%"));
            dot1->setVisible(true);
            dot2->setVisible(true);
            labelCheck1->setVisible(true);
            labelCheck2->setVisible(true);
            nextStep->setVisible(true);
            recheck->setVisible(false);
        } else {
            resultLogo->setThemeIconSchema("dialog-error", ":/symbos/dialog-error");
            resultLogo->setVisible(true);
            // 环境校验失败
            bigTitle->setDeplayText(tr("Check failure"));
            labelCheck1->setDeplayText(errMsg);
            labelCheck2->setDeplayText(errTip);
            if (errMsg.isEmpty()) {
                dot1->setVisible(false);
                labelCheck1->setVisible(false);
            } else {
                dot1->setVisible(true);
                labelCheck1->setVisible(true);
            }
            if (errTip.isEmpty()) {
                dot2->setVisible(false);
                labelCheck2->setVisible(false);
            } else {
                dot2->setVisible(true);
                labelCheck2->setVisible(true);
            }
            recheck->setVisible(true);
            nextStep->setVisible(false);
        }

        preStep->setVisible(true);
        emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();
    });

    // 调整中部控件大小
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::widthChanged, this, [=](){
        if (!isVisible()) {
            return;
        }

        labelCheck1->setMaximumWidth(DEFAULT_MAX_SIZE);
        labelCheck2->setMaximumWidth(DEFAULT_MAX_SIZE);
        // 控件bigTitle的宽度
        int len_title = 0;
        if (bigTitle->isVisible()) {
            QFontMetrics fontMetrics(bigTitle->font());
            len_title = fontMetrics.width(bigTitle->getOriginalText());
            len_title += LEN_TITLE_PRE;
        }
        // 控件labelCheck1的宽度
        int len_labelCheck1 = 0;
        if (labelCheck1->isVisible()) {
            QFontMetrics fontMetrics(labelCheck1->font());
            len_labelCheck1 = fontMetrics.width(labelCheck1->getOriginalText());
            len_labelCheck1 += LEN_DOT_PRE;
        }
        // 控件labelCheck2的宽度
        int len_labelCheck2 = 0;
        if (labelCheck2->isVisible()) {
            QFontMetrics fontMetrics(labelCheck2->font());
            len_labelCheck2 = fontMetrics.width(labelCheck2->getOriginalText());
            len_labelCheck2 += LEN_DOT_PRE;
        }
        int len_max = len_title > len_labelCheck1 ? len_title : len_labelCheck1;
        len_max = len_max > len_labelCheck2 ? len_max : len_labelCheck2;
        if (len_max >= this->width() - 20*2)
            len_max = this->width() - 20*2;

        centerFont->setFixedWidth(len_max);
        if (labelCheck1->isVisible() && len_labelCheck1 > len_max) {
            labelCheck1->setMaximumWidth(len_max - LEN_DOT_PRE);
            labelCheck1->setFontWordWrap(true);
            labelCheck1->adjustSize();
        }
        if (labelCheck2->isVisible() && len_labelCheck2 > len_max) {
            labelCheck2->setMaximumWidth(len_max - LEN_DOT_PRE);
            labelCheck2->setFontWordWrap(true);
            labelCheck2->adjustSize();
        }
    });

    addWidget(checkPage);
}



/**
 * @brief 开始进行环境检测
 */
void SystemBackup::on_checkEnv_start()
{
    qDebug() << "环境检测:开始" ;
    GlobelBackupInfo::instance().setIsBusy(true);
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.systembackup", "/update", QDBusConnection::systemBus(), this);
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemBackup::on_checkEnv_end);

    // 是否已存在备份、还原等操作
    bool isActive = false;
    if(int(BackupState::BACKUP_STATE_INIT) != m_pInterface->getBackupState(isActive)){
        on_checkEnv_end(int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING));

        return;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_backupName = m_backupName;
    backupWrapper.m_type = BackupType::BACKUP_SYSTEM;
    backupWrapper.m_User = BackupUser::USER;
    //2025-5-15 目前只有本地和udisk两种方式
   // backupWrapper.m_iPosition = m_isLocal ? (this->m_prefixDestPath.isEmpty() ? BackupPosition::LOCAL : BackupPosition::CUSTOMIZE) : BackupPosition::UDISK;
    backupWrapper.m_iPosition = m_isLocal ? BackupPosition::LOCAL: BackupPosition::UDISK;
    QString backupPath = Utils::getSysRootPath();
    backupPath += "/";
    backupPath.replace("//", "/");
    backupWrapper.m_backupPaths << backupPath;
    backupWrapper.m_prefixDestPath = m_prefixDestPath;
    backupWrapper.m_backupExcludePaths.append(Utils::getFromExcludePathsFile());
    if (backupWrapper.m_iPosition == BackupPosition::CUSTOMIZE){
        QString prefixPath = m_prefixDestPath;
        prefixPath += BACKUP_SNAPSHOTS_PATH;
        prefixPath.replace("//", "/");
        backupWrapper.m_backupExcludePaths << prefixPath;
    }
    backupWrapper.m_frontUid = getuid();
    backupWrapper.m_frontUserName = qgetenv("USER");
    backupWrapper.m_gid = getgid();
    m_pInterface->checkEnv(backupWrapper);
}

/**
 * @brief 环境检测结束
 * @param result, 环境校验结果
 */
void SystemBackup::on_checkEnv_end(int result)
{
    qDebug() << "环境检测:结束" ;
    m_systemBackupState = SystemBackupState::IDEL;
    bool bRst = false;
    QString errMsg, errTip;
    switch (result) {
    case int(BackupResult::LOCK_PROGRAM_FAIL):
        // 程序锁定失败，请重试
        errMsg = tr("Program lock failed, please retry");
        // 可能有其它备份/还原等任务在执行
        errTip = tr("There may be other backups or restores being performed");
        break;
    case int(BackupResult::NO_FOUND_DEALCLASS):
        // 不支持的任务类型
        errMsg = tr("Unsupported task type");
        // 没有找到相应的处理逻辑
        errTip = tr("No processing logic was found in the service");
        break;
    case int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL):
        // 备份分区挂载失败
        errMsg = tr("Failed to mount the backup partition");
        // 检查是否有备份分区
        errTip = tr("Check whether there is a backup partition");
        break;
    case int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_NOT_EXT4):
        // 移动设备的文件系统不是ext4格式
        errMsg = tr("The filesystem of device is not ext4 format");
        // 请换成ext4格式
        errTip = tr("Please change filesystem format to ext4");
        break;
    case int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_UDF):
        // 移动设备的文件系统是udf格式
        errMsg = tr("The filesystem of device is udf format");
        // 请换成ext4、ntfs等格式
        errTip = tr("Please change filesystem format to ext3、ext4 or ntfs");
        break;
    case int(BackupResult::UDISK_FILESYSTEM_IS_READONLY):
        // 移动设备是只读的
        errMsg = tr("The device is read only");
        // 请修改为可读写权限的
        errTip = tr("Please chmod to rw");
        break;
    case int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH):
        // 备份空间不足
        errMsg = tr("The storage for backup is not enough");
        // 建议释放空间后重试
        errTip = tr("Retry after release space");
        break;
    case int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING):
        // 其它备份还原等操作正在执行
        errMsg = tr("Other backup or restore task is being performed");
        // 请稍后重试
        errTip = tr("Please try again later");
        break;
    case int(BackupResult::CHECK_UDISK_TYPE_FAIL):
        // Udisk是vfat类型
        errMsg = tr("Error: FAT-formatted removable device detected.");
        errTip = tr("Format the removable storage device to NTFS exFAT or ext4 file system.");
        break;
    default:
        bRst = true;
        break;
    }

    Utils::wait(1);
    emit checkEnvResult(bRst, errMsg, errTip);
    GlobelBackupInfo::instance().setIsBusy(false);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemBackup::on_checkEnv_end);
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 初始化备份命名页面
 */
#define MAX_LEN_BACKUPNAME 64
void SystemBackup::initNameBackupPage()
{
    QWidget *nameBackupPage = new QWidget;
    nameBackupPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_nameBackupPage");
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", nameBackupPage, 24, QColor(COLOR_BLUE));
    one->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_one");
    LineLabel *line1 = new LineLabel(nameBackupPage, QColor(COLOR_BLUE));
    line1->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line1");
    CircleLable *two = new CircleLable("2", nameBackupPage, 24, QColor(COLOR_BLUE));
    two->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_two");
    LineLabel *line2 = new LineLabel(nameBackupPage, QColor(COLOR_BLUE));
    line2->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line2");
    CircleLable *three = new CircleLable("3", nameBackupPage);
    three->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_three");
    LineLabel *line3 = new LineLabel(nameBackupPage);
    line3->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line3");
    CircleLable *four = new CircleLable("4", nameBackupPage);
    four->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_four");

    QHBoxLayout *hlayoutTopLine1 = new QHBoxLayout;
    hlayoutTopLine1->addSpacing(80);
    hlayoutTopLine1->addWidget(one);
    hlayoutTopLine1->addWidget(line1);
    hlayoutTopLine1->addWidget(two);
    hlayoutTopLine1->addWidget(line2);
    hlayoutTopLine1->addWidget(three);
    hlayoutTopLine1->addWidget(line3);
    hlayoutTopLine1->addWidget(four);
    hlayoutTopLine1->addSpacing(80);
    vlayout->addLayout(hlayoutTopLine1);

    MyLabel *label1 = new MyLabel(tr("checking"), nameBackupPage);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("preparing"), nameBackupPage);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setFontColor(QColor(COLOR_BLUE));
    label2->setFixedWidth(164);
    MyLabel *label3 = new MyLabel(tr("backuping"), nameBackupPage);
    label3->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label3");
    label3->setFixedWidth(164);
    label3->setEnabled(false);
    MyLabel *label4 = new MyLabel(tr("finished"), nameBackupPage);
    label4->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label4");
    label4->setFixedWidth(164);
    label4->setEnabled(false);

    QHBoxLayout *hlayoutTopLine2 = new QHBoxLayout;
    hlayoutTopLine2->addSpacing(11);
    hlayoutTopLine2->addWidget(label1);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label2);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label3);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label4);
    hlayoutTopLine2->addSpacing(11);
    vlayout->addLayout(hlayoutTopLine2);

    vlayout->addSpacing(80);

    // ------------ 中部布局begin-------------

    // 中部第一行
    QHBoxLayout *hlayoutCenterLine1 = new QHBoxLayout;
    // 备份名称
    MyLabel *labelBackupName = new MyLabel(nameBackupPage);
    labelBackupName->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelBackupName");
    labelBackupName->setDeplayText(tr("Backup Name"));
    QFontMetrics fontMetrics(labelBackupName->font());
    int fontSize = fontMetrics.width(labelBackupName->getOriginalText());
    labelBackupName->setFixedWidth(fontSize);
    MyLineEdit *editBackupName = new MyLineEdit(nameBackupPage);
    editBackupName->setObjectName("yhkylin-backup-tools_kybackup_MyLineEdit_editBackupName");
    editBackupName->setAccessibleName("yhkylin-backup-tools_kybackup_MyLineEdit_editBackupName");
    editBackupName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    editBackupName->setMinimumWidth(350);
    editBackupName->setMaxLength(64);
    editBackupName->setPlaceholderText(QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss"));
    // 支持输入中英文数字和部分字符
    // QRegExp regx("^[\u4e00-\u9fa5a-zA-Z0-9~!@#$%^&*()-_+={}':;'\\[\\].<>/? ￥（）——；《》‘’：“”、？]+$");  //其中匹配中文[\u4e00-\u9fa5]
    QRegExp regx("[^<>,;`|#\\^\\$]+");//regx("^[\u4e00-\u9fa5a-zA-Z0-9-@& +():'（）——《》‘’：“”]*$");
    std::unique_ptr<InputValidator> validator(new InputValidator(regx));
    //InputValidator *validator = new InputValidator(regx);
    validator->setObjectName("yhkylin-backup-tools_kybackup_InputValidator_validator");
    editBackupName->setValidator(validator.get());
    labelBackupName->setFixedHeight(editBackupName->height());

    hlayoutCenterLine1->addStretch();
    hlayoutCenterLine1->addWidget(labelBackupName);
    hlayoutCenterLine1->addWidget(editBackupName);
    hlayoutCenterLine1->addStretch();
    hlayoutCenterLine1->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutCenterLine1);

    // 备份名称错误提示
    MyLabel *labelError = new MyLabel(nameBackupPage);
    labelError->setFixedSize(editBackupName->size());
    labelError->setFontSize(14);
    labelError->setFontColor(Qt::red);
    labelError->setVisible(false);
    labelError->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::widthChanged, this, [=]{
        if (!isVisible()) {
            return;
        }

        QFontMetrics fontMetrics(labelBackupName->font());
        int fontSize = fontMetrics.width(labelBackupName->getOriginalText());
        labelBackupName->setFixedWidth(fontSize);

        if (labelError->isVisible())
            labelError->move(editBackupName->geometry().left(), editBackupName->geometry().bottom() + 10);
    });

    connect(validator.get(), &InputValidator::checked, this, [=](bool valid, QString in, int pos, QChar c) {
        if (in.isEmpty())
            return;
        QString text = editBackupName->text();
        if (valid) {
            if (pos >= MAX_LEN_BACKUPNAME) {
                labelError->setDeplayText(tr("Maximum length reached"));
                labelError->setVisible(true);
                labelError->move(editBackupName->geometry().left(), editBackupName->geometry().bottom() + 10);
            } else if (!text.isEmpty() && this->isExistsBackupName(text)) {
                labelError->setDeplayText(tr("Name already exists"));
                labelError->setVisible(true);
            }  else if (text == in) {
                labelError->setVisible(false);
            }
        } else {
            labelError->setDeplayText(tr("Unsupported symbol : ") + c);
            labelError->setVisible(true);
            labelError->move(editBackupName->geometry().left(), editBackupName->geometry().bottom() + 10);
        }
    });

    connect(editBackupName, &MyLineEdit::textChanged, this, [=](const QString &text) {
        if (!text.isEmpty() && this->isExistsBackupName(text)) {
            labelError->setDeplayText(tr("Name already exists"));
            labelError->setVisible(true);

            labelError->move(editBackupName->geometry().left(), editBackupName->geometry().bottom() + 10);
        } else if (text.isEmpty()) {
            labelError->setDeplayText("");
            labelError->setVisible(false);
        } else {
            // 这里面的情况包含在了上面的InputValidator::checked信号处理逻辑中
        }
    });
    connect(this, &SystemBackup::clearBackupName, this, [=]() {
        this->m_backupName = "";
        editBackupName->setText("");
        editBackupName->setPlaceholderText(QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss"));
        editBackupName->setFocus();
        labelError->setDeplayText("");
        labelError->setVisible(false);
    });

    vlayout->addSpacing(56);
    // 中部第三行
    QHBoxLayout *hlayoutCenterLine3 = new QHBoxLayout;
    hlayoutCenterLine3->setSpacing(16);
    hlayoutCenterLine3->addStretch();
    // 上一步按钮
    QPushButton *preStep = new QPushButton(nameBackupPage);
    preStep->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_preStep");
    preStep->setText(tr("Back"));
    preStep->setEnabled(true);
    // preStep->setAutoRepeat(true);
    connect(preStep, &QPushButton::clicked, preStep, &QPushButton::clearFocus);
    // connect(preStep, &QPushButton::clicked, this, &SystemBackup::on_pre_clicked);
    connect(preStep, &QPushButton::clicked, this, [=](){
        this->setCurrentIndex(SELECT_PATH_PAGE);
    });
    hlayoutCenterLine3->addWidget(preStep);

    // 下一步按钮
    MyPushButton *nextStep = new MyPushButton(nameBackupPage);
    nextStep->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_nextStep");
    nextStep->setText(tr("Next"));
    nextStep->setEnabled(true);
    // nextStep->setAutoRepeat(true);
    nextStep->setProperty("isImportant", true);
    connect(nextStep, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        /*if (labelError->isVisible()) {
            editBackupName->setFocus();
        } else*/ {
            this->m_backupName = editBackupName->text();
            if (this->m_backupName.isEmpty()) {
                this->m_backupName = editBackupName->placeholderText();
            }

            if (this->isExistsBackupName(this->m_backupName)) {
                labelError->setDeplayText(tr("Name already exists"));
                labelError->setVisible(true);
                this->m_backupName = "";
                editBackupName->setFocus();
            } else {
                this->on_next_clicked(checked);
                emit this->startBackup();
            }
        }
    });

    hlayoutCenterLine3->addWidget(nextStep);
    hlayoutCenterLine3->addStretch();
    hlayoutCenterLine3->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutCenterLine3);

    vlayout->addStretch();
    nameBackupPage->setLayout(vlayout);

    addWidget(nameBackupPage);
}

/**
 * @brief 获取本地和移动设备中的备份点
 * @return
 */
QList<ParseBackupList::BackupPoint> SystemBackup::getBackupPointList()
{
    QList<ParseBackupList::BackupPoint> list;
    qDebug() << "获取备份点列表：";
    for (QString backupPath : m_udiskPaths) {
        qDebug() << "backupPath：" << backupPath;
        if (backupPath.isEmpty())
            backupPath = Utils::getSysRootPath();
        backupPath += SYS_BACKUP_XML_PATH;

        QFile xmlFile(backupPath);
        if (!xmlFile.exists() || 0 == xmlFile.size()) {
            continue;
        }

        ParseBackupList parse(backupPath);
        list.append(parse.getBackupPointList());
    }

    // 获取系统更新的备份点信息
    QString xmlPath = Utils::getSysRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    list.append(parse.getBackupPointList());

    return list;
}

/**
 * @brief SystemBackup::isExistsBackupName
 * @param backupName
 * @return bool true-已存在；false-不存在
 */
bool SystemBackup::isExistsBackupName(const QString & backupName)
{
    // 首先，校验唯一性文件中
    QString settingPath = Utils::getSysRootPath() + UDISK_UNIQUE_SETTINGS;
    settingPath.replace("//", "/");
    QSettings udisk_setting(settingPath, QSettings::IniFormat);
    udisk_setting.setIniCodec(QTextCodec::codecForName("utf-8"));
    udisk_setting.beginGroup(backupName);
    QString uuidFound = udisk_setting.value("uuid").toString();
    udisk_setting.endGroup();
    if (!uuidFound.isEmpty())
        return true;

    // 不能取名为出厂备份
    if (backupName == tr("factory backup"))
        return true;

    return false;

    // 然后，校验xml和log中的备份记录（这一步只是为了保险而兼容老版本的备份，以前的唯一性文件中只有U盘的备份）
//    QList<ParseBackupList::BackupPoint> backupPointList = getBackupPointList();
//    QList<BackupWrapper> list = Utils::getBackupLogList();
//    QSet<QString> setName;
//    for (BackupWrapper& wraper : list) {
//        if (wraper.m_backupName == wraper.m_uuid) {
//            // 如果在xml中不存在，则说明是此备份点是之前存在的备份点，现在已经删除
//            for (const ParseBackupList::BackupPoint & backupPonit : backupPointList) {
//                if (backupPonit.m_uuid == wraper.m_uuid) {
//                    wraper.m_backupName = backupPonit.m_backupName;
//                }
//            }
//        }

//        // type==8为删除备份点操作
//        if (BackupType::DELETE_BACKUP == wraper.m_type)
//            setName.remove(wraper.m_backupName);
//        else
//            setName.insert(wraper.m_backupName);
//    }

//    return setName.contains(backupName);
}

/**
 * @brief 初始化备份中页面
 */
void SystemBackup::initBackupingPage()
{
    QWidget *backupingPage = new QWidget;
    backupingPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_backupingPage");
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setSpacing(10);
    vlayout->addSpacing(40);

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", backupingPage, 24, QColor(COLOR_BLUE));
    one->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_one");
    LineLabel *line1 = new LineLabel(backupingPage, QColor(COLOR_BLUE));
    line1->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line1");
    CircleLable *two = new CircleLable("2", backupingPage, 24, QColor(COLOR_BLUE));
    two->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_two");
    LineLabel *line2 = new LineLabel(backupingPage, QColor(COLOR_BLUE));
    line2->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line2");
    CircleLable *three = new CircleLable("3", backupingPage, 24, QColor(COLOR_BLUE));
    three->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_three");
    LineLabel *line3 = new LineLabel(backupingPage, QColor(COLOR_BLUE));
    line3->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line3");
    CircleLable *four = new CircleLable("4", backupingPage);
    four->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_four");

    QHBoxLayout *hlayoutTopLine1 = new QHBoxLayout;
    hlayoutTopLine1->addSpacing(80);
    hlayoutTopLine1->addWidget(one);
    hlayoutTopLine1->addWidget(line1);
    hlayoutTopLine1->addWidget(two);
    hlayoutTopLine1->addWidget(line2);
    hlayoutTopLine1->addWidget(three);
    hlayoutTopLine1->addWidget(line3);
    hlayoutTopLine1->addWidget(four);
    hlayoutTopLine1->addSpacing(80);
    vlayout->addLayout(hlayoutTopLine1);

    MyLabel *label1 = new MyLabel(tr("checking"), backupingPage);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("preparing"), backupingPage);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setFontColor(QColor(COLOR_BLUE));
    label2->setFixedWidth(164);
    MyLabel *label3 = new MyLabel(tr("backuping"), backupingPage);
    label3->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label3");
    label3->setFontColor(QColor(COLOR_BLUE));
    label3->setFixedWidth(164);
    MyLabel *label4 = new MyLabel(tr("finished"), backupingPage);
    label4->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label4");
    label4->setFixedWidth(164);
    label4->setEnabled(false);

    QHBoxLayout *hlayoutTopLine2 = new QHBoxLayout;
    hlayoutTopLine2->addSpacing(11);
    hlayoutTopLine2->addWidget(label1);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label2);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label3);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label4);
    hlayoutTopLine2->addSpacing(11);
    vlayout->addLayout(hlayoutTopLine2);

    vlayout->addSpacing(80);

    // ------------ 中部布局begin-------------
    // 中部第一行
    QHBoxLayout *hlayoutCenterFont1 = new QHBoxLayout;
    // 检测等待图标
    QLabel *loadingGif = new QLabel(backupingPage);
    loadingGif->setObjectName("yhkylin-backup-tools_kybackup_QLabel_loadingGif");
    // 环境检测等待动画
    QMovie *movie = new QMovie(":/images/loading.gif", QByteArray(), backupingPage);
    movie->setObjectName("yhkylin-backup-tools_kybackup_QMovie_movie");
    loadingGif->setMovie(movie);
    loadingGif->setFixedSize(24,24);
    loadingGif->setVisible(false);
    // 进度条
   // RingsProgressbar *progressBar = new RingsProgressbar(backupingPage);
    KProgressCircle *progressBar = new KProgressCircle(backupingPage);
    progressBar->setObjectName("yhkylin-backup-tools_kybackup_KProgressCircle_progressBar");
    progressBar->setState(ProgressBarState::NormalProgress);
    progressBar->setFixedSize(100, 100);
    hlayoutCenterFont1->addStretch();
    hlayoutCenterFont1->addWidget(loadingGif);
    hlayoutCenterFont1->addWidget(progressBar);
    hlayoutCenterFont1->addStretch();
    hlayoutCenterFont1->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutCenterFont1);

    // 第二行
    vlayout->addSpacing(16);
    QHBoxLayout *hlayoutCenterFont2 = new QHBoxLayout;
    // 提醒
    MyLabel *labelTip = new MyLabel(backupingPage);
    labelTip->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelTip");
    labelTip->setAlignment(Qt::AlignCenter);
    labelTip->setIsOriginal(true);
    labelTip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont2->addStretch();
    hlayoutCenterFont2->addWidget(labelTip);
    hlayoutCenterFont2->addStretch();
    hlayoutCenterFont2->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutCenterFont2);

    // 第二行(隐)
    QHBoxLayout *hlayoutCenterFont2_1 = new QHBoxLayout;
    // 备份过程提醒信息
    MyLabel *labelTip_1 = new MyLabel(backupingPage);
    labelTip_1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelTip_1");
    labelTip_1->setAlignment(Qt::AlignCenter);
    labelTip_1->setIsOriginal(true);
    labelTip_1->setFontWordWrap(true);
    labelTip_1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    labelTip_1->setDeplayText("");
    hlayoutCenterFont2_1->addStretch();
    hlayoutCenterFont2_1->addWidget(labelTip_1);
    hlayoutCenterFont2_1->addStretch();
    hlayoutCenterFont2_1->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutCenterFont2_1);

    vlayout->addSpacing(56);
    // 第三行
    QHBoxLayout *hlayoutCenterFont3 = new QHBoxLayout;
    // 取消按钮
    MyPushButton *cancel = new MyPushButton(backupingPage);
    cancel->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_cancel");
    cancel->setText(tr("Cancel"));
    // cancel->setAutoRepeat(true);
    cancel->setProperty("isImportant", true);
    hlayoutCenterFont3->addStretch();
    hlayoutCenterFont3->addWidget(cancel);
    hlayoutCenterFont3->addStretch();
    hlayoutCenterFont3->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutCenterFont3);

    vlayout->addStretch();
    backupingPage->setLayout(vlayout);

    // ------------ 中部布局end-------------

    connect(this, &SystemBackup::backupWarnning, labelTip_1, [=](const QString& msg) {
        labelTip_1->setVisible(true);
        labelTip_1->setDeplayText(msg);
    });

    // 开始备份
    connect(this, &SystemBackup::startBackup, this, [=] {
      //  progressBar->setPersent(0);
        progressBar->setState(ProgressBarState::NormalProgress);
        progressBar->setValue(0);
        movie->start();
        // 不要使用电脑，以防数据丢失
        labelTip->setDeplayText(tr("Do not use computer in case of data loss"));
        labelTip_1->setVisible(false);
        labelTip_1->setDeplayText("");
        cancel->setEnabled(true);

        // 开始备份
        this->on_backup_start();
    });

    // 进度
    connect(this, &SystemBackup::progress, this, [=](int state, int rate) {
        Q_UNUSED(state)
        //progressBar->setPersent(rate);
        progressBar->setState(ProgressBarState::NormalProgress);
        progressBar->setValue(rate);
    });

    // 取消备份
    connect(cancel, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        // 确定取消当前操作？
        if (!MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(GlobelBackupInfo::instance().getMainWidget(), QObject::tr("Information"), QObject::tr("Are you sure to cancel the operation？"), QObject::tr("OK"), QObject::tr("Cancel"))) {
            return ;
        }
        cancel->setEnabled(false);
        if (m_pInterface)
            m_pInterface->cancel();
    });

    addWidget(backupingPage);
}

/**
 * @brief 开始进行系统备份
 */
void SystemBackup::on_backup_start()
{
    //数据埋点：关键功能
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    //传入事件描述“打开文件”，所在界面“主界面”后，上传事件
    kdk_dia_upload_default(node,"systembackup","systembackup");
    //释放内存
    kdk_dia_data_free(node);

    qDebug() << "执行系统备份:开始" ;
    GlobelBackupInfo::instance().setIsBusy(true);
    m_systemBackupState = SystemBackupState::BACKUPING;
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.systembackup", "/update", QDBusConnection::systemBus(), this);
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemBackup::on_checkBackup_end);
    connect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &SystemBackup::progress);
    connect(m_pInterface, &ComKylinBackupManagerInterface::backupFinished, this, &SystemBackup::on_backup_end);

    // 是否已存在备份、还原等操作
    bool isActive = false;
    if(int(BackupState::BACKUP_STATE_INIT) != m_pInterface->getBackupState(isActive)){
        on_checkBackup_end(int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING));
        qDebug()<<" block return when backup ";
        return;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_backupName = m_backupName;
    backupWrapper.m_uuid = Utils::createUuid();
    qDebug() << "UUID:" << backupWrapper.m_uuid;
    backupWrapper.m_uuid.replace("{", "");
    backupWrapper.m_uuid.replace("}", "");
    QString xmlPath = Utils::getBackupRootPath() + SYS_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parseXml(xmlPath);
    ParseBackupList::BackupPoint backupPoint = parseXml.findBackupPointByUuid(backupWrapper.m_uuid);
    if (backupWrapper.m_backupName.isEmpty())
    {
        if (backupPoint.m_backupName.isEmpty())
            backupWrapper.m_backupName = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        else
            backupWrapper.m_backupName = backupPoint.m_backupName;
    }
    backupWrapper.m_type = BackupType::BACKUP_SYSTEM;
    backupWrapper.m_prefixDestPath = m_prefixDestPath;
    qDebug()<<"m_isLocal on_backup_start function: " << m_isLocal;
    //2025-5-15 目前只有本地和udisk两种方式
   // backupWrapper.m_iPosition = m_isLocal ? (this->m_prefixDestPath.isEmpty() ? BackupPosition::LOCAL : BackupPosition::CUSTOMIZE) : BackupPosition::UDISK;
    backupWrapper.m_iPosition = m_isLocal ? BackupPosition::LOCAL: BackupPosition::UDISK;

    qDebug()<<"m_iPosition on_backup_start function: " << backupWrapper.m_iPosition;
    backupWrapper.m_backupPaths << "/";
    if(backupWrapper.m_iPosition == BackupPosition::CUSTOMIZE){
        backupWrapper.m_backupExcludePaths = Utils::getFromExcludePathsFile();
        backupWrapper.m_backupExcludePaths << "/home";
        backupWrapper.m_backupExcludePaths << "/root";
        backupWrapper.m_backupExcludePaths << "/data";
    }
    backupWrapper.m_User = BackupUser::USER;
    backupWrapper.m_frontUid = getuid();
    backupWrapper.m_frontUserName = qgetenv("USER");
    backupWrapper.m_gid = getgid();

    qDebug() << "当前用户名：" << backupWrapper.m_frontUserName;
    qDebug() << "执行dbus调用:" << "backupSystemByUser";
    m_pInterface->backupSystemByUser(backupWrapper);
}

/**
 * @brief 系统备份校验结果处理
 * @param result
 */
void SystemBackup::on_checkBackup_end(int result)
{
    bool bRst = false;
    QString errMsg, errTip;
    switch (result) {
    case int(BackupResult::LOCK_PROGRAM_FAIL):
        // 程序锁定失败，请重试
        errMsg = tr("Program lock failed, please retry");
        // 可能有其它备份/还原等任务在执行
        errTip = tr("There may be other backups or restores being performed");
        break;
    case int(BackupResult::NO_FOUND_DEALCLASS):
        // 不支持的任务类型
        errMsg = tr("Unsupported task type");
        // 没有找到相应的处理逻辑
        errTip = tr("No processing logic was found in the service");
        break;
    case int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL):
        // 备份分区挂载失败
        errMsg = tr("Failed to mount the backup partition");
        // 检查是否有备份分区
        errTip = tr("Check whether there is a backup partition");
        break;
    case int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_VFAT):
        // 移动设备的文件系统是vfat格式
        errMsg = tr("The filesystem of device is vfat format");
        // 请换成ext4、ntfs等格式
        errTip = tr("Please change filesystem format to ext3、ext4 or ntfs");
        break;
    case int(BackupResult::UDISK_FILESYSTEM_IS_READONLY):
        // 移动设备是只读的
        errMsg = tr("The device is read only");
        // 请修改为可读写权限的
        errTip = tr("Please chmod to rw");
        break;
    case int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH):
        // 备份空间不足
        errMsg = tr("The storage for backup is not enough");
        // 建议释放空间后重试
        errTip = tr("Retry after release space");
        break;
    case int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING):
        // 其它备份还原等操作正在执行
        errMsg = tr("Other backup or restore task is being performed");
        // 请稍后重试
        errTip = tr("Please try again later");
        break;
    case int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED):
    case int(BackupResult::WRITE_STORAGEINFO_ADD_ITEM_FAIL):
    case int(BackupResult::WRITE_STORAGEINFO_UPDATE_ITEM_FAIL):
        // 创建备份点目录失败
        errMsg = tr("Failed to create the backup point directory");
        // 请检查备份目录是否有写权限
        errTip = tr("Please check backup partition permissions");
        break;
    case int(BackupResult::MKSQUASHFS_START_SUCCESS):
        // 正压缩系统到本地磁盘，请耐心等待...
        errTip = tr("The system is being compressed to the local disk, please wait patiently...");
        emit this->backupWarnning(errTip);
        return;
    case int(BackupResult::MKSQUASHFS_DO_FAIL):
        break;
    case int(BackupResult::BACKUP_START_SUCCESS): // 非移动设备备份不需要此信号
    case int(BackupResult::MKSQUASHFS_DO_SUCCESS):
        // 正在传输image文件到移动设备，即将完成...
        errTip = tr("Transferring image file to mobile device, about to be completed...");
        emit this->backupWarnning(errTip);
        return;
    case int(BackupResult::CANCEL_SUCCESS):
        // 已经取消本次备份
        errMsg = tr("The backup had been canceled");
        // 如需要可重新发起备份
        errTip = tr("Re-initiate the backup if necessary");
        break;
    case int(BackupResult::CHECK_UDISK_TYPE_FAIL):
        // Udisk是vfat类型
        errMsg = tr("Error: FAT-formatted removable device detected.");
        errTip = tr("Format the removable storage device to NTFS exFAT or ext4 file system.");
        break;
    default:
        bRst = true;
        break;
    }

    if (!bRst) {
        GlobelBackupInfo::instance().setIsBusy(false);
        m_systemBackupState = SystemBackupState::IDEL;
        this->on_next_clicked(true);
        if (errMsg.isEmpty()) {
            // 备份过程中出现错误
            errMsg = tr("An error occurred during backup");
            // 错误信息参考日志文件：/var/log/backup.log
            errTip = tr("Error messages refer to log file : /data/.backup/backup.log");
        }
        emit this->checkBackupResult(bRst, errMsg, errTip);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemBackup::on_checkEnv_end);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &SystemBackup::progress);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::backupFinished, this, &SystemBackup::on_backup_end);
        delete m_pInterface;
        m_pInterface = nullptr;
    }
}

/**
 * @brief 系统备份结束
 * @param result-false 失败; true 成功
 */
void SystemBackup::on_backup_end(bool result)
{
    m_systemBackupState = SystemBackupState::IDEL;

    this->on_next_clicked(true);
    if (result) {
        emit checkBackupResult(result);
    } else {
        // 备份过程中出现错误
        QString errMsg = tr("An error occurred during backup");
        // 错误信息参考日志文件：/var/log/backup.log
        QString errTip = tr("Error messages refer to log file : /data/.backup/backup.log");
        emit checkBackupResult(result, errMsg, errTip);
    }

    GlobelBackupInfo::instance().setIsBusy(false);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemBackup::on_checkEnv_end);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &SystemBackup::progress);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::backupFinished, this, &SystemBackup::on_backup_end);
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 最后备份结果界面
 */
void SystemBackup::initLastPage()
{
    QWidget *last = new QWidget;
    last->setObjectName("yhkylin-backup-tools_kybackup_QWidget_last");
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", last, 24, QColor(COLOR_BLUE));
    one->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_one");
    LineLabel *line1 = new LineLabel(last, QColor(COLOR_BLUE));
    line1->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line1");
    CircleLable *two = new CircleLable("2", last, 24, QColor(COLOR_BLUE));
    two->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_two");
    LineLabel *line2 = new LineLabel(last, QColor(COLOR_BLUE));
    line2->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line2");
    CircleLable *three = new CircleLable("3", last, 24, QColor(COLOR_BLUE));
    three->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_three");
    LineLabel *line3 = new LineLabel(last, QColor(COLOR_BLUE));
    line3->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line3");
    CircleLable *four = new CircleLable("4", last, 24, QColor(COLOR_BLUE));
    four->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_four");

    QHBoxLayout *hlayoutTopLine1 = new QHBoxLayout;
    hlayoutTopLine1->addSpacing(80);
    hlayoutTopLine1->addWidget(one);
    hlayoutTopLine1->addWidget(line1);
    hlayoutTopLine1->addWidget(two);
    hlayoutTopLine1->addWidget(line2);
    hlayoutTopLine1->addWidget(three);
    hlayoutTopLine1->addWidget(line3);
    hlayoutTopLine1->addWidget(four);
    hlayoutTopLine1->addSpacing(80);
    vlayout->addLayout(hlayoutTopLine1);

    MyLabel *label1 = new MyLabel(tr("checking"), last);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("preparing"), last);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setFontColor(QColor(COLOR_BLUE));
    label2->setFixedWidth(164);
    MyLabel *label3 = new MyLabel(tr("backuping"), last);
    label3->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label3");
    label3->setFontColor(QColor(COLOR_BLUE));
    label3->setFixedWidth(164);
    MyLabel *label4 = new MyLabel(tr("finished"), last);
    label4->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label4");
    label4->setFontColor(QColor(COLOR_BLUE));
    label4->setFixedWidth(164);

    QHBoxLayout *hlayoutTopLine2 = new QHBoxLayout;
    hlayoutTopLine2->addSpacing(11);
    hlayoutTopLine2->addWidget(label1);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label2);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label3);
    hlayoutTopLine2->addStretch();
    hlayoutTopLine2->addWidget(label4);
    hlayoutTopLine2->addSpacing(11);
    vlayout->addLayout(hlayoutTopLine2);

    vlayout->addSpacing(80);

    //------------ 中部布局begin-------------
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addSpacing(20);
    QWidget *centerFont = new QWidget(last);
    centerFont->setObjectName("yhkylin-backup-tools_kybackup_QWidget_centerFont");
    centerFont->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *vlayoutCenterFont = new QVBoxLayout;

    // 第一行
    QHBoxLayout *hlayoutCenterFont1 = new QHBoxLayout;
    // 备份结果对错图标
    PixmapLabel *resultLogo = new PixmapLabel(centerFont);
    resultLogo->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_resultLogo");
    resultLogo->setFixedSize(24,24);
    hlayoutCenterFont1->addWidget(resultLogo);
    // 检测中大标题
    MyLabel *bigTitle = new MyLabel(centerFont);
    bigTitle->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_bigTitle");
    bigTitle->setFontSize(24);
    bigTitle->setIsOriginal(true);
    bigTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont1->addWidget(bigTitle);
    vlayoutCenterFont->addLayout(hlayoutCenterFont1);

    // 第二行
    QHBoxLayout *hlayoutCenterFont2 = new QHBoxLayout;
    hlayoutCenterFont2->addSpacing(10);
    // 备份结果错误提示：黑点和文字
    CircleLable *dot1 = new CircleLable(QString(""), centerFont, 6, Qt::black);
    dot1->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_dot1");
    hlayoutCenterFont2->addWidget(dot1);
    hlayoutCenterFont2->addSpacing(5);
    MyLabel *labelError1 = new MyLabel(centerFont);
    labelError1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelError1");
    labelError1->setIsOriginal(true);
    labelError1->setWordWrap(true);
    labelError1->adjustSize();
    labelError1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont2->addWidget(labelError1);
    hlayoutCenterFont2->setAlignment(Qt::AlignLeft);
    vlayoutCenterFont->addLayout(hlayoutCenterFont2);

    // 第三行
    QHBoxLayout *hlayoutCenterFont3 = new QHBoxLayout;
    hlayoutCenterFont3->addSpacing(10);
    // 检测中的记录：黑点2和文字2
    CircleLable *dot2 = new CircleLable(QString(""), centerFont, 6, Qt::black);
    dot2->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_dot2");
    hlayoutCenterFont3->addWidget(dot2);
    hlayoutCenterFont3->addSpacing(5);
    MyLabel *labelError2 = new MyLabel(centerFont);
    labelError2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelError2");
    labelError2->setIsOriginal(true);
    labelError2->setWordWrap(true);
    labelError2->adjustSize();
    labelError2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont3->addWidget(labelError2);
    hlayoutCenterFont3->setAlignment(Qt::AlignLeft);
    vlayoutCenterFont->addLayout(hlayoutCenterFont3);

    // 第四行
    vlayoutCenterFont->addSpacing(56);

    // 第五行
    QHBoxLayout *hlayoutCenterFont5 = new QHBoxLayout;
    hlayoutCenterFont5->setSpacing(16);
    hlayoutCenterFont5->addStretch();
    // 返回首页
    MyPushButton *homePage = new MyPushButton(centerFont);
    homePage->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_homePage");
    homePage->setText(tr("Home Page"));
    homePage->setEnabled(true);
    // homePage->setAutoRepeat(true);
    homePage->setProperty("isImportant", true);
    hlayoutCenterFont5->addWidget(homePage);
    // 再试一次
    QPushButton *retry = new QPushButton(centerFont);
    retry->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_retry");
    retry->setText(tr("Retry"));
    retry->setEnabled(true);
    // retry->setAutoRepeat(true);
    hlayoutCenterFont5->addWidget(retry);
    hlayoutCenterFont5->addStretch();
    hlayoutCenterFont5->setAlignment(Qt::AlignCenter);
    vlayoutCenterFont->addLayout(hlayoutCenterFont5);

    centerFont->setLayout(vlayoutCenterFont);
    hlayout->addWidget(centerFont);
    hlayout->addSpacing(20);
    hlayout->addStretch();
    hlayout->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayout);

    vlayout->addStretch();
    last->setLayout(vlayout);
    //------------ 中部布局end-------------

    // 备份检测结果
    connect(this, &SystemBackup::checkBackupResult, this, [=](bool result, const QString &errMsg, const QString &errTip) {
        if (result) {
            resultLogo->setThemeIconSchema("ukui-dialog-success", ":/symbos/ukui-dialog-success");
            resultLogo->setVisible(true);
            // 备份成功
            bigTitle->setDeplayText(tr("The backup is successful"));
            hlayoutCenterFont1->setAlignment(Qt::AlignCenter);

            dot1->setVisible(false);
            dot2->setVisible(false);
            labelError1->setVisible(false);
            labelError2->setVisible(false);
            retry->setVisible(false);

            homePage->setVisible(true);
        } else {
            resultLogo->setThemeIconSchema("dialog-error", ":/symbos/dialog-error");
            resultLogo->setVisible(true);
            // 备份失败
            bigTitle->setDeplayText(tr("The backup is failed"));
            bigTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            hlayoutCenterFont1->setAlignment(Qt::AlignLeft);
            dot1->setVisible(true);
            dot2->setVisible(true);
            labelError1->setVisible(true);
            labelError2->setVisible(true);
            labelError1->setDeplayText(errMsg);
            labelError2->setDeplayText(errTip);
            retry->setVisible(true);

            homePage->setVisible(true);
        }

        emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();
        // 结束后声音提示
        kdk::KSoundEffects::playSound(kdk::SoundType::COMPLETE);
    });

    // 调整中部控件大小
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::widthChanged, this, [=](){
        if (!isVisible()) {
            return;
        }

        labelError1->setMaximumWidth(DEFAULT_MAX_SIZE);
        labelError2->setMaximumWidth(DEFAULT_MAX_SIZE);
        // 控件bigTitle的宽度
        int len_title = 0;
        if (bigTitle->isVisible()) {
            QFontMetrics fontMetrics(bigTitle->font());
            len_title = fontMetrics.width(bigTitle->getOriginalText());
            len_title += LEN_TITLE_PRE;
        }
        // 控件labelError1的宽度
        int len_labelError1 = 0;
        if (labelError1->isVisible()) {
            QFontMetrics fontMetrics(labelError1->font());
            len_labelError1 = fontMetrics.width(labelError1->getOriginalText());
            len_labelError1 += LEN_DOT_PRE;
        }
        // 控件labelError2的宽度
        int len_labelError2 = 0;
        if (labelError2->isVisible()) {
            QFontMetrics fontMetrics(labelError2->font());
            len_labelError2 = fontMetrics.width(labelError2->getOriginalText());
            len_labelError2 += LEN_DOT_PRE;
        }
        int len_max = len_title > len_labelError1 ? len_title : len_labelError1;
        len_max = len_max > len_labelError2 ? len_max : len_labelError2;
        if (len_max >= this->width() - 20*2)
            len_max = this->width() - 20*2;

        centerFont->setFixedWidth(len_max);
        if (labelError1->isVisible() && len_labelError1 > len_max) {
            labelError1->setMaximumWidth(len_max - LEN_DOT_PRE);
            labelError1->setFontWordWrap(true);
            labelError1->adjustSize();
        }
        if (labelError2->isVisible() && len_labelError2 > len_max) {
            labelError2->setMaximumWidth(len_max - LEN_DOT_PRE);
            labelError2->setFontWordWrap(true);
            labelError2->adjustSize();
        }
    });

    // 再试一次
    connect(retry, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        retry->clearFocus();
        this->setCurrentIndex(SystemBackupPage::NAME_BACKUP_PAGE);
    });

    // 返回首页
    connect(homePage, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        this->setCurrentIndex(SystemBackupPage::HOME_PAGE);
    });

    addWidget(last);
}

