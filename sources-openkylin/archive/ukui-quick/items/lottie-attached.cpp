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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

/*!
    \qmltype Lottie
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 为 Icon 和 LottieItem 提供 Lottie 动画播放控制的附加属性.

    Lottie 不能直接创建。它会跟随宿主的 \c source 属性加载动画，并通过附加属性
    提供播放、循环、速度、帧位置和 dotLottie 主题控制。动画成功加载后，宿主会
    自动重绘当前帧。

    Lottie is not constructible from QML. It is supported only when attached to an
    \l Icon or \l LottieItem; an arbitrary QObject or QQuickItem receives no working
    Lottie host. Asset loading, theme application, JSON work, and rasterization are
    performed behind the host adapter. The GUI thread receives only committed,
    immutable CPU images.

    \section1 使用示例

    \qml
    import QtQuick
    import org.ukui.quick.items 1.0

    LottieItem {
        width: 64
        height: 64
        source: "qrc:/animations/loading.lottie"
        Lottie.playing: true
        Lottie.loop: true
        Lottie.speed: 1.5
    }
    \endqml

    \section1 支持的宿主

    \list
        \li \l Icon ：当图标源为 Lottie 动画时显示并控制动画
        \li \l LottieItem ：专用于显示 Lottie 动画的场景图项目
    \endlist

    \section1 Source replacement and themes

    当 source 被替换为另一个非空值时，已经提交的旧像素会保留到新 source 的
    首个有效目标帧提交；新 source 失败时清除这些像素。LottieItem 在失败时显示
    透明图像，Icon 则继续显示其静态 \c fallbackSource 。清空 source 会回到
    \c Null 。

    Replacing a non-empty source retains the last committed pixels until the new
    source commits its first valid target frame. A failed replacement clears those
    pixels: LottieItem is transparent and Icon uses its static fallback. Clearing
    source returns to \c Null.

    \c theme reports the effective theme. A non-empty assignment stores a manual
    preference and disables host following; an empty assignment selects the source
    default. With \c followHostTheme enabled, the host adapter supplies light or dark
    candidates automatically. Normal and automatic Host modes follow the system
    background; Highlight and ForceHighlight select the dark-background-compatible
    candidate. If a manual preference is unavailable, the source
    default or the available fallback is used and a warning is emitted; a theme
    application failure reports \c Error. A non-checkpoint raster failure only
    drops that frame and keeps \c Ready with the last committed pixels, while an
    initial, manual, paused, or terminal checkpoint failure enters \c Error and
    clears the pixels. \c preferredTheme retains the caller's preference, and
    \l resetThemePreference() clears it and re-enables following. Ordinary Lottie
    JSON has source-default semantics and no theme slots.

    主题应用失败会进入 \c Error ；连续播放中的非 checkpoint 栅格失败只丢弃该帧，
    保持 \c Ready 和已提交像素。初始、手动、暂停或末帧 checkpoint 失败则进入
    \c Error 并清除像素。

    \section1 Implementation boundary

    The runtime service, scheduler, asset worker, raster worker, ThorVG session,
    parser, cache, and immutable asset/frame types are implementation details. They
    are not supported QML API, installed headers, or public ABI. Only \l Icon,
    \l LottieItem, and the documented \c Lottie attached properties are supported.

    \sa LottieItem, Icon
*/

/*!
    \qmlproperty string Lottie::source
    \brief 宿主选中的动画定位符.

    \readonly

    该属性始终跟随宿主选中的 \c source 定位符。它不是文件系统中已
    解析的最终路径：motion-icon 搜索和 QML 相对路径解析会在内部
    asset worker 中完成。请在 Icon 或 LottieItem 上设置动画源，而不是
    直接设置此属性。Icon 的无扩展名 \c *-motion source 若最终解析为同名
    普通主题图标，本属性会清空且 \l status 回到 \c Lottie.Null；原始赋值仍由
    \c Icon.source 保留，其解析类型由 \c Icon.sourceType 报告。
*/

/*!
    \qmlproperty bool Lottie::playing
    \brief 播放意图，而不是内部时钟是否正在运行.

    设置为 \c true 表示调用方希望继续播放，设置为 \c false 表示暂停在当前帧。
    \c true 不保证内部时钟此刻正在运行：加载中、首帧等待、宿主不可渲染、宿主
    被隐藏或动画已经 \c completed 时，时钟都会停止，但播放意图仍会保留。默认值
    为 \c false 。

    \c playing expresses playback intent; it is not a clock-running indicator. The
    visible clock advances only while the source is \c Ready, the host can render,
    and playback is not completed. A hidden or detached host never renders frames;
    \l hiddenPlaybackPolicy decides whether logical playback pauses, catches up when
    shown, or restarts from frame zero. None of these choices clears \c playing.
*/

/*!
    \qmlproperty enumeration Lottie::hiddenPlaybackPolicy
    \brief 宿主隐藏后如何处理动画进度.

    默认值为 \c Lottie.PauseWhenHidden 。宿主自身或任一祖先不可见、窗口隐藏或
    最小化、或者宿主离开场景时，会按照以下策略处理：

    \list
        \li \c Lottie.PauseWhenHidden ：停在当前帧，重新显示后从该帧继续。
        \li \c Lottie.ContinueWhenHidden ：隐藏期间不运行周期性帧定时器、不提交
            栅格任务；只记录单调时钟的起止时间。重新显示时一次性计算应到达的帧，
            并只请求这一帧。系统时间或时区变化不会影响结果。
        \li \c Lottie.RestartWhenShown ：隐藏时立即把逻辑帧重置为第 0 帧并停止
            内部时钟；重新显示后先提交第 0 帧，再按 \l playing 决定是否继续播放。
    \endlist

    三种策略都保留调用方设置的 \l playing 。尺寸暂时为零或超过 Lottie 渲染上限
    只会暂停渲染，不属于“隐藏”，因此不会追帧或重置到第 0 帧。非循环动画使用
    \c ContinueWhenHidden 追到末帧时，仍要等末帧真正提交后才设置 \l completed 。

    使用 \c ContinueWhenHidden 时，修改 \c playing 、 \c loop 、 \c speed 、
    \c currentFrame 或本属性会先结算到修改时刻，再以新值建立计时起点。若
    \c source 在 Host 已隐藏时才完成 metadata 加载，则从 metadata 可用时开始计时。
*/

/*!
    \qmlproperty bool Lottie::loop
    \brief 是否循环播放动画.

    设置为 \c true 时，动画到达末帧后从首帧继续播放。默认值为 \c false 。
*/

/*!
    \qmlproperty real Lottie::speed
    \brief 动画播放速度倍率.

    \c 1.0 表示原始速度。有效值会限制在 \c 0.01 到 \c 1000.0 之间；默认值为
    \c 1.0 。
*/

/*!
    \qmlproperty int Lottie::currentFrame
    \brief 当前显示的动画帧.

    可设置该属性跳转到指定帧，也可在动画就绪前预先选择首个显示帧。播放时，该值
    会随帧进度自动更新；小于 0 的值夹到首帧，超出总帧数的值夹到末帧，不会取模
    回绕。
*/

/*!
    \qmlproperty int Lottie::totalFrame
    \brief 动画的总帧数.

    \readonly

    动画成功加载后提供其总帧数；未加载动画时为 \c 0 。
*/

/*!
    \qmlproperty real Lottie::fps
    \brief 动画的原始帧率.

    \readonly

    动画成功加载后提供其帧率。
*/

/*!
    \qmlproperty string Lottie::theme
    \brief 当前使用的 dotLottie 主题标识.

    读取该属性返回当前实际生效的主题。设置该属性会保存手动偏好并暂停宿主
    主题跟随；空字符串表示手动选择 source-default。普通 Lottie JSON 没有可切换主题。
*/

/*!
    \qmlproperty string Lottie::preferredTheme
    \brief 保存的手动 dotLottie 主题偏好.

    \readonly

    该属性保留调用方最后写入的偏好，即使该偏好当前不可用或宿主主题跟随已重新启用。
*/

/*!
    \qmlproperty bool Lottie::followHostTheme
    \brief 是否使用 Host Adapter 提供的自动浅色/深色主题候选.

    默认值为 \c true 。关闭跟随后会恢复保存的手动偏好；如果此前没有偏好，关闭时
    会冻结当前实际生效主题。启用跟随时，Icon 和 LottieItem 的普通/自动模式跟随
    系统背景，高亮/强制高亮模式选择适合品牌色等深色背景的 \c dark 主题。
*/

/*!
    \qmlmethod void Lottie::resetThemePreference()
    清除手动主题偏好并恢复宿主主题跟随。
*/

/*!
    \qmlproperty list<string> Lottie::availableThemes
    \brief 动画可用的 dotLottie 主题标识列表.

    \readonly

    该列表由动画清单和主题文件解析得到；普通 Lottie JSON 的列表为空。
*/

