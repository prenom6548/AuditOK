/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "background-manager.h"
#include "screen-manager.h"
#include <QVariant>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QGSettings/QGSettings>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QImageReader>
#include <QImage>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <gio/gio.h>
#include "file-utils.h"


static BackgroundManager *global_instance = nullptr;

/*! Returns pixel size for wallpaper scaling; handles .dib / odd formats where QImageReader::size() fails. */
static QSize readImagePixelSize(const QString &normalizedPath)
{
    if (normalizedPath.isEmpty()) {
        return QSize();
    }

    QImageReader reader(normalizedPath);
    reader.setDecideFormatFromContent(true);
    QSize s = reader.size();
    if (s.isValid() && !s.isEmpty()) {
        return s;
    }

    const QString lower = normalizedPath.toLower();
    if (lower.endsWith(QLatin1String(".dib"))) {
        QImageReader dibReader(normalizedPath, QByteArrayLiteral("bmp"));
        s = dibReader.size();
        if (s.isValid() && !s.isEmpty()) {
            return s;
        }
    }

    QImage decoded;
    if (decoded.load(normalizedPath) && !decoded.size().isEmpty()) {
        return decoded.size();
    }

    return QSize();
}

/*!
 * Qt Quick Image cannot decode some raster formats (e.g. .dib) even when QImage can.
 * Decode to a PNG under the cache dir and return that path for QML; otherwise return \a wallpaperPath unchanged.
 */
static QString pathForQmlWallpaperImage(const QString &wallpaperPath)
{
    if (wallpaperPath.isEmpty()) {
        return wallpaperPath;
    }

    QString local = wallpaperPath;
    if (local.startsWith(QStringLiteral("file://"))) {
        local = local.mid(7);
    }
    if (local.isEmpty() || !QFile::exists(local)) {
        return wallpaperPath;
    }

    const QString lower = local.toLower();
    if (!lower.endsWith(QStringLiteral(".dib"))) {
        return wallpaperPath;
    }

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/peony-desktop");
    if (!QDir().mkpath(cacheDir)) {
        qWarning() << "pathForQmlWallpaperImage: mkpath failed:" << cacheDir;
        return wallpaperPath;
    }

    QString cacheKey = QString::fromLatin1(
        QCryptographicHash::hash(local.toUtf8(), QCryptographicHash::Md5).toHex());
    const QString cachePath = cacheDir + QStringLiteral("/wallpaper-") + cacheKey + QStringLiteral(".png");

    if (QFileInfo::exists(cachePath)) {
        return cachePath;
    }

    QImage image;
    QImageReader probe(local);
    probe.setDecideFormatFromContent(true);
    if (probe.canRead()) {
        image = probe.read();
    }
    if (image.isNull()) {
        QImageReader dibReader(local, QByteArrayLiteral("bmp"));
        if (dibReader.canRead()) {
            image = dibReader.read();
        }
    }
    if (image.isNull() && !image.load(local)) {
        qWarning() << "pathForQmlWallpaperImage: cannot decode DIB:" << local;
        return wallpaperPath;
    }

    if (!image.save(cachePath, "PNG")) {
        qWarning() << "pathForQmlWallpaperImage: save PNG failed:" << cachePath;
        return wallpaperPath;
    }
    return cachePath;
}

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject{parent}
{
//    m_delayedUpdateTimer = new QTimer(this);
//    m_delayedUpdateTimer->setSingleShot(true);
//    m_delayedUpdateTimer->setInterval(2000);
//    connect(m_delayedUpdateTimer, &QTimer::timeout, this, &BackgroundManager::onDelayedUpdateSourceSizes);

    initGSettings();
    initBackground();
    initScreenMonitoring();
}

BackgroundManager *BackgroundManager::globalInstance()
{
    if (!global_instance) {
        global_instance = new BackgroundManager;
    }
    return global_instance;
}

