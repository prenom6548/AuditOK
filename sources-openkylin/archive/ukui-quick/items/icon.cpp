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

/*!
    \qmltype Icon
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供高级图标显示组件。

    Icon是一个功能强大的图标组件，继承自QQuickItem，使用场景图（Scene Graph）进行渲染。
    相比ThemeIcon，Icon提供了更多高级功能，包括：
    \list
        \li 支持多种图标源类型（主题图标、文件路径、QIcon、QPixmap等）
        \li 支持图标模式（正常、高亮、自动高亮、禁用等）
        \li 支持强制圆形显示
        \li 支持应用图标代理，自动处理应用图标
        \li 支持为第三方图标添加统一底板
        \li 自动识别图标类型并进行优化处理
    \endlist

    当 \c source 是 Lottie JSON 或 dotLottie 时，Icon 通过 \c Lottie 附加属性
    使用异步 runtime。\c Highlight 和 \c ForceHighlight 会选择适合品牌色等深色
    背景的 \c dark 动画主题；普通和自动模式跟随系统背景。Icon 不会使用
    \c highlightColor 给动画像素重新着色。Lottie 失败时 Icon 使用静态
    \c fallbackSource；对普通静态图标，现有模式和着色行为保持不变。

    For a Lottie source, Icon uses the asynchronous Lottie runtime. Highlighted
    modes select the standard dark-background-compatible theme without recoloring
    animation pixels; on failure the static \c fallbackSource remains available.

    \section1 使用示例

    基本用法：
    \qml
    import org.ukui.quick.items 1.0

    Icon {
        width: 64
        height: 64
        source: "application-name"
        mode: Icon.AutoHighlight
        radius: 8
    }
    \endqml

    \sa ThemeIcon
*/

/*!   \qmlproperty enumeration Icon::Mode
    \brief 图标显示模式枚举。

    可选值：
    \list
        \li Icon.Normal - 正常模式
        \li Icon.Highlight - 高亮模式
        \li Icon.AutoHighlight - 自动高亮模式，根据主题自动切换
        \li Icon.ForceHighlight - 强制高亮模式
        \li Icon.Disabled - 禁用模式，显示为灰色
    \endlist
*/

/*!
    \qmlproperty Icon::Mode Icon::mode
    \brief 图标的显示模式。

    设置图标的显示模式，支持正常、高亮、自动高亮、强制高亮和禁用等模式。
    对 Lottie source，高亮和强制高亮选择适合深色高亮背景的 \c dark 主题；
    普通和自动高亮模式跟随系统背景。显式设置的 \l Lottie::theme 仍然优先。
*/

/*!
    \qmlproperty var Icon::source
    \brief 图标源。

    可以是主题图标名称、文件路径、QIcon、QPixmap、QImage或QBitmap。
    读取属性时会返回最近一次成功赋值的原始类型和值；主题图标、
    Lottie 定位符或应用图标代理解析出的内部结果不会覆盖该属性。

    不带路径的 \c *-motion 或 \c *-motion.lottie 会优先在系统
    motion-icon 目录中解析，未找到时再相对当前 QML 文件查找。
    显式路径、file/qrc URL 和 JSON 文件名不会被当作 motion-icon 别名。
    对于名称形如 \c *-motion 且不带扩展名的 source，若两个动画候选均不存在，Icon 会按原始
    名称查找普通主题图标。该普通图标是主 source 的静态表现，优先于 \c fallbackSource 。只有
    同名普通图标也无法加载时才使用 \c fallbackSource 。
    已找到但无法解析的动画仍按 Lottie 失败处理，不会伪装成普通图标。
*/

/*!
    \qmlproperty enumeration Icon::sourceType
    \readonly
    \brief 当前主 source 使用的渲染路径。

    \value Icon.UnknownSource source 为空，或者无扩展名的 \c *-motion 名称仍在异步解析。
    \value Icon.StaticIconSource source 使用普通静态图标路径。两个动画候选均不存在而转为
           同名主题图标时也属于此类型。
    \value Icon.LottieSource source 使用 Lottie runtime。动画文件存在但加载失败时仍保持此类型，
           具体生命周期和错误由附加属性的 \c status 与 \c errorString 描述。

    \c fallbackSource 不是主 source，因此不会改变此属性。应用可以始终预设 Lottie 的播放、
    循环和主题意图；当 source 使用静态图标路径时，这些控制保持 dormant。
*/

/*!
    \qmlproperty var Icon::fallbackSource
    \brief 备用图标源。

    当主图标源无法加载时使用的备用图标。
*/

/*!
    \qmlproperty int Icon::radius
    \brief 图标的圆角半径。

    设置图标的圆角半径，单位为像素。
    仅当addUnifiedIconBox为false且forceRound为false时生效。
    小于0的值会按0处理。
*/

/*!
    \qmlproperty bool Icon::forceRound
    \brief 是否强制显示为圆形。

    当设置为true时，图标将被裁剪为圆形。
    该属性优先级高于radius，但在addUnifiedIconBox为true时不生效。
*/

/*!
    \qmlproperty enumeration Icon::highlightColor
    \brief 高亮时的颜色角色。

    设置图标高亮时使用的主题颜色角色，可选值请参阅Theme::ColorRole。
*/

/*!
    \qmlproperty GradientColor* Icon::dtThemeHighlightColor
    \brief DT主题高亮颜色。

    设置DT主题系统的高亮颜色对象。如果未设置，将使用系统默认高亮颜色。
*/

/*!
    \qmlproperty ApplicationIconProxy* Icon::proxy
    \brief 应用图标代理。

    \readonly

    返回应用图标代理对象，用于处理应用图标的特殊逻辑。
*/

/*!
    \qmlproperty bool Icon::addUnifiedIconBox
    \brief 是否为第三方图标添加统一底板。

    \list
        \li true - 为第三方图标添加统一背景底板
        \li false - 不添加统一底板
    \endlist

    默认值为false。该属性优先级高于forceRound和radius。
*/

#include "icon.h"
#include "icon-heuristics.h"
#include "icon-helper.h"
#include "icon-provider.h"
#include "lottie-attached.h"
#include "lottie-host-renderability_p.h"
#include "theme-icon.h"
#include "ukui/lottie/lottie-source-reference.h"

#include <QHash>
#include <QIcon>
#include <QImage>
#include <QBitmap>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QSGTexture>
#include <QSGImageNode>
#include <QQuickWindow>
#include <QUrl>
#include <QVector>
#include <algorithm>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>
#include <qqml.h>
#include <limits>
#include <memory>
#include <utility>
#include <mutex>

