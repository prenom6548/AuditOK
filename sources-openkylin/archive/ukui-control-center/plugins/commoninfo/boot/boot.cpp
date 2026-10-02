/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "boot.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include <polkit-qt5-1/polkitqt1-authority.h>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#include "lightlabel.h"

#include "grubverifydialog.h"
#include "ukcccommon.h"
using namespace ukcc;

enum TaskType {
    GRUBVERIFY,
    BOOTDELAY,
    DEFAULTBOOT,
    RESCUEMODE,
    FAILURETIMES,
};

#define DEFAULTLISTHEIGHT 240

Boot::Boot() : m_firstLoad(true)
{
    m_pluginName = tr("Boot");
    m_pluginType = COMMONINFO;
}

Boot::~Boot()
{
}

QString Boot::plugini18nName()
{
    return m_pluginName;
}

int Boot::pluginTypes()
{
    return m_pluginType;
}

QWidget *Boot::pluginUi()
{
    if (m_firstLoad) {
        m_firstLoad = false;
        m_pPluginWidget = new QWidget;
        m_pPluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        initUI(m_pPluginWidget);

        m_pUkccSystemDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                          "/",
                                          "com.control.center.interface",
                                          QDBusConnection::systemBus(), this);
        qCritical() << "default timeout = " << m_pUkccSystemDbus->timeout();
        m_pUkccSystemDbus->setTimeout(2147483647); // -1 为默认的25s超时;
        if (!m_pUkccSystemDbus->isValid()){
            qCritical() << "Create Client Interface Failed:" << QDBusConnection::systemBus().lastError();
        }

        m_pUkccSessionDbus = new QDBusInterface( "org.ukui.ukcc.session",
                                                "/Boot",
                                                "org.ukui.ukcc.session.Boot",
                                                QDBusConnection::sessionBus(), this);

        m_pUkccSessionDbus->setTimeout(2147483647); // -1 为默认的25s超时;
        if (!m_pUkccSessionDbus->isValid()){
            qCritical() << "Create Client Interface Failed:" << QDBusConnection::sessionBus().lastError();
        }

        m_pukccGsetting = new QGSettings(UKCC_SCHAME, QByteArray(), this);

        initConnection();
    }
    return m_pPluginWidget;
}

const QString Boot::name() const
{
    return QStringLiteral("Boot");
}

bool Boot::isShowOnHomePage() const
{
    return true;
}

QIcon Boot::icon() const
{
    return QIcon::fromTheme("ukui-bootmenu-symbolic");
}

bool Boot::isEnable() const
{
    // icbc项目要求隐藏
    QString platformName = QString(QLatin1String(kdk_system_get_appScene()));
    qDebug() << "platformName = " << platformName;
    if (platformName.contains("ICBC", Qt::CaseInsensitive)) {
        return false;
    }
    return true;
}

