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

#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>

class ThumbnailManagerPrivate;

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = nullptr);
    ~ThumbnailManager();

signals:
    void thumbnailFinished(const QString &path, const QImage &image, const QString &errorMessage);

public slots:
    Q_INVOKABLE void startPreview(const QString &path);

    /*!
     * \brief setStrictedMode
     * \param strictedMode
     * \note
     * send thumbnailFinished() signal only the last thumbnail job done.
     */
    Q_INVOKABLE void setStrictedMode(bool strictedMode);

private:
    ThumbnailManagerPrivate *priv;
};

#endif // THUMBNAILMANAGER_H
