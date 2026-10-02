/*
 * Peony-Qt
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 */

#include "widget-manager-dialog.h"
#include "desktop-widgets-manager.h"
#include "platformwindow.h"

#include <widget-loader.h>
#include <widget-container-item.h>

#include <QScrollArea>
#include <QStackedWidget>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <ksearchlineedit.h>
#include <QPushButton>
#include <QListWidget>
#include <QMimeData>
#include <QDrag>
#include <QMouseEvent>
#include <QIcon>
#include <QPixmap>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>
#include <QListWidgetItem>
#include <QVariantList>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QLocale>
#include <QTranslator>
#include <QQmlContext>
#include <QQmlError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSet>
#include <QMap>
#include <QFileInfo>
#include <QScrollBar>
#include <QEvent>
#include <QTimer>
#include <QShowEvent>
#include <QPalette>
#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QRegion>
#include <QTransform>
#include <QGraphicsDropShadowEffect>
#include <QX11Info>
#include <QGuiApplication>
#include <QScreen>
#include <QCursor>
#include <QMoveEvent>
#include <QWindow>
#include <QQuickWindow>
#include "xatom-helper.h"

#include <KWindowEffects>

#if defined(KY_SDK_WAYLANDHELPER)
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>
#endif
#include <algorithm>

/*!
 * \file widget-manager-dialog.cpp
 * \brief 桌面小插件管理对话框与卡片实现：预览加载、拖拽 MIME、视口内预览数量限制等。
 */

/*!
 * \brief 根据插件元数据解析列表/卡片用图标。
 * \param metadata UkuiQuick 插件元数据。
 * \return 有效 QIcon；无法解析时使用主题图标 application-x-addon。
 */
static QIcon iconForWidgetMetadata(const UkuiQuick::WidgetMetadata &metadata)
{
    QIcon icon;
    const QString iconName = metadata.icon();
    if (!iconName.isEmpty()) {
        if (iconName.startsWith(QLatin1Char('/')) || iconName.startsWith(QLatin1Char(':'))) {
            icon = QIcon(iconName);
        } else {
            icon = QIcon::fromTheme(iconName);
        }
    }
    if (icon.isNull()) {
        icon = QIcon::fromTheme(QStringLiteral("application-x-addon"));
    }
    return icon;
}

/*!
 * \brief 将前景图绘制到同尺寸画布上，便于与卡片底色合成（避免 grab 透明区变黑块）。
 * \param foreground 预览或占位截图。
 * \return 合成后的 QPixmap；输入为空时原样返回。
 */
static QPixmap pixmapCompositedOnCardBackdrop(const QPixmap &foreground)
{
    if (foreground.isNull()) {
        return foreground;
    }
    QPixmap composed(foreground.size());
    composed.setDevicePixelRatio(foreground.devicePixelRatioF());
    QPainter painter(&composed);
    painter.drawPixmap(0, 0, foreground);
    return composed;
}

/*!
 * \brief 为拖拽缩略图叠加圆角裁切与阴影，用于 mouseMoveEvent 中 QDrag::setPixmap。
 * \param foreground 卡片区域截图。
 * \return 带边距的画布图像；输入为空时原样返回。
 */
static QPixmap buildDragCardPixmap(const QPixmap &foreground)
{
    if (foreground.isNull()) {
        return foreground;
    }
    const qreal dpr = qMax<qreal>(1.0, foreground.devicePixelRatioF());
    const QSize logicalSize(qMax(1, qRound(foreground.width() / dpr)),
                            qMax(1, qRound(foreground.height() / dpr)));
    const int shadowMargin = 10;
    const int radius = 6;
    const QSize canvasLogicalSize(logicalSize.width() + shadowMargin * 2,
                                  logicalSize.height() + shadowMargin * 2);
    QPixmap canvas((QSizeF(canvasLogicalSize) * dpr).toSize());
    canvas.setDevicePixelRatio(dpr);
    canvas.fill(Qt::transparent);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const QRectF cardRect(shadowMargin, shadowMargin, logicalSize.width(), logicalSize.height());
    const QPainterPath cardPath = [&cardRect, radius]() {
        QPainterPath path;
        path.addRoundedRect(cardRect, radius, radius);
        return path;
    }();

    painter.fillPath(cardPath.translated(0, 2), QColor(0, 0, 0, 56));
    painter.setClipPath(cardPath);
    painter.drawPixmap(cardRect.toRect(), foreground);
    painter.setClipping(false);
    painter.setPen(QPen(QColor(255, 255, 255, 56), 1));
    painter.drawPath(cardPath);
    return canvas;
}

/*! 管理器内 QQuickWidget 预览圆角半径（逻辑像素）。 */
static constexpr int kManagerPreviewCornerRadius = 12;

static QPixmap roundedPreviewPixmap(const QImage &image, int radius)
{
    if (image.isNull()) {
        return QPixmap();
    }

    QPixmap source = QPixmap::fromImage(image);
    const qreal dpr = qMax<qreal>(1.0, source.devicePixelRatioF());
    const QSize logicalSize(qMax(1, qRound(source.width() / dpr)),
                            qMax(1, qRound(source.height() / dpr)));
    QPixmap rounded(source.size());
    rounded.setDevicePixelRatio(dpr);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QPainterPath clipPath;
    clipPath.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(logicalSize)), radius, radius);
    painter.setClipPath(clipPath);
    painter.drawPixmap(QRect(QPoint(0, 0), logicalSize), source);
    return rounded;
}

/*!
 * \brief 将圆角矩形路径栅格化为 QRegion，供 QWidget::setMask 使用。
 * \param path 圆角路径（通常为单矩形圆角）。
 * \return 与路径近似的区域（通过缩放展平再还原以减轻锯齿）。
 */
static QRegion maskRegionFromRoundedPath(const QPainterPath &path)
{
    constexpr qreal kFlattenScale = 8.0;
    const QTransform scaleUp = QTransform::fromScale(kFlattenScale, kFlattenScale);
    const QList<QPolygonF> scaledPolys = path.toSubpathPolygons(scaleUp);
    QRegion region;
    for (const QPolygonF &scaledPoly : scaledPolys) {
        QPolygon logicalPoly;
        logicalPoly.reserve(scaledPoly.size());
        for (const QPointF &pt : scaledPoly) {
            logicalPoly << QPoint(qRound(pt.x() / kFlattenScale), qRound(pt.y() / kFlattenScale));
        }
        region += QRegion(logicalPoly);
    }
    return region;
}

