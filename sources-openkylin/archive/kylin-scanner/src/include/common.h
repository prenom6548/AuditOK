/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QSize>
#include <QColor>
#include <QDir>
#include <QFileInfo>

/******** Mainwidget ********/
#define MainWidgetWidth 960
#define MainWidgetHeight 677
#define ApplicationName "Scanner"

/******** TitlebarWidget ********/
#define TitlebarLogoSize  QSize(24,24)
#define TitlebarFontSize  14
#define TitlebarButtonSize  QSize(30,30)
#define TitlebarHeight  40


/******** ScanSettingsWidget ********/
// font size by desinger maybe small, so normal font can make window large
#define AddWidthForLargeFontSize 20

#define ScanSettingsWidgetWidth  (250+AddWidthForLargeFontSize)
#define ScanSettingsWidgetHeight (MainWidgetHeight-TitlebarHeight)
#define ScanSettingsWidgetScanButtonSize  QSize(144, 36)//216, 36
#define ScanSettingsWidgetComboboxSize  QSize(144, 36)//166, 36
#define ScanSettingsWidgetButtonSize  QSize(80, 36)//102, 36
#define ScanSettingsLabelWidth (42+AddWidthForLargeFontSize)
#define ScanSettingsLabelElideWidth 49
#define ScanSettingsButtonElideWidth 122
#define ScanSettingsMailAndButtonElideWidth 70


/******** LeftSuccessPageWidget ********/
#define LeftSuccessPageWidth (MainWidgetWidth-ScanSettingsWidgetWidth)
#define LeftSuccessPageHeight (MainWidgetHeight-TitlebarHeight)
#define LeftSuccessPageIconSize  QSize(96, 96)

/******** LeftImageHandleSuccessPageWidget ********/
#define LeftImageHandleSuccessPageWidth  (MainWidgetWidth-ScanSettingsWidgetWidth)
#define LeftImageHandleSuccessPageHeight (MainWidgetHeight-TitlebarHeight)


/******** ThumbnailWidget ********/
#define ThumbnailWidgetMinimumWidth 68
#define ThumbnailWidgetTabletMinimumWidth 78    // 平板模式下的宽度
#define ThumbnailIconSize QSize(40, 40)
#define ThumbnailShadowColor QColor(5,15,25,133)
#define ThumbnailShadowRadiu 20

/******** ShowImageWidget ********/
#define ShowImageWidgetWidth (LeftImageHandleSuccessPageWidth-ThumbnailWidgetMinimumWidth)
#define ShowImageWidgetHeight LeftImageHandleSuccessPageHeight
#define ShowImageWidgetSpacing 128
#define ShowImageWidgetImageLabelWidth (ShowImageWidgetWidth-2*ShowImageWidgetSpacing)


/******** showOcrWidget ********/
#define ShowOcrWidgetWidth (LeftImageHandleSuccessPageWidth-ThumbnailWidgetMinimumWidth)
#define ShowOcrWidgetHeight LeftImageHandleSuccessPageHeight
#define ShowOcrWidgetSpacing 16
#define ShowOcrWidgetTextEditWidth (ShowOcrWidgetWidth-2*ShowOcrWidgetSpacing)

#define ChineseTranslationFilePath			"/usr/share/kylin-scanner/translations/kylin-scanner."
#define ChangelogFilePath                      "/usr/share/doc/kylin-scanner/changelog.Debian.gz"

#define UserExistFile 		"/tmp/kylin-scanner/user.pid"
#define MailPicture					"/tmp/kylin-scanner/mail.jpg"
#define ScanningPicture				"/tmp/kylin-scanner/scanning.png"

#define BashType 							"/bin/bash"

#define SUCCESS								0
#define FAIL								-1



#endif // COMMON_H
