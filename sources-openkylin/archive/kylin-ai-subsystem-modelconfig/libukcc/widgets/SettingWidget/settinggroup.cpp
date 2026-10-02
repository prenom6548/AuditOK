#include "settinggroup.h"
#include <QApplication>

SettingGroup::SettingGroup(QWidget *parent, Shape shape, UkccFrame::BorderRadiusStyle style, int spacing)
    : UkccFrame(parent, style, false)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(spacing);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(mainLayout);
    this->setFrameShape(shape);
    qApp->installEventFilter(this);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

}

SettingGroup::~SettingGroup()
{
}

void SettingGroup::addWidget(QFrame *item, bool autoUpdateShape, bool visible)
{
    mainLayout->addWidget(item, autoUpdateShape);
    item->setVisible(visible);
    if (autoUpdateShape) {
        updateShape();
    }
}

void SettingGroup::insertWidget(int index, QFrame *item, bool autoUpdateShape, bool visible)
{
    mainLayout->insertWidget(index, item);
    item->setVisible(visible);
    if (autoUpdateShape) {
        updateShape();
    }
}

void SettingGroup::removeWidget(QWidget *w, bool autoUpdateShape)
{
    mainLayout->removeWidget(w);
    if (autoUpdateShape) {
        updateShape();
    }
}

void SettingGroup::updateAllItemList(UkccFrame *frame)
{
    if (frame->iscontainer() && frame->frameShape() == QFrame::NoFrame && frame->layout()) {
        for (int i = 0; i < frame->layout()->count(); i++) {
            QLayoutItem *it = frame->layout()->itemAt(i);
            UkccFrame *item = qobject_cast<UkccFrame *>(it->widget());
            if (!item) {
                continue;
            }
            updateAllItemList(item);
        }
    } else {
        allItemList.append(frame);
        return;
    }
}

void SettingGroup::updateShowItemList(UkccFrame *frame)
{
    if (frame->iscontainer() && frame->frameShape() == QFrame::NoFrame && frame->layout()) {
        for (int i = 0; i < frame->layout()->count(); i++) {
            QLayoutItem *it = frame->layout()->itemAt(i);
            UkccFrame *item = qobject_cast<UkccFrame *>(it->widget());
            if (!item) {
                continue;
            }
            updateShowItemList(item);
        }
    } else {
        if (frame->isVisibleTo(this)) {
            showItemList.append(frame);
        }
        return;
    }
}

void SettingGroup::updateShape()
{
    showItemList.clear();
    for (int i = 0; i < mainLayout->count(); i++) {
        QLayoutItem *it = mainLayout->itemAt(i);
        UkccFrame *item = qobject_cast<UkccFrame *>(it->widget());
        if (!item) {
            continue;
        }
        updateShowItemList(item);
    }

    for (int i = 0; i < showItemList.size(); i++) {
        UkccFrame *item = showItemList.at(i);
        if (showItemList.size() == 1) {
            item->setRadiusType(m_OneRadiusStyle);
        } else if (i == 0) {
            item->setRadiusType(UkccFrame::Top);
        } else if (i == showItemList.size() - 1) {
            item->setRadiusType(m_lastRadiusStyle);
        } else {
            item->setRadiusType(UkccFrame::None);
        }
    }
}

QVBoxLayout *SettingGroup::layout()
{
    return mainLayout;
}

void SettingGroup::setLastRadiusStyle(UkccFrame::BorderRadiusStyle style)
{
    m_lastRadiusStyle = style;
}

void SettingGroup::setOneRadiusStyle(UkccFrame::BorderRadiusStyle style)
{
    m_OneRadiusStyle = style;
}

bool SettingGroup::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Hide || event->type() == QEvent::Show) {
        allItemList.clear();
        for (int i = 0; i < mainLayout->count(); i++) {
             QLayoutItem *it = mainLayout->itemAt(i);
             UkccFrame *item = qobject_cast<UkccFrame *>(it->widget());
             updateAllItemList(item);

        }
        for (int i = 0; i < allItemList.size(); i++) {
            if (allItemList.at(i) == watched) {
               updateShape();
            }
        }
    }
    return UkccFrame::eventFilter(watched, event);
}

int SettingGroup::showItemsCount()
{
    showItemList.clear();
    for (int i = 0; i < mainLayout->count(); i++) {
        QLayoutItem *it = mainLayout->itemAt(i);
        UkccFrame *item = qobject_cast<UkccFrame *>(it->widget());
        if (!item) {
            continue;
        }
        updateShowItemList(item);
    }
    return showItemList.size();
}

void SettingGroup::removeAndDeleteAllWidget()
{
    while (mainLayout->count() > 0) {
        QLayoutItem *it = mainLayout->itemAt(0);
        UkccFrame *item = qobject_cast<UkccFrame *>(it->widget());
        if (!item) {
            continue;
        }
        mainLayout->removeWidget(item);
        item->deleteLater();
        item = nullptr;
    }
    return;
}