/*!
 * \brief 为控件设置圆角样式表（可选 1px 边框与背景色）。
 * \param widget 目标控件。
 * \param radius 圆角半径（像素）。
 * \param backgroundCss 传给 stylesheet 的 background-color，空则不加背景。
 * \param drawBorder 为 false 时不绘制边框，避免与 QQuickWidget 内容叠边裁切。
 */
static void applyRoundedEdgeStyle(QWidget *widget, int radius, const QString &backgroundCss = QString(),
                                    bool drawBorder = true)
{
    if (!widget) {
        return;
    }
    widget->setAttribute(Qt::WA_StyledBackground, true);
    QString style = drawBorder
        ? QStringLiteral("border: 1px solid rgba(255, 255, 255, 52); border-radius: %1px;").arg(radius)
        : QStringLiteral("border: none; border-radius: %1px;").arg(radius);
    if (!backgroundCss.isEmpty()) {
        style += QStringLiteral("background-color: %1;").arg(backgroundCss);
    }
    widget->setStyleSheet(style);
}

/*!
 * \brief 按控件当前 rect 生成圆角遮罩并 setMask。
 * \param widget 目标控件。
 * \param radius 圆角半径（像素）。
 */
static void applyRoundedMask(QWidget *widget, int radius)
{
    if (!widget) {
        return;
    }
    const QRect rect = widget->rect();
    if (rect.isEmpty()) {
        return;
    }
    QPainterPath path;
    // 使用完整像素矩形生成遮罩；半像素内缩 + QRegion 取整易吃掉最右/最下一列像素。
    path.addRoundedRect(QRectF(rect), qreal(radius), qreal(radius));
    widget->setMask(maskRegionFromRoundedPath(path));
}

/*!
 * \brief 自祖先链查找第一个启用 autoFillBackground 的调色板 Window 色，作为 QQuickWidget 清屏色。
 * \param anchor 起始控件（通常为 QQuickWidget）。
 * \return 用作 clearColor 的 QColor。
 */
static QColor managerPreviewBackdropColor(const QWidget *anchor)
{
    for (const QWidget *p = anchor; p; p = p->parentWidget()) {
        if (p->autoFillBackground()) {
            return p->palette().color(QPalette::Window);
        }
    }
    return QApplication::palette().color(QPalette::Window);
}

/*!
 * \brief 从 ManagerVariants 项读取本地化字符串（与 WidgetMetadata::name/description 规则一致）。
 * \param variant 单条变体配置。
 * \param key 字段名，如 "name"、"description"。
 */
static QString localizedVariantString(const QVariantMap &variant, const QString &key)
{
    const QString localeKey = key + QLatin1Char('[') + QLocale::system().name() + QLatin1Char(']');
    QString value = variant.value(localeKey).toString();
    if (value.isEmpty()) {
        value = variant.value(key).toString();
    }
    return value;
}

static QHash<QString, QTranslator *> s_managerPreviewTranslators;

/*!
 * \brief 管理器预览加载插件 .qm（与 WidgetQuickItemPrivate::installTranslator 一致）。
 */
static void installManagerPreviewTranslator(const UkuiQuick::WidgetMetadata &metadata)
{
    const QString wid = metadata.id();
    if (s_managerPreviewTranslators.contains(wid)) {
        return;
    }
    const QString i18n = metadata.contents().value(QStringLiteral("I18n")).toString();
    if (i18n.isEmpty()) {
        return;
    }
    auto *translator = new QTranslator(QCoreApplication::instance());
    const bool loaded = translator->load(QLocale(), i18n, QStringLiteral("_"), metadata.root().absolutePath());
    if (loaded) {
        QCoreApplication::installTranslator(translator);
        s_managerPreviewTranslators.insert(wid, translator);
    } else {
        qWarning() << "Widget manager preview: failed to load translation for" << wid
                   << "locale" << QLocale::system().name();
        delete translator;
    }
}

/*!
 * \brief 根据变体映射计算管理器内 QQuickWidget 的固定逻辑尺寸（仅影响预览，非桌面最终像素）。
 * \param variant 可含 previewSize: [w,h] 或 sizeHint: [cols, rows]（与桌面网格公式一致）。
 * \return 有效 QSize；缺省或非法时返回 120×80。
 */
static QSize previewPixelSizeFromVariant(const QVariantMap &variant)
{
    // 仅影响管理器内 QQuickWidget 尺寸。桌面像素可与 previewSize 不同，由 ManagerVariants.desktopSize + addWidgetFromVariant 写入 width/height。
    // 插件可在 ManagerVariants 中提供 previewSize: [w, h]，与预览 QML 设计分辨率一致，避免缩放变形。
    const QVariant previewSizeVariant = variant.value(QStringLiteral("previewSize"));
    if (previewSizeVariant.canConvert<QVariantList>()) {
        const QVariantList ps = previewSizeVariant.toList();
        if (ps.size() == 2) {
            const int pw = qMax(1, ps.at(0).toInt());
            const int ph = qMax(1, ps.at(1).toInt());
            return QSize(pw, ph);
        }
    }
    const QVariant sizeHintVariant = variant.value("sizeHint");
    if (!sizeHintVariant.canConvert<QVariantList>()) {
        return QSize(120, 80);
    }
    const QVariantList hint = sizeHintVariant.toList();
    if (hint.size() != 2) {
        return QSize(120, 80);
    }
    const int w = qMax(1, hint.at(0).toInt());
    const int h = qMax(1, hint.at(1).toInt());
    return QSize(48 + w * 36, 34 + h * 24);
}

/*!
 * \brief 解析预览用 QML 的本地 file URL 列表（去重）。
 * \param metadata 插件根目录来自 metadata.root()。
 * \param variant 可含 previewMain；否则使用 Contents.Preview。
 * \return 存在的文件对应的 QUrl 列表；无有效项时为空，卡片将仅显示占位。
 */
