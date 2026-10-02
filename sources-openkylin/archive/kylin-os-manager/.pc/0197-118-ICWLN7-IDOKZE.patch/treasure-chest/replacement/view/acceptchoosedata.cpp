#include <QDebug>
#include "../globaldata.h"
#include "acceptchoosedata.h"
#include "dbus_args.h"
#include <QDir>
#include <QTime>

AcceptChooseData::AcceptChooseData(QWidget *parent)
{
    initUI();
    initConn();
}

AcceptChooseData::~AcceptChooseData()
{

}

void AcceptChooseData::initUI()
{
    this->setFixedWidth(600);
    m_checkBoxApp = new DataCheckBox(true, this);
    m_checkBoxUser = new DataCheckBox(false, this);
    m_checkBoxSys = new DataCheckBox(false, this);

    m_lable = new K::TextLabel(this);
    m_lableSize = new K::TextLabel(this);

    m_btnBack = new QPushButton(this);
    m_btnStart = new QPushButton(this);

    m_layoutVMain = new QVBoxLayout(this);
    m_layoutHLabel = new QHBoxLayout();
    m_layoutH = new QHBoxLayout();

    m_layoutH->addStretch();
    m_layoutH->addWidget(m_btnBack);
    m_layoutH->addSpacing(16);
    m_layoutH->addWidget(m_btnStart);
    m_layoutH->addStretch();
    m_layoutH->setSpacing(0);
    m_layoutH->setMargin(0);

    m_layoutHLabel->addSpacing(24);
    m_layoutHLabel->addWidget(m_lable);
    m_layoutHLabel->addWidget(m_lableSize);
    m_layoutHLabel->addStretch();

    m_layoutVMain->addWidget(m_checkBoxApp, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(16);
    m_layoutVMain->addWidget(m_checkBoxUser, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(16);
    m_layoutVMain->addWidget(m_checkBoxSys, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(16);
    m_layoutVMain->addLayout(m_layoutHLabel);
    m_layoutVMain->addSpacing(16);
    m_layoutVMain->addLayout(m_layoutH);
    m_layoutVMain->addStretch();
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);

    m_checkBoxApp->setLabelText(DataCheckBox::DataType::AppData);
    m_checkBoxUser->setLabelText(DataCheckBox::DataType::UserData);
    m_checkBoxSys->setLabelText(DataCheckBox::DataType::SysData);

    m_lable->SetText(GlobalData::g_textRectSelected);
    m_lableSize->SetText(GlobalData::g_textRectCalculating);
    m_btnBack->setText(GlobalData::g_textBackBtn);
    m_btnStart->setText(GlobalData::g_textRectMigrate);
    m_btnStart->setEnabled(false);
    
    GlobalData::getInstance()->setHighColorBtn(m_btnStart);
    m_lable->hide();
    m_lableSize->hide();

    configAppData();
    configSysData();
    configUseData();
}

void AcceptChooseData::initConn()
{
    connect(m_btnBack, &QPushButton::clicked, this, &AcceptChooseData::slotBackPage);
    connect(m_btnStart, &QPushButton::clicked, this, &AcceptChooseData::slotStartMigration);
    connect(m_checkBoxApp, &DataCheckBox::sigClicked, this, &AcceptChooseData::slotChangeBtnStyle);
    connect(m_checkBoxApp, &DataCheckBox::sigCannel, this, &AcceptChooseData::slotChangeBtnStyle);
    connect(m_checkBoxSys, &DataCheckBox::sigClicked, this, &AcceptChooseData::slotChangeBtnStyle);
    connect(m_checkBoxSys, &DataCheckBox::sigCannel, this, &AcceptChooseData::slotChangeBtnStyle);
    connect(m_checkBoxUser, &DataCheckBox::sigClicked, this, &AcceptChooseData::slotChangeBtnStyle);
    connect(m_checkBoxUser, &DataCheckBox::sigCannel, this, &AcceptChooseData::slotChangeBtnStyle);
}

void AcceptChooseData::configSysData()
{
    QString resourceFilePath = ":/data/etcPolicy.yaml";
    QString destinationPath = confDirPath;
    QString destinationFile = m_configDataYamlFile;

    // 检查目标文件是否存在
    if (!QFile::exists(destinationFile)) {
        // 创建目标目录（如果不存在）
        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            if (!destinationDir.mkpath(destinationPath)) {
                qDebug() << " Config File dir mkpath faile";
            }
        }
        // 复制资源文件到目标路径
        if (!QFile::copy(resourceFilePath, destinationFile)) {
            qDebug() << "Failed to copy resource file!";
        }
        qDebug() << "Resource file has been copied to" << destinationFile;
    } else {
        qDebug() << "File" << destinationFile << "already exists.";
    }
}

void AcceptChooseData::configUseData()
{
    QString resourceFilePath = ":/data/etcPolicy.yaml";
    QString destinationPath = confDirPath;
    QString destinationFile = m_configDataYamlFile;

    // 检查目标文件是否存在
    if (!QFile::exists(destinationFile)) {
        // 创建目标目录（如果不存在）
        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            if (!destinationDir.mkpath(destinationPath)) {
                qDebug() << " Config File dir mkpath faile";
            }
        }
        // 复制资源文件到目标路径
        if (!QFile::copy(resourceFilePath, destinationFile)) {
            qDebug() << "Failed to copy resource file!";
        }
        qDebug() << "Resource file has been copied to" << destinationFile;
    } else {
        qDebug() << "File" << destinationFile << "already exists.";
    }
}

