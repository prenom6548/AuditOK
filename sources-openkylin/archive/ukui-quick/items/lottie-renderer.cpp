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
    \class UkuiQuick::LottieRenderer
    \inmodule UkuiQuickItems
    \brief 在唯一 raster worker 中消费 immutable LottieAsset 的 ThorVG 渲染器.

    LottieRenderer 只接受已由 asset worker 限额、验证并物化的 \c LottieAsset，
    读取其动画 JSON 并使用 ThorVG 将指定帧渲染为 QImage。它同时处理 dotLottie
    的主题 slot 和渲染缓冲区生命周期；它不是 GUI 同步宿主路径。

    LottieRenderer consumes only an immutable, validated \c LottieAsset. The
    production instance is owned by the single raster worker; host adapters never
    construct it and the class is not a supported QML, installed-header, or public
    ABI surface.

    \section1 主要功能

    \list
        \li 加载普通 Lottie JSON 和 dotLottie 动画资产
        \li 验证动画元数据和渲染尺寸，并使用资产中已检查的图片快照
        \li 渲染指定帧，并按目标尺寸缩放动画内容
        \li 应用和重置 dotLottie 主题 slot
    \endlist

    此类是 Lottie QML 类型的内部渲染支持，通常应通过 \l LottieItem 或其
    \l Lottie 附加属性使用。

    \sa LottieItem, Lottie
*/

#include "lottie-renderer.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSizeF>
#include <QtMath>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <thorvg.h>
#include <thorvg_lottie.h>

namespace UkuiQuick {

namespace {

// 单边允许的最大渲染像素尺寸。
constexpr int kMaxRenderDimension = 8192;
// 一帧允许分配的最大像素总数。
constexpr size_t kMaxRenderPixels = 64U * 1024U * 1024U;
// 允许的最大帧率，防止异常文件要求过于频繁地刷新。
constexpr double kMaxAnimationFrameRate = 1000.0;

class ThorVGEngineLifetime
{
public:
    bool acquire()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_usable) {
            return false;
        }
        if (m_ownerCount > 0) {
            ++m_ownerCount;
            return true;
        }

        const auto result = tvg::Initializer::init(0);
        if (result != tvg::Result::Success) {
            qWarning() << "LottieRenderer: Failed to initialize ThorVG engine"
                       << "result:" << static_cast<int>(result);
            m_usable = false;
            return false;
        }

        m_ownerCount = 1;
        return true;
    }

    void release()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        Q_ASSERT(m_ownerCount > 0);
        if (m_ownerCount <= 0 || --m_ownerCount > 0) {
            return;
        }

        const auto result = tvg::Initializer::term();
        if (result != tvg::Result::Success) {
            qWarning() << "LottieRenderer: Failed to terminate ThorVG engine"
                       << "result:" << static_cast<int>(result);
            // ThorVG 1.0 即使报告 term 失败，也已经减少了内部初始化计数。
            // 此时库处于部分关闭状态，继续重新初始化并不安全，所以禁止再创建
            // 新的 LottieRenderer。
            m_usable = false;
            return;
        }
    }

private:
    mutable std::mutex m_mutex;
    int m_ownerCount = 0;
    bool m_usable = true;
};

ThorVGEngineLifetime &thorVGEngineLifetime()
{
    static ThorVGEngineLifetime lifetime;
    return lifetime;
}

// 从 JSON 对象读取有限浮点数，拒绝无穷大和 NaN。
bool finiteJsonNumber(const QJsonObject &object, const QString &key, double *value)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isDouble()) {
        return false;
    }
    const double number = jsonValue.toDouble();
    if (!std::isfinite(number)) {
        return false;
    }
    *value = number;
    return true;
}

