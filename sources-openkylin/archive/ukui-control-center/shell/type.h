#ifndef TYPE_H
#define TYPE_H

#include <QDBusArgument>
#include <QDBusMetaType>

typedef struct _KeyEntry {
    QString gsSchema;
    QString keyStr;
    QString valueStr;
    QString descStr;
    QString gsPath;
    QString nameStr;
    QString bindingStr;
    QString actionStr;
}KeyEntry;

const QDBusArgument &operator<<(QDBusArgument &argument, const KeyEntry &key);
const QDBusArgument &operator>>(const QDBusArgument &argument, KeyEntry &key);
Q_DECLARE_METATYPE(KeyEntry)

typedef struct
{
    QString name;
    QString key;
    QString mediaKey;
}QStringPair;

const QDBusArgument &operator<<(QDBusArgument &argument, const QStringPair &string);
const QDBusArgument &operator>>(const QDBusArgument &argument, QStringPair &string);
Q_DECLARE_METATYPE(QStringPair)

typedef QList<KeyEntry> KeyEntryList;
typedef QList<QStringPair> QStringPairList;

#endif // TYPE_H
