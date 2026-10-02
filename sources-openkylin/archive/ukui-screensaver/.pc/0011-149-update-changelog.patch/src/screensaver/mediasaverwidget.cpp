#include "mediasaverwidget.h"
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QHBoxLayout>
#include <QPalette>
#include <QPainter>
#include <QDebug>
#include <QFileInfo>
#include "mprisdbusclient.h"

#define AUDIOTYPE "audio/x-vorbis+ogg"

#define DESKTOPPATH "/usr/share/applications/"

#define LOCAL_CONFIG_DIR "/.config/"
#define LOCAL_APP_DIR "/.local/share/applications/"
#define SYSTEM_CONFIG_DIR "/usr/share/applications/"

MediaSaverWidget::MediaSaverWidget(QWidget *parent, bool saverMedia)
    : QWidget(parent)
    , m_bSaverMedia(saverMedia)
{
    if (m_bSaverMedia) {
        initSaverMediaWidget();
    }
}

MediaSaverWidget::~MediaSaverWidget() {}

void MediaSaverWidget::setSaverMediaType(bool saverMedia)
{
    if (m_bSaverMedia != saverMedia && m_pMPRISWidget) {
        m_pMPRISWidget->setSaverMediaType(saverMedia);
        if (!saverMedia) {
            m_strMediaPath = "";
            initMultimediaControl();
        }
    }
    m_bSaverMedia = saverMedia;
}

void MediaSaverWidget::initMultimediaControl()
{
    qDebug() << "initMultimediaControl---";
    if (!m_dbusListNameWatcher) {
        QDBusInterface dbusInter(
            "org.freedesktop.DBus", "/", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);

        QDBusPendingCall call = dbusInter.asyncCall("ListNames");

        m_dbusListNameWatcher = new QDBusPendingCallWatcher(call, this);
        connect(m_dbusListNameWatcher, &QDBusPendingCallWatcher::finished, [=] {
            if (!call.isError()) {
                QDBusReply<QStringList> reply = call.reply();
                const QStringList &serviceList = reply.value();
                QString service = QString();
                // check is default audio app
                QString defaultAudioExeInfo = getDefaultAudioAppExecInfo();
                for (const QString &serv : serviceList) {
                    if (!serv.startsWith("org.mpris.MediaPlayer2.") || serv.endsWith("KylinVideo")) {
                        continue;
                    }
                    // 优先默认音频应用，其次按dbus列表第一个
                    if (!serv.isEmpty()) {
                        QString strServicePath = getServiceCmd(serv);
                        if (!strServicePath.isEmpty() && !defaultAudioExeInfo.isEmpty()) {
                            if (defaultAudioExeInfo.contains(strServicePath)) {
                                service = serv;
                                break;
                            }
                        }
                    }
                }
                if (service.isEmpty()) {
                    // check is playing app
                    for (const QString &serv : serviceList) {
                        if (!serv.startsWith("org.mpris.MediaPlayer2.") || serv.endsWith("KylinVideo")) {
                            continue;
                        }
                        if (service.isEmpty()) {
                            service = serv;
                        }
                        // 优先播放状态应用，其次按dbus列表第一个
                        if (!serv.isEmpty()) {
                            MPRISDBusClient *client = new MPRISDBusClient(
                                serv, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), this);
                            if (client->playbackStatus() == "Playing") {
                                service = serv;
                                break;
                            }
                            client->deleteLater();
                            client = nullptr;
                        }
                    }
                }
                // 未检测到支持mpris的mediaplayer服务
                if (service.isEmpty()) {
                    if (!m_dbusDaemonInterface) {
                        m_dbusDaemonInterface = QDBusConnection::sessionBus().interface();
                    } else {
                        disconnect(m_dbusDaemonInterface);
                    }
                    connect(
                        m_dbusDaemonInterface,
                        &QDBusConnectionInterface::serviceOwnerChanged,
                        this,
                        [=](const QString &name, const QString &oldOwner, const QString &newOwner) {
                            Q_UNUSED(oldOwner)
                            if (name.startsWith("org.mpris.MediaPlayer2.") && !newOwner.isEmpty()) {
                                initMultimediaControl();
                                disconnect(m_dbusDaemonInterface);
                            }
                        });
                    init("");
                    m_dbusListNameWatcher->deleteLater();
                    m_dbusListNameWatcher = nullptr;
                    Q_EMIT loadDbusListNameFinished(false);
                    return;
                }

                qDebug() << "Found MultimediaControl DBus service:" << service;
                init(service);
                Q_EMIT loadDbusListNameFinished(true);
            } else {
                qInfo() << "init MultimediaControl error: " << call.error().message();
                Q_EMIT loadDbusListNameFinished(false);
            }

            m_dbusListNameWatcher->deleteLater();
            m_dbusListNameWatcher = nullptr;
        });
    }
}

void MediaSaverWidget::init(const QString &strMediaPath)
{
    if (m_strMediaPath == strMediaPath && !strMediaPath.isEmpty()) {
        return;
    }

    if (!m_pMPRISWidget) {
        m_pMPRISWidget = new MPRISSaverWidget(this);
        m_pMPRISWidget->updateFont(m_strCurFont, m_lfFontSizeF);

        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        mainLayout->addWidget(m_pMPRISWidget);

        setLayout(mainLayout);
        initConnections();
    }
    m_strMediaPath = strMediaPath;
    m_pMPRISWidget->init(m_strMediaPath);
    m_pMPRISWidget->setFixedSize(this->size());
    m_pMPRISWidget->updateMediaSize(m_scale);
}

