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

#include "camerawidget.h"
#include "preferences.h"

#include <QComboBox>
#include <QBoxLayout>
#include <QCameraInfo>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDBusInterface>

CameraWidget::CameraWidget(QWidget * parent)
    : SettingScrollContent(parent)
    , m_pref(new Preferences())
{
    //setTitleTip(tr("Camera Devices"));

    QFrame *centralWidget = new QFrame;

    m_cameraLabel = new QLabel(this);
    m_cameraComboBox = new QComboBox(this);

    m_cameraLabel->setText(tr("Camera device:"));
    m_cameraComboBox->setFixedSize(380, 32);
//    m_cameraComboBox->addItem(tr("No camera detected"));
//    m_cameraComboBox->setCurrentText(tr("No camera detected"));
    m_cameraComboBox->setDuplicatesEnabled(false);


    QFrame *frame = new QFrame;
    frame->setStyleSheet("QFrame{border-radius:6px;}");
    frame->setAttribute(Qt::WA_TranslucentBackground, true);

    QHBoxLayout *hlayout = new QHBoxLayout;
    //hlayout->setMargin(10);
    //hlayout->addSpacing(20);
    hlayout->addWidget(m_cameraLabel);
    hlayout->addSpacing(32);
    hlayout->addWidget(m_cameraComboBox);
    hlayout->addStretch();
    frame->setLayout(hlayout);

    QVBoxLayout *layout =new QVBoxLayout();
    QLabel *m_title = new QLabel;
    m_title->setText(tr("Camera Devices"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    layout->setContentsMargins(30,2,39,40);
    layout->addWidget(m_title);
    //layout->setSpacing(10);
    //layout->setMargin(10);
    layout->addWidget(frame);

    centralWidget->setLayout(layout);
    this->setContent(centralWidget);

    watcher = new QFileSystemWatcher;
    watcher->addPath("/dev/");
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, [=] (const QString &path) {
        Q_UNUSED(path)
        QTimer::singleShot(1000, this, SLOT(updateCameraDevices()));//1s后去获取，否则无法获取到正确的设备列表
    });

    connect(m_cameraComboBox, &QComboBox::currentTextChanged, this, [=]() {
        QString devname =  m_cameraMap[m_cameraComboBox->currentText()];
        m_pref->m_cameraDeviceName = devname;
        m_pref->updateCameraConfig();
        if (!devname.isEmpty() && !devname.isNull()) {
            QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
            interface.call("setCameraDevice", devname);
        }

    });
}

CameraWidget::~CameraWidget()
{
    if (m_pref) {
        delete m_pref;
        m_pref = nullptr;
    }
}

void CameraWidget::updateCurrentCameraDevice(const QString &deviceName)
{
    m_pref->m_cameraDeviceName = deviceName;
    m_pref->updateCameraConfig();

    if (!m_pref->m_cameraDeviceName.isEmpty() && !m_pref->m_cameraDeviceName.isNull()) {
        QList<QString> values = m_cameraMap.values();
        for (int i = 0; i < values.count(); i++) {
            if (values.at(i) == m_pref->m_cameraDeviceName) {
                m_cameraComboBox->setCurrentText(m_cameraMap.key(m_pref->m_cameraDeviceName));
                m_cameraComboBox->setToolTip(m_cameraMap.key(m_pref->m_cameraDeviceName));
            }
        }
    }
}

void CameraWidget::updateCameraDevices()
{
    bool selectedDeviceExists = false;
    m_cameraComboBox->blockSignals(true);
    m_cameraComboBox->clear();
    //m_cameraComboBox->setDuplicatesEnabled(1);
    m_cameraMap.clear();

    QString nocamera = tr("No camera detected");
    m_cameraComboBox->addItem(nocamera);
    m_cameraComboBox->setCurrentText(nocamera);
    m_cameraComboBox->setEnabled(false);

    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        QString nameTag;
        nameTag = QString("%1 (%2)").arg(cameraInfo.description()).arg(cameraInfo.deviceName());

        m_cameraMap[nameTag] = cameraInfo.deviceName();
        m_cameraComboBox->addItem(nameTag);

        if (!m_pref->m_cameraDeviceName.isEmpty() && !m_pref->m_cameraDeviceName.isNull()) {
            if (nameTag.contains(m_pref->m_cameraDeviceName)) {
                m_cameraComboBox->removeItem(0);
                m_cameraComboBox->setEnabled(true);
                m_cameraComboBox->setCurrentText(nameTag);
                m_cameraComboBox->setToolTip(nameTag);
                selectedDeviceExists = true;
            }
        }
    }

    //之前选择的摄像头设备已经不存在了
    if (!selectedDeviceExists && (m_cameraMap.count() > 0)) {
        QString name = m_cameraMap.keys().first();
        QString deviceName = m_cameraMap.value(name);
        m_pref->m_cameraDeviceName = deviceName;
        m_pref->updateCameraConfig();
        m_cameraComboBox->setCurrentText(m_cameraMap.key(m_pref->m_cameraDeviceName));
        m_cameraComboBox->setToolTip(m_cameraMap.key(m_pref->m_cameraDeviceName));
    }

    m_cameraComboBox->blockSignals(false);
}
