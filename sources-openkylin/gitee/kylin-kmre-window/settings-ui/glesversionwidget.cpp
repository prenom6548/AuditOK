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

#include "glesversionwidget.h"
#include "settingsitem.h"
#include "radiobuttonitem.h"
#include "global.h"

#ifdef UKUI_WAYLAND
#include "messagebox.h"
using namespace KylinUI;
#endif

#include <QVBoxLayout>
#include <QMessageBox>
#include <QButtonGroup>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <sys/syslog.h>


using namespace Global;
GlesVersionWidget::GlesVersionWidget(const QString &gpuVendor, QWidget * parent)
    : SettingScrollContent(parent)
    , m_rebootBtn(new QPushButton)
    , m_radiosWidget(new QWidget())
    , m_btnGroup(new QButtonGroup)
{
    m_descLists.append("");
    m_descLists.append(tr("(Compatibility)"));
    m_descLists.append("");
    m_descLists.append(tr("(Renderer maximum)"));

    m_glesLists.append(QString(tr("Autoselect")));
    m_glesLists.append(QString(tr("OpenGL ES 2.0")));//兼容性
    m_glesLists.append(QString(tr("OpenGL ES 3.0")));
    m_glesLists.append(QString(tr("OpenGL ES 3.1")));//渲染器最大值

//    m_descLists.append(QString(tr("Autoselect")));
//    m_descLists.append(QString(tr("OpenGL ES 2.0(Compatibility)")));//兼容性
//    m_descLists.append(QString(tr("OpenGL ES 3.0")));
//    m_descLists.append(QString(tr("OpenGL ES 3.1(Renderer maximum)")));//渲染器最大值

//    m_glesLists.append(QString(tr("Default")));
//    m_glesLists.append("2.0");
//    m_glesLists.append("3.0");
//    m_glesLists.append("3.1");
    //TODO: 3.2

    const QString &configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
    if (!QDir(configPath).exists("kmre")) {
        QDir(configPath).mkdir("kmre");
    }
    m_confName = QDir::homePath() + "/.config/kmre/render_gles";
    m_qsettings = new QSettings(m_confName, QSettings::IniFormat, this);
    m_qsettings->setIniCodec("UTF-8");

    //setTitleTip(tr("OpenGL ES API level(requires restart)"));

    m_rebootBtn->setText(tr("Restart system"));
    m_rebootBtn->setFixedSize(120, 36);
    m_rebootBtn->setFocusPolicy(Qt::NoFocus);
    m_rebootBtn->setEnabled(false);
    m_rebootBtn->setChecked(false);

    QFrame *centralWidget = new QFrame;
    centralWidget->setWindowFlags(Qt::FramelessWindowHint);

    m_radiosWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_radiosWidget->setStyleSheet("QWidget{border-radius:6px;}");
    m_radiosWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_radiosWidget->setMinimumHeight(100);

    this->initRadioBtns();
    this->setContentsMargins(0,0,0,0);

    QHBoxLayout *hlayout = new QHBoxLayout;

    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    //hlayout->addStretch();
    hlayout->addWidget(m_rebootBtn, 0, Qt::AlignLeft);
    hlayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *restart = new QLabel;
    restart->setText(tr("The modification takes effect after you restart the system"));
    QLabel *m_title = new QLabel;
    m_title->setText(tr("OpenGL ES API level(requires restart)"));
    m_title->setStyleSheet("QLabel{color:#A1A1A1;}");
//    layout->setContentsMargins(30,2,39,40);
    layout->setMargin(0);
    layout->addSpacing(0);
    layout->addWidget(m_title);
//    layout->setSpacing(24);
    layout->addSpacing(10);
    layout->addWidget(m_radiosWidget);
    layout->addSpacing(10);
//    layout->addWidget(restart);
//    layout->addLayout(hlayout);
    layout->addStretch();

    centralWidget->setLayout(layout);
    this->setContent(centralWidget);


//    connect(m_rebootBtn, &QPushButton::clicked, this, [=]() {
//    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(m_btnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked), [=](QAbstractButton * button) {
#else
    connect(m_btnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton *button) {
#endif
//        if (button->isChecked()) {
//        }
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

//    this->enablePage();
    if (!gpuVendor.isNull() && !gpuVendor.isEmpty() && (gpuVendor == "NVIDIA" || gpuVendor == "AMD" || gpuVendor == "INTEL")) {
        if (m_itemList.length() == 4) {
            // read default gles version
            QString gles = this->loadGlesVersion();
            if (gles.isEmpty() || gles.isNull()) {
                if (m_itemList.length() > 0) {
                    RadioButtonItem *item = m_itemList.at(0);
                    if (item) {
                        item->setChecked();
                    }
                }
            }
            else {
                if (gles == "2.0") {
                    RadioButtonItem *item = m_itemList.at(1);
                    if (item) {
                        item->setChecked();
                    }
                }
                else if (gles == "3.0") {
                    RadioButtonItem *item = m_itemList.at(2);
                    if (item) {
                        item->setChecked();
                    }
                }
                else if (gles == "3.1") {
                    RadioButtonItem *item = m_itemList.at(3);
                    if (item) {
                        item->setChecked();
                    }
                }
            }
        }
    }
}

GlesVersionWidget::~GlesVersionWidget()
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

void GlesVersionWidget::initRadioBtns()
{
    if (m_glesLists.length() == m_descLists.length()) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(10);

        for (int i=0; i<m_glesLists.length();i++) {
            RadioButtonItem *item = new RadioButtonItem(m_btnGroup, i, nullptr);
            layout->addWidget(item);
//            QObject::connect(m_display,&DisplayModeWidget::emugl_gle,this,[=](){
//                syslog(LOG_DEBUG,"####YANGCHEN emugl_gle");
//                item->setEnabled(true);
//            });
//            QObject::connect(m_display,&DisplayModeWidget::drm_gle,this,[=](){
//                syslog(LOG_DEBUG,"####YANGCHEN drm_gle");
//               item->setEnabled(false);
//            });
            connect(item, &RadioButtonItem::radioBtnClicked, this, &GlesVersionWidget::onRadioButtonClicked);
            connect(item, &RadioButtonItem::restartEnv, this, [=]() {emit restartEnv();});
            item->setTitle(m_glesLists.at(i), m_descLists.at(i));
            m_itemList.append(item);

        }
        m_radiosWidget->setLayout(layout);
    }
}

void GlesVersionWidget::saveGlesVersion(const QString &version)
{
    m_qsettings->beginGroup("gles");
    m_qsettings->setValue("version", version);
    m_qsettings->endGroup();
    m_qsettings->sync();

    m_defaultGles = version;
}

QString GlesVersionWidget::loadGlesVersion()
{
    m_qsettings->beginGroup("gles");
    QString version = m_qsettings->value("version").toString();
    m_qsettings->endGroup();

    m_defaultGles = version;

    return version;
}

void GlesVersionWidget::onRadioButtonClicked(int index)
{
    m_rebootBtn->setEnabled(true);
    m_rebootBtn->setChecked(true);
    if (index == 1) {
        this->saveGlesVersion("2.0");
    }
    else if (index == 2) {
        this->saveGlesVersion("3.0");
    }
    else if (index == 3) {
        this->saveGlesVersion("3.1");
    }
    else {
        this->saveGlesVersion("");
    }
}
