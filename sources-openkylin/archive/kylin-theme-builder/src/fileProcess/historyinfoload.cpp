/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "historyinfoload.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

/**
 * @brief 构造函数，用于加载历史信息
 *
 * 根据给定的日期，创建一个 HistoryInfoLoad 对象，用于加载历史信息。
 *
 * @param date 日期
 * @param parent 父对象指针
 */
HistoryInfoLoad::HistoryInfoLoad(const QString &date, QObject *parent)
{
    m_historyInfo.historytime = date;
    m_historyInfo.filepath = QDir::homePath() + "/.cache/theme-build/" + m_historyInfo.historytime;
    getWallpaper();
    getAccentColor();
    getRadius();
    getTransparency();
    getWindowRadius();
    getWidgetStyle();
}

/**
 * @brief 获取封面
 *
 * 从历史信息中获取封面的路径。
 *
 * @return 封面路径
 */
QString HistoryInfoLoad::getCover()
{
    QString filePath = m_historyInfo.filepath + "/src/globalTheme/preview";
    QString previewFilePath;

    QFileInfo pngFileInfo(filePath + ".png");
    if (pngFileInfo.exists()) {
        previewFilePath = pngFileInfo.filePath();
    } else {
        QFileInfo jpgFileInfo(filePath + ".jpg");
        if (jpgFileInfo.exists()) {
            previewFilePath = jpgFileInfo.filePath();
        } else {
            previewFilePath = nullptr;
        }
    }
    m_historyInfo.coverpath = previewFilePath;
    return m_historyInfo.coverpath;
}

/**
 * @brief 获取主题名称
 *
 * 从指定的文件路径中读取主题名称并返回。
 *
 * @return 主题名称
 */
QString HistoryInfoLoad::getThemeName()
{
    QString filePath = m_historyInfo.filepath + "/src/debian/control";

    QFile file(filePath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QString content(file.readAll());
        file.close();

        QString sourceString = "Source:";
        int sourceIndex = content.indexOf(sourceString);

        if (sourceIndex != -1) {
            int startIndex = sourceIndex + sourceString.length();
            QString fieldValue = content.mid(startIndex).trimmed().split("\n").at(0);

            qDebug() << "Source field value: " << fieldValue;
            m_historyInfo.themename = fieldValue;
        } else {
            qDebug() << "No Source field found.";
        }
    } else {
        qDebug() << "File not found or cannot open file.";
    }
    return m_historyInfo.themename;
}

/**
 * @brief 获取主题中文名称
 *
 * 从指定的文件路径中读取主题名称并返回。
 *
 * @return 中文主题名称
 */
QString HistoryInfoLoad::getThemeCNName()
{
    QString filePath = m_historyInfo.filepath + "/src/debian/control";

    QFile file(filePath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QString content(file.readAll());
        file.close();

        QString sourceString = "Description: Theme Package";
        int sourceIndex = content.indexOf(sourceString);

        if (sourceIndex != -1) {
            int startIndex = sourceIndex + sourceString.length();
            QString fieldValue = content.mid(startIndex).trimmed().split("\n").at(0);

            qDebug() << "Source field value: " << fieldValue;
            m_historyInfo.themecnname = fieldValue;
        } else {
            qDebug() << "No Source field found.";
        }
    } else {
        qDebug() << "File not found or cannot open file.";
    }
    return m_historyInfo.themecnname;
}

/**
 * @brief 获取创作名称
 *
 从指定的文件路径中读取主题名称并返回。
 *
 * @return 创作者名称
 */
QString HistoryInfoLoad::getThemeBuilder()
{
    QString filePath = m_historyInfo.filepath + "/src/debian/control";

    QFile file(filePath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QString content(file.readAll());
        file.close();

        QString sourceString = "Maintainer:";
        int sourceIndex = content.indexOf(sourceString);

        // 检查是否找到了 "Maintainer:"
        if (sourceIndex != -1) {
            // 从 "Maintainer:" 的位置开始截取字符串
            QString substring = content.mid(sourceIndex + sourceString.length());

            // 定义正则表达式
            QRegularExpression regex("\\s(\\S+)\\s<");

            // 进行匹配
            QRegularExpressionMatch match = regex.match(substring);

            // 检查是否有匹配
            if (match.hasMatch()) {
                // 获取匹配的部分
                QString extractedName = match.captured(1).trimmed();

                // 输出提取的人名
                qDebug() << "Extracted Name: " << extractedName;
                m_historyInfo.themebuilder = extractedName;
            } else {
                // 没有匹配
                qDebug() << "No match found.";
            }
        } else {
            qDebug() << "Maintainer not found in the file.";
        }

    } else {
        qDebug() << "File not found or cannot open file.";
    }
    return m_historyInfo.themebuilder;
}

