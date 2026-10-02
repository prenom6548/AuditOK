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
#ifndef MEDIAINFODIALOG_H
#define MEDIAINFODIALOG_H

#include <QDialog>
#include "core/mpvtypes.h"

#define IconFixedSize QSize(24,24)

namespace Ui {
class MediaInfoDialog;
}

class MediaInfoDialog : public QDialog
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明

public:
    explicit MediaInfoDialog(QWidget *parent = nullptr);
    ~MediaInfoDialog();

    void setData(QString data);
    void setTitle(QString s)    {m_title = s;}
    void setType(QString s)     {m_type = s;}
    void setSize(QString s)     {m_size = s;}
    void setDuration(QString s) {m_duration = s;}
    void setPath(QString s)     {m_filePath = s;}
    void updateDate();

private slots:
    void on_btn_ok_clicked();
    void gSettingChanged(QString key);

private:
    Ui::MediaInfoDialog *ui;
    QString m_title,
            m_type,
            m_size,
            m_duration,
            m_filePath;
    QString ProcessText(const QFontMetrics& font, const QString& text, int nLabelSize); //处理较长字符串，实现在指定位置处换行
    int findMaxSize();            //找出文件相关的最长属性名

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // MEDIAINFODIALOG_H
