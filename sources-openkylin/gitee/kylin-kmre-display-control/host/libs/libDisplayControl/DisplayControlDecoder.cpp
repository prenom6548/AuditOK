/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Ma Chao    machao@kylinos.cn
 * Alan Xie   xiehuijun@kylinos.cn
 * Clom       huangcailong@kylinos.cn
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DisplayControlDecoder.h"

DisplayControlDecoder::Garbo DisplayControlDecoder::garbo;
DisplayControl::Mutex DisplayControlDecoder::lock;
DisplayControlDecoder *DisplayControlDecoder::m_pInstance = NULL;

DisplayControlDecoder* DisplayControlDecoder::getInstance()
{
    if(NULL == m_pInstance)
    {
        DisplayControl::AutoMutex  _l(lock);
        if(NULL == m_pInstance)
        {
            m_pInstance = new DisplayControlDecoder;
        }

    }

    return m_pInstance;
}

