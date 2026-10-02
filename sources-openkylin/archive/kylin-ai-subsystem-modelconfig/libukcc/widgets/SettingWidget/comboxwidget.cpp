#include "comboxwidget.h"

#include <QDebug>

ComboxWidget::ComboxWidget(const QString title, QWidget *parent, UkccFrame::BorderRadiusStyle style)
    : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_combox = new QComboBox(this);
    init();
}

ComboxWidget::~ComboxWidget()
{
}

void ComboxWidget::init()
{
    mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_settingName->setFixedWidth(214);
    mainLayout->addWidget(m_settingName);
    mainLayout->setSpacing(16);
    mainLayout->addWidget(m_combox);
    this->setLayout(mainLayout);
    connect(m_combox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComboxWidget::currentIndexChanged);
    connect(m_combox, &QComboBox::currentTextChanged, this, &ComboxWidget::currentTextChanged);
}

void ComboxWidget::setEnabled(const bool checked)
{
    if (m_combox) {
        m_combox->setEnabled(checked);
    }
}

bool ComboxWidget::isEnabled() const
{
    return m_combox->isEnabled();
}

void ComboxWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
    m_combox->setAccessibleName(title);
}

QString ComboxWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void ComboxWidget::setCurrentText(const QString &text)
{
    m_combox->setCurrentText(text);
}

void ComboxWidget::setCurrentIndex(int index)
{
    m_combox->setCurrentIndex(index);
}

void ComboxWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

QComboBox *ComboxWidget::comboBox() const
{
    return m_combox;
}


