/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMRemoteSummary.h"

#include "common/KMLogger.h"

class KMRemoteSummary::Private
{
public:
};

KMRemoteSummary::KMRemoteSummary()
    : d(make_shared<Private>())
{
}

KMRemoteSummary::~KMRemoteSummary() { }

/**
 * @brief 通过 curl 命令行下载文件
 *
 * @param url 下载的文件链接
 * @param outputFile  存储到本地的文件路径
 * @return true 下载成功
 * @return false 下载失败
 */
bool KMRemoteSummary::curlCmdDownload()
{
    return true;
}
