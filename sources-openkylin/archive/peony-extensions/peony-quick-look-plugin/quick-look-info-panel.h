/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef QUICK_LOOK_INFO_PANEL_H
#define QUICK_LOOK_INFO_PANEL_H

#include <QWidget>

class QLabel;

class QuickLookInfoPanel : public QWidget
{
    Q_OBJECT

public:
    explicit QuickLookInfoPanel(QWidget *parent = nullptr);

    void setFolderInfo(const QString &uri);
    void setFileInfo(const QString &uri);
    void clearPanel();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateIconAndName(const QString &uri);
    void startFolderCount(const QString &uri);
    void updateTextWidths();

    QWidget *m_contentWidget = nullptr;
    QLabel *m_icon = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_metaLabel = nullptr;
    QString m_uri;
};

#endif