static QList<QUrl> previewUrlsFromMetadata(const UkuiQuick::WidgetMetadata &metadata, const QVariantMap &variant)
{
    QList<QUrl> urls;
    QSet<QString> seenPaths;
    auto appendAbsPathIfValid = [&urls, &seenPaths](const QString &absPath) {
        if (absPath.isEmpty()) {
            return;
        }
        const QFileInfo fileInfo(absPath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            return;
        }
        if (seenPaths.contains(absPath)) {
            return;
        }
        seenPaths.insert(absPath);
        urls.append(QUrl::fromLocalFile(absPath));
    };
    auto appendIfValid = [&metadata, &appendAbsPathIfValid](const QString &relativePath) {
        if (relativePath.isEmpty()) {
            return;
        }
        const QString absPath = metadata.root().absoluteFilePath(relativePath);
        appendAbsPathIfValid(absPath);

        // 兼容部分插件安装时把子目录文件“铺平”到根目录的场景
        const QFileInfo relInfo(relativePath);
        const QString fileNameOnly = relInfo.fileName();
        if (!fileNameOnly.isEmpty() && fileNameOnly != relativePath) {
            const QString flattenedAbsPath = metadata.root().absoluteFilePath(fileNameOnly);
            appendAbsPathIfValid(flattenedAbsPath);
        }
    };

    QString previewMain = variant.value("previewMain").toString();
    if (previewMain.isEmpty()) {
        previewMain = metadata.contents().value("Preview").toString();
    }
    appendIfValid(previewMain);

    return urls;
}

/*!
 * \brief 判断插件是否出现在「桌面小插件管理」列表中。
 * \param metadata Contents.ShowInManager 或 ManagerVisible 为真时返回 true；均未配置时返回 false。
 * \return 是否显示。
 */
static bool showInWidgetManager(const UkuiQuick::WidgetMetadata &metadata)
{
    const QVariantMap contents = metadata.contents();
    // 仅在 metadata.json 的 Contents 中显式开启时才出现在「桌面小插件管理」列表（不影响桌面直接加载）
    if (contents.contains(QStringLiteral("ShowInManager"))) {
        return contents.value(QStringLiteral("ShowInManager")).toBool();
    }
    if (contents.contains(QStringLiteral("ManagerVisible"))) {
        return contents.value(QStringLiteral("ManagerVisible")).toBool();
    }
    return false;
}

// ──────────────── WidgetCard ────────────────

WidgetCard::WidgetCard(const UkuiQuick::WidgetMetadata &metadata,
                       const QVariantMap &variant,
                       QQmlEngine *sharedPreviewEngine,
                       QWidget *parent,
                       int previewSlotHeight)
    : QFrame(parent)
    , m_widgetId(metadata.id())
    , m_variant(variant)
    , m_previewSlotHeight(previewSlotHeight)
    , m_sharedPreviewEngine(sharedPreviewEngine)
{
    setupUi(metadata);
}

/*!
 * \brief 搭建卡片布局：预览框（占位/后续 QQuickWidget）、标题与可选描述。
 * \param metadata 用于名称/描述回退及图标路径解析（此处主要用 name/description）。
 */
void WidgetCard::setupUi(const UkuiQuick::WidgetMetadata &metadata)
{
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setAutoFillBackground(false);

    QString displayName = localizedVariantString(m_variant, QStringLiteral("name"));
    if (displayName.isEmpty()) {
        displayName = metadata.name();
    }
    if (displayName.isEmpty()) {
        displayName = metadata.id();
    }
    QString desc = localizedVariantString(m_variant, QStringLiteral("description"));
    if (desc.isEmpty()) {
        desc = metadata.description();
    }
    installManagerPreviewTranslator(metadata);
    m_searchText = QString("%1 %2 %3").arg(m_widgetId, displayName, desc).toLower();

    m_previewSize = previewPixelSizeFromVariant(m_variant);
    if (m_previewSlotHeight <= 0) {
        m_previewSlotHeight = m_previewSize.height();
    }
    const int sideMargin = 6;
    const int btnRowMinW = 160;
    const int cardW = qMax(m_previewSize.width() + 2 * sideMargin, btnRowMinW);
    const int captionMaxW = cardW - 2 * sideMargin;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(sideMargin, sideMargin, sideMargin, sideMargin);
    mainLayout->setSpacing(6);

    m_previewFrame = new QFrame(this);
    m_previewFrame->setFrameShape(QFrame::NoFrame);
    m_previewFrame->setFixedHeight(m_previewSlotHeight);
    m_previewFrame->setAutoFillBackground(false);
    auto *previewShadow = new QGraphicsDropShadowEffect(m_previewFrame);
    previewShadow->setBlurRadius(18);
    previewShadow->setOffset(0, 2);
    previewShadow->setColor(QColor(0, 0, 0, 36));
    m_previewFrame->setGraphicsEffect(previewShadow);
    m_previewLayout = new QVBoxLayout(m_previewFrame);
    m_previewLayout->setContentsMargins(0, 0, 0, 0);
    m_previewLayout->setSpacing(0);
    m_previewUrls = previewUrlsFromMetadata(metadata, m_variant);
    m_previewLayout->addStretch(1);
    m_previewStack = new QStackedWidget(m_previewFrame);
    m_previewStack->setFixedSize(m_previewSize);
    m_previewPlaceholder = new QLabel(displayName, m_previewStack);
    m_previewPlaceholder->setAlignment(Qt::AlignCenter);
    m_previewPlaceholder->setFixedSize(m_previewSize);
    m_previewPlaceholder->setAutoFillBackground(true);
    {
        QPalette pp = m_previewPlaceholder->palette();
        m_previewPlaceholder->setPalette(pp);
    }
    applyRoundedMask(m_previewStack, kManagerPreviewCornerRadius);
    applyRoundedEdgeStyle(m_previewStack, kManagerPreviewCornerRadius, QStringLiteral("palette(base)"), false);
    m_previewStack->addWidget(m_previewPlaceholder);
    m_previewLayout->addWidget(m_previewStack, 0, Qt::AlignHCenter);
    m_previewLayout->addStretch(1);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignHCenter);
    m_titleLabel->setWordWrap(false);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setMaximumWidth(captionMaxW);
    const int titleH = m_titleLabel->fontMetrics().height();
    m_titleLabel->setFixedHeight(titleH);
    m_titleLabel->setText(m_titleLabel->fontMetrics().elidedText(displayName, Qt::ElideRight, captionMaxW));
    m_titleLabel->setToolTip(displayName);

    m_descLabel = new QLabel(this);
    m_descLabel->setAlignment(Qt::AlignHCenter);
    m_descLabel->setWordWrap(false);
    {
        QFont df = m_descLabel->font();
        df.setPointSize(qMax(8, df.pointSize() - 1));
        m_descLabel->setFont(df);
        QPalette dp = m_descLabel->palette();
        dp.setColor(QPalette::WindowText, dp.color(QPalette::Disabled, QPalette::Text));
        m_descLabel->setPalette(dp);
    }
    m_descLabel->setMaximumWidth(captionMaxW);
    const int descH = m_descLabel->fontMetrics().height();
    m_descLabel->setFixedHeight(descH);
    if (desc.isEmpty()) {
        m_descLabel->hide();
    } else {
        m_descLabel->setText(m_descLabel->fontMetrics().elidedText(desc, Qt::ElideRight, captionMaxW));
        m_descLabel->setToolTip(desc);
    }

    mainLayout->addWidget(m_previewFrame, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    if (!desc.isEmpty()) {
        mainLayout->addWidget(m_descLabel, 0, Qt::AlignHCenter);
    }

    const int gapTitleDesc = desc.isEmpty() ? 0 : 6;
    const int cardH = sideMargin + m_previewSlotHeight + 6 + titleH + gapTitleDesc
        + (desc.isEmpty() ? 0 : descH) + sideMargin;
    setFixedSize(cardW, cardH);
}