namespace UkuiQuick {
class IconBackgroundUtils;
static IconBackgroundUtils *g_iconBackgroundUtils = nullptr;
static std::once_flag onceFlag;
constexpr qreal kCircleFitThreshold = 0.85; //圆形外轮廓与同尺寸离散圆的最低拟合分
constexpr qreal kFilledCircleFitThreshold = 0.97; //第一方兼容分支沿用填充圆拟合阈值
#define COLOR_DIFFERENCE 15

namespace {

// 将图标逻辑尺寸安全转换为纹理像素尺寸。
bool roundedTextureDimension(qreal logicalDimension, qreal devicePixelRatio, int *pixelDimension)
{
    const qreal scaledDimension = logicalDimension * devicePixelRatio;
    if (!qIsFinite(logicalDimension) || logicalDimension < 0.0 ||
        !qIsFinite(scaledDimension) || scaledDimension < 0.0) {
        return false;
    }

    const qreal maximumRoundableDimension =
        static_cast<qreal>(std::numeric_limits<int>::max()) - 0.5;
    if (logicalDimension > maximumRoundableDimension ||
        scaledDimension > maximumRoundableDimension) {
        return false;
    }

    *pixelDimension = qMax(1, qRound(scaledDimension));
    return true;
}

// 将任意 Icon 的逻辑尺寸转换为纹理像素尺寸。
bool iconTextureSize(const QSizeF &logicalSize, qreal devicePixelRatio, QSize *targetSize)
{
    if (!qIsFinite(devicePixelRatio) || devicePixelRatio <= 0.0) {
        return false;
    }

    int pixelWidth = 0;
    int pixelHeight = 0;
    if (!roundedTextureDimension(logicalSize.width(), devicePixelRatio, &pixelWidth) ||
        !roundedTextureDimension(logicalSize.height(), devicePixelRatio, &pixelHeight)) {
        return false;
    }

    *targetSize = QSize(pixelWidth, pixelHeight);
    return true;
}

// 创建无法渲染时使用的 1×1 透明占位图像。
QImage transparentIconImage()
{
    QImage image(QSize(1, 1), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    return image;
}

bool hasAssignedSourceValue(const QVariant &source)
{
    if (!source.isValid() || source.isNull()) {
        return false;
    }

    switch (source.userType()) {
    case QMetaType::QString:
        return !source.toString().trimmed().isEmpty();
    case QMetaType::QUrl:
        return !source.toUrl().isEmpty();
    case QMetaType::QIcon:
        return !source.value<QIcon>().isNull();
    case QMetaType::QImage:
        return !source.value<QImage>().isNull();
    case QMetaType::QPixmap:
        return !source.value<QPixmap>().isNull();
    case QMetaType::QBitmap:
        return !source.value<QBitmap>().isNull();
    default:
        return true;
    }
}

bool usesHighlightedLottieTheme(Icon::Mode mode)
{
    return mode.testFlag(Icon::Highlight) &&
        !mode.testFlag(Icon::AutoHighlight);
}

} // namespace

class IconLottieHostAdapter : public QObject
{
public:
    IconLottieHostAdapter(Icon *host, LottieAttached *controller)
        : QObject()
        , m_host(host)
        , m_controller(controller)
    {
        const QPointer<LottieAttached> guardedController(m_controller);
        QObject::connect(m_controller, &LottieAttached::frameImageChanged,
                         this, [host = m_host, guardedController] {
            host->acceptCommittedLottieImage(
                guardedController ? guardedController->committedImage() : QImage());
        });
        QObject::connect(m_controller, &LottieAttached::statusChanged,
                         this, [host = m_host] {
            host->notifyLottieChanged();
        });
        QObject::connect(m_controller, &LottieAttached::totalFrameChanged,
                         this, [host = m_host] {
            host->notifyLottieChanged();
        });
        QObject::connect(m_controller, &QObject::destroyed, this,
                         [host = m_host](QObject *) {
            host->acceptCommittedLottieImage(QImage());
            host->notifyLottieChanged();
        });

        QObject::connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::styleColorChanged,
                         this, [this] { syncTheme(); });
        QObject::connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::themeNameChanged,
                         this, [this] { syncTheme(); });
        QObject::connect(m_host, &Icon::modeChanged, this, [this] { syncTheme(); });

        syncSource();
        m_renderability =
            std::make_unique<LottieHostRenderabilityTracker>(m_host, m_controller);
    }

    LottieAttached *controller() const
    {
        return m_controller;
    }

    bool renderSizeValid() const
    {
        return m_renderability && m_renderability->renderSizeValid();
    }

    void settleMissingSourceAsStaticIcon()
    {
        if (m_controller) {
            m_controller->setSource(QString());
        }
    }

    void syncSource()
    {
        if (!m_controller) {
            return;
        }
        syncTheme();
        m_controller->setSource(m_host->effectiveLottieSource());
    }

    void syncRenderConfiguration()
    {
        if (m_renderability) {
            m_renderability->sync();
        }
    }

    void syncRenderConfigurationForWindow(QQuickWindow *window)
    {
        if (m_renderability) {
            m_renderability->syncForWindow(window);
        }
    }

private:
    void syncTheme()
    {
        if (!m_controller) {
            return;
        }
        m_controller->setHostTheme(
            automaticLottieThemeCandidate(usesHighlightedLottieTheme(m_host->mode())));
    }

    Icon *m_host = nullptr;
    QPointer<LottieAttached> m_controller;
    std::unique_ptr<LottieHostRenderabilityTracker> m_renderability;
};

class IconBackgroundUtils
{
public:
    static IconBackgroundUtils* instance();
    void setIconThemeName(const QString& iconThemeName);
    bool isValid() const;
    QIcon iconBackground();
    QIcon iconBackgroundWithoutShadow();
    bool containShadow(uint size) const;
    qreal offsetX(uint size) const;
    qreal offsetY(uint size) const;
    QVarLengthArray<qreal, 256> outLineDistance(uint width);
    qreal centerAreaRate() const;
private:
    IconBackgroundUtils();
    void updateIconBackConfig();
    void updateParam();
    void reset();
    QMap<uint, bool> m_containShadow;    //当前图标主题第三方图标统一背景图是否包含阴影
    QMap<uint, qreal> m_offsetX;               //当前图标主题第三方图标统一背景图X轴偏移量系数
    QMap<uint, qreal> m_offsetY;               //当前图标主题第三方图标统一背景图Y轴偏移量系数
    qreal m_centerAreaRate = 0.625;         //当前图标主题视觉中心区域占背景图总大小比值
    QJsonObject m_config;
    QString m_themeName;
    QIcon m_background = QIcon::fromTheme(QStringLiteral("icon-background"));
    QIcon m_backgroundWithoutShadow = QIcon::fromTheme(QStringLiteral("icon-background-noshadow"));
    QMap<uint, QVarLengthArray<qreal, 256>> m_outLineDistance;   //当前图标主题第三方图标统一背景图外边框轮廓
    bool m_valid = false;
};

IconBackgroundUtils* IconBackgroundUtils::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_iconBackgroundUtils = new IconBackgroundUtils();
    });
    return g_iconBackgroundUtils;
}

void IconBackgroundUtils::setIconThemeName(const QString &iconThemeName)
{
    if (iconThemeName == m_themeName) {
        return;
    }
    m_themeName = iconThemeName;
    updateParam();
}

bool IconBackgroundUtils::isValid() const
{
    return m_valid;
}

