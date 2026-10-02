#include "modelcallpriority.h"

#include <ukui-log4qt.h>

#include <QHBoxLayout>
#include <QLabel>

using namespace kyai::config::model;

namespace {
const QString& cloudModelText() {
    static const QString text = QObject::tr("Cloud Model First (Kylin Certification)");
    return text;
}

const QString& localModelText() {
    static const QString text = QObject::tr("Local Model First (Kylin Certification)");
    return text;
}

const QString& privateModelText() {
    static const QString text = QObject::tr("Custom Model First");
    return text;
}

const QString& sharedModelText() {
    static const QString text = QObject::tr("Computing Power Sharing Model First");
    return text;
}

const QMap<QString, DeployType>& modelTypeMap() {
    static const QMap<QString, DeployType> map = {{cloudModelText(), DeployType::PublicCloud},
                                                  {localModelText(), DeployType::Ondevice},
                                                  {privateModelText(), DeployType::Custom},
                                                  {sharedModelText(), DeployType::Shared}};
    return map;
}
}  // namespace

ModelCallPriority::ModelCallPriority(bool hasLocalModel, bool isShowSharedOption, QWidget* parent)
    : m_hasLocalModel(hasLocalModel), m_isShowSharedOption(isShowSharedOption), QWidget(parent) {
    m_modelCallPriorityLabel = new kdk::KLabel(parent);
    m_modelCallPriorityLabel->setText(tr("Model Call Priority"));
    m_modelCallPriorityLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_modelCallPriorityLabel");

    m_modelPriorityComboBox = new QComboBox(parent);
    m_modelPriorityComboBox->setAccessibleName("kylin-ai-subsystem-modelconfig_QComboBox_m_modelCallPriorityLabel");
    m_modelPriorityComboBox->setMinimumSize(320, 36);

    m_modelPriorityComboBox->addItem(cloudModelText());
    if (m_hasLocalModel) {
        m_modelPriorityComboBox->addItem(localModelText());
    }
    m_modelPriorityComboBox->addItem(privateModelText());
    if (m_isShowSharedOption) {
        m_modelPriorityComboBox->addItem(sharedModelText());
    }

    QHBoxLayout* modelCellPriLayout = new QHBoxLayout();
    modelCellPriLayout->setContentsMargins(16, 0, 16, 0);
    modelCellPriLayout->setSpacing(2);

    modelCellPriLayout->addWidget(m_modelCallPriorityLabel);
    modelCellPriLayout->addWidget(m_modelPriorityComboBox);

    parent->setLayout(modelCellPriLayout);

    initConnections();
}

void ModelCallPriority::setModelCallPriority(kyai::config::model::DeployType deployType) {
    if (deployType == DeployType::PublicCloud) {
        m_modelPriorityComboBox->setCurrentText(cloudModelText());
    } else if (deployType == DeployType::Ondevice) {
        m_modelPriorityComboBox->setCurrentText(localModelText());
    } else if (deployType == DeployType::Custom) {
        m_modelPriorityComboBox->setCurrentText(privateModelText());
    } else if (deployType == DeployType::Shared) {
        m_modelPriorityComboBox->setCurrentText(sharedModelText());
    }
}

void ModelCallPriority::setSharedModelOptionVisible(bool visible) {
    QString sharedText = sharedModelText();
    int index = m_modelPriorityComboBox->findText(sharedText);

    if (visible && index == -1) {
        m_modelPriorityComboBox->addItem(sharedText);
    }
    if (!visible && index != -1) {
        m_modelPriorityComboBox->removeItem(index);
    }
}

void ModelCallPriority::initConnections() {
    connect(m_modelPriorityComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modelPriorityChanged(int)));
}

void ModelCallPriority::modelPriorityChanged(int index) {
    Q_UNUSED(index);
    QString comboBoxCurText = m_modelPriorityComboBox->currentText();
    DeployType type = modelTypeMap().value(comboBoxCurText, DeployType::PublicCloud);
    emit topModelPriorityChanged(type);
}
