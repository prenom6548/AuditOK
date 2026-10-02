/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef FONTINTERFACE_H
#define FONTINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QtDBus>
#include "libfun.h"
#include "external.h"

class FontInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Font")

public:
    FontInterface();
    ~FontInterface();

private:
    void clear();
    bool init();
    bool installFont(QString path);                     /* 安装字体 */
    bool copyFont(QString srcPath , QString dstPath);   /* 复制字体 */
    bool judgeBadFontFile(QString path);   /* 判断字体文件是否损坏 */
    void getCurrentFontInfo();
    QStringList splitFontNameSize(QString value);
    void fontKwinSlot();
    bool isUsingFont(ViewData* viewData);

public Q_SLOTS:
    bool resetDefault();
    QList<ViewData> getViewData();                  /* 获取全部字体 */
    QList<ViewData> getInstallViewData();
    FontInformation getFontDetails(QString path); /* 获取单个字体详细信息 */
    void setFontSize(double size);
    double getFontSize();
    bool installFontList(QVariantList pathList);
    bool applyFont(QString family, QString path, QString style);   /* 应用字体 */
    bool uninstallFont(QStringList pathList, QString fontName); /* 卸载字体 */
    void updateFontList(bool firstLoad = false);                              /* 更新字体列表 */
    void updateFontApplyList();

Q_SIGNALS:
    void updateFontSignal();

private:
     QList<FontInformation> m_fontData;     // 所有字体
     QList<ViewData> m_viewDataList;        // 显示字体列表
     QList<ViewData> m_installViewDataList;
     QString m_installPath;
     External m_external;
     QStringList m_gtkfontStrList;
     QStringList m_docfontStrList;
     QStringList m_monospacefontStrList;
     QGSettings* m_ifsettings = nullptr;
     QGSettings* m_styleSettings = nullptr;
     QGSettings* m_ukccSettings = nullptr;
     bool m_firstLoad = true;
};

#endif // FONTINTERFACE_H