void IconBackgroundUtils::updateIconBackConfig()
{
    QFile file("/usr/share/ukui/ukui-quick-items/iconThemeBackgroundConfig.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "IconBackgroundUtils::updateIconBackConfig() failed to open config file" << file.errorString();
        return;
    }
    QTextStream iStream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    iStream.setCodec("UTF-8");
#else
    iStream.setEncoding(QStringConverter::Utf8);
#endif

    QJsonParseError parseError {};
    QJsonDocument document = QJsonDocument::fromJson(iStream.readAll().toUtf8(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "IconBackgroundUtils::updateIconBackConfig() failed to parse config file" << parseError.errorString();
        return;
    }

    if (!document.isObject()) {
        qWarning() << "IconBackgroundUtils::updateIconBackConfig() failed to parse config file: Is not object.";
        return;
    }
    m_config = document.object();
}

void IconBackgroundUtils::updateParam()
{
    if (m_background.isNull() || m_background.name() != QStringLiteral("icon-background")
        || m_backgroundWithoutShadow.isNull() || m_backgroundWithoutShadow.name() != QStringLiteral("icon-background-noshadow")) {
        qWarning() << "IconBackgroundUtils::updateParam() failed to get icon background, background or icon background without shadow is null.";
        reset();
        return;
    }
    auto currentThemeConfig = m_config.value(m_themeName).toObject();
    if (currentThemeConfig.isEmpty()) {
        qWarning() << "IconBackgroundUtils::updateParam() failed to get theme config: theme config is empty.";
        reset();
        return;
    }
    m_centerAreaRate = currentThemeConfig.value("centerAreaRate").toDouble();

    QStringList sizeList = {"16", "24", "32", "48", "64", "96", "128", "512"};
    for(auto size : sizeList) {
        auto sizeObject = currentThemeConfig.value(size).toObject();
        if (sizeObject.isEmpty()) {
            qWarning() << QString("IconBackgroundUtils::updateParam() failed to get theme config: size = %1 config is empty.").arg(size);
            reset();
            return;
        }
        qreal offsetX = sizeObject.contains("offsetX") ? sizeObject.value("offsetX").toDouble() : currentThemeConfig.value("offsetX").toDouble();
        qreal offsetY = sizeObject.contains("offsetY") ? sizeObject.value("offsetY").toDouble() : currentThemeConfig.value("offsetY").toDouble();
        m_offsetX.insert(size.toUInt(), offsetX);
        m_offsetY.insert(size.toUInt(), offsetY);

        bool containShadow = sizeObject.contains("containShadow") ? sizeObject.value("containShadow").toBool() : currentThemeConfig.value("containShadow").toBool();
        m_containShadow.insert(size.toUInt(), containShadow);
        auto distanceArr = sizeObject.value("outLineDistance").toArray();
        if(distanceArr.size() != 256) {
            qWarning() << QString("IconBackgroundUtils::updateParam() failed to get theme config: %1 outLineDistance size = %2 is invalid.").arg(m_themeName, size);
            reset();
            return;
        }
        QVarLengthArray<qreal, 256> distance;
        for (auto val : distanceArr) {
            distance.append(val.toVariant().toReal());
        }
        m_outLineDistance.insert(size.toUInt(), distance);
    }
    m_valid = true;
}

void IconBackgroundUtils::reset()
{
    m_valid = false;
    m_containShadow.clear();
    m_offsetX.clear();
    m_offsetY.clear();
    m_outLineDistance.clear();
}

QIcon IconBackgroundUtils::iconBackground()
{
    return m_background;
}

QIcon IconBackgroundUtils::iconBackgroundWithoutShadow()
{
    return m_backgroundWithoutShadow;
}

bool IconBackgroundUtils::containShadow(uint size) const
{
    if (m_backgroundWithoutShadow.availableSizes().contains(QSize(size, size)) || m_backgroundWithoutShadow.actualSize(QSize(size, size)) != QSize(size, size)) {
        if (size <= 16) {
            return m_containShadow.value(16);
        } else if (size <= 24) {
            return m_containShadow.value(24);
        } else if (size <= 32) {
            return m_containShadow.value(32);
        } else if (size <= 48) {
            return m_containShadow.value(48);
        } else if (size <= 64) {
            return m_containShadow.value(64);
        } else if (size <= 96) {
            return m_containShadow.value(96);
        }
    }
    return m_containShadow.value(128);
}

qreal IconBackgroundUtils::offsetX(uint size) const
{
    if (m_backgroundWithoutShadow.availableSizes().contains(QSize(size, size)) || m_backgroundWithoutShadow.actualSize(QSize(size, size)) != QSize(size, size)) {
        if (size <= 16) {
            return m_offsetX.value(16);
        } else if (size <= 24) {
            return m_offsetX.value(24);
        } else if (size <= 32) {
            return m_offsetX.value(32);
        } else if (size <= 48) {
            return m_offsetX.value(48);
        } else if (size <= 64) {
            return m_offsetX.value(64);
        } else if (size <= 96) {
            return m_offsetX.value(96);
        }
    }
    return m_offsetX.value(128);
}

qreal IconBackgroundUtils::offsetY(uint size) const
{
    if (m_backgroundWithoutShadow.availableSizes().contains(QSize(size, size)) || m_backgroundWithoutShadow.actualSize(QSize(size, size)) != QSize(size, size)) {
        if (size <= 16) {
            return m_offsetY.value(16);
        } else if (size <= 24) {
            return m_offsetY.value(24);
        } else if (size <= 32) {
            return m_offsetY.value(32);
        } else if (size <= 48) {
            return m_offsetY.value(48);
        } else if (size <= 64) {
            return m_offsetY.value(64);
        } else if (size <= 96) {
            return m_offsetY.value(96);
        }
    }
    return m_offsetY.value(128);
}

QVarLengthArray<qreal, 256> IconBackgroundUtils::outLineDistance(uint size)
{
    if (size <= 20) {
        return m_outLineDistance.value(16);
    } else if (size > 20 && size <= 28) {
        return m_outLineDistance.value(24);
    } else if (size > 28 && size <= 40) {
        return m_outLineDistance.value(32);
    } else if (size > 40 && size <= 56) {
        return m_outLineDistance.value(48);
    } else if (size > 56 && size <= 80) {
        return m_outLineDistance.value(64);
    } else if (size <= 96) {
        return m_outLineDistance.value(96);
    } else if (size == 512) {
        return m_outLineDistance.value(512);
    }
    return m_outLineDistance.value(128);
}

qreal IconBackgroundUtils::centerAreaRate() const
{
    return m_centerAreaRate;
}

IconBackgroundUtils::IconBackgroundUtils()
{
    updateIconBackConfig();
    // QObject::connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::iconThemeNameChanged, [this]() {
    //     setIconThemeName(ThemeGlobalConfig::instance()->iconThemeName());
    // });
}

class IconPrivate
{
public:
    int radius {0};
    bool forceRound {false};
    bool textureChanged {true};
    QVariant source = QVariant::fromValue(QIcon());
    quint64 sourceGeneration {0};
    Icon::SourceType sourceType {Icon::UnknownSource};
    QIcon icon;
    QString iconManagedLottieSource;
    bool iconManagedStaticFallbackEligible {false};
    std::unique_ptr<IconLottieHostAdapter> lottieHostAdapter;
    QImage committedLottieImage;
    QVariant fallbackSource;
    QIcon fallbackIcon;
    bool fallbackIconLoaded {false};
    Icon::Mode mode {Icon::Normal};
    QString          pointText {""};
    Types::Pos    pointPos {Types::TopRight};
    Icon::PointType  pointType {Icon::Null};
    Theme::ColorRole pointColor {Theme::Highlight};
    Theme::ColorRole highlightColor {Theme::HighlightedText};
    GradientColor* dtThemeHighlightColor = nullptr;
    std::shared_ptr<QSGTexture> currentTexture;
    static QHash<QQuickWindow*, QHash<qint64, std::weak_ptr<QSGTexture> > > textureCache;
    ApplicationIconProxy* proxy = nullptr;
    bool addUnifiedIconBox {false};
    //图标类型
    Icon::ImageType imageType = {Icon::Uninitialized};
    QImage imageCache;
    static const QColor symbolicColor;
};

const QColor IconPrivate::symbolicColor = QColor(26, 26, 26);
QHash<QQuickWindow*, QHash<qint64, std::weak_ptr<QSGTexture> > > IconPrivate::textureCache = {};

// ===== ICON ====== //
Icon::Icon(QQuickItem *parent) : QQuickItem(parent), d(new IconPrivate)
{
    setFlag(QQuickItem::ItemHasContents);
    connect(this, &QQuickItem::widthChanged, this, &Icon::onSizeChanged);
    connect(this, &QQuickItem::heightChanged, this, &Icon::onSizeChanged);
}

Icon::~Icon()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

// ====== DRAW ====== //
QSGNode *Icon::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *data)
{
    QSGImageNode *imageNode = nullptr;
    if (node) {
        imageNode = static_cast<QSGImageNode *>(node);
    } else {
        imageNode = window()->createImageNode();
        imageNode->setOwnsTexture(true);
        imageNode->setFiltering(QSGTexture::Linear);
        node = imageNode;
    }

    if (d->textureChanged) {
        QSGTexture *texture = createTexture();
        imageNode->setTexture(texture);
        imageNode->setSourceRect(texture
            ? QRectF(QPoint(0, 0), texture->textureSize())
            : QRectF());
        d->textureChanged = false;
    }

    imageNode->setRect(boundingRect());
    return node;
}

