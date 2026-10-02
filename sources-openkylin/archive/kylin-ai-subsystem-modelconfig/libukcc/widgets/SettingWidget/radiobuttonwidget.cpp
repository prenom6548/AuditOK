#include "radiobuttonwidget.h"

#include <QDebug>

RadioButtonWidget::RadioButtonWidget(const QString title,  QWidget *parent, UkccFrame::BorderRadiusStyle style)
    : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_buttonGroup = new QButtonGroup(this);
    init();
}

RadioButtonWidget::~RadioButtonWidget()
{
}

void RadioButtonWidget::init()
{
    mainLayout = new QHBoxLayout();
    radioLayout = new QHBoxLayout();
    radioLayout->setContentsMargins(0, 0, 0, 0);
    radioLayout->setSpacing(80);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_settingName->setFixedWidth(214);
    mainLayout->addWidget(m_settingName);
    mainLayout->setSpacing(16);
    mainLayout->addLayout(radioLayout);
    mainLayout->addStretch();
    this->setLayout(mainLayout);
    connect(m_buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &RadioButtonWidget::buttonClicked);
}


void RadioButtonWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
}

QString RadioButtonWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void RadioButtonWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

void RadioButtonWidget::addButton(QRadioButton *button, int id, bool addStretch)
{
    m_buttonGroup->addButton(button, id);
    radioLayout->addWidget(button, 0, Qt::AlignLeft | Qt::AlignVCenter);
    if (addStretch) {
        radioLayout->addStretch();
    }
}

void RadioButtonWidget::setButtonSpacing(int spacing)
{
    radioLayout->setSpacing(spacing);
}

QButtonGroup *RadioButtonWidget::buttonGroup() const
{
    return m_buttonGroup;
}
