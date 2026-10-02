/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "ghostimage.h"
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMovie>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHostInfo>
#include <unistd.h>
#include <ksoundeffects.h>

#include "../component/clicklabel.h"
#include "../component/circlelabel.h"
#include "../component/myiconlabel.h"
#include "../component/mylabel.h"
#include "../component/mylineedit.h"
#include "../component/mypushbutton.h"
#include "../component/linelabel.h"
#include "../component/ringsprogressbar.h"
#include "../component/myfileselect.h"
#include "../component/pixmaplabel.h"
#include "../../common/utils.h"
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"
#include "selectrestorepoint.h"
#include "messageboxutils.h"
#include "qpushbutton.h"
#include "kprogresscircle.h"

using namespace kdk;
// bigTitle控件前面的宽度：24+spacing
#define LEN_TITLE_PRE 50
// labelCheck1、labelCheck2控件前面的宽度：10+6+5
#define LEN_DOT_PRE 50
#define PICTURE_SIZE 256

IMPLEMENT_DYNCREATE_WITH_ONE_PARAM(GhostImage, QStackedWidget*, QWidget*)

GhostImage::GhostImage(QWidget *parent) :
    QStackedWidget(parent),
    m_udector(new UdiskDetector()),
    m_isLocal(true),
    m_ghostImageState(GhostImageState::IDEL),
    m_pInterface(nullptr)
{
    m_iPosition = BackupPosition::LOCAL;

    // 界面手写代码创建，作为练手
    initHomePage();
    initSetSavePathPage();
    initCheckPage();
    initGhostingPage();
    initLastPage();
}

GhostImage::~GhostImage()
{
    delete m_udector;
    m_udector = nullptr;

    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 初始化第一个页面
 */
void GhostImage::initHomePage()
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
    vLayout->addStretch();
    vLayout->addSpacing(60);
    homePage->setLayout(vLayout);
    // 图片
    PixmapLabel *imageGhost = new PixmapLabel(center);
    imageGhost->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_imageGhost");
    imageGhost->setGeometry(center->width() - 40 - PICTURE_SIZE, 160, PICTURE_SIZE, PICTURE_SIZE);
    imageGhost->setScaledContents(true);
    imageGhost->setLightAndDarkPixmap(":/images/ghost_image.png", ":/images/ghost_image_dark.png");
    imageGhost->setAttribute(Qt::WA_DeleteOnClose,true);

    // Ghost Image大字提示
    MyLabel *labelGhost = new MyLabel(center);
    labelGhost->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelGhost");
    labelGhost->setDeplayText(tr("Ghost Image"));
    labelGhost->setFixedWidth(center->width() - 200);
    labelGhost->setFixedHeight(48);
    labelGhost->move(41, 120);
    // 默认水平左对齐，上下居中对齐；故不需要设置
    // labelGhost->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // labelGhost->setText(tr("System Backup"));
    QFont font;
    font.setBold(true);
    font.setPixelSize(36);
    labelGhost->setFont(font);
    // labelGhost->setAttribute(Qt::WA_TranslucentBackground);
    labelGhost->setScaledContents(true);
    labelGhost->adjustSize();
    labelGhost->setAttribute(Qt::WA_DeleteOnClose,true);

    // Ghost说明
    MyLabel *labelNote = new MyLabel(center);
    labelNote->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelNote");
    labelNote->setFixedWidth(center->width() - 45);
    labelNote->setFixedHeight(24);
    labelNote->move(41, 180);
    // 必须存在本地系统备份，才能制作镜像文件
    labelNote->setDeplayText(tr("A ghost image file can only be created after backup system to local disk"));
    font.setBold(false);
    font.setPixelSize(18);
    labelNote->setFont(font);
    labelNote->setScaledContents(true);
    labelNote->adjustSize();
    labelNote->setAttribute(Qt::WA_DeleteOnClose,true);

    // 操作简单
    MyIconLabel *iconSimple = new MyIconLabel(center);
    iconSimple->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSimple");
    iconSimple->setGeometry(41, 244, 180, 36);
    iconSimple->setThemeIcon("ukui-bf-simple-symbolic", ":/symbos/ukui-bf-simple-symbolic");
    iconSimple->setDesplayText(tr("Simple"));
    iconSimple->setEnabled(false);
    iconSimple->setAttribute(Qt::WA_DeleteOnClose,true);

    // 快速
    MyIconLabel *iconFast = new MyIconLabel(center);
    iconFast->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconFast");
    iconFast->setGeometry(206, 244, 180, 36);
    iconFast->setThemeIcon("ukui-bf-fast-symbolic", ":/symbos/ukui-bf-fast-symbolic");
    iconFast->setDesplayText(tr("Fast"));
    iconFast->setEnabled(false);
    iconFast->setAttribute(Qt::WA_DeleteOnClose,true);

    // 安全
    MyIconLabel *iconSecurity = new MyIconLabel(center);
    iconSecurity->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSecurity");
    iconSecurity->setGeometry(41, 296, 180, 36);
    iconSecurity->setThemeIcon("ukui-bf-security-symbolic", ":/symbos/ukui-bf-security-symbolic");
    iconSecurity->setDesplayText(tr("Security"));
    iconSecurity->setEnabled(false);
    iconSecurity->setAttribute(Qt::WA_DeleteOnClose,true);

    // 节省时间
    MyIconLabel *iconSimple1 = new MyIconLabel(center);
    iconSimple1->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSimple1");
    iconSimple1->setGeometry(206, 296, 180, 36);
    iconSimple1->setThemeIcon("document-open-recent-symbolic", ":/symbos/document-open-recent-symbolic");
    iconSimple1->setDesplayText(tr("Timesaving"));
    iconSimple1->setEnabled(false);
    iconSimple1->setAttribute(Qt::WA_DeleteOnClose,true);

    // 制作镜像按钮
    MyPushButton *beginBackup = new MyPushButton(center);
    beginBackup->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_beginBackup");
    beginBackup->setFixedHeight(52);
    beginBackup->move(41, 372);
    beginBackup->setText(tr("Start Ghost"));
    beginBackup->setEnabled(true);
    beginBackup->setProperty("isImportant", true);
    font.setPixelSize(24);
    beginBackup->setFont(font);
    beginBackup->setAttribute(Qt::WA_DeleteOnClose,true);
    if (beginBackup->text() == "Start Ghost" || beginBackup->text() == "创建镜像")
        beginBackup->setFixedWidth(180);
    else {
        QFontMetrics fontMetrics(font);
        int fontSize = fontMetrics.width(beginBackup->text());
        if (180 > fontSize * 1.2)
            beginBackup->setFixedWidth(180);
        else
            beginBackup->setFixedWidth(fontSize * 1.2);
    }
    connect(beginBackup, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        this->m_uuid = "";
        this->m_prefixDestPath = "";
        this->m_backupName = "";

        SelectRestorePoint * selectDialog = new SelectRestorePoint(this, SelectRestorePoint::SYSTEM, true,true);
        selectDialog->setObjectName("yhkylin-backup-tools_kybackup_SelectRestorePoint_selectDialog");
        selectDialog->setNeedConfirm(false);
        connect(selectDialog, &SelectRestorePoint::selected, this, [=](ParseBackupList::BackupPoint backupPoint){
            this->m_uuid = backupPoint.m_uuid;
            this->m_backupName = backupPoint.m_backupName;
            this->m_iPosition = backupPoint.m_iPosition;
        });

        if (QDialog::Accepted == selectDialog->exec()) {
            this->m_isLocal = true;
            this->setCurrentIndex(SELECT_PATH_PAGE);
        }
        selectDialog->deleteLater();

    });

    addWidget(homePage);
}

