/*
    SPDX-FileCopyrightText: 2010 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "screenshotdbusinterfacekylin.h"
#include "screenshotlogging.h"
#include "utils/serviceutils.h"

#include <KLocalizedString>
#include <KNotification>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDir>
#include <QTemporaryFile>
#include <QtConcurrent>

#include <unistd.h>

namespace KWin
{

static const QString s_dbusServiceName = QStringLiteral("org.ukui.KWin");
static const QString s_dbusInterface = QStringLiteral("org.ukui.kwin.Screenshot");
static const QString s_dbusObjectPath = QStringLiteral("/Screenshot");

const static QString s_errorAlreadyTaking = QStringLiteral("org.ukui.kwin.Screenshot.Error.AlreadyTaking");
const static QString s_errorAlreadyTakingMsg = QStringLiteral("A screenshot is already been taken");
const static QString s_errorNotAuthorized = QStringLiteral("org.ukui.kwin.Screenshot.Error.NoAuthorized");
const static QString s_errorNotAuthorizedMsg = QStringLiteral("The process is not authorized to take a screenshot");
const static QString s_errorFd = QStringLiteral("org.ukui.kwin.Screenshot.Error.FileDescriptor");
const static QString s_errorFdMsg = QStringLiteral("No valid file descriptor");
const static QString s_errorCancelled = QStringLiteral("org.ukui.kwin.Screenshot.Error.Cancelled");
const static QString s_errorCancelledMsg = QStringLiteral("Screenshot got cancelled");
const static QString s_errorInvalidArea = QStringLiteral("org.ukui.kwin.Screenshot.Error.InvalidArea");
const static QString s_errorInvalidAreaMsg = QStringLiteral("Invalid area requested");
const static QString s_errorInvalidScreen = QStringLiteral("org.ukui.kwin.Screenshot.Error.InvalidScreen");
const static QString s_errorInvalidScreenMsg = QStringLiteral("Invalid screen requested");
const static QString s_dbusInterfaceName = QStringLiteral("org.ukui.kwin.Screenshot");
const static QString s_errorScreenMissing = QStringLiteral("org.ukui.kwin.Screenshot.Error.ScreenMissing");
const static QString s_errorScreenMissingMsg = QStringLiteral("Screen not found");

class ScreenShotSourceKylin : public QObject
{
    Q_OBJECT

public:
    explicit ScreenShotSourceKylin(QObject *parent = nullptr);

    virtual QImage data() const = 0;
    virtual void marshal(ScreenShotSinkKylin *sink) = 0;

    virtual bool isCancelled() const = 0;
    virtual bool isCompleted() const = 0;

Q_SIGNALS:
    void cancelled();
    void completed();
};

class ScreenShotSourceBasicKylin : public ScreenShotSourceKylin
{
    Q_OBJECT

public:
    explicit ScreenShotSourceBasicKylin(const QFuture<QImage> &future);

    bool isCancelled() const override;
    bool isCompleted() const override;
    QImage data() const override;
    void marshal(ScreenShotSinkKylin *sink) override;

private:
    QFuture<QImage> m_future;
    QFutureWatcher<QImage> *m_watcher;
};

class ScreenShotSourceScreenKylin : public ScreenShotSourceBasicKylin
{
    Q_OBJECT

public:
    ScreenShotSourceScreenKylin(ScreenShotEffect *effect, EffectScreen *screen, ScreenShotFlags flags);
};

class ScreenShotSourceAreaKylin : public ScreenShotSourceBasicKylin
{
    Q_OBJECT

public:
    ScreenShotSourceAreaKylin(ScreenShotEffect *effect, const QRect &area, ScreenShotFlags flags);
};

class ScreenShotSourceWindowKylin : public ScreenShotSourceBasicKylin
{
    Q_OBJECT

public:
    ScreenShotSourceWindowKylin(ScreenShotEffect *effect, EffectWindow *window, ScreenShotFlags flags);
};

class ScreenShotSourceMultiKylin : public ScreenShotSourceKylin
{
    Q_OBJECT

public:
    explicit ScreenShotSourceMultiKylin(const QList<ScreenShotSourceKylin *> &sources);

    bool isCancelled() const override;
    bool isCompleted() const override;
    QImage data() const override;
    void marshal(ScreenShotSinkKylin *sink) override;

private:
    void handleSourceCancelled();
    void handleSourceCompleted();

    QList<ScreenShotSourceKylin *> m_sources;
};

class ScreenShotSinkKylin : public QObject
{
    Q_OBJECT

public:
    explicit ScreenShotSinkKylin(ScreenShotDBusInterfaceKylin *interface,
                                 QDBusMessage replyMessage = QDBusMessage());

    virtual void flush(const QImage &image);
    virtual void flushMulti(const QList<QImage> &images);

    virtual void cancel();

protected:
    ScreenShotDBusInterfaceKylin *m_interface;
    QDBusMessage m_replyMessage;
};

class ScreenShotSinkPipeKylin : public ScreenShotSinkKylin
{
    Q_OBJECT

public:
    ScreenShotSinkPipeKylin(ScreenShotDBusInterfaceKylin *interface, int fileDescriptor,
                            QDBusMessage replyMessage = QDBusMessage());
    ~ScreenShotSinkPipeKylin() override;

    void flush(const QImage &image) override;
    void flushMulti(const QList<QImage> &images) override;

private:
    int m_fileDescriptor;
};

class ScreenShotSinkXpixmapKylin : public ScreenShotSinkKylin
{
    Q_OBJECT

public:
    explicit ScreenShotSinkXpixmapKylin(ScreenShotDBusInterfaceKylin *interface,
                                        QDBusMessage replyMessage = QDBusMessage());

    void flush(const QImage &image) override;
};

class ScreenShotSinkFileKylin : public ScreenShotSinkKylin
{
    Q_OBJECT

public:
    explicit ScreenShotSinkFileKylin(ScreenShotDBusInterfaceKylin *interface,
                                     QDBusMessage replyMessage = QDBusMessage());

    void flush(const QImage &image) override;
};

ScreenShotSourceKylin::ScreenShotSourceKylin(QObject *parent)
    : QObject(parent)
{
}

ScreenShotSourceBasicKylin::ScreenShotSourceBasicKylin(const QFuture<QImage> &future)
    : m_future(future)
{
    m_watcher = new QFutureWatcher<QImage>(this);
    connect(m_watcher, &QFutureWatcher<QImage>::finished, this, &ScreenShotSourceKylin::completed);
    connect(m_watcher, &QFutureWatcher<QImage>::canceled, this, &ScreenShotSourceKylin::cancelled);
    m_watcher->setFuture(m_future);
}

bool ScreenShotSourceBasicKylin::isCancelled() const
{
    return m_future.isCanceled();
}

bool ScreenShotSourceBasicKylin::isCompleted() const
{
    return m_future.isFinished();
}

QImage ScreenShotSourceBasicKylin::data() const
{
    return m_future.result();
}

void ScreenShotSourceBasicKylin::marshal(ScreenShotSinkKylin *sink)
{
    sink->flush(data());
}

ScreenShotSourceScreenKylin::ScreenShotSourceScreenKylin(ScreenShotEffect *effect,
                                                         EffectScreen *screen,
                                                         ScreenShotFlags flags)
    : ScreenShotSourceBasicKylin(effect->scheduleScreenShot(screen, flags))
{
}

ScreenShotSourceAreaKylin::ScreenShotSourceAreaKylin(ScreenShotEffect *effect,
                                                     const QRect &area,
                                                     ScreenShotFlags flags)
    : ScreenShotSourceBasicKylin(effect->scheduleScreenShot(area, flags))
{
}

ScreenShotSourceWindowKylin::ScreenShotSourceWindowKylin(ScreenShotEffect *effect,
                                                         EffectWindow *window,
                                                         ScreenShotFlags flags)
    : ScreenShotSourceBasicKylin(effect->scheduleScreenShot(window, flags))
{
}

ScreenShotSourceMultiKylin::ScreenShotSourceMultiKylin(const QList<ScreenShotSourceKylin *> &sources)
    : m_sources(sources)
{
    for (ScreenShotSourceKylin *source : sources) {
        source->setParent(this);

        connect(source, &ScreenShotSourceKylin::cancelled,
                this, &ScreenShotSourceMultiKylin::handleSourceCancelled);
        connect(source, &ScreenShotSourceKylin::completed,
                this, &ScreenShotSourceMultiKylin::handleSourceCompleted);
    }
}

bool ScreenShotSourceMultiKylin::isCancelled() const
{
    return std::any_of(m_sources.begin(), m_sources.end(), [](const ScreenShotSourceKylin *source) {
        return source->isCancelled();
    });
}

bool ScreenShotSourceMultiKylin::isCompleted() const
{
    return std::all_of(m_sources.begin(), m_sources.end(), [](const ScreenShotSourceKylin *source) {
        return source->isCompleted();
    });
}

QImage ScreenShotSourceMultiKylin::data() const
{
    return QImage(); // We don't allow nesting multi screenshot sources.
}

void ScreenShotSourceMultiKylin::marshal(ScreenShotSinkKylin *sink)
{
    QList<QImage> images;
    images.reserve(m_sources.count());

    for (ScreenShotSourceKylin *source : qAsConst(m_sources)) {
        images.append(source->data());
    }

    sink->flushMulti(images);
}

void ScreenShotSourceMultiKylin::handleSourceCancelled()
{
    Q_EMIT cancelled();
}

void ScreenShotSourceMultiKylin::handleSourceCompleted()
{
    // If all sources are complete now, emit the completed signal.
    if (isCompleted()) {
        Q_EMIT completed();
    }
}

ScreenShotSinkKylin::ScreenShotSinkKylin(ScreenShotDBusInterfaceKylin *interface, QDBusMessage message)
    : m_interface(interface)
    , m_replyMessage(message)
{
}

void ScreenShotSinkKylin::flush(const QImage &image)
{

    qCWarning(KWIN_SCREENSHOT) << metaObject()->className() << "does not implement" << Q_FUNC_INFO;
}

void ScreenShotSinkKylin::flushMulti(const QList<QImage> &images)
{

    qCWarning(KWIN_SCREENSHOT) << metaObject()->className() << "does not implement" << Q_FUNC_INFO;
}

void ScreenShotSinkKylin::cancel()
{
    if (m_replyMessage.isDelayedReply()) {
        QDBusConnection::sessionBus().send(m_replyMessage.createErrorReply(s_errorCancelled,
                                                                           s_errorCancelledMsg));
    }
}

ScreenShotSinkPipeKylin::ScreenShotSinkPipeKylin(ScreenShotDBusInterfaceKylin *interface, int fileDescriptor,
                                                 QDBusMessage replyMessage)
    : ScreenShotSinkKylin(interface, replyMessage)
    , m_fileDescriptor(fileDescriptor)
{
}

ScreenShotSinkPipeKylin::~ScreenShotSinkPipeKylin()
{
    if (m_fileDescriptor != -1) {
        close(m_fileDescriptor);
    }
}

void ScreenShotSinkPipeKylin::flush(const QImage &image)
{
    if (m_fileDescriptor == -1) {
        return;
    }

    QtConcurrent::run([](int fd, const QImage &image) {
        QFile file;
        if (file.open(fd, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle)) {
            QDataStream ds(&file);
            ds << image;
            file.close();
        } else {
            close(fd);
        }
    },
                      m_fileDescriptor, image);

    // The ownership of the pipe file descriptor has been moved to the worker thread.
    m_fileDescriptor = -1;
}

void ScreenShotSinkPipeKylin::flushMulti(const QList<QImage> &images)
{
    if (m_fileDescriptor == -1) {
        return;
    }

    QtConcurrent::run([](int fd, const QList<QImage> &images) {
        QFile file;
        if (file.open(fd, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle)) {
            QDataStream ds(&file);
            ds.setVersion(QDataStream::Qt_DefaultCompiledVersion);
            ds << images;
            file.close();
        } else {
            close(fd);
        }
    },
                      m_fileDescriptor, images);

    // The ownership of the pipe file descriptor has been moved to the worker thread.
    m_fileDescriptor = -1;
}

ScreenShotSinkXpixmapKylin::ScreenShotSinkXpixmapKylin(ScreenShotDBusInterfaceKylin *interface,
                                                       QDBusMessage replyMessage)
    : ScreenShotSinkKylin(interface, replyMessage)
{
}

static QSize pickWindowSize(const QImage &image)
{
    xcb_connection_t *c = effects->xcbConnection();

    // This will implicitly enable BIG-REQUESTS extension.
    const uint32_t maximumRequestSize = xcb_get_maximum_request_length(c);
    const xcb_setup_t *setup = xcb_get_setup(c);

    uint32_t requestSize = sizeof(xcb_put_image_request_t);

    // With BIG-REQUESTS extension an additional 32-bit field is inserted into
    // the request so we better take it into account.
    if (setup->maximum_request_length < maximumRequestSize) {
        requestSize += 4;
    }

    const uint32_t maximumDataSize = 4 * maximumRequestSize - requestSize;
    const uint32_t bytesPerPixel = image.depth() >> 3;
    const uint32_t bytesPerLine = image.bytesPerLine();

    if (image.sizeInBytes() <= maximumDataSize) {
        return image.size();
    }

    if (maximumDataSize < bytesPerLine) {
        return QSize(maximumDataSize / bytesPerPixel, 1);
    }

    return QSize(image.width(), maximumDataSize / bytesPerLine);
}

static xcb_pixmap_t xpixmapFromImage(const QImage &image)
{
    xcb_connection_t *c = effects->xcbConnection();

    xcb_pixmap_t pixmap = xcb_generate_id(c);
    xcb_gcontext_t gc = xcb_generate_id(c);

    xcb_create_pixmap(c, image.depth(), pixmap, effects->x11RootWindow(),
                      image.width(), image.height());
    xcb_create_gc(c, gc, pixmap, 0, nullptr);

    const int bytesPerPixel = image.depth() >> 3;

    // Figure out how much data we can send with one invocation of xcb_put_image.
    // In contrast to XPutImage, xcb_put_image doesn't implicitly split the data.
    const QSize window = pickWindowSize(image);

    for (int i = 0; i < image.height(); i += window.height()) {
        const int targetHeight = qMin(image.height() - i, window.height());
        const uint8_t *line = image.scanLine(i);

        for (int j = 0; j < image.width(); j += window.width()) {
            const int targetWidth = qMin(image.width() - j, window.width());
            const uint8_t *bytes = line + j * bytesPerPixel;
            const uint32_t byteCount = targetWidth * targetHeight * bytesPerPixel;

            xcb_put_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, gc,
                          targetWidth, targetHeight, j, i, 0, image.depth(),
                          byteCount, bytes);
        }
    }

    xcb_flush(c);
    xcb_free_gc(c, gc);

    return pixmap;
}

void ScreenShotSinkXpixmapKylin::flush(const QImage &image)
{
    const xcb_pixmap_t pixmap = xpixmapFromImage(image);
    Q_EMIT m_interface->screenshotCreated(pixmap);
}

ScreenShotSinkFileKylin::ScreenShotSinkFileKylin(ScreenShotDBusInterfaceKylin *interface,
                                                 QDBusMessage replyMessage)
    : ScreenShotSinkKylin(interface, replyMessage)
{
}

static QString saveTempImage(const QImage &image)
{
    if (image.isNull()) {
        return QString();
    }
    QTemporaryFile temp(QDir::tempPath() + QDir::separator() + QLatin1String("kwin_screenshot_XXXXXX.png"));
    temp.setAutoRemove(false);
    if (!temp.open()) {
        return QString();
    }
    image.save(&temp);
    temp.close();
    KNotification::event(KNotification::Notification,
                         i18nc("Notification caption that a screenshot got saved to file", "Screenshot"),
                         i18nc("Notification with path to screenshot file", "Screenshot saved to %1", temp.fileName()),
                         QStringLiteral("kylin-screenshot"));
    return temp.fileName();
}

void ScreenShotSinkFileKylin::flush(const QImage &image)
{
    QDBusConnection::sessionBus().send(m_replyMessage.createReply(saveTempImage(image)));
}

ScreenShotDBusInterfaceKylin::ScreenShotDBusInterfaceKylin(ScreenShotEffect *effect, QObject *parent)
    : QObject(parent)
    , m_effect(effect)
{
    // QDBusConnection::sessionBus().registerObject(QStringLiteral("/Screenshot"), this, QDBusConnection::ExportScriptableContents);
    // QDBusConnection::sessionBus().registerService(QStringLiteral("org.ukui.KWin"));

    QDBusConnection::sessionBus().registerObject(s_dbusObjectPath, this, QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerService(s_dbusServiceName);
}

ScreenShotDBusInterfaceKylin::~ScreenShotDBusInterfaceKylin()
{
    QDBusConnection::sessionBus().unregisterService(s_dbusServiceName);
    QDBusConnection::sessionBus().unregisterObject(s_dbusObjectPath);
}

bool ScreenShotDBusInterfaceKylin::checkCall() const
{
    if (!calledFromDBus()) {
        return false;
    }

    const QDBusReply<uint> reply = connection().interface()->servicePid(message().service());
    if (reply.isValid()) {
        const uint pid = reply.value();
        const auto interfaces = KWin::fetchRestrictedDBusInterfacesFromPid(pid);
        if (!interfaces.contains(s_dbusInterfaceName)) {
            qCWarning(KWIN_SCREENSHOT) << "Process" << pid << "tried to take a screenshot without being granted to DBus interface" << s_dbusInterfaceName;
        }
    } else {
        return false;
    }

    if (isTakingScreenshot()) {
        sendErrorReply(s_errorAlreadyTaking, s_errorAlreadyTakingMsg);
        return false;
    }
    return true;
}

void ScreenShotDBusInterfaceKylin::screenshotWindowUnderCursor(int mask)
{
    if (isTakingScreenshot()) {
        sendErrorReply(s_errorAlreadyTaking, s_errorAlreadyTakingMsg);
        return;
    }

    EffectWindow *hoveredWindow = nullptr;

    const QPoint cursor = effects->cursorPos();
    EffectWindowList order = effects->stackingOrder();
    EffectWindowList::const_iterator it = order.constEnd(), first = order.constBegin();
    while (it != first) {
        hoveredWindow = *(--it);
        if (hoveredWindow->isOnCurrentDesktop()
            && !hoveredWindow->isMinimized()
            && !hoveredWindow->isDeleted()
            && hoveredWindow->frameGeometry().contains(cursor)) {
            break;
        }
        hoveredWindow = nullptr;
    }

    if (hoveredWindow) {
        takeScreenShot(hoveredWindow, ScreenShotFlags(mask),
                       new ScreenShotSinkXpixmapKylin(this, message()));
    }
}

void ScreenShotDBusInterfaceKylin::screenshotForWindow(qulonglong winId, int mask)
{
    EffectWindow *window = effects->findWindow(winId);
    if (!window || window->isMinimized() || window->isDeleted()) {
        return;
    }

    takeScreenShot(window, ScreenShotFlags(mask), new ScreenShotSinkXpixmapKylin(this, message()));
}

QString ScreenShotDBusInterfaceKylin::interactive(int mask)
{
    if (!calledFromDBus()) {
        return QString();
    }
    if (isTakingScreenshot()) {
        sendErrorReply(s_errorAlreadyTaking, s_errorAlreadyTakingMsg);
        return QString();
    }

    const QDBusMessage replyMessage = message();
    setDelayedReply(true);

    effects->startInteractiveWindowSelection([this, mask, replyMessage](EffectWindow *window) {
        hideInfoMessage();
        if (!window) {
            QDBusConnection::sessionBus().send(replyMessage.createErrorReply(s_errorCancelled,
                                                                             s_errorCancelledMsg));
            return;
        }
        takeScreenShot(window, ScreenShotFlags(mask), new ScreenShotSinkFileKylin(this, replyMessage));
    });

    showInfoMessage(InfoMessageMode::Window);
    return QString();
}

void ScreenShotDBusInterfaceKylin::interactive(QDBusUnixFileDescriptor fd, int mask)
{
    if (!calledFromDBus()) {
        return;
    }
    if (isTakingScreenshot()) {
        sendErrorReply(s_errorAlreadyTaking, s_errorAlreadyTakingMsg);
        return;
    }

    const int fileDescriptor = dup(fd.fileDescriptor());
    if (fileDescriptor == -1) {
        sendErrorReply(s_errorFd, s_errorFdMsg);
        return;
    }

    effects->startInteractiveWindowSelection([this, fileDescriptor, mask](EffectWindow *window) {
        hideInfoMessage();
        if (!window) {
            close(fileDescriptor);
            return;
        }
        takeScreenShot(window, ScreenShotFlags(mask),
                       new ScreenShotSinkPipeKylin(this, fileDescriptor));
    });

    showInfoMessage(InfoMessageMode::Window);
}

QString ScreenShotDBusInterfaceKylin::screenshotFullscreen(bool captureCursor)
{
    if (!checkCall()) {
        return QString();
    }

    ScreenShotFlags flags = ScreenShotFlags();
    if (captureCursor) {
        flags |= ScreenShotIncludeCursor;
    }

    takeScreenShot(effects->virtualScreenGeometry(), flags, new ScreenShotSinkFileKylin(this, message()));

    setDelayedReply(true);
    return QString();
}

void ScreenShotDBusInterfaceKylin::screenshotFullscreen(QDBusUnixFileDescriptor fd,
                                                        bool captureCursor, bool shouldReturnNativeSize)
{
    if (!checkCall()) {
        return;
    }

    const int fileDescriptor = dup(fd.fileDescriptor());
    if (fileDescriptor == -1) {
        sendErrorReply(s_errorFd, s_errorFdMsg);
        return;
    }

    ScreenShotFlags flags = ScreenShotFlags();
    if (captureCursor) {
        flags |= ScreenShotIncludeCursor;
    }
    if (shouldReturnNativeSize) {
        flags |= ScreenShotNativeResolution;
    }

    takeScreenShot(effects->virtualScreenGeometry(), flags,
                   new ScreenShotSinkPipeKylin(this, fileDescriptor));
}

QString ScreenShotDBusInterfaceKylin::screenshotScreen(int screenId, bool captureCursor)
{
    if (!checkCall()) {
        return QString();
    }

    EffectScreen *screen = effects->findScreen(screenId);
    if (!screen) {
        sendErrorReply(s_errorInvalidScreen, s_errorInvalidScreenMsg);
        return QString();
    }

    ScreenShotFlags flags = ScreenShotNativeResolution;
    if (captureCursor) {
        flags |= ScreenShotIncludeCursor;
    }

    takeScreenShot(screen, flags, new ScreenShotSinkFileKylin(this, message()));

    setDelayedReply(true);
    return QString();
}

void ScreenShotDBusInterfaceKylin::screenshotScreen(QDBusUnixFileDescriptor fd, bool captureCursor)
{
    if (!checkCall()) {
        return;
    }

    const int fileDescriptor = dup(fd.fileDescriptor());
    if (fileDescriptor == -1) {
        sendErrorReply(s_errorFd, s_errorFdMsg);
        return;
    }

    ScreenShotFlags flags = ScreenShotNativeResolution;
    if (captureCursor) {
        flags |= ScreenShotIncludeCursor;
    }

    effects->startInteractivePositionSelection([this, fileDescriptor, flags](const QPoint &p) {
        hideInfoMessage();
        if (p == QPoint(-1, -1)) {
            close(fileDescriptor);
        } else {
            EffectScreen *screen = effects->screenAt(p);
            if (!screen) {
                close(fileDescriptor);
                return;
            }
            takeScreenShot(screen, flags, new ScreenShotSinkPipeKylin(this, fileDescriptor));
        }
    });
    showInfoMessage(InfoMessageMode::Screen);
}

void ScreenShotDBusInterfaceKylin::screenshotScreens(QDBusUnixFileDescriptor fd,
                                                     const QStringList &screensNames,
                                                     bool captureCursor, bool shouldReturnNativeSize)
{
    if (!checkCall()) {
        return;
    }

    const int fileDescriptor = dup(fd.fileDescriptor());
    if (fileDescriptor == -1) {
        sendErrorReply(s_errorFd, s_errorFdMsg);
        return;
    }

    ScreenShotFlags flags = ScreenShotFlags();
    if (captureCursor) {
        flags |= ScreenShotIncludeCursor;
    }
    if (shouldReturnNativeSize) {
        flags |= ScreenShotNativeResolution;
    }

    QList<EffectScreen *> screens;
    screens.reserve(screensNames.count());

    for (const QString &screenName : screensNames) {
        EffectScreen *screen = effects->findScreen(screenName);
        if (!screen) {
            close(fileDescriptor);
            sendErrorReply(s_errorScreenMissing, s_errorScreenMissingMsg + ": " + screenName);
            return;
        }
        screens.append(screen);
    }

    ScreenShotSinkPipeKylin *sink = new ScreenShotSinkPipeKylin(this, fileDescriptor);
    takeScreenShot(screens, flags, sink);
}

QString ScreenShotDBusInterfaceKylin::screenshotArea(int x, int y, int width, int height, bool captureCursor)
{
    if (!checkCall()) {
        return QString();
    }

    const QRect area(x, y, width, height);
    if (area.isEmpty()) {
        sendErrorReply(s_errorInvalidArea, s_errorInvalidAreaMsg);
        return QString();
    }

    ScreenShotFlags flags = ScreenShotFlags();
    if (captureCursor) {
        flags |= ScreenShotIncludeCursor;
    }

    takeScreenShot(area, flags, new ScreenShotSinkFileKylin(this, message()));
    setDelayedReply(true);

    return QString();
}

bool ScreenShotDBusInterfaceKylin::isTakingScreenshot() const
{
    return m_source;
}

void ScreenShotDBusInterfaceKylin::showInfoMessage(InfoMessageMode mode)
{
    QString text;
    switch (mode) {
    case InfoMessageMode::Window:
        text = i18n("Select window to screen shot with left click or enter.\nEscape or right click to cancel.");
        break;
    case InfoMessageMode::Screen:
        text = i18n("Create screen shot with left click or enter.\nEscape or right click to cancel.");
        break;
    }
    effects->showOnScreenMessage(text, QStringLiteral("kylin-screenshot"));
}

void ScreenShotDBusInterfaceKylin::hideInfoMessage()
{
    effects->hideOnScreenMessage(EffectsHandler::OnScreenMessageHideFlag::SkipsCloseAnimation);
}

void ScreenShotDBusInterfaceKylin::handleSourceCancelled()
{
    m_sink->cancel();

    m_source.reset();
    m_sink.reset();
}

void ScreenShotDBusInterfaceKylin::handleSourceCompleted()
{
    m_source->marshal(m_sink.data());

    m_source.reset();
    m_sink.reset();
}

void ScreenShotDBusInterfaceKylin::bind(ScreenShotSinkKylin *sink, ScreenShotSourceKylin *source)
{
    m_sink.reset(sink);
    m_source.reset(source);

    connect(m_source.data(), &ScreenShotSourceKylin::cancelled,
            this, &ScreenShotDBusInterfaceKylin::handleSourceCancelled);
    connect(m_source.data(), &ScreenShotSourceKylin::completed,
            this, &ScreenShotDBusInterfaceKylin::handleSourceCompleted);
}

void ScreenShotDBusInterfaceKylin::takeScreenShot(EffectScreen *screen, ScreenShotFlags flags,
                                                  ScreenShotSinkKylin *sink)
{
    bind(sink, new ScreenShotSourceScreenKylin(m_effect, screen, flags));
}

void ScreenShotDBusInterfaceKylin::takeScreenShot(const QList<EffectScreen *> &screens,
                                                  ScreenShotFlags flags, ScreenShotSinkKylin *sink)
{
    QList<ScreenShotSourceKylin *> sources;
    sources.reserve(screens.count());
    for (EffectScreen *screen : screens) {
        sources.append(new ScreenShotSourceScreenKylin(m_effect, screen, flags));
    }

    bind(sink, new ScreenShotSourceMultiKylin(sources));
}

void ScreenShotDBusInterfaceKylin::takeScreenShot(const QRect &area, ScreenShotFlags flags,
                                                  ScreenShotSinkKylin *sink)
{
    bind(sink, new ScreenShotSourceAreaKylin(m_effect, area, flags));
}

void ScreenShotDBusInterfaceKylin::takeScreenShot(EffectWindow *window, ScreenShotFlags flags,
                                                  ScreenShotSinkKylin *sink)
{
    bind(sink, new ScreenShotSourceWindowKylin(m_effect, window, flags));
}

} // namespace KWin

#include "screenshotdbusinterfacekylin.moc"
