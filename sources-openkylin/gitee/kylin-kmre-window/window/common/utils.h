/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _KMRE_UTILS_H_
#define _KMRE_UTILS_H_

#include <QString>
#include <QFileInfo>
#include <QList>
#include <QUrl>
#include <QLockFile>

namespace kmre {
namespace utils {

const QString& getUserName();
const QString& getUid();
QString makeContainerName();
QString makeContainerName(const QString& userName, int uid);
int checkLockFile();
void releaseLockFile();
bool tryLockFile(const QString& lockFilePath, int msecond);
bool tryLockKylinKmreWindow();
void unLockKylinKmreWindow();
QString readFileContent(const QString &path);
bool isFileSuffixSupported(const QString &path);
const QFileInfoList getAllFilesInfo(const QString &dir, bool recursive);
int isInCpuinfo(const char *fmt, const char *str);
QString getIconPath(const QString &packageName);
bool isFileExist(QString fullFilePath);
int currentDisplayOrientation(int initialOrientation, int imageRotation);
bool readX11WindowFullScreenFromConifg(const QString &pkgName);
bool readAppNonuniformFromConifg(const QString &pkgName);
int isZoomSupported(const QString &packageName);
void getWindowScaleAndCenterPosition(int scaleWidth, int scaleHeight, int width, int height, int &x, int &y, float &scale);
QString getFixedDefaultCameraDevice(const QString &deviceName);
QString getDefaultScreenshotFolder();
QString getDebPkgVersion(const QString &debPkgName);
void clearApkFiles();
QStringList convertUrlsToUris(const QList<QUrl> &urls);
void openFolder(const QString &path);
void initShmOfDisplayType();
void destroyShmOfDisplayType();
void saveDisplayTypeToShm(const QString& displayType);
QString getDisplayTypeFromShm();
bool getAppNameFromDesktop(const QString &pkgName, QString &appName_en, QString &appName_zh);
void showUserGuide();
QString getRealPkgName(const QString &pkgName);
QString getCpuArch();
bool checkAndCreateFile(const QString& filePath, int permission);
QByteArray tryReadFile(const QString& filePath, bool lock, int msecond);
bool tryWriteFile(const QString& filePath, const QByteArray& data, bool lock, int msecond);
std::tuple<bool, QString> getElideText(const QString& origString, const uint32_t maxLength, 
            Qt::TextElideMode mode = Qt::ElideRight);

} // namespace utils
} // namespace kmre

#endif // _KMRE_UTILS_H_

