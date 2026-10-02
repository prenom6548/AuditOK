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

#ifndef UKUI_QUICK_ITEMS_LOTTIE_RENDERER_H
#define UKUI_QUICK_ITEMS_LOTTIE_RENDERER_H

#include <QImage>
#include <QSize>
#include <QString>

#include "lottie-internal-visibility_p.h"
#include "ukui/lottie/lottie-asset.h"

namespace tvg {
class Paint;
}

namespace UkuiQuick {

class LottieRendererPrivate;
class LottieAsset;

class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRenderer
{
public:
    enum class Failure {
        None,
        Source,
        DeterministicTheme,
        TransientTheme,
        Raster
    };

    struct FrameResult {
        Failure failure = Failure::Raster;
        QImage image;
    };

    LottieRenderer();
    ~LottieRenderer();

    Failure load(const LottieAsset &asset, const QString &theme);
    FrameResult imageForFrame(int frame, const QSize &targetSize);

    static bool isRenderSizeAllowed(const QSize &targetSize);

private:
    void clear();
    void discardRenderSession();
    void resetCanvas();
    void updatePictureTransform(int w, int h);
    FrameResult renderFrame(const QSize &targetSize);
    bool ensureCanvas();
    bool setupCanvasTarget(int w, int h);
    bool resolveAsset(tvg::Paint *paint, const char *source);
    Failure applyTheme(const QString &themeId);
    void resetThemeSlot();

    LottieRendererPrivate *d = nullptr;
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_RENDERER_H
