#ifndef WIDGETS_PROXY_MODEL_H
#define WIDGETS_PROXY_MODEL_H

#include <QObject>
#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <widget-container-item.h>

/*!
 * \brief The WidgetsProxyModel class
 * 作为ukui quick container item widget model的代理model，实现额外操作
 * 主要的目的是和桌面现有代码进行交互，实现小插件布局，相关位置配置读取和写入，多屏处理等桌面特性功能
 */
class WidgetsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit WidgetsProxyModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = UkuiQuick::WidgetItemModel::Key::WidgetItemRole) const override;

    Q_INVOKABLE void updateGeometryConfig(int row, UkuiQuick::WidgetItem *item, QQuickItem *control);

    void updateItemLayoutConfig(int row, UkuiQuick::WidgetItem *item, int gridX, int gridY);

    Q_INVOKABLE void doStackAfter(QQuickItem *srcItem, QQuickItem *dstItem);

    Q_INVOKABLE bool hasPositionConfig(UkuiQuick::WidgetItem *item);

    Q_INVOKABLE void initWithConfig(UkuiQuick::WidgetItem *item, QQuickItem *delegate = nullptr);

    Q_INVOKABLE bool checkItemsIntersected(QQuickItem *item1, QQuickItem *item2);

    QList<UkuiQuick::WidgetItem *> getAllWidgets();

    void updateItemPositionConfig(UkuiQuick::WidgetItem *item, const QPoint &pos);

    static bool isItemLayoutWithGrid(UkuiQuick::WidgetItem *widgetItem);

    static int itemWidgetGridUnitSize(UkuiQuick::WidgetItem *widgetItem);
Q_SIGNALS:
};

#endif // WIDGETS_PROXY_MODEL_H