/**
 * @brief “上一步”按钮响应槽
 * @param checked
 */
void GhostImage::on_pre_clicked(bool checked)
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
void GhostImage::on_next_clicked(bool checked)
{
    Q_UNUSED(checked)
    int index = this->currentIndex() + 1;
    if (index < this->count()) {
        this->setCurrentIndex(index);
    }
}

/**
 * @brief 初始化选择镜像保存位置页面
 */
void GhostImage::initSetSavePathPage()
{
    QWidget *savePathPage = new QWidget;
    savePathPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_savePathPage");
    // 纵向布局
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);

    // 第一行
    QHBoxLayout * hlayoutLine1 = new QHBoxLayout;
    hlayoutLine1->addSpacing(40);
    // 请选择存储位置
    MyLabel* labelSelect = new MyLabel(savePathPage);
    labelSelect->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelSelect");
    labelSelect->setDeplayText(tr("Please select storage location"));
    labelSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QFont font;
    font.setBold(true);
    font.setPixelSize(18);
    labelSelect->setFont(font);
    labelSelect->setScaledContents(true);
    labelSelect->adjustSize();
    hlayoutLine1->addWidget(labelSelect);
    hlayoutLine1->setAlignment(Qt::AlignLeft);
    vlayout->addLayout(hlayoutLine1);

    // 第二行
    vlayout->addSpacing(5);
    QHBoxLayout * hlayoutLine2 = new QHBoxLayout;
    hlayoutLine2->addSpacing(40);
    // 备份路径选择框
    QComboBox* comboSelect = new QComboBox(savePathPage);
    comboSelect->setObjectName("yhkylin-backup-tools_kybackup_QComboBox_comboSelect");
    // 添加本地默认路径、移动设备目录
    connect(m_udector, &UdiskDetector::udiskListChanged, this, [=](QList<QStorageInfo> diskList) {
        comboSelect->clear();
        this->m_udiskPaths.clear();
        QIcon iconFolder = QIcon::fromTheme("insync-folder", QIcon(":/images/insync-folder.png"));

        QString qsLocalDefaultPath = Utils::getSysRootPath() + GHOST_PATH;
        qsLocalDefaultPath.replace("//", "/");
        this->m_udiskPaths << "";
        comboSelect->addItem(iconFolder, tr("local default path : ") + qsLocalDefaultPath);

        QString qsPreDevPath(tr("removable devices path : "));
        for (QStorageInfo& disk : diskList) {
            this->m_udiskPaths << disk.rootPath();
            comboSelect->addItem(iconFolder, qsPreDevPath + disk.rootPath() + GHOST_PATH);
        }
    });
    m_udector->getStorageInfo();
    hlayoutLine2->addWidget(comboSelect);
    hlayoutLine2->addSpacing(40);
    vlayout->addLayout(hlayoutLine2);
    vlayout->addSpacing(56);

    // 第三行
    QHBoxLayout * hlayoutLine3 = new QHBoxLayout;
    hlayoutLine3->setSpacing(16);
    hlayoutLine3->addStretch();
    // 上一步按钮
    QPushButton *preStep = new QPushButton(savePathPage);
    preStep->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_preStep");
    preStep->setText(tr("Back"));
    preStep->setEnabled(true);
    // preStep->setAutoRepeat(true);
    connect(preStep, &QPushButton::clicked, preStep, &QPushButton::clearFocus);
    connect(preStep, &QPushButton::clicked, this, &GhostImage::on_pre_clicked);
    hlayoutLine3->addWidget(preStep);

    // 下一步按钮
    MyPushButton *nextStep = new MyPushButton(savePathPage);
    nextStep->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_nextStep");
    nextStep->setText(tr("Next"));
    nextStep->setEnabled(true);
    // nextStep->setAutoRepeat(true);
    nextStep->setProperty("isImportant", true);
    connect(nextStep, &MyPushButton::clicked, this, [=](bool checked) {
        // 备份路径选择索引
        int index = comboSelect->currentIndex();
        // 第一个选项是本地系统备份
        this->m_isLocal = GlobelBackupInfo::instance().hasBackupPartition() && (index == 0);
        this->m_prefixDestPath = this->m_udiskPaths.at(index);
        this->on_next_clicked(checked);
        emit this->startCheckEnv();
    });

    hlayoutLine3->addWidget(nextStep);
    hlayoutLine3->addStretch();
    hlayoutLine3->setAlignment(Qt::AlignCenter);
    vlayout->addLayout(hlayoutLine3);

    vlayout->addStretch();
    savePathPage->setLayout(vlayout);

    addWidget(savePathPage);
}

