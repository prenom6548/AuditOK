/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QProcess>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSet>
#include <QSysInfo>
#include <QTemporaryDir>
#include <QTextStream>
#include <QVariant>

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <vector>

#include "icon.h"
#define private public
#include "lottie-attached.h"
#undef private
#include "lottie-item.h"
#include "lottie-runtime-test-helpers_p.h"
#include "lottie-runtime-service_p.h"
#include "ukui-quick-items-plugin.h"

using namespace UkuiQuick;

#ifndef LOTTIE_RUNTIME_BENCHMARK_BUILD_TYPE
#define LOTTIE_RUNTIME_BENCHMARK_BUILD_TYPE "unknown"
#endif
#ifndef LOTTIE_RUNTIME_BENCHMARK_COVERAGE
#define LOTTIE_RUNTIME_BENCHMARK_COVERAGE "unknown"
#endif

namespace {

constexpr qint64 kFrameBudgetNanoseconds = 16'670'000;
constexpr int kDefaultHostCount = 64;
constexpr int kDefaultDistinctKeyCount = 56;
constexpr int kDefaultPhysicalSize = 24;
constexpr int kDefaultCadenceFps = 60;
constexpr int kDefaultPlaybackMilliseconds = 1000;
constexpr int kWaitGuardMilliseconds = 120'000;

struct ResourceRecord
{
    QString path;
    QString name;
    qint64 bytes = 0;
    QByteArray sha256;
};

struct HostRecord
{
    std::unique_ptr<QQuickItem> host;
    LottieAttached *controller = nullptr;
    QString source;
    QByteArray assetSha256;
    int keyOrdinal = 0;
    qint64 sourceRequestedNanoseconds = 0;
    qint64 initialReadyLatencyNanoseconds = 0;
    qint64 frameRequestedNanoseconds = 0;
    int frameImageChanges = 0;
    int configuredFrame = 0;
    QString configuredTheme;
    int frameRequestsSubmitted = 0;
    int lastMeasuredFrame = -1;
    int lastCommittedFrame = -1;
    QSize lastCommittedPhysicalSize;
    QString lastCommittedTheme;
    QSize lastMeasuredPhysicalSize;
    QString lastMeasuredTheme;
    bool measuring = false;
    QVector<qint64> frameDeliveryNanoseconds;
    bool loaded = false;
};

QByteArray sha256File(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(1024 * 1024);
        if (chunk.isEmpty() && !file.atEnd()) {
            return {};
        }
        hash.addData(chunk);
    }
    return hash.result();
}

QString firstCpuModel()
{
    QFile cpuInfo(QStringLiteral("/proc/cpuinfo"));
    if (!cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QStringLiteral("unavailable");
    }

    while (!cpuInfo.atEnd()) {
        const QString line = QString::fromUtf8(cpuInfo.readLine()).trimmed();
        if (line.startsWith(QStringLiteral("model name")) ||
            line.startsWith(QStringLiteral("Hardware"))) {
            const int separator = line.indexOf(QLatin1Char(':'));
            return separator >= 0 ? line.mid(separator + 1).trimmed() : line;
        }
    }

    QProcess process;
    process.start(QStringLiteral("lscpu"), QStringList());
    if (process.waitForFinished(5000) && process.exitStatus() == QProcess::NormalExit &&
        process.exitCode() == 0) {
        const QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
        const QStringList lines = output.split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            if (line.trimmed().startsWith(QStringLiteral("Model name:"))) {
                return line.section(QLatin1Char(':'), 1).trimmed();
            }
        }
    }
    return QStringLiteral("unavailable");
}

QString commandOutput(const QString &program, const QStringList &arguments)
{
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForFinished(5000) || process.exitStatus() != QProcess::NormalExit ||
        process.exitCode() != 0) {
        return QStringLiteral("unavailable");
    }
    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    return output.isEmpty() ? QStringLiteral("unavailable") : output;
}

qint64 percentile95(QVector<qint64> values)
{
    if (values.isEmpty()) {
        return 0;
    }
    std::sort(values.begin(), values.end());
    const int index = qMin(values.size() - 1,
                           static_cast<int>(std::ceil(values.size() * 0.95) - 1));
    return values.at(index);
}

QString milliseconds(qint64 nanoseconds)
{
    return QString::number(static_cast<double>(nanoseconds) / 1'000'000.0, 'f', 3);
}

QString markdownCell(QString value)
{
    return value.replace(QLatin1Char('|'), QStringLiteral("\\|"))
        .replace(QLatin1Char('\n'), QLatin1Char(' '));
}

bool waitUntil(const std::function<bool()> &condition, QString *error)
{
    QElapsedTimer guard;
    guard.start();
    while (!condition()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        if (guard.elapsed() >= kWaitGuardMilliseconds) {
            if (error) {
                *error = QStringLiteral("event-loop guard expired while waiting for production hosts");
            }
            return false;
        }
    }
    return true;
}

QSize physicalSize(const QQuickWindow &window, const QQuickItem &host)
{
    return QSize(qMax(1, qRound(host.width() * window.devicePixelRatio())),
                 qMax(1, qRound(host.height() * window.devicePixelRatio())));
}

int currentDistinctRenderKeyCount(const std::vector<HostRecord> &hosts,
                                  const QQuickWindow &window)
{
    QSet<QByteArray> keys;
    for (const HostRecord &record : hosts) {
        const QSize size = physicalSize(window, *record.host);
        QByteArray key = record.assetSha256.toHex();
        key.append('\0');
        key.append(record.controller->theme().toUtf8());
        key.append('\0');
        key.append(QByteArray::number(record.controller->currentFrame()));
        key.append('\0');
        key.append(QByteArray::number(size.width()));
        key.append('x');
        key.append(QByteArray::number(size.height()));
        keys.insert(key);
    }
    return keys.size();
}

