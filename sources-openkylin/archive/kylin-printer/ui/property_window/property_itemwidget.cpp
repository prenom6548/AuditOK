/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QLineEdit>

#include "custom_label.h"
#include "property_itemwidget.h"
#include "information_collector.h"

ProperytItemWidget::ProperytItemWidget(QString propertyName,
                       PropertyBoxStyle style,
                       QWidget *parent): 
    m_name(propertyName),
    m_style(style),
    QWidget(parent)
{
    init();
}

ProperytItemWidget::ProperytItemWidget(QString propertyName,
                       PropertyBoxStyle style,
                       AttrName propertyValue,
                       QWidget *parent): 
    m_name(propertyName),
    m_style(style),
    m_value(propertyValue),
    QWidget(parent)
{
    init();
}

ProperytItemWidget::ProperytItemWidget(QString propertyName,
                       PropertyBoxStyle style,
                       AttrName propertyValue,
                       QList<AttrName> propertyList,
                       QWidget *parent): 
    m_name(propertyName),
    m_style(style),
    m_value(propertyValue),
    m_valueList(propertyList),
    QWidget(parent)
{
    init();
}

void ProperytItemWidget::init()
{
    setFixedHeight(36);
    initLayout();
    initFirst();
    initSecond();
    initThird();
    setLayout(m_mainLayout);
    m_warningIcon->hide();
}

void ProperytItemWidget::showWarning(const QString &what)
{
    if (what.isEmpty()) {
        m_warningIcon->hide();
    }
    else {
        m_warningIcon->show();
        m_warningIcon->setToolTip(what);
    }
}

void ProperytItemWidget::initLayout()
{
    setContentsMargins(0,0,0,0);
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
}

void ProperytItemWidget::initFirst()
{
    m_firstTitle = new CustomLabel(this);
    m_firstTitle->setFixedSize(142, 36);
    setfirstTitleText(m_name);
    m_mainLayout->addWidget(m_firstTitle);
}

void ProperytItemWidget::initSecond()
{
    switch (m_style)
    {
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE :
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE :
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_AND_BUTTON :
        m_secondLineEdit = new QLineEdit(this);
        m_secondLineEdit->setFixedSize(300, 36);
        if (m_style != PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE) {
            m_secondLineEdit->setReadOnly(true);
        }
        m_mainLayout->addWidget(m_secondLineEdit);
        break;
    case PropertyBoxStyle::TEXT_WITH_COMBOBOX :
        m_secondComboBox = new QComboBox(this);
        connect(m_secondComboBox, &QComboBox::currentTextChanged, this, [=](const QString &text){
            qDebug() << "Property" << objectName() << "changed to " << getSecondComboBoxContent();
            InformationCollector::getInstance().addMessage(QString("Property %1 changed to %2").arg(objectName()).arg(getSecondComboBoxContent()));
            emit showProperyChanged(objectName(), getSecondComboBoxContent());
        });
        m_secondComboBox->setFixedSize(300, 36);
        m_mainLayout->addWidget(m_secondComboBox);
        break;
    case PropertyBoxStyle::LABEL_WITH_STATE :
        m_secondIcon = new QPushButton(this);
        m_secondIcon->setFixedSize(20,20);
        m_secondIcon->setFlat(true);
        m_secondIcon->setStyleSheet("border-radius:4px;");
        m_mainLayout->addWidget(m_secondIcon);
        m_secondLabel = new QLabel(this);
        m_secondLabel->setFixedSize(240,36);
        m_mainLayout->addWidget(m_secondLabel);
        break;
    default:
        break;
    }
}

void ProperytItemWidget::initThird()
{
    if (m_style == PropertyBoxStyle::TEXT_WITH_LINEEDIT_AND_BUTTON) {
        m_mainLayout->addSpacing(16);
        m_thirdOperateButton = new CustomPushButton(this);
        m_thirdOperateButton->setFixedSize(96, 36);
        m_mainLayout->addWidget(m_thirdOperateButton);
    }
    connect(m_thirdOperateButton, &QPushButton::clicked, this, &ProperytItemWidget::thirdOperateButtonClicked);
    m_mainLayout->addSpacing(8);
    m_warningIcon = new QPushButton(this);
    m_warningIcon->setIcon(QIcon::fromTheme("dialog-warning"));
    m_warningIcon->setFlat(true);
    m_warningIcon->setStyleSheet("border-radius:4px;");
    m_mainLayout->addWidget(m_warningIcon);
    m_mainLayout->addStretch();
    return ;
}