/*!
    \qmlproperty bool Lottie::active
    \brief 动画是否已视觉就绪.

    \readonly

    仅当当前 source 的首个仍有效目标帧已经提交、\l status 为
    \c Lottie.Ready 时，该属性为 \c true 。

    \c active == (\c status == \c Lottie.Ready ) 。metadata 可以在 \c Loading
    状态、首帧提交之前发布，因此 \c totalFrame 、\c fps 和 \c availableThemes
    可能已经有值，而 \c active 仍为 \c false 。

    \c active == (\c status == \c Lottie.Ready ) . Metadata may be published while
    the status is \c Loading, before the first valid target frame is committed.
*/

/*!
    \qmlsignal Lottie::loaded()

    当前 source 的首个仍有效目标帧提交后发射一次。source 被替换后，新的 source
    重新拥有一次 \c loaded() ；metadata 发布或失败不会提前发射该信号。

    Emitted once after the current source commits its first still-valid target frame.
    Replacing the source starts a new one-shot lifecycle; metadata publication and
    failures do not emit \c loaded().
*/

/*!
    \qmlproperty bool Lottie::completed
    \brief 是否已提交非循环播放的末帧.

    Whether non-looping playback has committed its final frame.

    \readonly

    This property becomes \c true only after the final frame of a non-looping
    playback has been committed to the host. Pausing, loading, hiding the host,
    seeking, and errors before completion do not complete playback. A later
    theme, size, or raster error does not clear an already recorded completion;
    changing the source, enabling looping, selecting a frame, or replaying does.

    \c completed is independent of \c playing. Setting \c playing to \c true after
    completion replays from frame zero; selecting a frame, changing the source, or
    enabling looping resets completion. A replay/reset waits for its new checkpoint
    frame to be committed before the clock runs again.
*/

/*!
    \qmlproperty enumeration Lottie::status
    \brief 当前动画 source 的加载和视觉就绪状态.

    \readonly

    \list
        \li \c Lottie.Null ：没有动画 source，或者 Icon 的 motion source 已解析为普通主题图标
        \li \c Lottie.Loading ：正在加载，或 metadata 已发布但目标帧尚未提交
        \li \c Lottie.Ready ：当前目标帧已经提交并可显示
        \li \c Lottie.Error ：当前 source 或目标帧失败
    \endlist

    The state is asynchronous. \c Loading includes the period after metadata has
    been published and before the requested frame is committed. \c Ready means that
    the current target frame is displayable; it does not mean that playback intent
    is true.
*/

/*!
    \qmlproperty string Lottie::errorString
    \brief 当前错误的安全摘要.

    \readonly

    仅在 \l status 为 \c Lottie.Error 时非空；文本不包含 source 路径或底层
    解析器诊断。

    The value is limited to a safe error category: source validation, theme
    application, frame rasterization, or runtime unavailability. It never exposes a
    path, archive member, parser message, or backend diagnostic.
*/

#include "lottie-attached.h"
#include "icon.h"
#include "lottie-item.h"
#include "lottie-monotonic-clock_p.h"
#include "lottie-renderer.h"
#include "lottie-runtime-service_p.h"
#include "ukui/lottie/lottie-source-reference.h"

#include <QElapsedTimer>
#include <QList>
#include <QQmlContext>
#include <QDir>
#include <QPointer>
#include <QTimer>
#include <QUrl>
#include <QtMath>
#include <limits>
#include <memory>
#include <utility>

namespace UkuiQuick {

namespace {
constexpr qreal kMinPlaybackSpeed = 0.01;
constexpr qreal kMaxPlaybackSpeed = 1000.0;
constexpr int kThemeWarningLruCapacity = 256;

class QElapsedLottieMonotonicClock final : public LottieMonotonicClock
{
public:
    QElapsedLottieMonotonicClock()
    {
        m_timer.start();
    }

    qint64 nowNsecs() const override
    {
        return m_timer.nsecsElapsed();
    }

private:
    QElapsedTimer m_timer;
};

qint64 saturatedElapsedSum(qint64 first, qint64 second)
{
    first = qMax<qint64>(0, first);
    second = qMax<qint64>(0, second);
    if (second > std::numeric_limits<qint64>::max() - first) {
        return std::numeric_limits<qint64>::max();
    }
    return first + second;
}

struct ThemeWarningKey
{
    LottieAssetRevision revision;
    QString preferredTheme;
};

QList<ThemeWarningKey> s_themeWarningLru;

void warnUnavailableManualTheme(const LottieAssetRevision &revision,
                                const QString &preferredTheme)
{
    if (!revision.isValid() || preferredTheme.isEmpty()) {
        return;
    }

    for (int index = 0; index < s_themeWarningLru.size(); ++index) {
        const ThemeWarningKey &key = s_themeWarningLru.at(index);
        if (key.revision == revision && key.preferredTheme == preferredTheme) {
            s_themeWarningLru.append(s_themeWarningLru.takeAt(index));
            return;
        }
    }

    if (s_themeWarningLru.size() >= kThemeWarningLruCapacity) {
        s_themeWarningLru.removeFirst();
    }
    s_themeWarningLru.append({revision, preferredTheme});
    qWarning() << "LottieAttached: preferred theme is unavailable for the current source:"
               << preferredTheme;
}

// 保存 source 所在 QML 文件的位置，供后台线程解析相对路径。这里不读文件。
QUrl lottieSourceBaseUrl(const QObject *object)
{
    QQmlContext *context = qmlContext(object);
    if (!context && object->parent()) {
        context = qmlContext(object->parent());
    }
    if (context) {
        return context->resolvedUrl(QUrl(QStringLiteral("./")));
    }
    return QUrl::fromLocalFile(QDir::currentPath() + QLatin1Char('/'));
}

LottieRuntimeSubscription subscribeLottieRuntimeService()
{
    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    return runtime ? runtime->subscribe() : LottieRuntimeSubscription();
}

void suspendLottieFrameDemand(LottieRuntimeSubscription &subscription)
{
    if (LottieRuntimeService *runtime = LottieRuntimeService::instance()) {
        runtime->suspendFrame(subscription);
    }
}

QString errorSummaryString(LottieErrorSummary summary)
{
    switch (summary) {
    case LottieErrorSummary::None:
        return {};
    case LottieErrorSummary::SourceValidation:
        return QStringLiteral("Lottie source validation failed.");
    case LottieErrorSummary::ThemeApplication:
        return QStringLiteral("Lottie theme application failed.");
    case LottieErrorSummary::FrameRasterization:
        return QStringLiteral("Lottie frame rasterization failed.");
    case LottieErrorSummary::RuntimeUnavailable:
        return QStringLiteral("Lottie runtime unavailable.");
    }
    return {};
}

}


class Q_DECL_HIDDEN LottieAttachedPrivate
{
public:
    enum class FrameSelectionKind {
        Default,
        Explicit,
        Playback
    };

    LottieAttachedPrivate(
        bool supportedHost,
        std::unique_ptr<LottieMonotonicClock> monotonicClock)
        : hostSupported(supportedHost)
        , runtimeSubscription(supportedHost ? subscribeLottieRuntimeService()
                                             : LottieRuntimeSubscription())
        , monotonicClock(std::move(monotonicClock))
    {
        Q_ASSERT(this->monotonicClock);
    }

    qint64 monotonicNowNsecs() const
    {
        return monotonicClock->nowNsecs();
    }

    void emitThemeChanged(LottieAttached *attached)
    {
        ++themeNotificationEpoch;
        Q_EMIT attached->themeChanged();
    }

