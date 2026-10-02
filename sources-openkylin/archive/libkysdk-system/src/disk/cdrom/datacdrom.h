/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef DATA_CDROM_H
#define DATA_CDROM_H

#include <string>
#include <thread>

class DataCDROMImpl;
namespace KYSDKUdfBurn
{
    class DataCDROM
    {
    public:
        explicit DataCDROM(const char *blockName);
        ~DataCDROM();

    public:
        /**
         * @brief 获取光盘信息： 光盘类型、光盘总容量、光盘已使用容量
         * @note 主要用于查询光盘信息，但未返回信息，只是加载到内存中，使用其他getxxx方法获取具体的属性信息。
         *
         *     使用样例：
         *
         *     DataCDROM *p = new DataCDROM("/dev/sr0", this);
         *     p->getCDROMInfo();
         *     // 获取光盘类型
         *     QString discType = p->getCDROMType();
         */
        void getCDROMInfo();

        //        /**
        //         * @brief 异步获取光盘信息：光盘类型，光盘总容量、光盘已使用容量
        //         *
        //         * 使用样例：
        //         *     DataCDROM *p = new DataCDROM("/dev/sr0", this);
        //         *     connect(p, &DataCDROM::sigCdromInfo, this, &TestClass::slotTest);
        //         *     p->cdromInfoAsync();
        //         *     // 槽函数slotTest(QString mediumType, unsigned long totalCapacity
        //         *                       unsigned long usedCapacity)
        //         */
        //        void cdromInfoAsync();

        /**
         * @brief 获取光盘总容量
         * @return 总容量，单位b
         * @note 使用此方法前需要先调用方法getCDROMInfo()
         */
        unsigned long getCDROMCapacity();

        /**
         * @brief 获取光盘已使用容量
         * @return 已使用容量，单位b
         * @note 使用此方法前需要先调用方法getCDROMInfo()
         */
        unsigned long getCDROMUsedCapacity();

        /**
         * @brief 获取光盘类型
         * @return 光盘类型，形如“DVD+RW”
         * @note 使用此方法前需要先调用方法getCDROMInfo()
         */
        std::string getCDROMType();

        /**
         * @brief 获取光盘类型
         * @return 光盘类型，形如“DVD+RW”
         * @note 与方法getCDROMInfo()的区别是，该方法只获取光盘类型
         */
        std::string getDiscType();

        //        void sigCdromInfo(std::string mediumType, unsigned long totalCapacity, unsigned long usedCapacity);

        //        void slotGetCdromInfo();

    private:
        DataCDROMImpl *m_impl = nullptr;
        std::thread *m_thread = nullptr;
        bool m_threadRunning = false;
    };

}

#endif // DATA_CDROM_H