/**
 * @brief 初始化检测页面
 */
void GhostImage::initCheckPage()
{
    QWidget *checkPage = new QWidget;
    checkPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_checkPage");

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", checkPage, 24, QColor(COLOR_BLUE));
    LineLabel *line1 = new LineLabel(checkPage, QColor(COLOR_BLUE));
    CircleLable *two = new CircleLable("2", checkPage);
    LineLabel *line2 = new LineLabel(checkPage, QColor(COLOR_GRAY));
    CircleLable *three = new CircleLable("3", checkPage);
    one->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_one");
    line1->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line1");
    two->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_two");
    line2->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line2");
    three->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_three");

    QHBoxLayout *layoutLine1 = new QHBoxLayout;
    layoutLine1->addSpacing(128);
    layoutLine1->addWidget(one);
    layoutLine1->addWidget(line1);
    layoutLine1->addWidget(two);
    layoutLine1->addWidget(line2);
    layoutLine1->addWidget(three);
    layoutLine1->addSpacing(128);

    MyLabel *label1 = new MyLabel(tr("checking"), checkPage);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setIsOriginal(true);
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("ghosting"), checkPage);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setIsOriginal(true);
    label2->setEnabled(false);
    label2->setFixedWidth(164);
    MyLabel *label3 = new MyLabel(tr("finished"), checkPage);
    label3->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label3");
    label3->setIsOriginal(true);
    label3->setEnabled(false);
    label3->setFixedWidth(164);
    QHBoxLayout *layoutLine2 = new QHBoxLayout;
    layoutLine2->addSpacing(54);
    layoutLine2->addWidget(label1);
    layoutLine2->addStretch();
    layoutLine2->addWidget(label2);
    layoutLine2->addStretch();
    layoutLine2->addWidget(label3);
    layoutLine2->addSpacing(54);

    // ------------ 中部布局begin-------------
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
    vlayoutCenterFont->addSpacing(16);

    // 第二行
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
    connect(preStep, &QPushButton::clicked, this, &GhostImage::on_pre_clicked);
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
        emit this->startGhost();
    });
    hlayoutCenterFont5->addWidget(nextStep);
    // 重新检测按钮
    MyPushButton *recheck = new MyPushButton(centerFont);
    recheck->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_recheck");
    recheck->setText(tr("Recheck"));
    recheck->setEnabled(true);
    // recheck->setAutoRepeat(true);
    recheck->setProperty("isImportant", true);
    hlayoutCenterFont5->addWidget(recheck);
    hlayoutCenterFont5->addStretch();
    hlayoutCenterFont5->setAlignment(Qt::AlignCenter);
    vlayoutCenterFont->addLayout(hlayoutCenterFont5);

    centerFont->setLayout(vlayoutCenterFont);
    QHBoxLayout *layoutLine3 = new QHBoxLayout;
    layoutLine3->addStretch();
    layoutLine3->addSpacing(20);
    layoutLine3->addWidget(centerFont);
    layoutLine3->addSpacing(20);
    layoutLine3->addStretch();
    layoutLine3->setAlignment(Qt::AlignCenter);

    // ------------ 中部布局end-------------

    // 布局
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);
    vlayout->addLayout(layoutLine1);
    vlayout->addLayout(layoutLine2);
    vlayout->addSpacing(80);
    vlayout->addLayout(layoutLine3);
    vlayout->addStretch();
    checkPage->setLayout(vlayout);

    // 开始检测
    connect(this, &GhostImage::startCheckEnv, this, [=]() {
        this->m_ghostImageState = GhostImageState::CHECKING;
        loadingGif->setVisible(true);
        movie->start();
        resultLogo->setVisible(false);
        // 环境检测中，请等待
        bigTitle->setDeplayText(tr("Checking, wait a moment ..."));
        dot1->setBackgroundColor(Qt::black);
        dot2->setBackgroundColor(Qt::black);
        labelCheck1->setFontColor(Qt::black);
        labelCheck2->setFontColor(Qt::black);
        // 检测是否具备制作Ghost镜像条件
        labelCheck1->setDeplayText(tr("Check whether the conditions for creating an ghost image are met"));
        // 制作Ghost镜像期间不要做其它操作，以防数据丢失
        labelCheck2->setDeplayText(tr("Do not perform other operations to avoid data loss"));
        preStep->setVisible(false);
        nextStep->setVisible(false);
        recheck->setVisible(false);

        this->on_checkEnv_start();
        emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();
    });

    // 检测结果
    connect(this, &GhostImage::checkEnvResult, this, [=](bool result, const QString &errMsg, const QString &errTip) {
        loadingGif->setVisible(false);
        movie->stop();

        if (result) {
            resultLogo->setThemeIconSchema("ukui-dialog-success", ":/symbos/ukui-dialog-success");
            resultLogo->setVisible(true);
            // 检测成功
            bigTitle->setDeplayText(tr("Check success"));
            // 存储空间充足
            labelCheck1->setDeplayText(tr("The storage space is enough"));
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
        labelCheck1->setMaximumWidth(DEFAULT_MAX_SIZE);
        labelCheck2->setMaximumWidth(DEFAULT_MAX_SIZE);
        // 控件bigTitle的宽度
        int len_title = 0;
        if (bigTitle->isVisible()) {
            QFontMetrics fontMetrics(bigTitle->font());
            len_title = fontMetrics.width(bigTitle->getOriginalText());
            len_title += LEN_TITLE_PRE;
        }
        // 控件labelError1的宽度
        int len_labelError1 = 0;
        if (labelCheck1->isVisible()) {
            QFontMetrics fontMetrics(labelCheck1->font());
            len_labelError1 = fontMetrics.width(labelCheck1->getOriginalText());
            len_labelError1 += LEN_DOT_PRE;
        }
        // 控件labelError2的宽度
        int len_labelError2 = 0;
        if (labelCheck2->isVisible()) {
            QFontMetrics fontMetrics(labelCheck2->font());
            len_labelError2 = fontMetrics.width(labelCheck2->getOriginalText());
            len_labelError2 += LEN_DOT_PRE;
        }
        int len_max = len_title > len_labelError1 ? len_title : len_labelError1;
        len_max = len_max > len_labelError2 ? len_max : len_labelError2;
        if (len_max >= this->width() - 20*2)
            len_max = this->width() - 20*2;

        centerFont->setFixedWidth(len_max);
        if (labelCheck1->isVisible() && len_labelError1 > len_max) {
            labelCheck1->setMaximumWidth(len_max - LEN_DOT_PRE);
            labelCheck1->setFontWordWrap(true);
            labelCheck1->adjustSize();
        }
        if (labelCheck2->isVisible() && len_labelError2 > len_max) {
            labelCheck2->setMaximumWidth(len_max - LEN_DOT_PRE);
            labelCheck2->setFontWordWrap(true);
            labelCheck2->adjustSize();
        }
    });

    // 重新检查
    connect(recheck, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        emit this->startCheckEnv();
    });

    addWidget(checkPage);
}

