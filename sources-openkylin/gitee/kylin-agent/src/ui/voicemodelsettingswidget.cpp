#include "ui/voicemodelsettingswidget.h"

VoiceModelSettingsWidget::VoiceModelSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_platformCombo(nullptr)
    , m_modelCombo(nullptr)
    , m_apiKeyEdit(nullptr)
    , m_autoStopCheck(nullptr)
    , m_autoStopSecondsSpin(nullptr)
    , m_autoStopSendCheck(nullptr)
    , m_statusLabel(nullptr)
{
}
