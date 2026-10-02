/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IMAGEEMBEDDINGSERVER_H
#define IMAGEEMBEDDINGSERVER_H

#include <unistd.h>

#include "coreimageembeddingserviceglue.h"
#include "protocolversion.h"
#include "server.h"

namespace kyai::coreai::embedding {

class ImageEmbeddingServer : public Server {
public:
    static ImageEmbeddingServer *getInstance();

    AiRuntimeCoreImageEmbeddingService *getImageEmbeddingProxy();

protected:
    ImageEmbeddingServer();

    // 实现基类的纯虚函数
    GDBusProxy *createProxy(GDBusConnection *connection,
                            GError **error) override;
    void refreshProtocolVersion(GDBusProxy *proxy) override;
};

}  // namespace kyai::coreai::embedding

#endif
