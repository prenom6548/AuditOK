#include "modelordermanager.h"
#include <QDir>
#include <QFile>
#include <QDebug>

namespace  {
const QMap<int, QString> modelTypeMap {
    {(int)kyai::config::model::AiCapability::Nlp, "nlp"},
    {(int)kyai::config::model::AiCapability::Speech, "speech"},
    {(int)kyai::config::model::AiCapability::Vision, "vision"}
};
}

ModelOrderManager::ModelOrderManager(QObject *parent) : QObject(parent) {
    checkConfigFile();
    initSettings();
    getModelOrderFormConfigFile();
}

ModelOrderManager::~ModelOrderManager() {
    if (m_settings) {
        delete m_settings;
        m_settings = nullptr;
    }
}

void ModelOrderManager::addModel(kyai::config::model::AiCapability aiCapability, const QString& modelName) {
    QStringList modelList = m_modelOrders.value(getModelTypeString(aiCapability));
    if (modelList.contains(modelName)) {
        return;
    }
    modelList.push_back(modelName);
    saveModelOrder(aiCapability, modelList);
}

void ModelOrderManager::deleteModel(kyai::config::model::AiCapability aiCapability, const QString& modelName) {
    QStringList modelList = m_modelOrders.value(getModelTypeString(aiCapability));
    if (!modelList.contains(modelName)) {
        return;
    }
    modelList.removeOne(modelName);
    saveModelOrder(aiCapability, modelList);
}

QString ModelOrderManager::getModelTypeString(kyai::config::model::AiCapability aiCapability) {
    return modelTypeMap.value((int)aiCapability);
}

void ModelOrderManager::saveModelOrder(kyai::config::model::AiCapability aiCapability, const QStringList& order) {
    m_modelOrders[getModelTypeString(aiCapability)] = order;

    checkConfigFile();
    m_settings->setValue(getModelTypeString(aiCapability), order.join(","));
    m_settings->sync();
}

QStringList ModelOrderManager::getModelOrder(kyai::config::model::AiCapability aiCapability) {
    QString modelType = getModelTypeString(aiCapability);
    if (m_modelOrders.keys().contains(modelType)) {
        return m_modelOrders.value(modelType);
    }
    qWarning() << "Failed to obtain model order. Model type is " << modelType;
    return {};
}

void ModelOrderManager::checkConfigFile() {
    QFile file(m_configFile.c_str());
    if (file.exists()) {
        return;
    }
    createDefaultConfig();
}

void ModelOrderManager::initSettings() {
    m_settings = new QSettings(m_configFile.c_str(), QSettings::IniFormat);
}

void ModelOrderManager::getModelOrderFormConfigFile() {
    QStringList keys = m_settings->allKeys();
    for (const QString& key : keys) {
        QString value = m_settings->value(key).toString();
        m_modelOrders[key] = value.split(",", QString::SkipEmptyParts);
    }
}

void ModelOrderManager::createDefaultConfig() {
    QDir dir(m_configDir.c_str());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create config file " << m_configDir.c_str();
            return;
        }
    }

    QFile file(m_configFile.c_str());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot create default config file " << m_configFile.c_str();
    }

    file.close();
}
