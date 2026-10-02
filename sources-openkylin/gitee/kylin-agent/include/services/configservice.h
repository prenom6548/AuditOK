#ifndef CONFIGSERVICE_H
#define CONFIGSERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QVariant>

class ConfigService : public QObject
{
    Q_OBJECT

public:
    static ConfigService& instance();

    /// Build model list from config.yaml model/custom_providers entries.
    /// Only explicitly configured models are listed; endpoints are not
    /// probed for additional models.
    QVector<QMap<QString, QVariant>> buildModelList();
    QString defaultModel() const { return m_defaultModel; }
    QString agentRuntimeHost() const { return m_agentRuntimeHost; }
    QString agentRuntimeEndpoint() const;
    QString selectedModelId() const { return m_selectedModelId; }
    void setAgentRuntimeHost(const QString &host);
    void setSelectedModelId(const QString &modelId);

signals:
    void agentRuntimeChanged(const QString &host);

private:
    explicit ConfigService(QObject *parent = nullptr);
    ~ConfigService();
    ConfigService(const ConfigService&) = delete;
    ConfigService& operator=(const ConfigService&) = delete;

    void load();
    void save() const;
    void reload();
    static QString normalizeAgentRuntimeHost(QString host);
    void parseConfigYaml(const QString &path);
    void fetchRemoteModels();
    QStringList fetchModelsFromEndpoint(const QString &baseUrl, const QString &apiKey);

    struct ProviderEntry {
        QString slug;
        QString name;
        QString baseUrl;
        QString apiKey;
        QStringList models;
    };

    QString m_hermesDir;
    QString m_defaultModel;
    QString m_configProvider;
    QString m_configBaseUrl;
    QString m_configApiKey;
    QString m_agentRuntimeHost;
    QString m_selectedModelId;
    QVector<ProviderEntry> m_providers;
};

#endif // CONFIGSERVICE_H
