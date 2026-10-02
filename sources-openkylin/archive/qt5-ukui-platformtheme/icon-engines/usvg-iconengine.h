/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2025, KylinSoft Co., Ltd.
** 
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef USVGICONENGINE_H
#define USVGICONENGINE_H

#include<QIconEngine>
#include<QWidget>
#include<QString>


class USvgIconEnginePrivate;
class USvgIconEngine : public QIconEngine
{
private:
   USvgIconEnginePrivate* d_ptr;
public:
   USvgIconEngine();
   ~USvgIconEngine();
   USvgIconEngine(const USvgIconEngine &other);//拷贝构造

public:
   void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override;
   void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override;

   QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
   
   QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QString key() const override;
   QIconEngine *clone() const override;

   bool read(QDataStream &in) override;
   bool write(QDataStream &out) const override;

   void virtual_hook(int id, void *data) override;
public:
   //一些辅助函数
   const QString getBaseSvg() const;
   void printPixmapCache(const QHash<int, QPixmap>* pixmapCache);
   void printSvgCache(const QHash<int, QString>* svgCache);
   const QHash<int,QString>* getSvgCache() const;
   void printPcmCache();
};

#endif
