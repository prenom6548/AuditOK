/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "PackageCheckUtils.h"

// 辅助函数，用于检查包状态并输出日志信息
bool PackageCheckUtils::checkAndLogPackageStatus(const Package * package, const QString &functionName, const QString &className)
{
    if (package == nullptr ||
        package->m_status == PackageEnum::Status::OTHERS ||
        package->m_status == PackageEnum::Status::DAMAGED)
    {
        if (package == nullptr)
        {
            qWarning() << className << "::" << functionName << "::package=nullptr";
        }
        else
        {
            qWarning() << className << "::" << functionName << "::package=" << package << ",package->m_status=" << package->m_status;
        }
        return false;
    }
    return true;
}