/**
 * @brief 开始进行环境检测
 */
void GhostImage::on_checkEnv_start()
{
    GlobelBackupInfo::instance().setIsBusy(true);
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.backup", "/", QDBusConnection::systemBus(), this);
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &GhostImage::on_checkEnv_end);

    // 是否已存在备份、还原等操作
    bool isActive = false;
    if(int(BackupState::BACKUP_STATE_INIT) != m_pInterface->getBackupState(isActive)){
        on_checkEnv_end(int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING));

        return;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_uuid = m_uuid;
    backupWrapper.m_type = BackupType::GHOST_IMAGE;
    backupWrapper.m_backupName = createGhostImageName(m_backupName);
    QString prePath = Utils::getSysRootPath() + "/media";
    prePath = prePath.replace("//", "/");
    if (m_iPosition != BackupPosition::CUSTOMIZE && m_prefixDestPath.startsWith(prePath)) {
        backupWrapper.m_iPosition = BackupPosition::UDISK;
    } else {
        backupWrapper.m_iPosition = m_iPosition;
    }
    backupWrapper.m_prefixDestPath = m_prefixDestPath;
    backupWrapper.m_frontUid = getuid();
    backupWrapper.m_frontUserName = qgetenv("USER");
    backupWrapper.m_gid = getgid();
    m_pInterface->checkEnv(backupWrapper);
}