QSGTexture *Icon::createTexture()
{
    QSize targetSize;
    if (!window() || !iconTextureSize(size(), window()->devicePixelRatio(), &targetSize)) {
        return window() ? window()->createTextureFromImage(transparentIconImage()) : nullptr;
    }

    QImage image;
    bool lottieImage = false;
    const bool useAssignedStaticMotionIcon =
        d->sourceType == StaticIconSource &&
        d->iconManagedStaticFallbackEligible && !d->icon.isNull();
    // 获取原始图像
    if (!d->iconManagedLottieSource.isEmpty() && !useAssignedStaticMotionIcon) {
        if (d->lottieHostAdapter && !d->lottieHostAdapter->renderSizeValid()) {
            if (d->fallbackSource.isNull()) {
                return window()->createTextureFromImage(transparentIconImage());
            }
            image = loadFallbackIcon();
        } else {
            image = d->committedLottieImage;
            lottieImage = !image.isNull();
            if (!lottieImage) {
                image = loadFallbackIcon();
            }
        }
    } else if (d->icon.isNull()) {
        image = loadFallbackIcon();
    } else {
        const RenderPath path = renderPath();
        if (path == RenderPath::ForceRound) {
            // 处理圆形图标
            QSize rawSize = size().toSize();
            if (!d->icon.availableSizes().isEmpty()) {
                auto sizes = d->icon.availableSizes();
                rawSize = *std::max_element(sizes.begin(), sizes.end(),
                                                  [](const QSize&a, const QSize&b) {
                                                      return a.width() < b.width();
                                                  });
            }

            QPixmap rawPixmap = d->icon.pixmap(rawSize);
            if (rawPixmap.isNull()) {
                qWarning() << "Icon::createTexture : Can't load icon with size " << targetSize;
                image = loadFallbackIcon();
            } else {
                int diameter = std::min(rawPixmap.height(), rawPixmap.width());
                QPixmap pixmap(diameter, diameter);
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                QPainterPath path;
                path.addEllipse(0, 0, diameter, diameter);
                painter.setClipPath(path);

                QRect targetRect, sourceRect;
                targetRect = QRect(0, 0, diameter, diameter);
                if (rawPixmap.width() > rawPixmap.height()) {
                    sourceRect = QRect((rawPixmap.width() - diameter) / 2, 0, diameter, diameter);
                } else {
                    sourceRect = QRect(0, (rawPixmap.height() - diameter) / 2, diameter, diameter);
                }
                painter.drawPixmap(targetRect, rawPixmap, sourceRect);
                image = pixmap.toImage().scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
        } else {
            image = d->icon.pixmap(size().toSize()).toImage();
            if (image.isNull()) {
                qWarning() << "Icon::createTexture : Can't load icon with size " << targetSize;
                image = loadFallbackIcon();
            } else if (path == RenderPath::UnifiedIconBox) {
                if (IconBackgroundUtils::instance()->isValid()) {
                    image = addIconBox(image);
                }
            } else if (path == RenderPath::Radius) {
                image = clipRoundedRect(image, d->radius);
            }
        }
    }

//    // 缩放大图像？
//    QSize imageSize = image.size();
//    if (imageSize.width() > 512 || imageSize.height() > 512) {
//        image.scaled();
//        imageSize = image.size();
//    }

    // 修改图像
    //imageCorrection(image);
    if (!lottieImage && d->mode & Icon::Disabled) {
        // use style
        image = QIcon(QPixmap::fromImage(std::move(image))).pixmap(size().toSize(), QIcon::Disabled).toImage();
    } else if (!lottieImage && d->mode & Icon::Highlight) {
        bool isPureColor = true;
        if(image.format() != QImage::Format_ARGB32) {
            image = image.convertToFormat(QImage::Format_ARGB32);
        }
        if (!d->mode.testFlag(Icon::ForceHighlight)) {
            // TODO: 更新判断纯色算法
            isPureColor = isImagePureColor(image);
        }
        GradientColor* highlightColor = d->dtThemeHighlightColor
                                            ? d->dtThemeHighlightColor
                                            : DtTheme::self(qmlEngine(this))->highlightedTextActive();
        if (isPureColor) {
            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            if (highlightColor->colorType() == DtColorType::Type::Pure) {
                p.fillRect(image.rect(), highlightColor->pureColor());
            } else {
                //设置渐变色的方向
                QLinearGradient linearGradient(
                    highlightColor->getGradientStartPoint(highlightColor->gradientDeg(), image.width(), image.height())
                    , highlightColor->getGradientEndPoint(highlightColor->gradientDeg(), image.width(),
                                                          image.height()));
                //设置渐变色颜色
                linearGradient.setColorAt(
                    0.0, highlightColor->mixBackGroundColor(highlightColor->gradientStart(),
                                                            highlightColor->gradientBackground(), 1));
                linearGradient.setColorAt(
                    1.0, highlightColor->mixBackGroundColor(highlightColor->gradientEnd(),
                                                            highlightColor->gradientBackground(), 1));
                QBrush brush(linearGradient);
                p.fillRect(image.rect(), brush);
            }
        } else {
            int symbolicRed = d->symbolicColor.red();
            int symbolicGreen = d->symbolicColor.green();
            int symbolicBlue = d->symbolicColor.blue();
            if (highlightColor->colorType() == DtColorType::Type::Pure) {
                QColor baseColor = highlightColor->pureColor();
                uchar *pixels = image.bits();
                int bytesPerLine = image.bytesPerLine();
                for (int y = 0; y < image.height(); y++) {
                    QRgb* line = reinterpret_cast<QRgb*>(pixels + y * bytesPerLine);
                    for (int x = 0; x < image.width(); x++) {
                        QRgb &color = line[x];
                        if (qAlpha(color) > 0.3 * 256) {
                            if (qAbs(qRed(color) - symbolicRed) < COLOR_DIFFERENCE && qAbs(qGreen(color) - symbolicGreen) < COLOR_DIFFERENCE
                                    && qAbs(qBlue(color) - symbolicBlue) < COLOR_DIFFERENCE) {
                                color = qRgba(baseColor.red(),baseColor.green(),baseColor.blue(), qAlpha(color));
                            }
                        }
                    }
                }
            } else {
                QImage gradientImg(image.width(), image.height(), QImage::Format::Format_ARGB32);
                //设置渐变色的方向
                QLinearGradient linearGradient(
                    highlightColor->getGradientStartPoint(highlightColor->gradientDeg(), image.width(), image.height())
                    , highlightColor->getGradientEndPoint(highlightColor->gradientDeg(), image.width(),
                                                          image.height()));
                //设置渐变色颜色
                linearGradient.setColorAt(
                    0.0, highlightColor->mixBackGroundColor(highlightColor->gradientStart(),
                                                            highlightColor->gradientBackground(), 1));
                linearGradient.setColorAt(
                    1.0, highlightColor->mixBackGroundColor(highlightColor->gradientEnd(),
                                                            highlightColor->gradientBackground(), 1));
                QBrush brush(linearGradient);
                //将渐变色绘制到与图标相同大小的image上
                QPainter painterImg(&gradientImg);
                painterImg.setBrush(brush);
                painterImg.drawRect(gradientImg.rect());
                painterImg.end();

                uchar *pixels = image.bits();
                const uchar *gradientPixels = gradientImg.constBits();
                int bytesPerLine = image.bytesPerLine();
                int bytesPerLineGra = gradientImg.bytesPerLine();
                for (int y = 0; y < image.height(); y++) {
                    QRgb* line = reinterpret_cast<QRgb*>(pixels + y * bytesPerLine);
                    const QRgb* gradientLine = reinterpret_cast<const QRgb*>(gradientPixels + y * bytesPerLineGra);
                    for (int x = 0; x < image.width(); x++) {
                        QRgb &color = line[x];
                        if (qAlpha(color) > 0.3 * 256) {
                            if (qAbs(qRed(color) - symbolicRed) < COLOR_DIFFERENCE && qAbs(qGreen(color) - symbolicGreen) < COLOR_DIFFERENCE
                                    && qAbs(qBlue(color) - symbolicBlue) < COLOR_DIFFERENCE) {
                                color = qRgba(qRed(gradientLine[x]), qGreen(gradientLine[x]), qBlue(gradientLine[x]), qAlpha(color));
                            }
                        }
                    }
                }
            }
        }
    }



    //std::shared_ptr<QSGTexture> texturePtr;
    //qint64 cacheKey = image.cacheKey();
    //auto hash = IconPrivate::textureCache[window()];
    //if (hash.contains(cacheKey)) {
    //    texturePtr = hash[cacheKey].lock();
    //    if (texturePtr) {
    //        d->currentTexture = texturePtr;
    //        return d->currentTexture.get();
    //    }
    //}

    QSGTexture *texture = window()->createTextureFromImage(image);
    //texturePtr = std::shared_ptr<QSGTexture>(texture, [this] (QSGTexture *texture) {
    //    //hash.remove(cacheKey);
    //    //if (hash.isEmpty()) {
    //    //    IconPrivate::textureCache.remove(window());
    //    //}
    //
    //    delete texture;
    //});

    //hash[cacheKey] = texturePtr;
    //d->currentTexture = texturePtr;
    return texture;
}

void Icon::imageCorrection(QImage &image)
{
    // 画点
    drawPoint(image);
    // 修改状态
}

QPoint Icon::posPoint(const QSize &imageSize, const QSize &pointSize) const
{
    QPoint point;
    switch (d->pointPos) {
        default:
        case Types::TopRight:
            point = {imageSize.width() - pointSize.width(), 0};
            break;
        case Types::TopLeft:
            point = {0, 0};
            break;
        case Types::BottomRight:
            point = {imageSize.width() - pointSize.width(), imageSize.height() - pointSize.height()};
            break;
        case Types::BottomLeft:
            point = {0, imageSize.height() - pointSize.height()};
            break;
        case Types::Center:
            point = {(imageSize.width() - pointSize.width()) / 2, (imageSize.height() - pointSize.height()) / 2};
            break;
        case Types::LeftCenter:
            point = {0, (imageSize.height() - pointSize.height()) / 2};
            break;
        case Types::TopCenter:
            point = {(imageSize.width() - pointSize.width()) / 2, 0};
            break;
        case Types::RightCenter:
            point = {imageSize.width() - pointSize.width(), (imageSize.height() - pointSize.height()) / 2};
            break;
        case Types::BottomCenter:
            point = {(imageSize.width() - pointSize.width()) / 2, imageSize.height() - pointSize.height()};
            break;
    }

    if (point.x() < 0) {
        point.setX(0);
    } else if ((point.x() + pointSize.width()) > imageSize.width()) {
        point.setX(imageSize.width() - pointSize.width());
    }

    if (point.y() < 0) {
        point.setY(0);
    } else if ((point.y() + pointSize.height()) > imageSize.height()) {
        point.setY(imageSize.height() - pointSize.height());
    }

    return point;
}

void Icon::drawPoint(QImage &image)
{
    if (d->pointType == Icon::Null) {
        return;
    }

    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setBrush(Theme::instance()->color(d->pointColor));
    painter.setPen(Theme::instance()->color(d->pointColor));

    if (d->pointType == Icon::Point) {
        QSize pointSize(qFloor(image.width() * 0.35), qFloor(image.height() * 0.35));
        QRect rect(posPoint(image.size(), pointSize), pointSize);

        painter.drawEllipse(rect);

    } else if (d->pointType == Icon::Text) {
        QRect rect = {0, 0, qFloor(image.width() * 0.4), qFloor(image.height() * 0.4)};

        QFont font = painter.font();
        // 为了将两位数正常显示在框框中
        font.setPointSize(qFloor(6.0 * rect.width() / 12.0));
        font.setBold(true);

        QString text = d->pointText;
        QFontMetrics fontMetrics(font);
        QRect textRect = fontMetrics.boundingRect(rect, Qt::AlignVCenter | Qt::AlignHCenter, text);
        if (textRect.width() > rect.width()) {
            if (textRect.width() > image.width()) {
                rect.setWidth(image.width());
                text = fontMetrics.elidedText(text, Qt::ElideRight, rect.width() - 4);
            } else {
                rect.setWidth(textRect.width() + 8);
            }
        }

        rect.moveTopLeft(posPoint(image.size(), rect.size()));

        // 底部圆
        painter.save();
        painter.setPen(Theme::instance()->colorWithCustomTransparency(Theme::BrightText, Theme::Active, 0.15));
        painter.drawRoundedRect(rect, qFloor(rect.height() * 0.5), qFloor(rect.height() * 0.5));
        painter.restore();

        // draw text.
        painter.save();
        // TODO: 优化字体显示
        painter.setFont(font);
        painter.setPen(Theme::instance()->buttonText());
        painter.drawText(rect, Qt::AlignVCenter | Qt::AlignHCenter, text);
        painter.restore();
    }
}

void Icon::onSizeChanged()
{
    syncLottieHostAdapter();
    markTextureChanged();
}
// ====== METHODS ====== //
QVariant Icon::source() const
{
    return d->source;
}

Icon::SourceType Icon::sourceType() const
{
    return d->sourceType;
}

void Icon::setSource(const QVariant &source)
{
    if (d->proxy && !d->proxy->icon().isEmpty()) {
        qInfo() << "This icon has been set a effective proxy, you can not set source explicitly."
                << "source attempted to set:" << source
                << "proxy icon:" << d->proxy->icon();
        return;
    }

    if (d->source == source) {
        return;
    }

    const SourceType oldSourceType = d->sourceType;
    const QVariant assignedSource = source;
    const quint64 sourceGeneration = ++d->sourceGeneration;
    d->source = assignedSource;
    applyAssignedSource();
    syncLottieSource();
    markTextureChanged();
    QPointer<Icon> guard(this);
    Q_EMIT sourceChanged();
    if (!guard || guard->d->sourceGeneration != sourceGeneration) {
        return;
    }
    if (oldSourceType != d->sourceType) {
        Q_EMIT sourceTypeChanged();
    }
}

QVariant Icon::fallbackSource() const
{
    return d->fallbackSource;
}

void Icon::setFallbackSource(const QVariant &fallback)
{
    if(d->fallbackSource != fallback) {
        d->fallbackSource = fallback;
        d->fallbackIcon = QIcon();
        d->fallbackIconLoaded = false;
        markTextureChanged();
        Q_EMIT fallbackSourceChanged();
    }
}

bool Icon::applyVariantToIcon(const QVariant &source, QIcon &icon)
{
    icon = QIcon();
    switch (source.userType()) {
    case QMetaType::QIcon:
        icon = source.value<QIcon>();
        break;
    case QMetaType::QImage:
        icon.addPixmap(QPixmap::fromImage(source.value<QImage>()));
        break;
    case QMetaType::QPixmap:
        icon.addPixmap(source.value<QPixmap>());
        break;
    case QMetaType::QBitmap:
        icon.addPixmap(source.value<QBitmap>());
        break;
    case QMetaType::QString: {
        icon = IconHelper::loadIcon(source.value<QString>());
        break;
    }
    default:
        break;
    }

    return !icon.isNull();
}

QImage Icon::loadFallbackIcon()
{
    if (d->fallbackSource.isNull()) {
        qWarning() << "Icon::loadFallbackIcon : fallback icon is Null";
        QPixmap pixmap(size().toSize() * window()->devicePixelRatio());
        pixmap.fill(Qt::transparent);
        return pixmap.toImage();
    }
    if (!d->fallbackIconLoaded) {
        applyVariantToIcon(d->fallbackSource, d->fallbackIcon);
        d->fallbackIconLoaded = true;
    }
    if (!d->fallbackIcon.isNull()) {
        return d->fallbackIcon.pixmap(size().toSize() * window()->devicePixelRatio()).toImage();
    }

    qWarning() << "Icon::loadFallbackIcon : failed to load fallback icon from" << d->fallbackSource;
    QPixmap pixmap(size().toSize() * window()->devicePixelRatio());
    pixmap.fill(Qt::transparent);
    return pixmap.toImage();
}

bool Icon::forceRound() const
{
    return d->forceRound;
}

void Icon::setForceRound(bool forceRound)
{
    if (d->forceRound == forceRound) {
        return;
    }

    d->forceRound = forceRound;
    markTextureChanged();
    Q_EMIT forceRoundChanged();
}

Icon::Mode Icon::mode() const
{
    return d->mode;
}

void Icon::setMode(const Icon::Mode &mode)
{
    if (d->mode == mode) {
        return;
    }

    d->mode = mode;
    if (d->mode & Icon::AutoHighlight) {
        updateMode();
        connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::styleColorChanged, this, &Icon::updateMode, Qt::UniqueConnection);
        connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::themeNameChanged, this, &Icon::updateMode, Qt::UniqueConnection);
    } else {
        disconnect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::styleColorChanged, this, &Icon::updateMode);
        disconnect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::themeNameChanged, this, &Icon::updateMode);
        markTextureChanged();
    }

    Q_EMIT modeChanged();
}

