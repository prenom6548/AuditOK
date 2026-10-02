/*
 * Peony-Qt
 *
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "widget-manager-dialog.h"
#include "desktop-window-manager.h"
#include "platformwindow.h"
#include "global-settings.h"
#include "screen-manager.h"

#include <widget-loader.h>
#include <widget.h>
#include <widget-item.h>
#include <config.h>
#include <shared-engine-view.h>

#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <ksearchlineedit.h>
#include <QPushButton>
#include <QListWidget>
#include <QIcon>
#include <QApplication>
#include <QDrag>
#include <QFont>
#include <QDebug>
#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QVariantList>
#include <QLocale>
#include <QQmlContext>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QSignalBlocker>
#include <QMimeData>
#include <QEvent>
#include <QTimer>
#include <QShowEvent>
#include <QPalette>
#include <QColor>
#include <QDataStream>
#include <QPainter>
#include <QRegion>
#include <QQuickItem>
#include <QX11Info>
#include <QGuiApplication>
#include <QScreen>
#include <QCursor>
#include <QMoveEvent>
#include <QWindow>
#include "xatom-helper.h"

#include <KWindowEffects>

#if defined(KY_SDK_WAYLANDHELPER)
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>
#endif
/*!
 * \file widget-manager-dialog.cpp
 * \brief 桌面小插件管理对话框与卡片实现：ukui-quick 预览加载、筛选与桌面拖放。
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

/*! 桌面小插件管理器与屏幕可用区域底部的间距 */
static constexpr int kWidgetManagerBottomMargin = 12;

template<typename T>
static QByteArray serializeDragValue(const T &value)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << value;
    return data;
}

class WidgetManagerListItemDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const QWidget *widget = opt.widget;
        const QStyle *style = widget ? widget->style() : QApplication::style();
        if (!opt.state.testFlag(QStyle::State_Selected)) {
            QStyleOptionViewItem contentOpt = opt;
            contentOpt.state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
            style->drawControl(QStyle::CE_ItemViewItem, &contentOpt, painter, widget);
            return;
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(opt.palette.color(QPalette::Highlight));
        painter->drawRoundedRect(opt.rect.adjusted(0, 1, 0, -1), 4, 4);
        painter->restore();

        QStyleOptionViewItem contentOpt = opt;
        contentOpt.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver | QStyle::State_HasFocus);
        const QColor highlightedText = opt.palette.color(QPalette::HighlightedText);
        contentOpt.palette.setColor(QPalette::Text, highlightedText);
        contentOpt.palette.setColor(QPalette::WindowText, highlightedText);
        contentOpt.palette.setColor(QPalette::ButtonText, highlightedText);
        contentOpt.palette.setColor(QPalette::HighlightedText, highlightedText);
        style->drawControl(QStyle::CE_ItemViewItem, &contentOpt, painter, widget);
    }
};

static QVariantList sizeValues(const QVariant &value)
{
    if (value.canConvert<QVariantList>()) {
        const QVariantList list = value.toList();
        if (!list.isEmpty()) {
            return list;
        }
    }
    return value.toJsonArray().toVariantList();
}

/*! 管理器对桌面 WidgetItem 做整体缩放：96 / 136 = 12 / 17。 */
static constexpr qreal kManagerPreviewScale = 12.0 / 17.0;

static QVariantMap standardManagerVariant(const QString &sizeId)
{
    QVariantMap variant;
    variant.insert(QStringLiteral("id"), sizeId);
    variant.insert(QStringLiteral("fixedResizable"), false);
    if (sizeId == QLatin1String("small")) {
        variant.insert(QStringLiteral("sizeHint"), QVariantList{2, 2});
        variant.insert(QStringLiteral("desktopSize"), QVariantList{136, 136});
    } else if (sizeId == QLatin1String("medium")) {
        variant.insert(QStringLiteral("sizeHint"), QVariantList{4, 2});
        variant.insert(QStringLiteral("desktopSize"), QVariantList{296, 136});
    } else if (sizeId == QLatin1String("large")) {
        variant.insert(QStringLiteral("sizeHint"), QVariantList{4, 4});
        variant.insert(QStringLiteral("desktopSize"), QVariantList{296, 296});
    } else {
        return QVariantMap();
    }
    return variant;
}

