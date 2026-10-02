#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariant>

class SavedModel
{
    Q_GADGET
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString provider READ provider)
    Q_PROPERTY(QString model READ model)
    Q_PROPERTY(QString baseUrl READ baseUrl)
    Q_PROPERTY(qint64 createdAt READ createdAt)

public:
    SavedModel() = default;
    SavedModel(const QString &id, const QString &name, const QString &provider,
               const QString &model, const QString &baseUrl, qint64 createdAt)
        : m_id(id), m_name(name), m_provider(provider), m_model(model),
          m_baseUrl(baseUrl), m_createdAt(createdAt) {}

    QString id() const { return m_id; }
    QString name() const { return m_name; }
    QString provider() const { return m_provider; }
    QString model() const { return m_model; }
    QString baseUrl() const { return m_baseUrl; }
    qint64 createdAt() const { return m_createdAt; }

    void setId(const QString &id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setProvider(const QString &provider) { m_provider = provider; }
    void setModel(const QString &model) { m_model = model; }
    void setBaseUrl(const QString &baseUrl) { m_baseUrl = baseUrl; }
    void setCreatedAt(qint64 createdAt) { m_createdAt = createdAt; }

private:
    QString m_id;
    QString m_name;
    QString m_provider;
    QString m_model;
    QString m_baseUrl;
    qint64 m_createdAt = 0;
};

class ModelProvider
{
    Q_GADGET
    Q_PROPERTY(QString value READ value)
    Q_PROPERTY(QString label READ label)

public:
    ModelProvider() = default;
    ModelProvider(const QString &value, const QString &label)
        : m_value(value), m_label(label) {}

    QString value() const { return m_value; }
    QString label() const { return m_label; }

private:
    QString m_value;
    QString m_label;
};

Q_DECLARE_METATYPE(SavedModel)
Q_DECLARE_METATYPE(ModelProvider)

#endif // CHATMODEL_H
