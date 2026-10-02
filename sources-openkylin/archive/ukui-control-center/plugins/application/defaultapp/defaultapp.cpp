/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include <QWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QApplication>
#include <KSycoca>
#include <KApplicationTrader>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDBusArgument>

#include "defaultapp.h"
#include "ukcccommon.h"
using namespace ukcc;

#define BROWSERTYPE "x-scheme-handler/http"
#define MAILTYPE    "x-scheme-handler/mailto"
#define IMAGETYPE   "image/png"
#define AUDIOTYPE   "audio/x-vorbis+ogg"
#define VIDEOTYPE   "video/mp4"
#define TEXTTYPE    "text/plain"

#define UKCC_EXENAME "ukui-control-center"

#define LOCAL_CONFIG_DIR "/.config/"

DefaultApp::DefaultApp() : mFirstLoad(true)
{
    pluginName = tr("Default App");
    pluginType = APPLICATION;
}

DefaultApp::~DefaultApp() {
    if (!mFirstLoad) {

    }
}

QString DefaultApp::plugini18nName() {
    return pluginName;
}

int DefaultApp::pluginTypes() {
    return pluginType;
}

void DefaultApp::preInitialize() {
    // BROWSER
    loadData(&browserService, BROWSERTYPE);

    // IMAGE
    loadData(&imageService, IMAGETYPE);

    // MAIL
    loadData(&mailService, MAILTYPE);

    // AUDIO
    loadData(&audioService, AUDIOTYPE);

    // VIDEO
    loadData(&videoService, VIDEOTYPE);

    // TEXT
    loadData(&textService, TEXTTYPE);
}

QWidget *DefaultApp::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        if (mDefaultUkccDbus == nullptr) {
            mDefaultUkccDbus = new QDBusInterface("org.ukui.ukcc.session",
                                                  "/Default",
                                                  "org.ukui.ukcc.session.Default",
                                                  QDBusConnection::sessionBus(), this);
        }
        if (!mDefaultUkccDbus->isValid()) {
            qCritical() << "org.ukui.ukcc.session.Default DBus error:" << mDefaultUkccDbus->lastError();
        }

        preInitialize();

        mDefaultWidget = new DefaultAppUi;
        setupCompenent();
        initDefaultUI();
        connectToServer();
        initSearchText();
        initSlots();
    }
    return mDefaultWidget;
}

const QString DefaultApp::name() const {

    return QStringLiteral("Defaultapp");
}

bool DefaultApp::isShowOnHomePage() const
{
    return true;
}

QIcon DefaultApp::icon() const
{
    return QIcon::fromTheme("ukui-defaultapp-symbolic");
}

bool DefaultApp::isEnable() const
{
    return true;
}

/* 建立对应的信号槽 */
void DefaultApp::initSlots() {
    connect(mDefaultWidget->getBrowserWidget(), &ComboxWidget::currentIndexChanged, this, &DefaultApp::browserComBoBox_changed_cb);
    connect(mDefaultWidget->getMailWidget(), &ComboxWidget::currentIndexChanged, this, &DefaultApp::mailComBoBox_changed_cb);
    connect(mDefaultWidget->getImageWidget(), &ComboxWidget::currentIndexChanged, this, &DefaultApp::imageComBoBox_changed_cb);
    connect(mDefaultWidget->getAudioWidget(), &ComboxWidget::currentIndexChanged, this, &DefaultApp::audioComBoBox_changed_cb);
    connect(mDefaultWidget->getVideoWidget(), &ComboxWidget::currentIndexChanged, this, &DefaultApp::videoComBoBox_changed_cb);
    connect(mDefaultWidget->getTextWidget(), &ComboxWidget::currentIndexChanged, this, &DefaultApp::textComBoBox_changed_cb);
    connect(mDefaultWidget->getResetWidget(), &PushButtonWidget::clicked, this, &DefaultApp::reset);
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Default",
                                          "org.ukui.ukcc.session.Default",
                                          "defaultChanged",
                                          this,
                                          SLOT(defaultChangedSlot()));
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Default",
                                          "org.ukui.ukcc.session.Default",
                                          "defaultMimeTypeChanged",
                                          this,
                                          SLOT(defaultMimeTypeChangedSlot(const QString&, const QString&)));
    connect(KSycoca::self(), static_cast<void(KSycoca::*)()>(&KSycoca::databaseChanged), this, [=]() {
        kSycocaChangedSlot();
    });
}