/**
 * @brief 环境检测结束
 * @param result, 环境校验结果
 */
void GhostImage::on_checkEnv_end(int result)
{
    m_ghostImageState = GhostImageState::IDEL;
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
        // 镜像空间不足
        errMsg = tr("The storage for ghost is not enough");
        // 建议释放空间后重试
        errTip = tr("Retry after release space");
        break;
    case int(BackupResult::BACKUP_CAPACITY_FOR_UDISKIMG_IS_NOT_ENOUGH):
        // 没有足够的空间存放临时.kyimg文件
        errMsg = tr("There is not enough space for temporary .kyimg file");
        // 建议释放空间后重试
        errTip = tr("Retry after release space");
        break;
    case int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING):
        // 其它备份还原等操作正在执行
        errMsg = tr("Other backup or restore task is being performed");
        // 请稍后重试
        errTip = tr("Please try again later");
        break;
    case int(BackupResult::GHOST_CANNOT_FIND_BACKUPPOINT):
    case int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST):
        // 备份节点已不存在
        errMsg = tr("The backup node does not exist");
        // 请检查刚刚是否有删除备份点操作
        errTip = tr("Check whether the backup point has been deleted");
        break;
    default:
        bRst = true;
        break;
    }

    Utils::wait(2);
    emit checkEnvResult(bRst, errMsg, errTip);
    GlobelBackupInfo::instance().setIsBusy(false);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &GhostImage::on_checkEnv_end);
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 创建ghost镜像名称
 * @param comment
 * @return
 */
QString GhostImage::createGhostImageName(const QString& backupName)
{
    QString localHostName = QHostInfo::localHostName();

    QRegExp rx = QRegExp("([0-9a-zA-Z_\\-])+");
    QString destFile = "ghost";
    if (rx.exactMatch(localHostName))
        destFile = localHostName;

    //使用uname -m获取体系结构
    QString result;
    Utils::executeCMD("uname -m", result);
    QString arch = result.trimmed();

    //用comment
    QString mycomment = backupName;
    mycomment.replace("-", "");
    mycomment.replace(":", "");
    mycomment.replace(" ", "");
    destFile = destFile + "-" + arch + "-" + mycomment + ".kyimg";

    return destFile;
}

/**
 * @brief 初始化镜像创建页面
 */
