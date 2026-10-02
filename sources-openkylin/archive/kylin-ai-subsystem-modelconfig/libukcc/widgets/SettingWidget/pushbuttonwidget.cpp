#include "pushbuttonwidget.h"

#include <QDebug>

PushButtonWidget::PushButtonWidget(const QString title, QWidget *parent, UkccFrame::BorderRadiusStyle style)
   : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_pushButton = new QPushButton(this);
    m_iconLabel = new QLabel(this);
    init();
}

PushButtonWidget::~PushButtonWidget()
{

}

void PushButtonWidget::init()
{
    m_iconLabel->setVisible(false);
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_pushButton->setFixedWidth(100);
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addWidget(m_settingName);
    mainLayout->addStretch();
    mainLayout->addWidget(m_pushButton);
    this->setLayout(mainLayout);

    connect(m_pushButton, &QPushButton::clicked, this, &PushButtonWidget::clicked);
}

void PushButtonWidget::setEnabled(const bool checked)
{
    if (m_pushButton) {
        m_pushButton->setEnabled(checked);
    }
}

bool PushButtonWidget::isEnabled() const
{
    return m_pushButton->isEnabled();
}

void PushButtonWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
    m_pushButton->setAccessibleName(title);
}

QString PushButtonWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void PushButtonWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

void PushButtonWidget::setButtonText(const QString &text)
{
    m_pushButton->setText(text);
}

void PushButtonWidget::setButtonFixedWidth(int w)
{
     m_pushButton->setFixedWidth(w);
}

QPushButton *PushButtonWidget::pushButton()
{
    return m_pushButton;
}

void PushButtonWidget::setIconLabel(QIcon icon)
{
    m_iconLabel->setPixmap(icon.pixmap(16, 16));
    m_iconLabel->setVisible(true);
}
