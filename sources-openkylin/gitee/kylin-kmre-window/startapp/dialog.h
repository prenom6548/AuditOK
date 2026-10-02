/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QPoint>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QPushButton>
#include <QProgressBar>

class Worker;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(const QString &tip, const QString &addition = "", QWidget *parent = 0);
    ~Dialog();

    void showAnimation();
    void doWork();
    void setFixedPosition();
//    void showWindow();

private slots:
    void closeAnimation();
    void clearAll();
    void onTimerOut();

public slots:
    void onImageLoaded();
    void onShutDown(QString container);
    void onAndroidRunCompleted();
    void onFinishApp();
    void onAnimationOver();
    void onAppOver();

    void onServiceStartFailed(QString name);
    void onServiceNotFound(QString name);
    void onImageConfNotFound();
    void onImageFileNotFound();
    void onImageLoadFailed();
    void onImageNotMatched();

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

signals:
    void sendMessage();
    void animaTimeOut();

private:
    Ui::Dialog *ui;
    QDesktopWidget desktop;
    QPropertyAnimation* animation;
    QString m_tipMSg;
    QString m_additionMsg;
    Worker *m_worker;
    QThread *m_thread = nullptr;
    QTimer *m_timer;
    int m_progressValue;
    QProgressBar *progressBar = nullptr;
    QLabel *logo_label = nullptr;
    QLabel *text_label = nullptr;
};

#endif // DIALOG_H