void Boot::initUI(QWidget *widget)
{
    QVBoxLayout *mainVLayout = new QVBoxLayout(widget);
    mainVLayout->setContentsMargins(0, 0, 0, 0);

    KLabel *bootTitleLabel = new KLabel(m_pPluginWidget);
    bootTitleLabel->setContentsMargins(16,0,0,0);
    KDK_EXTEND_ALL_INFO_FORMAT(bootTitleLabel, name(), "", "");
    //~ contents_path /Boot/Boot
    bootTitleLabel->setText(tr("Boot"));
    bootTitleLabel->setContentsMargins(14,0,0,0);

    //~ contents_path /Boot/Grub Password Verification
    m_pBootFrame = new SwitchWidget(tr("Grub Password Verification"), m_pPluginWidget, UkccFrame::BorderRadiusStyle::Around, tr("Password verification is required to edit Grub startup parameters"));
    m_pBootFrame->setMinimumSize(QSize(550, 0));
    m_pBootFrame->setMaximumSize(QSize(16777215, 16777215));
    m_pBootFrame->setHeightAdaptive(false);
    m_pResetPasswdBtn = new KPushButton();
    m_pResetPasswdBtn->setText(tr("Reset password"));
    m_pBootFrame->insertWidget(2, m_pResetPasswdBtn);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pBootFrame, name(), "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pResetPasswdBtn, name(), "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pBootFrame->switchButton(), name(), "", "");
    m_pBootFrame->nameLabel()->setAccessibleName(tr("Grub Password Verification"));
    m_pBootFrame->nameLabel()->setBuddy(m_pResetPasswdBtn);
    m_pResetPasswdBtn->setAccessibleName(tr("Reset password"));
    m_pBootFrame->switchButton()->setAccessibleName(tr("Grub Password Verification"));

    //~ contents_path /Boot/Grub Boot Delay
    m_pGrubBootDelayFrame = new ComboxWidget(tr("Grub Boot Delay"), m_pPluginWidget, UkccFrame::Around);
    m_pGrubBootDelayFrame->label()->setFixedWidth(550);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pGrubBootDelayFrame->label(), name(), "BootDelayLabel", "");
    m_pGrubBootDelayFrame->comboBox()->addItem(tr("1s"), 1);
    m_pGrubBootDelayFrame->comboBox()->addItem(tr("3s"), 3);
    m_pGrubBootDelayFrame->comboBox()->addItem(tr("5s"), 5);
    m_pGrubBootDelayFrame->comboBox()->addItem(tr("10s"), 10);
    m_pGrubBootDelayFrame->comboBox()->addItem(tr("Infinite Waiting"), -1);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pGrubBootDelayFrame->comboBox(), name(), "BootDelayCombox", "");
    m_pGrubBootDelayFrame->label()->setAccessibleName(tr("Grub Boot Delay"));
    m_pGrubBootDelayFrame->label()->setBuddy(m_pGrubBootDelayFrame->comboBox());
    m_pGrubBootDelayFrame->comboBox()->setAccessibleName(tr("Grub Boot Delay"));


    m_pRescueModeGroup = new SettingGroup(m_pPluginWidget);
    //~ contents_path /Boot/After the system fails to start, it automatically enters the rescue system
    m_pRescueModeFrame = new SwitchWidget(tr("After the system fails to start, it automatically enters the rescue system"),
                                          m_pPluginWidget, UkccFrame::BorderRadiusStyle::Around,
                                          tr("In the rescue system, tasks such as repairing GRUB, setting up boot security configurations, and fixing disks can be performed."));
    m_pRescueModeFrame->setMinimumSize(QSize(550, 1));
    m_pRescueModeFrame->setMaximumSize(QSize(16777215, 16777215));
    m_pRescueModeFrame->setHeightAdaptive(false);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pRescueModeFrame, name(), "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pRescueModeFrame->switchButton(), name(), "", "");
    //~ contents_path /Boot/Number of system boot failures
    m_pFailuersFrame = new ComboxWidget(tr("Number of system boot failures"), m_pPluginWidget, UkccFrame::Around);
    m_pFailuersFrame->label()->setFixedWidth(550);
    m_pFailuersFrame->comboBox()->addItem(tr("2 times"), 2);
    m_pFailuersFrame->comboBox()->addItem(tr("3 times"), 3);
    m_pFailuersFrame->comboBox()->addItem(tr("4 times"), 4);
    m_pFailuersFrame->comboBox()->addItem(tr("5 times"), 5);
    m_pFailuersFrame->comboBox()->addItem(tr("6 times"), 6);
    m_pFailuersFrame->comboBox()->addItem(tr("7 times"), 7);
    m_pFailuersFrame->comboBox()->addItem(tr("8 times"), 8);
    m_pFailuersFrame->comboBox()->addItem(tr("9 times"), 9);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pFailuersFrame, name(), "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pFailuersFrame->comboBox(), name(), "Failuers combox", "");

    m_pRescueModeGroup->insertWidget(0, m_pRescueModeFrame);
    m_pRescueModeGroup->insertWidget(1, m_pFailuersFrame);

    m_pBootOrderFrame = new KBackgroundGroup(m_pPluginWidget);
    QWidget *defaultBootwidget = new QWidget();
    m_pDefaultBootLabel = new KLabel(defaultBootwidget);
    //~ contents_path /Boot/Default Startup System
    m_pDefaultBootLabel->setText(tr("Default Startup System"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_pDefaultBootLabel, name(), "BootOrderLabel", "");
    QHBoxLayout *widget1Layout = new QHBoxLayout(defaultBootwidget);
    widget1Layout->setContentsMargins(16, 0, 16, 0);
    widget1Layout->addWidget(m_pDefaultBootLabel);
    m_pBootOrderFrame->addWidget(defaultBootwidget);
    m_pDefaultBootLabel->setAccessibleName(tr("Default Startup System"));

    m_pBootListGroup = new QButtonGroup();

    mainVLayout->addWidget(bootTitleLabel);
    mainVLayout->addWidget(m_pBootFrame);
    mainVLayout->addWidget(m_pGrubBootDelayFrame);
    mainVLayout->addWidget(m_pRescueModeGroup);
    mainVLayout->addWidget(m_pBootOrderFrame);
    mainVLayout->addStretch();

    // 判断CPU架构隐藏
    QString cpu = UkccCommon::getCpuArchitecture();
    if (cpu.contains("sw") || cpu.contains("mips")) {
        m_pGrubBootDelayFrame->setVisible(false);
        m_pBootOrderFrame->setVisible(false);
    }
}

