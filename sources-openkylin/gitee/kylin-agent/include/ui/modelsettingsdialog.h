#ifndef MODELSETTINGSDIALOG_H
#define MODELSETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>

#include "../models/chatmodel.h"

class ModelSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModelSettingsDialog(QWidget *parent = nullptr);
    ~ModelSettingsDialog();

signals:
    void modelAdded(const SavedModel &model);
    void modelUpdated(const SavedModel &model);
    void modelDeleted(const QString &id);

private slots:
    void selectModel(int row);
    void clearForm();
    void saveModel();
    void deleteModel();
    void restoreDefaults();
    void loadModels();

private:
    void setupUi();
    void fillForm(const SavedModel &model);
    QString currentModelId() const;
    void populateProviders();
    bool validateForm();
    
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_contentLayout;
    
    // Left side - model list
    QVBoxLayout *m_listLayout;
    QListWidget *m_modelList;
    QHBoxLayout *m_listButtonsLayout;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;
    
    // Right side - edit form
    QGroupBox *m_editGroup;
    QFormLayout *m_formLayout;
    QLineEdit *m_nameEdit;
    QComboBox *m_providerCombo;
    QLineEdit *m_modelEdit;
    QLineEdit *m_modelIdEdit;
    QLineEdit *m_baseUrlEdit;
    QPushButton *m_saveButton;
    QPushButton *m_restoreButton;
    QDialogButtonBox *m_buttonBox;
    
    // State
    QString m_editingModelId;
    bool m_isEditing = false;
    QVector<SavedModel> m_models;
};

#endif // MODELSETTINGSDIALOG_H
