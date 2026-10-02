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

class KLabel;
class QPushButton;
class QListView;
class LockDialogModel;
class QTimer;

class BlockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BlockWidget(LockDialogModel *model, QWidget *parent = nullptr);
    ~BlockWidget();
    void setTips(const QString tips);
    void setWarning(QStringList list, int type);
    void updateFontFamily(QString fontFamily);

    /**
     * @brief 设置关机倒计时
     * @param type   0关机，1重启
     * @return
     */
    void setMsgTips(int type);

private:
    void initUi();

private Q_SLOTS:
    void onFontSizeChanged(double fontSize);
    void hideEvent(QHideEvent *event);
Q_SIGNALS:
    void cancelButtonclicked();
    void confirmButtonclicked();

private:
    KLabel *m_tipLabel = nullptr;
    KLabel *m_msgTipLabel = nullptr;
    QListView *m_listView = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_confirmButton = nullptr;
    LockDialogModel *m_modelLockDialog = nullptr;

    int m_blockType = 0;
    bool m_multiUsers = false;
    double m_curFontSize;
    double m_ptToPx = 1.0;
    QFont sysFont;

    QTimer *shutdownTimer = nullptr;
    QDateTime lastDateTime;

    int msg_type;
    QString getHibited_tr_lowcase(int type);
};

#endif // BLOCKWIDGET_H