void Boot::initConnection()
{
    initGrubVerifyStatus();
    initBootTime();

    initBootList();
    initDefaultBoot();
    initRescueMode();

    connect(m_pBootFrame->switchButton(), &KSwitchButton::stateChanged, this, &Boot::bootSlot);

    connect(m_pResetPasswdBtn, &QPushButton::clicked, this, &Boot::resetPasswdSlot);

    connect(m_pGrubBootDelayFrame->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Boot::changeBootDelay);

    connect(m_pBootListGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &Boot::setDefaultBoot);

    connect(m_pRescueModeFrame->switchButton(), &KSwitchButton::stateChanged, this, &Boot::rescueModeChangedSlot);

    connect(m_pFailuersFrame->comboBox(),  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Boot::failureTimesChangedSlot);
}

bool Boot::authoriyGrub()
{
    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
            "org.control.center.qt.systemdbus.action.grub",
            PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
            PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes){
        return true;
    } else {
        return false;
    }
}

void Boot::initBootList()
{
    char *rawJson = kdk_system_get_grub_menu();
    qDebug() << "origin data list = " << rawJson;

    QJsonDocument doc = QJsonDocument::fromJson(QByteArray(rawJson));
    QJsonObject obj = doc.object();

    // 按键在原始字符串中的位置排序
    QString rawStr(rawJson);
    QMap<int, QPair<QString, QString>> orderedMap;

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        int pos = rawStr.indexOf(it.key());
        orderedMap.insert(pos, {it.key(), it.value().toString()});
    }

    int index = 0;
    for (auto it = orderedMap.begin(); it != orderedMap.end(); ++it) {
        QString name = it.value().first;
        QString type = it.value().second;
        qDebug() << index << type << ":" << name;

        if (type == "menuentry") {
            QWidget *widget = new QWidget(m_pBootOrderFrame);
            QRadioButton *bootItemRadioBtn = new QRadioButton(widget);
            bootItemRadioBtn->setText(name);
            bootItemRadioBtn->setProperty("grubOrder", index);
            bootItemRadioBtn->setAccessibleName(name);
            // 选中变化后把行标题挪到当前项（Qt setBuddy 常无 LABELLED_BY，须写 accessibleName）
            connect(bootItemRadioBtn, &QAbstractButton::toggled, this, [this](bool) {
                refreshDefaultBootRowAccessible();
            });

            QHBoxLayout *layout = new QHBoxLayout(widget);
            layout->setContentsMargins(16, 0, 16, 0);
            layout->addWidget(bootItemRadioBtn);

            m_pBootOrderFrame->addWidget(widget);
            m_pBootListGroup->addButton(bootItemRadioBtn, index);
        }
        index++;
    }
    refreshDefaultBootRowAccessible();
}

void Boot::refreshDefaultBootRowAccessible()
{
    if (!m_pBootListGroup || !m_pDefaultBootLabel) {
        return;
    }
    const QList<QAbstractButton *> buttons = m_pBootListGroup->buttons();
    if (buttons.isEmpty()) {
        return;
    }

    const QString section = tr("Default Startup System");
    m_pDefaultBootLabel->setAccessibleName(section);

    // 只给「当前落点」一项带行标题：优先已选，否则首项；其余只读系统名
    QAbstractButton *focusItem = m_pBootListGroup->checkedButton();
    if (!focusItem) {
        focusItem = buttons.first();
    }

    for (QAbstractButton *btn : buttons) {
        if (!btn) {
            continue;
        }
        const QString osName = btn->text();
        if (btn == focusItem) {
            // Qt radio 的 setBuddy 常不暴露 LABELLED_BY，须把标题写入这一项的 accessibleName
            btn->setAccessibleName(section + QLatin1Char(' ') + osName);
        } else {
            btn->setAccessibleName(osName);
        }
    }
    m_pDefaultBootLabel->setBuddy(focusItem);
}