/**
 * @brief 获取创作者邮箱
 *
 从指定的文件路径中读取主题名称并返回。
 *
 * @return 创作者邮箱
 */

QString HistoryInfoLoad::getThemeMail()
{
    QString filePath = m_historyInfo.filepath + "/src/debian/control";

    QFile file(filePath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QString content(file.readAll());
        file.close();

        QString sourceString = "Maintainer:";
        int sourceIndex = content.indexOf(sourceString);

        // 检查是否找到了 "Maintainer:"
        if (sourceIndex != -1) {
            // 从 "Maintainer:" 的位置开始截取字符串
            QString substring = content.mid(sourceIndex + sourceString.length());

            // 定义正则表达式
            QRegularExpression regex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}\\b");

            // 进行匹配
            QRegularExpressionMatch match = regex.match(substring);

            // 检查是否有匹配
            if (match.hasMatch()) {
                // 获取匹配的部分
                QString extractedName = match.captured(0).trimmed();

                // 输出提取的人名
                qDebug() << "Extracted Mail: " << extractedName;
                m_historyInfo.thememail = extractedName;
            } else {
                // 没有匹配
                qDebug() << "No match found.";
            }
        } else {
            qDebug() << "Maintainer not found in the file.";
        }

    } else {
        qDebug() << "File not found or cannot open file.";
    }
    return m_historyInfo.thememail;
}

/**
 * @brief 获取主题类型
 *
 * 根据文件路径获取主题类型。
 *
 * @return 主题类型
 */
QString HistoryInfoLoad::getThemeType()
{
    QString filePath = m_historyInfo.filepath + "/src";
    QDir themeDir(filePath);

    if (themeDir.exists("globalTheme")) {
        m_historyInfo.themetype = "globalTheme";
    } else {
        if (themeDir.exists("cursorTheme")) {
            m_historyInfo.themetype = "cursorTheme";
        } else if (themeDir.exists("iconTheme")) {
            m_historyInfo.themetype = "iconTheme";
        }else if (themeDir.exists("plymouthTheme")) {
            m_historyInfo.themetype = "plymouthTheme";
        }else if (themeDir.exists("grubTheme")) {
            m_historyInfo.themetype = "grubTheme";
        }
    }
    return m_historyInfo.themetype;
}

/**
 * @brief 获取半径
 *
 * 从给定的文件路径中加载半径信息，并保存在 HistoryInfo 对象的 radius 成员变量中。
 */
void HistoryInfoLoad::getRadius()
{
    QString filePath = m_historyInfo.filepath + "/src/globalTheme/json.txt";

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening file";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        qDebug() << "jsonDoc is null";
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    QJsonObject radiusObj = rootObj["Radius"].toObject();
//    qDebug() << "Normal_Radius value:" << radiusObj["Normal_Radius"].toObject()["value"];
//    qDebug() << "radiusObj:" << radiusObj;
//    qDebug() << "Normal_Radius value type:" << radiusObj["Normal_Radius"].toObject()["value"].type();
    QString normalRadiusValue = radiusObj["Normal_Radius"].toObject()["value"].toString();
    m_historyInfo.radius = normalRadiusValue.toInt();

}

/**
 * @brief 获取壁纸
 *
 * 从指定的文件中读取壁纸路径。
 */
void HistoryInfoLoad::getWallpaper()
{
    QDir folder(m_historyInfo.filepath + "/src/globalTheme");
    QStringList filters;
    filters << "*.png" << "*.jpg";
    folder.setNameFilters(filters);
    QStringList fileList = folder.entryList();

    if(fileList.contains("1-openkylin.png")){
        m_historyInfo.wallpaperpath = m_historyInfo.filepath + "/src/globalTheme/1-openkylin.png";
    } else if (fileList.contains("1-openkylin.jpg")){
        m_historyInfo.wallpaperpath = m_historyInfo.filepath + "/src/globalTheme/1-openkylin.jpg";
    } else {
        qDebug()<<"wallpaper file is null";
    }
}

/**
 * @brief 获取强调色
 *
 * 从指定路径读取主题配置文件，并获取其中的强调色信息。
 */
