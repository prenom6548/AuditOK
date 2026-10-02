#include "widgets-proxy-model.h"
#include "file-action-controler.h"
#include <widget.h>
#include <QJsonArray>
#include <QJsonValue>
#include "desktop-window-manager.h"

int itemLayer(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget()) {
        return 0;
    }
    auto widget = item->widget();
    auto metadata = widget->metadata();
    int layer = metadata.contents().value("Layer").toInt();
    return layer;
}

QPoint itemPosition(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        return {};
    }
    auto widget = item->widget();
    auto config = widget->config();
    if (config->contains("x") && config->contains("y")) {
        return QPoint(config->getValue("x").toInt(), config->getValue("y").toInt());
    }
    // 未配置位置的item在界面加载时需要进行位置初始化
    return QPoint();
}

QSize itemSizeHint(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget()) {
        return QSize(3, 2);
    }
    auto widget = item->widget();
    if (widget->metadata().contents().contains("SizeHint")) {
        auto array = widget->metadata().contents().value("SizeHint").toJsonArray();
        if (array.count() == 2) {
            return QSize(array[0].toInt(), array[1].toInt());
        }
    }
    return QSize(3, 2);
}

QSize itemSize(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        return QSize(200, 150);
    }
    auto widget = item->widget();
    auto config = widget->config();
    if (config->contains("width") && config->contains("height")) {
        return QSize(config->getValue("width").toInt(), config->getValue("height").toInt());
    }else if (config->contains("SizeHint")||widget->metadata().contents().contains("SizeHint")) {
        auto array = config->contains("SizeHint") ? config->getValue("SizeHint").toJsonArray() :  widget->metadata().contents().value("SizeHint").toJsonArray();
        if (array.count() == 2) {
            bool layoutWidthGrid = WidgetsProxyModel::isItemLayoutWithGrid(item);
            if (layoutWidthGrid) {
                int unitSize = WidgetsProxyModel::itemWidgetGridUnitSize(item);
                int width = (unitSize + WIDGET_SPACING) * array[0].toInt() - WIDGET_SPACING - WIDGET_MARGIN * 2;
                int height = (unitSize + WIDGET_SPACING) * array[1].toInt() - WIDGET_SPACING - WIDGET_MARGIN * 2;
                widget->config()->setValue("initGridWidth", array[0].toInt());
                widget->config()->setValue("initGridHeight", array[1].toInt());
                return QSize(width, height);
            } else return QSize(array[0].toInt(), array[1].toInt());
        }
    }
    auto childRect = item->childrenRect().size();

    bool layoutWidthGrid = WidgetsProxyModel::isItemLayoutWithGrid(item);
    if (!childRect.isNull()) {
        if (layoutWidthGrid) {
            // 计算能够完全放置的最小网格规格，并且以此规格返回对应的size
            int unitSize = WidgetsProxyModel::itemWidgetGridUnitSize(item);
            int minGridWidth = qCeil(childRect.width() + 2 * WIDGET_MARGIN + WIDGET_SPACING/(unitSize + WIDGET_SPACING));
            int minGridHeight = qCeil(childRect.height() + 2 * WIDGET_MARGIN + WIDGET_SPACING /(unitSize + WIDGET_SPACING));
            int width = minGridWidth * (unitSize + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
            int height = minGridHeight * (unitSize + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
            widget->config()->setValue("initGridWidth", minGridWidth);
            widget->config()->setValue("initGridHeight", minGridHeight);
            return QSize(width, height);
        }
        return childRect.toSize();
    } else {
        if (layoutWidthGrid) {
            // 返回2x2网格
            int unitSize = WidgetsProxyModel::itemWidgetGridUnitSize(item);

            return QSize(unitSize, unitSize);
        }
        return QSize(200, 150);
    }
}

bool itemDragable(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget()) {
        return false;
    }
    auto widget = item->widget();
    if (widget->metadata().contents().contains("Dragable")) {
        return widget->metadata().contents().value("Dragable").toBool();
    }
    // 默认可以拖拽改变插件位置
    return true;
}

bool itemResizable(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        return false;
    }
    auto widget = item->widget();
    auto config = widget->config();
    if (config->contains("Resizable")) {
        return config->getValue("Resizable").toBool();
    }
    return widget->metadata().contents().value("Resizable").toBool();
}

bool itemHasDecoration(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget()) {
        return false;
    }
    auto widget = item->widget();
    return widget->metadata().contents().value("DoDecoration").toBool();
}

bool itemHasBlurBehind(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget()) {
        return false;
    }
    auto widget = item->widget();
    return widget->metadata().contents().value("DoBlur").toBool();
}

