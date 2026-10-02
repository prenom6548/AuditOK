/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_ITEMS_ICON_H
#define UKUI_QUICK_ITEMS_ICON_H

#include <QObject>
#include <QQuickItem>
#include <QImage>
#include <QRect>

#include "types.h"
#include "ukui/ukui-theme-proxy.h"
#include "ukui/dt-theme.h"
#include "ukui/application-icon-proxy.h"

class QSGTexture;
class QSGImageNode;
class icon_test;

namespace UkuiQuick {

class IconPrivate;
class LottieAttached;
class IconLottieHostAdapter;

/**
 * @class Icon "icon/Icon"
 *
 * Usage:
 *
 * Icon {
 *  mode: Icon.Highlight
 *  source: "xxx"
 * }
 *
 */
class Icon : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(UkuiQuick::Icon::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(UkuiQuick::Icon::SourceType sourceType READ sourceType NOTIFY sourceTypeChanged)
    Q_PROPERTY(QVariant fallbackSource READ fallbackSource WRITE setFallbackSource NOTIFY fallbackSourceChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QString pointText READ pointText WRITE setPointText NOTIFY pointTextChanged)
    Q_PROPERTY(bool forceRound READ forceRound WRITE setForceRound NOTIFY forceRoundChanged)
    Q_PROPERTY(UkuiQuick::Types::Pos pointPos READ pointPos WRITE setPointPos NOTIFY pointPosChanged)
    Q_PROPERTY(UkuiQuick::Icon::PointType pointType READ pointType WRITE setPointType NOTIFY pointTypeChanged)
    Q_PROPERTY(UkuiQuick::Theme::ColorRole pointColor READ pointColor WRITE setPointColor NOTIFY pointColorChanged)
    Q_PROPERTY(UkuiQuick::Theme::ColorRole highlightColor READ highlightColor WRITE setHighlightColor NOTIFY highlightColorChanged)
    Q_PROPERTY(GradientColor* dtThemeHighlightColor READ dtThemeHighlightColor WRITE setDtThemeHighlightColor NOTIFY dtThemeHighlightColorChanged)
    Q_PROPERTY(UkuiQuick::ApplicationIconProxy* proxy READ proxy CONSTANT FINAL)
    Q_PROPERTY(bool addUnifiedIconBox READ addUnifiedIconBox WRITE setAddUnifiedIconBox NOTIFY addUnifiedIconBoxChanged)
public:
    enum SourceType {
        UnknownSource,
        StaticIconSource,
        LottieSource
    };
    Q_ENUM(SourceType)

    enum Mod {
        Normal = 0x01,
        Highlight = 0x02,
        AutoHighlight = 0x04, /* 高亮 */
        ForceHighlight = Highlight | 0x08, /* 高亮 */
        Disabled = 0x80 /* 灰色 */
    };
    Q_DECLARE_FLAGS(Mode, Mod)
    Q_FLAG(Mode)

    enum PointType {
        Null,
        Point,
        Text
    };
    Q_ENUM(PointType)

    enum ImageType {
        FirstParty, /* 第一方图标 */
        RectangleHorizontal, /* 水平方向长方形图标 */
        RectangleVertical, /* 垂直方向长方形图标 */
        RectangleWithBack, /* 有底正方形图标 */
        Other, /* 其他类型图标 */
        Uninitialized /* 未初始化 */
    };

    explicit Icon(QQuickItem *parent = nullptr);
    ~Icon() override;

    Icon::Mode mode() const;
    void setMode(const Icon::Mode &mode);

    Icon::PointType pointType() const;
    void setPointType(Icon::PointType type);

    Types::Pos pointPos() const;
    void setPointPos(Types::Pos pos);

    Theme::ColorRole pointColor() const;
    void setPointColor(const Theme::ColorRole &color);

    QString pointText() const;
    void setPointText(const QString &text);

    Theme::ColorRole highlightColor() const;
    void setHighlightColor(const Theme::ColorRole &color);

    GradientColor* dtThemeHighlightColor() const;
    void setDtThemeHighlightColor(GradientColor *color);

    int radius() const;
    void setRadius(int radius);

    QVariant source() const;
    void setSource(const QVariant &source);
    SourceType sourceType() const;

    QVariant fallbackSource() const;
    void setFallbackSource(const QVariant &fallback);

    bool forceRound() const;
    void setForceRound(bool forceRound);

    bool addUnifiedIconBox() const;
    void setAddUnifiedIconBox(bool addUnifiedIconBox);

    // 在该函数中修改图像的细节
    virtual void imageCorrection(QImage &image);
    virtual QSGTexture *createTexture();
    virtual void drawPoint(QImage &image);
    QPoint posPoint(const QSize &imageSize, const QSize &pointSize) const;

    LottieAttached *lottieAttached() const;
    void notifyLottieChanged();

    inline void markTextureChanged();

    ApplicationIconProxy *proxy();

Q_SIGNALS:
    void modeChanged();
    void sourceChanged();
    void sourceTypeChanged();
    void fallbackSourceChanged();
    void radiusChanged();
    void highlightColorChanged();
    void pointTypeChanged();
    void pointTextChanged();
    void pointPosChanged();
    void pointColorChanged();
    void forceRoundChanged();
    void dtThemeHighlightColorChanged();
    void addUnifiedIconBoxChanged();

private Q_SLOTS:
    void onSizeChanged();
    void updateMode();
    void updateSourceFromProxy();
    void onIconThemeNameChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    void componentComplete() override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;
private:
    enum class RenderPath {
        Direct = 0, /* 不额外处理，直接渲染 */
        ForceRound = 1, /* 强制画成圆形 */
        UnifiedIconBox = 2, /* 按照应用图标规范统一绘制*/
        Radius = 3 /* 裁圆角 */
    };

    bool applyVariantToIcon(const QVariant &source, QIcon &icon);
    QImage loadFallbackIcon();
    RenderPath renderPath() const;
    QImage clipRoundedRect(const QImage &image, int radius) const;

    IconPrivate *d {nullptr};
    friend class ::icon_test;
    friend class LottieAttached;
    friend class IconLottieHostAdapter;

    void acceptCommittedLottieImage(const QImage &image);
    void attachLottieController(LottieAttached *attached);
    void ensureLottieController();
    void syncLottieHostAdapter();
    void syncLottieHostAdapter(QQuickWindow *window);
    void syncLottieSource();
    bool settleMissingLottieSource();
    LottieAttached *existingLottieAttached() const;
    void applyAssignedSource();
    QString effectiveLottieSource() const;

    //第三方图标添加统一底板
    QImage addIconBox(QImage &image);
    //图标裁剪圆角
    QImage clipBackground(QImage &image, QImage targetImage, QRectF targetRect);
    //判断是否为第一方图标
    bool isFirstPartyIcon(const QImage &effectArea, int size);
    //获取图标类型
    void getImageType(const QImage &effectArea, int imageSize, int standardEffectAreaSize);

    QImage getEffectiveArea(const QImage &image);
    //判断图标是否为纯色
    bool isImagePureColor(QImage &image);

    QHash<QPair<int, int>, int> getTransparentPoint(const QImage &image);

    static bool lottieSourceFromVariant(const QVariant &source, QString *lottieSource = nullptr);
    void clearLottieSource();
};

} // UkuiQuick

Q_DECLARE_METATYPE(UkuiQuick::Icon::PointType)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::Icon::Mode)

#endif //UKUI_QUICK_ITEMS_ICON_H
