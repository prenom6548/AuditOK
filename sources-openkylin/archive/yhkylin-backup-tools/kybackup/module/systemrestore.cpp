/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "systemrestore.h"
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QMovie>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <unistd.h>
#include <ksoundeffects.h>

#include "../component/circlelabel.h"
#include "../component/mycheckbox.h"
#include "../component/myiconlabel.h"
#include "../component/mylabel.h"
#include "../component/mylineedit.h"
#include "../component/mypushbutton.h"
#include "../component/linelabel.h"
#include "../component/ringsprogressbar.h"
#include "../component/pixmaplabel.h"
#include "../../common/utils.h"
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"
#include "messageboxutils.h"
#include "selectrestorepoint.h"
#include "qpushbutton.h"
#include "kprogresscircle.h"
using namespace kdk;
// bigTitle控件前面的宽度：24+spacing
#define LEN_TITLE_PRE 50
// labelCheck1、labelCheck2控件前面的宽度：10+6+5
#define LEN_DOT_PRE 50
#define PICTURE_SIZE 256

#include <libkydatacollect.h>

IMPLEMENT_DYNCREATE_WITH_ONE_PARAM(SystemRestore, QStackedWidget*, QWidget*)

SystemRestore::SystemRestore(QWidget *parent) :
    QStackedWidget(parent)
{
    m_isRetainUserData = false;
    m_isFactoryRestore = false;
    m_pInterface = nullptr;
    m_iPosition = BackupPosition::LOCAL;

    // 界面手写代码创建，作为练手
    initHomePage();
    initCheckPage();
    initRestoringPage();
    initLastPage();
}