void GhostImage::initGhostingPage()
{
    QWidget *ghostingPage = new QWidget;
    ghostingPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_ghostingPage");

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", ghostingPage, 24, QColor(COLOR_BLUE));
    LineLabel *line1 = new LineLabel(ghostingPage, QColor(COLOR_BLUE));
    CircleLable *two = new CircleLable("2", ghostingPage, 24, QColor(COLOR_BLUE));
    LineLabel *line2 = new LineLabel(ghostingPage, QColor(COLOR_BLUE));
    CircleLable *three = new CircleLable("3", ghostingPage);
    one->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_one");
    line1->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line1");
    two->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_two");
    line2->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line2");
    three->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_three");

    QHBoxLayout *layoutLine1 = new QHBoxLayout;
    layoutLine1->addSpacing(128);
    layoutLine1->addWidget(one);
    layoutLine1->addWidget(line1);
    layoutLine1->addWidget(two);
    layoutLine1->addWidget(line2);
    layoutLine1->addWidget(three);
    layoutLine1->addSpacing(128);

    MyLabel *label1 = new MyLabel(tr("checking"), ghostingPage);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setIsOriginal(true);
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("ghosting"), ghostingPage);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setIsOriginal(true);
    label2->setFontColor(QColor(COLOR_BLUE));
    label2->setFixedWidth(164);
    MyLabel *label3 = new MyLabel(tr("finished"), ghostingPage);
    label3->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label3");
    label3->setIsOriginal(true);
    label3->setEnabled(false);
    label3->setFixedWidth(164);
    QHBoxLayout *layoutLine2 = new QHBoxLayout;
    layoutLine2->addSpacing(54);
    layoutLine2->addWidget(label1);
    layoutLine2->addStretch();
    layoutLine2->addWidget(label2);
    layoutLine2->addStretch();
    layoutLine2->addWidget(label3);
    layoutLine2->addSpacing(54);

    // ------------ 中部布局begin-------------
    // 中部第一行
    QHBoxLayout *hlayoutCenterFont1 = new QHBoxLayout;
    // 检测等待图标
    QLabel *loadingGif = new QLabel(ghostingPage);
    loadingGif->setObjectName("yhkylin-backup-tools_kybackup_QLabel_loadingGif");
    // 环境检测等待动画
    QMovie *movie = new QMovie(":/images/loading.gif", QByteArray(), ghostingPage);
    movie->setObjectName("yhkylin-backup-tools_kybackup_QMovie_movie");
    loadingGif->setMovie(movie);
    loadingGif->setFixedSize(24,24);
    loadingGif->setVisible(false);
    // 进度条
   // RingsProgressbar *progressBar = new RingsProgressbar(ghostingPage);
    KProgressCircle *progressBar = new KProgressCircle(ghostingPage);
    progressBar->setObjectName("yhkylin-backup-tools_kybackup_KProgressCircle_progressBar");
    progressBar->setFixedSize(100, 100);
    hlayoutCenterFont1->addStretch();
    hlayoutCenterFont1->addWidget(loadingGif);
    hlayoutCenterFont1->addWidget(progressBar);
    hlayoutCenterFont1->addStretch();
    hlayoutCenterFont1->setAlignment(Qt::AlignCenter);

    // 第二行
    QHBoxLayout *hlayoutCenterFont2 = new QHBoxLayout;
    // 提醒
    MyLabel *labelTip = new MyLabel(ghostingPage);
    labelTip->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelTip");
    labelTip->setAlignment(Qt::AlignCenter);
    labelTip->setIsOriginal(true);
    labelTip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hlayoutCenterFont2->addStretch();
    hlayoutCenterFont2->addWidget(labelTip);
    hlayoutCenterFont2->addStretch();
    hlayoutCenterFont2->setAlignment(Qt::AlignCenter);
    // 第二行
    QHBoxLayout *hlayoutCenterFont2_1 = new QHBoxLayout;
    // 备份过程提醒信息
    MyLabel *labelTip_1 = new MyLabel(ghostingPage);
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

    // 第三行
    QHBoxLayout *hlayoutCenterFont3 = new QHBoxLayout;
    // 取消按钮
    MyPushButton *cancel = new MyPushButton(ghostingPage);
    cancel->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_cancel");
    cancel->setText(tr("Cancel"));
    cancel->setEnabled(true);
    // cancel->setAutoRepeat(true);
    cancel->setProperty("isImportant", true);
    hlayoutCenterFont3->addStretch();
    hlayoutCenterFont3->addWidget(cancel);
    hlayoutCenterFont3->addStretch();
    hlayoutCenterFont3->setAlignment(Qt::AlignCenter);

    // ------------ 中部布局end-------------

    // 布局
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);
    vlayout->addLayout(layoutLine1);
    vlayout->addLayout(layoutLine2);
    vlayout->addSpacing(80);
    vlayout->addLayout(hlayoutCenterFont1);
    vlayout->addSpacing(16);
    vlayout->addLayout(hlayoutCenterFont2);
    vlayout->addLayout(hlayoutCenterFont2_1);
    vlayout->addSpacing(56);
    vlayout->addLayout(hlayoutCenterFont3);
    vlayout->addStretch();
    ghostingPage->setLayout(vlayout);

    // 开始制作ghost镜像
    connect(this, &GhostImage::startGhost, this, [=] {
      //  progressBar->setPersent(0);
        progressBar->setValue(0);
        movie->start();
        // 不要使用电脑，以防数据丢失
        labelTip->setDeplayText(tr("Do not use computer in case of data loss"));
        labelTip_1->setVisible(false);
        labelTip_1->setDeplayText("");
        cancel->setEnabled(true);

        // 开始备份
        this->on_ghost_start();
    });

    connect(this, &GhostImage::backupWarnning, labelTip_1, [=](const QString& msg) {
        labelTip_1->setVisible(true);
        labelTip_1->setDeplayText(msg);
    });

    // 进度
    connect(this, &GhostImage::progress, this, [=](int state, int rate) {
        Q_UNUSED(state)
       // progressBar->setPersent(rate);
        progressBar->setValue(rate);
    });

    // 取消备份
    connect(cancel, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        // 确定取消当前操作？
        if (!MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(this, QObject::tr("Information"), QObject::tr("Are you sure to cancel the operation？"), QObject::tr("OK"), QObject::tr("Cancel"))) {
            return ;
        }
        cancel->setEnabled(false);
        if (m_pInterface)
            m_pInterface->cancel();
    });

    addWidget(ghostingPage);
}

/**
 * @brief 开始进行系统备份
 */
