#ifndef INFORMATIONITEM_H
#define INFORMATIONITEM_H

#include <QString>
#include <QSet>
typedef QSet<QString> ModuleTypeSet;
class InformationItem
{
public:
    enum CollectionType { File = 0, CMD, Others };
    InformationItem(const QString &n, CollectionType t, const QString &m, const ModuleTypeSet &s);
    bool hasType(const ModuleTypeSet &set);
    CollectionType collectionType();
    QString getItemName() const;
    QString getMessage() const;

private:
    QString m_itemName;
    CollectionType m_collectionType;
    QString m_message;
    ModuleTypeSet m_moduleTypeSet;
};

#endif // INFORMATIONITEM_H
