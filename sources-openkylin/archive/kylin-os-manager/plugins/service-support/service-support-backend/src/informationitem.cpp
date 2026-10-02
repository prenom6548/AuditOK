#include "informationitem.h"

InformationItem::InformationItem(const QString &n, CollectionType t, const QString &m, const ModuleTypeSet &s)
{
    m_itemName = n;
    m_collectionType = t;
    m_message = m;
    m_moduleTypeSet = s;
}

bool InformationItem::hasType(const ModuleTypeSet &set)
{
    for (const QString &str : set) {
        if (m_moduleTypeSet.contains(str)) {
            return true;
        }
    }
    return false;
}

InformationItem::CollectionType InformationItem::collectionType()
{
    return m_collectionType;
}

QString InformationItem::getItemName() const
{
    return m_itemName;
}

QString InformationItem::getMessage() const
{
    return m_message;
}