void WidgetCard::setPreviewActive(bool active)
{
    if (active) {
        if (isHidden()) {
            return;
        }
        createPreviewWidget();
    } else {
        destroyPreviewWidget();
    }
}

/*!
 * \brief 创建离屏 QQuickWidget，注入上下文属性并将 grabFramebuffer 结果显示到 QLabel。
 *
 * QQuickWidget 不进入卡片控件树，避免其 OpenGL 合成路径与 QWidget 刷新不同步。
 */
void WidgetCard::createPreviewWidget()
{
    if (m_previewWidget || !m_previewFrame || !m_previewStack) {
        return;
    }
    for (const QUrl &previewUrl : m_previewUrls) {
        if (!previewUrl.isValid()) {
            continue;
        }
        // 方案说明（动态预览刷新）：
        // 在 X11 + 共享 QQmlEngine + QStackedWidget 嵌入场景下，已被实测证实：
        //   - QML 场景脏检测、grabFramebuffer 与 paintEvent 派发都正常；
        //   - 但 QQuickWidget OpenGL 模式下走 QPlatformBackingStore::composeAndFlush
        //     的合成路径不会主动重采样最新 FBO 纹理，导致秒针等动态绑定只有在
        //     hover/scroll/click 等外部事件触发整个 backingstore 重合成时才"跳一下"。
        // 因此放弃让 QQuickWidget 自己上屏，改为：
        //   1) QQuickWidget 作为纯离屏渲染源（无父窗口 + WA_DontShowOnScreen），不进入卡片控件树；
        //   2) m_previewStack 中放 QLabel，由定时器把 grabFramebuffer() 的 QImage 写成 pixmap。
        // 渲染开销：9 个预览 × 最大 209×209 × 4 fps，GL readback 完全可接受。
        QQuickWidget *qw = m_sharedPreviewEngine
            ? new QQuickWidget(m_sharedPreviewEngine, nullptr)
            : new QQuickWidget(nullptr);
        qw->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
        qw->setAttribute(Qt::WA_DontShowOnScreen, true);
        qw->setAttribute(Qt::WA_ShowWithoutActivating, true);
        qw->setResizeMode(QQuickWidget::SizeRootObjectToView);
        qw->setClearColor(managerPreviewBackdropColor(qw));
        qw->setAttribute(Qt::WA_TranslucentBackground, false);
        qw->setFixedSize(m_previewSize);
        QQmlContext *rootCtx = qw->rootContext();
        rootCtx->setContextProperty(QStringLiteral("widgetPreviewMode"), true);
        rootCtx->setContextProperty(QStringLiteral("widgetPreviewVariantId"),
                                    m_variant.value(QStringLiteral("id")).toString());
        qw->setSource(previewUrl);
        if (qw->status() == QQuickWidget::Error) {
            const QList<QQmlError> errors = qw->errors();
            qWarning() << "WidgetManagerDialog preview load failed:" << m_widgetId << previewUrl.toString();
            for (const QQmlError &error : errors) {
                qWarning() << "  " << error.toString();
            }
            qw->deleteLater();
            continue;
        }
        qw->setFocusPolicy(Qt::NoFocus);
        qw->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        // 必须 show() 才会初始化 OpenGL context + FBO；WA_DontShowOnScreen 让它不真正占屏幕。
        // 额外移动到屏幕外，避免个别平台忽略 WA_DontShowOnScreen 时造成预览叠影。
        qw->move(-10000, -10000);
        qw->show();

        QLabel *display = new QLabel(m_previewStack);
        display->setFixedSize(m_previewSize);
        display->setAlignment(Qt::AlignCenter);
        display->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        display->setStyleSheet(QStringLiteral("background: transparent; border: none;"));

        auto refresh = [qw, display]() {
            if (qw->status() != QQuickWidget::Ready) {
                return;
            }
            const QImage img = qw->grabFramebuffer();
            if (img.isNull()) {
                return;
            }
            display->setPixmap(roundedPreviewPixmap(img, kManagerPreviewCornerRadius));
        };

        auto *kick = new QTimer(qw);
        kick->setInterval(250);
        connect(kick, &QTimer::timeout, qw, refresh);
        kick->start();
        // 启动前若已 Ready，立刻刷一帧避免初始空白。
        refresh();

        m_previewWidget = qw;
        m_previewDisplay = display;
        m_previewStack->addWidget(display);
        connect(qw, &QQuickWidget::statusChanged, this, [this, refresh](QQuickWidget::Status status) {
            if (!m_previewStack || !m_previewDisplay) {
                return;
            }
            if (status == QQuickWidget::Ready) {
                refresh();
                m_previewStack->setCurrentWidget(m_previewDisplay);
            } else if (status == QQuickWidget::Error) {
                m_previewStack->setCurrentWidget(m_previewPlaceholder);
            }
        });
        if (qw->status() == QQuickWidget::Ready) {
            m_previewStack->setCurrentWidget(display);
        } else {
            m_previewStack->setCurrentWidget(m_previewPlaceholder);
        }
        qDebug() << "WidgetManagerDialog preview loaded:" << m_widgetId << previewUrl.toString();
        return;
    }
}

void WidgetCard::destroyPreviewWidget()
{
    if (!m_previewWidget && !m_previewDisplay) {
        return;
    }
    if (m_previewStack && m_previewDisplay) {
        m_previewStack->removeWidget(m_previewDisplay);
        m_previewStack->setCurrentWidget(m_previewPlaceholder);
    }
    if (m_previewDisplay) {
        m_previewDisplay->deleteLater();
        m_previewDisplay = nullptr;
    }
    if (m_previewWidget) {
        m_previewWidget->deleteLater();
        m_previewWidget = nullptr;
    }
    if (m_previewPlaceholder) {
        m_previewPlaceholder->show();
    }
}