Theme::ColorRole Icon::highlightColor() const
{
    return d->highlightColor;
}

void Icon::setHighlightColor(const Theme::ColorRole &color)
{
    if (d->highlightColor == color) {
        return;
    }

    d->highlightColor = color;
    markTextureChanged();
    Q_EMIT highlightColorChanged();
}

GradientColor* Icon::dtThemeHighlightColor() const
{
    return d->dtThemeHighlightColor;
}

void Icon::setDtThemeHighlightColor(GradientColor *color)
{
    if(d->dtThemeHighlightColor == nullptr && isComponentComplete()) {
        disconnect(DtTheme::self(qmlEngine(this)), &DtTheme::highlightedTextActiveChanged, this, &Icon::markTextureChanged);
    }
    if(color != nullptr && d->dtThemeHighlightColor != color) {
        d->dtThemeHighlightColor = color;
        markTextureChanged();
        Q_EMIT dtThemeHighlightColorChanged();
    }
}

int Icon::radius() const
{
    return d->radius;
}

void Icon::setRadius(int radius)
{
    const int normalizedRadius = radius < 0 ? 0 : radius;
    if (d->radius == normalizedRadius) {
        return;
    }

    d->radius = normalizedRadius;
    markTextureChanged();
    Q_EMIT radiusChanged();
}

Icon::PointType Icon::pointType() const
{
    return d->pointType;
}

void Icon::setPointType(Icon::PointType type)
{
    if (d->pointType == type) {
        return;
    }

    d->pointType = type;
    markTextureChanged();
    Q_EMIT pointTypeChanged();
}

Theme::ColorRole Icon::pointColor() const
{
    return d->pointColor;
}

void Icon::setPointColor(const Theme::ColorRole &color)
{
    if (d->pointColor == color) {
        return;
    }

    d->pointColor = color;
    markTextureChanged();
    Q_EMIT pointColorChanged();
}

QString Icon::pointText() const
{
    return d->pointText;
}

void Icon::setPointText(const QString &text)
{
    if (d->pointText == text) {
        return;
    }

    d->pointText = text;
    markTextureChanged();
    Q_EMIT pointTextChanged();
}

Types::Pos Icon::pointPos() const
{
    return d->pointPos;
}

