#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QDBusPendingReply>
#include <QDBusPendingCall>
#include "../globaldata.h"
#include "dbus_args.h"

#include "acceptmigrationdata.h"

AcceptMigrationData::AcceptMigrationData(QWidget *parent)
{
    initUI();
    initStyle();
    initConn();
}

AcceptMigrationData::~AcceptMigrationData()
{

}

void AcceptMigrationData::initUI()
{
    this->setFixedSize(600, 300);
    m_widget = new QWidget(this);
    m_btnWaring = new QPushButton(m_widget);
    m_labelWarning = new K::TextLabel(m_widget);
    
    QHBoxLayout *hWarningLayout = new QHBoxLayout(m_widget);
    hWarningLayout->addSpacing(16);
    hWarningLayout->addWidget(m_btnWaring, 0, Qt::AlignVCenter);
    hWarningLayout->addSpacing(8);
    hWarningLayout->addWidget(m_labelWarning, 0, Qt::AlignVCenter);
    hWarningLayout->addStretch();
    hWarningLayout->addSpacing(8);
    hWarningLayout->setSpacing(0);
    hWarningLayout->setMargin(0);

    // m_labelOld = new K::TextLabel(this);
    // m_labelIconOld = new QLabel(this);

    m_iconWidgetOld = new IconWidget(this);
    m_iconWidgetNew = new IconWidget(this);
    m_iconWidgetSend = new IconWidget(this);
    m_labelCurrent = new K::TextLabel(this);
    m_label = new K::TextLabel(this);

    QVBoxLayout *vIconLayoutNew = new QVBoxLayout();
    vIconLayoutNew->addWidget(m_iconWidgetNew);
    vIconLayoutNew->addWidget(m_labelCurrent, 0, Qt::AlignHCenter);
    vIconLayoutNew->addStretch();

    QVBoxLayout *vIconLayoutOld = new QVBoxLayout();
    vIconLayoutOld->addWidget(m_iconWidgetOld);
    vIconLayoutOld->addWidget(m_label, 0, Qt::AlignHCenter);
    vIconLayoutOld->addStretch();


    QHBoxLayout *hIconLayou = new QHBoxLayout();
    hIconLayou->addSpacing(86);
    hIconLayou->addLayout(vIconLayoutOld);
    hIconLayou->addStretch();
    hIconLayou->addWidget(m_iconWidgetSend, 0, Qt::AlignVCenter);
    hIconLayou->addStretch();
    hIconLayou->addLayout(vIconLayoutNew);
    hIconLayou->addSpacing(86);

    m_progressBar = new ProgressBar(this);

    m_layouV = new QVBoxLayout(this);
    m_layouV->addWidget(m_widget);
    m_layouV->addSpacing(14);
    m_layouV->addLayout(hIconLayou);
    m_layouV->addSpacing(12);
    m_layouV->addWidget(m_progressBar, 0, Qt::AlignHCenter);
    m_layouV->addStretch();
    m_layouV->setMargin(0);
    m_layouV->setSpacing(0);

}

void AcceptMigrationData::initStyle()
{
    m_btnWaring->setFixedSize(20, 20);
    m_btnWaring->setIconSize(QSize(16, 16));               
    m_btnWaring->setIcon(GlobalData::g_iconWarning); 
    m_btnWaring->setFlat(true);
    m_btnWaring->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_labelWarning->SetText(GlobalData::g_textRectMigrateTip);
    m_labelWarning->SetFontColor(QColor(248, 163, 76));
    m_labelWarning->SetPixelSize(12, true);

    m_widget->setFixedSize(600, 28);
    m_widget->setObjectName("warningWidget");
    m_widget->setStyleSheet("#warningWidget{background-color: rgba(248, 163, 76, 0.1);border-top-left-radius: 16px;border-top-right-radius: 16px;}");

    m_iconWidgetNew->setLabelIcon(GlobalData::g_sizeDeviceIconBig, GlobalData::g_iconNewSys, GlobalData::g_textRectMigrateNew);
    m_iconWidgetOld->setLabelIcon(GlobalData::g_sizeDeviceIconBig, GlobalData::g_iconOldSys, GlobalData::g_textReceMigrateOld);
    m_iconWidgetSend->setLabelIcon(QSize(16, 16), GlobalData::g_iconSending, GlobalData::g_textReceMigrateSending);

    m_labelCurrent->SetText(GlobalData::g_textRectMigrateCurrent);
    m_labelCurrent->SetFontColor(QColor(166, 166, 166));

    m_progressBar->SetBarRange(0, 1100);
    // m_progressBar->setBarValue(50);
    // m_progressBar->setFileName("C:====================");
    // m_progressBar->setSpeedValue("1.6MB/s");

    m_dbus = new QDBusInterface(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                QDBusConnection::systemBus());
}

