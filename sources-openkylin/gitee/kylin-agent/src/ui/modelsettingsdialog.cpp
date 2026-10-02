#include "ui/modelsettingsdialog.h"

#include "services/configservice.h"
#include "services/modelservice.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ModelSettingsDialog::ModelSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_modelList(nullptr)
    , m_nameEdit(nullptr)
    , m_providerCombo(nullptr)
    , m_modelEdit(nullptr)
    , m_baseUrlEdit(nullptr)
    , m_saveButton(nullptr)
    , m_deleteButton(nullptr)
    , m_restoreButton(nullptr)
{
    setupUi();
    loadModels();
}

ModelSettingsDialog::~ModelSettingsDialog() = default;

void ModelSettingsDialog::setupUi()
{
    setWindowTitle(QStringLiteral("Model Settings"));
    setMinimumSize(620, 380);

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(14);

    QVBoxLayout *listLayout = new QVBoxLayout;
    QLabel *listTitle = new QLabel(QStringLiteral("Models"), this);
    listTitle->setStyleSheet(QStringLiteral("font-weight: 600;"));
    listLayout->addWidget(listTitle);

    m_modelList = new QListWidget(this);
    connect(m_modelList, &QListWidget::currentRowChanged, this, &ModelSettingsDialog::selectModel);
    listLayout->addWidget(m_modelList, 1);
    root->addLayout(listLayout, 1);

    QVBoxLayout *editorLayout = new QVBoxLayout;
    QFormLayout *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight);

    m_nameEdit = new QLineEdit(this);
    form->addRow(QStringLiteral("Name:"), m_nameEdit);

    m_providerCombo = new QComboBox(this);
    for (const ModelProvider &provider : ModelService::instance().getProviders()) {
        m_providerCombo->addItem(provider.label(), provider.value());
    }
    form->addRow(QStringLiteral("Provider:"), m_providerCombo);

    m_modelEdit = new QLineEdit(this);
    form->addRow(QStringLiteral("Model:"), m_modelEdit);

    m_baseUrlEdit = new QLineEdit(this);
    m_baseUrlEdit->setPlaceholderText(QStringLiteral("Optional"));
    form->addRow(QStringLiteral("Base URL:"), m_baseUrlEdit);
    editorLayout->addLayout(form);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *newButton = new QPushButton(QStringLiteral("New"), this);
    connect(newButton, &QPushButton::clicked, this, &ModelSettingsDialog::clearForm);
    buttonLayout->addWidget(newButton);

    m_restoreButton = new QPushButton(QStringLiteral("Restore Defaults"), this);
    connect(m_restoreButton, &QPushButton::clicked, this, &ModelSettingsDialog::restoreDefaults);
    buttonLayout->addWidget(m_restoreButton);

    buttonLayout->addStretch();
    m_deleteButton = new QPushButton(QStringLiteral("Delete"), this);
    connect(m_deleteButton, &QPushButton::clicked, this, &ModelSettingsDialog::deleteModel);
    buttonLayout->addWidget(m_deleteButton);

    m_saveButton = new QPushButton(QStringLiteral("Save"), this);
    connect(m_saveButton, &QPushButton::clicked, this, &ModelSettingsDialog::saveModel);
    buttonLayout->addWidget(m_saveButton);
    editorLayout->addLayout(buttonLayout);

    root->addLayout(editorLayout, 2);
}

void ModelSettingsDialog::loadModels()
{
    const QString previousId = currentModelId();
    m_modelList->blockSignals(true);
    m_modelList->clear();

    int selectedRow = -1;
    const QVector<SavedModel> models = ModelService::instance().listModels();
    for (const SavedModel &model : models) {
        const QString label = model.name().isEmpty() ? model.model() : model.name();
        m_modelList->addItem(label);
        m_modelList->item(m_modelList->count() - 1)->setData(Qt::UserRole, model.id());
        if (!previousId.isEmpty() && model.id() == previousId) {
            selectedRow = m_modelList->count() - 1;
        }
    }

    m_modelList->blockSignals(false);
    if (m_modelList->count() == 0) {
        clearForm();
        m_restoreButton->setEnabled(true);
        return;
    }

    m_restoreButton->setEnabled(true);
    m_modelList->setCurrentRow(selectedRow >= 0 ? selectedRow : 0);
}

void ModelSettingsDialog::selectModel(int row)
{
    if (row < 0 || row >= m_modelList->count()) {
        clearForm();
        return;
    }

    fillForm(ModelService::instance().getModel(currentModelId()));
}

void ModelSettingsDialog::fillForm(const SavedModel &model)
{
    m_nameEdit->setText(model.name());
    m_modelEdit->setText(model.model());
    m_baseUrlEdit->setText(model.baseUrl());

    const int providerIndex = m_providerCombo->findData(model.provider());
    m_providerCombo->setCurrentIndex(providerIndex >= 0 ? providerIndex : 0);
    m_deleteButton->setEnabled(!model.id().isEmpty());
}

QString ModelSettingsDialog::currentModelId() const
{
    QListWidgetItem *item = m_modelList ? m_modelList->currentItem() : nullptr;
    return item ? item->data(Qt::UserRole).toString() : QString();
}

void ModelSettingsDialog::clearForm()
{
    if (m_modelList) {
        m_modelList->setCurrentRow(-1);
    }
    m_nameEdit->clear();
    m_modelEdit->clear();
    m_baseUrlEdit->clear();
    m_providerCombo->setCurrentIndex(0);
    m_deleteButton->setEnabled(false);
}

void ModelSettingsDialog::saveModel()
{
    const QString name = m_nameEdit->text().trimmed();
    const QString modelName = m_modelEdit->text().trimmed();
    const QString provider = m_providerCombo->currentData().toString();
    const QString baseUrl = m_baseUrlEdit->text().trimmed();

    if (name.isEmpty() || modelName.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Model Settings"),
                             QStringLiteral("Name and model are required."));
        return;
    }

    const QString id = currentModelId();
    if (ModelService::instance().hasModel(provider, modelName, id)) {
        QMessageBox::warning(this, QStringLiteral("Model Settings"),
                             QStringLiteral("A model with this provider and model ID already exists."));
        return;
    }

    if (id.isEmpty()) {
        const SavedModel model = ModelService::instance().addModel(name, provider, modelName, baseUrl);
        ConfigService::instance().setSelectedModelId(model.id());
    } else {
        QMap<QString, QString> fields;
        fields.insert(QStringLiteral("name"), name);
        fields.insert(QStringLiteral("provider"), provider);
        fields.insert(QStringLiteral("model"), modelName);
        fields.insert(QStringLiteral("baseUrl"), baseUrl);
        ModelService::instance().updateModel(id, fields);
    }
    loadModels();
}

void ModelSettingsDialog::deleteModel()
{
    const QString id = currentModelId();
    if (id.isEmpty()) {
        return;
    }

    if (QMessageBox::question(this, QStringLiteral("Delete Model"),
                              QStringLiteral("Delete this model?")) != QMessageBox::Yes) {
        return;
    }

    if (ConfigService::instance().selectedModelId() == id) {
        ConfigService::instance().setSelectedModelId(QString());
    }
    ModelService::instance().removeModel(id);
    loadModels();
}

void ModelSettingsDialog::restoreDefaults()
{
    ModelService::instance().restoreDefaultModels();
    loadModels();
}