QString themeForKey(const LottieAttached *controller, int keyOrdinal)
{
    const QStringList themes = controller->availableThemes();
    if (themes.isEmpty()) {
        return {};
    }
    return themes.at(keyOrdinal % themes.size());
}

void stopBenchmarkClock(LottieAttached *controller)
{
    controller->stopFrameTimer();
    QCoreApplication::removePostedEvents(controller, QEvent::Timer);
}

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("lottie-runtime-benchmark"));
    QCoreApplication::setApplicationVersion(QStringLiteral("Ticket 11"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Manual benchmark for the production LottieRuntimeService pipeline."));
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption assetDirectoryOption(
        {QStringLiteral("a"), QStringLiteral("asset-dir")},
        QStringLiteral("Directory containing .lottie resources."),
        QStringLiteral("directory"),
        qEnvironmentVariable("UKUI_LOTTIE_BENCHMARK_DIR"));
    const QCommandLineOption hostCountOption(
        QStringLiteral("hosts"), QStringLiteral("Number of visible Host Adapter instances."),
        QStringLiteral("count"), QString::number(kDefaultHostCount));
    const QCommandLineOption distinctKeyCountOption(
        QStringLiteral("keys"), QStringLiteral("Number of distinct Render Keys in the profile."),
        QStringLiteral("count"), QString::number(kDefaultDistinctKeyCount));
    const QCommandLineOption physicalSizeOption(
        {QStringLiteral("s"), QStringLiteral("size")},
        QStringLiteral("Steady physical pixel size (24, 32, or 48)."),
        QStringLiteral("pixels"), QString::number(kDefaultPhysicalSize));
    const QCommandLineOption cadenceOption(
        QStringLiteral("cadence-fps"),
        QStringLiteral("Controlled frame-request cadence (30 or 60 FPS)."),
        QStringLiteral("fps"), QString::number(kDefaultCadenceFps));
    const QCommandLineOption profileOption(
        QStringLiteral("profile"),
        QStringLiteral("Profile: steady or resize-theme (kept separate)."),
        QStringLiteral("name"), QStringLiteral("steady"));
    const QCommandLineOption playbackOption(
        QStringLiteral("playback-ms"), QStringLiteral("Continuous-playback sampling duration."),
        QStringLiteral("milliseconds"), QString::number(kDefaultPlaybackMilliseconds));
    const QCommandLineOption reportOption(
        {QStringLiteral("o"), QStringLiteral("output")},
        QStringLiteral("Markdown report path."), QStringLiteral("path"),
        QStringLiteral("/tmp/ukui-quick-ticket11-lottie-runtime-benchmark.md"));
    const QCommandLineOption warmCacheOption(
        QStringLiteral("warm-cache"),
        QStringLiteral("Use the current HOME cache instead of an isolated cold cache."));

    parser.addOption(assetDirectoryOption);
    parser.addOption(hostCountOption);
    parser.addOption(distinctKeyCountOption);
    parser.addOption(physicalSizeOption);
    parser.addOption(cadenceOption);
    parser.addOption(profileOption);
    parser.addOption(playbackOption);
    parser.addOption(reportOption);
    parser.addOption(warmCacheOption);
    parser.process(application);

    const QString requestedAssetDirectory = parser.value(assetDirectoryOption).trimmed();
    if (requestedAssetDirectory.isEmpty()) {
        QTextStream(stderr)
            << "asset directory is required: pass --asset-dir or set "
               "UKUI_LOTTIE_BENCHMARK_DIR"
            << Qt::endl;
        return 2;
    }
    const QString assetDirectory = QDir::cleanPath(requestedAssetDirectory);
    const QDir resourceDir(assetDirectory);
    if (!resourceDir.exists()) {
        QTextStream(stderr) << "asset directory does not exist: " << assetDirectory << Qt::endl;
        return 2;
    }

    const QFileInfoList resourceFiles = resourceDir.entryInfoList(
        {QStringLiteral("*.lottie")}, QDir::Files | QDir::Readable, QDir::Name);
    if (resourceFiles.isEmpty()) {
        QTextStream(stderr) << "asset directory contains no .lottie resources: "
                            << assetDirectory << Qt::endl;
        return 2;
    }

    bool hostCountOk = false;
    bool distinctKeyCountOk = false;
    bool physicalSizeOk = false;
    bool cadenceOk = false;
    bool playbackOk = false;
    const int hostCount = parser.value(hostCountOption).toInt(&hostCountOk);
    const int distinctKeyCount = parser.value(distinctKeyCountOption).toInt(&distinctKeyCountOk);
    const int targetPhysicalSize = parser.value(physicalSizeOption).toInt(&physicalSizeOk);
    const int cadenceFps = parser.value(cadenceOption).toInt(&cadenceOk);
    const QString profile = parser.value(profileOption).trimmed();
    const int playbackMilliseconds = parser.value(playbackOption).toInt(&playbackOk);
    if (!hostCountOk || !distinctKeyCountOk || !physicalSizeOk || !cadenceOk || !playbackOk ||
        hostCount <= 0 || distinctKeyCount <= 0 || distinctKeyCount > hostCount ||
        targetPhysicalSize <= 0 || (cadenceFps != 30 && cadenceFps != 60) ||
        (targetPhysicalSize != 24 && targetPhysicalSize != 32 && targetPhysicalSize != 48) ||
        (profile != QStringLiteral("steady") && profile != QStringLiteral("resize-theme")) ||
        playbackMilliseconds <= 0) {
        QTextStream(stderr) << "invalid hosts/keys/size/cadence/profile/playback options"
                            << Qt::endl;
        return 2;
    }

    QVector<ResourceRecord> resources;
    resources.reserve(resourceFiles.size());
    for (const QFileInfo &fileInfo : resourceFiles) {
        const QByteArray hash = sha256File(fileInfo.absoluteFilePath());
        if (hash.isEmpty()) {
            QTextStream(stderr) << "cannot hash resource: " << fileInfo.absoluteFilePath()
                                << Qt::endl;
            return 2;
        }
        resources.append({fileInfo.absoluteFilePath(), fileInfo.fileName(), fileInfo.size(), hash});
    }

    QTemporaryDir coldCacheHome;
    QString cacheMode = QStringLiteral("cold-cache (isolated temporary HOME)");
    if (!parser.isSet(warmCacheOption)) {
        if (!coldCacheHome.isValid()) {
            QTextStream(stderr) << "cannot create isolated HOME for cold-cache benchmark"
                                << Qt::endl;
            return 2;
        }
        qputenv("HOME", coldCacheHome.path().toUtf8());
    } else {
        cacheMode = QStringLiteral("warm-cache (existing HOME)");
    }

    QQuickWindow window;
    UkuiQuickItemsPlugin plugin;
    plugin.registerTypes("org.ukui.quick.items");
    window.setGeometry(0, 0, 512, 512);
    window.show();
    if (!window.isVisible()) {
        QTextStream(stderr) << "QQuickWindow did not become visible" << Qt::endl;
        return 2;
    }
    const qreal devicePixelRatio = window.devicePixelRatio();
    const qreal logicalHostSize = targetPhysicalSize / devicePixelRatio;
    LottieRuntimeService *service = LottieRuntimeService::instance();
    if (!service) {
        QTextStream(stderr) << "production LottieRuntimeService was not created" << Qt::endl;
        return 3;
    }

    QElapsedTimer benchmarkClock;
    benchmarkClock.start();
    QVector<qint64> sourceToInitialHostReadyNanoseconds;
    std::vector<HostRecord> hosts;
    hosts.reserve(static_cast<size_t>(hostCount));
    int loadedHostCount = 0;

    for (int index = 0; index < hostCount; ++index) {
        const int keyOrdinal = index % distinctKeyCount;
        const ResourceRecord &resource = resources.at(keyOrdinal % resources.size());
        const QString source = resource.path;
        std::unique_ptr<QQuickItem> host;
        LottieAttached *controller = nullptr;
        const qint64 sourceRequested = benchmarkClock.nsecsElapsed();
        if (index % 2 == 0) {
            auto item = std::make_unique<LottieItem>();
            item->setParentItem(window.contentItem());
            item->setSize(QSizeF(logicalHostSize, logicalHostSize));
            item->setSource(source);
            controller = item->lottieAttached();
            host = std::move(item);
        } else {
            auto icon = std::make_unique<Icon>();
            LottieRuntimeTest::setTransparentFallback(icon.get());
            icon->setParentItem(window.contentItem());
            icon->setSize(QSizeF(logicalHostSize, logicalHostSize));
            controller = icon->lottieAttached();
            icon->setSource(QVariant(source));
            host = std::move(icon);
        }
        if (!controller) {
            QTextStream(stderr) << "Host Adapter did not create a Lottie controller at index "
                                << index << Qt::endl;
            return 2;
        }

        hosts.push_back({std::move(host), controller, source, resource.sha256,
                         keyOrdinal, sourceRequested});
        HostRecord *record = &hosts.back();
        QObject::connect(controller, &LottieAttached::loaded, controller,
                         [record, &loadedHostCount, &sourceToInitialHostReadyNanoseconds,
                          &benchmarkClock] {
            if (record->loaded) {
                return;
            }
            record->loaded = true;
            record->initialReadyLatencyNanoseconds =
                benchmarkClock.nsecsElapsed() - record->sourceRequestedNanoseconds;
            sourceToInitialHostReadyNanoseconds.append(
                record->initialReadyLatencyNanoseconds);
            ++loadedHostCount;
        });
        QObject::connect(controller, &LottieAttached::frameImageChanged, controller,
                         [record, &benchmarkClock] {
            ++record->frameImageChanges;
            const QImage committedImage = record->controller->committedImage();
            if (record->measuring && !committedImage.isNull()) {
                record->lastCommittedFrame = record->controller->currentFrame();
                record->lastCommittedPhysicalSize = committedImage.size();
                record->lastCommittedTheme = record->controller->theme();
            }
            if (record->measuring && record->frameRequestedNanoseconds > 0) {
                record->frameDeliveryNanoseconds.append(
                    benchmarkClock.nsecsElapsed() - record->frameRequestedNanoseconds);
                record->frameRequestedNanoseconds = 0;
            }
        });
        controller->setLoop(true);
        controller->setPlaying(false);
    }

    QString waitError;
    QTextStream(stderr) << "benchmark stage: waiting source load" << Qt::endl;
    if (!waitUntil([&] { return loadedHostCount == hostCount; }, &waitError)) {
        QTextStream(stderr) << waitError << Qt::endl;
        return 3;
    }
    QTextStream(stderr) << "benchmark stage: source loaded" << Qt::endl;

    const qint64 initialHostPopulationReadyNanoseconds = benchmarkClock.nsecsElapsed();
    QVector<double> frameRates;
    frameRates.reserve(hosts.size());
    QSize minimumPhysicalSize;
    QSize maximumPhysicalSize;
    QSize minimumResizedPhysicalSize;
    QSize maximumResizedPhysicalSize;
    int themeSwitchableHostCount = 0;
    int minimumFrames = 0;
    int maximumFrames = 0;

    for (int index = 0; index < hostCount; ++index) {
        HostRecord &record = hosts.at(static_cast<size_t>(index));
        const QSize initialPhysicalSize = physicalSize(window, *record.host);
        if (index == 0) {
            minimumPhysicalSize = initialPhysicalSize;
            maximumPhysicalSize = initialPhysicalSize;
            minimumFrames = record.controller->totalFrame();
            maximumFrames = minimumFrames;
        } else {
            minimumPhysicalSize = minimumPhysicalSize.boundedTo(initialPhysicalSize);
            maximumPhysicalSize = maximumPhysicalSize.expandedTo(initialPhysicalSize);
            minimumFrames = qMin(minimumFrames, record.controller->totalFrame());
            maximumFrames = qMax(maximumFrames, record.controller->totalFrame());
        }
        frameRates.append(record.controller->fps());
        if (record.controller->availableThemes().size() > 1) {
            ++themeSwitchableHostCount;
        }
    }

    // Establish a fixed Render Key per Host before measuring cadence. Repeated
    // source paths use fixed theme/frame variants, so 16/32/56 distinct keys
    // are real production keys even though the input directory has 14 files.
    for (int index = 0; index < hostCount; ++index) {
        HostRecord &record = hosts.at(static_cast<size_t>(index));
        record.controller->setPlaying(false);
        record.controller->setFollowHostTheme(false);
        QString theme = themeForKey(record.controller, record.keyOrdinal + 1);
        const QStringList availableThemes = record.controller->availableThemes();
        if (availableThemes.size() > 1 && theme == record.controller->theme()) {
            theme = availableThemes.at((record.keyOrdinal + 2) % availableThemes.size());
        }
        record.configuredTheme = theme;
        if (!theme.isEmpty()) {
            record.controller->setTheme(theme);
        }
        const int totalFrame = qMax(1, record.controller->totalFrame());
        record.configuredFrame = (record.keyOrdinal + 1) % totalFrame;
        record.frameRequestedNanoseconds = benchmarkClock.nsecsElapsed();
        ++record.frameRequestsSubmitted;
        record.controller->setCurrentFrame(record.configuredFrame);
        const QSize resizedPhysicalSize = physicalSize(window, *record.host);
        if (index == 0) {
            minimumResizedPhysicalSize = resizedPhysicalSize;
            maximumResizedPhysicalSize = resizedPhysicalSize;
        } else {
            minimumResizedPhysicalSize = minimumResizedPhysicalSize.boundedTo(resizedPhysicalSize);
            maximumResizedPhysicalSize = maximumResizedPhysicalSize.expandedTo(resizedPhysicalSize);
        }
    }

    QTextStream(stderr) << "benchmark stage: waiting configured frames" << Qt::endl;
    if (!waitUntil([&] {
            const bool hostsReady = std::all_of(hosts.cbegin(), hosts.cend(),
                                                [](const HostRecord &record) {
                return record.controller->active() &&
                    record.controller->currentFrame() == record.configuredFrame &&
                    (record.configuredTheme.isEmpty() ||
                     record.controller->theme() == record.configuredTheme);
            });
            const auto stats = LottieRuntimeTest::frameSchedulerDiagnostics(service);
            return hostsReady && stats.runningJobs == 0 && stats.queuedKeys == 0 &&
                stats.pendingGuiBatches == 0;
        }, &waitError)) {
        QTextStream(stderr) << waitError << Qt::endl;
        return 3;
    }
    QTextStream(stderr) << "benchmark stage: configured frames" << Qt::endl;

    // Production Render Keys use content revision, effective theme, canonical
    // frame and physical pixels. Resource SHA-256 is the content identity from
    // which the revision is derived, so this counts the exact current key set
    // separately from cumulative keys observed throughout playback.
    const int configuredDistinctRenderKeyCount =
        currentDistinctRenderKeyCount(hosts, window);

    const bool coldCache = !parser.isSet(warmCacheOption);
    if (coldCache && !LottieRuntimeTest::testResetFrameSchedulerCache(service)) {
        QTextStream(stderr) << "cannot reset frame cache before cold-cache measurement"
                            << Qt::endl;
        return 3;
    }
    LottieRuntimeTest::resetFrameSchedulerDiagnostics(service);
    for (HostRecord &record : hosts) {
        record.frameDeliveryNanoseconds.clear();
        record.frameRequestedNanoseconds = 0;
        record.frameImageChanges = 0;
        record.frameRequestsSubmitted = 0;
        record.lastMeasuredFrame = -1;
        record.lastCommittedFrame = -1;
        record.lastCommittedPhysicalSize = QSize();
        record.lastCommittedTheme.clear();
        record.lastMeasuredPhysicalSize = physicalSize(window, *record.host);
        record.lastMeasuredTheme = record.controller->theme();
        record.measuring = true;
        const qreal assetFps = qMax<qreal>(0.01, record.controller->fps());
        record.controller->setSpeed(cadenceFps / assetFps);
        record.controller->setPlaying(true);
        stopBenchmarkClock(record.controller);
    }

    const int requestedTickCount = qMax(
        1, qRound(static_cast<double>(playbackMilliseconds * cadenceFps) / 1000.0));
    int submittedTickCount = 0;
    qint64 submittedHostRequestCount = 0;
    qint64 finalCheckpointRequestCount = 0;
    int minimumTickDistinctRenderKeyCount = hostCount;
    int maximumTickDistinctRenderKeyCount = 0;
    QElapsedTimer cadenceClock;
    cadenceClock.start();
    if (profile == QStringLiteral("resize-theme")) {
        minimumResizedPhysicalSize = QSize();
        maximumResizedPhysicalSize = QSize();
    }

    // This is a controlled cadence, not a timer-speed assumption: every tick
    // is counted at the moment all 64 production Hosts submit their newest
    // frame selection. The loop waits for the target cadence while allowing
    // the real GUI/raster pipeline to make progress.
    for (int tick = 0; tick < requestedTickCount; ++tick) {
        const qint64 targetNanoseconds =
            static_cast<qint64>(tick) * 1'000'000'000LL / cadenceFps;
        while (cadenceClock.nsecsElapsed() < targetNanoseconds) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
        }

        for (HostRecord &record : hosts) {
            const bool applyChurn = profile == QStringLiteral("resize-theme") &&
                (tick == 0 || tick % 15 == 0);
            if (applyChurn) {
                const int churnPhase = (tick / 15) % 3;
                const int churnPhysicalSize =
                    24 + (churnPhase == 0 ? 0 : churnPhase == 1 ? 8 : 24);
                const qreal churnLogicalSize = churnPhysicalSize / devicePixelRatio;
                record.host->setSize(QSizeF(churnLogicalSize, churnLogicalSize));
                const QSize actualSize = physicalSize(window, *record.host);
                record.lastMeasuredPhysicalSize = actualSize;
                if (!minimumResizedPhysicalSize.isValid()) {
                    minimumResizedPhysicalSize = actualSize;
                    maximumResizedPhysicalSize = actualSize;
                } else {
                    minimumResizedPhysicalSize = minimumResizedPhysicalSize.boundedTo(actualSize);
                    maximumResizedPhysicalSize = maximumResizedPhysicalSize.expandedTo(actualSize);
                }
                const QString theme = themeForKey(record.controller, record.keyOrdinal + tick + 1);
                if (!theme.isEmpty()) {
                    record.controller->setTheme(theme);
                    record.lastMeasuredTheme = theme;
                }
            }

            const int previousFrame = record.controller->currentFrame();
            record.frameRequestedNanoseconds = benchmarkClock.nsecsElapsed();
            record.controller->onClockElapsed(record.controller->frameIntervalNsecs());
            if (record.controller->currentFrame() != previousFrame) {
                ++record.frameRequestsSubmitted;
                ++submittedHostRequestCount;
                record.lastMeasuredFrame = record.controller->currentFrame();
            } else {
                record.frameRequestedNanoseconds = 0;
            }
        }
        const int tickDistinctRenderKeyCount =
            currentDistinctRenderKeyCount(hosts, window);
        minimumTickDistinctRenderKeyCount = qMin(
            minimumTickDistinctRenderKeyCount, tickDistinctRenderKeyCount);
        maximumTickDistinctRenderKeyCount = qMax(
            maximumTickDistinctRenderKeyCount, tickDistinctRenderKeyCount);
        ++submittedTickCount;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
        for (const HostRecord &record : hosts) {
            stopBenchmarkClock(record.controller);
        }
    }
    const qint64 cadenceElapsedNanoseconds = cadenceClock.nsecsElapsed();

    QTextStream(stderr) << "benchmark stage: waiting cadence drain" << Qt::endl;
    if (!waitUntil([&] {
            const auto currentStats = LottieRuntimeTest::frameSchedulerDiagnostics(service);
            return currentStats.runningJobs == 0 && currentStats.queuedKeys == 0 &&
                currentStats.pendingGuiBatches == 0;
        }, &waitError)) {
        QTextStream(stderr) << waitError << Qt::endl;
        return 3;
    }
    QTextStream(stderr) << "benchmark stage: cadence drained" << Qt::endl;
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    for (HostRecord &record : hosts) {
        record.controller->setPlaying(false);
    }

    // Pausing makes the current frame a presentation checkpoint. A latest-wins
    // playback result may have been discarded while a newer selection was
    // submitted, so explicitly re-submit only Hosts whose current checkpoint
    // is not yet represented by a committed image. This is a deterministic
    // drain phase, not another playback tick or an additional worker.
    for (HostRecord &record : hosts) {
        const QImage committedImage = record.controller->committedImage();
        const bool currentFrameCommitted = record.lastCommittedFrame ==
                record.lastMeasuredFrame &&
            record.lastCommittedPhysicalSize == record.lastMeasuredPhysicalSize &&
            record.lastCommittedTheme == record.lastMeasuredTheme &&
            !committedImage.isNull();
        if (!currentFrameCommitted) {
            record.frameRequestedNanoseconds = benchmarkClock.nsecsElapsed();
            ++record.frameRequestsSubmitted;
            ++submittedHostRequestCount;
            ++finalCheckpointRequestCount;
            record.controller->setCurrentFrame(record.controller->currentFrame());
        }
    }
    if (finalCheckpointRequestCount > 0 &&
        !waitUntil([&] {
            const auto currentStats = LottieRuntimeTest::frameSchedulerDiagnostics(service);
            return currentStats.deferredSubscriptions == 0 &&
                currentStats.runningJobs == 0 && currentStats.queuedKeys == 0 &&
                currentStats.pendingGuiBatches == 0;
        }, &waitError)) {
        QTextStream(stderr) << waitError << Qt::endl;
        return 3;
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    for (HostRecord &record : hosts) {
        record.measuring = false;
    }
    const LottieRuntimeFrameSchedulerStats stats =
        LottieRuntimeTest::frameSchedulerDiagnostics(service);
    const qint64 sourceLoadingP95Nanoseconds =
        LottieRuntimeTest::sourceLoadingP95Nanoseconds(service);
    const qint64 actualCadenceNanoseconds = cadenceElapsedNanoseconds;
    QVector<qint64> hostFrameDeliveryNanoseconds;
    for (const HostRecord &record : hosts) {
        hostFrameDeliveryNanoseconds += record.frameDeliveryNanoseconds;
    }
    const qint64 hostFrameDeliveryP95Nanoseconds =
        percentile95(hostFrameDeliveryNanoseconds);
    int hostsWithoutCommittedFrame = 0;
    int hostsWithCommittedFrameMismatch = 0;
    int hostsWithCommittedSizeMismatch = 0;
    int hostsWithCommittedThemeMismatch = 0;
    int inactiveHosts = 0;
    QStringList committedFrameMismatchExamples;
    for (const HostRecord &record : hosts) {
        if (record.lastCommittedFrame < 0) {
            ++hostsWithoutCommittedFrame;
        } else if (record.lastCommittedFrame != record.lastMeasuredFrame) {
            ++hostsWithCommittedFrameMismatch;
            if (committedFrameMismatchExamples.size() < 4) {
                committedFrameMismatchExamples.append(
                    QStringLiteral("%1:%2/%3/%4")
                        .arg(record.keyOrdinal)
                        .arg(record.lastCommittedFrame)
                        .arg(record.lastMeasuredFrame)
                        .arg(record.controller->currentFrame()));
            }
        }
        if (record.lastCommittedPhysicalSize != record.lastMeasuredPhysicalSize) {
            ++hostsWithCommittedSizeMismatch;
        }
        if (record.lastCommittedTheme != record.lastMeasuredTheme) {
            ++hostsWithCommittedThemeMismatch;
        }
        if (!record.controller->active()) {
            ++inactiveHosts;
        }
    }
    const double averageSubmittedHostTicks = hostCount > 0
        ? static_cast<double>(submittedHostRequestCount) / hostCount
        : 0.0;
    const double measuredHostIntervals = averageSubmittedHostTicks > 1.0
        ? averageSubmittedHostTicks - 1.0
        : averageSubmittedHostTicks;
    const double actualCadenceFps = actualCadenceNanoseconds > 0
        ? measuredHostIntervals * 1'000'000'000.0 /
              static_cast<double>(actualCadenceNanoseconds)
        : 0.0;
    const bool exactDistinctKeyProfileMet =
        configuredDistinctRenderKeyCount == distinctKeyCount &&
        minimumTickDistinctRenderKeyCount == distinctKeyCount &&
        maximumTickDistinctRenderKeyCount == distinctKeyCount;
    const bool everyHostSubmittedPlayback = std::all_of(
        hosts.cbegin(), hosts.cend(), [](const HostRecord &record) {
            return record.frameRequestsSubmitted > 0 && record.lastMeasuredFrame >= 0;
        });
    const bool everyHostCommittedCurrentFrame = std::all_of(
        hosts.cbegin(), hosts.cend(), [&](const HostRecord &record) {
            return record.frameImageChanges > 0 &&
                record.lastCommittedFrame == record.lastMeasuredFrame &&
                record.controller->active() &&
                record.lastCommittedPhysicalSize == record.lastMeasuredPhysicalSize &&
                record.lastCommittedTheme == record.lastMeasuredTheme;
        });
    const bool measuredPipeline = stats.rasterJobCount > 0 &&
        stats.scheduledGuiBatchCount > 0 && everyHostSubmittedPlayback &&
        stats.deferredSubscriptions == 0 && everyHostCommittedCurrentFrame &&
        !hostFrameDeliveryNanoseconds.isEmpty();
    const bool frameBudgetMet = stats.rasterExecutionP95Nanoseconds <= kFrameBudgetNanoseconds &&
        stats.endToEndP95Nanoseconds <= kFrameBudgetNanoseconds &&
        stats.guiBatchDeliveryP95Nanoseconds <= kFrameBudgetNanoseconds &&
        hostFrameDeliveryP95Nanoseconds <= kFrameBudgetNanoseconds;
    const bool benchmarkAcceptanceMet = measuredPipeline && frameBudgetMet &&
        exactDistinctKeyProfileMet;

    QFile report(parser.value(reportOption));
    if (!report.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream(stderr) << "cannot write benchmark report: " << report.fileName()
                            << Qt::endl;
        return 2;
    }
    QTextStream out(&report);
    out << "# Ticket 11 production Lottie runtime benchmark\n\n";
    out << "This is a manual benchmark report. Absolute timing values are evidence for this "
           "machine only; they are not CI hard thresholds.\n\n";
    out << "## Invocation and profile\n\n";
    out << "- Asset directory: `" << markdownCell(assetDirectory) << "`\n";
    out << "- Cache mode: " << cacheMode << "\n";
    out << "- Profile: " << profile << "\n";
    out << "- Host count: " << hostCount << "\n";
    out << "- Distinct Render Key target: " << distinctKeyCount << "\n";
    out << "- Exact configured Render Keys before cadence: "
        << configuredDistinctRenderKeyCount << "\n";
    out << "- Exact current Render Keys per submitted tick: "
        << minimumTickDistinctRenderKeyCount << " to "
        << maximumTickDistinctRenderKeyCount << "\n";
    out << "- Cumulative distinct Render Keys observed over the run: "
        << stats.distinctRenderKeyCount << "\n";
    out << "- Frame cache/session reset before measured cadence: "
        << (coldCache ? "yes (cold-cache)" : "no (warm-cache)") << "\n";
    out << "- Steady physical size: " << targetPhysicalSize << " x " << targetPhysicalSize
        << "\n";
    out << "- Effective logical size: " << QString::number(logicalHostSize, 'f', 3) << " x "
        << QString::number(logicalHostSize, 'f', 3) << "\n";
    out << "- Device pixel ratio: " << devicePixelRatio << "\n";
    out << "- Initial physical size range: " << minimumPhysicalSize.width() << " x "
        << minimumPhysicalSize.height() << " to " << maximumPhysicalSize.width() << " x "
        << maximumPhysicalSize.height() << "\n";
    out << "- Resized physical size range: " << minimumResizedPhysicalSize.width() << " x "
        << minimumResizedPhysicalSize.height() << " to " << maximumResizedPhysicalSize.width()
        << " x " << maximumResizedPhysicalSize.height() << "\n";
    out << "- Animation frame range: " << minimumFrames << " to " << maximumFrames << "\n";
    out << "- FPS range: " << *std::min_element(frameRates.cbegin(), frameRates.cend())
        << " to " << *std::max_element(frameRates.cbegin(), frameRates.cend()) << "\n";
    out << "- Hosts with at least two selectable themes: " << themeSwitchableHostCount << "\n";
    out << "- Controlled cadence: " << cadenceFps << " FPS for " << playbackMilliseconds
        << " ms\n";
    out << "- Playback speed policy: cadence / real asset FPS (production playing=true)\n";
    out << "- Real asset FPS range: " << *std::min_element(frameRates.cbegin(), frameRates.cend())
        << " to " << *std::max_element(frameRates.cbegin(), frameRates.cend()) << "\n\n";

    out << "## Environment\n\n";
    out << "| Field | Value |\n| --- | --- |\n";
    QString hostName = QString::fromLocal8Bit(qgetenv("HOSTNAME"));
    if (hostName.isEmpty()) {
        hostName = QSysInfo::machineHostName();
    }
    out << "| Hostname | " << markdownCell(hostName) << " |\n";
    out << "| Product | " << markdownCell(QSysInfo::prettyProductName()) << " |\n";
    out << "| Kernel | " << markdownCell(QSysInfo::kernelVersion()) << " |\n";
    out << "| Architecture | " << markdownCell(QSysInfo::currentCpuArchitecture()) << " |\n";
    out << "| CPU | " << markdownCell(firstCpuModel()) << " |\n";
    out << "| Qt | " << QT_VERSION_STR << " |\n";
    out << "| Compiler | " << __VERSION__ << " |\n";
    out << "| Build type | " << LOTTIE_RUNTIME_BENCHMARK_BUILD_TYPE << " |\n";
    out << "| Coverage instrumentation | " << LOTTIE_RUNTIME_BENCHMARK_COVERAGE << " |\n";
    out << "| ThorVG | " << markdownCell(commandOutput(QStringLiteral("pkg-config"),
                                                     {QStringLiteral("--modversion"),
                                                      QStringLiteral("thorvg-1")})) << " |\n\n";

    out << "## Resource SHA-256\n\n";
    out << "| Resource | Bytes | SHA-256 |\n| --- | ---: | --- |\n";
    for (const ResourceRecord &resource : resources) {
        out << "| " << markdownCell(resource.name) << " | " << resource.bytes << " | `"
            << resource.sha256.toHex() << "` |\n";
    }
    out << "\n";

    out << "## Measurements\n\n";
    out << "| Metric | Value |\n| --- | ---: |\n";
    out << "| Production source validation/loading executor p95 | "
        << milliseconds(sourceLoadingP95Nanoseconds) << " ms |\n";
    out << "| Initial Host population ready end-to-end | "
        << milliseconds(initialHostPopulationReadyNanoseconds) << " ms |\n";
    out << "| Source request-to-initial-Host-ready p95 (includes first frame commit) | "
        << milliseconds(percentile95(sourceToInitialHostReadyNanoseconds)) << " ms |\n";
    out << "| Production scheduler queue wait p95 | "
        << milliseconds(stats.queueWaitP95Nanoseconds) << " ms |\n";
    out << "| Production ThorVG raster execution p95 | "
        << milliseconds(stats.rasterExecutionP95Nanoseconds) << " ms |\n";
    out << "| Production scheduler request-to-raster-completion p95 | "
        << milliseconds(stats.endToEndP95Nanoseconds) << " ms |\n";
    out << "| Production GUI batch delivery p95 | "
        << milliseconds(stats.guiBatchDeliveryP95Nanoseconds) << " ms |\n";
    out << "| Deferred subscriptions at drain | " << stats.deferredSubscriptions << " |\n";
    out << "| Request-to-current-Host commit p95 | "
        << milliseconds(hostFrameDeliveryP95Nanoseconds) << " ms |\n";
    out << "| Request ticks submitted | " << submittedTickCount << " / target "
        << requestedTickCount << " |\n";
    out << "| Host frame-request setters submitted | " << submittedHostRequestCount << " |\n";
    out << "| Final checkpoint Host requests | " << finalCheckpointRequestCount << " |\n";
    out << "| Average Host playback ticks submitted | "
        << QString::number(averageSubmittedHostTicks, 'f', 3) << " |\n";
    out << "| Every Host submitted playback work | "
        << (everyHostSubmittedPlayback ? "yes" : "NO") << " |\n";
    out << "| Every Host committed its latest measured frame | "
        << (everyHostCommittedCurrentFrame ? "yes" : "NO") << " |\n";
    out << "| Hosts without a measured commit | " << hostsWithoutCommittedFrame << " |\n";
    out << "| Hosts with committed frame mismatch | " << hostsWithCommittedFrameMismatch
        << " |\n";
    out << "| Hosts with committed size mismatch | " << hostsWithCommittedSizeMismatch << " |\n";
    out << "| Hosts with committed theme mismatch | " << hostsWithCommittedThemeMismatch
        << " |\n";
    out << "| Inactive Hosts at drain | " << inactiveHosts << " |\n";
    out << "| Committed frame mismatch examples (key:committed/selected/current) | "
        << markdownCell(committedFrameMismatchExamples.join(QStringLiteral(", "))) << " |\n";
    out << "| Actual submitted tick cadence | " << QString::number(actualCadenceFps, 'f', 3)
        << " FPS |\n";
    out << "| Raster jobs | " << stats.rasterJobCount << " |\n";
    out << "| Cache hits | " << stats.cacheHitCount << " |\n";
    out << "| Coalesced requests | " << stats.coalescedRequestCount << " |\n";
    out << "| Superseded requests | " << stats.supersededRequestCount << " |\n";
    out << "| Dropped/stale results | " << stats.droppedResultCount << " |\n";
    out << "| Scheduled GUI batches | " << stats.scheduledGuiBatchCount << " |\n";
    out << "| Maximum pending GUI batches | " << stats.maxPendingGuiBatches << " |\n";
    out << "| Maximum queued keys | " << stats.maxQueuedKeys << " |\n";
    out << "| Maximum running jobs | " << stats.maxRunningJobs << " |\n";
    out << "| Raster workers | " << stats.rasterWorkerCount << " |\n";
    out << "| Immutable CPU frames | " << stats.immutableFrameCount << " |\n";
    out << "| Frame budget | 16.670 ms |\n";
    out << "| Release target evaluation (ThorVG + raster completion + GUI + Host p95) | "
        << (frameBudgetMet ? "within 16.670 ms" : "OVER 16.670 ms") << " |\n";
    out << "| Exact distinct-key profile evaluation | "
        << (exactDistinctKeyProfileMet ? "met" : "NOT MET") << " |\n";
    out << "| Benchmark acceptance (pipeline + budget + exact distinct-key profile) | "
        << (benchmarkAcceptanceMet ? "within target" : "FAILED") << " |\n\n";

    out << "## Matrix row\n\n";
    out << "| Row | Cache | Profile | Keys target | Configured exact | Tick key range | Cumulative observed | Hosts | Physical px | Cadence target | "
           "Ticks submitted | Actual FPS | Source executor p95 ms | Queue p95 ms | Raster exec p95 ms | "
           "Raster completion p95 ms | GUI batch p95 ms | Host commit p95 ms | Raster jobs | Cache hits | Coalesced | "
           "Superseded | Dropped/stale | GUI batches | Budget |\n";
    out << "| --- | --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | "
           "---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |\n";
    const QString matrixPhysicalSize = profile == QStringLiteral("resize-theme")
        ? QStringLiteral("%1-%2").arg(minimumResizedPhysicalSize.width())
              .arg(maximumResizedPhysicalSize.width())
        : QString::number(targetPhysicalSize);
    out << "| Row | " << markdownCell(cacheMode) << " | " << profile << " | "
        << distinctKeyCount << " | " << configuredDistinctRenderKeyCount << " | "
        << minimumTickDistinctRenderKeyCount << "-" << maximumTickDistinctRenderKeyCount
        << " | " << stats.distinctRenderKeyCount << " | " << hostCount
        << " | " << matrixPhysicalSize << " | "
        << cadenceFps << " | " << submittedTickCount << " | "
        << QString::number(actualCadenceFps, 'f', 3) << " | "
        << milliseconds(sourceLoadingP95Nanoseconds) << " | "
        << milliseconds(stats.queueWaitP95Nanoseconds) << " | "
        << milliseconds(stats.rasterExecutionP95Nanoseconds) << " | "
        << milliseconds(stats.endToEndP95Nanoseconds) << " | "
        << milliseconds(stats.guiBatchDeliveryP95Nanoseconds) << " | "
        << milliseconds(hostFrameDeliveryP95Nanoseconds) << " | " << stats.rasterJobCount << " | "
        << stats.cacheHitCount << " | " << stats.coalescedRequestCount << " | "
        << stats.supersededRequestCount << " | " << stats.droppedResultCount << " | "
        << stats.scheduledGuiBatchCount << " | " << (frameBudgetMet ? "within" : "over")
        << " |\n\n";

    out << "The benchmark uses production `LottieRuntimeService`, production scheduler, "
           "production Host Adapters, and the production ThorVG raster worker. The worker "
           "boundary reports immutable CPU `QImage` frames; no `QSGTexture` is shared across "
           "that boundary.\n";
    report.close();

    QTextStream(stdout) << "benchmark report: " << report.fileName() << Qt::endl;
    QTextStream(stdout) << "resources: " << resources.size() << ", hosts: " << hostCount
                        << ", exact current keys: " << minimumTickDistinctRenderKeyCount
                        << "-" << maximumTickDistinctRenderKeyCount
                        << ", cumulative observed keys: " << stats.distinctRenderKeyCount
                        << Qt::endl;
    QTextStream(stdout) << "ticks: " << submittedTickCount << "/" << requestedTickCount
                        << ", actual cadence: " << QString::number(actualCadenceFps, 'f', 3)
                        << " FPS, ThorVG p95: " << milliseconds(stats.rasterExecutionP95Nanoseconds)
                        << " ms, raster completion p95: "
                        << milliseconds(stats.endToEndP95Nanoseconds)
                        << " ms, Host commit p95: "
                        << milliseconds(hostFrameDeliveryP95Nanoseconds) << " ms, budget: "
                        << (frameBudgetMet ? "within" : "over") << Qt::endl;
    return benchmarkAcceptanceMet ? 0 : 4;
}