SystemRestore::~SystemRestore()
{
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 初始化第一个页面
 */
void SystemRestore::initHomePage()
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
    PixmapLabel *imageRestore = new PixmapLabel(center);
    imageRestore->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_imageRestore");
    imageRestore->setGeometry(center->width() - 40 - PICTURE_SIZE, 145, PICTURE_SIZE, PICTURE_SIZE);
    imageRestore->setScaledContents(true);
    imageRestore->setLightAndDarkPixmap(":/images/system_restore.png", ":/images/system_restore_dark.png");
    imageRestore->setAttribute(Qt::WA_DeleteOnClose,true);

    // 系统还原大字提示
    MyLabel *labelRestore = new MyLabel(center);
    labelRestore->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelRestore");
    labelRestore->setDeplayText(tr("System Restore"));
    labelRestore->setFixedWidth(center->width() - 200);
    labelRestore->setFixedHeight(48);
    labelRestore->move(41, 80);
    QFont font;
    font.setBold(true);
    font.setPixelSize(36);
    labelRestore->setFont(font);
    // labelRestore->setAttribute(Qt::WA_TranslucentBackground);
    labelRestore->setScaledContents(true);
    labelRestore->adjustSize();
    labelRestore->setAttribute(Qt::WA_DeleteOnClose,true);

    // 系统还原说明
    MyLabel *labelNote = new MyLabel(center);
    labelNote->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelNote");
    labelNote->setFixedWidth(center->width() - 45);
    labelNote->setFixedHeight(24);
    labelNote->move(41, 140);
    // 在您遇到问题时可将系统还原到以前的状态
    labelNote->setDeplayText(tr("You can restore the system to its previous state"));
    font.setBold(false);
    font.setPixelSize(18);
    labelNote->setFont(font);
    labelNote->setScaledContents(true);
    labelNote->adjustSize();
    labelNote->setAttribute(Qt::WA_DeleteOnClose,true);

    // 操作简单
    MyIconLabel *iconSimple = new MyIconLabel(center);
    iconSimple->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSimple");
    iconSimple->setGeometry(41, 214, 210, 36);
    iconSimple->setThemeIcon("ukui-bf-simple-symbolic", ":/symbos/ukui-bf-simple-symbolic");
    iconSimple->setDesplayText(tr("Simple"));
    iconSimple->setEnabled(false);
    iconSimple->setAttribute(Qt::WA_DeleteOnClose,true);

    // 安全可靠
    MyIconLabel *iconSecurity = new MyIconLabel(center);
    iconSecurity->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSecurity");
    iconSecurity->setGeometry(231, 214, 180, 36);
    iconSecurity->setThemeIcon("ukui-bf-security-symbolic", ":/symbos/ukui-bf-security-symbolic");
    iconSecurity->setDesplayText(tr("Security"));
    iconSecurity->setEnabled(false);
    iconSecurity->setAttribute(Qt::WA_DeleteOnClose,true);

    // 修复系统损坏
    MyIconLabel *iconMultiBackup = new MyIconLabel(center);
    iconMultiBackup->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconMultiBackup");
    iconMultiBackup->setGeometry(41, 266, 210, 36);
    iconMultiBackup->setThemeIcon("ukui-bf-damage-symbolic", ":/symbos/ukui-bf-damage-symbolic");
    iconMultiBackup->setDesplayText(tr("Repair"));
    iconMultiBackup->setEnabled(false);
    iconMultiBackup->setAttribute(Qt::WA_DeleteOnClose,true);

    // 自主操作
    MyIconLabel *iconSmallSize = new MyIconLabel(center);
    iconSmallSize->setObjectName("yhkylin-backup-tools_kybackup_MyIconLabel_iconSmallSize");
    iconSmallSize->setGeometry(231, 266, 180, 36);
    iconSmallSize->setThemeIcon("ukui-self-help-symbolic", ":/symbos/ukui-self-help-symbolic");
    iconSmallSize->setDesplayText(tr("Independent"));
    iconSmallSize->setEnabled(false);
    iconSmallSize->setAttribute(Qt::WA_DeleteOnClose,true);

    // 开始还原按钮
    MyPushButton *beginRestore = new MyPushButton(center);
    beginRestore->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_beginRestore");
    beginRestore->setGeometry(41, 342, 180, 52);
    beginRestore->setText(tr("Start Restore"));
    beginRestore->setEnabled(true);
    // beginRestore->setAutoRepeat(true);
    beginRestore->setProperty("isImportant", true);
    font.setPixelSize(24);
    beginRestore->setFont(font);
    beginRestore->setAttribute(Qt::WA_DeleteOnClose,true);
    if (beginRestore->text() == "Start Restore" || beginRestore->text() == "开始还原")
        beginRestore->setFixedWidth(180);
    else {
        QFontMetrics fontMetrics(font);
        int fontSize = fontMetrics.width(beginRestore->text());
        if (180 > fontSize * 1.2)
            beginRestore->setFixedWidth(180);
        else
            beginRestore->setFixedWidth(fontSize * 1.2);
    }
    connect(beginRestore, &MyPushButton::clicked, this, &SystemRestore::on_button_beginRestore_clicked);

//    // 底部控件布局
//    QHBoxLayout *bottomHBoxLayout = new QHBoxLayout;
//    bottomHBoxLayout->addStretch();
//    // 恢复出厂复选框
//    MyCheckBox * checkFactoryRestore = new MyCheckBox(tr("Factory Restore"));
//    checkFactoryRestore->setObjectName("yhkylin-backup-tools_kybackup_MyCheckBox_checkFactoryRestore");

//    checkFactoryRestore->setVisible(true);
//    // 保留用户数据复选框
//    MyCheckBox * retainUserData = new MyCheckBox(tr("Retaining User Data"));
//    retainUserData->setObjectName("yhkylin-backup-tools_kybackup_MyCheckBox_retainUserData");
//    bottomHBoxLayout->addWidget(checkFactoryRestore);
//    bottomHBoxLayout->addSpacing(16);
//    bottomHBoxLayout->addWidget(retainUserData);

//    retainUserData->setMaximumSize(0,0);
//    retainUserData->setVisible(false);

//    bottomHBoxLayout->addSpacing(40);
//    bottomHBoxLayout->setAlignment(Qt::AlignRight);

    vLayout->addStretch();
    //vLayout->addLayout(bottomHBoxLayout);
    vLayout->addSpacing(60);
    homePage->setLayout(vLayout);

//    connect(checkFactoryRestore, &MyCheckBox::stateChanged, this, [=](int state) {
//        this->m_isFactoryRestore = Qt::Unchecked == state ? false : true;
//        if (this->m_isFactoryRestore) {
//            retainUserData->setChecked(false);
//        }
//    });
//    connect(retainUserData, &MyCheckBox::stateChanged, this, [=](int state) {
//        this->m_isRetainUserData = Qt::Unchecked == state ? false : true;
//        if (this->m_isRetainUserData) {
//            checkFactoryRestore->setChecked(false);
//        }
//    });

    addWidget(homePage);
}