void Boot::initDefaultBoot()
{
    if (m_pUkccSystemDbus == nullptr || m_pBootListGroup == nullptr) {
        return;
    }

    // 获取到当前系统不一致，启动项列表不进行勾选
    qDebug() << "gsetting key of system-same = " << m_pukccGsetting->get(SYSTEMS_SAME).toBool();
    if (!m_pukccGsetting->get(SYSTEMS_SAME).toBool()) {
        return;
    }

    QDBusReply<int> ret = m_pUkccSystemDbus->call("getDefaultBootItem");
    m_preBootDefault = ret;
    if (!ret.isValid() || ret < 0) {
        return;
    }

    int id = ret.value();

    // 防御：确保 ID 有效且按钮存在
    m_pBootListGroup->blockSignals(true);
    QAbstractButton *btn = m_pBootListGroup->button(id);
    if (btn != nullptr && btn->parent() != nullptr) {
        btn->setChecked(true);
    } else {
        qWarning() << "Boot: invalid button for default boot index" << id;
    }
    m_pBootListGroup->blockSignals(false);
    // blockSignals 期间 toggled 不会刷新读屏名，这里补一次
    refreshDefaultBootRowAccessible();

}

void Boot::initGrubVerifyStatus()
{
    if (m_pUkccSystemDbus != nullptr) {
        QDBusReply<bool> ret = m_pUkccSystemDbus->call("getGrupPasswdStatus");
        m_pBootFrame->switchButton()->blockSignals(true);
        m_pBootFrame->switchButton()->setChecked(ret);
        m_pBootFrame->switchButton()->blockSignals(false);
    }

    m_pResetPasswdBtn->setVisible(m_pBootFrame->switchButton()->isChecked());
}

void Boot::initBootTime()
{
    if (m_pUkccSystemDbus != nullptr) {
        QDBusReply<int> ret = m_pUkccSystemDbus->call("getGrubBootTime");
        m_preBootDelay = ret;
        m_pGrubBootDelayFrame->comboBox()->blockSignals(true);
        switch (ret) {
        case 1:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(0);
            break;
        case 3:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(1);
            break;
        case 5:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(2);
            break;
        case 10:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(3);
            break;
        case -1:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(4);
            break;
        default:
            break;
        }
        m_pGrubBootDelayFrame->comboBox()->blockSignals(false);
    }
}

void Boot::initRescueMode()
{
    QFile file("/etc/default/grub.d/minimalos.cfg");
    if (!file.exists()) {
        m_pRescueModeGroup->hide();
    }

    if (m_pUkccSystemDbus != nullptr) {
        QDBusReply<bool> ret = m_pUkccSystemDbus->call("getRescueModeStatus");
        m_pRescueModeFrame->switchButton()->blockSignals(true);
        m_pRescueModeFrame->switchButton()->setChecked(ret);
        m_pRescueModeFrame->switchButton()->blockSignals(false);

        QDBusReply<int> times =  m_pUkccSystemDbus->call("getFailuresTimes");
        m_preFailureTimes = times;
        qDebug() << "rescue mode = " << ret << "failure times = " << times;

        m_pFailuersFrame->comboBox()->blockSignals(true);
        m_pFailuersFrame->comboBox()->setCurrentIndex((times - 2) >= 0 ? (times - 2) : 0);
        m_pFailuersFrame->comboBox()->blockSignals(false);
    }

    m_pFailuersFrame->setVisible(m_pRescueModeFrame->switchButton()->isChecked());
}

void Boot::rescueModeChangedSlot(bool checked)
{
    if (!authoriyGrub()){
        m_pRescueModeFrame->switchButton()->blockSignals(true);
        m_pRescueModeFrame->switchButton()->setChecked(!checked);
        m_pRescueModeFrame->switchButton()->blockSignals(false);
        return;
    }

    showLoadingWidget(m_pRescueModeFrame->title());
    QString lang = qgetenv("LANG");
    QDBusReply<bool> reply = m_pUkccSystemDbus->call(QDBus::BlockWithGui, "setRescueMode", checked, lang);
    if (reply.isValid()) {
        qDebug() << "reply of setRescueMode = " << reply;
    }
    initRescueMode();

    if (m_pLoadingWidget) {
        m_pLoadingWidget->hide();
        m_pLoadingWidget->close();
    }

    UkccCommon::buriedSettings(name(), m_pRescueModeFrame->title(), QString("settings"), m_pRescueModeFrame->switchButton()->isChecked() ? "true" : "false");
}