static QVariantMap managerVariantFromConfig(const QVariant &value)
{
    const QVariantMap config = value.toMap();
    const QString sizeId = config.isEmpty()
        ? value.toString().toLower()
        : config.value(QStringLiteral("id")).toString().toLower();
    QVariantMap variant = standardManagerVariant(sizeId);
    if (variant.isEmpty()) {
        return variant;
    }

    // 名称统一取插件顶层 Name；这里只复制各尺寸的默认及本地化描述。
    for (auto it = config.cbegin(); it != config.cend(); ++it) {
        if (it.key() == QLatin1String("description")
            || it.key().startsWith(QLatin1String("description["))) {
            variant.insert(it.key(), it.value());
        }
    }
    return variant;
}

static QString managerVariantDescription(const QVariantMap &variant, const QString &fallback)
{
    const QString localeKey = QStringLiteral("description[%1]").arg(QLocale::system().name());
    const QString localized = variant.value(localeKey).toString();
    if (!localized.isEmpty()) {
        return localized;
    }
    const QString description = variant.value(QStringLiteral("description")).toString();
    return description.isEmpty() ? fallback : description;
}

static QList<QVariantMap> managerVariantsFromMetadata(const UkuiQuick::WidgetMetadata &metadata)
{
    QList<QVariantMap> variants;
    const QVariantMap contents = metadata.contents();

    const QVariantList managerSizes = sizeValues(contents.value(QStringLiteral("ManagerSizes")));
    for (const QVariant &value : managerSizes) {
        const QVariantMap variant = managerVariantFromConfig(value);
        if (!variant.isEmpty()) {
            variants.append(variant);
        } else {
            qWarning() << "Widget manager: unsupported ManagerSizes value" << value
                       << "for" << metadata.id();
        }
    }
    if (!variants.isEmpty()) {
        return variants;
    }

    // 兼容尚未迁移的插件：只读取旧 ManagerVariants 的 id，忽略 previewMain/previewSize。
    const QVariantList legacyVariants = sizeValues(contents.value(QStringLiteral("ManagerVariants")));
    for (const QVariant &value : legacyVariants) {
        const QVariantMap variant = managerVariantFromConfig(value);
        if (!variant.isEmpty()) {
            variants.append(variant);
        }
    }
    if (variants.isEmpty()) {
        variants.append(QVariantMap());
    }
    return variants;
}

static QSize previewPixelSize(const QSize &desktopSize)
{
    return QSize(qMax(1, qRound(desktopSize.width() * kManagerPreviewScale)),
                 qMax(1, qRound(desktopSize.height() * kManagerPreviewScale)));
}