/**
 * @brief “上一步”按钮响应槽
 * @param checked
 */
void SystemRestore::on_pre_clicked(bool checked)
{
    Q_UNUSED(checked)
    int index = this->currentIndex() - 1;
    if (index >= 0) {
        this->setCurrentIndex(index);
    }
}

/**
 * @brief “下一步”按钮响应槽
 * @param checked
 */
void SystemRestore::on_next_clicked(bool checked)
{
    Q_UNUSED(checked)
    int index = this->currentIndex() + 1;
    if (index < this->count()) {
        this->setCurrentIndex(index);
    }
}

/**
 * @brief “开始还原”按钮响应槽
 * @param checked
 */
void SystemRestore::on_button_beginRestore_clicked(bool checked)
{
    Q_UNUSED(checked)

    this->m_uuid = "";
    this->m_devPath = "";
    this->m_backupName = "";
    this->m_isOtherMachine = false;
    this->m_iPosition = BackupPosition::LOCAL;

    // 出厂还原，不用去选择备份点
    if (m_isFactoryRestore) {
        // 出厂还原后，用户数据将会丢失
        if (!MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(this, QObject::tr("Warning"),
                                                          QObject::tr("Restore factory settings, your system user data will not be retained. Are you sure to continue?"),
                                                          QObject::tr("Continue"), QObject::tr("Cancel"))) {
            return;
        }

        //出厂还原
        m_uuid = FACTORY_BACKUP_UUID;
        m_devPath = "";
        this->m_backupName = "";
        this->m_isOtherMachine = false;
    } else {
        // 系统备份点列表中选择备份点
        SelectRestorePoint *selectRestoreDialog = new SelectRestorePoint(this);
        selectRestoreDialog->setObjectName("yhkylin-backup-tools_kybackup_SelectRestorePoint_selectRestoreDialog");
        connect(selectRestoreDialog, &SelectRestorePoint::selected, this, [=](ParseBackupList::BackupPoint backupPoint){
            this->m_uuid = backupPoint.m_uuid;
            this->m_devPath = backupPoint.m_path;
            this->m_backupName = backupPoint.m_backupName;
           // 2025-5-15 有疑问
           // this->m_isOtherMachine = backupPoint.m_iPosition == BackupPosition::OTHER ? true : false;
           // this->m_iPosition = backupPoint.m_iPosition == BackupPosition::OTHER ? BackupPosition::UDISK : backupPoint.m_iPosition;
            this->m_iPosition = backupPoint.m_iPosition;
            qDebug()<<"i_position is " << this->m_iPosition;
        });

        if (QDialog::Rejected == selectRestoreDialog->exec()) {
            selectRestoreDialog->deleteLater();
            return ;
        }
        selectRestoreDialog->deleteLater();
    }

    on_next_clicked();
    emit this->startCheckEnv();
}

/**
 * @brief 初始化第二界面--检测中
 */
