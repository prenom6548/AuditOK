/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */
#ifndef UKUI_QUICK_DISCARDTEXTURERESOURCESRUNNABLE_H
#define UKUI_QUICK_DISCARDTEXTURERESOURCESRUNNABLE_H

#include <QRunnable>
namespace UkuiThumbnail
{
    class DiscardTextureResourcesRunnable : public QRunnable
    {
    public:
        using ReleaseFunc = std::function<void()>;

        DiscardTextureResourcesRunnable(const ReleaseFunc& releaseFunc)
                : m_releaseFunc(releaseFunc){}
        void run() override {
            if (m_releaseFunc) {
                m_releaseFunc();
            }
        }
    private:
        ReleaseFunc m_releaseFunc;
    };
}
#endif //UKUI_QUICK_DISCARDTEXTURERESOURCESRUNNABLE_H