void DefaultApp::kSycocaChangedSlot() {
    if (qAppName() == QLatin1String(UKCC_EXENAME))
        return;
    // 删除之后，在获取时更新缓存配置文件
    QFile::remove(KSycoca::absoluteFilePath());
    KSycoca::self()->ensureCacheValid();
    preInitialize();
    initDefaultUI();
}

/* 初始化各默认应用 */
void DefaultApp::initDefaultUI() {
    // BROWSER
    load(mDefaultWidget->getBrowserWidget(), &browserService, BROWSERTYPE);
    // IMAGE
    load(mDefaultWidget->getImageWidget(), &imageService, IMAGETYPE);

    // MAIL
    load(mDefaultWidget->getMailWidget(), &mailService, MAILTYPE);

    // AUDIO
    load(mDefaultWidget->getAudioWidget(), &audioService, AUDIOTYPE);

    // VIDEO
    load(mDefaultWidget->getVideoWidget(), &videoService, VIDEOTYPE);

    // TEXT
    load(mDefaultWidget->getTextWidget(), &textService, TEXTTYPE);
}

void DefaultApp::setupCompenent()
{
    mDefaultString = tr("No program available");
    mSelectString = tr("Choose default app");
    mLocalMimefile = QDir::homePath() + LOCAL_CONFIG_DIR + "mimeapps.list";
}

/* 添加搜索索引 */
void DefaultApp::initSearchText() {
    //~ contents_path /Defaultapp/Browser
    //~ contents_path /Defaultapp/Mail
    //~ contents_path /Defaultapp/Image Viewer
    //~ contents_path /Defaultapp/Audio Player
    //~ contents_path /Defaultapp/Video Player
    //~ contents_path /Defaultapp/Text Editor
}

///* BROWSER SLOT */
void DefaultApp::browserComBoBox_changed_cb(int index) {
   QtConcurrent::run([=] {
        QTime timedebuge;
        timedebuge.start();
        QString appid = mDefaultWidget->getBrowserWidget()->comboBox()->itemData(index).toString();
        mDefaultUkccDbus->call("setDefaultApp", appid, BROWSERTYPE);
        findSelectItem(mDefaultWidget->getBrowserWidget()->comboBox());
        UkccCommon::buriedSettings(name(), "the default browser", QString("settings"), appid);
        qDebug()<<"browserComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";
    });
}

/* MAIL SLOT */
void DefaultApp::mailComBoBox_changed_cb(int index) {
    QtConcurrent::run([=] {
        QTime timedebuge;
        timedebuge.start();
        QString appid = mDefaultWidget->getMailWidget()->comboBox()->itemData(index).toString();
        mDefaultUkccDbus->call("setDefaultApp", appid, MAILTYPE);
        findSelectItem(mDefaultWidget->getMailWidget()->comboBox());
        UkccCommon::buriedSettings(name(), "the defaultapp to open mail", QString("settings"), appid);
        qDebug()<<"mailComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";
    });
}

/* IMAGE SLOT */
void DefaultApp::imageComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;
        timedebuge.start();
        QString appid = mDefaultWidget->getImageWidget()->comboBox()->itemData(index).toString();
        mDefaultUkccDbus->call("setDefaultApp", appid, IMAGETYPE);
        findSelectItem(mDefaultWidget->getImageWidget()->comboBox());
        UkccCommon::buriedSettings(name(), "the defaultapp to open image", QString("settings"), appid);
        qDebug()<<"imageComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";
    });
}

/* AUDIO SLOT */
void DefaultApp::audioComBoBox_changed_cb(int  index) {

    QtConcurrent::run([=] {
        QTime timedebuge;
        timedebuge.start();
        QString appid = mDefaultWidget->getAudioWidget()->comboBox()->itemData(index).toString();
        mDefaultUkccDbus->call("setDefaultApp", appid, AUDIOTYPE);
        findSelectItem(mDefaultWidget->getAudioWidget()->comboBox());
        UkccCommon::buriedSettings(name(), "the defaultapp to play audio", QString("settings"), appid);
        qDebug()<<"audioComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";
    });
}

/* VIDEO SLOT */
void DefaultApp::videoComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;
        timedebuge.start();
        QString appid = mDefaultWidget->getVideoWidget()->comboBox()->itemData(index).toString();
        mDefaultUkccDbus->call("setDefaultApp", appid, VIDEOTYPE);
        findSelectItem(mDefaultWidget->getVideoWidget()->comboBox());
        UkccCommon::buriedSettings(name(), "the defaultapp to play video", QString("settings"), appid);
        qDebug()<<"videoComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";
    });
}

