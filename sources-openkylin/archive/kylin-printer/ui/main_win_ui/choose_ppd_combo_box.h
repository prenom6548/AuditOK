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
#ifndef __CHOOSE_PPD_COMBO_BOX_H__
#define __CHOOSE_PPD_COMBO_BOX_H__

#include "custom_combo_box.h"
#include "select_ppd_dialog.h"


class ChoosePpdComboBox : public CustomComboBox
{
    Q_OBJECT
public:
    ChoosePpdComboBox(QWidget *parent = nullptr);
    ~ChoosePpdComboBox();

private:
    void onChooseClicked();
    void showSelectPpdDialog();
    QString m_retPpdName{QString()};
    //安装状态，0：安装成功，1：安装失败，2：已经安装
    int m_installOk{-1};
    std::unique_ptr<SelectPpdDialog> m_pSelectPPDDialog{nullptr};
private slots:
    void updatePPD(QString text);
signals:
    void currentPpdChanged(bool isDefault, const QString &ppdName);
    void installEnd();
    void receivePPD();
protected:
    void showPopup();
    void hidePopup();
};
#endif // __CHOOSE_PPD_COMBO_BOX_H__