void SystemRestore::initCheckPage()
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
    MyLabel *label2 = new MyLabel(tr("restoring"), checkPage);
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
    connect(preStep, &QPushButton::clicked, this, &SystemRestore::on_pre_clicked);
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
        emit this->startRestore();
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
    connect(this, &SystemRestore::startCheckEnv, this, [=]() {
        this->m_systemRestoreState = SystemRestoreState::CHECKING;
        loadingGif->setVisible(true);
        movie->start();
        resultLogo->setVisible(false);
        // 环境检测中，请等待
        bigTitle->setDeplayText(tr("Checking, wait a moment ..."));
        dot1->setBackgroundColor(Qt::black);
        dot2->setBackgroundColor(Qt::black);
        labelCheck1->setFontColor(Qt::black);
        labelCheck2->setFontColor(Qt::black);
        // 还原过程中不要做其它操作，以防数据丢失
        labelCheck1->setDeplayText(tr("Check whether the restore environment meets the requirements"));
        // 检测还原环境是否满足
        labelCheck2->setDeplayText(tr("Do not perform other operations during restore to avoid data loss"));
        preStep->setVisible(false);
        nextStep->setVisible(false);
        recheck->setVisible(false);

        this->on_checkEnv_start();
        emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();
    });

    // 检测结果
    connect(this, &SystemRestore::checkEnvResult, this, [=](bool result, const QString &errMsg, const QString &errTip) {
        loadingGif->setVisible(false);
        movie->stop();

        if (result) {
            resultLogo->setThemeIconSchema("ukui-dialog-success", ":/symbos/ukui-dialog-success");
            resultLogo->setVisible(true);
            // 检测成功
            bigTitle->setDeplayText(tr("Check success"));
            // 还原完成后将自动重启
            labelCheck1->setDeplayText(tr("The system will reboot automatically after the restore is successful"));
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
void SystemRestore::on_checkEnv_start()
{
    GlobelBackupInfo::instance().setIsBusy(true);
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.systembackup", "/update", QDBusConnection::systemBus(), this);
    m_pInterface->setObjectName("yhkylin-backup-tools_kybackup_ComKylinBackupManagerInterface_m_pInterface");
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemRestore::on_checkEnv_end);

    // 是否已存在备份、还原等操作
    bool isActive = false;
    if(int(BackupState::BACKUP_STATE_INIT) != m_pInterface->getBackupState(isActive)){
        on_checkEnv_end(int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING));

        return;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_type = m_isRetainUserData ? BackupType::RESTORE_SYSTEM_WITH_DATA : BackupType::RESTORE_SYSTEM;
    backupWrapper.m_iPosition = m_iPosition;
    backupWrapper.m_User = BackupUser::USER;
    backupWrapper.m_uuid = m_uuid;
    backupWrapper.m_backupName = m_backupName;
    backupWrapper.m_prefixDestPath = m_devPath;
    backupWrapper.m_isOtherMachine = m_isOtherMachine ? 1 : 0;
    backupWrapper.m_frontUid = getuid();
    backupWrapper.m_frontUserName = qgetenv("USER");
    backupWrapper.m_gid = getgid();
    qDebug() << "环境检测：UUID：" << backupWrapper.m_uuid;
    qDebug() << "环境检测：m_backupName" << backupWrapper.m_backupName;
    qDebug() << "环境检测：m_iPosition：" << backupWrapper.m_iPosition;
    qDebug() << "环境检测：m_type" << backupWrapper.m_type;
    m_pInterface->checkEnv(backupWrapper);
}

/**
 * @brief 环境检测结束
 * @param result, 环境校验结果
 */
void SystemRestore::on_checkEnv_end(int result)
{
    m_systemRestoreState = SystemRestoreState::IDEL;
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
    case int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED):
        // ".user.txt文件不存在"
        errMsg = tr("The .user.txt file does not exist");
        // 备份点可能被损坏
        errTip = tr("Backup points may be corrupted");
        break;
    case int(BackupResult::WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED):
        // .exclude.user.txt文件不存在
        errMsg = tr("The .exclude.user.txt file does not exist");
        // 备份点可能被损坏
        errTip = tr("Backup points may be corrupted");
        break;
    case int(BackupResult::INC_NOT_FOUND_DIR):
        // 备份点数据目录不存在
        errMsg = tr("The backup point data directory does not exist");
        // 备份点可能被损坏
        errTip = tr("Backup points may be corrupted");
        break;
    case int(BackupResult::EFI_RSYNC_FAIL):
        // 同步/boot/efi失败
        errMsg = tr("Failed to rsync /boot/efi");
        // 请检查/boot/efi分区挂载方式
        errTip = tr("Check the mounting mode of the /boot/efi partition");
        break;
    default:
        bRst = true;
        break;
    }

    Utils::wait(1);
    emit checkEnvResult(bRst, errMsg, errTip);
    GlobelBackupInfo::instance().setIsBusy(false);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemRestore::on_checkEnv_end);
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 第三个页面-还原中
 */
