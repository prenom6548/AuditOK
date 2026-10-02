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
#ifndef PROPERTY_ITEMWIDGET_H
#define PROPERTY_ITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>

#include "custom_push_button.h"
#include "size_para.h"
#include "ukui_printer.h"

class CustomLabel;

enum class PropertyBoxStyle : int {
    TEXT_WITH_LINEEDIT_EDITABLE = 0,
    TEXT_WITH_LINEEDIT_UNEDITABLE,
    TEXT_WITH_LINEEDIT_AND_BUTTON,
    TEXT_WITH_COMBOBOX,
    TEXT_WITH_SWITCH,
    LABEL_WITH_STATE,
};

const QMap<QString, PropertyBoxStyle> g_propertyList {
    {"Name",           PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE},
    {"Location",       PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE},
    {"Status",         PropertyBoxStyle::LABEL_WITH_STATE},
    {"Driver",         PropertyBoxStyle::TEXT_WITH_LINEEDIT_AND_BUTTON},
    {"URI",            PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE},
    {"Resolution",     PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"PageSize",       PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"Source",         PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"PageType",       PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"Color",          PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"Direction",      PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"Duplex",         PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"Pages per Side", PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"Ink Rank",       PropertyBoxStyle::TEXT_WITH_COMBOBOX},
    {"Output Order",   PropertyBoxStyle::TEXT_WITH_COMBOBOX},
//  {"Scale To Fit Size:", PropertyBoxStyle::TEXT_WITH_SWITCH},
};

class ProperytItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProperytItemWidget(QString propertyName,
                        PropertyBoxStyle style,
                        QWidget *parent = nullptr);

    explicit ProperytItemWidget(QString propertyName,
                        PropertyBoxStyle style,
                        AttrName propertyValue,
                        QWidget *parent = nullptr);

    explicit ProperytItemWidget(QString propertyName,
                        PropertyBoxStyle style,
                        AttrName propertyValue,
                        QList<AttrName> propertyList,
                        QWidget *parent = nullptr);

    //设置第一项标题的内容
    void setfirstTitleText(const QString &content);

    void setSecondProperty(const QString &content);

    void setSecondProperty(const AttrName &content, const QList<AttrName> &list);

    //设置第三项按钮文字
    void setThirdButtonText(const QString &content);
    void showWarning(const QString &what);
    bool isPropertyChanged();
    QString getChangedProperty();
    QString getProperty();
    QString getSecondComboBoxContent();

signals:
    void thirdOperateButtonClicked();
    void showProperyChanged(QString, QString);

private:
    //布局
    void initLayout();
    void init();
    void initFirst();
    void initSecond();
    void initThird();

private:
    //设置第二项label需要显示的icon
    void setSecondLabelIcon(const QString &content);
    PropertyBoxStyle m_style = PropertyBoxStyle::TEXT_WITH_LINEEDIT_EDITABLE;
    QString m_name = QString();
    AttrName m_value;
    QList<AttrName> m_valueList;

    CustomLabel *m_firstTitle = nullptr;
    
    QPushButton *m_secondIcon = nullptr;
    QLabel *m_secondLabel = nullptr;
    QLineEdit *m_secondLineEdit = nullptr;
    QComboBox *m_secondComboBox = nullptr;

    QPushButton *m_warningIcon = nullptr;
    CustomPushButton *m_thirdOperateButton = nullptr;

    QHBoxLayout *m_mainLayout = nullptr;

signals:
    //第三项按钮点击信号
    void thirdButtonClicked();
};

#endif // PROPERTY_ITEMWIDGET_H