    bool hostSupported = false;
    LottieRuntimeSubscription runtimeSubscription;
    LottieAsset asset;
    QString source;
    bool playing = false;
    bool loop = false;
    qreal speed = 1.0;
    int currentFrame = 0;
    int totalFrame = 0;
    qreal fps = 30.0;
    QString theme;
    LottieAssetRevision assetRevision;
    QString requestedTheme;
    QString automaticThemeCandidate;
    bool hasRequestedTheme = false;
    bool followHostTheme = true;
    QStringList availableThemes;
    LottieAttached::Status status = LottieAttached::Null;
    QString errorString;
    LottieErrorSummary errorSummary = LottieErrorSummary::None;
    bool loaded = false;
    bool readyWasPublished = false;
    bool readyStatusNotificationPending = false;
    bool readyErrorNotificationPending = false;
    bool readyActiveNotificationPending = false;
    bool loadedNotificationPending = false;
    bool active = false;
    bool completed = false;
    LottieAttached::HiddenPlaybackPolicy hiddenPlaybackPolicy =
        LottieAttached::PauseWhenHidden;
    bool sceneActive = false;
    QImage currentImage;
    QSize currentImageSize;
    int committedFrame = -1;
    QSize hostRenderSize;
    QTimer asyncClock;
    QElapsedTimer frameClock;
    qint64 elapsedRemainderNsecs = 0;
    std::unique_ptr<LottieMonotonicClock> monotonicClock;
    qint64 hiddenPlaybackStartedAtNsecs = -1;
    qint64 hiddenPlaybackRemainderNsecs = 0;
    quint64 sourceGeneration = 0;
    quint64 themeGeneration = 0;
    // 用户偏好或“跟随系统主题”发生变化时，最终使用的主题可能并没有变。
    // 因此分别记录“偏好变化”和“实际渲染主题变化”，避免误丢仍然有效的帧。
    quint64 renderConfigurationGeneration = 0;
    quint64 themeNotificationEpoch = 0;
    quint64 frameGeneration = 0;
    quint64 playbackGeneration = 0;
    bool frameRequestPending = false;
    int requestedFrame = 0;
    QSize requestedFrameSize;
    quint64 requestedFrameGeneration = 0;
    bool clockRunning = false;
    int clockIntervalMsecs = 0;
    FrameSelectionKind frameSelectionKind = FrameSelectionKind::Default;
    bool checkpointPending = false;
    bool initialPresentationPending = false;
};

LottieAttached::LottieAttached(QObject *parent)
    : LottieAttached(parent, std::make_unique<QElapsedLottieMonotonicClock>())
{
}

LottieAttached::LottieAttached(
    QObject *parent,
    std::unique_ptr<LottieMonotonicClock> monotonicClock)
    : QObject(parent)
    , d(new LottieAttachedPrivate(qobject_cast<LottieItem *>(parent) != nullptr ||
                                  qobject_cast<Icon *>(parent) != nullptr,
                                  std::move(monotonicClock)))
{
    d->asyncClock.setTimerType(Qt::PreciseTimer);
    connect(&d->asyncClock, &QTimer::timeout,
            this, &LottieAttached::onFrameTimerTimeout);
}

LottieAttached::~LottieAttached()
{
    stopFrameTimer();
    delete d;
    d = nullptr;
}

QString LottieAttached::source() const
{
    return d->source;
}

bool LottieAttached::playing() const
{
    return d->playing;
}

bool LottieAttached::loop() const
{
    return d->loop;
}

qreal LottieAttached::speed() const
{
    return d->speed;
}

int LottieAttached::currentFrame() const
{
    return d->currentFrame;
}

int LottieAttached::totalFrame() const
{
    return d->totalFrame;
}

qreal LottieAttached::fps() const
{
    return d->fps;
}

QString LottieAttached::theme() const
{
    return d->theme;
}

QString LottieAttached::preferredTheme() const
{
    return d->hasRequestedTheme ? d->requestedTheme : QString();
}

bool LottieAttached::followHostTheme() const
{
    return d->followHostTheme;
}

QStringList LottieAttached::availableThemes() const
{
    return d->availableThemes;
}

LottieAttached::Status LottieAttached::status() const
{
    return d->status;
}

QString LottieAttached::errorString() const
{
    return d->errorString;
}

bool LottieAttached::active() const
{
    return d->active;
}

bool LottieAttached::completed() const
{
    return d->completed;
}

LottieAttached::HiddenPlaybackPolicy LottieAttached::hiddenPlaybackPolicy() const
{
    return d->hiddenPlaybackPolicy;
}

void LottieAttached::setHiddenPlaybackPolicy(HiddenPlaybackPolicy policy)
{
    if (!d->hostSupported || d->hiddenPlaybackPolicy == policy) {
        return;
    }
    if (policy != PauseWhenHidden && policy != ContinueWhenHidden &&
        policy != RestartWhenShown) {
        return;
    }

    const HiddenPlaybackPolicy oldPolicy = d->hiddenPlaybackPolicy;
    qint64 elapsedNsecs = -1;
    if (!d->sceneActive && oldPolicy == ContinueWhenHidden) {
        elapsedNsecs = takeHiddenPlaybackElapsed();
    }

    // 先写入新策略，再处理帧位置。这样同步信号回调读到的是同一份新状态，
    // 不会看到“帧已经重置，但策略还是旧值”的中间状态。
    d->hiddenPlaybackPolicy = policy;
    QPointer<LottieAttached> guard(this);
    if (!d->sceneActive) {
        if (policy == RestartWhenShown) {
            resetPlaybackForHiddenRestart();
        } else if (elapsedNsecs >= 0) {
            applyHiddenPlaybackElapsed(elapsedNsecs, false, false);
        }
        if (!guard || !guard->d || guard->d->hiddenPlaybackPolicy != policy) {
            return;
        }
        if (!guard->d->sceneActive && policy == ContinueWhenHidden) {
            startHiddenPlaybackTimer();
        }
    }

    Q_EMIT hiddenPlaybackPolicyChanged();
}

void LottieAttached::setSource(const QString &source)
{
    if (!d->hostSupported || d->source == source) {
        return;
    }

    const bool retainPixels = !source.isEmpty() && !d->currentImage.isNull();
    const int oldCurrentFrame = d->currentFrame;
    const int oldTotalFrame = d->totalFrame;
    const qreal oldFps = d->fps;
    const QString oldTheme = d->theme;
    const QStringList oldAvailableThemes = d->availableThemes;
    const Status oldStatus = d->status;
    const QString oldErrorString = d->errorString;
    const bool oldActive = d->active;
    const bool oldCompleted = d->completed;
    const bool hadPixels = !d->currentImage.isNull();

    stopFrameTimer();
    cancelHiddenPlaybackTimer();
    ++d->sourceGeneration;
    ++d->frameGeneration;
    d->runtimeSubscription = subscribeLottieRuntimeService();
    d->source = source;
    d->asset = LottieAsset();
    d->loaded = false;
    d->readyWasPublished = false;
    d->readyStatusNotificationPending = false;
    d->readyErrorNotificationPending = false;
    d->readyActiveNotificationPending = false;
    d->loadedNotificationPending = false;
    d->frameRequestPending = false;
    d->currentFrame = 0;
    d->frameSelectionKind = LottieAttachedPrivate::FrameSelectionKind::Default;
    d->checkpointPending = !source.isEmpty();
    d->initialPresentationPending = !source.isEmpty();
    d->totalFrame = 0;
    d->fps = 30.0;
    d->theme.clear();
    d->assetRevision = LottieAssetRevision();
    d->availableThemes.clear();
    if (!retainPixels) {
        d->currentImage = QImage();
        d->currentImageSize = QSize();
    }
    d->committedFrame = -1;
    d->status = source.isEmpty() ? Null : Loading;
    d->errorString.clear();
    d->errorSummary = LottieErrorSummary::None;
    d->active = false;
    d->completed = false;

    QPointer<LottieAttached> guard(this);
    const quint64 sourceGeneration = d->sourceGeneration;
    const auto isCurrent = [&] {
        return guard && guard->d && guard->d->sourceGeneration == sourceGeneration;
    };
    Q_EMIT sourceChanged();
    if (!isCurrent()) {
        return;
    }
    if (oldCurrentFrame != d->currentFrame) {
        Q_EMIT currentFrameChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (oldTotalFrame != d->totalFrame) {
        Q_EMIT totalFrameChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (!qFuzzyCompare(oldFps, d->fps)) {
        Q_EMIT fpsChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (oldTheme != d->theme) {
        d->emitThemeChanged(this);
        if (!isCurrent()) {
            return;
        }
    }
    if (oldAvailableThemes != d->availableThemes) {
        Q_EMIT availableThemesChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (hadPixels && !retainPixels) {
        Q_EMIT frameImageChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (oldStatus != d->status) {
        Q_EMIT statusChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (oldErrorString != d->errorString) {
        Q_EMIT errorStringChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (oldActive != d->active) {
        Q_EMIT activeChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (oldCompleted != d->completed) {
        Q_EMIT completedChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (!source.isEmpty()) {
        requestSourceAsync();
    }
}

void LottieAttached::setPlaying(bool playing)
{
    if (!d->hostSupported) {
        return;
    }
    if (d->playing == playing) {
        if (playing && d->completed) {
            beginReplay();
        }
        if (playing && !d->sceneActive) {
            startHiddenPlaybackTimer();
        }
        return;
    }
    if (!playing && !d->sceneActive) {
        const bool previousPlaying = d->playing;
        QPointer<LottieAttached> settleGuard(this);
        const qint64 elapsedNsecs = takeHiddenPlaybackElapsed();
        if (elapsedNsecs >= 0) {
            applyHiddenPlaybackElapsed(elapsedNsecs, false, false);
            if (!settleGuard || !settleGuard->d ||
                settleGuard->d->playing != previousPlaying) {
                return;
            }
        }
    }
    const bool replayCompleted = playing && d->completed;
    d->playing = playing;
    const quint64 playbackGeneration = ++d->playbackGeneration;
    const quint64 sourceGeneration = d->sourceGeneration;
    const bool restartFrame = d->frameRequestPending;
    if (!playing && (restartFrame || d->committedFrame != d->currentFrame)) {
        d->checkpointPending = true;
    }
    d->frameRequestPending = false;
    QPointer<LottieAttached> guard(this);
    if (replayCompleted) {
        const quint64 replaySourceGeneration = sourceGeneration;
        const quint64 replayPlaybackGeneration = playbackGeneration + 1;
        beginReplay();
        if (!guard || !guard->d) {
            return;
        }

        const auto isReplayStateCurrent = [&] {
            return guard && guard->d &&
                guard->d->sourceGeneration == replaySourceGeneration &&
                guard->d->playbackGeneration == replayPlaybackGeneration &&
                guard->d->currentFrame == 0 &&
                guard->d->frameSelectionKind ==
                    LottieAttachedPrivate::FrameSelectionKind::Playback &&
                !guard->d->completed;
        };
        if (!isReplayStateCurrent()) {
            return;
        }
        Q_EMIT playingChanged();
        if (!isReplayStateCurrent()) {
            return;
        }
        if (!guard->d->sceneActive) {
            guard->startHiddenPlaybackTimer();
        }
        return;
    }
    Q_EMIT playingChanged();
    if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
        guard->d->playbackGeneration != playbackGeneration) {
        return;
    }
    if (restartFrame) {
        restartSelectedFrameRequest();
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
            guard->d->playbackGeneration != playbackGeneration) {
            return;
        }
    }
    if (!d->sceneActive) {
        startHiddenPlaybackTimer();
    }
    updateFrameTimer();
}

void LottieAttached::setLoop(bool loop)
{
    if (!d->hostSupported) {
        return;
    }
    if (d->loop == loop) {
        return;
    }
    const bool previousLoop = d->loop;
    if (!d->sceneActive) {
        QPointer<LottieAttached> settleGuard(this);
        const qint64 elapsedNsecs = takeHiddenPlaybackElapsed();
        if (elapsedNsecs >= 0) {
            applyHiddenPlaybackElapsed(elapsedNsecs, false, false);
            if (!settleGuard || !settleGuard->d ||
                settleGuard->d->loop != previousLoop) {
                return;
            }
        }
    }
    const bool completionChanged = loop && d->completed;
    d->loop = loop;
    const quint64 playbackGeneration = ++d->playbackGeneration;
    const quint64 sourceGeneration = d->sourceGeneration;
    const bool restartFrame = d->frameRequestPending;
    d->frameRequestPending = false;
    if (completionChanged) {
        d->completed = false;
    }

    QPointer<LottieAttached> guard(this);
    Q_EMIT loopChanged();
    if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
        guard->d->playbackGeneration != playbackGeneration) {
        return;
    }
    if (completionChanged) {
        Q_EMIT completedChanged();
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
            guard->d->playbackGeneration != playbackGeneration) {
            return;
        }
    }
    if (restartFrame) {
        restartSelectedFrameRequest();
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
            guard->d->playbackGeneration != playbackGeneration) {
            return;
        }
    }
    if (!d->sceneActive) {
        startHiddenPlaybackTimer();
    }
    updateFrameTimer();
}

void LottieAttached::setSpeed(qreal speed)
{
    if (!d->hostSupported) {
        return;
    }
    if (!qIsFinite(speed)) {
        speed = 1.0;
    }
    speed = qBound(kMinPlaybackSpeed, speed, kMaxPlaybackSpeed);
    if (qFuzzyCompare(d->speed, speed)) {
        return;
    }

    const qreal previousSpeed = d->speed;
    if (!d->sceneActive) {
        QPointer<LottieAttached> settleGuard(this);
        const qint64 elapsedNsecs = takeHiddenPlaybackElapsed();
        if (elapsedNsecs >= 0) {
            applyHiddenPlaybackElapsed(elapsedNsecs, false, false);
            if (!settleGuard || !settleGuard->d ||
                !qFuzzyCompare(settleGuard->d->speed, previousSpeed)) {
                return;
            }
        }
    }

    d->speed = speed;
    QPointer<LottieAttached> guard(this);
    Q_EMIT speedChanged();
    if (!guard || !guard->d) {
        return;
    }

    d->frameClock.invalidate();
    d->elapsedRemainderNsecs = 0;
    if (!d->sceneActive) {
        startHiddenPlaybackTimer();
    }
    updateFrameTimer();
}

void LottieAttached::setCurrentFrame(int frame)
{
    if (!d->hostSupported) {
        return;
    }
    frame = qMax(0, frame);
    if (d->totalFrame > 0) {
        frame = qBound(0, frame, d->totalFrame - 1);
    }

    cancelHiddenPlaybackTimer();

    const bool changed = d->currentFrame != frame;
    const bool completionChanged = d->completed;
    d->currentFrame = frame;
    d->frameSelectionKind = LottieAttachedPrivate::FrameSelectionKind::Explicit;
    d->checkpointPending = !d->source.isEmpty();
    ++d->frameGeneration;
    d->frameRequestPending = false;
    stopFrameTimer();
    d->completed = false;

    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 playbackGeneration = d->playbackGeneration;
    const quint64 frameGeneration = d->frameGeneration;
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;
    const int selectedFrame = d->currentFrame;
    QPointer<LottieAttached> guard(this);
    const auto isCurrentState = [&] {
        return guard && guard->d &&
            guard->d->sourceGeneration == sourceGeneration &&
            guard->d->playbackGeneration == playbackGeneration &&
            guard->d->currentFrame == selectedFrame &&
            guard->d->frameSelectionKind == LottieAttachedPrivate::FrameSelectionKind::Explicit &&
            !guard->d->completed;
    };
    const auto isCurrent = [&](quint64 expectedFrameGeneration) {
        return isCurrentState() && guard->d->frameGeneration == expectedFrameGeneration &&
            guard->d->renderConfigurationGeneration == renderConfigurationGeneration;
    };
    if (changed) {
        Q_EMIT currentFrameChanged();
        if (!isCurrentState()) {
            return;
        }
    }
    if (completionChanged) {
        Q_EMIT completedChanged();
        if (!isCurrentState()) {
            return;
        }
    }

    if (!d->loaded) {
        return;
    }

    const bool alreadyCommitted = d->status == Ready &&
        !d->currentImage.isNull() &&
        d->committedFrame == d->currentFrame &&
        d->currentImageSize == d->hostRenderSize;
    if (alreadyCommitted) {
        d->checkpointPending = false;
        updateFrameTimer();
        return;
    }

    requestCurrentFrameAsync();
    if (!d->sceneActive) {
        startHiddenPlaybackTimer();
    }
}

void LottieAttached::setTheme(const QString &theme)
{
    if (!d->hostSupported) {
        return;
    }
    const bool preferenceChanged = !d->hasRequestedTheme || d->requestedTheme != theme;
    const bool followingChanged = d->followHostTheme;
    if (!preferenceChanged && !followingChanged) {
        return;
    }

    d->requestedTheme = theme;
    d->hasRequestedTheme = true;
    d->followHostTheme = false;
    const quint64 themeGeneration = ++d->themeGeneration;
    const quint64 sourceGeneration = d->sourceGeneration;

    bool effectiveThemeChanged = false;
    if (d->loaded) {
        effectiveThemeChanged = applyAsyncEffectiveTheme(themeGeneration, false);
    }

    QPointer<LottieAttached> guard(this);
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;
    const auto isCurrent = [&] {
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
            return false;
        }
        return effectiveThemeChanged
            ? guard->d->renderConfigurationGeneration == renderConfigurationGeneration
            : guard->d->themeGeneration == themeGeneration;
    };
    if (preferenceChanged) {
        Q_EMIT preferredThemeChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (followingChanged) {
        Q_EMIT followHostThemeChanged();
        if (!isCurrent()) {
            return;
        }
    }

    if (!d->loaded) {
        return;
    }

    if (!effectiveThemeChanged) {
        return;
    }
    d->emitThemeChanged(this);
    if (!isCurrent()) {
        return;
    }
    requestCurrentFrameAsync();
}

void LottieAttached::setHostTheme(const QString &theme)
{
    if (!d->hostSupported) {
        return;
    }
    if (d->automaticThemeCandidate == theme) {
        return;
    }

    d->automaticThemeCandidate = theme;
    if (!d->followHostTheme) {
        return;
    }

    const quint64 themeGeneration = ++d->themeGeneration;
    if (!d->loaded) {
        return;
    }
    applyAsyncEffectiveTheme(themeGeneration);
}

void LottieAttached::setFollowHostTheme(bool followHostTheme)
{
    if (!d->hostSupported) {
        return;
    }
    if (d->followHostTheme == followHostTheme) {
        return;
    }

    bool preferenceChanged = false;
    if (!followHostTheme && !d->hasRequestedTheme) {
        d->requestedTheme = d->theme;
        d->hasRequestedTheme = true;
        preferenceChanged = true;
    }

    d->followHostTheme = followHostTheme;
    const quint64 themeGeneration = ++d->themeGeneration;
    const quint64 sourceGeneration = d->sourceGeneration;
    bool effectiveThemeChanged = false;
    if (d->loaded) {
        effectiveThemeChanged = applyAsyncEffectiveTheme(themeGeneration, false);
    }

    QPointer<LottieAttached> guard(this);
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;
    const auto isCurrent = [&] {
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
            return false;
        }
        return effectiveThemeChanged
            ? guard->d->renderConfigurationGeneration == renderConfigurationGeneration
            : guard->d->themeGeneration == themeGeneration;
    };
    if (preferenceChanged) {
        Q_EMIT preferredThemeChanged();
        if (!isCurrent()) {
            return;
        }
    }
    Q_EMIT followHostThemeChanged();
    if (!isCurrent()) {
        return;
    }

    if (!d->loaded) {
        return;
    }
    if (!effectiveThemeChanged) {
        return;
    }
    d->emitThemeChanged(this);
    if (!isCurrent()) {
        return;
    }
    requestCurrentFrameAsync();
}

void LottieAttached::resetThemePreference()
{
    if (!d->hostSupported) {
        return;
    }
    if (!d->hasRequestedTheme && d->followHostTheme) {
        return;
    }

    const bool preferenceChanged = d->hasRequestedTheme;
    const bool followingChanged = !d->followHostTheme;
    d->requestedTheme.clear();
    d->hasRequestedTheme = false;
    d->followHostTheme = true;
    const quint64 themeGeneration = ++d->themeGeneration;
    const quint64 sourceGeneration = d->sourceGeneration;
    bool effectiveThemeChanged = false;
    if (d->loaded) {
        effectiveThemeChanged = applyAsyncEffectiveTheme(themeGeneration, false);
    }

    QPointer<LottieAttached> guard(this);
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;
    const auto isCurrent = [&] {
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
            return false;
        }
        return effectiveThemeChanged
            ? guard->d->renderConfigurationGeneration == renderConfigurationGeneration
            : guard->d->themeGeneration == themeGeneration;
    };
    if (preferenceChanged) {
        Q_EMIT preferredThemeChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (followingChanged) {
        Q_EMIT followHostThemeChanged();
        if (!isCurrent()) {
            return;
        }
    }

    if (!d->loaded) {
        return;
    }
    if (!effectiveThemeChanged) {
        return;
    }
    d->emitThemeChanged(this);
    if (!isCurrent()) {
        return;
    }
    requestCurrentFrameAsync();
}

QString LottieAttached::requestedThemeForSource() const
{
    if (d->followHostTheme) {
        return d->automaticThemeCandidate;
    }
    return d->hasRequestedTheme ? d->requestedTheme : d->theme;
}

QString LottieAttached::effectiveThemeForAsset(const LottieAsset &asset,
                                               bool *manualPreferenceUnavailable) const
{
    const QString requestedTheme = requestedThemeForSource();
    const bool unavailable = !d->followHostTheme && d->hasRequestedTheme &&
        !d->requestedTheme.isEmpty() && !asset.canUseTheme(d->requestedTheme);
    if (manualPreferenceUnavailable) {
        *manualPreferenceUnavailable = unavailable;
    }
    return asset.preferredTheme(requestedTheme);
}

bool LottieAttached::applyAsyncEffectiveTheme(quint64 themeGeneration, bool notifyTheme)
{
    if (!d->loaded || d->themeGeneration != themeGeneration) {
        return false;
    }

    bool manualPreferenceUnavailable = false;
    const QString effectiveTheme = effectiveThemeForAsset(d->asset, &manualPreferenceUnavailable);
    if (manualPreferenceUnavailable) {
        warnUnavailableManualTheme(d->asset.revision(), d->requestedTheme);
    }

    if (d->theme == effectiveTheme) {
        return false;
    }

    // The old theme must leave the resident-demand set before any synchronous
    // notification can let another Host request a new session key.
    suspendLottieFrameDemand(d->runtimeSubscription);
    d->theme = effectiveTheme;
    d->asset.setTheme(effectiveTheme);
    ++d->renderConfigurationGeneration;
    ++d->frameGeneration;
    d->frameRequestPending = false;
    d->checkpointPending = true;
    stopFrameTimer();
    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;

    if (d->status == Error) {
        const bool hadError = !d->errorString.isEmpty();
        d->status = Loading;
        d->errorString.clear();
        d->errorSummary = LottieErrorSummary::None;
        d->active = false;
        d->checkpointPending = true;

        QPointer<LottieAttached> recoveryGuard(this);
        Q_EMIT statusChanged();
        if (!recoveryGuard || !recoveryGuard->d ||
            recoveryGuard->d->sourceGeneration != sourceGeneration ||
            recoveryGuard->d->renderConfigurationGeneration !=
                renderConfigurationGeneration) {
            return false;
        }
        if (hadError) {
            Q_EMIT errorStringChanged();
            if (!recoveryGuard || !recoveryGuard->d ||
                recoveryGuard->d->sourceGeneration != sourceGeneration ||
                recoveryGuard->d->renderConfigurationGeneration !=
                    renderConfigurationGeneration) {
                return false;
            }
        }
    }

    if (!notifyTheme) {
        return true;
    }

    QPointer<LottieAttached> guard(this);
    const auto isCurrent = [&] {
        return guard && guard->d &&
            guard->d->sourceGeneration == sourceGeneration &&
            guard->d->renderConfigurationGeneration == renderConfigurationGeneration;
    };
    d->emitThemeChanged(this);
    if (!isCurrent()) {
        return false;
    }
    requestCurrentFrameAsync();
    return isCurrent();
}

QImage LottieAttached::committedImage() const
{
    return d->currentImage;
}

void LottieAttached::setHostRenderSize(const QSize &size)
{
    if (!d->hostSupported) {
        return;
    }
    if (d->hostRenderSize == size) {
        return;
    }
    d->hostRenderSize = size;
    ++d->frameGeneration;
    d->frameRequestPending = false;
    if (d->loaded && d->status == Ready) {
        d->checkpointPending = true;
        stopFrameTimer();
    }
    if (!LottieRenderer::isRenderSizeAllowed(size)) {
        suspendLottieFrameDemand(d->runtimeSubscription);
        return;
    }
    requestCurrentFrameAsync();
}

void LottieAttached::setSceneActive(bool active)
{
    if (!d->hostSupported) {
        return;
    }
    if (d->sceneActive == active) {
        return;
    }

    d->sceneActive = active;
    ++d->frameGeneration;
    d->frameRequestPending = false;
    QPointer<LottieAttached> guard(this);
    if (active) {
        const qint64 elapsedNsecs = takeHiddenPlaybackElapsed();
        if (elapsedNsecs >= 0) {
            const bool resumed = applyHiddenPlaybackElapsed(elapsedNsecs);
            if (!guard || !guard->d) {
                return;
            }
            if (!guard->d->sceneActive) {
                return;
            }
            if (resumed) {
                return;
            }
        }
        if (d->loaded && d->status == Ready) {
            d->checkpointPending = true;
        }
        requestCurrentFrameAsync();
    } else {
        if (d->hiddenPlaybackPolicy == RestartWhenShown) {
            // 先释放旧帧的 session demand，再发出重置通知。同步回调即使马上
            // 重新显示 Host，也只会为第 0 帧建立新的 demand。
            suspendLottieFrameDemand(d->runtimeSubscription);
            resetPlaybackForHiddenRestart();
            if (!guard || !guard->d || guard->d->sceneActive) {
                return;
            }
        } else {
            startHiddenPlaybackTimer();
            suspendLottieFrameDemand(d->runtimeSubscription);
        }
        stopFrameTimer();
    }
}

void LottieAttached::onFrameTimerTimeout()
{
    if (!d->playing || !d->loaded || d->status != Ready || !d->sceneActive ||
        !LottieRenderer::isRenderSizeAllowed(d->hostRenderSize) ||
        d->checkpointPending || d->completed) {
        updateFrameTimer();
        return;
    }

    if (!d->frameClock.isValid()) {
        d->frameClock.start();
        return;
    }

    const qint64 elapsedNsecs = d->frameClock.nsecsElapsed();
    d->frameClock.restart();

    onClockElapsed(elapsedNsecs);
}

void LottieAttached::onClockElapsed(qint64 elapsedNsecs)
{
    if (!d->playing || !d->loaded || d->status != Ready || !d->sceneActive ||
        !LottieRenderer::isRenderSizeAllowed(d->hostRenderSize) ||
        d->checkpointPending || d->completed) {
        updateFrameTimer();
        return;
    }

    const qint64 interval = frameIntervalNsecs();
    if (interval <= 0) {
        updateFrameTimer();
        return;
    }

    if (elapsedNsecs <= 0) {
        return;
    }

    QPointer<LottieAttached> guard(this);
    d->elapsedRemainderNsecs = saturatedElapsedSum(
        d->elapsedRemainderNsecs, elapsedNsecs);
    const qint64 frameCount = d->elapsedRemainderNsecs / interval;
    if (frameCount > 0) {
        d->elapsedRemainderNsecs %= interval;
        advanceFrames(frameCount);
    }

    if (!guard || !guard->d) {
        return;
    }
    if (!d->playing) {
        updateFrameTimer();
    }
}

void LottieAttached::requestSourceAsync()
{
    if (!d->hostSupported) {
        return;
    }

    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    if (!runtime || !d->runtimeSubscription.isValid()) {
        failAsync(LottieErrorSummary::RuntimeUnavailable);
        return;
    }

    const QString requestedTheme = requestedThemeForSource();

    const quint64 sourceGeneration = d->sourceGeneration;
    QPointer<LottieAttached> guard(this);
    const bool accepted = runtime->requestSource(
        d->runtimeSubscription,
        LottieSourceReference::fromText(d->source, lottieSourceBaseUrl(this)),
        requestedTheme,
        [guard, sourceGeneration](
            const LottieRuntimeSourceResult &result) {
            if (!guard || !guard->d ||
                guard->d->sourceGeneration != sourceGeneration) {
                return;
            }

            if (!result.loaded || !result.asset.isValid() || result.asset.frameCount() <= 0) {
                if (result.sourceNotFound) {
                    // 对 Icon 的裸 *-motion 名称，“动画不存在”不是加载错误，
                    // 而是尝试同名普通图标的信号。必须在发布 Error 前交回
                    // Icon，观察者才只会看到 Loading -> Null。
                    auto *icon = qobject_cast<Icon *>(guard->parent());
                    if (icon && icon->settleMissingLottieSource()) {
                        return;
                    }
                    if (!guard || !guard->d ||
                        guard->d->sourceGeneration != sourceGeneration) {
                        return;
                    }
                }
                guard->failAsync(LottieErrorSummary::SourceValidation);
                return;
            }

            const int previousFrame = guard->d->currentFrame;
            const int previousTotalFrame = guard->d->totalFrame;
            const qreal previousFps = guard->d->fps;
            const QString previousTheme = guard->d->theme;
            const QStringList previousThemes = guard->d->availableThemes;
            const quint64 previousThemeNotificationEpoch =
                guard->d->themeNotificationEpoch;

            guard->d->asset = result.asset;
            bool manualPreferenceUnavailable = false;
            const QString effectiveTheme = guard->effectiveThemeForAsset(
                guard->d->asset, &manualPreferenceUnavailable);
            if (manualPreferenceUnavailable) {
                warnUnavailableManualTheme(guard->d->asset.revision(),
                                           guard->d->requestedTheme);
            }
            guard->d->asset.setTheme(effectiveTheme);
            guard->d->loaded = true;
            guard->d->totalFrame = result.asset.frameCount();
            guard->d->fps = result.asset.frameRate();
            guard->d->theme = effectiveTheme;
            guard->d->availableThemes = result.asset.availableThemes();
            if (previousTheme != guard->d->theme) {
                ++guard->d->renderConfigurationGeneration;
            }
            guard->d->currentFrame = qBound(0,
                                            guard->d->currentFrame,
                                            guard->d->totalFrame - 1);
            ++guard->d->frameGeneration;
            guard->d->frameRequestPending = false;

            // metadata 从这里起已经可用。若 Host 正隐藏且要求继续计时，先记下
            // 起点再发布同步通知；通知处理耗时也属于资源已可播放后的隐藏时间。
            if (!guard->d->sceneActive) {
                guard->startHiddenPlaybackTimer();
            }

            if (previousFrame != guard->d->currentFrame) {
                Q_EMIT guard->currentFrameChanged();
                if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
                    return;
                }
            }
            if (previousTotalFrame != guard->d->totalFrame) {
                Q_EMIT guard->totalFrameChanged();
                if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
                    return;
                }
            }
            if (!qFuzzyCompare(previousFps, guard->d->fps)) {
                Q_EMIT guard->fpsChanged();
                if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
                    return;
                }
            }
            if (previousTheme != guard->d->theme &&
                guard->d->themeNotificationEpoch == previousThemeNotificationEpoch) {
                guard->d->emitThemeChanged(guard);
                if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
                    return;
                }
            }
            if (previousThemes != guard->d->availableThemes) {
                Q_EMIT guard->availableThemesChanged();
                if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
                    return;
                }
            }

            if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration) {
                return;
            }
            guard->requestCurrentFrameAsync();
        });

    if (!accepted) {
        failAsync(LottieErrorSummary::RuntimeUnavailable);
    }
}

