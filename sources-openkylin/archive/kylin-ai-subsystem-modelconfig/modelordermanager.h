#ifndef MODELORDERMANAGER_H
#define MODELORDERMANAGER_H

#include <QObject>
#include <QMap>
#include <aiconfig/modelconfig.h>
#include <QSettings>

class ModelOrderManager : public QObject
{
    Q_OBJECT
public:
    explicit ModelOrderManager(QObject *parent = nullptr);
    ~ModelOrderManager();

    void addModel(kyai::config::model::AiCapability aiCapability, const QString& modelName);
    void deleteModel(kyai::config::model::AiCapability aiCapability, const QString& modelName);

    QStringList getModelOrder(kyai::config::model::AiCapability aiCapability);

private:
    QString getModelTypeString(kyai::config::model::AiCapability aiCapability);

    void checkConfigFile();
    void initSettings();
    void getModelOrderFormConfigFile();
    void createDefaultConfig();
    void saveModelOrder(kyai::config::model::AiCapability aiCapability, const QStringList& order);

signals:

private:
    QSettings* m_settings;
    QMap<QString, QStringList> m_modelOrders;

    const std::string m_configDir =
        std::string(getenv("HOME")) + "/.config/kylin-ai-subsystem-modelconfig";
    const std::string m_configFile =
        std::string(getenv("HOME")) + "/.config/kylin-ai-subsystem-modelconfig/modelOrder.conf";
};

#endif // MODELORDERMANAGER_H
