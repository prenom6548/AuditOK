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

#ifndef SYSTEMAPI_H
#define SYSTEMAPI_H

#include <map>
#include <regex>
#include <string>
#include <vector>

struct ApiParam {
    std::string name;
    std::vector<std::string> values;
};

struct GDbusInfo {
    std::string busName;
    std::string objectPath;
    std::string interfaceName;
    std::string setMethodName;
    std::string getMethodName;
    std::string valueType;
    bool isNumeric;
};

using ApiParamsMapper = std::function<std::string(const std::string &)>;

class SystemApi {
public:
    virtual ~SystemApi() {}
    virtual bool execute() = 0;
};

class CommandSystemApi : public SystemApi {
public:
    CommandSystemApi() = default;
    CommandSystemApi(const std::string &command);

    bool execute() override;

protected:
    std::string command_;
};

class ParamsCommandSystemApi : public CommandSystemApi {
public:
    ParamsCommandSystemApi(const std::string &command, const std::vector<std::string> &paramNames,
                           const std::map<std::string, std::string> &paramValues);

    bool execute() override;

protected:
    std::vector<std::string> paramNames_;
    std::map<std::string, std::string> paramValues_;
};

class NumericCommandSystemApi : public CommandSystemApi {
public:
    NumericCommandSystemApi(const std::string &cmdName, const std::string &valueName,
                            const std::map<std::string, std::string> &commandDetail);

    NumericCommandSystemApi(const std::string &cmdName, const std::string &valueName, const std::string &currentValue);

    std::string getValue() { return value_; };

protected:
    std::string value_;

private:
    std::string cmdName_;
    std::string valueName_;
    std::string currentValue_;

    std::map<std::string, std::string> commandDetail_;
    static const std::map<std::string, double> numericWordMap_;
    static const std::map<std::string, double> deltaWordMap_;
    static const std::map<std::string, std::tuple<std::string, double, double>> commandRangeMap_;

    bool isNumber(const std::string &str) {
        static const std::regex pattern("[+-]?([0-9]*[.])?[0-9]+");
        return std::regex_match(str, pattern);
    }
    bool isPercentage(const std::string &str) {
        static const std::regex pattern("^([+-]?([0-9]*[.])?[0-9]+)%$");
        return std::regex_match(str, pattern);
    }

    std::string value();
    std::string currentValue();
    std::string getFontSizeValue(const std::string &valueName) const;
    std::string getScreenScaleValue(const std::string &valueName) const;
};

class AppLaunchSystemApi : public SystemApi {
public:
    AppLaunchSystemApi(const std::string &desktopFile);

    bool execute() override;

private:
    std::string desktopFile_;
};

class DBusCommandSystemApi : public SystemApi {
public:
    DBusCommandSystemApi(const std::string &command, const std::string &valueName);

    bool execute() override;

    static void registerCommandToApiMapper(const std::string &cmd, ApiParamsMapper mapper);

private:
    bool callDbusInterface(const GDbusInfo &info, const std::string &value);
    std::string getDbusInterfaceValue(const GDbusInfo &info);

private:
    std::string cmdName_;
    std::string valueName_;
    std::string value_;
    static const std::map<std::string, GDbusInfo> dbusInterfaceMap_;
    std::unique_ptr<NumericCommandSystemApi> numericCommandSystemApi_;
    static std::map<std::string, ApiParamsMapper> apiParamsMappers_;
};

#endif  // SYSTEMAPI_H
