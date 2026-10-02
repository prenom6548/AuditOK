#ifndef MODELSERVICE_H
#define MODELSERVICE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include "../models/chatmodel.h"

class ModelService : public QObject
{
    Q_OBJECT

public:
    static ModelService& instance();
    
    // Model management
    QVector<SavedModel> listModels();
    SavedModel addModel(const QString &name, const QString &provider, 
                        const QString &model, const QString &baseUrl = "");
    bool updateModel(const QString &id, const QMap<QString, QString> &fields);
    bool removeModel(const QString &id);
    SavedModel getModel(const QString &id);
    
    // Default providers
    QVector<ModelProvider> getProviders() const;
    
    // Default models (seeded on first install)
    QVector<SavedModel> getDefaultModels() const;
    QVector<SavedModel> restoreDefaultModels();
    bool hasModel(const QString &provider,
                  const QString &modelName,
                  const QString &exceptId = QString());

signals:
    void modelsChanged();
    void modelAdded(const SavedModel &model);
    void modelUpdated(const SavedModel &model);
    void modelRemoved(const QString &id);

private:
    explicit ModelService(QObject *parent = nullptr);
    ~ModelService();
    ModelService(const ModelService&) = delete;
    ModelService& operator=(const ModelService&) = delete;
    
    bool initialize();
    QString modelsFilePath() const;
    QString generateModelId() const;
    QVector<SavedModel> readModels() const;
    bool writeModels(const QVector<SavedModel> &models);
    QVector<SavedModel> seedDefaults();
    
    bool m_initialized = false;
};

#endif // MODELSERVICE_H