void GhostImage::on_ghost_start()
{
    GlobelBackupInfo::instance().setIsBusy(true);
    m_ghostImageState = GhostImageState::GHOSTING;
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.backup", "/", QDBusConnection::systemBus(), this);
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &GhostImage::on_checkGhost_end);
    connect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &GhostImage::progress);
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendGhostBackupResult, this, &GhostImage::on_ghost_end);

    // 是否已存在备份、还原等操作
    bool isActive = false;
    if(int(BackupState::BACKUP_STATE_INIT) != m_pInterface->getBackupState(isActive)){
        on_checkGhost_end(int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING));

        return;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_uuid = m_uuid;
    backupWrapper.m_type = BackupType::GHOST_IMAGE;
    backupWrapper.m_backupName = createGhostImageName(m_backupName);
    QString prePath = Utils::getSysRootPath() + "/media";
    prePath = prePath.replace("//", "/");
    if (m_iPosition != BackupPosition::CUSTOMIZE && m_prefixDestPath.startsWith(prePath)) {
        backupWrapper.m_iPosition = BackupPosition::UDISK;
    } else {
        backupWrapper.m_iPosition = m_iPosition;
    }
    backupWrapper.m_prefixDestPath = m_prefixDestPath;
    backupWrapper.m_frontUid = getuid();
    backupWrapper.m_frontUserName = qgetenv("USER");
    backupWrapper.m_gid = getgid();
    m_pInterface->ghostBackup(backupWrapper);
}

/**
 * @brief 系统备份校验结果处理
 * @param result
 */
void GhostImage::on_checkGhost_end(int result)
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
        // 镜像空间不足
        errMsg = tr("The storage for ghost is not enough");
        // 建议释放空间后重试
        errTip = tr("Retry after release space");
        break;
    case int(BackupResult::BACKUP_CAPACITY_FOR_UDISKIMG_IS_NOT_ENOUGH):
        // 没有足够的空间存放临时.kyimg文件
        errMsg = tr("There is not enough space for temporary .kyimg file");
        // 建议释放空间后重试
        errTip = tr("Retry after release space");
        break;
    case int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING):
        // 其它备份还原等操作正在执行
        errMsg = tr("Other backup or restore task is being performed");
        // 请稍后重试
        errTip = tr("Please try again later");
        break;
    case int(BackupResult::GHOST_CANNOT_FIND_BACKUPPOINT):
    case int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST):
        // 备份节点已不存在
        errMsg = tr("The backup node does not exist");
        // 请检查刚刚是否有删除备份点操作
        errTip = tr("Check whether the backup point has been deleted");
        break;
    case int(BackupResult::GHOST_START_SUCCESS):
        // 正压缩数据到本地磁盘，请耐心等待...
        errTip = tr("The data is being compressed to the local disk, please wait patiently...");
        emit this->backupWarnning(errTip);
        return;
    case int(BackupResult::MKSQUASHFS_DO_SUCCESS):
        // 正在传输image文件到移动设备，即将完成...
        errTip = tr("Transferring image file to mobile device, about to be completed...");
        emit this->backupWarnning(errTip);
        return;
    case int(BackupResult::CANCEL_SUCCESS):
        // 已经取消本次镜像制作
        errMsg = tr("The image creation had been canceled");
        // 如需要可重新发起镜像制作
        errTip = tr("Re-initiate the image creation if necessary");
        break;
    default:
        bRst = true;
        break;
    }

    if (!bRst) {
        GlobelBackupInfo::instance().setIsBusy(false);
        m_ghostImageState = GhostImageState::IDEL;
        this->on_next_clicked(true);
        emit this->checkGhostResult(bRst, errMsg, errTip);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &GhostImage::on_checkEnv_end);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &GhostImage::progress);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendGhostBackupResult, this, &GhostImage::on_ghost_end);
        delete m_pInterface;
        m_pInterface = nullptr;
    }
}

/**
 * @brief 系统备份结束
 * @param result-false 失败; true 成功
 */
void GhostImage::on_ghost_end(bool result)
{
    m_ghostImageState = GhostImageState::IDEL;

    this->on_next_clicked(true);
    if (result) {
        emit checkGhostResult(result);
    } else {
        // 备份过程中出现错误
        QString errMsg = tr("An error occurred during make ghost image");
        // 错误信息参考日志文件：/var/log/backup.log
        QString errTip = tr("Error messages refer to log file : /var/log/backup.log");
        emit checkGhostResult(result, errMsg, errTip);
    }

    GlobelBackupInfo::instance().setIsBusy(false);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &GhostImage::on_checkEnv_end);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &GhostImage::progress);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendGhostBackupResult, this, &GhostImage::on_ghost_end);
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 最后备份结果界面
 */