void HistoryInfoLoad::getAccentColor()
{
    QString filePath = m_historyInfo.filepath + "/src/globalTheme/theme.conf";
    // 读取文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << filePath;
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int startIndex = content.indexOf("accent=");
    if (startIndex != -1) {
        int lineStartIndex = content.lastIndexOf('\n', startIndex) + 1;
        int lineEndIndex = content.indexOf('\n', startIndex);

        if (lineEndIndex == -1) {
            lineEndIndex = content.size();
        }

        QString lineContent = content.mid(lineStartIndex, lineEndIndex - lineStartIndex).trimmed();
        lineContent.remove("accent=");

        m_historyInfo.accentcolor = lineContent;
    }
}

/**
 * @brief 获取透明度
 *
 * 从指定的路径加载配置文件中获取透明度信息，并将其保存在 HistoryInfo 对象的 transparency 属性中。
 */
void HistoryInfoLoad::getTransparency()
{
    QString filePath = m_historyInfo.filepath + "/src/globalTheme/theme.conf";
    // 读取文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << filePath;
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int startIndex = content.indexOf("transparencyBlur=");
    if (startIndex != -1) {
        int lineStartIndex = content.lastIndexOf('\n', startIndex) + 1;
        int lineEndIndex = content.indexOf('\n', startIndex);

        if (lineEndIndex == -1) {
            lineEndIndex = content.size();
        }

        QString lineContent = content.mid(lineStartIndex, lineEndIndex - lineStartIndex).trimmed();
        lineContent.remove("transparencyBlur=");

        bool conversionSuccess = false;
        int transparencyBlurValue = lineContent.toInt(&conversionSuccess);
        if (conversionSuccess) {
            m_historyInfo.transparency = transparencyBlurValue;
        } else {
            qDebug() << "无法将字符串转换为整数：" << lineContent;
        }
    }
}

void HistoryInfoLoad::getWidgetStyle()
{
    QString filePath = m_historyInfo.filepath + "/src/globalTheme/theme.conf";
    // 读取文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << filePath;
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int startIndex = content.indexOf("defaultLightDarkMode=");
    if (startIndex != -1) {
        int lineStartIndex = content.lastIndexOf('\n', startIndex) + 1;
        int lineEndIndex = content.indexOf('\n', startIndex);

        if (lineEndIndex == -1) {
            lineEndIndex = content.size();
        }

        QString lineContent = content.mid(lineStartIndex, lineEndIndex - lineStartIndex).trimmed();
        lineContent.remove("defaultLightDarkMode=");

        m_historyInfo.widgetstyle = lineContent;
    }
}

/**
 * @brief 获取窗口半径
 *
 * 从指定路径读取主题配置文件，并从中获取窗口半径值。
 */
void HistoryInfoLoad::getWindowRadius()
{
    QString filePath = m_historyInfo.filepath + "/src/globalTheme/theme.conf";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << filePath;
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int startIndex = content.indexOf("windowRadius=");
    if (startIndex != -1) {
        int lineStartIndex = content.lastIndexOf('\n', startIndex) + 1;
        int lineEndIndex = content.indexOf('\n', startIndex);

        if (lineEndIndex == -1) {
            lineEndIndex = content.size();
        }

        QString lineContent = content.mid(lineStartIndex, lineEndIndex - lineStartIndex).trimmed();
        lineContent.remove("windowRadius=");

        bool conversionSuccess = false;
        int windowRadiusValue = lineContent.toInt(&conversionSuccess);
        if (conversionSuccess) {
            m_historyInfo.windowradius = windowRadiusValue;
//            qDebug() << "m_historyInfo.windowradius" << windowRadiusValue;
        } else {
            qDebug() << "无法将字符串转换为整数：" << lineContent;
        }
    }
}

/**
 * @brief 获取历史信息数据
 *
 * 返回包含历史信息的数据对象。
 *
 * @return 历史信息数据对象
 */
HistoryInfo HistoryInfoLoad::getInfoData()
{
//    qDebug()<<m_historyInfo.accentcolor<<m_historyInfo.wallpaperpath<<m_historyInfo.filepath<<m_historyInfo.windowradius<<m_historyInfo.widgetstyle;
    return m_historyInfo;
}

/**
 * @brief 更新历史信息
 *
 * 更新历史信息的函数。
 */
void HistoryInfoLoad::updateHistoryInfo()
{
    getCover();
    getThemeName();
    getThemeType();
    getRadius();
    getWallpaper();
    getAccentColor();
    getTransparency();
    getWindowRadius();
    getWidgetStyle();
}

