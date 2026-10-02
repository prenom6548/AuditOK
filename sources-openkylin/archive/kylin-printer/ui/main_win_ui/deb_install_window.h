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
#ifndef DEB_INSTALL_WINDOW_H
#define DEB_INSTALL_WINDOW_H

#include <QWidget>

#ifdef USE_SELF_CONTAINED_DEP
    #include "kdialog.h"
#else
    #include <kysdk/applications/kdialog.h>
#endif // USE_SELF_CONTAINED_DEP

class SelectPpdDialog;

namespace Ui {
class DebInstallWindow;
}

class DebInstallWindow : public kdk::KDialog
{
    Q_OBJECT

public:
    explicit DebInstallWindow(QWidget *parent = nullptr);
    ~DebInstallWindow();
    void onShow(const QString &ppdName);
signals:
    void installEnd();
    void modifyPPDName(QString);
    void getCurFontSize();
private slots:
    void onCancelBtnClicked();
    void onForwardBtnClicked();

private:
    QString m_oldPpdName{QString()};
    Ui::DebInstallWindow *m_ui;
    //安装状态，0：安装成功，1：安装失败，2：已经安装
    int m_installOk = -1;
    //父对象中心位置
    QWidget* m_parentWidget;
    SelectPpdDialog* m_pSelectPpdDialog = nullptr;
    QFont m_font14;
};

#endif // DEB_INSTALL_WINDOW_H
