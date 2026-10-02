/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CMESSAGEBOX_H
#define CMESSAGEBOX_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <ukuistylehelper/ukuistylehelper.h>

class CMessageBox : public QDialog
{
    Q_OBJECT

public:
    explicit CMessageBox(QWidget *parent = nullptr);
    ~CMessageBox();

    void setText(const QString &text);

private:
    void init();
    void resetText();

private:
    QWidget *m_widget = nullptr;
    QVBoxLayout *m_vLayout = nullptr;
    QPushButton *m_btnClose = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_textLabel = nullptr;
    QHBoxLayout *m_labelLayout = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    QHBoxLayout *m_btnLayout = nullptr;
    QString m_tip;


};

#endif // CMESSAGEBOX_H