bool WidgetCard::matchesKeyword(const QString &keyword) const
{
    if (keyword.isEmpty()) {
        return true;
    }
    return m_searchText.contains(keyword);
}

bool WidgetCard::matchesPlugin(const QString &pluginId) const
{
    if (pluginId.isEmpty()) {
        return true;
    }
    return m_widgetId == pluginId;
}

bool WidgetCard::isPointInPreview(const QPoint &posInCard) const
{
    if (!m_previewStack) {
        return false;
    }
    const QPoint topLeft = m_previewStack->mapTo(this, QPoint(0, 0));
    return QRect(topLeft, m_previewStack->size()).contains(posInCard);
}

void WidgetCard::mousePressEvent(QMouseEvent *event)
{
    m_pressInPreview = false;
    if (event->button() == Qt::LeftButton && isPointInPreview(event->pos())) {
        m_dragStartPos = event->pos();
        m_pressInPreview = true;
    }
    QFrame::mousePressEvent(event);
}

void WidgetCard::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_pressInPreview || !(event->buttons() & Qt::LeftButton)) {
        QFrame::mouseMoveEvent(event);
        return;
    }
    if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
        QFrame::mouseMoveEvent(event);
        return;
    }
    QVariantMap payload = m_variant;
    payload.insert("widgetId", m_widgetId);
    QMimeData *mimeData = new QMimeData;
    QJsonObject obj = QJsonObject::fromVariantMap(payload);
    QJsonDocument doc(obj);
    const QByteArray jsonUtf8 = doc.toJson(QJsonDocument::Compact);
    mimeData->setData(QStringLiteral("application/x-peony-widget"), jsonUtf8);
    mimeData->setData(QStringLiteral("application/x-peony-widget-drag"), QByteArrayLiteral("1"));
    mimeData->setData(QStringLiteral("application/json"), jsonUtf8);
    mimeData->setText(QString::fromUtf8(jsonUtf8));

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    QPixmap thumb;
    if (m_previewWidget) {
        thumb = m_previewWidget->grab();
    } else if (m_previewPlaceholder) {
        thumb = m_previewPlaceholder->grab();
    }
    if (thumb.isNull()) {
        const QRect grabRect = m_previewFrame ? m_previewFrame->geometry() : rect();
        thumb = grab(grabRect);
    }
    thumb = buildDragCardPixmap(pixmapCompositedOnCardBackdrop(thumb));
    if (!thumb.isNull()) {
        const int maxSide = 160;
        const qreal thumbDpr = qMax<qreal>(1.0, thumb.devicePixelRatioF());
        const QSize logicalSize(qRound(thumb.width() / thumbDpr), qRound(thumb.height() / thumbDpr));
        if (logicalSize.width() > maxSide || logicalSize.height() > maxSide) {
            const QSize scaledLogical = logicalSize.scaled(maxSide, maxSide, Qt::KeepAspectRatio);
            thumb = thumb.scaled((QSizeF(scaledLogical) * thumbDpr).toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            thumb.setDevicePixelRatio(thumbDpr);
        }
        drag->setPixmap(thumb);
        const qreal dpr = qMax<qreal>(1.0, thumb.devicePixelRatioF());
        drag->setHotSpot(QPoint(qRound(thumb.width() / dpr / 2.0),
                                qRound(thumb.height() / dpr / 2.0)));
    }
    // Desktop DropArea 在非 Ctrl 时会选 MoveAction；仅 CopyAction 会导致动作不匹配、放置失败
    WidgetManagerDialog::beginPaletteDragFromCard();
    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
    WidgetManagerDialog::endPaletteDragFromCard();
    QFrame::mouseMoveEvent(event);
}

void WidgetCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_pressInPreview) {
        if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance()
            && isPointInPreview(event->pos())) {
            Q_EMIT addRequested(m_widgetId, m_variant);
        }
    }
    m_pressInPreview = false;
    QFrame::mouseReleaseEvent(event);
}

// ──────────────── WidgetManagerDialog ────────────────

static WidgetManagerDialog *s_instance = nullptr;

WidgetManagerDialog *WidgetManagerDialog::instance()
{
    if (!s_instance) {
        s_instance = new WidgetManagerDialog;
        // 窗口关闭时不销毁，只隐藏，方便复用
        s_instance->setAttribute(Qt::WA_DeleteOnClose, false);
    }
    return s_instance;
}

WidgetManagerDialog::WidgetManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Desktop Widget Manager"));
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    // 总宽 938 = 侧栏 240 + 预览区 698；高度 560
    setFixedSize(938, 560);
    setupUi();
    loadWidgets();
}

void WidgetManagerDialog::beginPaletteDragFromCard()
{
    if (!s_instance || !s_instance->isVisible()) {
        return;
    }
    s_instance->schedulePaletteHideWhenDragActive();
}

void WidgetManagerDialog::endPaletteDragFromCard()
{
    if (!s_instance) {
        return;
    }
    s_instance->finalizePaletteDragAfterExec();
}

void WidgetManagerDialog::schedulePaletteHideWhenDragActive()
{
    ++m_paletteHideSequence;
    const int seq = m_paletteHideSequence;
    QTimer::singleShot(0, this, [this, seq]() {
        if (seq != m_paletteHideSequence) {
            return;
        }
        if (!isVisible()) {
            return;
        }
        m_pendingShowAfterPaletteDrag = true;
        hide();
    });
}

void WidgetManagerDialog::finalizePaletteDragAfterExec()
{
    ++m_paletteHideSequence;
    const bool restore = m_pendingShowAfterPaletteDrag;
    m_pendingShowAfterPaletteDrag = false;
    if (!restore) {
        return;
    }
    if (isVisible()) {
        repositionToBottomCenter();
        raise();
        activateWindow();
        return;
    }
    repositionToBottomCenter();
    show();
    raise();
    activateWindow();
}

void WidgetManagerDialog::repositionToBottomCenter()
{
    QScreen *screen = nullptr;
    if (QWindow *wh = windowHandle()) {
        screen = wh->screen();
    }
    if (!screen) {
        screen = QGuiApplication::screenAt(QCursor::pos());
    }
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }
    const QRect ag = screen->availableGeometry();
    const int w = width();
    const int h = height();
    const int x = ag.x() + (ag.width() - w) / 2;
    const int y = ag.bottom() - h + 1;
    const QPoint target(x, y);
    if (pos() == target) {
        return;
    }
    m_inProgrammaticMove = true;
    move(target);
    m_inProgrammaticMove = false;
}

void WidgetManagerDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *body = new QWidget(this);
    body->setAttribute(Qt::WA_TranslucentBackground, true);
    body->setAutoFillBackground(false);
    body->setFixedWidth(938);
    QHBoxLayout *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    // 左侧：不铺底色，由窗口毛玻璃透出桌面；搜索与列表同一列
    QWidget *leftPane = new QWidget(body);
    leftPane->setFixedWidth(240);
    leftPane->setAttribute(Qt::WA_TranslucentBackground, true);
    leftPane->setAutoFillBackground(false);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(12, 12, 12, 12);
    leftLayout->setSpacing(10);

    m_searchEdit = new kdk::KSearchLineEdit(leftPane);
    m_searchEdit->setPlaceholderText(tr("Search application cards"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(32);

    m_pluginList = new QListWidget(leftPane);
    m_pluginList->setIconSize(QSize(22, 22));
    m_pluginList->setSpacing(2);
    m_pluginList->setFrameShape(QFrame::NoFrame);
    m_pluginList->setAutoFillBackground(false);
    m_pluginList->viewport()->setAutoFillBackground(false);

    leftLayout->addWidget(m_searchEdit);
    leftLayout->addWidget(m_pluginList, 1);

    // 右侧：放入不透明窗口容器；底部说明与完成按钮同一水平布局
    QWidget *rightPane = new QWidget(body);
    rightPane->setFixedWidth(698);
    QVBoxLayout *rightPaneLayout = new QVBoxLayout(rightPane);
    rightPaneLayout->setContentsMargins(0, 0, 0, 0);
    rightPaneLayout->setSpacing(0);

    QWidget *rightWindow = new QWidget(rightPane);
    rightWindow->setAutoFillBackground(true);
    {
        QPalette rightPalette = rightWindow->palette();
        rightWindow->setPalette(rightPalette);
    }
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWindow);
    rightLayout->setContentsMargins(12, 12, 12, 10);
    rightLayout->setSpacing(8);
    m_rightTitleLabel = new QLabel(rightWindow);
    QFont titleFont = m_rightTitleLabel->font();
    titleFont.setBold(true);
    m_rightTitleLabel->setFont(titleFont);
    m_rightTitleLabel->setWordWrap(false);
    m_rightTitleLabel->setMaximumWidth(680);
    rightLayout->addWidget(m_rightTitleLabel);

    m_scrollArea = new QScrollArea(rightWindow);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setAutoFillBackground(true);
    m_scrollArea->viewport()->setAutoFillBackground(true);
    {
        QPalette scrollPalette = m_scrollArea->palette();
        m_scrollArea->setPalette(scrollPalette);
        m_scrollArea->viewport()->setPalette(scrollPalette);
    }

    m_cardContainer = new QWidget(m_scrollArea);
    m_cardContainer->setAutoFillBackground(true);
    {
        QPalette cardPalette = m_cardContainer->palette();
        m_cardContainer->setPalette(cardPalette);
    }
    m_cardLayout = new QGridLayout(m_cardContainer);
    m_cardLayout->setContentsMargins(2, 4, 2, 4);
    m_cardLayout->setHorizontalSpacing(6);
    m_cardLayout->setVerticalSpacing(10);
    m_cardLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_scrollArea->setWidget(m_cardContainer);

    rightLayout->addWidget(m_scrollArea, 1);
    m_scrollArea->viewport()->installEventFilter(this);
    connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        schedulePreviewUpdate();
    });

    QFrame *footerSeparator = new QFrame(rightWindow);
    footerSeparator->setFrameShape(QFrame::HLine);
    rightLayout->addWidget(footerSeparator);

    QHBoxLayout *rightBottomRow = new QHBoxLayout;
    rightBottomRow->setContentsMargins(0, 6, 0, 0);
    QLabel *hintLabel = new QLabel(tr("Click a card to add it to the desktop, or drag a card onto the desktop."), rightWindow);
    rightBottomRow->addWidget(hintLabel, 1, Qt::AlignLeft | Qt::AlignVCenter);
    rightBottomRow->addStretch(1);
    QPushButton *closeBtn = new QPushButton(tr("Done"), rightWindow);
    closeBtn->setFixedSize(84, 32);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    rightBottomRow->addWidget(closeBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    rightLayout->addLayout(rightBottomRow);
    rightPaneLayout->addWidget(rightWindow);

    bodyLayout->addWidget(leftPane);
    bodyLayout->addWidget(rightPane, 1);
    mainLayout->addWidget(body, 1);

    m_previewDebounceTimer = new QTimer(this);
    m_previewDebounceTimer->setSingleShot(true);
    m_previewDebounceTimer->setInterval(100);
    connect(m_previewDebounceTimer, &QTimer::timeout, this, &WidgetManagerDialog::updateVisiblePreviewsNow);

    connect(m_searchEdit, &kdk::KSearchLineEdit::textChanged, this, &WidgetManagerDialog::onSearchTextChanged);
    connect(m_pluginList, &QListWidget::itemSelectionChanged, this, &WidgetManagerDialog::onPluginChanged);

    repairRightScrollAreaBackground();
}

void WidgetManagerDialog::ensureSharedPreviewEngine()
{
    if (!m_sharedPreviewEngine) {
        m_sharedPreviewEngine = new QQmlEngine(this);
    }
}

/*!
 * \brief 枚举 WidgetLoader 中 Desktop 插件，按 ShowInManager 过滤，并为每条 ManagerVariants 项生成 WidgetCard。
 *
 * 左侧列表填充「全部」与各插件名；无变体时插入一条空 variant。最后调度一次可见预览更新。
 */