void LottieAttached::requestCurrentFrameAsync()
{
    if (!d->hostSupported || !d->loaded || !d->sceneActive ||
        !LottieRenderer::isRenderSizeAllowed(d->hostRenderSize) ||
        d->status == Null || d->status == Error) {
        return;
    }

    // 帧在后台渲染期间，source、当前帧、播放状态、主题或尺寸都可能改变。
    // 保存发起请求时的版本号，结果回来后只要有一项不同，就丢弃旧结果。
    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 frameGeneration = d->frameGeneration;
    const quint64 playbackGeneration = d->playbackGeneration;
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;
    const int requestedFrame = d->currentFrame;
    const QSize requestedSize = d->hostRenderSize;
    const bool presentationCheckpoint = d->checkpointPending || d->status != Ready ||
        !d->playing ||
        d->frameSelectionKind != LottieAttachedPrivate::FrameSelectionKind::Playback;
    if (d->frameRequestPending && d->requestedFrame == requestedFrame &&
        d->requestedFrameSize == requestedSize &&
        d->requestedFrameGeneration == frameGeneration) {
        return;
    }

    LottieRuntimeFrameRequest request;
    request.assetRevision = d->asset.revision();
    request.asset = d->asset;
    request.effectiveTheme = d->theme;
    request.frame = requestedFrame;
    request.physicalPixelSize = requestedSize;

    d->frameRequestPending = true;
    d->requestedFrame = requestedFrame;
    d->requestedFrameSize = requestedSize;
    d->requestedFrameGeneration = frameGeneration;

    QPointer<LottieAttached> guard(this);
    const auto completeFrame = [guard, sourceGeneration, frameGeneration,
                                playbackGeneration, renderConfigurationGeneration,
                                requestedFrame, requestedSize, presentationCheckpoint](
                                   LottieErrorSummary failure, const QImage &image) {
        if (!guard || !guard->d ||
            guard->d->sourceGeneration != sourceGeneration ||
            guard->d->frameGeneration != frameGeneration ||
            guard->d->playbackGeneration != playbackGeneration ||
            guard->d->renderConfigurationGeneration !=
                renderConfigurationGeneration ||
            guard->d->currentFrame != requestedFrame ||
            guard->d->hostRenderSize != requestedSize ||
            !guard->d->frameRequestPending ||
            guard->d->requestedFrameGeneration != frameGeneration) {
            return;
        }

        guard->d->frameRequestPending = false;
        if (failure == LottieErrorSummary::ThemeApplication) {
            guard->failAsync(LottieErrorSummary::ThemeApplication);
            return;
        }
        if (failure != LottieErrorSummary::None || image.isNull()) {
            if (presentationCheckpoint) {
                guard->failAsync(LottieErrorSummary::FrameRasterization);
            }
            return;
        }

        guard->commitFrameImage(image, requestedSize);
    };

    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    if (!runtime || !d->runtimeSubscription.isValid()) {
        d->frameRequestPending = false;
        failAsync(LottieErrorSummary::RuntimeUnavailable);
        return;
    }

    const bool accepted = runtime->requestFrame(
        d->runtimeSubscription, request,
        [guard, sourceGeneration, frameGeneration, playbackGeneration,
         renderConfigurationGeneration, requestedFrame, requestedSize,
         presentationCheckpoint, completeFrame](const LottieRuntimeFrameResult &result) {
            if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
                guard->d->frameGeneration != frameGeneration ||
                guard->d->playbackGeneration != playbackGeneration ||
                guard->d->renderConfigurationGeneration !=
                    renderConfigurationGeneration ||
                guard->d->currentFrame != requestedFrame ||
                guard->d->hostRenderSize != requestedSize ||
                !guard->d->frameRequestPending ||
                guard->d->requestedFrameGeneration != frameGeneration) {
                return;
            }

            if (result.assetRevision != guard->d->asset.revision() ||
                result.effectiveTheme != guard->d->theme ||
                result.frameNumber != requestedFrame ||
                result.physicalPixelSize != requestedSize) {
                return;
            }

            LottieErrorSummary failure = LottieErrorSummary::None;
            if (result.failure == LottieRuntimeFrameResult::Failure::DeterministicTheme ||
                result.failure == LottieRuntimeFrameResult::Failure::TransientTheme) {
                failure = LottieErrorSummary::ThemeApplication;
            } else if (result.failure != LottieRuntimeFrameResult::Failure::None) {
                failure = LottieErrorSummary::FrameRasterization;
            }
            completeFrame(failure, result.frame.isValid() ? result.frame.image() : QImage());
        });

    if (!accepted) {
        d->frameRequestPending = false;
        failAsync(LottieErrorSummary::RuntimeUnavailable);
    }
}