void Boot::failureTimesChangedSlot(int index)
{
    if (!authoriyGrub()){
        m_pFailuersFrame->comboBox()->blockSignals(true);
        m_pFailuersFrame->comboBox()->setCurrentIndex((m_preFailureTimes - 2) >= 0 ? (m_preFailureTimes - 2) : 0);
        m_pFailuersFrame->comboBox()->blockSignals(false);
        return;
    }

    // 失败次数 修改 /etc/default/grub.d/minimalos.cfg 中 GRUB_BOOTFAIL_LIMIT,update-grub
    qDebug() << "set failure times = " << m_pFailuersFrame->comboBox()->currentData().toInt() << m_pFailuersFrame->title();
    m_preFailureTimes = m_pFailuersFrame->comboBox()->currentData().toInt();
    UkccCommon::buriedSettings(name(), m_pFailuersFrame->title(), QString("selected"), m_pFailuersFrame->comboBox()->currentData().toString());

    showLoadingWidget(m_pFailuersFrame->title());
    QString lang = qgetenv("LANG");
    QDBusReply<bool> reply = m_pUkccSystemDbus->call(QDBus::BlockWithGui, "setFailuresTimes", m_pFailuersFrame->comboBox()->currentData().toInt(), lang);
    if (reply.isValid()) {
        qDebug() << "reply of setFailuresTimes = " << reply;
    }
    initRescueMode();

    if (m_pLoadingWidget) {
        m_pLoadingWidget->hide();
        m_pLoadingWidget->close();
    }
}

void Boot::bootSlot(bool checked)
{
    if (checked) {
        GrubVerify *dia = new GrubVerify(m_pPluginWidget);
        QPushButton *confirmBtn = dia->getConfirmBtn();
        connect(confirmBtn, &QPushButton::clicked, this, [=](){
            setGrubPasswd(dia->getPwd(),checked);
        });
        if (dia->exec() != QDialog::Accepted) {
            m_pBootFrame->switchButton()->blockSignals(true);
            m_pBootFrame->switchButton()->setChecked(!checked);
            m_pBootFrame->switchButton()->blockSignals(false);
            m_pResetPasswdBtn->setVisible(m_pBootFrame->switchButton()->isChecked());
        }
    } else {
        setGrubPasswd("", checked);
    }

    UkccCommon::buriedSettings(name(), "open grub passwd", QString("settings"), m_pBootFrame->switchButton()->isChecked() ? "true" : "false");
    m_pResetPasswdBtn->setVisible(m_pBootFrame->switchButton()->isChecked());
}

void Boot::setGrubPasswd(QString pwd, bool isOpen)
{
    if (!authoriyGrub()){
        m_pBootFrame->switchButton()->blockSignals(true);
        m_pBootFrame->switchButton()->setChecked(!isOpen);
        m_pBootFrame->switchButton()->blockSignals(false);
        return;
    }

    showLoadingWidget(m_pBootFrame->title());
    QString lang = qgetenv("LANG");
    QDBusReply<bool> reply = m_pUkccSystemDbus->call(QDBus::BlockWithGui, "setGrupPasswd", "root", pwd, lang, isOpen);
    if (reply.isValid() && !reply) {
        qDebug() << "reply of setGrupPasswd = " << reply;
        m_pBootFrame->switchButton()->blockSignals(true);
        m_pBootFrame->switchButton()->setChecked(!isOpen);
        m_pBootFrame->switchButton()->blockSignals(false);
    }
    if (m_pLoadingWidget) {
        m_pLoadingWidget->hide();
        m_pLoadingWidget->close();
    }
}

void Boot::resetPasswdSlot()
{
    GrubVerify *dia =  new GrubVerify(m_pPluginWidget);
    QPushButton *confirmBtn = dia->getConfirmBtn();
    connect(confirmBtn, &QPushButton::clicked, this, [=](){
        if (!authoriyGrub()){
            return;
        }

        UkccCommon::buriedSettings(name(), m_pResetPasswdBtn->text(), QString("clicked"), m_pBootFrame->title());

        showLoadingWidget(m_pBootFrame->title());
        QString lang = qgetenv("LANG");
        m_pUkccSystemDbus->call(QDBus::BlockWithGui, "setGrupPasswd", "root", dia->getPwd(), lang, true);

        if (m_pLoadingWidget) {
            m_pLoadingWidget->hide();
            m_pLoadingWidget->close();
        }
    });
    dia->exec();
}

