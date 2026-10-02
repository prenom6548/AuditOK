/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef SAVEPATHWIDGET_H
#define SAVEPATHWIDGET_H
#include <kaboutdialog.h>
#include "../fileProcess/fileprocess.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
class SavePathDialog : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit SavePathDialog(QWidget *parent = nullptr);
    QString getSavePath();
    void updateLabelShow(QString savepath);
signals:
    void savePathUpdate(const QString& filePath);
public slots:
    void onModifyBtnClicked();


private:
    QLabel *m_pathLabel;
    QPushButton *m_modifyBtn;
    QPushButton *m_confirmBtn;
    QString m_savepath;
};

#endif // SAVEPATHWIDGET_H