bool LottieAttached::commitFrameImage(const QImage &image, const QSize &size)
{
    if (image.isNull()) {
        return false;
    }

    const bool statusBecomesReady = d->status != Ready;
    const bool publishesInitialReady = !d->readyWasPublished;
    const bool shouldEmitReadyStatus = statusBecomesReady ||
        d->readyStatusNotificationPending;
    const bool shouldEmitReadyError =
        (statusBecomesReady && !d->errorString.isEmpty()) ||
        d->readyErrorNotificationPending;
    const bool shouldEmitReadyActive =
        (statusBecomesReady && !d->active) ||
        d->readyActiveNotificationPending;
    const bool shouldEmitLoaded = publishesInitialReady || d->loadedNotificationPending;
    const bool initialPresentation = shouldEmitLoaded || d->initialPresentationPending;
    const bool hadCheckpoint = d->checkpointPending;
    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 frameGeneration = d->frameGeneration;
    const quint64 playbackGeneration = d->playbackGeneration;
    const bool completionStateChanged = !d->completed && d->checkpointPending &&
        !d->loop && d->playing &&
        d->currentFrame == d->totalFrame - 1 &&
        (d->frameSelectionKind == LottieAttachedPrivate::FrameSelectionKind::Playback ||
         (publishesInitialReady && d->totalFrame == 1 &&
          d->frameSelectionKind == LottieAttachedPrivate::FrameSelectionKind::Default));

    d->currentImage = image;
    d->currentImageSize = size;
    d->committedFrame = d->currentFrame;
    if (statusBecomesReady) {
        d->status = Ready;
        d->errorString.clear();
        d->errorSummary = LottieErrorSummary::None;
        d->active = true;
        d->readyStatusNotificationPending = true;
        d->readyErrorNotificationPending = shouldEmitReadyError;
        d->readyActiveNotificationPending = shouldEmitReadyActive;
    }
    if (publishesInitialReady) {
        d->readyWasPublished = true;
        d->loadedNotificationPending = true;
        d->initialPresentationPending = true;
    }
    if (d->checkpointPending) {
        d->checkpointPending = false;
    }
    if (completionStateChanged) {
        d->completed = true;
        stopFrameTimer();
    }

    // Qt 信号是同步调用的：任意回调都可能换 source、修改当前帧、暂停，甚至
    // 删除当前对象。每发完一个信号都重新确认这仍是同一次提交，避免
    // 旧通知继续污染回调刚建立的新状态。
    QPointer<LottieAttached> guard(this);
    const auto isCurrent = [&] {
        return guard && guard->d && guard->d->sourceGeneration == sourceGeneration &&
            guard->d->frameGeneration == frameGeneration &&
            guard->d->playbackGeneration == playbackGeneration &&
            guard->d->status == Ready;
    };
    Q_EMIT frameImageChanged();
    if (!isCurrent()) {
        return false;
    }
    if (shouldEmitReadyStatus) {
        d->readyStatusNotificationPending = false;
        Q_EMIT statusChanged();
        if (!isCurrent()) {
            return false;
        }
    }
    if (shouldEmitReadyError) {
        d->readyErrorNotificationPending = false;
        Q_EMIT errorStringChanged();
        if (!isCurrent()) {
            return false;
        }
    }
    if (shouldEmitReadyActive) {
        d->readyActiveNotificationPending = false;
        Q_EMIT activeChanged();
        if (!isCurrent()) {
            return false;
        }
    }
    if (completionStateChanged) {
        Q_EMIT completedChanged();
        if (!isCurrent()) {
            return false;
        }
    }
    if (initialPresentation || hadCheckpoint) {
        updateFrameTimer();
        if (!isCurrent()) {
            return false;
        }
    }
    if (shouldEmitLoaded) {
        d->loadedNotificationPending = false;
        d->initialPresentationPending = false;
        Q_EMIT loaded();
        if (!isCurrent()) {
            return false;
        }
    }
    return true;
}

