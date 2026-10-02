/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef FILEFILTERRULES_H
#define FILEFILTERRULES_H

#include <QDialog>
#include <QMutex>
#include <QPoint>
#include <QPushButton>
#include <QShowEvent>

namespace Ui {
class FileFilterRules;
}

class UkuiWindowHelper;

class FileFilterRules : public QDialog
{
    Q_OBJECT
Q_SIGNALS:
    void hidden(bool);
    void broken(bool);
    void replace(bool);
public:
    static FileFilterRules *instance(QWidget *parent = nullptr);
public:
    void init(void *data = nullptr);
    void show();
    void showDlg();
    void updatePosition(QPoint);
    void initCheckBoxState();
public slots:
    void slotclose();
public:
    FileFilterRules(const FileFilterRules*)=delete;
    FileFilterRules *operator =(const FileFilterRules *)=delete;
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
public:
    explicit FileFilterRules(QWidget *parent = nullptr);
    ~FileFilterRules();
private:
    void uiAdjust();

public slots:
    void slotChangeUIFontSize(int fontSize);

public slots:
    void slotscancel();
    void slotsensure();
private:
    static FileFilterRules *m_oInstance;
    static QMutex           m_oMutex;
    QPoint                  m_oPosition;
    QPushButton *m_oEnsureBtn;
    QPushButton *m_oCancelBtn;
    UkuiWindowHelper *m_windowHelper;
    bool m_ishidden;
    bool m_isbroken;
    bool m_isrepleace;

private:
    Ui::FileFilterRules *ui;
};
#endif // FILEFILTERRULES_H
