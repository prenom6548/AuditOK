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


#ifndef TIP_BURNRESULT_H
#define TIP_BURNRESULT_H

#include <QDialog>
#include <QMutex>
#include <QAbstractButton>

namespace Ui {
class Tip_BurnResult;
}

class UkuiWindowHelper;
class Tip_BurnResult : public QDialog
{
    Q_OBJECT

public:
    explicit Tip_BurnResult(QWidget *parent = nullptr);
    ~Tip_BurnResult();
    static Tip_BurnResult *instance(QWidget *parent = nullptr);
    void updatePosition(QPoint);

Q_SIGNALS:
    void success(QString);
    void fail(QString);

public slots:
    void slotsSuccess(QString);
    void slotsFailed(QString);
    void slotsUmountFailed(QString);
    void slotsWarn(QString);
    void slotsTitle(QString);
    void slotsClose();
    void slotChangeUIFontSize(int fontSize);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::Tip_BurnResult *ui;
private:
    static Tip_BurnResult *m_oInstance;
    UkuiWindowHelper *m_windowHelper;
    static QMutex       m_oMutex;
};
#define TIP_BURNRESULT Tip_BurnResult::instance()
#endif // TIP_BURNRESULT_H