void AcceptMigrationData::initConn()
{

    QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                          QString("sigMigrateDataSuccess"), this, SLOT(slotConfigData()));
}

void AcceptMigrationData::getMigrationFinishData()
{
    int suffixNum = 0;
    _Float64 size = GlobalData::getInstance()->g_migrateFinishDataSize;
    QString suffixStr = "B";
    if (GlobalData::getInstance()->g_migrateFinishDataSize > 1024) {
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

    GlobalData::getInstance()->g_migrateFinishDataStr = tmpSize + suffixStr;

}

void AcceptMigrationData::slotConfigData()
{
    QDBusPendingCall pcall = m_dbus->asyncCall("receiverConfigData");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this, SLOT(slotConfigFinish()));

}

void AcceptMigrationData::slotMigrationDataInfo(QStringList list)
{
    if (list[0] != "") {
        m_progressBar->setFileName(list[0]);
    }
    if (list[1] != "") {
        double f =  (double)list[1].toLongLong() / GlobalData::getInstance()->g_migrationDataSize;
        int value = f * 1000;
        qDebug() << value << list[1] << list[1].toLongLong() << GlobalData::getInstance()->g_migrationDataSize  <<  f;
        m_progressBar->setBarValue(value);
        GlobalData::getInstance()->g_migrateFinishDataSize = list[1].toLongLong();
    } 
    if (list[2] != "") {
        m_progressBar->setSpeedValue(list[2]);
    }

    if (list[1].toLongLong() == GlobalData::getInstance()->g_migrationDataSize) {
        // 配置 receiverConfigData
        QDBusMessage msg = m_dbus->call("receiverConfigData");
        if (msg.errorMessage() != "")     
            qDebug() << "dbus interface error : receiverConfigData" << msg.errorMessage();
    }
}

void AcceptMigrationData::slotSetProgressRange(qint64 num)
{
    GlobalData::getInstance()->g_migrationDataSize = num;
}

void AcceptMigrationData::slotConfigFinish()
{
    m_progressBar->setBarValue(GlobalData::getInstance()->g_migrationDataSize);
    GlobalData::getInstance()->g_timeEnd = QDateTime::currentDateTime();

    int totalSeconds = GlobalData::getInstance()->g_timeStart.secsTo(GlobalData::getInstance()->g_timeEnd);
    
    int hours = totalSeconds / 3600;      // 计算小时
    int minutes = (totalSeconds / 60) % 60; // 计算分钟
    int seconds = totalSeconds % 60;        // 计算剩余的秒钟
    if (hours == 0) {
        if (minutes == 0) {
            GlobalData::getInstance()->g_time = QString::number(seconds) + tr("sec");
        } else {
            GlobalData::getInstance()->g_time = QString::number(minutes) + tr("min")+ QString::number(seconds) + tr("sec");
        }
    } else {
        GlobalData::getInstance()->g_time = QString::number(hours) + tr("hour") + QString::number(minutes) + tr("min")+ QString::number(seconds) + tr("sec");
    }
    getMigrationFinishData();

    emit sigMigratDataFinish();
}

void AcceptMigrationData::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = QApplication::palette().window().color();
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    QColor highLightColor = opt.palette.highlight().color();
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 16, 16); // 绘制圆角
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);    
}