void LottieAttached::failAsync(LottieErrorSummary errorSummary)
{
    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 themeGeneration = d->themeGeneration;
    const bool hadPixels = !d->currentImage.isNull();
    const bool statusDidChange = d->status != Error;
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;
    const QString safeError = errorSummaryString(errorSummary);
    const bool errorChanged = d->errorString != safeError;
    const bool activeDidChange = d->active;

    suspendLottieFrameDemand(d->runtimeSubscription);
    stopFrameTimer();
    cancelHiddenPlaybackTimer();
    d->frameRequestPending = false;
    d->currentImage = QImage();
    d->currentImageSize = QSize();
    d->committedFrame = -1;
    d->initialPresentationPending = false;
    d->readyStatusNotificationPending = false;
    d->readyErrorNotificationPending = false;
    d->readyActiveNotificationPending = false;
    d->status = Error;
    d->errorString = safeError;
    d->errorSummary = errorSummary;
    d->active = false;

    QPointer<LottieAttached> guard(this);
    if (hadPixels) {
        Q_EMIT frameImageChanged();
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
            guard->d->themeGeneration != themeGeneration ||
            guard->d->renderConfigurationGeneration != renderConfigurationGeneration) {
            return;
        }
    }
    if (statusDidChange) {
        Q_EMIT statusChanged();
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
            guard->d->themeGeneration != themeGeneration ||
            guard->d->renderConfigurationGeneration != renderConfigurationGeneration) {
            return;
        }
    }
    if (errorChanged) {
        Q_EMIT errorStringChanged();
        if (!guard || !guard->d || guard->d->sourceGeneration != sourceGeneration ||
            guard->d->themeGeneration != themeGeneration ||
            guard->d->renderConfigurationGeneration != renderConfigurationGeneration) {
            return;
        }
    }
    if (activeDidChange) {
        Q_EMIT activeChanged();
    }
}

