/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
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

#ifndef ABOUT_H
#define ABOUT_H

#include <QWidget>
#include <QScrollArea>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

class About : public QDialog
{
    Q_OBJECT
public:
    explicit About(const QString &icon, QWidget *parent = nullptr);
    ~About();

private:
    void initUI();
    void initConnection();
    QString getKmreVersion();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QLabel *m_centerIcon = nullptr;
    QLabel *m_centerTitleLabel = nullptr;
    QLabel *m_versionLabel = nullptr;
    QLabel *m_developerLabel = nullptr;
    QPushButton *m_developerEmailBtn = nullptr;
    QTextEdit *m_detailTextEdit = nullptr;
    QHBoxLayout *m_centerIconLayout = nullptr;
    QHBoxLayout *m_centerTitleLayout = nullptr;
    QHBoxLayout *m_centerVersionLayout = nullptr;
    QHBoxLayout *m_detailLayout = nullptr;
    QHBoxLayout *m_developerLayout = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;

private:
    QString m_icon;
};

//class About : public QScrollArea
//{
//    Q_OBJECT
//public:
//    explicit About(const QString &icon, QWidget *parent = nullptr);

//    QString link(const QString &url, QString name = "");

//};

#endif // ABOUT_H
