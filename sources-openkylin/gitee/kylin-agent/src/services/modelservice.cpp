#include "services/modelservice.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QUuid>

#include <algorithm>

ModelService::ModelService(QObject *parent)
    : QObject(parent)
{
    initialize();
}

ModelService::~ModelService() = default;

ModelService &ModelService::instance()
{
    static ModelService service;
    return service;
}

bool ModelService::initialize()
{
    if (m_initialized) {
        return true;
    }

    const QString path = modelsFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    if (!QFileInfo::exists(path)) {
        seedDefaults();
    }

    m_initialized = true;
    return true;
}

QString ModelService::modelsFilePath() const
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(configPath).filePath(QStringLiteral("models.json"));
}

QString ModelService::generateModelId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QVector<SavedModel> ModelService::listModels()
{
    return readModels();
}

QVector<SavedModel> ModelService::readModels() const
{
    QVector<SavedModel> models;

    QFile file(modelsFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open models file:" << file.fileName();
        return models;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        qWarning() << "Models file is not a JSON array:" << file.fileName();
        return models;
    }

    for (const QJsonValue &value : doc.array()) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();
        SavedModel model;
        model.setId(obj.value(QStringLiteral("id")).toString());
        model.setName(obj.value(QStringLiteral("name")).toString());
        model.setProvider(obj.value(QStringLiteral("provider")).toString());
        model.setModel(obj.value(QStringLiteral("model")).toString());
        model.setBaseUrl(obj.value(QStringLiteral("baseUrl")).toString());
        model.setCreatedAt(obj.value(QStringLiteral("createdAt")).toVariant().toLongLong());
        models.append(model);
    }

    return models;
}

bool ModelService::writeModels(const QVector<SavedModel> &models)
{
    const QString path = modelsFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonArray array;
    for (const SavedModel &model : models) {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), model.id());
        obj.insert(QStringLiteral("name"), model.name());
        obj.insert(QStringLiteral("provider"), model.provider());
        obj.insert(QStringLiteral("model"), model.model());
        obj.insert(QStringLiteral("baseUrl"), model.baseUrl());
        obj.insert(QStringLiteral("createdAt"), QString::number(model.createdAt()));
        array.append(obj);
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Failed to open models file for writing:" << path;
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    emit modelsChanged();
    return true;
}

QVector<SavedModel> ModelService::seedDefaults()
{
    QVector<SavedModel> models;
    for (SavedModel model : getDefaultModels()) {
        model.setId(generateModelId());
        model.setCreatedAt(QDateTime::currentSecsSinceEpoch());
        models.append(model);
    }

    writeModels(models);
    return models;
}

SavedModel ModelService::addModel(const QString &name, const QString &provider,
                                  const QString &modelId, const QString &baseUrl)
{
    QVector<SavedModel> models = readModels();
    if (hasModel(provider, modelId)) {
        auto it = std::find_if(models.begin(), models.end(), [&provider, &modelId](const SavedModel &model) {
            return model.provider() == provider && model.model() == modelId;
        });
        return it == models.end() ? SavedModel() : *it;
    }

    SavedModel model;
    model.setId(generateModelId());
    model.setName(name);
    model.setProvider(provider);
    model.setModel(modelId);
    model.setBaseUrl(baseUrl);
    model.setCreatedAt(QDateTime::currentSecsSinceEpoch());
    models.append(model);

    if (writeModels(models)) {
        emit modelAdded(model);
    }
    return model;
}

