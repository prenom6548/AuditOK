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
#ifndef POPUPFONTINFO_H
#define POPUPFONTINFO_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QGridLayout>
#include <QPushButton>
#include <QDBusInterface>
#include "libfun.h"
#include "lightlabel.h"
#include "kdialog.h"
#include "kpushbutton.h"
#include "klabel.h"
#include <qcombobox.h>
#include "kbackgroundgroup.h"
#include "settinggroup.h"
#include <QMessageBox>

using namespace kdk;

/* 字体信息弹窗 */
class PopupFontInfo : public KDialog
{
    Q_OBJECT

public:
    PopupFontInfo(ViewData* viewData, QWidget *parent = nullptr);
    ~PopupFontInfo();
    void setWidegtUi();
    void setWidLayout();

private:
    void initConnect();
    bool isUsingFont(QString family, QString style);
    void styleCombBoxChangedSlot(int index);

private:

    QWidget *m_widget = nullptr;
    QScrollArea *m_scrollArea = nullptr;

    KLabel* m_stylelabel = nullptr;
    QComboBox* m_styleCombBox = nullptr;

    KLabel* m_fontName = nullptr;         /* 属性：全称 */
    KLabel* m_nameValue = nullptr;        /* 全称 */

    KLabel* m_fontStyle = nullptr;        /* 属性：样式 */
    KLabel* m_styleValue = nullptr;       /* 样式 */

    KLabel* m_fontType = nullptr;         /* 属性：种类 */
    KLabel* m_typeValue = nullptr;        /* 种类 */

    KLabel* m_fontVersion = nullptr;      /* 属性：版本 */
    KLabel* m_versionValue = nullptr;     /* 版本 */

    KLabel* m_fontPath = nullptr;         /* 属性：位置 */
    KLabel* m_pathValue = nullptr;        /* 位置 */

    KLabel* m_fontCopyright = nullptr;    /* 属性：版权 */
    KLabel* m_copyrightValue = nullptr;   /* 版权 */

    KLabel* m_fontTrademark = nullptr;    /* 属性：商标 */
    KLabel* m_trademarkValue = nullptr;   /* 商标 */

    KLabel* m_removeFontLabel = nullptr;
    KPushButton* m_removeFontButton = nullptr;         // 卸载

    KLabel* m_letterLabel = nullptr;
    KLabel* m_numberLabel = nullptr;
    KLabel* m_wordLabel = nullptr;

    KPushButton *m_continueBtn = nullptr;
    KPushButton *m_cancelBtn = nullptr;

    QVBoxLayout *m_vLayout = nullptr;
    QVBoxLayout *m_vMainLayout = nullptr;
    QDBusInterface* m_fontInterface = nullptr;
    FontInformation m_font;   /* 字体信息结构体 */
    QStringList m_pathList;
    QStringList m_styleList;
    ViewData* m_viewData = nullptr;
    QGSettings* m_styleSettings = nullptr;
    KBackgroundGroup* m_previewGroup = nullptr;

Q_SIGNALS:
    void fontStyleChanged(QString, QString);
    void fontStyleRemoved(QString, QString, QString);

public slots:
    void slotCancelClick();
    void slotContinueClick();
    void clickRemoveFontButtonClick();
};

#endif // POPUPFONTINFO_H
