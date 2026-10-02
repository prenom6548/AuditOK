#ifndef CUAMODELCONFIGSYNC_H
#define CUAMODELCONFIGSYNC_H

#include <QMap>
#include <QString>
#include <QVariant>
#include <QVector>

class CuaModelConfigSync
{
public:
    // Keeps CUA role configuration attached to the stable id of a global
    // model. previousModels is used to migrate legacy CUA configurations that
    // only contain model_name/base_url before a model is renamed.
    static bool synchronize(
        const QVector<QMap<QString, QVariant>> &previousModels,
        const QVector<QMap<QString, QVariant>> &currentModels);
};

#endif // CUAMODELCONFIGSYNC_H