void AcceptChooseData::configAppData()
{
    QString resourceFilePath = ":/data/appClassification.yaml";
    QString destinationPath = confDirPath;
    QString destinationFile = m_appDataYamlFile;

    // 检查目标文件是否存在
    if (!QFile::exists(destinationFile)) {
        // 创建目标目录（如果不存在）
        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            if (!destinationDir.mkpath(destinationPath)) {
                qDebug() << " Config File dir mkpath faile";
            }
        }
        // 复制资源文件到目标路径
        if (!QFile::copy(resourceFilePath, destinationFile)) {
            qDebug() << "Failed to copy resource file!";
        }
        qDebug() << "Resource file has been copied to" << destinationFile;
    } else {
        qDebug() << "File" << destinationFile << "already exists.";
    }
}

void AcceptChooseData::slotBackPage()
{
    emit sigBackPage();
}

void AcceptChooseData::slotStartMigration()
{
    ChooseDataType type;
    bool app = m_checkBoxApp->getCilckStyle();
    bool sys = m_checkBoxSys->getCilckStyle();
    bool user = m_checkBoxUser->getCilckStyle();
    if (app && sys && user) {
        type = ChooseDataType::AllData;
    } else if (!app && sys && user) {
        type = ChooseDataType::UserAndSys;
    } else if (app && !sys && user) {
        type = ChooseDataType::AppAndUser;
    } else if (app && sys && !user) {
        type = ChooseDataType::AppAndSys;
    } else if (!app && !sys && user) {
        type = ChooseDataType::UserData;
    } else if (app && !sys && !user) {
        type = ChooseDataType::AppData;
    } else if (!app && sys && !user) {
        type = ChooseDataType::SysData;
    }

    GlobalData::getInstance()->g_timeStart = QDateTime::currentDateTime();

    emit sigChooseData(type);

    m_checkBoxApp->initAppWidget();
}

void AcceptChooseData::slotChangeBtnStyle()
{
    bool app = m_checkBoxApp->getCilckStyle();
    bool sys = m_checkBoxSys->getCilckStyle();
    bool user = m_checkBoxUser->getCilckStyle();
    if (app || sys || user) {
        // m_btnStart->setEnabled(true);
    } else {
        m_btnStart->setEnabled(false);
    }
/*
    enum MigrateDataType {
        AppData = 0,
        UserData,
        SysConfData,
        AppAndUserData,
        AppAndSysData,
        UserAndSysData,
        AllData,
    };
*/
    m_lable->show();
    m_lableSize->show();
    m_lableSize->SetText(GlobalData::g_textRectCalculating);
    m_btnStart->setEnabled(false);
    QDBusMessage msg;
    ChooseDataType type;
    if (app && sys && user) {
        type = ChooseDataType::AllData;
    } else if (!app && sys && user) {
        type = ChooseDataType::UserAndSys;
    }else if (app && !sys && user) {
        type = ChooseDataType::AppAndUser;
    }else if (app && sys && !user) {
        type = ChooseDataType::AppAndSys;
    }else if (app && !sys && !user) {
        type = ChooseDataType::AppData;
    }else if (!app && !sys && user) {
        type = ChooseDataType::UserData;
    }else if (!app && sys && !user) {
        type = ChooseDataType::SysData;
    } else {
        m_lable->hide();
        m_lableSize->hide();
        return;
    }
    GlobalData::getInstance()->dbusReceiverGetDataInfo(type);
}

void AcceptChooseData::slotMigrationDataSize(qint64 num)
{
    int suffixNum = 0;
    _Float64 size = num;
    QString suffixStr = "B";
    if (num > 1024) {
        for (int i = 0; size > 1024; i++) {
            size = size / 1024;
            suffixNum = i + 1;
            if (suffixNum == 1) {
                suffixStr = "KB";
            } else if(suffixNum == 2) {
                suffixStr = "MB";
            } else if(suffixNum == 3) {
                suffixStr = "GB";
                break;
            }
        }
    }
    QString tmpSize = QString::number(size, 'f', 1);

    m_lableSize->SetText(tmpSize + suffixStr);
    qDebug() << " ===== slotMigrationDataSize ===== 当前类型拉取数据大小" << num << "转换后==》" << tmpSize + suffixStr;
    m_btnStart->setEnabled(true);
}

void AcceptChooseData::initAppWidget()
{
    m_checkBoxApp->initAppWidget();
}