static QSize desktopPixelSizeFromMetadata(const UkuiQuick::WidgetMetadata &metadata,
                                          const QVariantMap &variant)
{
    const QVariantList desktopSize = sizeValues(variant.value(QStringLiteral("desktopSize")));
    if (desktopSize.size() == 2) {
        return QSize(qMax(1, desktopSize.at(0).toInt()), qMax(1, desktopSize.at(1).toInt()));
    }

    QVariantList sizeHint = sizeValues(variant.value(QStringLiteral("sizeHint")));
    const QVariantMap contents = metadata.contents();
    if (sizeHint.size() != 2) {
        sizeHint = sizeValues(contents.value(QStringLiteral("SizeHint")));
    }

    const bool layoutWithGrid = contents.value(QStringLiteral("SizePolicy")).toInt() != 0;
    const int gridUnitSize = qMax(1, contents.value(QStringLiteral("GridUnitSize"), 60).toInt());
    if (sizeHint.size() == 2) {
        const int widthHint = qMax(1, sizeHint.at(0).toInt());
        const int heightHint = qMax(1, sizeHint.at(1).toInt());
        if (!layoutWithGrid) {
            return QSize(widthHint, heightHint);
        }
        const int width = (gridUnitSize + WIDGET_SPACING) * widthHint
            - WIDGET_SPACING - WIDGET_MARGIN * 2;
        const int height = (gridUnitSize + WIDGET_SPACING) * heightHint
            - WIDGET_SPACING - WIDGET_MARGIN * 2;
        return QSize(qMax(1, width), qMax(1, height));
    }

    return layoutWithGrid ? QSize(gridUnitSize, gridUnitSize) : QSize(200, 150);
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

WidgetManagerDialog::~WidgetManagerDialog()
{
    clearPreviewWidgets();
}

UkuiQuick::WidgetItem *WidgetManagerDialog::loadPreviewWidget(const QString &widgetId,
                                                              const QVariantMap &variant,
                                                              const QSize &desktopSize)
{
    if (!m_previewView) {
        return nullptr;
    }

    UkuiQuick::Widget *widget = UkuiQuick::WidgetLoader::globalLoader().loadWidget(widgetId);
    if (!widget) {
        qWarning() << "Widget manager: failed to load widget" << widgetId;
        return nullptr;
    }
    widget->setParent(this);

    QVariantMap configData;
    configData.insert(QStringLiteral("id"), widgetId);
    configData.insert(QStringLiteral("instanceId"), -1);
    configData.insert(QStringLiteral("width"), desktopSize.width());
    configData.insert(QStringLiteral("height"), desktopSize.height());
    if (variant.contains(QStringLiteral("id"))) {
        configData.insert(QStringLiteral("variantId"), variant.value(QStringLiteral("id")));
    }
    if (variant.contains(QStringLiteral("sizeHint"))) {
        configData.insert(QStringLiteral("SizeHint"), variant.value(QStringLiteral("sizeHint")));
    }
    if (variant.contains(QStringLiteral("fixedResizable"))) {
        configData.insert(QStringLiteral("Resizable"), variant.value(QStringLiteral("fixedResizable")));
    }
    widget->setConfig(new UkuiQuick::Config(configData, widget));
    widget->setInstanceId(-1);

    auto *previewContext = new QQmlContext(m_previewView->rootContext(), widget);
    previewContext->setContextProperty(QStringLiteral("widgetPreviewMode"), true);

    auto *item = qobject_cast<UkuiQuick::WidgetItem *>(
        UkuiQuick::WidgetQuickItem::loadWidgetItem(widget, previewContext));
    if (!item) {
        qWarning() << "Widget manager: failed to create WidgetItem for" << widgetId
                   << widget->uiError();
        delete widget;
        return nullptr;
    }

    item->setWidth(desktopSize.width());
    item->setHeight(desktopSize.height());
    m_previewWidgets.append(widget);
    return item;
}

void WidgetManagerDialog::clearPreviewWidgets()
{
    if (m_previewView && m_previewView->rootObject()) {
        m_previewView->rootObject()->setProperty("cards", QVariantList());
    }
    m_previewEntries.clear();
    for (UkuiQuick::Widget *widget : qAsConst(m_previewWidgets)) {
        Q_EMIT widget->aboutToDeleted();
        widget->deleteLater();
    }
    m_previewWidgets.clear();
}

QVariantMap WidgetManagerDialog::previewEntryAt(int entryIndex) const
{
    if (entryIndex < 0 || entryIndex >= m_previewEntries.size()) {
        qWarning() << "Widget manager: invalid preview entry index" << entryIndex;
        return {};
    }
    return m_previewEntries.at(entryIndex).toMap();
}

void WidgetManagerDialog::addWidgetFromPreview(int entryIndex)
{
    const QVariantMap entry = previewEntryAt(entryIndex);
    const QString widgetId = entry.value(QStringLiteral("widgetId")).toString();
    if (widgetId.isEmpty()) {
        return;
    }
    PlatformWindow *window = getTargetWindow();
    if (!window) {
        qWarning() << "WidgetManagerDialog: no platform window available";
        return;
    }
    window->addWidgetFromVariant(widgetId, entry.value(QStringLiteral("variant")).toMap());
}

void WidgetManagerDialog::startPreviewDrag(int entryIndex, QQuickItem *dragSource)
{
    const QVariantMap entry = previewEntryAt(entryIndex);
    const QString widgetId = entry.value(QStringLiteral("widgetId")).toString();
    if (widgetId.isEmpty()) {
        qWarning() << "[WidgetManagerDrag] refusing preview drag without widgetId";
        return;
    }

    QVariantMap dragPayload = entry.value(QStringLiteral("variant")).toMap();
    dragPayload.insert(QStringLiteral("widgetId"), widgetId);
    const QByteArray payload = QJsonDocument::fromVariant(dragPayload).toJson(QJsonDocument::Compact);
    auto *mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("application/x-peony-widget"), payload);

    // QDrag 的平台实现可能在 exec() 返回后仍处理清理事件，按 Qt 契约在堆上创建。
    auto *drag = new QDrag(dragSource ? static_cast<QObject *>(dragSource)
                                      : static_cast<QObject *>(this));
    drag->setMimeData(mimeData);

    const int desktopWidth = entry.value(QStringLiteral("desktopWidth")).toInt();
    const int desktopHeight = entry.value(QStringLiteral("desktopHeight")).toInt();
    const QPoint hotSpot(desktopWidth / 2, desktopHeight / 2);
    auto *previewItem = qobject_cast<QQuickItem *>(
        entry.value(QStringLiteral("widgetItem")).value<QObject *>());
    QQuickItem *originalParentItem = nullptr;
    QPointF originalPosition;
    QSizeF originalSize;
    qreal originalScale = 1.0;
    bool originalVisible = true;
    UkuiQuick::SharedEngineView *liveDragView = nullptr;

    if (previewItem && desktopWidth > 0 && desktopHeight > 0) {
        // 复用正在运行的 WidgetItem：拖拽期间仍由 QML 场景逐帧渲染，时钟等动态内容不会冻结。
        originalParentItem = previewItem->parentItem();
        originalPosition = previewItem->position();
        originalSize = previewItem->size();
        originalScale = previewItem->scale();
        originalVisible = previewItem->isVisible();

        liveDragView = new UkuiQuick::SharedEngineView;
        liveDragView->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool
                               | Qt::WindowDoesNotAcceptFocus);
        liveDragView->setColor(Qt::transparent);
        liveDragView->resize(desktopWidth, desktopHeight);
        if (QScreen *cursorScreen = QGuiApplication::screenAt(QCursor::pos())) {
            liveDragView->setScreen(cursorScreen);
        }
        liveDragView->setPosition(QCursor::pos() - hotSpot);

        previewItem->setParentItem(nullptr);
        previewItem->setPosition(QPointF(0, 0));
        previewItem->setSize(QSizeF(desktopWidth, desktopHeight));
        previewItem->setScale(1.0);
        previewItem->setVisible(true);
        previewItem->setParentItem(liveDragView->contentItem());

        // 与桌面插件跨屏拖拽使用相同的窗口 MIME，让平台拖拽实现移动实时预览窗口。
        mimeData->setData(QStringLiteral("application/x-qt-mainwindowdrag-window"),
                          serializeDragValue(reinterpret_cast<qintptr>(liveDragView)));
        mimeData->setData(QStringLiteral("application/x-qt-mainwindowdrag-position"),
                          serializeDragValue(hotSpot));
        liveDragView->show();
    }

    beginPreviewDrag(entry, QString::fromUtf8(payload));
    if (dragSource) {
        dragSource->ungrabMouse();
    }
    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);

    if (liveDragView) {
        liveDragView->hide();
        previewItem->setParentItem(nullptr);
        previewItem->setPosition(originalPosition);
        previewItem->setSize(originalSize);
        previewItem->setScale(originalScale);
        previewItem->setVisible(originalVisible);
        previewItem->setParentItem(originalParentItem);
        liveDragView->deleteLater();
    }
    endPreviewDrag();
    drag->deleteLater();
}

