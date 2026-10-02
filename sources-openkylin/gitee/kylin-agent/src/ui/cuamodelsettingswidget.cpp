#include "ui/cuamodelsettingswidget.h"

#include <QShowEvent>

CuaModelSettingsWidget::CuaModelSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_dedicatedTable(nullptr)
    , m_statusLabel(nullptr)
    , m_updatingSelectors(false)
{
}

void CuaModelSettingsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}
