/*
 * libkysdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#ifndef CURRENCY_H_
#define CURRENCY_H_

#include <QString>

/**
 * @brief kabase模块应用通用函数
 * @since 1.0.0
 */

namespace kdk
{
namespace kabase
{
/* 应用名 */
enum AppName {
    KylinIpmsg = 0, /* 传书 */
    KylinFontViewer, /* 字体管理器 */
    KylinCalculator, /* 麒麟计算器 */
    KylinGpuController, /* 显卡控制器 */
    KylinMusic, /* 音乐 */
    KylinWeather, /* 天气 */
    KylinPhotoViewer, /* 看图 */
    KylinServiceSupport, /* 服务与支持 */
    KylinPrinter, /* 麒麟打印 */
    KylinCalendar, /* 日历 */
    KylinRecorder, /* 录音 */
    KylinCamera, /* 摄像头 */
    KylinNotebook, /* 便签 */
    KylinOsManager, /* 麒麟管家 */
    KylinNetworkCheck, /* 网络检测工具 */
    KylinGallery, /* 相册 */
    KylinScanner, /* 扫描 */
    KylinMobileAssistant, /* 多端协同 */
    KylinTest /* 测试预留 */
};

class Currency
{
public:
    Currency();
    ~Currency();

    static QString getAppName(AppName appName);

private:
};

} /* namespace kabase */
} /* namespace kdk */

#endif
