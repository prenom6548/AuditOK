#include "multimediawin.h"

#include <QDebug>
#include <QScrollBar>

MultiMediaWin::MultiMediaWin(QWidget *parent) : QListWidget(parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    // 设置显示图标模式
    setViewMode(QListView::IconMode);
    // 自适应布局
    setResizeMode(QListView::Adjust);
    // 列表中的图标不可拖动
    setMovement(QListView::Static);
    // 去边框
    setFrameShape(QListWidget::NoFrame);
    // 设置边距
    setSpacing(16);
    // 不可选择项目
    setSelectionMode(QListView::NoSelection);
    // 禁用滚动
    verticalScrollBar()->setDisabled(true);
    horizontalScrollBar()->setDisabled(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setStyleSheet("QListWidget{background:transparent;}"
                  "QListWidget::Item{background:transparent;}"
                  "QListWidget::Item:Hover{background:transparent;}");

    for (int i = 0; i < 4; i++) {
        addMultiMediaItem(AndroidItem::Type(i));
    }
}

MultiMediaWin::~MultiMediaWin() {}

void MultiMediaWin::setAppItem()
{
    for (int i = 4; i < 6; i++) {
        addMultiMediaItem(AndroidItem::Type(i));
    }
}

void MultiMediaWin::setItemCount(AndroidItem::Type type, int num)
{
    if (type > m_maxItemIndex) {
        qCritical() << "More than the maximum number of items.";
        return;
    }
    m_itemMap.value(type)->setItemCount(num);
}

void MultiMediaWin::setTheme(PublicAttributes::Theme theme)
{
    for (AndroidItem *item : m_itemMap) {
        item->setTheme(theme);
    }
}

void MultiMediaWin::changeFontSize(double fontSize)
{
    for (AndroidItem *item : m_itemMap) {
        item->changeFontSize(fontSize);
    }
}

void MultiMediaWin::resizeEvent(QResizeEvent *event)
{
    int column = (size().width() - 16) / 219;
    int row = m_itemMap.size() / column;
    if (row == 0 || (m_itemMap.size() % column) != 0) {
        row = row + 1;
    }
    setFixedHeight(row * 96);
    QListWidget::resizeEvent(event);
}

void MultiMediaWin::addMultiMediaItem(AndroidItem::Type type)
{
    m_maxItemIndex++;
    QListWidgetItem *item = new QListWidgetItem(this);
    AndroidItem *itemWidget = new AndroidItem(type, this);
    item->setSizeHint(itemWidget->size());
    addItem(item);
    setItemWidget(item, itemWidget);
    m_itemMap.insert(type, itemWidget);
    connect(itemWidget, &AndroidItem::sigBtnClicked, this, &MultiMediaWin::sigBtnClicked);
}
