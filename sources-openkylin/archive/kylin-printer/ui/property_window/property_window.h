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
#ifndef __PROPERTY_WINDOW_H__
#define __PROPERTY_WINDOW_H__

#include <QHBoxLayout>

#ifdef USE_SELF_CONTAINED_DEP
    #include "knavigationbar.h"
#else
    #include <kysdk/applications/knavigationbar.h>
#endif // USE_SELF_CONTAINED_DEP

#include "custom_widget.h"
#include "ukui_printer.h"
#include "size_para.h"
#include "property_list_window.h"
#include "property_itemwidget.h"
#include "select_ppd_dialog.h"

class PropertyWindow : public CustomWidget
{
    Q_OBJECT
public:
    explicit PropertyWindow(const UkuiUsbPrinter &printerInfo, QWidget *parent = nullptr);
    ~PropertyWindow();

    void onShow();

private slots:
    void updatePPD(QString text);
    void selectPPDDialogShow();
    void checkOptionConflicts(QString optionName, QString curVal);

private:
    std::unique_ptr<QHBoxLayout> m_leftLayout{nullptr};
    std::unique_ptr<QHBoxLayout> m_rightLayout{nullptr};
    std::unique_ptr<kdk::KNavigationBar> m_navigationBar{nullptr};
    std::unique_ptr<PropertyListWindow> m_propertyListWindow{nullptr};
    PropertyCupsInfo cupsinfo;

    UkuiUsbPrinter m_currentPrinterInfo;
    bool needSaveDriver = false;

    void initUi();
    void initConnect();

    void addTab(const QString &name, PropertyListWindow::PAGE page);
    void setShowPage(const QModelIndex &index);

    void doSave();
    void insertItemListLayout();
    void initBasePage();
    void initAdvancedPage();
    void initInkPage();
    void initKwrePage();

    void updateItemListLayout(const UkuiUsbPrinter& printerInfo);
    void setAllConflicts(QString option, QMap<QString, QString> conflicts);
    void getZhPropertyfromEn(SelectableProperty& enProperty);
    std::unique_ptr<SelectPpdDialog> m_pSelectPPDDialog{nullptr};


    ProperytItemWidget *createItem(int index, PropertyBoxStyle style, const QString &propertyName);
    ProperytItemWidget *findItem(const QString &propertyName);

    std::unique_ptr<QMap<QString, ProperytItemWidget *> > m_itemMap{nullptr};
};
#endif // __PROPERTY_WINDOW_H__