void SystemRestore::initRestoringPage()
{
    QWidget *restoringPage = new QWidget;
    restoringPage->setObjectName("yhkylin-backup-tools_kybackup_QWidget_restoringPage");

    // 流程进度提示栏
    CircleLable *one = new CircleLable("1", restoringPage, 24, QColor(COLOR_BLUE));
    LineLabel *line1 = new LineLabel(restoringPage, QColor(COLOR_BLUE));
    CircleLable *two = new CircleLable("2", restoringPage, 24, QColor(COLOR_BLUE));
    LineLabel *line2 = new LineLabel(restoringPage, QColor(COLOR_GRAY));
    CircleLable *three = new CircleLable("3", restoringPage);

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

    MyLabel *label1 = new MyLabel(tr("checking"), restoringPage);
    label1->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label1");
    label1->setIsOriginal(true);
    label1->setFontColor(QColor(COLOR_BLUE));
    label1->setFixedWidth(164);
    MyLabel *label2 = new MyLabel(tr("restoring"), restoringPage);
    label2->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_label2");
    label2->setIsOriginal(true);
    label2->setFontColor(QColor(COLOR_BLUE));
    label2->setFixedWidth(164);
    MyLabel *label3 = new MyLabel(tr("finished"), restoringPage);
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
    QLabel *loadingGif = new QLabel(restoringPage);
    loadingGif->setObjectName("yhkylin-backup-tools_kybackup_QLabel_loadingGif");
    // 环境检测等待动画
    QMovie *movie = new QMovie(":/images/loading.gif", QByteArray(), restoringPage);
    movie->setObjectName("yhkylin-backup-tools_kybackup_QMovie_movie");
    loadingGif->setMovie(movie);
    loadingGif->setVisible(false);
    // 进度条
    KProgressCircle *progressBar = new KProgressCircle(restoringPage);
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
    MyLabel *labelTip = new MyLabel(restoringPage);
    labelTip->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_labelTip");
    labelTip->setAlignment(Qt::AlignCenter);
    labelTip->setIsOriginal(true);
    labelTip->setFontWordWrap(true);
    labelTip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // 不要使用电脑，以防数据丢失
    labelTip->setDeplayText(tr("Do not use computer in case of data loss"));
    hlayoutCenterFont2->addStretch();
    hlayoutCenterFont2->addWidget(labelTip);
    hlayoutCenterFont2->addStretch();
    hlayoutCenterFont2->setAlignment(Qt::AlignCenter);

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
    vlayout->addStretch();
    restoringPage->setLayout(vlayout);

    // 开始还原
    connect(this, &SystemRestore::startRestore, this, [=] {
     //   progressBar->setPersent(0);
        progressBar->setValue(0);
        movie->start();

        // 开始还原
        this->on_restore_start();
    });

    // 进度
    connect(this, &SystemRestore::progress, this, [=](int state, int rate) {
        Q_UNUSED(state)
    //    progressBar->setPersent(rate);
        progressBar->setValue(rate);
    });

    addWidget(restoringPage);
}