void WidgetManagerDialog::loadWidgets()
{
    // 清除旧卡片
    for (auto card : qAsConst(m_cards)) {
        m_cardLayout->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();
    ensureSharedPreviewEngine();

    // 通过 ukui-quick WidgetLoader 枚举所有已安装的 Desktop 插件
    auto &loader = UkuiQuick::WidgetLoader::globalLoader();
    m_allMetadata = loader.widgetsMetadata();
    m_pluginList->clear();
    {
        QListWidgetItem *allItem = new QListWidgetItem(tr("All"), m_pluginList);
        allItem->setIcon(QIcon::fromTheme(QStringLiteral("view-grid-symbolic")));
        allItem->setData(Qt::UserRole + 1, QStringLiteral("all"));
        if (allItem->icon().isNull()) {
            allItem->setIcon(QIcon::fromTheme(QStringLiteral("applications-other")));
        }
    }
    QMap<QString, QString> pluginIdToName;
    QMap<QString, QIcon> pluginIdToIcon;

    int maxVariantCountInRow = 1;
    constexpr int kColumns = 3;
    int cardIndex = 0;

    for (const UkuiQuick::WidgetMetadata &meta : qAsConst(m_allMetadata)) {
        // 只展示标记为 Desktop 的插件
        if (!(meta.showIn() & UkuiQuick::WidgetMetadata::Desktop)) {
            continue;
        }
        if (!showInWidgetManager(meta)) {
            continue;
        }
        QString pluginName = meta.name();
        if (pluginName.isEmpty()) {
            pluginName = meta.id();
        }
        pluginIdToName.insert(meta.id(), pluginName);
        pluginIdToIcon.insert(meta.id(), iconForWidgetMetadata(meta));

        QList<QVariantMap> variants;
        const QVariant variantsValue = meta.contents().value("ManagerVariants");
        if (variantsValue.canConvert<QVariantList>()) {
            const QVariantList variantsList = variantsValue.toList();
            for (const QVariant &value : variantsList) {
                if (value.canConvert<QVariantMap>()) {
                    variants << value.toMap();
                }
            }
        }
        if (variants.isEmpty()) {
            variants << QVariantMap();
        }

        maxVariantCountInRow = qMax(maxVariantCountInRow, variants.size());

        int rowSlotH = 0;
        for (const QVariantMap &v : qAsConst(variants)) {
            rowSlotH = qMax(rowSlotH, previewPixelSizeFromVariant(v).height());
        }

        for (const QVariantMap &variant : qAsConst(variants)) {
            WidgetCard *card = new WidgetCard(meta, variant, m_sharedPreviewEngine, m_cardContainer, rowSlotH);

            connect(card, &WidgetCard::addRequested, this, &WidgetManagerDialog::onAddWidget);
            const int row = cardIndex / kColumns;
            const int col = cardIndex % kColumns;
            m_cardLayout->addWidget(card, row, col);
            m_cards.append(card);
            ++cardIndex;
        }
    }
    for (auto it = pluginIdToName.cbegin(); it != pluginIdToName.cend(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(it.value(), m_pluginList);
        item->setData(Qt::UserRole + 1, QStringLiteral("plugin"));
        item->setData(Qt::UserRole, it.key());
        item->setIcon(pluginIdToIcon.value(it.key()));
    }
    m_pluginList->setCurrentRow(0);
    updateRightPaneTitle();

    if (m_cards.isEmpty()) {
        QLabel *emptyLabel = new QLabel(tr("No desktop widgets found.\nInstall widget packages to get started."), m_cardContainer);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setWordWrap(true);
        m_cardLayout->addWidget(emptyLabel, 0, 0, 1, qMax(3, maxVariantCountInRow), Qt::AlignCenter);
    }
    updateVisiblePreviewsNow();
}

PlatformWindow *WidgetManagerDialog::getPrimaryWindow() const
{
    for (auto window : PlatformWindow::allWindows()) {
        if (window->id() == 0) {
            return window;
        }
    }
    // 如果没有找到主屏，返回第一个可用窗口
    auto windows = PlatformWindow::allWindows();
    if (!windows.isEmpty()) {
        return windows.first();
    }
    return nullptr;
}

void WidgetManagerDialog::onAddWidget(const QString &widgetId, const QVariantMap &variant)
{
    PlatformWindow *window = getPrimaryWindow();
    if (!window) {
        qWarning() << "WidgetManagerDialog: no platform window available";
        return;
    }

    // 统一走 addWidgetFromVariant，确保点击添加也复用“就近空位放置 + 防重叠”逻辑。
    window->addWidgetFromVariant(widgetId, variant);

}

void WidgetManagerDialog::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text)
    applyFilters();
}

void WidgetManagerDialog::onPluginChanged()
{
    updateRightPaneTitle();
    applyFilters();
}

void WidgetManagerDialog::updateRightPaneTitle()
{
    if (!m_rightTitleLabel) {
        return;
    }
    if (!m_pluginList || !m_pluginList->currentItem() || m_pluginList->currentRow() <= 0) {
        m_rightTitleLabel->setText(tr("All desktop widgets"));
        return;
    }
    m_rightTitleLabel->setText(m_pluginList->currentItem()->text());
}

void WidgetManagerDialog::refreshCards()
{
    schedulePreviewUpdate();
}

/*!
 * \brief 根据搜索框小写文本与左侧选中插件 Id 设置卡片 visible，并调度预览与右侧底色修复。
 *
 * 被隐藏的卡片不销毁预览实例，以便切回分类时快速恢复。
 */
void WidgetManagerDialog::applyFilters()
{
    // 不要用 setUpdatesEnabled(false) 包住整页刷新：与嵌入的 QQuickView（独立窗口）
    // 合成时容易导致切换左侧插件类型后视口残影、错位或长时间不刷新。
    const QString keyword = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : QString();
    QString selectedPluginId;
    if (m_pluginList && m_pluginList->currentItem() && m_pluginList->currentRow() >= 0) {
        const QListWidgetItem *current = m_pluginList->currentItem();
        const QString mode = current->data(Qt::UserRole + 1).toString();
        if (mode == QLatin1String("plugin")) {
            selectedPluginId = current->data(Qt::UserRole).toString();
        }
    }
    for (auto card : qAsConst(m_cards)) {
        const bool matched = card->matchesKeyword(keyword)
            && card->matchesPlugin(selectedPluginId);
        card->setVisible(matched);
        // 筛选隐藏时不销毁预览：再次切回分类时可立即显示，避免反复加载与首帧黑闪。
    }
    schedulePreviewUpdate();
    repairRightScrollAreaBackground();
}

void WidgetManagerDialog::schedulePreviewUpdate()
{
    if (m_previewDebounceTimer) {
        m_previewDebounceTimer->start();
    }
}

namespace {
/*!
 * \brief 滚动区「仍保留 QML 预览」的候选卡片排序键：避免纯按相交面积排序导致边缘卡片在滚动时
 * 与邻居反复争抢有限名额，从而 destroy/create 引发占位符与 QML 切换闪烁。
 */
struct PreviewRankEntry {
    WidgetCard *card = nullptr;
    int strictArea = 0;   //!< 与真实视口的相交面积；0 表示仅落在 hysteresis 带内
    int expandedArea = 0; //!< 与扩展视口的相交面积（用于 hysteresis 带内优先级）
    int yTopInViewport = 0; //!< 稳定 tie-break：视口坐标系内卡片顶边
};
} // namespace