// 校验动画帧率、帧区间和画布尺寸，阻止异常元数据导致资源耗尽。
bool validateAnimationMetadata(const QJsonObject &animation, QString *errorString)
{
    double frameRate = 0.0;
    double firstFrame = 0.0;
    double lastFrame = 0.0;
    double width = 0.0;
    double height = 0.0;
    if (!finiteJsonNumber(animation, QStringLiteral("fr"), &frameRate) ||
        !finiteJsonNumber(animation, QStringLiteral("ip"), &firstFrame) ||
        !finiteJsonNumber(animation, QStringLiteral("op"), &lastFrame) ||
        !finiteJsonNumber(animation, QStringLiteral("w"), &width) ||
        !finiteJsonNumber(animation, QStringLiteral("h"), &height)) {
        *errorString = QStringLiteral("animation metadata contains a missing or non-finite number");
        return false;
    }

    const double frameCount = lastFrame - firstFrame;
    if (frameRate <= 0.0 || frameRate > kMaxAnimationFrameRate ||
        frameCount <= 0.0 || frameCount > static_cast<double>(std::numeric_limits<int>::max()) ||
        width <= 0.0 || width > kMaxRenderDimension ||
        height <= 0.0 || height > kMaxRenderDimension ||
        width * height > static_cast<double>(kMaxRenderPixels)) {
        *errorString = QStringLiteral("animation metadata exceeds the supported limits");
        return false;
    }
    return true;
}

// 递归检查主题规则，拒绝动画资产没有声明的内嵌图片。
bool containsUntrustedSlotImage(const QJsonValue &value, bool insideSlot = false)
{
    if (value.isArray()) {
        for (const QJsonValue &item : value.toArray()) {
            if (containsUntrustedSlotImage(item, insideSlot)) {
                return true;
            }
        }
        return false;
    }
    if (!value.isObject()) {
        return false;
    }

    const QJsonObject object = value.toObject();
    if (insideSlot && object.value(QStringLiteral("p")).toString().startsWith(
                          QStringLiteral("data:image/"), Qt::CaseInsensitive)) {
        return true;
    }
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (containsUntrustedSlotImage(it.value(), insideSlot || it.key() == QStringLiteral("slots"))) {
            return true;
        }
    }
    return false;
}

}

class LottieRendererPrivate
{
public:
    tvg::SwCanvas *canvas = nullptr;
    tvg::LottieAnimation *animation = nullptr;
    tvg::Picture *picture = nullptr;
    QImage targetImage;

    LottieAsset asset;
    bool loaded = false;
    int totalFrame = 0;
    QString theme;
    uint32_t themeSlotCode = 0;
    QByteArray themeSlotJson;

    int renderWidth = 0;
    int renderHeight = 0;
    bool canvasReady = false;
    bool pictureOnCanvas = false;
    bool assetResolutionFailed = false;
    bool ownsThorVGEngine = false;
};

LottieRenderer::LottieRenderer()
    : d(new LottieRendererPrivate)
{
    d->ownsThorVGEngine = thorVGEngineLifetime().acquire();
}

LottieRenderer::~LottieRenderer()
{
    clear();
    if (d->ownsThorVGEngine) {
        thorVGEngineLifetime().release();
        d->ownsThorVGEngine = false;
    }
    delete d;
    d = nullptr;
}

bool LottieRenderer::ensureCanvas()
{
    if (!d->ownsThorVGEngine) {
        return false;
    }
    if (d->canvas) {
        return true;
    }

    d->canvas = tvg::SwCanvas::gen();
    if (!d->canvas) {
        qWarning() << "LottieRenderer: Failed to create SwCanvas";
        return false;
    }
    d->canvasReady = false;
    return true;
}