void Icon::setPointPos(Types::Pos pos)
{
    if (d->pointPos == pos) {
        return;
    }

    d->pointPos = pos;
    markTextureChanged();
    Q_EMIT pointPosChanged();
}

bool Icon::addUnifiedIconBox() const
{
    return d->addUnifiedIconBox;
}

void Icon::setAddUnifiedIconBox(bool addUnifiedIconBox)
{
    if(d->addUnifiedIconBox == addUnifiedIconBox) {
        return;
    }
    d->addUnifiedIconBox = addUnifiedIconBox;
    if (d->addUnifiedIconBox) {
        IconBackgroundUtils::instance()->setIconThemeName(ThemeGlobalConfig::instance()->iconThemeName());
        QImage().swap(d->imageCache);
    }
    markTextureChanged();
    Q_EMIT addUnifiedIconBoxChanged();
}

inline void Icon::markTextureChanged()
{
    d->textureChanged = true;
    update();
}

ApplicationIconProxy* Icon::proxy()
{
    if (d->proxy == nullptr) {
        d->proxy = new ApplicationIconProxy(this);
        updateSourceFromProxy();
        connect(d->proxy, &ApplicationIconProxy::iconChanged, this, &Icon::updateSourceFromProxy);
    }
    return d->proxy;
}

void Icon::updateMode()
{
    if (ThemeGlobalConfig::instance()->isDarkTheme()) {
        d->mode |= Icon::Highlight;
    } else {
        d->mode &= ~Icon::Highlight;
    }

    markTextureChanged();
}

void Icon::updateSourceFromProxy()
{
    ++d->sourceGeneration;
    const SourceType oldSourceType = d->sourceType;
    if (!d->proxy->icon().isEmpty()) {
        clearLottieSource();
        d->icon = IconHelper::loadIcon(d->proxy->icon());
        d->sourceType = StaticIconSource;
    } else {
        applyAssignedSource();
    }
    syncLottieSource();
    markTextureChanged();
    if (oldSourceType != d->sourceType) {
        Q_EMIT sourceTypeChanged();
    }
}

void Icon::onIconThemeNameChanged()
{
    if (d->addUnifiedIconBox) {
        IconBackgroundUtils::instance()->setIconThemeName(ThemeGlobalConfig::instance()->iconThemeName());
        QImage().swap(d->imageCache);
    }
    markTextureChanged();
}

void Icon::componentComplete()
{
    QQuickItem::componentComplete();
    if(d->dtThemeHighlightColor == nullptr) {
        connect(DtTheme::self(qmlEngine(this)), &DtTheme::highlightedTextActiveChanged, this, &Icon::markTextureChanged);
    }
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::iconThemeNameChanged, this, &Icon::onIconThemeNameChanged);
}

void Icon::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemDevicePixelRatioHasChanged) {
        syncLottieHostAdapter();
        markTextureChanged();
    } else if (change == ItemSceneChange) {
        const bool attachedToScene = value.window != nullptr;
        setFlag(ItemHasContents, attachedToScene);
        d->textureChanged = true;
        syncLottieHostAdapter(value.window);
        if (attachedToScene) {
            update();
        }
    }
    QQuickItem::itemChange(change, value);
}

void Icon::notifyLottieChanged()
{
    if (d->sourceType == UnknownSource && d->lottieHostAdapter) {
        LottieAttached *attached = d->lottieHostAdapter->controller();
        // 裸 *-motion 名称刚赋值时还不能确定是动画还是同名普通图标。
        // 一旦动画元数据发布，或已找到动画但加载失败，就可以确定它走
        // Lottie 路径。单纯“文件不存在”会在进入 Error 前改走静态路径。
        if (attached &&
            (attached->totalFrame() > 0 || attached->status() == LottieAttached::Ready ||
             attached->status() == LottieAttached::Error)) {
            d->sourceType = LottieSource;
            markTextureChanged();
            Q_EMIT sourceTypeChanged();
            return;
        }
    }
    markTextureChanged();
}

bool Icon::settleMissingLottieSource()
{
    if (!d->iconManagedStaticFallbackEligible || !d->lottieHostAdapter) {
        return false;
    }

    const SourceType oldSourceType = d->sourceType;
    const QVariant assignedSource = d->source;
    const QString lottieSource = d->iconManagedLottieSource;
    const quint64 sourceGeneration = d->sourceGeneration;

    // 两个动画候选都不存在时，再把原名称当作普通主题图标加载。
    // source 属性仍保留调用方赋的原值；这里只改变实际渲染路径。
    applyVariantToIcon(QVariant(lottieSource), d->icon);
    d->sourceType = StaticIconSource;
    markTextureChanged();

    QPointer<Icon> guard(this);
    // 先让 Lottie 附加属性回到 Null，再通知 sourceType 变化。这样信号
    // 回调读取到的是一套完整状态，而不是 StaticIconSource + Loading。
    d->lottieHostAdapter->settleMissingSourceAsStaticIcon();
    // 上面的同步信号可能换掉 source，甚至销毁 Icon。此时当前结果已经
    // 过期，不能继续为旧 source 发通知。
    if (!guard || guard->d->sourceGeneration != sourceGeneration ||
        guard->d->source != assignedSource ||
        guard->d->iconManagedLottieSource != lottieSource) {
        return true;
    }
    if (oldSourceType != d->sourceType) {
        Q_EMIT sourceTypeChanged();
    }
    return true;
}

Icon::RenderPath Icon::renderPath() const
{
    if (d->addUnifiedIconBox) {
        return RenderPath::UnifiedIconBox;
    }

    if (d->forceRound) {
        return RenderPath::ForceRound;
    }

    if (d->radius > 0) {
        return RenderPath::Radius;
    }

    return RenderPath::Direct;
}

QImage Icon::clipRoundedRect(const QImage &image, int radius) const
{
    if (image.isNull() || radius <= 0) {
        return image;
    }

    QImage clipped(image.size(), QImage::Format_ARGB32_Premultiplied);
    clipped.fill(Qt::transparent);
    const qreal effectiveRadius = qMin<qreal>(radius, qMin(image.width() / 2.0, image.height() / 2.0));

    QPainter painter(&clipped);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addRoundedRect(QRectF(clipped.rect()), effectiveRadius, effectiveRadius);
    painter.setClipPath(path);
    painter.drawImage(QPoint(0, 0), image);
    painter.end();

    return clipped;
}

QImage Icon::addIconBox(QImage &image)
{
    if(image.isNull()) {
        return image;
    }

    QImage backImage = IconBackgroundUtils::instance()->iconBackground().pixmap(size().toSize()).toImage();
    if(backImage.isNull()) {
        return image;
    }

    QImage backImageNoShadow = IconBackgroundUtils::instance()->iconBackgroundWithoutShadow().pixmap(size().toSize()).toImage();
    QImage backImageEffectArea = getEffectiveArea(backImageNoShadow);

    //根据计算出来的非透明点找到有效区域并截取出来
    QImage effectiveArea = getEffectiveArea(image);
    if(effectiveArea.format() != QImage::Format_ARGB32) {
        effectiveArea = effectiveArea.convertToFormat(QImage::Format_ARGB32);
    }
    if (image != d->imageCache) {
        getImageType(effectiveArea, backImage.width(), backImageEffectArea.width());
        d->imageCache = image;
    }
    bool clipped = false;
    switch (d->imageType) {
        //第一方图标和未定义类型图标不作处理
        case Icon::Uninitialized :
        case Icon::FirstParty : {
            return image;
        }
        //不是正方形的图标，按比例将图标有效区域缩放到视觉中心区域并居中
        case Icon::RectangleHorizontal : {
            effectiveArea = effectiveArea.scaledToWidth(backImage.width() * IconBackgroundUtils::instance()->centerAreaRate(), Qt::SmoothTransformation);
            break;
        }
        case Icon::RectangleVertical : {
            effectiveArea = effectiveArea.scaledToHeight(backImage.height() * IconBackgroundUtils::instance()->centerAreaRate(), Qt::SmoothTransformation);
            break;
        }
        //有底图标按照第一方规范裁剪圆角并缩放至背景区域大小
        case Icon::RectangleWithBack : {
            IconHeuristics::repairBackdropCorners(effectiveArea);
            if(IconBackgroundUtils::instance()->containShadow(image.width())) {
                effectiveArea = clipBackground(effectiveArea, backImageNoShadow, backImageEffectArea.rect());
                clipped = true;
            } else {
                effectiveArea = effectiveArea.scaledToWidth(backImage.width() * backImageEffectArea.width() / backImageNoShadow.width(), Qt::SmoothTransformation);
                if(backImage.width() < 24) {
                    if(effectiveArea.format() != QImage::Format_ARGB32) {
                        effectiveArea = effectiveArea.convertToFormat(QImage::Format_ARGB32);
                    }
                    QHash<QPair<int, int>, int> imageAlphaHash = getTransparentPoint(backImageEffectArea);
                    QHash<QPair<int, int>, int>::const_iterator it;
                    for(it = imageAlphaHash.constBegin();it != imageAlphaHash.constEnd();it++) {
                        QColor color = effectiveArea.pixelColor(it.key().first, it.key().second);
                        color.setAlpha(it.value());
                        effectiveArea.setPixelColor(it.key().first, it.key().second, color);
                    }
                }
            }
            break;
        }
        //默认将图标有效区域缩减到背景图的视觉中心区域
        default:
        case Icon::Other : {
            effectiveArea = effectiveArea.scaledToWidth(backImage.width() * IconBackgroundUtils::instance()->centerAreaRate(), Qt::SmoothTransformation);
            break;
        }
    }
    qreal dpr = backImage.devicePixelRatio();
    backImage.setDevicePixelRatio(1);
    QPainter painter(&backImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!clipped) {
        qreal offsetX = IconBackgroundUtils::instance()->offsetX(backImage.width()) * backImage.width();
        qreal offsetY = IconBackgroundUtils::instance()->offsetY(backImage.width()) * backImage.height();
        painter.translate(offsetX, offsetY);
    }

    //将处理好的图标有效区域绘制到统一背景
    painter.drawImage(QRectF(static_cast<qreal>(backImage.width() - effectiveArea.width()) / 2, static_cast<qreal>(backImage.height() - effectiveArea.height()) / 2,effectiveArea.width(), effectiveArea.height()), effectiveArea);
    painter.end();
    backImage.setDevicePixelRatio(dpr);
    return backImage;
}