void setItemPos(UkuiQuick::WidgetItem *item, const QPoint &pos)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        return;
    }
    item->widget()->config()->setValue("x", pos.x());
    item->widget()->config()->setValue("y", pos.y());
}

void setItemSize(UkuiQuick::WidgetItem *item, const QSize &size)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        return;
    }
    item->widget()->config()->setValue("width", size.width());
    item->widget()->config()->setValue("height", size.height());
}

WidgetsProxyModel::WidgetsProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    connect(DesktopOrganizationManager::instance(), &DesktopOrganizationManager::expandStatusChangedRequest, this, [=](int instanceid,bool expand){
        for (int row = 0; row < rowCount(); row++) {
            auto item = qvariant_cast<UkuiQuick::WidgetItem *>(data(index(row, 0), UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
            if (!item || !item->widget()) {
                continue;
            }
            int id = item->widget()->instanceId();
            QModelIndex index = this->index(row, 0);
            if(instanceid == id){
                const QVector<int> roles = {Qt::UserRole + 8};
                Q_EMIT dataChanged(index, index, roles);
            }
        }
    });
}

QHash<int, QByteArray> WidgetsProxyModel::roleNames() const
{
    QHash<int, QByteArray> roles = {};

    if (sourceModel()) {
        roles.insert(sourceModel()->roleNames()); //widgetItem
        roles.insert(Qt::UserRole + 1, "position");
        roles.insert(Qt::UserRole + 2, "size");
        roles.insert(Qt::UserRole + 3, "layer");
        roles.insert(Qt::UserRole + 4, "hasAnchors");
        roles.insert(Qt::UserRole + 5, "sizePolicy");
        roles.insert(Qt::UserRole + 6, "sizeHint");
        roles.insert(Qt::UserRole + 7, "dragable");
        roles.insert(Qt::UserRole + 8, "resizable");
        roles.insert(Qt::UserRole + 9, "doDecoration");
        roles.insert(Qt::UserRole + 10, "doBlur");
        roles.insert(Qt::UserRole + 11, "anchors");
        roles.insert(Qt::UserRole + 12, "instanceId");
        roles.insert(Qt::UserRole + 13, "widgetGridSizeStep"); // sizepolicy为1时需要
    }

    return roles;
}

QVariant WidgetsProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (role == UkuiQuick::WidgetItemModel::Key::WidgetItemRole) {
        return QSortFilterProxyModel::data(index, role);
    }

    UkuiQuick::WidgetItem *item = qvariant_cast<UkuiQuick::WidgetItem *>(QSortFilterProxyModel::data(index, UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
    if (!item || !item->widget()) {
        return {};
    }
    QVariant retVal;
    switch (role) {
    case Qt::UserRole + 1: {
        return itemPosition(item);
    }
    case Qt::UserRole + 2: {
        return itemSize(item);
    }
    case Qt::UserRole + 3: {
        return itemLayer(item);
    }
    case Qt::UserRole + 4: {
        auto widget = item->widget();
        return widget->metadata().contents().contains("Anchors");
        break;
    }
    case Qt::UserRole + 5: {
        return isItemLayoutWithGrid(item)? 1: 0;
    }
    case Qt::UserRole + 6: {
        return itemSizeHint(item);
    }
    case Qt::UserRole + 7: {
        return itemDragable(item);
    }
    case Qt::UserRole + 8: {
        return itemResizable(item);
    }
    case Qt::UserRole + 9: {
        return itemHasDecoration(item);
    }
    case Qt::UserRole + 10: {
        return itemHasBlurBehind(item);
    }
    case Qt::UserRole + 11: {
        auto widget = item->widget();
        auto list = widget->metadata().contents().value("Anchors").toJsonArray().toVariantList();
        if (list.count() == 6) {
            return list;
        }
        return QVariantList{0, 0, 0, 0, 0, 0};
    }
    case Qt::UserRole + 12:{
        return item->widget()->instanceId();
    }
    case Qt::UserRole+13: {
        return itemWidgetGridUnitSize(item);
    }
    default:
        break;
    }

    return QVariant();
}

void WidgetsProxyModel::updateGeometryConfig(int row, UkuiQuick::WidgetItem *item, QQuickItem *control)
{
    if (!item || !control || !item->widget() || !item->widget()->config()) {
        qWarning() << __FUNCTION__ << "invalid widget item or control";
        return;
    }
    // qDebug() << item->width() << item->height() << item->childrenRect();
    // qDebug() << control->width() << control->height();
    // int itemWidth = item->width();
    // int itemHight = item->height();
    // if (isItemLayoutWithGrid(item)) {
    //     qWarning() << "网格布局item请使用updateGridItemLayoutConfig方法";
    //     return;
    // }
    QModelIndex index = this->index(row, 0);
    // 由于item在control中，需要用control获取位置，否则得到的位置是(0, 0)
    item->widget()->config()->setValue("x", int(control->x()));
    item->widget()->config()->setValue("y", int(control->y()));
    item->widget()->config()->setValue("width", int(item->width()));
    item->widget()->config()->setValue("height", int(item->height()));
    const QVector<int> roles = {Qt::UserRole + 1, Qt::UserRole + 2};
    Q_EMIT dataChanged(index, index, roles);
}

void WidgetsProxyModel::updateItemLayoutConfig(int row, UkuiQuick::WidgetItem *item, int x, int y)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        qWarning() << __FUNCTION__ << "invalid widget item";
        return;
    }
    QModelIndex index = this->index(row, 0);
    item->widget()->config()->setValue("x", x);
    item->widget()->config()->setValue("y", y);
    const QVector<int> roles = {Qt::UserRole + 1, Qt::UserRole + 2};
    Q_EMIT dataChanged(index, index, roles);
}