bool LottieRenderer::setupCanvasTarget(int w, int h)
{
    if (!d->canvas) {
        return false;
    }

    QImage targetImage(w, h, QImage::Format_ARGB32_Premultiplied);
    if (targetImage.isNull()) {
        qWarning() << "LottieRenderer: Failed to allocate render target";
        return false;
    }

    d->canvas->sync();

    // 直接把 QImage 自己的像素内存交给 ThorVG，避免先画到临时缓冲区再
    // 拷贝到 QImage。每行实际占用的字节可能多于像素宽度，所以 ThorVG
    // 需要的行跨度必须按 bytesPerLine 计算。
    const auto result = d->canvas->target(
        reinterpret_cast<uint32_t*>(targetImage.bits()),
        static_cast<uint32_t>(targetImage.bytesPerLine() / sizeof(uint32_t)),
        static_cast<uint32_t>(w),
        static_cast<uint32_t>(h),
        tvg::ColorSpace::ARGB8888);
    if (result != tvg::Result::Success) {
        qWarning() << "LottieRenderer: Failed to set canvas target";
        return false;
    }

    // ThorVG 会继续保存 targetImage 的像素指针，所以渲染结束前不能释放它。
    // 已发布的 QImage 只读共享这块内存；下一帧会先换一块新内存，绝不会覆盖旧帧。
    d->targetImage = std::move(targetImage);
    d->renderWidth = w;
    d->renderHeight = h;
    d->canvasReady = true;
    return true;
}

bool LottieRenderer::isRenderSizeAllowed(const QSize &targetSize)
{
    const int width = targetSize.width();
    const int height = targetSize.height();
    if (width <= 0 || height <= 0 ||
        width > kMaxRenderDimension || height > kMaxRenderDimension) {
        return false;
    }

    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    return pixelCount <= kMaxRenderPixels &&
           pixelCount <= std::numeric_limits<size_t>::max() / sizeof(uint32_t);
}

void LottieRenderer::clear()
{
    discardRenderSession();
    d->asset = LottieAsset();
    d->totalFrame = 0;
}

void LottieRenderer::discardRenderSession()
{
    d->loaded = false;
    d->assetResolutionFailed = false;
    resetCanvas();

    if (d->animation) {
        resetThemeSlot();
    }

    delete d->canvas;
    d->canvas = nullptr;

    delete d->animation;
    d->animation = nullptr;
    d->picture = nullptr;

    d->targetImage = QImage();

}

