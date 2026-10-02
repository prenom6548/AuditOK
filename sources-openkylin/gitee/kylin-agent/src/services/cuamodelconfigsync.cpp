#include "services/cuamodelconfigsync.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace {

const QStringList kCuaRoleNames = {
    QStringLiteral("brain_llm"),
    QStringLiteral("actor_llm"),
    QStringLiteral("memory_llm"),
    QStringLiteral("planner_llm")
};

QString cuaConfigPath()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".local/share/kylin-cua/run/config.json"));
}

QString rowValue(const QMap<QString, QVariant> &row, const QString &primary,
                 const QString &fallback = QString())
{
    QString value = row.value(primary).toString().trimmed();
    if (value.isEmpty() && !fallback.isEmpty()) {
        value = row.value(fallback).toString().trimmed();
    }
    return value;
}

QString rowId(const QMap<QString, QVariant> &row)
{
    return rowValue(row, QStringLiteral("id"));
}

QString rowModel(const QMap<QString, QVariant> &row)
{
    QString model = rowValue(row, QStringLiteral("model"));
    if (model.isEmpty()) {
        model = rowId(row).section(QLatin1Char('/'), -1);
    }
    return model;
}

QString rowBaseUrl(const QMap<QString, QVariant> &row)
{
    return rowValue(row, QStringLiteral("baseUrl"), QStringLiteral("base_url"));
}

QString normalizedBaseUrl(QString value)
{
    value = value.trimmed();
    while (value.endsWith(QLatin1Char('/'))) {
        value.chop(1);
    }
    return value.toLower();
}

QString operationalKey(const QString &model, const QString &baseUrl)
{
    return model.trimmed().toLower() + QLatin1Char('\n') + normalizedBaseUrl(baseUrl);
}

const QMap<QString, QVariant> *findById(
    const QVector<QMap<QString, QVariant>> &models, const QString &id)
{
    for (const auto &model : models) {
        if (rowId(model) == id) {
            return &model;
        }
    }
    return nullptr;
}

const QMap<QString, QVariant> *findByConfiguration(
    const QVector<QMap<QString, QVariant>> &models, const QJsonObject &section)
{
    const QString key = operationalKey(section.value(QStringLiteral("model_name")).toString(),
                                       section.value(QStringLiteral("base_url")).toString());
    for (const auto &model : models) {
        if (operationalKey(rowModel(model), rowBaseUrl(model)) == key) {
            return &model;
        }
    }
    return nullptr;
}

void applyModel(QJsonObject &section, const QMap<QString, QVariant> &model)
{
    const QString id = rowId(model);
    const QString name = rowValue(model, QStringLiteral("name"));
    const QString apiKey = rowValue(model, QStringLiteral("apiKey"), QStringLiteral("api_key"));
    section.insert(QStringLiteral("provider"), QStringLiteral("openai_compatible"));
    section.insert(QStringLiteral("model_id"), QStringLiteral("global:") + id);
    section.insert(QStringLiteral("display_name"), name.isEmpty() ? rowModel(model) : name);
    section.insert(QStringLiteral("model_name"), rowModel(model));
    section.insert(QStringLiteral("base_url"), rowBaseUrl(model));
    section.insert(QStringLiteral("api_key"),
                   apiKey.isEmpty() ? QStringLiteral("not_needed") : apiKey);
}

} // namespace

bool CuaModelConfigSync::synchronize(
    const QVector<QMap<QString, QVariant>> &previousModels,
    const QVector<QMap<QString, QVariant>> &currentModels)
{
    QFile input(cuaConfigPath());
    if (!input.exists()) {
        return true;
    }
    if (!input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(input.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return false;
    }

    QJsonObject config = document.object();
    bool changed = false;
    for (const QString &role : kCuaRoleNames) {
        if (!config.value(role).isObject()) {
            continue;
        }
        QJsonObject section = config.value(role).toObject();
        const QString reference = section.value(QStringLiteral("model_id")).toString();
        if (reference.startsWith(QStringLiteral("dedicated:"))) {
            continue;
        }

        QString globalId;
        if (reference.startsWith(QStringLiteral("global:"))) {
            globalId = reference.mid(QStringLiteral("global:").size());
        }

        const QMap<QString, QVariant> *selected = globalId.isEmpty()
            ? findByConfiguration(currentModels, section)
            : findById(currentModels, globalId);

        // Legacy configurations have no stable reference. Resolve their old
        // operational values before an edit, then follow that card's id into
        // the new model list.
        if (!selected && globalId.isEmpty()) {
            if (const auto *previous = findByConfiguration(previousModels, section)) {
                globalId = rowId(*previous);
                selected = findById(currentModels, globalId);
                // A legacy reference that belonged to a deleted global model
                // must fall back instead of remaining as a stale custom entry.
                if (!selected && !currentModels.isEmpty()) {
                    selected = &currentModels.first();
                }
            }
        }

        // A stable global reference whose card was deleted uses the first
        // configured model, which is the runtime's current default model.
        if (!selected && !globalId.isEmpty() && !currentModels.isEmpty()) {
            selected = &currentModels.first();
        }
        if (!selected) {
            continue;
        }

        const QJsonObject before = section;
        applyModel(section, *selected);
        if (section != before) {
            config.insert(role, section);
            changed = true;
        }
    }

    if (!changed) {
        return true;
    }
    QSaveFile output(cuaConfigPath());
    if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    const QByteArray data = QJsonDocument(config).toJson(QJsonDocument::Indented);
    const bool ok = output.write(data) == data.size() && output.commit();
    if (ok) {
        QFile::setPermissions(cuaConfigPath(), QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }
    return ok;
}