void WidgetManagerDialog::beginPreviewDrag(const QVariantMap &entry, const QString &payload)
{
    const int width = entry.value(QStringLiteral("desktopWidth")).toInt();
    const int height = entry.value(QStringLiteral("desktopHeight")).toInt();
    DesktopWindowManager::getInstance()->setDesktopWidgetCrossScreenDragInfo(
        width, height, width / 2, height / 2,
        entry.value(QStringLiteral("layoutWithGrid")).toBool(),
        entry.value(QStringLiteral("gridUnitSize"), 60).toInt(), payload);
    schedulePaletteHideWhenDragActive();
}

void WidgetManagerDialog::endPreviewDrag()
{
    DesktopWindowManager::getInstance()->clearDesktopWidgetCrossScreenDragInfo();
    finalizePaletteDragAfterExec();
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
    const int y = ag.bottom() - h + 1 - kWidgetManagerBottomMargin;
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

    // 左侧：使用与 Peony 侧边栏一致的半透明 Base 色，保留窗口毛玻璃效果但避免完全透底。
    m_leftPane = new QWidget(body);
    m_leftPane->setFixedWidth(240);
    m_leftPane->setAttribute(Qt::WA_TranslucentBackground, false);
    m_leftPane->setAutoFillBackground(true);
    QVBoxLayout *leftLayout = new QVBoxLayout(m_leftPane);
    leftLayout->setContentsMargins(12, 12, 12, 12);
    leftLayout->setSpacing(10);

    m_searchEdit = new kdk::KSearchLineEdit(m_leftPane);
    m_searchEdit->setPlaceholderText(tr("Search application cards"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(32);

    m_pluginList = new QListWidget(m_leftPane);
    m_pluginList->setIconSize(QSize(22, 22));
    m_pluginList->setSpacing(2);
    m_pluginList->setFrameShape(QFrame::NoFrame);
    m_pluginList->setAutoFillBackground(false);
    m_pluginList->viewport()->setAutoFillBackground(false);
    m_pluginList->setItemDelegate(new WidgetManagerListItemDelegate(m_pluginList));

    leftLayout->addWidget(m_searchEdit);
    leftLayout->addWidget(m_pluginList, 1);

    // 右侧：放入不透明窗口容器；底部说明与完成按钮同一水平布局
    QWidget *rightWindow = new QWidget(body);
    rightWindow->setFixedWidth(698);
    rightWindow->setAutoFillBackground(true);
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

    // 与桌面 IslandView 一致：右侧只有一个 QQuickWindow，所有预览都是该场景中的 QQuickItem。
    // 滚动、裁切和筛选在 QML 内完成，避免在 QScrollArea 中嵌入多个原生窗口。
    m_previewView = new UkuiQuick::SharedEngineView;
    m_previewView->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    m_previewView->setResizeMode(UkuiQuick::SharedEngineView::SizeRootObjectToView);
    m_previewView->rootContext()->setContextProperty(QStringLiteral("widgetManagerDialog"), this);
    m_previewView->setSource(QUrl(QStringLiteral("qrc:/qml-desktop/WidgetManagerPreviewPane.qml")));
    QWidget *previewContainer = QWidget::createWindowContainer(m_previewView, rightWindow);
    previewContainer->setFocusPolicy(Qt::NoFocus);
    rightLayout->addWidget(previewContainer, 1);

    QFrame *footerSeparator = new QFrame(rightWindow);
    footerSeparator->setFrameShape(QFrame::HLine);
    rightLayout->addWidget(footerSeparator);

    QHBoxLayout *rightBottomRow = new QHBoxLayout;
    rightBottomRow->setContentsMargins(0, 6, 0, 0);
    QLabel *hintLabel = new QLabel(tr("Click a card to add it to the desktop, or drag a card onto the desktop."), rightWindow);
    rightBottomRow->addWidget(hintLabel, 1, Qt::AlignLeft | Qt::AlignVCenter);
    QPushButton *closeBtn = new QPushButton(tr("Done"), rightWindow);
    closeBtn->setFixedSize(84, 32);
    closeBtn->setAutoDefault(false);
    closeBtn->setDefault(false);
    closeBtn->setProperty("isImportant", true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    rightBottomRow->addWidget(closeBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    rightLayout->addLayout(rightBottomRow);

    bodyLayout->addWidget(m_leftPane);
    bodyLayout->addWidget(rightWindow, 1);
    mainLayout->addWidget(body, 1);

    connect(m_searchEdit, &kdk::KSearchLineEdit::textChanged,
            this, &WidgetManagerDialog::onSearchTextChanged);
    connect(m_pluginList, &QListWidget::itemSelectionChanged, this, &WidgetManagerDialog::onPluginChanged);
    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, this, [this](const QString &key) {
        if (key == QLatin1String(SIDEBAR_BG_OPACITY)) {
            updateLeftPaneBackground();
        }
    });
    connect(qApp, &QApplication::fontChanged, this, [this](const QFont &) {
        updatePreviewPaneFont();
    });

    updatePreviewPanePalette();
    updatePreviewPaneFont();
}

/*!
 * \brief 枚举 WidgetLoader 中 Desktop 插件，按 ShowInManager 过滤，并生成右侧 QML 预览模型。
 *
 * 左侧列表填充「全部」与各插件名；无变体时插入一条空 variant。最后调度一次可见预览更新。
 */
void WidgetManagerDialog::loadWidgets()
{
    clearPreviewWidgets();
    // 通过 ukui-quick WidgetLoader 枚举所有已安装的 Desktop 插件
    const QList<UkuiQuick::WidgetMetadata> allMetadata =
        UkuiQuick::WidgetLoader::globalLoader().widgetsMetadata();
    m_pluginList->clear();
    {
        QListWidgetItem *allItem = new QListWidgetItem(tr("All"), m_pluginList);
        allItem->setIcon(QIcon::fromTheme(QStringLiteral("kylin-app-desk-assistant")));
        allItem->setData(Qt::UserRole + 1, QStringLiteral("all"));
        if (allItem->icon().isNull()) {
            allItem->setIcon(QIcon::fromTheme(QStringLiteral("applications-other")));
        }
    }
    QMap<QString, QString> pluginIdToName;
    QMap<QString, QIcon> pluginIdToIcon;

    for (const UkuiQuick::WidgetMetadata &meta : allMetadata) {
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
        const QList<QVariantMap> variants = managerVariantsFromMetadata(meta);

        int rowSlotH = 0;
        for (const QVariantMap &v : qAsConst(variants)) {
            rowSlotH = qMax(rowSlotH,
                            previewPixelSize(desktopPixelSizeFromMetadata(meta, v)).height());
        }

        for (const QVariantMap &variant : qAsConst(variants)) {
            const QString description = managerVariantDescription(variant, meta.description());
            const QSize desktopSize = desktopPixelSizeFromMetadata(meta, variant);
            const QSize previewSize = previewPixelSize(desktopSize);
            UkuiQuick::WidgetItem *previewWidget = loadPreviewWidget(meta.id(), variant, desktopSize);

            QVariantMap entry;
            entry.insert(QStringLiteral("widgetId"), meta.id());
            entry.insert(QStringLiteral("variant"), variant);
            entry.insert(QStringLiteral("displayName"), pluginName);
            entry.insert(QStringLiteral("description"), description);
            entry.insert(QStringLiteral("widgetItem"),
                         QVariant::fromValue(static_cast<QObject *>(previewWidget)));
            entry.insert(QStringLiteral("previewWidth"), previewSize.width());
            entry.insert(QStringLiteral("previewHeight"), previewSize.height());
            entry.insert(QStringLiteral("previewScale"), kManagerPreviewScale);
            entry.insert(QStringLiteral("previewSlotHeight"), rowSlotH);
            entry.insert(QStringLiteral("desktopWidth"), desktopSize.width());
            entry.insert(QStringLiteral("desktopHeight"), desktopSize.height());
            entry.insert(QStringLiteral("layoutWithGrid"),
                         meta.contents().value(QStringLiteral("SizePolicy")).toInt() != 0);
            entry.insert(QStringLiteral("gridUnitSize"),
                         qMax(1, meta.contents().value(QStringLiteral("GridUnitSize"), 60).toInt()));
            m_previewEntries.append(entry);
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
    if (m_previewView && m_previewView->rootObject()) {
        m_previewView->rootObject()->setProperty("cards", m_previewEntries);
    }
    applyFilters();
}

PlatformWindow *WidgetManagerDialog::getTargetWindow() const
{
    // 与“添加分区”一致，按点击位置选择对应屏幕的桌面窗口。
    // ScreenManager 在镜像模式及屏幕热插拔窗口尚未就绪时会自动回退到主屏。
    if (auto window = qobject_cast<PlatformWindow *>(SCREEN_MANAGER->getWindowAt(QCursor::pos()))) {
        return window;
    }

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

void WidgetManagerDialog::onPluginChanged()
{
    updateRightPaneTitle();
    applyFilters();
}

void WidgetManagerDialog::onSearchTextChanged(const QString &text)
{
    if (!text.trimmed().isEmpty() && m_pluginList && m_pluginList->currentRow() != 0) {
        const QSignalBlocker blocker(m_pluginList);
        m_pluginList->setCurrentRow(0);
        updateRightPaneTitle();
    }
    applyFilters();
}

void WidgetManagerDialog::updateRightPaneTitle()
{
    if (!m_rightTitleLabel) {
        return;
    }
    if (!m_pluginList || !m_pluginList->currentItem() || m_pluginList->currentRow() <= 0) {
        m_rightTitleLabel->setText(tr("All"));
        return;
    }
    m_rightTitleLabel->setText(m_pluginList->currentItem()->text());
}

/*!
 * \brief 将插件名称搜索词与左侧插件 Id 写入单一 QML 场景，由 Flow 重新排列匹配卡片。
 */
void WidgetManagerDialog::applyFilters()
{
    const QString keyword = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : QString();
    const bool searchActive = !keyword.isEmpty();

    if (m_pluginList) {
        // 搜索期间只保留名称匹配的插件；“全部”始终保留。
        if (m_pluginList->count() > 0) {
            m_pluginList->item(0)->setHidden(false);
        }
        for (int row = 1; row < m_pluginList->count(); ++row) {
            QListWidgetItem *item = m_pluginList->item(row);
            item->setHidden(searchActive
                            && !item->text().toLower().contains(keyword));
        }
    }

    QString selectedPluginId;
    if (m_pluginList && m_pluginList->currentItem() && m_pluginList->currentRow() >= 0) {
        const QListWidgetItem *current = m_pluginList->currentItem();
        const QString mode = current->data(Qt::UserRole + 1).toString();
        if (mode == QLatin1String("plugin")) {
            selectedPluginId = current->data(Qt::UserRole).toString();
        }
    }
    if (m_previewView && m_previewView->rootObject()) {
        m_previewView->rootObject()->setProperty("keyword", keyword);
        m_previewView->rootObject()->setProperty("pluginId", selectedPluginId);
    }
}

void WidgetManagerDialog::updateLeftPaneBackground()
{
    if (!m_leftPane) {
        return;
    }

    const bool isActive = isActiveWindow();
    QColor fillColor = qApp->palette().color(isActive ? QPalette::Active : QPalette::Inactive,
                                             QPalette::Window);
    if (!qApp->property("kcontain-secondary-normal").isNull()) {
        fillColor = qApp->property(isActive ? "kcontain-secondary-normal"
                                            : "kcontain-secondary-inactive").value<QColor>();
    }

    const int opacity = qBound(0,
                               Peony::GlobalSettings::getInstance()->getValue(SIDEBAR_BG_OPACITY).toInt(),
                               100);
    fillColor.setAlphaF(opacity / 100.0);

    QPalette palette = m_leftPane->palette();
    palette.setColor(QPalette::Window, fillColor);
    palette.setColor(QPalette::Base, fillColor);
    m_leftPane->setPalette(palette);
    m_leftPane->setAutoFillBackground(true);
    m_leftPane->update();

    if (m_pluginList) {
        m_pluginList->setAutoFillBackground(false);
        if (m_pluginList->viewport()) {
            m_pluginList->viewport()->setAutoFillBackground(false);
            m_pluginList->viewport()->update();
        }
    }
}

void WidgetManagerDialog::updatePreviewPanePalette()
{
    const QPalette appPal = QApplication::palette();
    const QColor windowFill = appPal.color(QPalette::Window);
    if (m_previewView) {
        m_previewView->setColor(windowFill);
        if (m_previewView->rootObject()) {
            m_previewView->rootObject()->setProperty("paneColor", windowFill);
            m_previewView->rootObject()->setProperty("textColor", appPal.color(QPalette::Text));
            m_previewView->rootObject()->setProperty("secondaryTextColor",
                                                     appPal.color(QPalette::Disabled, QPalette::Text));
        }
        m_previewView->requestUpdate();
    }
}

void WidgetManagerDialog::updatePreviewPaneFont()
{
    if (!m_previewView || !m_previewView->rootObject()) {
        return;
    }

    const QFont appFont = QApplication::font();
    qreal pointSize = appFont.pointSizeF();
    if (pointSize <= 0 && appFont.pixelSize() > 0) {
        const qreal logicalDpi = m_previewView->screen()
            ? m_previewView->screen()->logicalDotsPerInch() : 96.0;
        pointSize = appFont.pixelSize() * 72.0 / logicalDpi;
    }
    if (pointSize <= 0) {
        pointSize = 10;
    }
    m_previewView->rootObject()->setProperty("fontFamily", appFont.family());
    m_previewView->rootObject()->setProperty("fontPointSize", pointSize);
    m_previewView->requestUpdate();
}

void WidgetManagerDialog::done(int result)
{
    m_searchEdit->clear();
    m_searchEdit->clearFocus();
    QDialog::done(result);
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
    updateLeftPaneBackground();
    updatePreviewPanePalette();
    updatePreviewPaneFont();
}

void WidgetManagerDialog::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    if (m_inProgrammaticMove || !isVisible()) {
        return;
    }
    repositionToBottomCenter();
}

void WidgetManagerDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::ActivationChange || event->type() == QEvent::PaletteChange) {
        updateLeftPaneBackground();
        if (event->type() == QEvent::PaletteChange) {
            updatePreviewPanePalette();
        }
    } else if (event->type() == QEvent::FontChange || event->type() == QEvent::ApplicationFontChange) {
        updatePreviewPaneFont();
    }
}