QString BackgroundManager::getBackImagePath() const
{
    if (m_backImagePath.isEmpty()) {
        return "";
    }
    
    QString encodedPath = Peony::FileUtils::urlEncode(m_backImagePath);
    if (!encodedPath.startsWith("file://")) {
        encodedPath = "file://" + encodedPath;
    }
    return encodedPath;
}

QString BackgroundManager::getFrontImagePath() const
{
    if (m_frontImagePath.isEmpty()) {
        return "";
    }
    
    QString encodedPath = Peony::FileUtils::urlEncode(m_frontImagePath);
    if (!encodedPath.startsWith("file://")) {
        encodedPath = "file://" + encodedPath;
    }
    return encodedPath;
}

QString BackgroundManager::getBackgroundOption() const
{
    return m_backgroundOption;
}

void BackgroundManager::setBackgroundOption(const QString &backgroundOption)
{
    if (m_backgroundOption != backgroundOption) {
        m_backgroundOption = backgroundOption;
        Q_EMIT backgroundOptionChanged();
    }
}

QString BackgroundManager::getAccountBackground() const
{
    if (!m_shouldSyncAccountBackground) {
        if (m_backgroundSettings) {
            return m_backgroundSettings->get(PICTUREFILENAME).toString();
        }
        return "";
    }

    uid_t uid = getuid();
    QDBusInterface iface(ACCOUNT_SETTINGS, ACCOUNT_PATH,
                         ACCOUNT_SETTINGS, QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserById", (qint64)uid);
    if(!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface(ACCOUNT_SETTINGS, userPath.value().path(),
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> backgroundReply = userIface.call("Get", "org.freedesktop.Accounts.User", "BackgroundFile");
        if(backgroundReply.isValid())
            return  backgroundReply.value().variant().toString();
    }
    return "";
}

void BackgroundManager::setAccountBackground(const QString &backgroundPath)
{
    if (!m_shouldSyncAccountBackground) {
        return;
    }

    QDBusInterface * interface = new QDBusInterface(ACCOUNT_SETTINGS,
                                                    ACCOUNT_PATH,
                                                    ACCOUNT_SETTINGS,
                                                    QDBusConnection::systemBus());

    if (!interface->isValid()){
        qCritical() << "Create /org/freedesktop/Accounts Client Interface Failed " << QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusReply<QDBusObjectPath> reply = interface->call("FindUserByName", g_get_user_name());
    QString userPath;
    if (reply.isValid()){
        userPath = reply.value().path();
    } else {
        qCritical() << "Call 'GetComputerInfo' Failed!" << reply.error().message();
        return;
    }

    QDBusInterface *useriFace = new QDBusInterface(ACCOUNT_SETTINGS,
                                                   userPath,
                                                   "org.freedesktop.Accounts.User",
                                                   QDBusConnection::systemBus());

    if (!useriFace->isValid()){
        qCritical() << QString("Create %1 Client Interface Failed").arg(userPath) << QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusMessage msg = useriFace->call("SetBackgroundFile", backgroundPath);
    qDebug() << "setAccountBackground path:" << backgroundPath;
    if (!msg.errorMessage().isEmpty()) {
        qDebug() << "update user background file error: " << msg.errorMessage();
    } else {
        Q_EMIT accountBackgroundChanged();
    }

    //saveBlurBackground
    QProcess p;
    p.startDetached("/usr/bin/save-blurBackground");
}

bool BackgroundManager::getUsePureColor() const
{
    return m_usePureColor;
}

void BackgroundManager::setUsePureColor(const bool usePureColor)
{
    if (m_usePureColor != usePureColor) {
        m_usePureColor = usePureColor;
        Q_EMIT usePureColorChanged();
    }
}

QColor BackgroundManager::getColor() const
{
    return m_color;
}

void BackgroundManager::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        Q_EMIT colorChanged();
    }
}

QSize BackgroundManager::getFrontImageSourceSize() const
{
    return m_frontImageSourceSize;
}

QSize BackgroundManager::getBackImageSourceSize() const
{
    return m_backImageSourceSize;
}

QRect BackgroundManager::calculateSpannedSourceRect(const QRect &screenGeometry, const QString &imagePath) const
{
    QString normalizedPath = normalizeImagePath(imagePath);
    QRegion region;
    for (auto screen : qApp->screens()) {
        region += screen->geometry();
    }
    QRect virtualGeometry = region.boundingRect().translated(0, 0);

    // 根据图片路径确定使用哪个源尺寸
    QSize sourceSizeForCalculation;
    if (imagePath == m_frontImagePath) {
        sourceSizeForCalculation = m_frontImageSourceSize;
    } else if (imagePath == m_backImagePath) {
        sourceSizeForCalculation = m_backImageSourceSize;
    } else {
        sourceSizeForCalculation = calculateOptimalSourceSize(imagePath);
    }

    if (!sourceSizeForCalculation.isValid() || sourceSizeForCalculation.isEmpty()) {
        qWarning() << "Invalid source size for calculation, using original size. Path:" << imagePath
                  << "FrontSourceSize:" << m_frontImageSourceSize
                  << "BackSourceSize:" << m_backImageSourceSize;
        sourceSizeForCalculation = readImagePixelSize(normalizedPath);
    }

    qreal pixWidth = sourceSizeForCalculation.width();
    qreal pixHeight = sourceSizeForCalculation.height();
    if (pixWidth <= 0 || pixHeight <= 0) {
        qWarning() << "calculateSpannedSourceRect: invalid pixel dimensions for" << imagePath;
        return QRect();
    }

    QSize sourceSize = readImagePixelSize(normalizedPath);
    if (!sourceSize.isValid() || sourceSize.isEmpty()) {
        sourceSize = QSize(int(pixWidth), int(pixHeight));
    }
    sourceSize.setWidth(screenGeometry.width() * 1.0 / virtualGeometry.width() * pixWidth);
    sourceSize.setHeight(screenGeometry.height() * 1.0 / virtualGeometry.height() * pixHeight);

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (screenGeometry.x() > 0) {
        offsetX = (screenGeometry.x() * 1.0 / virtualGeometry.width() * pixWidth);
    }

    if (screenGeometry.y() > 0) {
        offsetY = (screenGeometry.y() * 1.0 / virtualGeometry.height() * pixHeight);
    }

    QPoint offsetPoint(0, 0);
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

QSize BackgroundManager::calculateOptimalSourceSize(const QString &imagePath) const
{
    if (imagePath.isEmpty()) {
        return QSize();
    }

    QSize cachedSize = getCachedOptimalSourceSize(imagePath);
    if (cachedSize.isValid()) {
        return cachedSize;
    }

    // Normalize the path to handle file:// prefix
    QString normalizedPath = normalizeImagePath(imagePath);

    QSize originalSize = readImagePixelSize(normalizedPath);
    if (!originalSize.isValid() || originalSize.isEmpty()) {
        qWarning() << "Failed to get image size for:" << normalizedPath;
        return QSize();
    }

    // Get current maximum screen dimensions with DPI scaling
    QSize currentMaxScreenSize = maxScreenSize();
    
    // Calculate optimal target size using the sophisticated algorithm
    QSize targetSize = calculateWallpaperTargetSize(originalSize, currentMaxScreenSize);
    
    // If calculation failed, fallback to original size
    if (!targetSize.isValid()) {
        targetSize = originalSize;
    } else if (isLeftGreaterThanRightSize(targetSize, originalSize)) {
        qDebug() << "Target size" << targetSize << "has larger area than original size" << originalSize 
                 << "- using original size to prevent quality loss";
        targetSize = originalSize;
    }
    
    // Cache the result
    QFileInfo fileInfo(normalizedPath);
    if (fileInfo.exists()) {
        SourceSizeInfo cacheInfo;
        cacheInfo.size = targetSize;
        cacheInfo.lastModified = fileInfo.lastModified().toSecsSinceEpoch();
        cacheInfo.maxScreenSize = currentMaxScreenSize;
        cacheInfo.originalImageSize = originalSize;
        m_sourceSizeCache[imagePath] = cacheInfo;
    }
    
    return targetSize;
}

bool BackgroundManager::fileExists(const QString &path) const
{
    return QFile::exists(path);
}

void BackgroundManager::forceUpdateBackground()
{
    switchBackground();
    Q_EMIT forceUpdated();
}

void BackgroundManager::initGSettings()
{
    if (QGSettings::isSchemaInstalled(BACKGROUND_MATE_SETTINGS)) {
        m_backgroundSettings = new QGSettings(BACKGROUND_MATE_SETTINGS, QByteArray(), this);
        m_backgroundOption = m_backgroundSettings->get(PICTUREOPTIONS).toString();

        g_autoptr (GSettings) settings = g_settings_new_with_path(BACKGROUND_MATE_SETTINGS, BACKGROUND_MATE_PATH);
        if (settings) {
            bool writable = g_settings_is_writable(settings, PICTURE_FILENAME);
            m_shouldSyncAccountBackground = writable;
        }
    } else if (QGSettings::isSchemaInstalled(BACKGROUND_SETTINGS)){
        m_backgroundSettings = new QGSettings(BACKGROUND_SETTINGS, QByteArray(), this);
        m_backgroundOption = m_backgroundSettings->get(PICTUREOPTIONS).toString();

        g_autoptr (GSettings) settings = g_settings_new_with_path(BACKGROUND_SETTINGS, BACKGROUND_PATH);
        if (settings) {
            bool writable = g_settings_is_writable(settings, PICTURE_FILENAME);
            m_shouldSyncAccountBackground = writable;
        }
    } else {
        m_backgroundOption = "scaled";
    }

    if (m_backgroundSettings) {
        connect(m_backgroundSettings, &QGSettings::changed, this, [=](const QString &key){
            if (key == PICTUREFILENAME) {
                m_shouldSyncAccountBackground = true;
                setAccountBackground(m_backgroundSettings->get(PICTUREFILENAME).toString());
            }
            if (key == PICTUREFILENAME || key == PRIMARYCOLOR || key == PICTUREOPTIONS) {
                switchBackground();
            }
        });
    }
}

void BackgroundManager::initBackground()
{
    QString defaultBg;
    auto accountBack = getAccountBackground();
    if (accountBack != "" && QFile::exists(accountBack))
        defaultBg = accountBack;

    if (!QFile::exists(defaultBg))
    {
       qWarning() << "default bg and account bg not exist";
       switchBackground();
       return;
    }

    const QString bridged = pathForQmlWallpaperImage(defaultBg);
    updateImagePaths(bridged, bridged);
    // 在初始化阶段立即更新sourceSize，避免延迟更新导致qml端因为sourcesize延迟改变而reload image
    updateSourceSizes();

    if (defaultBg != accountBack) {
        setAccountBackground(defaultBg);
    }
}

void BackgroundManager::switchBackground()
{
    if (!m_backgroundSettings)
        return;

    setBackgroundOption(m_backgroundSettings->get(PICTUREOPTIONS).toString());

    auto path = m_backgroundSettings->get(PICTUREFILENAME).toString();
    QString localPath = path;
    if (!QFile::exists(path))
        path = getAccountBackground();

    // try fix #124971
    if (!localPath.isEmpty() && (path.isEmpty() || !QFile::exists(path))) {
        path = DEFAULTBACKGROUND;
    }

    if (path.isEmpty()) {
        setUsePureColor(true);
        setColor(QColor(m_backgroundSettings->get(PRIMARYCOLOR).toString()));
        updateImagePaths("", "");
    } else {
        setUsePureColor(false);
        setColor(QColor(m_backgroundSettings->get(PRIMARYCOLOR).toString()));

        if (m_backImagePath.isEmpty() && m_frontImagePath.isEmpty()) {
            const QString bridged = pathForQmlWallpaperImage(path);
            updateImagePaths(bridged, bridged);
        } else {
            // front = new wallpaper (bridge .dib for Qt Quick Image); back = previous front layer
            updateImagePaths(m_frontImagePath, pathForQmlWallpaperImage(path));
        }

//        scheduleUpdateSourceSizes();
        updateSourceSizes();

        qDebug() << __func__ << __LINE__ << m_backImagePath << m_frontImagePath;

        if (path != getAccountBackground()) {
            setAccountBackground(path);
        }
    }
}

QSize BackgroundManager::maxScreenSize() const
{
    QSize maxSize;
    for (QScreen* screen : qApp->screens()) {
        // Consider device pixel ratio for high DPI screens
        // QSize size = screen->size() * screen->devicePixelRatio();
        QSize size = screen->size();
        if (isLeftGreaterThanRightSize(size, maxSize)) {
            maxSize = size;
        }
    }
    return maxSize;
}

QSize BackgroundManager::calculateWallpaperTargetSize(const QSize &originalSize, const QSize &screenSize) const
{
    // Invalid size check
    if (!originalSize.isValid() || !screenSize.isValid()) {
        qWarning() << "Invalid size detected:"
                  << "Original:" << originalSize
                  << "Screen:" << screenSize;
        return QSize();
    }

    // Calculate aspect ratios
    const qreal sourceRatio = static_cast<qreal>(originalSize.width()) / originalSize.height();
    const qreal screenRatio = static_cast<qreal>(screenSize.width()) / screenSize.height();

    // Maximum allowed scale multiplier to prevent excessive memory usage
    const qreal MAX_SCALE_MULTIPLIER = 1.5;

    QSize targetSize;

    if (sourceRatio >= screenRatio) {
        // Image is wider than screen (relative to aspect ratios)
        // Use screen height as base and calculate width to maintain aspect ratio
        int targetHeight = screenSize.height();
        int baseWidth = targetHeight * sourceRatio;

        // Limit maximum width to prevent excessive scaling
        int maxAllowedWidth = screenSize.width() * MAX_SCALE_MULTIPLIER;
        int targetWidth = qMin(baseWidth, maxAllowedWidth);

        // If width was limited, adjust height to maintain aspect ratio
        if (targetWidth < baseWidth) {
            targetHeight = targetWidth / sourceRatio;
        }

        targetSize = QSize(targetWidth, targetHeight);
    } else {
        // Image is taller than screen (relative to aspect ratios)
        // Use screen width as base and calculate height to maintain aspect ratio
        int targetWidth = screenSize.width();
        int baseHeight = targetWidth / sourceRatio;

        // Limit maximum height to prevent excessive scaling
        int maxAllowedHeight = screenSize.height() * MAX_SCALE_MULTIPLIER;
        int targetHeight = qMin(baseHeight, maxAllowedHeight);

        // If height was limited, adjust width to maintain aspect ratio
        if (targetHeight < baseHeight) {
            targetWidth = targetHeight * sourceRatio;
        }

        targetSize = QSize(targetWidth, targetHeight);
    }

    // Verify if aspect ratio is maintained
    qreal targetRatio = static_cast<qreal>(targetSize.width()) / targetSize.height();
    qreal ratioDiff = qAbs(targetRatio - sourceRatio);

    // Allow for a small floating-point error (0.01)
    if (ratioDiff > 0.01) {
        qWarning() << "Aspect ratio mismatch detected:"
                  << "Source:" << sourceRatio
                  << "Target:" << targetRatio;
        // Force aspect ratio correction
        targetSize.setWidth(targetSize.height() * sourceRatio);
    }

    return targetSize;
}

bool BackgroundManager::isLeftGreaterThanRightSize(const QSize &left, const QSize &right) const
{
    if (!right.isValid()) {
        return left.isValid();
    }
    if (!left.isValid()) {
        return false;
    }
    
    // Compare by total area
    return (left.width() * left.height()) > (right.width() * right.height());
}

QString BackgroundManager::normalizeImagePath(const QString &imagePath) const
{
    if (imagePath.isEmpty()) {
        return QString();
    }
    
    QString normalizedPath = imagePath;
    
    // Remove file:// prefix if present
    if (normalizedPath.startsWith("file://")) {
        normalizedPath = normalizedPath.mid(7); // Remove "file://"
    }
    
    return normalizedPath;
}

void BackgroundManager::updateSourceSizes()
{
    // 屏幕尺寸改变和壁纸尺寸改变都需要更新sourceSize
    // note: 更新sourceSize目前是做延迟，避免更新sourceSize以后image reload导致阻塞桌面窗口setGeometry流程，
    // 这里可能需要使用异步加载image的方式实现，即在c++端异步获取对应sourceSize的图片缓存，然后设置到qml中，
    // 现有的sourceSize更新机制需要废弃
    if (!shouldUpdateForScreenChange() && m_frontImagePath == m_backImagePath) {
        return;
    }

    QSize newFrontSize = calculateOptimalSourceSize(m_frontImagePath);
    QSize newBackSize = calculateOptimalSourceSize(m_backImagePath);
    if (m_frontImageSourceSize != newFrontSize) {
        m_frontImageSourceSize = newFrontSize;
        Q_EMIT frontImageSourceSizeChanged();
    }
    
    if (m_backImageSourceSize != newBackSize) {
        m_backImageSourceSize = newBackSize;
        Q_EMIT backImageSourceSizeChanged();
    }
}

void BackgroundManager::scheduleUpdateSourceSizes()
{
    if (m_delayedUpdateTimer) {
        m_delayedUpdateTimer->start();
    }
}

void BackgroundManager::initScreenMonitoring()
{
    ScreenManager* screenManager = ScreenManager::getInstance();
    connect(screenManager, &ScreenManager::screenAdded, this, [this](QScreen *screen, int id) {
        Q_UNUSED(id)
        qDebug() << "Screen added (via ScreenManager):" << (screen ? screen->name() : "unknown");
//        scheduleUpdateSourceSizes();
        updateSourceSizes();

        if (screen) {
            connectScreenDpiSignals(screen);
        }
    });
    
    connect(screenManager, &ScreenManager::screenRemoved, this, [this](QScreen *screen, int id) {
        Q_UNUSED(screen)
        Q_UNUSED(id)
        qDebug() << "Screen removed (via ScreenManager)";
//        scheduleUpdateSourceSizes();
        updateSourceSizes();

    });
    
    connect(screenManager, &ScreenManager::displayModeChanged, this, [this]() {
        qDebug() << "Display mode changed, scheduling update with delay";
//        scheduleUpdateSourceSizes();
        updateSourceSizes();
        //Q_EMIT forceUpdated();
    });
    
    for (QScreen* screen : qApp->screens()) {
        connectScreenDpiSignals(screen);
    }
}

void BackgroundManager::connectScreenDpiSignals(QScreen *screen)
{
    if (!screen) {
        qWarning() << "Cannot connect DPI signals to null screen";
        return;
    }
    
    // Check if screen is valid and has a name (avoid connecting to invalid screens)
    if (screen->name().isEmpty()) {
        qWarning() << "Cannot connect DPI signals to screen with empty name";
        return;
    }
    
    connect(screen, &QScreen::logicalDotsPerInchChanged, this, [this, screen](qreal dpi) {
        qDebug() << "Screen DPI changed for" << screen->name() << "to:" << dpi;
//        scheduleUpdateSourceSizes();
        updateSourceSizes();
    });
    
    qDebug() << "Connected DPI signals for screen:" << screen->name();
}

void BackgroundManager::updateImagePaths(const QString &newBackPath, const QString &newFrontPath)
{
    QString oldBackImagePath = m_backImagePath;
    QString oldFrontImagePath = m_frontImagePath;
    
    m_backImagePath = newBackPath;
    m_frontImagePath = newFrontPath;
    
    if (oldBackImagePath != m_backImagePath) {
        Q_EMIT backImagePathChanged();
    }
    if (oldFrontImagePath != m_frontImagePath) {
        Q_EMIT frontImagePathChanged();
    }
}

QSize BackgroundManager::getCachedOptimalSourceSize(const QString &imagePath) const
{
    if (imagePath.isEmpty()) {
        return QSize();
    }
    
    // Check cache first
    auto it = m_sourceSizeCache.find(imagePath);
    if (it != m_sourceSizeCache.end()) {
        const SourceSizeInfo& cacheInfo = it.value();
        
        // Verify cache validity
        QString normalizedPath = normalizeImagePath(imagePath);
        QFileInfo fileInfo(normalizedPath);
        
        if (fileInfo.exists()) {
            qint64 currentModified = fileInfo.lastModified().toSecsSinceEpoch();
            QSize currentMaxScreenSize = maxScreenSize();
            
            // Check if only screen size changed (file unchanged)
            if (cacheInfo.lastModified == currentModified && 
                cacheInfo.maxScreenSize != currentMaxScreenSize &&
                cacheInfo.originalImageSize.isValid()) {
                
                QSize targetSize = calculateWallpaperTargetSize(cacheInfo.originalImageSize, currentMaxScreenSize);
                if (!targetSize.isValid()) {
                    targetSize = cacheInfo.originalImageSize;
                } else if (isLeftGreaterThanRightSize(targetSize, cacheInfo.originalImageSize)) {
                    targetSize = cacheInfo.originalImageSize;
                }
                
                // Update cache with new result
                SourceSizeInfo newCacheInfo = cacheInfo;
                newCacheInfo.size = targetSize;
                newCacheInfo.maxScreenSize = currentMaxScreenSize;
                m_sourceSizeCache[imagePath] = newCacheInfo;
                
                qDebug() << "Using cached original size and recalculating for screen change:" << imagePath 
                        << "Original:" << cacheInfo.originalImageSize << "Target:" << targetSize;
                return targetSize;
            }
            
            // Cache is fully valid
            if (cacheInfo.lastModified == currentModified && 
                cacheInfo.maxScreenSize == currentMaxScreenSize) {
                qDebug() << "Using fully cached optimal source size for:" << imagePath 
                        << "Size:" << cacheInfo.size;
                return cacheInfo.size;
            } else {
                qDebug() << "Cache invalid for:" << imagePath 
                        << "Modified:" << (cacheInfo.lastModified != currentModified)
                        << "Screen size changed:" << (cacheInfo.maxScreenSize != currentMaxScreenSize);
            }
        }
        
        // Remove invalid cache entry
        m_sourceSizeCache.erase(it);
    }
    
    return QSize(); // No valid cache found
}

void BackgroundManager::clearSourceSizeCache()
{
    if (!m_sourceSizeCache.isEmpty()) {
        qDebug() << "Clearing source size cache (" << m_sourceSizeCache.size() << " entries)";
        m_sourceSizeCache.clear();
    }
}

bool BackgroundManager::shouldUpdateForScreenChange() const
{
    QSize currentMaxScreenSize = maxScreenSize();
    
    // If this is the first time or max screen size changed, we need to update
    if (!m_lastMaxScreenSize.isValid() || m_lastMaxScreenSize != currentMaxScreenSize) {
        qDebug() << "Max screen size changed from" << m_lastMaxScreenSize << "to" << currentMaxScreenSize;
        m_lastMaxScreenSize = currentMaxScreenSize;
        return true;
    }
    
    qDebug() << "Max screen size unchanged:" << currentMaxScreenSize;
    return false;
}

void BackgroundManager::onDelayedUpdateSourceSizes()
{
    qDebug() << "Delayed updateSourceSizes triggered after 50ms";
    updateSourceSizes();
}

