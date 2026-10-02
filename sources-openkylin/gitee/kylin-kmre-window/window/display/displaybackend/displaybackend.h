/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#ifndef DISPLAY_BACKEND_H
#define DISPLAY_BACKEND_H

#include <memory>
#include <mutex>
#include <QObject>
#include <unordered_map>
#include "displayconnection.h"
#include "typedef.h"
#include "singleton.h"

class KmreDisplay;
class DisplayWidget;
class DisplayManager;
class DisplayWorkManager;

using namespace kmre;

class DisplayBackend : public QObject, public kmre::SingletonP<DisplayBackend>
{
    Q_OBJECT
private:
    typedef struct {
        uint32_t name;
        int32_t width;
        int32_t height;
        int32_t stride;
        int32_t bpp;
    }BoInfo;

    typedef struct {
        std::string name;
        uint32_t id;
        int32_t orientation;
        BoInfo bo;
    }VirtualDisplay;

    std::mutex mDirtyDisplaysLock;
    std::vector<VirtualDisplay> mDirtyDisplays;// displays which need update
    std::mutex mUnupdatedDisplaysLock;
    std::unordered_map<int, VirtualDisplay> mUnupdatedDisplays;
    std::mutex mDisplayListLock;
    std::list<sp<KmreDisplay>> mDisplayList;

public:
    bool registerDisplay(DisplayManager *displayManager, DisplayWidget *displayWidget);
    void unregisterDisplay(int displayId);
    sp<DisplayWorkManager> getWorkManagerForDisplay(int displayId);
    bool connectDisplay(DisplayWidget *displayWidget);
    void addDirtyDisplay(const char* name, uint32_t id, uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t orientation);
    void updateDisplays();
    void enableDisplayUpdate(int displayId, bool enable);
    void forceRedraw(int displayId);
    void displayBlurUpdate(int displayId);

private:
    DisplayBackend();
    ~DisplayBackend();

    bool updateDisplay(int displayId, int name, int width, int height, int stride, int bpp, int orientation);
    void redrawMissedUpdating(sp<KmreDisplay> display);
    int getDelayUpdateTime(DisplayManager *displayManager);

    friend SingletonP<DisplayBackend>;
};

#endif // DISPLAY_BACKEND_H