/* TEXT SLOT */
void DefaultApp::textComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;
        timedebuge.start();
        QString appid = mDefaultWidget->getTextWidget()->comboBox()->itemData(index).toString();
        mDefaultUkccDbus->call("setDefaultApp", appid, TEXTTYPE);
        findSelectItem(mDefaultWidget->getAudioWidget()->comboBox());
        UkccCommon::buriedSettings(name(), "the defaultapp to open text", QString("settings"), appid);
        qDebug()<<"textComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";
    });
}

void DefaultApp::loadData(AppInfoService* appInfoService, const char *mimeType) {
    if (mimeType == nullptr || appInfoService == nullptr) {
        return;
    }
    QDBusReply<QVariantList> reply = mDefaultUkccDbus->call("getDefaultApp", mimeType);
    if (!reply.isValid()) {
        return;
    }
    QVariantList list = reply.value();
    for (QVariant app : list) {
        const QDBusArgument &dbusArgs = app.value<QDBusArgument>();
        dbusArgs.beginStructure();
        dbusArgs >> appInfoService->defaultService.icon;
        dbusArgs >> appInfoService->defaultService.name;
        dbusArgs >> appInfoService->defaultService.storageId;
        dbusArgs.endStructure();
    }

    reply = mDefaultUkccDbus->call("getAppList", mimeType);
    if (!reply.isValid()) {
        return;
    }
    list = reply.value();
    appInfoService->clear();
    for (QVariant app : list) {
        const QDBusArgument &dbusArgs = app.value<QDBusArgument>();
        QSharedPointer<Service> servicePtr = QSharedPointer<Service>::create();
        dbusArgs.beginStructure();
        dbusArgs >> servicePtr->icon;
        dbusArgs >> servicePtr->name;
        dbusArgs >> servicePtr->storageId;
        dbusArgs.endStructure();
        appInfoService->serviceList.append(servicePtr);
    }
}

void DefaultApp::load(ComboxWidget *widget, AppInfoService* appInfoService, const char *mimeType, const QString &type)
{
    if (widget == nullptr || appInfoService == nullptr || mimeType == nullptr)
        return;
    widget->comboBox()->blockSignals(true);
    widget->comboBox()->clear();
    // 若默认应用被卸载，显示"选择默认应用"，若不存在其它应用，则显示"无可用程序"
    QString defaultStorageId = appInfoService->defaultService.storageId;
    if (defaultStorageId.isEmpty()) {
        widget->comboBox()->addItem(mSelectString);
    }

    int size = appInfoService->serviceList.size();
    for (int index = 0; index < size; ++index) {
        Service* service = appInfoService->serviceList.at(index).data();
        if (service == nullptr)
            continue;
        qDebug() << service->storageId << "----" << mimeType;
        QIcon appicon;
        QString iconname = service->icon;
        QFileInfo iconFileInfo(iconname);
        if (iconFileInfo.isFile() && (QString(iconname).endsWith(".png") || QString(iconname).endsWith(".svg"))) {
            appicon = QIcon(iconname);
            qDebug() << Q_FUNC_INFO << "get icon from file" << iconname;
        } else {
            appicon = QIcon::fromTheme(iconname, QIcon(QString("/usr/share/pixmaps/" + iconname + ".png")));
            if (appicon.availableSizes().size() == 0) {
                appicon = QIcon::fromTheme(iconname, QIcon(QString("/usr/share/pixmaps/" + iconname + ".xpm")));
            }
        }
        qDebug() << Q_FUNC_INFO << "add item" << appicon << service->name << iconname;
        widget->comboBox()->addItem(appicon, service->name, service->storageId);
    }
    if (defaultStorageId.isEmpty() && widget->comboBox()->count() == 1) {
        widget->comboBox()->clear();
        widget->comboBox()->addItem(mDefaultString);
    }
    if (!defaultStorageId.isEmpty()) {
        widget->comboBox()->setCurrentText(appInfoService->defaultService.name);
    }
    widget->comboBox()->blockSignals(false);
}

void DefaultApp::connectToServer()
{
    QThread *netThread = new QThread;
    MThread *netWorker = new MThread;
    netWorker->moveToThread(netThread);
    connect(netThread, &QThread::started, netWorker, &MThread::run);
    connect(netWorker,&MThread::keychangedsignal,this,&DefaultApp::keyChangedSlot);
    connect(netThread, &QThread::finished, netWorker, &MThread::deleteLater);
    netThread->start();
}

