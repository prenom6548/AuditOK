#ifndef DEVICE_DISK_H
#define DEVICE_DISK_H

#include <QObject>
#include <QString>
#include <QHash>
namespace KServer {

typedef QHash<QString, QString>LabelItems;
typedef QHash<QString, QString> PartLableItems;
typedef QHash<QString, QString> UUIDItems;

LabelItems getparsedLabelDir();
PartLableItems getparsedPartLabelDir();
UUIDItems getparsedUUIDDir();

}
#endif // DEVICE_DISK_H
