#include "lineeditwidget.h"

#include <QDebug>

LineEditWidget::LineEditWidget(const QString title, QWidget *parent, UkccFrame::BorderRadiusStyle style)
    : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_lineEdit = new QLineEdit(this);
    init();
}

LineEditWidget::~LineEditWidget()
{
}

void LineEditWidget::init()
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_settingName->setFixedWidth(214);
    mainLayout->addWidget(m_settingName);
    mainLayout->setSpacing(16);
    mainLayout->addWidget(m_lineEdit);
    this->setLayout(mainLayout);
}

void LineEditWidget::setEnabled(const bool checked)
{
    if (m_lineEdit) {
        m_lineEdit->setEnabled(checked);
    }
}

bool LineEditWidget::isEnabled() const
{
    return m_lineEdit->isEnabled();
}

void LineEditWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
    m_lineEdit->setAccessibleName(title);
}

QString LineEditWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void LineEditWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

QLineEdit *LineEditWidget::lineEdit()
{
    return m_lineEdit;
}