void MediaSaverWidget::initSaverMediaWidget()
{
    if (!m_pMPRISWidget) {
        m_pMPRISWidget = new MPRISSaverWidget(this, m_bSaverMedia);
//        m_pMPRISWidget->onDelayMetadataUpdate();
        m_pMPRISWidget->updateFont(m_strCurFont, m_lfFontSizeF);

        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        mainLayout->addWidget(m_pMPRISWidget);

        setLayout(mainLayout);
    }
}

void MediaSaverWidget::initConnections()
{
    if (m_pMPRISWidget) {
        connect(m_pMPRISWidget, &MPRISSaverWidget::statusChanged, this, &MediaSaverWidget::onStatusChanged);
    }
}

void MediaSaverWidget::onStatusChanged(bool isActive)
{
    this->setVisible(isActive);
    if (!isActive) {
        m_strMediaPath.clear();
        QTimer::singleShot(10, this, [=]() { initMultimediaControl(); });
    }
}

//void MediaSaverWidget::paintEvent(QPaintEvent *event)
//{
//    // 设置窗体为圆角
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
//    painter.setBrush(QColor(245, 245, 245, 166));
//    painter.setPen(Qt::transparent);
//    auto rect = this->rect();
//    painter.drawRoundedRect(rect, 12, 12); // 窗口圆角

//    return QWidget::paintEvent(event);
//}

QString MediaSaverWidget::getServiceCmd(const QString &strMediaPath)
{
    QString execPath = "";
    if (strMediaPath.isEmpty()) {
        return execPath;
    }
    QDBusInterface dbusInter("org.freedesktop.DBus", "/", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);

    QDBusReply<unsigned> call = dbusInter.call("GetConnectionUnixProcessID", strMediaPath);
    if (!call.isValid()) {
        qDebug() << "getServicePID error:" << call.error();
        return execPath;
    }
    /*check caller white list*/
    QFileInfo file(QString("/proc/%1/exe").arg(call.value()));
    if (file.exists()) {
        execPath = file.canonicalFilePath();
    }
    return execPath;
}

/* 获取默认应用的应用名 */
QString MediaSaverWidget::getDefaultAppId(const char *contentType)
{
    QString localfile = QDir::homePath() + LOCAL_CONFIG_DIR + "mimeapps.list";
    QString systemfile = SYSTEM_CONFIG_DIR + QString("ukui-mimeapps.list");
    if (QFile(localfile).exists()) {
        QSettings *mimeappFile = new QSettings(localfile, QSettings::IniFormat);
        mimeappFile->setIniCodec("utf-8");
        QString str = mimeappFile->value(QString("Default Applications/%1").arg(contentType)).toString();
        delete mimeappFile;
        mimeappFile = nullptr;
        if (!str.isEmpty()) {
            if (QFile(SYSTEM_CONFIG_DIR + str).exists() || QFile(QDir::homePath() + LOCAL_APP_DIR + str).exists()) {
                return str;
            }
        }
    }
    if (QFile(systemfile).exists()) {
        QSettings *mimeappFile = new QSettings(systemfile, QSettings::IniFormat);
        mimeappFile->setIniCodec("utf-8");
        QString str = mimeappFile->value(QString("Default Applications/%1").arg(contentType)).toString();
        delete mimeappFile;
        mimeappFile = nullptr;
        if (!str.isEmpty()) {
            if (QFile(SYSTEM_CONFIG_DIR + str).exists()) {
                return str;
            } else {
                return QString("");
            }
        }
    }
    return QString("");
}

QString MediaSaverWidget::getDefaultAudioAppExecInfo()
{
    QString current(getDefaultAppId(AUDIOTYPE));
    QString defaultAppInfo;
    if (!current.isEmpty()) {
        QByteArray ba = QString(DESKTOPPATH + current).toUtf8();
        if (!QFile(DESKTOPPATH + current).exists()) {
            ba = QString(QDir::homePath() + LOCAL_APP_DIR + current).toUtf8();
        }
        GDesktopAppInfo *info = g_desktop_app_info_new_from_filename(ba.constData());
        QString execName = g_app_info_get_executable(G_APP_INFO(info));
        QString commandName = g_app_info_get_commandline(G_APP_INFO(info));
        if (!execName.isEmpty()) {
            defaultAppInfo.append(execName + " ");
            if (!execName.contains("/")) {
                QString strFullName = QStandardPaths::findExecutable(execName);
                if (!strFullName.isEmpty()) {
                    defaultAppInfo.append(strFullName + " ");
                }
            }
        }
        if (!commandName.isEmpty()) {
            defaultAppInfo.append(commandName);
        }
        if (g_app_info_can_delete(G_APP_INFO(info))) {
            g_app_info_delete(G_APP_INFO(info));
        }
    }
    qDebug() << "defalt app info:" << defaultAppInfo;
    return defaultAppInfo;
}

void MediaSaverWidget::updateFont(QString strFont, double fontSizeF)
{
    m_strCurFont = strFont;
    m_lfFontSizeF = fontSizeF;
    if (m_pMPRISWidget) {
        m_pMPRISWidget->updateFont(strFont, fontSizeF);
    }
}

void MediaSaverWidget::updateMediaSize(float scale)
{
    m_scale = scale;
    if (m_pMPRISWidget) {
        m_pMPRISWidget->setFixedSize(this->size());
        m_pMPRISWidget->updateMediaSize(scale);
    }
}