/*!
 * \brief 在「真实视口 + 垂直 hysteresis」范围内保留预览，并对超出并发上限的卡片按稳定规则排序后销毁预览。
 *
 * 限制并发 QML 预览数量，减轻 CPU/GPU 压力；扩展带与「严格视口优先」排序减轻滚动时反复卸载/加载造成的闪烁。
 */
void WidgetManagerDialog::updateVisiblePreviewsNow()
{
    if (!m_scrollArea || !m_scrollArea->viewport()) {
        return;
    }
    const QRect strictViewport = m_scrollArea->viewport()->rect();
    // 视口上下各扩展一段：刚滚出/即将滚入的卡片暂不销毁预览，避免与真实视口临界时频繁 setPreviewActive。
    constexpr int kViewportHysteresisPx = 480;
    const QRect expandedViewport = strictViewport.adjusted(0, -kViewportHysteresisPx, 0, kViewportHysteresisPx);
    // 3 列网格 + 部分行被裁切时，同一视口内可能有略多于 9 张卡片与视口相交；原先固定 9 且纯按面积排序
    // 易在滚动时让边缘卡片与邻居交替落选，触发 QML 反复销毁/创建。提高上限并与 hysteresis 配合。
    constexpr int kMaxLivePreviews = 18;
    QVector<PreviewRankEntry> ranked;
    ranked.reserve(m_cards.size());

    for (auto card : qAsConst(m_cards)) {
        if (card->isHidden()) {
            // 与 applyFilters 一致：仅因筛选隐藏时保留已加载预览，切换分类不反复销毁/创建。
            continue;
        }
        const QPoint cardTopLeftInViewport = m_scrollArea->viewport()->mapFromGlobal(card->mapToGlobal(QPoint(0, 0)));
        const QRect cardRectInViewport(cardTopLeftInViewport, card->size());
        const QRect expandedIntersection = expandedViewport.intersected(cardRectInViewport);
        if (expandedIntersection.isEmpty()) {
            card->setPreviewActive(false);
            continue;
        }
        const QRect strictIntersection = strictViewport.intersected(cardRectInViewport);
        PreviewRankEntry entry;
        entry.card = card;
        entry.strictArea = qMax(0, strictIntersection.width() * strictIntersection.height());
        entry.expandedArea = qMax(0, expandedIntersection.width() * expandedIntersection.height());
        entry.yTopInViewport = cardRectInViewport.top();
        ranked.append(entry);
    }

    std::sort(ranked.begin(), ranked.end(), [](const PreviewRankEntry &a, const PreviewRankEntry &b) {
        const bool aStrict = a.strictArea > 0;
        const bool bStrict = b.strictArea > 0;
        if (aStrict != bStrict) {
            return aStrict; // 凡与真实视口相交的卡片优先于仅在 hysteresis 带内的卡片
        }
        if (a.strictArea != b.strictArea) {
            return a.strictArea > b.strictArea;
        }
        if (a.expandedArea != b.expandedArea) {
            return a.expandedArea > b.expandedArea;
        }
        return a.yTopInViewport < b.yTopInViewport;
    });

    for (int i = kMaxLivePreviews; i < ranked.size(); ++i) {
        ranked[i].card->setPreviewActive(false);
    }

    const int nActivate = qMin(kMaxLivePreviews, ranked.size());
    for (int i = 0; i < nActivate; ++i) {
        ranked[i].card->setPreviewActive(true);
    }
    repairRightScrollAreaBackground();
}

void WidgetManagerDialog::repairRightScrollAreaBackground()
{
    const QPalette appPal = QApplication::palette();
    const QColor windowFill = appPal.color(QPalette::Window);
    const QColor baseFill = appPal.color(QPalette::Base);

    if (m_scrollArea) {
        m_scrollArea->setAutoFillBackground(true);
        QPalette sp = m_scrollArea->palette();
        sp.setColor(QPalette::Window, windowFill);
        sp.setColor(QPalette::Base, baseFill);
        m_scrollArea->setPalette(sp);
        if (m_scrollArea->viewport()) {
            m_scrollArea->viewport()->setAutoFillBackground(true);
            QPalette vp = sp;
            m_scrollArea->viewport()->setPalette(vp);
            m_scrollArea->viewport()->update();
        }
        m_scrollArea->update();
    }
    if (m_cardContainer) {
        m_cardContainer->setAutoFillBackground(true);
        QPalette cp = m_cardContainer->palette();
        cp.setColor(QPalette::Window, windowFill);
        cp.setColor(QPalette::Base, baseFill);
        m_cardContainer->setPalette(cp);
        m_cardContainer->update();
    }
}

void WidgetManagerDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    // 与 libpeony-qt properties-window、主窗口一致：X11 用 Motif 去标题栏；Wayland 用 UkuiWindowHelper::removeTitleBar。
    if (QX11Info::isPlatformX11()) {
        if (winId()) {
            MotifWmHints hints;
            hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
            hints.functions = MWM_FUNC_ALL & ~MWM_FUNC_MOVE;
            hints.decorations = MWM_DECOR_BORDER;
            XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
        }
        if (winId()) {
            KWindowEffects::enableBlurBehind(winId(), true);
        }
    } else {
#if defined(KY_SDK_WAYLANDHELPER)
        if (!m_wmHelper) {
            m_wmHelper = new UkuiWindowHelper(this);
            m_wmHelper->setWindowRole(UkuiWindowHelper::WindowRole::Dialog);
            m_wmHelper->removeTitleBar();
        }
        {
            const UkuiWindowHelper::States all = UkuiWindowHelper::States(0xff);
            m_wmHelper->setWindowState(all & ~UkuiWindowHelper::State::Movable);
        }
        m_wmHelper->setBlurEffect(QRegion());
#else
        if (winId()) {
            KWindowEffects::enableBlurBehind(winId(), true);
        }
#endif
    }
    repositionToBottomCenter();
    repairRightScrollAreaBackground();
    QTimer::singleShot(0, this, [this]() { updateVisiblePreviewsNow(); });
}

void WidgetManagerDialog::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    if (m_inProgrammaticMove || !isVisible()) {
        return;
    }
    repositionToBottomCenter();
}

bool WidgetManagerDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (m_scrollArea && watched == m_scrollArea->viewport()) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
            schedulePreviewUpdate();
        }
    }
    return QDialog::eventFilter(watched, event);
}
