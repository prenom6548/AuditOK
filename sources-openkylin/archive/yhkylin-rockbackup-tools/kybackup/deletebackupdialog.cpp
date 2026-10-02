/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "deletebackupdialog.h"
#include "ui_deletebackupdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <ukuistylehelper/ukuistylehelper.h>
#include <windowmanager/windowmanager.h>
#include <unistd.h>
#include "globalbackupinfo.h"
#include "../common/mydefine.h"
#include "../common/utils.h"
#include "messageboxutils.h"

DeleteBackupDialog::DeleteBackupDialog(ParseBackupList::BackupPoint backupPonit, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeleteBackupDialog),
    m_backupPonit(backupPonit),
    m_pInterface(nullptr)
{
    ui->setupUi(this);
    // 为解决自动贴边问题，设置为固定大小
    setFixedSize(450, 200);
    this->setAutoFillBackground(true);

    // 去除窗管标题栏，传入参数为QWidget *
    kdk::UkuiStyleHelper::self()->removeHeader(this);

    setWindowModality(Qt::WindowModal);

    // 设置标题栏
    QHBoxLayout * titleLayout = new QHBoxLayout();

    // logo
    m_mTitleIcon  = new PixmapLabel;
    m_mTitleIcon->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_m_mTitleIcon");
    m_mTitleIcon->setFixedSize(24, 24);
    m_mTitleIcon->setThemeIconSchema(THEME_YHKYLIN_BACKUP_TOOLS, ":/images/yhkylin-backup-tools.png");

    // 标题
    m_titleLabel  = new MyLabel;
    m_titleLabel->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_m_titleLabel");
    m_titleLabel->setFixedSize(380, 30);
    // 正在删除数据，请稍候
    m_titleLabel->setDeplayText(tr("Please wait while data is being removed"));
    m_titleLabel->setToolTip(tr("Please wait while data is being removed"));

    // 关闭按钮
    m_buttonClose = new QPushButton;
    m_buttonClose->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_m_buttonClose");
    m_buttonClose->setProperty("isWindowButton", 0x2);
    m_buttonClose->setProperty("useIconHighlightEffect", 0x8);
    m_buttonClose->setFlat(true);
    if (GlobelBackupInfo::instance().isTabletMode())
        m_buttonClose->setFixedSize(TABLET_TITLE_BAR_SIZE, TABLET_TITLE_BAR_SIZE);
    else
        m_buttonClose->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_buttonClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_buttonClose->setToolTip(tr("Close"));

    titleLayout->addWidget(m_mTitleIcon);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_buttonClose);

    // 提示信息
    m_labelMessage = new MyLabel;
    m_labelMessage->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_m_labelMessage");
    m_labelMessage->setIsOriginal(true);
    m_labelMessage->setWordWrap(true);
    // 正在删除备份点
    m_labelMessage->setDeplayText(tr("Removing backup point..."));

    // 等待图标
    m_loadingGif = new QLabel;
    m_loadingGif->setObjectName("yhkylin-backup-tools_kybackup_QLabel_m_loadingGif");
    m_loadingGif->setFixedSize(24,24);
    // 等待动画
    m_movie = new QMovie(":/images/loading.gif", QByteArray(), this);
    m_movie->setObjectName("yhkylin-backup-tools_kybackup_QMovie_m_movie");
    m_loadingGif->setMovie(m_movie);

    QHBoxLayout *layoutTop = new QHBoxLayout;
    layoutTop->addSpacing(20);
    layoutTop->addWidget(m_labelMessage);
    layoutTop->addWidget(m_loadingGif);
    layoutTop->addSpacing(20);

    // 确定按钮
    m_buttonOk = new MyPushButton;
    m_buttonOk->setObjectName("yhkylin-backup-tools_kybackup_MyPushButton_m_buttonOk");
    m_buttonOk->setText(tr("OK"));
    m_buttonOk->setProperty("isImportant", true);

    QHBoxLayout *layoutBottom = new QHBoxLayout;
    layoutBottom->addStretch();
    layoutBottom->addWidget(m_buttonOk);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addLayout(titleLayout);
    vlayout->addStretch();
    vlayout->addLayout(layoutTop);
    vlayout->addStretch();
    vlayout->addLayout(layoutBottom);

    connect(m_buttonClose, &MyPushButton::clicked, this, &DeleteBackupDialog::close);
    connect(m_buttonOk, &MyPushButton::clicked, this, &DeleteBackupDialog::close);

    setLayout(vlayout);

    deleteBackupPoint();
}

