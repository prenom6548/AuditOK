/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "displaymodewidget.h"
#include "radiobuttonitem.h"
#include "glesversionwidget.h"
#include "global.h"

//#include <QDBusInterface>
//#include <QDBusPendingReply>
//#include <QJsonObject>
//#include <QJsonDocument>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QButtonGroup>
#include <QStandardPaths>
#include <QDBusInterface>
#include <QDBusReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdlib.h>
#include <sys/syslog.h>

//namespace {

//static bool fbMatched(const char* fbName)
//{
//    FILE* fp = NULL;
//    char line[256] = {0};
//    bool matched = false;

//    fp = fopen("/proc/fb", "r");
//    if (!fp) {
//        fprintf(stderr, "GPU detection open /proc/fb error!");
//        return false;
//    }
//    while (fgets(line, sizeof(line), fp) != NULL) {
//        if (strcasestr(line, fbName)) {
//            matched = true;
//            break;
//        }
//    }

//    fclose(fp);

//    return matched;
//}

//bool isAMDGraphicCard()
//{
//    return (fbMatched("amdgpudrmfb") || fbMatched("radeondrmfb"));
//}

//}

using namespace Global;
DisplayModeWidget::DisplayModeWidget(const QString &displayType, const QString &gpuVendor, QWidget * parent)
    : SettingScrollContent(parent)
    , m_currentDisplayType(displayType)
    , m_rebootBtn(new QPushButton)
    , m_radiosWidget(new QWidget())
    , m_btnGroup(new QButtonGroup)
{
//    m_modeMap.insert(tr("Performance"), tr("Performance mode, only AMD graphics card is supported"));
//    m_modeMap.insert(tr("Compatibility"), tr("Compatible mode, supports all graphics cards"));
    this->initInfo();
    this->m_title->setText(tr("Display"));
    m_nameList << tr("Performance") << tr("Compatibility");
    m_descList << tr("(Supports AMD and Intel graphics cards)") << tr("(Supports all graphics cards)");

    QLabel *m_title = new QLabel;
    m_title->setText(tr("Display"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    const QString &configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
    if (!QDir(configPath).exists("kmre")) {
        QDir(configPath).mkdir("kmre");
    }

    m_confName = QDir::homePath() + "/.config/kmre/kmre.ini";
    m_qsettings = new QSettings(m_confName, QSettings::IniFormat, this);
    m_qsettings->setIniCodec("UTF-8");

    //setTitleTip(tr("Display mode(requires restart)"));

    QFrame *centralWidget = new QFrame;
    QFrame *displayWidget = new QFrame;
    GlesVersionWidget *gleWidget = new GlesVersionWidget(gpuVendor);

    m_rebootBtn->setText(tr("Restart system"));
    m_rebootBtn->setFixedSize(120, 36);
    m_rebootBtn->setFocusPolicy(Qt::NoFocus);
    m_rebootBtn->setEnabled(false);
    m_rebootBtn->setChecked(false);

    m_radiosWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    m_radiosWidget->setStyleSheet("QWidget{border-radius:6px;background:palette(Window);}");
    m_radiosWidget->setStyleSheet("QWidget{border-radius:6px;}");
    m_radiosWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_radiosWidget->setMinimumHeight(100);

    this->initRadioBtns();

//    QHBoxLayout *hlayout = new QHBoxLayout;
//    hlayout->setMargin(0);
//    hlayout->setSpacing(0);
////    hlayout->addStretch();
//    hlayout->addWidget(m_rebootBtn, 0, Qt::AlignLeft);
//    hlayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *restart = new QLabel;
    restart->setText(tr("The modification takes effect after you restart the system"));
    QLabel *m_displaytitle = new QLabel;
    m_displaytitle->setText(tr("Display mode"));
    m_displaytitle->setStyleSheet("QLabel{color:#A1A1A1;}");
    layout->setMargin(0);
    layout->addSpacing(16);
    layout->addWidget(m_displaytitle);
//    layout->setMargin(10);
//    layout->setSpacing(24);
    layout->addWidget(m_radiosWidget);
//    layout->addSpacing(24);
//    layout->addWidget(restart);
//    layout->addWidget(m_rebootBtn, 0, Qt::AlignLeft);
//    layout->addStretch();
    displayWidget->setLayout(layout);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setSpacing(0);
    mainlayout->setMargin(0);
//    mainlayout->setContentsMargins(30,2,30,40);
//    mainlayout->addWidget(m_title);
    mainlayout->addWidget(displayWidget);
    mainlayout->addWidget(gleWidget);
    mainlayout->addStretch();

    connect(gleWidget, &GlesVersionWidget::restartEnv, this, [=]() {emit restartEnv();});
    if (!m_gpuVendor.isNull() && !m_gpuVendor.isEmpty() && (m_gpuVendor == "NVIDIA" || m_gpuVendor == "AMD" || m_gpuVendor == "INTEL")) {
        if (isAMDGraphicCard() || m_gpuVendor == "INTEL") {
            if (m_displayTypes != "emugl,drm") {
                displayWidget->setVisible(false);
            }
            if (m_displayType == "drm") {
                gleWidget->setVisible(false);
            }
        }
        else {
            displayWidget->setVisible(false);
            if (m_displayType == "drm") {
                gleWidget->setVisible(false);
            }
        }
    }
    else {
        gleWidget->setVisible(false);
        if (m_displayTypes != "emugl,drm") {
            displayWidget->setVisible(false);
        }
    }
    centralWidget->setLayout(mainlayout);
    this->setContent(centralWidget);

//    connect(m_rebootBtn, &QPushButton::clicked, this, [=]() {
//    });

//    this->enablePage();
    this->initCurrentValue();

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(m_btnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked), [=](QAbstractButton * button) {
#else
    connect(m_btnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton *button) {
#endif
        if (button->isChecked()) {
        }
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(m_btnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [=](int id) {
#else
    connect(m_btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), [=](int id) {
#endif
        for (int i=0; i<m_itemList.length(); i++) {
            if (i == id) {
                m_itemList.at(i)->setConfirmButtonVisible(true);
            }
            else {
                m_itemList.at(i)->setConfirmButtonVisible(false);
            }
        }
    });
}

DisplayModeWidget::~DisplayModeWidget()
{
    for (RadioButtonItem *item :m_itemList) {
        if (item) {
            item->deleteLater();
        }
    }

    if (m_qsettings) {
        delete m_qsettings;
        m_qsettings = nullptr;
    }
}

//void DisplayModeWidget::enablePage()
//{
//#if defined(__aarch64__)
//    if (isAMDGraphicCard()) {
//        this->initCurrentValue();
//        this->setDisabled(false);
//    }
//    else {
//        this->setDisabled(true);
//        if (m_itemList.length() == 2) {
//            RadioButtonItem *item = m_itemList.at(1);
//            if (item) {
//                item->setChecked();
//            }
//        }
//    }
//#else
//    this->setDisabled(true);
//    if (m_itemList.length() == 2) {
//        RadioButtonItem *item = m_itemList.at(1);
//        if (item) {
//            item->setChecked();
//        }
//    }
//#endif
//}

void DisplayModeWidget::initRadioBtns()
{
    if (m_nameList.length() == m_descList.length()) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(0);

        for (int i=0; i<m_nameList.length();i++) {
            RadioButtonItem *item = new RadioButtonItem(m_btnGroup, i, nullptr);
            layout->addWidget(item);
            connect(item, &RadioButtonItem::radioBtnClicked, this, &DisplayModeWidget::onRadioButtonClicked);
            connect(item, &RadioButtonItem::restartEnv, this, [=]() {emit restartEnv();});
            item->setTitle(m_nameList.at(i), m_descList.at(i));
            m_itemList.append(item);

        }
        m_radiosWidget->setLayout(layout);
    }

    /*
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(10);
    int i = 0;
    for (QMap<QString,QString>::ConstIterator it=m_modeMap.constBegin(); it!=m_modeMap.constEnd(); ++it) {//for (auto it(m_modeMap.begin()); it != m_modeMap.end(); ++it)
        RadioButtonItem *item = new RadioButtonItem(m_btnGroup, i, nullptr);
        layout->addWidget(item);
        connect(item, &RadioButtonItem::radioBtnClicked, this, &DisplayModeWidget::onRadioButtonClicked);
        item->setTitle(it.key(), it.value());
        m_itemList.append(item);
        i++;
    }
    m_radiosWidget->setLayout(layout);
    */
}

void DisplayModeWidget::initCurrentValue()
{
    if (!m_currentDisplayType.isNull() && !m_currentDisplayType.isEmpty()) {
        if (m_currentDisplayType.toLower() == "drm") {
            if (m_itemList.length() == 2) {
                RadioButtonItem *item = m_itemList.at(0);
                if (item) {
                    item->setChecked();
                }
            }
        }
        else if (m_currentDisplayType.toLower() == "emugl") {
            if (m_itemList.length() == 2) {
                RadioButtonItem *item = m_itemList.at(1);
                if (item) {
                    item->setChecked();
                }
            }
        }
    }
    else {
        syslog(LOG_ERR, "Display_Type is empty!");
    }

//    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
//    QDBusPendingReply<QString> reply = interface.asyncCall("getDisplayInformation");
//    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
//    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
//        if (w->isError()) {
//            return;
//        }

//        QDBusPendingReply<QString> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
//        if (reply.isValid()) {
//            QString displayInfo = reply.value();
//            if (!displayInfo.isEmpty()) {
//                QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
//                if (!jsonDocument.isNull()) {
//                    QJsonObject jsonObject = jsonDocument.object();
//                    if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
//                        if (jsonObject.contains("display_type")) {
//                            m_currentDisplayType = jsonObject.value("display_type").toString();
//                            if (!m_currentDisplayType.isNull() && !m_currentDisplayType.isEmpty()) {
//                                if (m_currentDisplayType.toLower() == "drm") {
//                                    if (m_itemList.length() == 2) {
//                                        RadioButtonItem *item = m_itemList.at(0);
//                                        if (item) {
//                                            item->setChecked();
//                                        }
//                                    }
//                                }
//                                else if (m_currentDisplayType.toLower() == "emugl") {
//                                    if (m_itemList.length() == 2) {
//                                        RadioButtonItem *item = m_itemList.at(1);
//                                        if (item) {
//                                            item->setChecked();
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        w->deleteLater();
//    });
}

void DisplayModeWidget::initInfo()
{
    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    QString value;
    QDBusMessage response = interface.call("getDisplayInformation");
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
    }
    QString displayInfo = value;
    if (!displayInfo.isEmpty()) {
        QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
        if (!jsonDocument.isNull()) {
            QJsonObject jsonObject = jsonDocument.object();
            if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                if (jsonObject.contains("display_type")) {
                    m_displayType = jsonObject.value("display_type").toString();
                }
                if (jsonObject.contains("display_types")) {
                    m_displayTypes = jsonObject.value("display_types").toString();
                }
                if (jsonObject.contains("cpu_type")) {
                    m_cpuType = jsonObject.value("cpu_type").toString();
                }
                if (jsonObject.contains("gpu_vendor")) {
                    m_gpuVendor = jsonObject.value("gpu_vendor").toString();
                }
                if (jsonObject.contains("gpu_model")) {
                    m_gpuModel = jsonObject.value("gpu_model").toString();
                }
            }
        }
    }
}

void DisplayModeWidget::saveMode(const QString &mode)
{
    m_qsettings->beginGroup("display");
    m_qsettings->setValue("display_type", mode);
    m_qsettings->endGroup();
    m_qsettings->sync();

    m_currentDisplayType = mode;
}

QString DisplayModeWidget::loadMode()
{
    m_qsettings->beginGroup("display");
    QString mode = m_qsettings->value("display_type", "").toString();
    m_qsettings->endGroup();

    m_currentDisplayType = mode;

    return mode;
}

static bool fbMatched(const char* fbName)
{
    FILE* fp = NULL;
    char line[256] = {0};
    bool matched = false;

    fp = fopen("/proc/fb", "r");
    if (!fp) {
        fprintf(stderr, "GPU detection open /proc/fb error!");
        return false;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strcasestr(line, fbName)) {
            matched = true;
            break;
        }
    }

    fclose(fp);

    return matched;
}
bool DisplayModeWidget::isAMDGraphicCard()
{
    return (fbMatched("amdgpudrmfb") || fbMatched("radeondrmfb"));
}

void DisplayModeWidget::onRadioButtonClicked(int index)
{
    m_rebootBtn->setEnabled(true);
    m_rebootBtn->setChecked(true);
    if (index == 0) {
        this->saveMode("drm");
    }
    else {
        this->saveMode("emugl");
    }
}