LottieRenderer::Failure LottieRenderer::load(const LottieAsset &asset, const QString &theme)
{
    clear();
    d->theme = theme;

    if (!d->ownsThorVGEngine) {
        return Failure::Raster;
    }

    if (!asset.isValid()) {
        return Failure::Source;
    }

    d->asset = asset;
    d->theme = d->asset.preferredTheme(theme);

    const QByteArray animationData = d->asset.animationJsonData();
    QString loadError;
    if (animationData.isEmpty()) {
        qWarning() << "LottieRenderer: Animation data is empty";
        return Failure::Source;
    }

    QJsonParseError parseError;
    const QJsonDocument animationDocument = QJsonDocument::fromJson(animationData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !animationDocument.isObject() ||
        !validateAnimationMetadata(animationDocument.object(), &loadError) ||
        containsUntrustedSlotImage(animationDocument.object())) {
        qWarning() << "LottieRenderer: Refusing unsafe animation data:" << d->asset.animationJsonPath()
                   << "error:" << (loadError.isEmpty() ? parseError.errorString() : loadError);
        return Failure::Source;
    }
    d->animation = tvg::LottieAnimation::gen();
    if (!d->animation) {
        qWarning() << "LottieRenderer: Failed to create LottieAnimation";
        return Failure::Source;
    }
    d->picture = d->animation->picture();
    if (!d->picture) {
        qWarning() << "LottieRenderer: Failed to create picture";
        delete d->animation;
        d->animation = nullptr;
        return Failure::Source;
    }

    auto resolver = [](tvg::Paint *paint, const char *assetSource, void *data) -> bool {
        auto *renderer = static_cast<LottieRenderer *>(data);
        return renderer && renderer->resolveAsset(paint, assetSource);
    };
    if (d->picture->resolver(resolver, this) != tvg::Result::Success) {
        qWarning() << "LottieRenderer: Failed to set asset resolver";
        delete d->animation;
        d->animation = nullptr;
        d->picture = nullptr;
        return Failure::Source;
    }
    // 不把资源根交给 ThorVG。动画中的每张图片都必须经过 resolver，从
    // asset worker 已经限额、验证并固定下来的内存快照加载。
    const tvg::Result result = d->picture->load(
        animationData.constData(),
        static_cast<uint32_t>(animationData.size()),
        "lottie+json",
        nullptr,
        true);
    if (result != tvg::Result::Success) {
        qWarning() << "LottieRenderer: Failed to load:" << d->asset.animationJsonPath()
                   << "resolved as:" << d->asset.animationJsonPath()
                   << "result:" << static_cast<int>(result);
        delete d->animation;
        d->animation = nullptr;
        d->picture = nullptr;
        return Failure::Source;
    }
    if (d->assetResolutionFailed) {
        // resolver 返回 true 是为了禁止 ThorVG 回退到原文件路径；它是否真的
        // 加载成功由这个单独的标记决定，不能把空白图片当成成功动画。
        delete d->animation;
        d->animation = nullptr;
        d->picture = nullptr;
        return Failure::Source;
    }
    const float totalFrame = d->animation->totalFrame();
    if (!std::isfinite(totalFrame) || totalFrame <= 0.0f ||
        static_cast<double>(totalFrame) > static_cast<double>(std::numeric_limits<int>::max())) {
        qWarning() << "LottieRenderer: Invalid frame count:" << totalFrame;
        delete d->animation;
        d->animation = nullptr;
        d->picture = nullptr;
        return Failure::Source;
    }
    const float duration = d->animation->duration();
    if (!std::isfinite(duration) || duration <= 0.0f) {
        qWarning() << "LottieRenderer: Invalid animation duration:" << duration;
        delete d->animation;
        d->animation = nullptr;
        d->picture = nullptr;
        return Failure::Source;
    }
    d->totalFrame = qMax(1, static_cast<int>(totalFrame));
    const qreal fps = static_cast<qreal>(d->totalFrame / duration);
    if (!qIsFinite(fps) || fps <= 0.0 || fps > kMaxAnimationFrameRate) {
        qWarning() << "LottieRenderer: Invalid animation frame rate:" << fps;
        delete d->animation;
        d->animation = nullptr;
        d->picture = nullptr;
        return Failure::Source;
    }

    float fw = 0.0f;
    float fh = 0.0f;
    d->picture->size(&fw, &fh);
    if (!std::isfinite(fw) || !std::isfinite(fh) || fw <= 0 || fh <= 0 ||
        fw > kMaxRenderDimension || fh > kMaxRenderDimension ||
        static_cast<double>(fw) * static_cast<double>(fh) >
            static_cast<double>(kMaxRenderPixels)) {
        qWarning() << "LottieRenderer: Invalid animation canvas size:" << fw << fh;
        delete d->animation;
        d->animation = nullptr;
        d->picture = nullptr;
        return Failure::Source;
    }

    d->loaded = true;

    if (!d->theme.isEmpty()) {
        const Failure themeFailure = applyTheme(d->theme);
        if (themeFailure != Failure::None) {
            discardRenderSession();
            return themeFailure;
        }
    }

    return Failure::None;
}

