/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILEOPERATIONMANAGER_H
#define FILEOPERATIONMANAGER_H

#include <QObject>

#include "../../model/fileitemproxysortfiltermodel.h"

class QWindow;

class FileOperationManager : public QObject
{
    Q_OBJECT
public:
    static FileOperationManager *globalInstance();
    explicit FileOperationManager(QObject *parent = nullptr);

    static QString getNewSpaceName(const QString &currentSpaceName, QWindow *parentWindow = nullptr, bool *ok = nullptr);

    void setRootWindow(QWindow *newRootWindow);

signals:

public slots:
    void showMenu(FileItemProxySortFilterModel *model, int index, const QPoint &globalPos);
    void openFile(const QUrl &url);

private:
    QWindow *rootWindow = nullptr;
};

#endif // FILEOPERATIONMANAGER_H