void DefaultApp::findSelectItem(QComboBox *combox)
{
    for (int i = 0; i < combox->count(); i++) {
        if (combox->itemText(i) == mSelectString) {
            combox->removeItem(i);
            break;
        }
    }
}

void DefaultApp::keyChangedSlot(const QString &key) {
    if(key == "default-open") {
        preInitialize();
        initDefaultUI();
    }
}

void DefaultApp::reset()
{
    qDebug() << Q_FUNC_INFO << "reset defaultapp";
    UkccCommon::buriedSettings(name(), "reset defaultapp", QString("clicked"));
    mDefaultUkccDbus->call("resetDefault");
}

void DefaultApp::defaultChangedSlot()
{
    qDebug() << Q_FUNC_INFO << "reset defaultappUi";
    UkccCommon::buriedSettings(name(), "reset defaultappUi", QString("clicked"));
    preInitialize();
    initDefaultUI();
}

void DefaultApp::defaultMimeTypeChangedSlot(const QString& mimeType, const QString& appid) {
    qDebug() << Q_FUNC_INFO << "defaultMimeTypeChangedSlot mimeType" << mimeType << "appid" << appid;
    if (mimeType == BROWSERTYPE) {
        for (int index = 0; index < mDefaultWidget->getBrowserWidget()->comboBox()->count(); ++index) {
            QString curAppid = mDefaultWidget->getBrowserWidget()->comboBox()->itemData(index).toString();
            if (appid == curAppid) {
                mDefaultWidget->getBrowserWidget()->comboBox()->setCurrentIndex(index);
                qDebug() << Q_FUNC_INFO << "set default browser index" << index << appid << mimeType;
            }
        }
    } else if (mimeType == TEXTTYPE) {
        for (int index = 0; index < mDefaultWidget->getTextWidget()->comboBox()->count(); ++index) {
            QString curAppid = mDefaultWidget->getTextWidget()->comboBox()->itemData(index).toString();
            if (appid == curAppid) {
                mDefaultWidget->getTextWidget()->comboBox()->setCurrentIndex(index);
                qDebug() << Q_FUNC_INFO << "set default text index" << index << appid << mimeType;
            }
        }
    } else if (mimeType == MAILTYPE) {
        for (int index = 0; index < mDefaultWidget->getMailWidget()->comboBox()->count(); ++index) {
            QString curAppid = mDefaultWidget->getMailWidget()->comboBox()->itemData(index).toString();
            if (appid == curAppid) {
                mDefaultWidget->getMailWidget()->comboBox()->setCurrentIndex(index);
                qDebug() << Q_FUNC_INFO << "set default mail index" << index << appid << mimeType;
            }
        }
    } else if (mimeType == IMAGETYPE) {
        for (int index = 0; index < mDefaultWidget->getImageWidget()->comboBox()->count(); ++index) {
            QString curAppid = mDefaultWidget->getImageWidget()->comboBox()->itemData(index).toString();
            if (appid == curAppid) {
                mDefaultWidget->getImageWidget()->comboBox()->setCurrentIndex(index);
                qDebug() << Q_FUNC_INFO << "set default image index" << index << appid << mimeType;
            }
        }
    } else if (mimeType == AUDIOTYPE) {
        for (int index = 0; index < mDefaultWidget->getAudioWidget()->comboBox()->count(); ++index) {
            QString curAppid = mDefaultWidget->getAudioWidget()->comboBox()->itemData(index).toString();
            if (appid == curAppid) {
                mDefaultWidget->getAudioWidget()->comboBox()->setCurrentIndex(index);
                qDebug() << Q_FUNC_INFO << "set default audio index" << index << appid << mimeType;
            }
        }
    } else if (mimeType == VIDEOTYPE) {
        for (int index = 0; index < mDefaultWidget->getVideoWidget()->comboBox()->count(); ++index) {
            QString curAppid = mDefaultWidget->getVideoWidget()->comboBox()->itemData(index).toString();
            if (appid == curAppid) {
                mDefaultWidget->getVideoWidget()->comboBox()->setCurrentIndex(index);
                qDebug() << Q_FUNC_INFO << "set default video index" << index << appid << mimeType;
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "finished defaultMimeTypeChangedSlot mimeType" << mimeType << "appid" << appid;
}