/**
 * @brief 开始还原
 */
void SystemRestore::on_restore_start()
{
    //数据埋点：关键功能
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    //传入事件描述“打开文件”，所在界面“主界面”后，上传事件
    kdk_dia_upload_default(node,"systemrestore","systemrestore");
    //释放内存
    kdk_dia_data_free(node);

    GlobelBackupInfo::instance().setIsBusy(true);
    m_systemRestoreState = SystemRestoreState::RESTORING;
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.systembackup", "/update", QDBusConnection::systemBus(), this);
    m_pInterface->setObjectName("yhkylin-backup-tools_kybackup_ComKylinBackupManagerInterface_m_pInterface");
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemRestore::on_checkRestore_end);
    connect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &SystemRestore::progress);
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendRestoreResult, this, &SystemRestore::on_restore_end);

    // 是否已存在备份、还原等操作
    bool isActive = false;
    if(int(BackupState::BACKUP_STATE_INIT) != m_pInterface->getBackupState(isActive)){
        on_checkEnv_end(int(BackupResult::OTHER_BACKUP_OR_RESTORE_RUNNING));

        return;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_uuid = m_uuid;
    backupWrapper.m_backupName = m_backupName;
    backupWrapper.m_type = BackupType::RESTORE_SYSTEM;
    backupWrapper.m_iPosition = m_iPosition;
    backupWrapper.m_User = BackupUser::USER;
    backupWrapper.m_prefixDestPath = m_devPath;
    backupWrapper.m_isOtherMachine = m_isOtherMachine ? 1 : 0;
    backupWrapper.m_frontUid = getuid();
    backupWrapper.m_frontUserName = qgetenv("USER");
    backupWrapper.m_gid = getgid();
    qDebug() << "执行dbus调用:" << "restoreSystemByUser";
    qDebug() << "执行还原：UUID：" << backupWrapper.m_uuid;
    qDebug() << "执行还原：m_backupName" << backupWrapper.m_backupName;
    qDebug() << "执行还原：m_iPosition：" << backupWrapper.m_iPosition;
    qDebug() << "执行还原：m_type" << backupWrapper.m_type;
    m_pInterface->restoreSystemByUser(backupWrapper);
}

/**
 * @brief 系统还原校验结果处理
 * @param result
 */
void SystemRestore::on_checkRestore_end(int result)
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
    case int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED):
        // ".user.txt文件不存在"
        errMsg = tr("The .user.txt file does not exist");
        // 备份点可能被损坏
        errTip = tr("Backup points may be corrupted");
        break;
    case int(BackupResult::WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED):
        // .exclude.user.txt文件不存在
        errMsg = tr("The .exclude.user.txt file does not exist");
        // 备份点可能被损坏
        errTip = tr("Backup points may be corrupted");
        break;
    case int(BackupResult::INC_NOT_FOUND_DIR):
        // 备份点数据目录不存在
        errMsg = tr("The backup point data directory does not exist");
        // 备份点可能被损坏
        errTip = tr("Backup points may be corrupted");
        break;
    case int(BackupResult::EFI_RSYNC_FAIL):
        // 同步/boot/efi失败
        errMsg = tr("Failed to rsync /boot/efi");
        // 请检查/boot/efi分区挂载方式
        errTip = tr("Check the mounting mode of the /boot/efi partition");
        break;
    case int(BackupResult::RESTOREDIR_PREPARE_FAILED):
        // 还原目录准备失败
        errMsg = tr("Failed to prepare the restore directory");
        // 更多信息请参考日志/var/log/backup.log
        errTip = tr("Refer to log :/data/.backup/backup.log for more information");
        break;
    default:
        bRst = true;
        break;
    }

    if (!bRst) {
        GlobelBackupInfo::instance().setIsBusy(false);
        m_systemRestoreState = SystemRestoreState::IDEL;
        this->on_next_clicked(true);
        emit this->checkRestoreResult(bRst, errMsg, errTip);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemRestore::on_checkRestore_end);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &SystemRestore::progress);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendRestoreResult, this, &SystemRestore::on_restore_end);
        delete m_pInterface;
        m_pInterface = nullptr;
    }
}

