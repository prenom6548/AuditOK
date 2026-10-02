#include "customtreewidget.h"
#include <QTreeWidgetItem>
#include <QMap>
#include "../hwwidget.h"

CustomTreeWidget::CustomTreeWidget(QWidget *parent) : QTreeWidget(parent) {

}

void CustomTreeWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
}

void CustomTreeWidget::customSort() {
    static QMap<QString, int> sortOrder = {
            {tr("CPU"), 0}, {tr("Memory"), 1}, {tr("Graphic Card"), 2}, {tr("Mother Board"), 3}, {tr("Network Card"), 4}, {tr("Disk"), 5},
            {tr("Monitor"), 6}, {tr("Sound Card"), 7}, {tr("Keyboard"), 8}, {tr("Mouse"), 9}, {tr("Bluetooth"), 10}, {tr("CD"), 11}, {tr("Battery"), 12},
            {tr("Fan"), 13}, {tr("Camera"), 14}
    };

    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = topLevelItem(i);
        QString type = static_cast<HwWidget*>(this->itemWidget(item, 0))->getDevType();
        int order = sortOrder.value(type, topLevelItemCount()); // Default to end if not found in sortOrder
        item->setData(0, Qt::UserRole, order);
    }
    sortItems(0, Qt::AscendingOrder);
}