void LottieAttached::updateFrameTimer()
{
    const int timerInterval = frameTimerIntervalMsecs();
    if (d->playing && !d->completed && !d->checkpointPending && d->loaded &&
        d->status == Ready && timerInterval > 0 && d->sceneActive &&
        LottieRenderer::isRenderSizeAllowed(d->hostRenderSize)) {
        if (d->clockRunning && d->clockIntervalMsecs == timerInterval &&
            d->frameClock.isValid()) {
            return;
        }

        // 隐藏期间可能只走过半帧。恢复可见计时器时保留这段余数，避免
        // 每次隐藏/显示都把下一帧推迟一个完整帧间隔。
        const qint64 elapsedRemainderNsecs = d->elapsedRemainderNsecs;
        stopFrameTimer();
        d->elapsedRemainderNsecs = elapsedRemainderNsecs;
        d->clockRunning = true;
        d->clockIntervalMsecs = timerInterval;
        d->frameClock.start();
        d->asyncClock.start(timerInterval);
        return;
    }

    stopFrameTimer();
}

void LottieAttached::stopFrameTimer()
{
    d->clockRunning = false;
    d->clockIntervalMsecs = 0;
    d->asyncClock.stop();
    d->frameClock.invalidate();
    d->elapsedRemainderNsecs = 0;
}

void LottieAttached::startHiddenPlaybackTimer()
{
    if (d->hiddenPlaybackStartedAtNsecs >= 0) {
        return;
    }
    if (d->hiddenPlaybackPolicy != ContinueWhenHidden || !d->playing ||
        !d->loaded || d->status == Null || d->status == Error ||
        d->sceneActive || d->completed) {
        return;
    }

    // 这里只保存单调时钟读数，不启动 QTimer。隐藏多久都不会周期唤醒 GUI
    // 线程；重新显示时再用两次读数的差值一次性算出目标帧。
    d->hiddenPlaybackRemainderNsecs = d->elapsedRemainderNsecs;
    if (d->frameClock.isValid()) {
        d->hiddenPlaybackRemainderNsecs = saturatedElapsedSum(
            d->hiddenPlaybackRemainderNsecs,
            d->frameClock.nsecsElapsed());
    }
    d->hiddenPlaybackStartedAtNsecs = d->monotonicNowNsecs();
}

void LottieAttached::cancelHiddenPlaybackTimer()
{
    d->hiddenPlaybackStartedAtNsecs = -1;
    d->hiddenPlaybackRemainderNsecs = 0;
}