void ProperytItemWidget::setfirstTitleText(const QString &content)
{
    if (content.isEmpty()) {
        return ;
    }
    m_firstTitle->setText(content);
    m_name = content;
}

void ProperytItemWidget::setSecondProperty(const QString &content)
{
    if (content.isEmpty() || m_style == PropertyBoxStyle::TEXT_WITH_COMBOBOX) {
        return ;
    }

    switch (m_style)
    {
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE :
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE :
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_AND_BUTTON :
        m_secondLineEdit->setText(content);
        break;
    case PropertyBoxStyle::LABEL_WITH_STATE :
        m_secondLabel->setText(content);
        setSecondLabelIcon(content);
        break;
    default:
        break;
    }
    m_value.displayName = content;
    m_value.name = content;
}

QString ProperytItemWidget::getSecondComboBoxContent()
{
    QString realStr = QString();
    for(auto it : m_valueList) {
        if (it.displayName == m_secondComboBox->currentText()) {
            realStr = it.name;
            break;
        }
    }
    return realStr;
}

void ProperytItemWidget::setSecondProperty(const AttrName &content, const QList<AttrName> &list)
{
    if (content.name.isEmpty() || content.displayName.isEmpty() || list.size() < 1) {
        return ;
    }

    if (m_style != PropertyBoxStyle::TEXT_WITH_COMBOBOX) {
        return ;
    }

    int idx = -1;

    for (int i = 0; i < list.length(); ++i) {
        if (content == list.at(i)) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        return ;
    }

    for (int i = 0; i < list.length(); ++i) {
        m_secondComboBox->addItem(list.at(i).displayName);
    }
    m_value = content;
    m_valueList = list;
    m_secondComboBox->setCurrentIndex(idx);
}

void ProperytItemWidget::setSecondLabelIcon(const QString &content)
{
    if (content == tr("Idle")) {
        m_secondIcon->setIcon(QIcon(":/pic/green.svg"));
    }
    else if (content == tr("Printing")) {
        m_secondIcon->setIcon(QIcon(":/pic/orange.svg"));
    }
    else if (content == tr("Stopped")) {
        m_secondIcon->setIcon(QIcon(":/pic/red.svg"));
    }
    else if (content == tr("Unknow")) {
        m_secondIcon->setIcon(QIcon(":/pic/gray.svg"));
    }
    m_secondIcon->setIconSize(QSize(8,8));
    return ;
}

void ProperytItemWidget::setThirdButtonText(const QString &content)
{
    m_thirdOperateButton->setText(content);
}

bool ProperytItemWidget::isPropertyChanged()
{
    switch (m_style)
    {
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE:
        if (m_value.displayName == m_secondLineEdit->text()) {
            return false;
        }
        return true;
        break;
    case PropertyBoxStyle::TEXT_WITH_COMBOBOX:
        if (m_value.displayName == m_secondComboBox->currentText()) {
            return false;
        }
        return true;
        break;
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE:
    case PropertyBoxStyle::LABEL_WITH_STATE:
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_AND_BUTTON:
        return false;
        break;
    
    default:
        break;
    }
    return false;
}

QString ProperytItemWidget::getChangedProperty()
{
    if (isPropertyChanged() == false) {
        return QString();
    }

    return getProperty();
}

QString ProperytItemWidget::getProperty()
{
    switch (m_style)
    {
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_AND_BUTTON:
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE:
    case PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE:
        return m_secondLineEdit->text();
        break;
    case PropertyBoxStyle::TEXT_WITH_COMBOBOX:
        return m_secondComboBox->currentText();
        break;
    case PropertyBoxStyle::LABEL_WITH_STATE:
        return m_secondLabel->text();
        break;
    default:
        break;
    }
    return QString();
}