void WidgetsProxyModel::doStackAfter(QQuickItem *srcItem, QQuickItem *dstItem)
{
    if (!srcItem || !dstItem) {
        qWarning() << __FUNCTION__ << "源窗口或目标窗口不存在";
        return;
    }
    srcItem->stackAfter(dstItem);
}

bool WidgetsProxyModel::hasPositionConfig(UkuiQuick::WidgetItem *item)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        return false;
    }
    auto widget = item->widget();
    auto config = widget->config();
    if (config->contains("x") && config->contains("y")) {
        return true;
    }
    return false;
}

void WidgetsProxyModel::initWithConfig(UkuiQuick::WidgetItem *item, QQuickItem *delegate)
{
    if (!item || !item->widget() || !item->widget()->config()) {
        qWarning() << __FUNCTION__ << "invalid widget item";
        return;
    }
    QSize size = itemSize(item);
    item->setWidth(size.width());
    item->setHeight(size.height());
    if (delegate) {
        QPoint pos = itemPosition(item);
        delegate->setX(pos.x());
        delegate->setY(pos.y());
        item->setProperty("lastRect", QRectF(pos, size));
    }

    // if (isItemLayoutWithGrid(item)) {
    //     auto gridSize = FileActionController::getInstance()->widgetGridSize();
    //     item->setWidth(gridSize.width() * size.width());
    //     item->setHeight(gridSize.height() * size.height());
    // } else {
    //     item->setWidth(size.width());
    //     item->setHeight(size.height());
    // }
}

bool WidgetsProxyModel::checkItemsIntersected(QQuickItem *item1, QQuickItem *item2)
{
    if (!item1 || !item2) {
        return false;
    }
    QRectF rect1 = item1->boundingRect();
    QRectF rect2 = item2->boundingRect();
    rect1.moveTo(item1->x(), item1->y());
    rect2.moveTo(item2->x(), item2->y());
    return rect1.intersects(rect2);
}

QList<UkuiQuick::WidgetItem *> WidgetsProxyModel::getAllWidgets()
{
    QList<UkuiQuick::WidgetItem *> widgets;
    for (int row = 0; row < rowCount(); row++) {
        auto widget = qvariant_cast<UkuiQuick::WidgetItem *>(data(index(row, 0), UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
        if (widget) {
            widgets << widget;
        } else {
            qCritical() << "can not convert widget item from model" << row;
        }
    }
    return widgets;
}

void WidgetsProxyModel::updateItemPositionConfig(UkuiQuick::WidgetItem *item, const QPoint &pos)
{
    if (!item) {
        return;
    }
    for (int row = 0; row < rowCount(); row++) {
        auto tmpItem = qvariant_cast<UkuiQuick::WidgetItem *>(data(index(row, 0), UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
        if (item == tmpItem) {
            // 复用现有的更新item x、y配置的接口
            updateItemLayoutConfig(row, item, pos.x(), pos.y());
            break;
        }
    }
}

bool WidgetsProxyModel::isItemLayoutWithGrid(UkuiQuick::WidgetItem *widgetItem)
{
    if (!widgetItem || !widgetItem->widget()) {
        return false;
    }
    auto widget = widgetItem->widget();
    auto metadata = widget->metadata();
    int sizePolicy = metadata.contents().value("SizePolicy").toInt();
    return sizePolicy;
}

int WidgetsProxyModel::itemWidgetGridUnitSize(UkuiQuick::WidgetItem *widgetItem)
{
    if (!WidgetsProxyModel::isItemLayoutWithGrid(widgetItem)) {
        return 0;
    }
    auto widget = widgetItem->widget();
    if (widget->metadata().contents().contains("GridUnitSize")) {
        return widget->metadata().contents().value("GridUnitSize").toInt();
    }
    return 60;
}