qint64 LottieAttached::takeHiddenPlaybackElapsed()
{
    if (d->hiddenPlaybackStartedAtNsecs < 0) {
        return -1;
    }

    const qint64 nowNsecs = d->monotonicNowNsecs();
    const qint64 elapsedNsecs = saturatedElapsedSum(
        d->hiddenPlaybackRemainderNsecs,
        qMax<qint64>(0, nowNsecs - d->hiddenPlaybackStartedAtNsecs));
    cancelHiddenPlaybackTimer();
    return elapsedNsecs;
}

bool LottieAttached::applyHiddenPlaybackElapsed(qint64 elapsedNsecs,
                                                bool checkpoint,
                                                bool requestFrame)
{
    if (!d->playing || !d->loaded || d->status == Null ||
        d->status == Error || d->completed) {
        return false;
    }

    const qint64 interval = frameIntervalNsecs();
    if (interval <= 0 || d->totalFrame <= 0) {
        return false;
    }

    elapsedNsecs = qMax<qint64>(0, elapsedNsecs);
    const qint64 frameCount = elapsedNsecs / interval;
    const qint64 remainder = elapsedNsecs % interval;
    if (frameCount <= 0) {
        d->elapsedRemainderNsecs = remainder;
        return false;
    }

    int targetFrame = d->currentFrame;
    if (d->loop) {
        const qint64 offset = frameCount % d->totalFrame;
        targetFrame = static_cast<int>((d->currentFrame + offset) % d->totalFrame);
    } else {
        const qint64 framesToEnd = d->totalFrame - 1 - d->currentFrame;
        if (framesToEnd <= 0) {
            d->elapsedRemainderNsecs = 0;
            return false;
        }
        targetFrame = frameCount >= framesToEnd
            ? d->totalFrame - 1
            : d->currentFrame + static_cast<int>(frameCount);
    }

    // 先保存帧内余数，再发 currentFrameChanged。同步回调若再次隐藏，会把
    // 这段余数一起带入新的隐藏计时；若回调显式 seek/换 source，则对应
    // setter 会正常清掉它。
    d->elapsedRemainderNsecs = remainder;
    return selectPlaybackFrame(targetFrame, checkpoint, requestFrame);
}

void LottieAttached::resetPlaybackForHiddenRestart()
{
    cancelHiddenPlaybackTimer();
    stopFrameTimer();
    if (d->source.isEmpty()) {
        return;
    }

    const bool frameChanged = d->currentFrame != 0;
    const bool completionChanged = d->completed;
    d->currentFrame = 0;
    d->completed = false;
    d->frameSelectionKind = LottieAttachedPrivate::FrameSelectionKind::Playback;
    d->checkpointPending = true;
    d->frameRequestPending = false;
    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 playbackGeneration = ++d->playbackGeneration;
    const quint64 frameGeneration = ++d->frameGeneration;

    QPointer<LottieAttached> guard(this);
    const auto isCurrent = [&] {
        return guard && guard->d &&
            guard->d->sourceGeneration == sourceGeneration &&
            guard->d->playbackGeneration == playbackGeneration &&
            guard->d->frameGeneration == frameGeneration &&
            guard->d->currentFrame == 0 && !guard->d->completed;
    };
    if (completionChanged) {
        Q_EMIT completedChanged();
        if (!isCurrent()) {
            return;
        }
    }
    if (frameChanged) {
        Q_EMIT currentFrameChanged();
    }
}

void LottieAttached::restartSelectedFrameRequest()
{
    if (!d->loaded || !d->sceneActive) {
        return;
    }

    requestCurrentFrameAsync();
}

qint64 LottieAttached::frameIntervalNsecs() const
{
    if (d->fps <= 0 || d->speed <= 0) {
        return 0;
    }

    const qreal framesPerSecond = d->fps * d->speed;
    if (!qIsFinite(framesPerSecond) || framesPerSecond <= 0.0) {
        return 0;
    }
    const qreal interval = 1000000000.0 / framesPerSecond;
    if (!qIsFinite(interval) || interval >= static_cast<qreal>(std::numeric_limits<qint64>::max())) {
        return std::numeric_limits<qint64>::max();
    }
    return qMax<qint64>(1, static_cast<qint64>(interval));
}

int LottieAttached::frameTimerIntervalMsecs() const
{
    const qint64 interval = frameIntervalNsecs();
    if (interval <= 0) {
        return 0;
    }

    constexpr qint64 nsecsPerMsec = 1000000;
    const qint64 msecs = qMax<qint64>(1, 1 + (interval - 1) / nsecsPerMsec);
    return static_cast<int>(qMin<qint64>(msecs, std::numeric_limits<int>::max()));
}

void LottieAttached::advanceFrames(qint64 frameCount)
{
    if (!d->loaded || frameCount <= 0) {
        return;
    }

    if (d->loop) {
        const qint64 frameOffset = frameCount % d->totalFrame;
        selectPlaybackFrame(static_cast<int>((d->currentFrame + frameOffset) % d->totalFrame),
                            false);
        return;
    }

    if (d->currentFrame >= d->totalFrame - 1) {
        stopFrameTimer();
        return;
    }

    const qint64 targetFrame = static_cast<qint64>(d->currentFrame) + frameCount;
    if (targetFrame >= d->totalFrame - 1) {
        selectPlaybackFrame(d->totalFrame - 1, true);
    } else {
        selectPlaybackFrame(static_cast<int>(targetFrame), false);
    }
}

bool LottieAttached::selectPlaybackFrame(int frame, bool checkpoint,
                                         bool requestFrame)
{
    if (!d->loaded || d->totalFrame <= 0) {
        return false;
    }

    frame = qBound(0, frame, d->totalFrame - 1);
    const bool frameChanged = d->currentFrame != frame;
    d->currentFrame = frame;
    d->frameSelectionKind = LottieAttachedPrivate::FrameSelectionKind::Playback;
    d->checkpointPending = checkpoint;
    ++d->frameGeneration;
    d->frameRequestPending = false;
    if (checkpoint) {
        const qint64 elapsedRemainderNsecs = d->elapsedRemainderNsecs;
        stopFrameTimer();
        d->elapsedRemainderNsecs = elapsedRemainderNsecs;
    }

    QPointer<LottieAttached> guard(this);
    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 frameGeneration = d->frameGeneration;
    const quint64 playbackGeneration = d->playbackGeneration;
    const auto isCurrent = [&] {
        return guard && guard->d &&
            guard->d->sourceGeneration == sourceGeneration &&
            guard->d->frameGeneration == frameGeneration &&
            guard->d->playbackGeneration == playbackGeneration &&
            guard->d->currentFrame == frame &&
            guard->d->frameSelectionKind ==
                LottieAttachedPrivate::FrameSelectionKind::Playback;
    };
    if (frameChanged) {
        Q_EMIT currentFrameChanged();
        if (!isCurrent()) {
            return false;
        }
    }

    if (requestFrame) {
        requestCurrentFrameAsync();
    }
    return isCurrent();
}

void LottieAttached::beginReplay()
{
    if (!d->completed) {
        return;
    }

    cancelHiddenPlaybackTimer();
    const bool frameChanged = d->currentFrame != 0;
    d->completed = false;
    d->currentFrame = 0;
    const quint64 playbackGeneration = ++d->playbackGeneration;
    d->frameSelectionKind = LottieAttachedPrivate::FrameSelectionKind::Playback;
    d->checkpointPending = !d->source.isEmpty();
    const quint64 frameGeneration = ++d->frameGeneration;
    d->frameRequestPending = false;
    stopFrameTimer();

    const quint64 sourceGeneration = d->sourceGeneration;
    const quint64 renderConfigurationGeneration = d->renderConfigurationGeneration;
    const int selectedFrame = d->currentFrame;
    QPointer<LottieAttached> guard(this);
    const auto isCurrentState = [&] {
        return guard && guard->d &&
            guard->d->sourceGeneration == sourceGeneration &&
            guard->d->playbackGeneration == playbackGeneration &&
            guard->d->currentFrame == selectedFrame &&
            guard->d->frameSelectionKind == LottieAttachedPrivate::FrameSelectionKind::Playback &&
            !guard->d->completed;
    };
    Q_EMIT completedChanged();
    if (!isCurrentState()) {
        return;
    }
    if (frameChanged) {
        Q_EMIT currentFrameChanged();
        if (!isCurrentState()) {
            return;
        }
    }

    if (guard->d->frameGeneration != frameGeneration ||
        guard->d->renderConfigurationGeneration != renderConfigurationGeneration) {
        return;
    }

    if (!d->loaded) {
        return;
    }
    requestCurrentFrameAsync();
}

LottieAttached *LottieAttached::qmlAttachedProperties(QObject *object)
{
    auto *attached = new LottieAttached(object);
    if (auto *icon = qobject_cast<Icon *>(object)) {
        icon->attachLottieController(attached);
    } else if (auto *item = qobject_cast<LottieItem *>(object)) {
        item->attachLottieController(attached);
    }
    return attached;
}

} // namespace UkuiQuick