LottieRenderer::Failure LottieRenderer::applyTheme(const QString &themeId)
{
    if (themeId.isEmpty()) {
        resetThemeSlot();
        return Failure::None;
    }

    if (!d->loaded || !d->animation) {
        return Failure::Raster;
    }

    if (!d->asset.canUseTheme(themeId)) {
        qWarning() << "LottieRenderer: Theme is not declared for current dotLottie animation:"
                   << themeId;
        return Failure::Raster;
    }

    const QByteArray slotJson = d->asset.themeSlotJson(themeId);
    if (slotJson.isEmpty() || slotJson == "{}") {
        resetThemeSlot();
        return Failure::None;
    }

    const uint32_t slotCode = d->animation->gen(slotJson.constData());
    if (slotCode == 0) {
        qWarning() << "LottieRenderer: ThorVG rejected dotLottie theme slots for:" << themeId;
        return Failure::DeterministicTheme;
    }

    const auto result = d->animation->apply(slotCode);
    if (result != tvg::Result::Success) {
        d->animation->del(slotCode);
        qWarning() << "LottieRenderer: Failed to apply dotLottie theme:" << themeId
                   << "result:" << static_cast<int>(result);
        if (result == tvg::Result::InvalidArguments) {
            return Failure::DeterministicTheme;
        }
        if (result == tvg::Result::InsufficientCondition) {
            return Failure::TransientTheme;
        }
        return Failure::Raster;
    }
    d->themeSlotCode = slotCode;
    d->themeSlotJson = slotJson;
    return Failure::None;
}

void LottieRenderer::resetThemeSlot()
{
    if (!d->animation) {
        d->themeSlotCode = 0;
        d->themeSlotJson.clear();
        return;
    }

    if (d->themeSlotCode != 0) {
        d->animation->apply(0);
        d->animation->del(d->themeSlotCode);
    } else {
        d->animation->apply(0);
    }
    d->themeSlotCode = 0;
    d->themeSlotJson.clear();
}

bool LottieRenderer::resolveAsset(tvg::Paint *paint, const char *source)
{
    if (!paint || !source) {
        d->assetResolutionFailed = true;
        return true;
    }
    const QString assetSource = QString::fromUtf8(source);
    if (paint->type() == tvg::Type::Text) {
        if (assetSource.startsWith(QStringLiteral("name:"))) {
            return false;
        }
        d->assetResolutionFailed = true;
        qWarning() << "LottieRenderer: External font paths are not allowed:"
                   << assetSource;
        return true;
    }
    if (paint->type() != tvg::Type::Picture) {
        d->assetResolutionFailed = true;
        return true;
    }

    tvg::Picture *picture = static_cast<tvg::Picture *>(paint);
    const QList<LottieBoundImageResource> resources =
        d->asset.boundImageResourceCandidates(assetSource);
    for (const LottieBoundImageResource &resource : resources) {
        if (picture->load(resource.data.constData(),
                          static_cast<uint32_t>(resource.data.size()),
                          resource.format.constData(),
                          nullptr,
                          true) == tvg::Result::Success) {
            return true;
        }
    }

    d->assetResolutionFailed = true;
    qWarning() << "LottieRenderer: Image resource is missing, unsafe, or unreadable:"
               << assetSource;
    return true;
}

void LottieRenderer::resetCanvas()
{
    if (d->canvas) {
        d->canvas->sync();
        d->canvas->remove();
        d->canvas->sync();
    }

    d->canvasReady = false;
    d->pictureOnCanvas = false;
    d->renderWidth = 0;
    d->renderHeight = 0;
}

void LottieRenderer::updatePictureTransform(int w, int h)
{
    if (!d->picture) {
        return;
    }

    float picW = 0.0f;
    float picH = 0.0f;
    d->picture->size(&picW, &picH);
    if (!std::isfinite(picW) || !std::isfinite(picH) || picW <= 0.0f || picH <= 0.0f) {
        return;
    }

    const float scaleX = static_cast<float>(w) / picW;
    const float scaleY = static_cast<float>(h) / picH;
    const float scale = qMin(scaleX, scaleY);
    if (!std::isfinite(scale) || scale <= 0.0f) {
        return;
    }
    const float offsetX = (static_cast<float>(w) - picW * scale) / 2.0f;
    const float offsetY = (static_cast<float>(h) - picH * scale) / 2.0f;
    const tvg::Matrix matrix = {
        scale, 0.0f, offsetX,
        0.0f, scale, offsetY,
        0.0f, 0.0f, 1.0f
    };

    const auto result = d->picture->transform(matrix);
    if (result != tvg::Result::Success) {
        qWarning() << "LottieRenderer: Failed to update picture transform"
                   << "result:" << static_cast<int>(result);
    }
}