bool ModelService::updateModel(const QString &id, const QMap<QString, QString> &fields)
{
    QVector<SavedModel> models = readModels();
    auto it = std::find_if(models.begin(), models.end(), [&id](const SavedModel &model) {
        return model.id() == id;
    });

    if (it == models.end()) {
        return false;
    }

    if (fields.contains(QStringLiteral("name"))) it->setName(fields.value(QStringLiteral("name")));
    const QString provider = fields.value(QStringLiteral("provider"), it->provider());
    const QString modelName = fields.value(QStringLiteral("model"), it->model());
    if (hasModel(provider, modelName, id)) {
        return false;
    }

    if (fields.contains(QStringLiteral("provider"))) it->setProvider(provider);
    if (fields.contains(QStringLiteral("model"))) it->setModel(modelName);
    if (fields.contains(QStringLiteral("baseUrl"))) it->setBaseUrl(fields.value(QStringLiteral("baseUrl")));

    const SavedModel updated = *it;
    if (!writeModels(models)) {
        return false;
    }

    emit modelUpdated(updated);
    return true;
}

bool ModelService::removeModel(const QString &id)
{
    QVector<SavedModel> models = readModels();
    const int initialSize = models.size();
    models.erase(std::remove_if(models.begin(), models.end(), [&id](const SavedModel &model) {
        return model.id() == id;
    }), models.end());

    if (models.size() == initialSize || !writeModels(models)) {
        return false;
    }

    emit modelRemoved(id);
    return true;
}

SavedModel ModelService::getModel(const QString &id)
{
    for (const SavedModel &model : readModels()) {
        if (model.id() == id) {
            return model;
        }
    }
    return SavedModel();
}

bool ModelService::hasModel(const QString &provider, const QString &modelName, const QString &exceptId)
{
    for (const SavedModel &model : readModels()) {
        if (!exceptId.isEmpty() && model.id() == exceptId) {
            continue;
        }
        if (model.provider() == provider && model.model() == modelName) {
            return true;
        }
    }
    return false;
}

QVector<SavedModel> ModelService::restoreDefaultModels()
{
    QVector<SavedModel> restored = readModels();
    bool changed = false;

    for (SavedModel defaultModel : getDefaultModels()) {
        const bool exists = std::any_of(restored.cbegin(), restored.cend(), [&defaultModel](const SavedModel &model) {
            return model.provider() == defaultModel.provider() && model.model() == defaultModel.model();
        });
        if (exists) {
            continue;
        }

        defaultModel.setId(generateModelId());
        defaultModel.setCreatedAt(QDateTime::currentSecsSinceEpoch());
        restored.append(defaultModel);
        changed = true;
    }

    if (changed) {
        writeModels(restored);
    }
    return restored;
}

QVector<ModelProvider> ModelService::getProviders() const
{
    return {
        ModelProvider(QStringLiteral("openrouter"), QStringLiteral("OpenRouter")),
        ModelProvider(QStringLiteral("anthropic"), QStringLiteral("Anthropic")),
        ModelProvider(QStringLiteral("openai"), QStringLiteral("OpenAI")),
        ModelProvider(QStringLiteral("custom"), QStringLiteral("Custom")),
        ModelProvider(QStringLiteral("lmstudio"), QStringLiteral("LM Studio")),
        ModelProvider(QStringLiteral("ollama"), QStringLiteral("Ollama")),
        ModelProvider(QStringLiteral("vllm"), QStringLiteral("vLLM")),
        ModelProvider(QStringLiteral("llamacpp"), QStringLiteral("llama.cpp")),
    };
}

QVector<SavedModel> ModelService::getDefaultModels() const
{
    return {
        SavedModel(QString(), QStringLiteral("Claude Sonnet 4 (OpenRouter)"),
                   QStringLiteral("openrouter"), QStringLiteral("anthropic/claude-sonnet-4-20250514"),
                   QString(), 0),
        SavedModel(QString(), QStringLiteral("Claude Sonnet 4 (Anthropic)"),
                   QStringLiteral("anthropic"), QStringLiteral("claude-sonnet-4-20250514"),
                   QString(), 0),
        SavedModel(QString(), QStringLiteral("GPT-4.1 (OpenAI)"),
                   QStringLiteral("openai"), QStringLiteral("gpt-4.1"),
                   QString(), 0),
    };
}
