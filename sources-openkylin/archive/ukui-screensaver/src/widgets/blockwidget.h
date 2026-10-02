/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef BLOCKWIDGET_H
#define BLOCKWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QPainter>
#include "customlabel.h"
#include "../common/windowdata.h"

class QPushButton;
class QListView;
class LockDialogModel;
class QTimer;

class SimpleDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SimpleDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

class BlockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BlockWidget(LockDialogModel *model, QWidget *parent = nullptr);
    ~BlockWidget();
    void setTips(const QString tips, int type);
    void setWarning(QVector<InhibitorInfo> list, int type);
    void setWarning(QVector<InhibitorInfo> list, QVector<WindowInfo::WindowInfo> &windows, int type);
    void updateFontFamily(QString fontFamily);
    void setConfirmButton(bool isHidden);

    /**
     * @brief 设置关机倒计时
     * @param type   0关机，1重启
     * @return
     */
    void setMsgTips(int type);

private:
    void initUi();
    QString getElidedText(QFont font, int width, QString strInfo);
    void adjustButtonText();

private Q_SLOTS:
    void onFontSizeChanged(double fontSize);
    void hideEvent(QHideEvent *event);
    void keyPressEvent(QKeyEvent *);
Q_SIGNALS:
    void cancelButtonclicked();
    void confirmButtonclicked();

private:
    CustomLabel *m_tipLabel1 = nullptr;
    CustomLabel *m_tipLabel2 = nullptr;
    CustomLabel *m_msgTipLabel = nullptr;
    QListView *m_listView = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_confirmButton = nullptr;
    LockDialogModel *m_modelLockDialog = nullptr;

    bool m_multiUsers = false;
    double m_curFontSize;
    double m_ptToPx = 1.0;
    QFont sysFont;

    QTimer *shutdownTimer = nullptr;
    QDateTime lastDateTime;

    int msg_type;
    QString getHibited_tr_lowcase(int type);
    int m_type = 0;
};

#endif // BLOCKWIDGET_H
