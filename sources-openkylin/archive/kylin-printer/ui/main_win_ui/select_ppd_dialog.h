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
#ifndef SELECT_PPD_DIALOG_H
#define SELECT_PPD_DIALOG_H

#include "custom_widget.h"
#include <QStandardItemModel>
#include "findppdsthread.h"

#ifdef USE_SELF_CONTAINED_DEP
    #include "ksearchlineedit.h"
#else
    #include <kysdk/applications/ksearchlineedit.h>
#endif // USE_SELF_CONTAINED_DEP

#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QCompleter>

QT_BEGIN_NAMESPACE
namespace Ui { class SelectPpdDialog; }
QT_END_NAMESPACE

class SelectPpdDialog : public CustomWidget
{
    Q_OBJECT

public:
    explicit SelectPpdDialog(QWidget *parentWindow = nullptr,QDialog *parent = nullptr);
    ~SelectPpdDialog();
    void onShow();
signals:
    void mapIsReady();
    void chosenPPDName(QString ppd);
    void userCanceled();
    void ppdUpdateFinished();
public slots:
    void updateTableView();

private slots:
    void venderListClicked(const QModelIndex &index);
    void modelListClicked(const QModelIndex &index);
//    void venderListShowToolTip(const QModelIndex &index);
    void modelListShowToolTip(const QModelIndex &index);
    void sendOutPPDName();
    void cancelAndClose();
    void startSearch();
    void clearSearch();
    void searchClicked();
private:
    Ui::SelectPpdDialog *m_ui;

    QStandardItemModel *m_vendorModal;
    QStandardItemModel *m_modelModal;

    QSortFilterProxyModel *m_proxyVendor;
    QSortFilterProxyModel *m_proxyModel;

    ppdPrinterMap *m_ppdMap;
    QMap<QString,PPDsAndAttr> m_currentVendor;
    bool m_updateFailed;

    QString m_currentChoosenPPD;
    QString m_currentChoosenModel;

    QWidget *m_pParentWindow;

    QComboBox *m_comboBox;
    kdk::KSearchLineEdit* m_searchLineEdit;

    QCompleter *m_completer;

    QFont m_font;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // SELECT_PPD_DIALOG_H
