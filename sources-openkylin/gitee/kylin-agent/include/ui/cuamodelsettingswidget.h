#ifndef CUAMODELSETTINGSWIDGET_H
#define CUAMODELSETTINGSWIDGET_H

#include <QJsonObject>
#include <QMap>
#include <QVector>
#include <QWidget>

class QComboBox;
class QLabel;
class QTableWidget;
class QShowEvent;

class CuaModelSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CuaModelSettingsWidget(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private:
    struct ModelOption {
        QString id;
        QString name;
        QString model;
        QString baseUrl;
        QString apiKey;
        bool dedicated = false;
        bool currentConfig = false;
    };

    void setupUi();
    void loadConfiguration();
    void refreshModelOptions();
    void rebuildRoleSelectors(bool selectFromConfig);
    void rebuildDedicatedTable();
    void showDedicatedModelDialog(const QString &modelId = QString());
    void deleteDedicatedModel(const QString &modelId);
    void saveConfiguration();

    QVector<ModelOption> readGlobalModels() const;
    QVector<ModelOption> readDedicatedModels() const;
    bool writeDedicatedModels(const QVector<ModelOption> &models) const;
    const ModelOption *findOption(const QString &id) const;
    QString matchingOptionId(const QJsonObject &section) const;

    QMap<QString, QComboBox *> m_roleSelectors;
    QTableWidget *m_dedicatedTable;
    QLabel *m_statusLabel;
    QVector<ModelOption> m_options;
    QVector<ModelOption> m_dedicatedModels;
    QJsonObject m_config;
    bool m_updatingSelectors;
};

#endif // CUAMODELSETTINGSWIDGET_H
