#include "informationclassitem.h"
#include "informationitem.h"

InformationClassItem::InformationClassItem(const QString &name, bool select) : m_itemName(name), m_select(select)
{
    m_children = new InformationItemList;
}

InformationClassItem::~InformationClassItem()
{
    for (InformationItem *item : *m_children) {
        if (item) {
            delete item;
        }
    }
    m_children->clear();
    delete m_children;
}

QString InformationClassItem::getItemName() const
{
    return m_itemName;
}

QString InformationClassItem::getItemNameShow() const
{
    if (m_itemNameShow.isEmpty()) {
        return m_itemName;
    }
    return m_itemNameShow;
}

bool InformationClassItem::isSelect() const
{
    return m_select;
}

void InformationClassItem::setSelect(bool c)
{
    m_select = c;
}

int InformationClassItem::Detailed() const
{
    return m_detailed;
}

void InformationClassItem::setDetailed(int i)
{
    m_detailed = i;
}

void InformationClassItem::setItemNameShow(const QString &str)
{
    m_itemNameShow = str;
}

InformationItemList *InformationClassItem::children()
{
    return m_children;
}
