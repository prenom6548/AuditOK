#include "diskviewscrollarea.h"
#include "diskitemwidget.h"
#include <QThread>
#include <QGridLayout>
#include <QTimer>
#include <QDebug>

#include "commoninfo.h"

const int itemSpace = 6;

DiskViewScrollArea::DiskViewScrollArea(QWidget *parent) : QScrollArea(parent)
{
    m_pCentralWidget = new QWidget(this);
    this->setWidget(m_pCentralWidget);
    this->setFrameShape(QFrame::NoFrame);

    onUpdateData();
}

QString DiskViewScrollArea::getCurrentDiskName()
{
    return m_diskName;
}

void DiskViewScrollArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    updateWidgetGeometry();
}

void DiskViewScrollArea::fontChanged(const QFont &font)
{
    for (int i = 0; i < m_listBlockItemWidget.size(); ++i) {
        m_listBlockItemWidget[i]->fontChanged(font);
    }
}

void DiskViewScrollArea::updateWidgetGeometry()
{
    int deviceCount  = m_listDevice.size();
    if (deviceCount <= 0) {
        return ;
    }

    if (deviceCount == 1) {
        showItemOne();
    } else if (deviceCount == 2) {
        showItemTwo();
    } else if (deviceCount > 2) {
        showItemLgTwo(deviceCount);
    }
}

void DiskViewScrollArea::onSetItemStatus(const QString &deviceName)
{
    for (auto it = m_mapDeviceItemWidget.begin(); it != m_mapDeviceItemWidget.end(); ++it) {
        it.value()->activeItemWidget(it.key() == deviceName);
    }
    m_diskName = deviceName;
    emit changeInfo(deviceName);
}

void DiskViewScrollArea::showItemOne()
{
    DiskItemWidget *item = m_listBlockItemWidget.at(0);
    item->updateData(m_listDevice.at(0));
    item->setMode(DiskItemWidget::TITLE_HORIZONTAL);
    item->show();

    m_mapDeviceItemWidget.insert(m_listDevice.at(0), item);

    item->setGeometry(0, 0, this->width(), this->height());
    item->activeItemWidget(true);

    m_pCentralWidget->setFixedSize(this->width(), this->height());

    for (int i = 1 ; i < m_listBlockItemWidget.size(); i++) {
        m_listBlockItemWidget.at(i)->hide();
        m_listBlockItemWidget.at(i)->setMode(DiskItemWidget::TITLE_HORIZONTAL);
    }
    emit changeInfo(m_listDevice.at(0));
}

void DiskViewScrollArea::showItemTwo()
{
    int avgWidth = (this->width() - itemSpace) / 2;
    int avgheight = this->height();

    DiskItemWidget *item1 = m_listBlockItemWidget.at(0);
    DiskItemWidget *item2 = m_listBlockItemWidget.at(1);
    item1->show();
    item2->show();

    item1->updateData(m_listDevice.at(0));
    item2->updateData(m_listDevice.at(1));

    item1->setMode(DiskItemWidget::TITLE_VERTICAL);
    item2->setMode(DiskItemWidget::TITLE_VERTICAL);

    m_mapDeviceItemWidget.insert(m_listDevice.at(0), item1);
    m_mapDeviceItemWidget.insert(m_listDevice.at(1), item2);

    item1->setGeometry(0, 0, avgWidth, avgheight);
    item2->setGeometry(item1->geometry().right() + itemSpace, 0, avgWidth, avgheight);

    if (!item1->isActiveItem() && !item2->isActiveItem()) {
        item1->activeItemWidget(true);
    }

    m_pCentralWidget->setFixedSize(this->width(), this->height());

    bool  haveSelect = false;
    for (int i = 2 ; i < m_listBlockItemWidget.size(); i++) {
        m_listBlockItemWidget.at(i)->hide();
        if (m_listBlockItemWidget.at(i)->isActiveItem()) {
            haveSelect = true;
        }
        m_listBlockItemWidget.at(i)->setMode(DiskItemWidget::TITLE_HORIZONTAL);
    }

    if (haveSelect) {
        m_listBlockItemWidget.at(0)->activeItemWidget(true);
        emit changeInfo(m_listDevice.at(0));
    }
}

void DiskViewScrollArea::showItemLgTwo(int count)
{
    int totalPage = qMax(0, (count - 1) / 4);
    int itemWidth = (this->width() - itemSpace) / 2;;
    int itemHeight = this->height() / 2;

    bool noSelect = true;
    for (int i = 0 ; i < count; i++) {
        int page = i / 4;
        DiskItemWidget *itemWidget = m_listBlockItemWidget.at(i);
        itemWidget->setMode(DiskItemWidget::TITLE_VERTICAL);
        itemWidget->show();

        m_mapDeviceItemWidget.insert(m_listDevice.at(i), itemWidget);
        if (i % 4 == 0) {
            itemWidget->setGeometry(page * this->width(), 0, itemWidth, itemHeight);
        } else if (i % 4 == 1) {
            itemWidget->setGeometry(page * this->width() + itemWidth + itemSpace, 0, itemWidth, itemHeight);
        } else if (i % 4 == 2) {
            itemWidget->setGeometry(page * this->width(), itemHeight, itemWidth, itemHeight);
        } else if (i % 4 == 3) {
            itemWidget->setGeometry(page * this->width() + itemWidth + itemSpace, itemHeight, itemWidth, itemHeight);
        }

        if (itemWidget->isActiveItem()) {
            noSelect = false;
        }
        itemWidget->updateData(m_listDevice.at(i));
    }

    if (noSelect) {
        m_listBlockItemWidget.at(0)->activeItemWidget(true);
        emit changeInfo(m_listDevice.at(0));
    }

    m_pCentralWidget->setFixedSize(this->width() + this->width() * totalPage, this->height());

    bool  haveSelect = false;
    for (int i = count ; i < m_listBlockItemWidget.size(); i++) {
        m_listBlockItemWidget.at(i)->hide();
        if (m_listBlockItemWidget.at(i)->isActiveItem()) {
            haveSelect = true;
        }
        m_listBlockItemWidget.at(i)->setMode(DiskItemWidget::TITLE_HORIZONTAL);
    }

    if (haveSelect) {
        m_listBlockItemWidget.at(0)->activeItemWidget(true);
        emit changeInfo(m_listDevice.at(0));
    }
}

void DiskViewScrollArea::onUpdateData()
{
    m_listDevice = diskInfo::getDiskName();
    qDebug() << "device name list = " << m_listDevice;
    m_mapDeviceItemWidget.clear();

    int deviceCount = m_listDevice.size();
    int curItemSize = m_listBlockItemWidget.size();
    m_diskName = m_listDevice.at(0);
    for (int i = 0 ; i < deviceCount - curItemSize; i++) {
        DiskItemWidget *item = new DiskItemWidget(m_pCentralWidget);
        m_listBlockItemWidget << item;
        connect(item, &DiskItemWidget::clicked, this, &DiskViewScrollArea::onSetItemStatus);
    }
    updateWidgetGeometry();
}
