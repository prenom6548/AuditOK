/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOADWIDGET_H
#define LOADWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QFont>
#include <QPalette>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QDebug>
#include <QKeyEvent>
#include <qgsettings.h>

class LoadWidget : public QDialog
{
    Q_OBJECT
public:
    explicit LoadWidget(QWidget *parent = nullptr);
    explicit LoadWidget(QString devStatus, QWidget *parent = nullptr);
    ~LoadWidget();

    void initUI();
    // void initUI(QString *devStatus);
    void setAppIcon(const QString &text);
    void setAppName(const QString &text);
    void setAppVersion(const QString &text);
    void setAppDescription(const QString &text);
protected:
//    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event);

private:
    void initThemeMode();

private:
    QString m_status;
    QVBoxLayout *main_layout = nullptr;

    QLabel *title_icon = nullptr;
    QLabel *title_name = nullptr;
    QPushButton *close_btn = nullptr;

    QLabel *app_icon = nullptr;
    QLabel *app_name = nullptr;
    QLabel *app_version = nullptr;
    QTextBrowser *app_description = nullptr;

    QGSettings *qtSettings = nullptr;
};

#endif // LOADWIDGET_H
