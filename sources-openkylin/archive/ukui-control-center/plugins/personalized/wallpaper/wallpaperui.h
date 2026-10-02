/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef WALLPAPERUI_H
#define WALLPAPERUI_H

#include "previewlabel.h"
#include "settinggroup.h"
#include "ukccframe.h"
#include "addbutton.h"
#include "comboxwidget.h"
#include "pictureunit.h"
#include "flowlayout.h"
#include "colordialog.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QStackedWidget>
#include <klabel.h>
#include <kpushbutton.h>

using namespace kdk;

class WallpaperUi : public QWidget
{
    Q_OBJECT
public:
    WallpaperUi();
    ~WallpaperUi();

public:
    void setWallpaperModes(const QStringList &modeList, const QStringList &modeDataList);
    void setDisplayTypes(const QStringList &typeList, const QStringList &typeDataList);
    void setWallpaperMode(const QString &mode, const QString &data);
    void setDisplayType(const QString &type);
    void setPicturesFile(QList<PictureInfo*> picInfoList); // 压缩文件、源文件
    void showPictures();
    void showColors();
    KPushButton* getLocalBtn() {return m_localBtn;}
    KPushButton* getOnlineBtn() {return m_onlineBtn;}
    KPushButton* getResetBtn() {return m_resetBtn;}
    QWidget* getModeWidget() {return m_modeWidget;}
    QWidget* getTypeWidget() {return m_typeWidget;}
    void setPicVisibleBySec(bool visible) {m_picVisibleBySec = visible;}
    void setModeWidgetVisible(bool visible);
// 初始化
private:
    void initUi();
    void initColors();
    void createObject();
    void initSignals();
    void resetClickedPic();
    void resetClickedColor();
    // 「恢复默认」之后 Tab 落到选中/首张缩略图（创建序早于按钮，须显式 setTabOrder）
    void ensurePictureTabOrder();

private:
    void createPictureUnit(PictureUnit* picUnit, const bool &clicked);
    void colorAddButtonSlot();
    void colorDialogSlot(QColor color);
    void typeComboBoxSlot();
    void localBtnClickSlot();
    void onlineBtnClickSlot();
    void resetBtnClickSlot();
    void picButtonClickSlot(PictureUnit* colorUnit, QString color);
    void modeComboBoxSlot();

private:
    QVBoxLayout *m_uiLayout      = nullptr;

private:
    KLabel   *m_wallpaperTitleLabel = nullptr;
    PreviewLabel *m_previewLabel        = nullptr;

private:
    SettingGroup *m_settingGroup = nullptr;
    SettingGroup *m_colorGroup = nullptr;
    UkccFrame* m_previewFrame = nullptr;
    UkccFrame    *m_bottomFrame = nullptr;
    UkccFrame    *m_picBtnFrame = nullptr;
    UkccFrame    *m_picFrame = nullptr;
    UkccFrame    *m_addbuttonFrame = nullptr;
    UkccFrame    *m_colorFrame = nullptr;
    FlowLayout   *m_colorLayout = nullptr;
    FlowLayout   *m_pictureLayout = nullptr;
    KPushButton  *m_localBtn = nullptr;
    QHBoxLayout  *m_addbuttonlayout = nullptr;
    AddButton    *m_colorAddButton = nullptr;
    KPushButton  *m_onlineBtn = nullptr;
    KPushButton  *m_resetBtn = nullptr;
    QWidget* m_modeWidget = nullptr;
    QWidget* m_typeWidget = nullptr;
    QComboBox* m_modeComboBox = nullptr;
    QComboBox* m_typeComboBox = nullptr;
    QScrollArea *m_colorScrollArea = nullptr;
    QScrollArea *m_contentScrollArea = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;

private:
    QString     currentFile;
    PictureUnit *prePictureUnit = nullptr;
    PictureUnit *preColorUnit = nullptr;
    bool m_picVisibleBySec = true;
    // 点「恢复默认」后，等选中图刷新再把焦点落到该缩略图
    bool m_focusSelectedPicAfterReset = false;

Q_SIGNALS:
    void wallpaperChanged(const QString &mode, const QString &data);
    void typeChanged(const QString &type);
    void localButtonClicked();
    void onlineButtonClicked();
    void resetButtonClicked();
    void colorAddButtonClicked();
};



#endif // WALLPAPERUI_H
