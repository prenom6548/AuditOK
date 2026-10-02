#include "ui/skillssettingswidget.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

SkillsSettingsWidget::SkillsSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_searchLineEdit(nullptr)
    , m_selfEvolutionCheckBox(nullptr)
    , m_countLabel(nullptr)
    , m_refreshButton(nullptr)
    , m_cardsWidget(nullptr)
    , m_cardsLayout(nullptr)
    , m_emptyLabel(nullptr)
{
    createSkillsGroup();
    loadSettings();
    refreshSkills();
}

void SkillsSettingsWidget::loadSettings()
{
    if (!m_selfEvolutionCheckBox) {
        return;
    }
    m_selfEvolutionCheckBox->setChecked(skillSelfEvolutionEnabled());
}

void SkillsSettingsWidget::saveSettings()
{
    if (m_selfEvolutionCheckBox) {
        setSkillSelfEvolutionEnabled(m_selfEvolutionCheckBox->isChecked());
    }
}

void SkillsSettingsWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const int columns = columnCountForWidth(event->size().width());
    if (columns != m_lastColumnCount) {
        m_lastColumnCount = columns;
        updateCards();
    }
}

void SkillsSettingsWidget::createSkillsGroup()
{
    m_mainLayout = new QVBoxLayout(this);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText(tr("搜索技能"));
    toolbarLayout->addWidget(m_searchLineEdit, 1);

    m_refreshButton = new QPushButton(tr("刷新"), this);
    toolbarLayout->addWidget(m_refreshButton);
    m_mainLayout->addLayout(toolbarLayout);

    m_selfEvolutionCheckBox = new QCheckBox(tr("启用技能自进化"), this);
    m_mainLayout->addWidget(m_selfEvolutionCheckBox);

    m_countLabel = new QLabel(this);
    m_mainLayout->addWidget(m_countLabel);

    m_cardsWidget = new QWidget(this);
    m_cardsLayout = new QGridLayout(m_cardsWidget);
    m_mainLayout->addWidget(m_cardsWidget, 1);

    m_emptyLabel = new QLabel(tr("暂无可用技能"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_cardsLayout->addWidget(m_emptyLabel, 0, 0);

    connect(m_refreshButton, &QPushButton::clicked,
            this, &SkillsSettingsWidget::refreshSkillsWithFeedback);
    connect(m_searchLineEdit, &QLineEdit::textChanged,
            this, &SkillsSettingsWidget::updateCards);
    connect(m_selfEvolutionCheckBox, &QCheckBox::toggled,
            this, [this]() { saveSettings(); });
}

void SkillsSettingsWidget::refreshSkills()
{
    m_installedSkills.clear();
    updateCards();
}

void SkillsSettingsWidget::refreshSkillsWithFeedback()
{
    refreshSkills();
    if (m_countLabel) {
        m_countLabel->setText(tr("已刷新技能列表"));
        QTimer::singleShot(1200, this, &SkillsSettingsWidget::updateCards);
    }
}

void SkillsSettingsWidget::updateCards()
{
    if (!m_countLabel || !m_emptyLabel) {
        return;
    }
    ++m_cardBuildGeneration;
    m_emptyLabel->setVisible(m_installedSkills.isEmpty());
    m_countLabel->setText(tr("已安装技能：%1").arg(m_installedSkills.size()));
}

void SkillsSettingsWidget::appendSkillCardBatch(const QVector<SkillInfo> &,
                                                int,
                                                int,
                                                int)
{
}

int SkillsSettingsWidget::columnCountForWidth(int width) const
{
    return width >= 960 ? 3 : (width >= 640 ? 2 : 1);
}

bool SkillsSettingsWidget::skillSelfEvolutionEnabled() const
{
    return QSettings().value(QStringLiteral("Skills/SelfEvolutionEnabled"), false).toBool();
}

bool SkillsSettingsWidget::setSkillSelfEvolutionEnabled(bool enabled) const
{
    QSettings settings;
    settings.setValue(QStringLiteral("Skills/SelfEvolutionEnabled"), enabled);
    settings.sync();
    return settings.status() == QSettings::NoError;
}