void GhostImage::initLastPage()
{
    QWidget *last = new QWidget;
    last->setObjectName("yhkylin-backup-tools_kybackup_QWidget_last");

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", last, 24, QColor(COLOR_BLUE));
    LineLabel *line1 = new LineLabel(last, QColor(COLOR_BLUE));
    CircleLable *two = new CircleLable("2", last, 24, QColor(COLOR_BLUE));
    LineLabel *line2 = new LineLabel(last, QColor(COLOR_BLUE));
    CircleLable *three = new CircleLable("3", last, 24, QColor(COLOR_BLUE));
    one->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_one");
    line1->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line1");
    two->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_two");
    line2->setObjectName("yhkylin-backup-tools_kybackup_LineLabel_line2");
    three->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_three");

    QHBoxLayout *layoutLine1 = new QHBoxLayout;
    layoutLine1->addSpacing(128);
    layoutLine1->addWidget(one);
    layoutLine1->addWidget(line1);
    layoutLine1->addWidget(two);
    layoutLine1->addWidget(line2);
    layoutLine1->addWidget(three);
    layoutLine1->addSpacing(128);

    MyLabel *label1 = new MyLabel(tr("checking"), last);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setIsOriginal(true);
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("ghosting"), last);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setIsOriginal(true);
    label2->setFontColor(QColor(COLOR_BLUE));
    label2->setFixedWidth(164);
    MyLabel *label3 = new MyLabel(tr("finished"), last);
    label3->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label3");
    label3->setIsOriginal(true);
    label3->setFontColor(QColor(COLOR_BLUE));
    label3->setFixedWidth(164);
    QHBoxLayout *layoutLine2 = new QHBoxLayout;
    layoutLine2->addSpacing(54);
    layoutLine2->addWidget(label1);
    layoutLine2->addStretch();
    layoutLine2->addWidget(label2);
    layoutLine2->addStretch();
    layoutLine2->addWidget(label3);
    layoutLine2->addSpacing(54);

    //------------ 中部布局begin-------------
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
    // hlayoutCenterFont1->setAlignment(Qt::AlignLeft);
    vlayoutCenterFont->addLayout(hlayoutCenterFont1);
    vlayoutCenterFont->addSpacing(16);

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
    // hlayoutCenterFont2->setAlignment(Qt::AlignLeft);
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
    QHBoxLayout *layoutLine3 = new QHBoxLayout;
    layoutLine3->addStretch();
    layoutLine3->addSpacing(20);
    layoutLine3->addWidget(centerFont);
    layoutLine3->addSpacing(20);
    layoutLine3->addStretch();
    //------------ 中部布局end-------------

    // 布局
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);
    vlayout->addLayout(layoutLine1);
    vlayout->addLayout(layoutLine2);
    vlayout->addSpacing(80);
    vlayout->addLayout(layoutLine3);
    vlayout->addStretch();
    last->setLayout(vlayout);

    // 检测结果
    connect(this, &GhostImage::checkGhostResult, this, [=](bool result, const QString &errMsg, const QString &errTip) {
        if (result) {
            resultLogo->setThemeIconSchema("ukui-dialog-success", ":/symbos/ukui-dialog-success");
            resultLogo->setVisible(true);
            // 镜像制作成功
            bigTitle->setDeplayText(tr("Ghost image creation is successful"));
            QFontMetrics fontMetrics(bigTitle->font());
            int fontSize = fontMetrics.width(bigTitle->getOriginalText());
            bigTitle->setFixedWidth(fontSize);
            hlayoutCenterFont1->setAlignment(Qt::AlignCenter);

            QString imgPath = m_prefixDestPath + GHOST_PATH;
            imgPath.replace("//", "/");
            labelError1->setDeplayText(tr("You can view it in the directory : %1").arg(imgPath));
            dot1->setVisible(false);
            dot2->setVisible(false);
            labelError1->setVisible(true);
            labelError2->setVisible(false);
            labelError1->setAlignment(Qt::AlignCenter);
            hlayoutCenterFont2->setAlignment(Qt::AlignCenter);

            retry->setVisible(false);
            homePage->setVisible(true);
        } else {
            resultLogo->setThemeIconSchema("dialog-error", ":/symbos/dialog-error");
            resultLogo->setVisible(true);
            // 镜像制作失败
            bigTitle->setDeplayText(tr("Ghost image creation is failed"));
            bigTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            hlayoutCenterFont1->setAlignment(Qt::AlignLeft);

            dot1->setVisible(true);
            dot2->setVisible(true);
            labelError1->setVisible(true);
            labelError2->setVisible(true);
            labelError1->setDeplayText(errMsg);
            labelError2->setDeplayText(errTip);
            labelError1->setAlignment(Qt::AlignLeft);
            hlayoutCenterFont2->setAlignment(Qt::AlignLeft);

            retry->setVisible(true);
            homePage->setVisible(true);
        }
        emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();

        // 结束后声音提示
        kdk::KSoundEffects::playSound(kdk::SoundType::COMPLETE);
    });

    // 调整中部控件大小
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::widthChanged, this, [=](){
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
        this->setCurrentIndex(GhostImagePage::GHOSTING_PAGE);
        emit this->startGhost();
    });

    // 返回首页
    connect(homePage, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        this->setCurrentIndex(GhostImagePage::HOME_PAGE);
    });

    addWidget(last);
}