void Boot::changeBootDelay(int index)
{
    if (!authoriyGrub()){
        m_pGrubBootDelayFrame->comboBox()->blockSignals(true);
        switch (m_preBootDelay) {
        case 1:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(0);
            break;
        case 3:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(1);
            break;
        case 5:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(2);
            break;
        case 10:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(3);
            break;
        case -1:
            m_pGrubBootDelayFrame->comboBox()->setCurrentIndex(4);
            break;
        default:
            break;
        }
        m_pGrubBootDelayFrame->comboBox()->blockSignals(false);
        return;
    }

    // 启动延时 修改/etc/default/grub中GRUB_TIMEOUT,update-grub
    qDebug() << "set boot delay = " << m_pGrubBootDelayFrame->comboBox()->currentData().toInt();
    m_preBootDelay = m_pGrubBootDelayFrame->comboBox()->currentData().toInt();

    UkccCommon::buriedSettings(name(), m_pGrubBootDelayFrame->title(), QString("selected"), QString::number(m_pGrubBootDelayFrame->comboBox()->currentData().toInt()));

    showLoadingWidget(m_pGrubBootDelayFrame->title());

    QString lang = qgetenv("LANG");
    QDBusReply<bool> reply = m_pUkccSystemDbus->call(QDBus::BlockWithGui, "setGrupBootTime", m_pGrubBootDelayFrame->comboBox()->currentData().toInt(), lang);
    if (reply.isValid() && !reply) {
        qDebug() << "reply of setGrupBootTime = " << reply;
        initBootTime();
    }

    if (m_pLoadingWidget) {
        m_pLoadingWidget->hide();
        m_pLoadingWidget->close();
    }
}

void Boot::setDefaultBoot(QAbstractButton *button)
{
    int order = button->property("grubOrder").toInt();

    // 获取到当前系统一致，设置默认启动项时不弹授权
    if (m_pukccGsetting->get(SYSTEMS_SAME).toBool()) {
        // 获取到当前系统一致并点击启动项为当前启动项，不弹授权
        QDBusReply<int> ret = m_pUkccSystemDbus->call("getDefaultBootItem");
        if (ret == order) {
            return;
        }
    }

    if (!authoriyGrub()){
        m_pBootListGroup->blockSignals(true);
        if (m_pBootListGroup->button(m_preBootDefault) != nullptr) {
            m_pBootListGroup->button(m_preBootDefault)->setChecked(true);
        }
        m_pBootListGroup->blockSignals(false);
        refreshDefaultBootRowAccessible();
        return;
    }

    qDebug() << "set default boot = " << order << button->text();
    m_preBootDefault = order;

    UkccCommon::buriedSettings(name(), m_pDefaultBootLabel->text(), QString("selected"), button->text());

    showLoadingWidget(m_pDefaultBootLabel->text());

    QString lang = qgetenv("LANG");
    QDBusReply<bool> reply = m_pUkccSystemDbus->call(QDBus::BlockWithGui, "setDefaultBootItem", order, lang);
    if (reply.isValid()) {
        qDebug() << "reply of setGrupBootTime = " << reply;
        if (!reply) {
            initDefaultBoot();
        } else {
           if (!m_pukccGsetting->get(SYSTEMS_SAME).toBool()) {
               m_pukccGsetting->set(SYSTEMS_SAME, true);
           }
        }
    }

    if (m_pLoadingWidget) {
        m_pLoadingWidget->hide();
        m_pLoadingWidget->close();
    }
}

void Boot::showLoadingWidget(QString text)
{
    if (m_pLoadingWidget == nullptr) {
        QWidget *parentWidget = m_pPluginWidget->topLevelWidget();
        m_pLoadingWidget = new LoadingWidget(parentWidget, 0.4);
        m_pLoadingWidget->setGeometry(0, 0, parentWidget->geometry().width(), parentWidget->geometry().height());
    }

    m_pLoadingWidget->setLoadingText(text);
    m_pLoadingWidget->setLoading();
    m_pLoadingWidget->show();

}