LottieRenderer::FrameResult LottieRenderer::imageForFrame(int frame,
                                                          const QSize &targetSize)
{
    if (!d->loaded || d->assetResolutionFailed) {
        return {Failure::Raster, {}};
    }

    const int total = d->totalFrame > 0 ? d->totalFrame : 1;
    frame %= total;
    if (frame < 0) {
        frame += total;
    }

    const auto selectFrame = [this](int requestedFrame) {
        const auto result = d->animation->frame(static_cast<float>(requestedFrame));
        if (result == tvg::Result::Success || result == tvg::Result::InsufficientCondition) {
            return true;
        }
        qWarning() << "LottieRenderer: Failed to switch to frame" << requestedFrame
                   << "result:" << static_cast<int>(result);
        return false;
    };
    if (!selectFrame(frame)) {
        return {Failure::Raster, {}};
    }

    return renderFrame(targetSize);
}

LottieRenderer::FrameResult LottieRenderer::renderFrame(const QSize &targetSize)
{
    if (!d->loaded || !d->animation || !d->picture ||
        d->assetResolutionFailed) {
        return {Failure::Raster, {}};
    }

    const int w = targetSize.width();
    const int h = targetSize.height();
    if (!isRenderSizeAllowed(targetSize)) {
        qWarning() << "LottieRenderer: Refusing unsafe render size:" << targetSize;
        return {Failure::Raster, {}};
    }

    if (!ensureCanvas()) {
        return {Failure::Raster, {}};
    }

    const bool renderSizeChanged = !d->canvasReady ||
        w != d->renderWidth || h != d->renderHeight;
    if (renderSizeChanged) {
        resetCanvas();
    }

    // 上一帧可能仍被缓存或组件使用。每次渲染都换一块新的 QImage 内存，
    // 避免 ThorVG 覆盖已经发布的像素。
    if (!setupCanvasTarget(w, h)) {
        return {Failure::Raster, {}};
    }

    if (renderSizeChanged) {
        updatePictureTransform(w, h);
    }

    if (!d->pictureOnCanvas) {
        if (d->canvas->add(d->picture) != tvg::Result::Success) {
            d->canvasReady = false;
            qWarning() << "LottieRenderer: Failed to add picture to canvas";
            return {Failure::Raster, {}};
        }
        d->pictureOnCanvas = true;
    }

    const tvg::Result updateResult = d->canvas->update();
    if (updateResult != tvg::Result::Success) {
        qWarning() << "LottieRenderer: Canvas update failed";
        return {Failure::Raster, {}};
    }
    // ThorVG 可能在 update、draw 或 sync 任一阶段才请求图片资源，因此每一步
    // 返回后都要检查 resolver 是否已经报告失败。
    if (d->assetResolutionFailed) {
        return {Failure::Raster, {}};
    }

    const tvg::Result drawResult = d->canvas->draw(true);
    if (drawResult != tvg::Result::Success) {
        qWarning() << "LottieRenderer: Canvas draw failed";
        return {Failure::Raster, {}};
    }
    if (d->assetResolutionFailed) {
        return {Failure::Raster, {}};
    }

    const tvg::Result syncResult = d->canvas->sync();
    if (syncResult != tvg::Result::Success) {
        qWarning() << "LottieRenderer: Canvas sync failed";
        return {Failure::Raster, {}};
    }
    if (d->assetResolutionFailed) {
        return {Failure::Raster, {}};
    }

    return {Failure::None, d->targetImage};
}

} // namespace UkuiQuick
