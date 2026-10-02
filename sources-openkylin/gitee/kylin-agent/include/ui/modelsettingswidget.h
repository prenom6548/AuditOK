#ifndef MODELSETTINGSWIDGET_H
#define MODELSETTINGSWIDGET_H

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

class QTimer;

class ModelSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModelSettingsWidget(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();
    void openAddModelDialog();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    struct SavedModel {
        QString id;
        QString name;
        QString provider;
        QString model;
        QString baseUrl;
        QString apiKey;
        int contextLength = 0;
        qint64 createdAt = 0;
        bool readOnly = false;
        bool fixedLocal = false;
        bool available = true;
        QString fixedModelId;
    };

    QVBoxLayout *m_mainLayout;
    QLineEdit *m_searchEdit;
    QWidget *m_cardsWidget;
    QGridLayout *m_cardsLayout;
    QLabel *m_emptyLabel;
    QTimer *m_downloadRefreshTimer;
    QVector<SavedModel> m_models;
    int m_lastColumnCount = 0;

    void createModelGroup();
    void refreshModels();
    void updateCards();
    void showModelDialog(const SavedModel *model = nullptr);
    void deleteModel(const QString &id);
    void downloadFixedModel(const SavedModel &model);
    void removeFixedModel(const SavedModel &model);

    QVector<SavedModel> readModels();
    bool writeModels(const QVector<SavedModel> &models);
    void seedDefaultModels();
    int columnCountForWidth(int width) const;
    QString providerLabel(const QString &provider) const;
};

#endif // MODELSETTINGSWIDGET_H
