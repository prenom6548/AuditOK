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


#ifndef MODALDIALOG_H
#define MODALDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QPoint>

namespace Ui {
class ModalDialog;
}

class UkuiWindowHelper;
class ModalDialog : public QDialog
{
    Q_OBJECT
public:
    enum Pages
    {
        Progress = 0,
        Tip,
        BusnessSetting,
        DataSetting,
        DiskCheck
    };

public:
    static ModalDialog *instance(QWidget *parent = nullptr);
public:
    ModalDialog(const ModalDialog*)=delete;
    ModalDialog *operator =(const ModalDialog*)=delete;

public slots:
    QWidget *showPage(int);
    QWidget *getPageByIndex(int index);
    void cancelEnable(bool);
    void updatePosition(QPoint);
    void show();
    void wexec();
    bool canBeClose() { return m_bCanClose; }

    void slotChangeUIFontSize(int fontSize);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    explicit ModalDialog(QWidget *parent = nullptr);
    ~ModalDialog();

private slots:
    void slotsTitle(QString);
    void slotsClose();

private:
    static ModalDialog *m_oInstance;
    static QMutex       m_oMutex;

private:
    bool                m_bCanClose;
    bool                m_isprogress;
    QPoint              m_oMainPos;
    QPushButton         *m_oCancelBtn;
    QPushButton         *m_oEnsureBtn;
    UkuiWindowHelper    *m_windowHelper;

private:
    Ui::ModalDialog    *ui;
Q_SIGNALS:
    void cancel();
};

#define MODAL ModalDialog::instance()
#define PROGRESS MODAL->showPage(ModalDialog::Progress)
#define BUSNESSSETTING MODAL->getPageByIndex(ModalDialog::BusnessSetting)

#endif // MODALDIALOG_H