DeleteBackupDialog::~DeleteBackupDialog()
{
    delete ui;
    delete m_pInterface;
}

/**
 * @brief 删除备份点
 * @param backupPonit
 */
void DeleteBackupDialog::deleteBackupPoint()
{
    m_buttonClose->hide();
    m_buttonOk->setEnabled(false);
    m_movie->start();

    GlobelBackupInfo::instance().setIsBusy(true);
    m_pInterface = new ComKylinBackupManagerInterface("com.kylin.backup", "/", QDBusConnection::systemBus(), this);
    m_pInterface->setObjectName("yhkylin-backup-tools_kybackup_ComKylinBackupManagerInterface_m_pInterface");
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &DeleteBackupDialog::on_checkEnv_end);
    connect(m_pInterface, &ComKylinBackupManagerInterface::sendDeleteResult, this, &DeleteBackupDialog::on_deleteBackup_end);

    // 是否已存在备份、还原等操作
    bool isActive = false;
    if(int(BackupState::BACKUP_STATE_INIT) != m_pInterface->getBackupState(isActive)){
        m_labelMessage->setDeplayText(tr("Other backup or restore task is being performed"));
        m_buttonClose->show();
        m_buttonOk->setEnabled(true);
        m_movie->stop();
        m_loadingGif->hide();
        emit deleteFinished(false);

        GlobelBackupInfo::instance().setIsBusy(false);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &DeleteBackupDialog::on_checkEnv_end);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendDeleteResult, this, &DeleteBackupDialog::on_deleteBackup_end);
        delete m_pInterface;
        m_pInterface = nullptr;

        return;
    }

    BackupWrapper backupWrapper;
    backupWrapper.m_backupName = m_backupPonit.m_backupName;
    backupWrapper.m_uuid = m_backupPonit.m_uuid;
    backupWrapper.m_prefixDestPath = m_backupPonit.m_path;
    backupWrapper.m_type = BackupType::DELETE_BACKUP;
    QString udiskPrePath = Utils::getSysRootPath();
    udiskPrePath += "/media";
    udiskPrePath.replace("//", "/");
    backupWrapper.m_iPosition = m_backupPonit.m_iPosition;
    backupWrapper.m_frontUid = getuid();
    backupWrapper.m_gid = getgid();
    m_pInterface->deleteBackupPoint(backupWrapper);
}

/**
 * @brief 环境校验结果
 * @param result
 */
void DeleteBackupDialog::on_checkEnv_end(int result)
{
    bool bRst = false;
    QString errMsg;
    switch (result) {
    case int(BackupResult::LOCK_PROGRAM_FAIL):
        // 程序锁定失败，请重试
        errMsg = tr("Program lock failed, please retry");
        break;
    case int(BackupResult::NO_FOUND_DEALCLASS):
        // 不支持的任务类型
        errMsg = tr("Unsupported task type");
        break;
    default:
        bRst = true;
    }

    if (!bRst) {
        m_labelMessage->setDeplayText(errMsg);
        m_buttonClose->show();
        m_buttonOk->setEnabled(true);
        m_movie->stop();
        m_loadingGif->hide();
        emit deleteFinished(false);

        GlobelBackupInfo::instance().setIsBusy(false);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &DeleteBackupDialog::on_checkEnv_end);
        disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendDeleteResult, this, &DeleteBackupDialog::on_deleteBackup_end);
        delete m_pInterface;
        m_pInterface = nullptr;
    }
}

/**
 * @brief 删除备份点结果
 * @param result
 */
void DeleteBackupDialog::on_deleteBackup_end(bool result)
{
    if (result) {
        // 备份删除成功
        m_labelMessage->setDeplayText(tr("Deleted backup successfully."));
    } else
        m_labelMessage->setDeplayText(tr("Failed to delete backup."));

    m_buttonClose->show();
    m_buttonOk->setEnabled(true);
    m_movie->stop();
    m_loadingGif->hide();
    emit deleteFinished(result);

    GlobelBackupInfo::instance().setIsBusy(false);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendEnvCheckResult, this, &DeleteBackupDialog::on_checkEnv_end);
    disconnect(m_pInterface, &ComKylinBackupManagerInterface::sendDeleteResult, this, &DeleteBackupDialog::on_deleteBackup_end);
    delete m_pInterface;
    m_pInterface = nullptr;
}

void DeleteBackupDialog::paintEvent(QPaintEvent *event)
{
    // 设置背景色
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, palette.color(QPalette::Base));
    this->setPalette(palette);
    QDialog::paintEvent(event);
}


