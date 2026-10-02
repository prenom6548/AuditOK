/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#pragma once

#include <pipewire/pipewire.h>
#include <pipewire/extensions/metadata.h>

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace UkuiAudioFramwork
{
class Pipewire;

class PipewireClient
{
    void propertiesChanged();
    void idChanged();
    void propertyChanged(const std::string& key, const std::string& value);

private:
    Pipewire *pipewire = nullptr;
	uint32_t m_id = 0;

    struct pw_client *client = nullptr;
    struct spa_hook client_listener;

    std::unordered_map<std::string, std::string> m_properties;

public:
    explicit PipewireClient(Pipewire *parent, uint32_t id, const spa_dict* props);
    virtual ~PipewireClient();

	[[nodiscard]] int id() const { return m_id; }

    std::string property(const std::string &key)
    {
        return m_properties[key];
    }

    bool has_property(const std::string &key)
    {
        return m_properties.contains(key);
    }

//    QVariantMap properties()
//    {
//        QVariantMap rval;
//        QMap<QString, QString>::iterator i = m_properties.begin();
//        while (i != m_properties.end()) {
//            rval[i.key()] = QVariant::fromValue<QString>(i.value());
//            ++i;
//        }
//        return rval;
//    }

//    QList<QVariantMap> propertiesList()
//    {
//        QList<QVariantMap> rval;
//        QMap<QString, QString>::iterator i = m_properties.begin();
//        while (i != m_properties.end()) {
//            QVariantMap value;
//            value["name"] = i.key();
//            value["value"] = i.value();
//            rval.append(value);
//            ++i;
//        }
//        return rval;
//    }



    std::unordered_map<std::string, std::string> properties()
    {
        std::unordered_map<std::string, std::string> rval;
        for (const auto& pair : m_properties) {
            rval[pair.first] = pair.second;
        }
        return rval;
    }

    std::vector<std::unordered_map<std::string, std::string>> propertiesList()
    {
        std::vector<std::unordered_map<std::string, std::string>> rval;
        for (const auto& pair : m_properties) {
            std::unordered_map<std::string, std::string> value;
            value["name"] = pair.first;
            value["value"] = pair.second;
            rval.push_back(value);
        }
        return rval;
    }

    void _client_info(const struct pw_client_info *info);

};
}