QImage Icon::clipBackground(QImage &image, QImage targetImage, QRectF targetRect)
{
    image = image.scaledToWidth(targetRect.width(),Qt::SmoothTransformation);
    qreal offsetX = IconBackgroundUtils::instance()->offsetX(targetImage.width()) * targetImage.width();
    qreal offsetY = IconBackgroundUtils::instance()->offsetY(targetImage.width()) * targetImage.height();
    qreal dpr = targetImage.devicePixelRatio();
    targetImage.setDevicePixelRatio(1);
    QPainter painter(&targetImage);
    painter.translate(offsetX, offsetY);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    painter.drawImage(QRectF(static_cast<qreal>(targetImage.width() - image.width()) / 2, static_cast<qreal>(targetImage.height() - image.height()) / 2, image.width(), image.height()), image);
    painter.end();
    targetImage.setDevicePixelRatio(dpr);

    return targetImage;
}

bool Icon::isFirstPartyIcon(const QImage &effectArea, int size)
{
    //根据当前主题获取第一方图标的欧氏距离
    QVarLengthArray<qreal, 256> distance;
    QSize itemSize = this->size().toSize();
    QImage image;
    if (d->icon.availableSizes().contains(itemSize) || d->icon.actualSize(itemSize) != itemSize) {
        distance = IconBackgroundUtils::instance()->outLineDistance(size);
        image = effectArea.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        distance = IconBackgroundUtils::instance()->outLineDistance(512);
        image = getEffectiveArea(d->icon.pixmap(512).toImage()).scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    int contradictoryCount = 0;
    int transparencyAlpha = size >= 64 ? 0 : 3;
    //找到图片的最外层不透明边界，计算欧氏距离并和第一方图标模板进行比较，总体误差低于百分之10认为是第一方图标
    int centerX = image.width()/2 - 1;
    int centerY = image.height()/2 - 1;

    const uchar *pixels = image.constBits();
    const int bytesPerLine = image.bytesPerLine(); // 每行字节数
    for(int x = 0; x < image.width(); x++) {
        for(int y = 0; y < image.height(); y++) {
            const uchar *line = pixels + y * bytesPerLine;
            if(qAlpha(*reinterpret_cast<const QRgb*>(line + x * 4)) > transparencyAlpha) {
                qreal dTemp = qSqrt(qPow(x - centerX, 2) + qPow(y - centerY, 2));
                if(qAbs(distance[x] - dTemp) > 2) {
                    ++contradictoryCount;
                }
                break;
            }
        }
    }
    for(int x = 0; x < image.width(); ++x) {
        for(int y = image.height() - 1; y >= 0; --y) {
            const uchar *line = pixels + y * bytesPerLine;
            if(qAlpha(*reinterpret_cast<const QRgb*>(line + x * 4)) > transparencyAlpha) {
                qreal dTemp = qSqrt(qPow(x - centerX, 2) + qPow(y - centerY, 2));
                if(qAbs(distance[x + 64] - dTemp) > 2) {
                    ++contradictoryCount;
                }
                break;
            }
        }
    }
    for(int y = 0; y < image.height(); ++y) {
        for(int x = 0; x < image.width(); ++x) {
            const uchar *line = pixels + y * bytesPerLine;
            if(qAlpha(*reinterpret_cast<const QRgb*>(line + x * 4)) > transparencyAlpha) {
                qreal dTemp = qSqrt(qPow(x - centerX, 2) + qPow(y - centerY, 2));
                if(qAbs(distance[y + 128] - dTemp) > 2) {
                    ++contradictoryCount;
                }
                break;
            }
        }
    }
    for(int y = 0; y < image.height(); ++y) {
        for(int x = image.width() - 1; x >= 0; --x) {
            const uchar *line = pixels + y * bytesPerLine;
            if(qAlpha(*reinterpret_cast<const QRgb*>(line + x * 4)) > transparencyAlpha) {
                qreal dTemp = qSqrt(qPow(x - centerX, 2) + qPow(y - centerY, 2));
                if(qAbs(distance[y + 192] - dTemp) > 2) {
                    ++contradictoryCount;
                }
                break;
            }
        }
    }
    return contradictoryCount <= 10;
}

void Icon::getImageType(const QImage &effectArea, int imageSize, int standardEffectAreaSize)
{
    if(effectArea.isNull()) {
        d->imageType = Icon::Uninitialized;
        return;
    }

    const auto classifySquareBackdrop = [&](qreal smallCornerOuterThreshold, qreal circleThreshold) {
        const QRgb dominantColor = IconHeuristics::dominantOpaqueColor(effectArea);
        const qreal outerContentRatio = IconHeuristics::outerContentRatioWithDominant(effectArea, dominantColor);
        if (circleThreshold > 0.0 && IconHeuristics::circleFit(effectArea) > circleThreshold) {
            return Icon::Other;
        }
        if (outerContentRatio > smallCornerOuterThreshold) {
            return Icon::Other;
        }
        return Icon::RectangleWithBack;
    };
    if (isFirstPartyIcon(effectArea, imageSize)) {
        if (!IconBackgroundUtils::instance()->containShadow(imageSize)
            && !((qAbs(effectArea.width() - standardEffectAreaSize) <= 2 && effectArea.width() <= standardEffectAreaSize) && effectArea.height() == effectArea.width())) {
            if(IconHeuristics::hasOpaqueRectBackdrop(IconHeuristics::cropNonShadowArea(effectArea))) {
                d->imageType = classifySquareBackdrop(0.33, 0.0);
                return;
            }
            d->imageType = Icon::Other;
            return;
        }
        if(IconHeuristics::filledCircleFit(effectArea) > kFilledCircleFitThreshold) {
            d->imageType = Icon::Other;
            return;
        }
        d->imageType = Icon::FirstParty;
        return;
    }

    if(qAbs(effectArea.width() - effectArea.height()) >= 2) {
        if(effectArea.width() > effectArea.height() + 1) {
            d->imageType = Icon::RectangleHorizontal;
            return;
        } else if (effectArea.width() + 1 < effectArea.height()) {
            d->imageType = Icon::RectangleVertical;
            return;
        }
    } else if(IconHeuristics::hasOpaqueRectBackdrop(IconHeuristics::cropNonShadowArea(effectArea))) {
        d->imageType = classifySquareBackdrop(0.35, kCircleFitThreshold);
        return;
    }
    d->imageType = Icon::Other;
}

QImage Icon::getEffectiveArea(const QImage &image)
{
    //从上到下，从下到上，从左到右，从右到左，找到图片中第一个非透明像素点
    QPoint top(0,0), right(0,0), left(0,0), bottom(0,0);
    bool getMax = false;
    bool getMin = false;

    const uchar *pixels = image.constBits();
    const int bytesPerLine = image.bytesPerLine(); // 每行字节数
    int transparencyAlpha = IconBackgroundUtils::instance()->containShadow(image.width()) ? 2 : 4;
    for (int y = 0; y < image.height(); ++y) {
        const uchar *lineTop = pixels + y * bytesPerLine;
        const uchar *lineBottom = pixels + (image.height() - y - 1) * bytesPerLine;
        for (int x = 0; x < image.width(); ++x) {
            if (!getMax && qAlpha(*reinterpret_cast<const QRgb*>(lineTop + x * 4)) > transparencyAlpha) {
                top.setX(x);
                top.setY(y);
                getMax = true;
            }
            if (!getMin && qAlpha(*reinterpret_cast<const QRgb*>(lineBottom + x * 4)) > transparencyAlpha) {
                bottom.setX(x);
                bottom.setY(image.height() - y - 1);
                getMin = true;
            }
            if(getMax && getMin) {
                break;
            }
        }
        if(getMax && getMin) {

            break;
        }
    }

    getMax = false;
    getMin = false;
    for (int x = 0; x < image.width(); ++x) {
        for (int y = 0; y < image.height(); ++y) {
            const uchar *line = pixels + y * bytesPerLine;
            if (!getMax && qAlpha(*reinterpret_cast<const QRgb*>(line + x * 4)) > transparencyAlpha) {
                left.setX(x);
                left.setY(y);
                getMax = true;
            }
            if (!getMin && qAlpha(*reinterpret_cast<const QRgb*>(line + (image.width() - x - 1) * 4)) > transparencyAlpha) {
                right.setX(image.width() - x - 1);
                right.setY(y);
                getMin = true;
            }
            if(getMax && getMin) {
                break;
            }
        }
        if(getMax && getMin) {
            break;
        }
    }

    //在非透明像素点中找到x和y的最大值和最小值
    auto xMax = std::max({top.x(),bottom.x(),left.x(),right.x()});
    auto xMin = std::min({top.x(),bottom.x(),left.x(),right.x()});
    auto yMax = std::max({top.y(),bottom.y(),left.y(),right.y()});
    auto yMin = std::min({top.y(),bottom.y(),left.y(),right.y()});

    //如果为透明图片，返回原图
    if(xMax == 0 || yMax == 0) {
        return image;
    }
    //根据计算出来的非透明点找到有效区域并截取出来
    return image.copy(xMin, yMin, xMax - xMin + 1, yMax - yMin + 1);
}

bool Icon::isImagePureColor(QImage &image)
{
    if(image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    const uchar *pixels = image.constBits();
    const int bytesPerLine = image.bytesPerLine(); // 每行字节数
    bool isPureColor = true;
    QVector<QRgb> vector;
    int total_red = 0;
    int total_green = 0;
    int total_blue = 0;
    int symbolicRed = d->symbolicColor.red();
    int symbolicGreen = d->symbolicColor.green();
    int symbolicBlue = d->symbolicColor.blue();
    for(int y = 0; y < image.height(); y++) {
        const uchar *line = pixels + y * bytesPerLine;
        for(int x = 0; x < image.width(); x++) {
            const QRgb* color = reinterpret_cast<const QRgb*>(line + x * 4);
            if(qAlpha(*color) > 0.3 * 256) {
                int red = qRed(*color);
                int green = qGreen(*color);
                int blue = qBlue(*color);
                vector.append(*color);
                total_red += red;
                total_green += green;
                total_blue += blue;
                if(!isPureColor) {
                    continue;
                }
                if(qAbs(symbolicRed - red) > COLOR_DIFFERENCE || qAbs(symbolicGreen - green) > COLOR_DIFFERENCE
                        || qAbs(symbolicBlue - blue) > COLOR_DIFFERENCE) {
                    isPureColor = false;
                }
            }

        }
    }
    if(isPureColor || vector.count() == 0) {
        return true;
    }
    qreal squareRoot_red = 0;
    qreal squareRoot_green = 0;
    qreal squareRoot_blue = 0;
    qreal average_red = total_red / vector.count();
    qreal average_green = total_green / vector.count();
    qreal average_blue = total_blue / vector.count();
    for (QRgb rgb : vector) {
        squareRoot_red += qPow(qRed(rgb) - average_red, 2);
        squareRoot_green += qPow(qGreen(rgb) - average_green, 2);
        squareRoot_blue += qPow(qBlue(rgb) - average_blue, 2);
    }

    qreal arithmeticSquareRoot_red = qSqrt(squareRoot_red / vector.count());
    qreal arithmeticSquareRoot_green = qSqrt(squareRoot_green / vector.count());
    qreal arithmeticSquareRoot_blue = qSqrt(squareRoot_blue / vector.count());

    return arithmeticSquareRoot_red < 2.0 && arithmeticSquareRoot_green < 2.0 && arithmeticSquareRoot_blue < 2.0;
}

QHash<QPair<int, int>, int> Icon::getTransparentPoint(const QImage &image)
{
    QHash<QPair<int, int>, int> imageAlphaList;
    for (int y = 0; y < image.height(); y++) {
        const QRgb *line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < image.width(); x++) {
            const QRgb &color = line[x];
            if (qAlpha(color) != 255) {
                imageAlphaList.insert(QPair(x, y),qAlpha(color));
            }
        }
    }
    return imageAlphaList;

}

LottieAttached *Icon::lottieAttached() const
{
    if (!existingLottieAttached()) {
        const_cast<Icon *>(this)->ensureLottieController();
    }
    return existingLottieAttached();
}

LottieAttached *Icon::existingLottieAttached() const
{
    return qobject_cast<LottieAttached *>(
        qmlAttachedPropertiesObject<LottieAttached>(const_cast<Icon *>(this), false));
}

void Icon::attachLottieController(LottieAttached *attached)
{
    if (d->lottieHostAdapter) {
        Q_ASSERT(d->lottieHostAdapter->controller() == attached);
        return;
    }
    d->lottieHostAdapter = std::make_unique<IconLottieHostAdapter>(this, attached);
}

void Icon::ensureLottieController()
{
    if (d->lottieHostAdapter) {
        return;
    }
    qmlAttachedPropertiesObject<LottieAttached>(this, true);
}

void Icon::syncLottieHostAdapter()
{
    if (d->lottieHostAdapter) {
        d->lottieHostAdapter->syncRenderConfiguration();
    }
}

void Icon::syncLottieHostAdapter(QQuickWindow *window)
{
    if (d->lottieHostAdapter) {
        d->lottieHostAdapter->syncRenderConfigurationForWindow(window);
    }
}

void Icon::syncLottieSource()
{
    if (d->lottieHostAdapter) {
        d->lottieHostAdapter->syncSource();
    }
}

void Icon::acceptCommittedLottieImage(const QImage &image)
{
    d->committedLottieImage = image;
    markTextureChanged();
}

bool Icon::lottieSourceFromVariant(const QVariant &source, QString *lottieSource)
{
    QString sourceString;
    if (source.userType() == QMetaType::QString) {
        sourceString = source.toString();
    } else if (source.userType() == QMetaType::QUrl) {
        const QUrl url = source.toUrl();
        sourceString = url.isLocalFile() ? url.toLocalFile() : url.toString();
    } else {
        return false;
    }

    if (!LottieSourceReference::isLottieSourceText(sourceString)) {
        return false;
    }

    if (lottieSource) {
        *lottieSource = sourceString;
    }
    return true;
}

void Icon::applyAssignedSource()
{
    QString lottieSource;
    if (lottieSourceFromVariant(d->source, &lottieSource)) {
        d->icon = QIcon();
        // 显式 .json/.lottie 一开始就能确定是动画；只有裸 *-motion
        // 还要等待 asset worker 判断动画文件是否存在，因此暂记为 Unknown。
        d->iconManagedStaticFallbackEligible =
            QFileInfo(lottieSource.trimmed()).suffix().isEmpty();
        d->iconManagedLottieSource = lottieSource;
        d->sourceType = d->iconManagedStaticFallbackEligible
            ? UnknownSource
            : LottieSource;
        ensureLottieController();
        return;
    }

    clearLottieSource();
    applyVariantToIcon(d->source, d->icon);
    d->sourceType = hasAssignedSourceValue(d->source)
        ? StaticIconSource
        : UnknownSource;
}

QString Icon::effectiveLottieSource() const
{
    if (d->proxy && !d->proxy->icon().isEmpty()) {
        return {};
    }
    return d->iconManagedLottieSource;
}

void Icon::clearLottieSource()
{
    d->iconManagedLottieSource.clear();
    d->iconManagedStaticFallbackEligible = false;
    d->committedLottieImage = QImage();
}

} // UkuiQuick