/**
 * @brief 系统还原结束
 * @param result-false 失败; true 成功
 */
void SystemRestore::on_restore_end(bool result)
{
    m_systemRestoreState = SystemRestoreState::IDEL;

    this->on_next_clicked(true);
    if (result) {
        emit checkRestoreResult(result);
    } else {
        // 还原过程中出现错误
        QString errMsg = tr("An error occurred during restore");
        // 错误信息参考日志文件：/var/log/backup.log
        QString errTip = tr("Refer to log :/data/.backup/backup.log for more information");
        emit checkRestoreResult(result, errMsg, errTip);
    }

    GlobelBackupInfo::instance().setIsBusy(false);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &SystemRestore::on_checkRestore_end);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::progress, this, &SystemRestore::progress);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendRestoreResult, this, &SystemRestore::on_restore_end);
    delete m_pInterface;
    m_pInterface = nullptr;
}

/**
 * @brief 初始化最后一页
 */
void SystemRestore::initLastPage()
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
    MyLabel *label2 = new MyLabel(tr("restoring"), last);
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

    // ------------ 中部布局begin-------------
    QWidget *centerFont = new QWidget(last);
    centerFont->setObjectName("yhkylin-backup-tools_kybackup_QWidget_centerFont");
    centerFont->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *vlayoutCenterFont = new QVBoxLayout;

    // 中部第一行
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
    hlayoutCenterFont1->setAlignment(Qt::AlignLeft);
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
    QHBoxLayout *layoutLine3 = new QHBoxLayout;
    layoutLine3->addStretch();
    layoutLine3->addSpacing(20);
    layoutLine3->addWidget(centerFont);
    layoutLine3->addSpacing(20);
    layoutLine3->addStretch();
    // ------------ 中部布局end-------------

    // 布局
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(40);
    vlayout->addLayout(layoutLine1);
    vlayout->addLayout(layoutLine2);
    vlayout->addSpacing(80);
    vlayout->addLayout(layoutLine3);
    vlayout->addStretch();
    last->setLayout(vlayout);

    // 还原检测结果
    connect(this, &SystemRestore::checkRestoreResult, this, [=](bool result, const QString &errMsg, const QString &errTip) {
        if (result) {
            resultLogo->setThemeIconSchema("ukui-dialog-success", ":/symbos/ukui-dialog-success");
            resultLogo->setVisible(true);
            // 系统还原成功
            bigTitle->setDeplayText(tr("Successfully restoring the system"));
            dot1->setVisible(true);
            dot2->setVisible(true);
            labelError1->setVisible(true);
            labelError2->setVisible(true);
            // 系统将自动重启
            labelError1->setDeplayText(tr("The system will automatically reboot."));
            labelError2->setDeplayText(tr("Or not, please force shutdown and restart."));
            retry->setVisible(false);
            homePage->setVisible(false);
        } else {
            resultLogo->setThemeIconSchema("dialog-error", ":/symbos/dialog-error");
            resultLogo->setVisible(true);
            // 系统还原失败
            bigTitle->setDeplayText(tr("Restoring the system failed"));
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
        this->setCurrentIndex(SystemRestorePage::RESTORE_PAGE);
        emit this->startRestore();
    });

    // 返回首页
    connect(homePage, &MyPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        this->setCurrentIndex(SystemRestorePage::HOME_PAGE);
    });

    addWidget(last);
}



