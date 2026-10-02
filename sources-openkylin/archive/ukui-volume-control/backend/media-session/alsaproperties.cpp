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


#include "alsaproperties.h"
#include "src/pw/qpipewireclient.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>

bool is_valid_file(const std::string &path)
{
    QFile file(path);
    if (!file.exists()) return false;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    return file.isReadable();
}

AlsaProperties::AlsaProperties(PipewireClient *client/*, QObject *parent*/)
//    : QObject(parent)
{
    std::string config_prefix = client->has_property("config.prefix") ?
                                    client->property("config.prefix").toStdString() : "media-session.d";
    const char *home_folder = std::getenv("HOME");
    if (home_folder == nullptr) {
        home_folder = "";
    }

    std::ostringstream global_conf_filename;
    global_conf_filename << "/usr/share/pipewire/"
                         << config_prefix
                         << "/alsa-monitor.conf";
    std::ostringstream user_conf_filename;
    user_conf_filename << home_folder
                       << "/.config/pipewire/"
                       << config_prefix
                       << "/alsa-monitor.conf";

    this->globalConf = global_conf_filename.str().c_str();
    this->userConf = user_conf_filename.str().c_str();

    bool has_global = is_valid_file(globalConf);
    bool has_user = is_valid_file(userConf);

    if (has_user) {
        this->readUserConf();
    } else if (has_global) {
        this->readGlobalConf();
    } else {
        throw std::runtime_error("Could not find pipewire alsa monitor config file!");
    }
}

void AlsaProperties::setBatchDisabled(bool disabled)
{
    if (_batchDisabled == disabled) return;
    _batchDisabled = disabled;
    writeUserConf();
//    emit batchDisabledChanged(_batchDisabled);
}

void AlsaProperties::setPeriodSize(int newPeriod)
{
    if (_periodSize == newPeriod) return;
    _periodSize = newPeriod;
    writeUserConf();
//    emit periodSizeChanged(_periodSize);
}

void AlsaProperties::readGlobalConf()
{
    readConf(globalConf);
}

void AlsaProperties::readUserConf()
{
    readConf(userConf);
}

template<typename T>
T _parse(const std::string&/*value*/) {
    throw std::runtime_error("Unsupported conversion");
}
template<>
int _parse<int>(const std::string& value) {
    return value.toInt();
}
template<>
bool _parse<bool>(const std::string& value) {
    const QString lower_case_value = value.toLower();
    if (lower_case_value == "true") {
        return true;
    } else if (lower_case_value == "false") {
        return false;
    } else {
        throw std::runtime_error((QString("Could not parse bool type from: ") + value).toStdString());
    }
}

template<typename T>
std::string _tostring(const T /*value*/) {
    throw std::runtime_error("diocane");
}
template<>
std::string _tostring<int>(const int value) {
    return QString::number(value);
}
template<>
std::string _tostring<bool>(const bool value) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

template<typename T>
bool parse_line(const std::string&line, const std::string& target, T &value)
{
    QRegularExpression re(QString("(#*)\\s*") + target + "\\s*=\\s*([\\d|\\w]+)");
    QRegularExpressionMatch match = re.match(line);
    if(!match.hasMatch()) {
        throw std::runtime_error((QString("Could not find ") + target).toStdString());
    }

    if (!match.captured(1).isEmpty()) {
        return false;
    }

    value = _parse<T>(match.captured(2));
    return true;
}

template<typename T>
void parse_and_change_line(std::string &line, const std::string &target, T value)
{
    QRegularExpression re(QString("(#*)\\s*") + target + "\\s*=\\s*([\\d|\\w]+)");
    QRegularExpressionMatch match = re.match(line);
    if(!match.hasMatch()) {
        throw std::runtime_error((QString("Could not find ") + target).toStdString());
    }

    if (!match.captured(1).isEmpty()) {
        line.remove(match.capturedStart(1), match.capturedLength(1));
        match = re.match(line);
    }

    line.remove(match.capturedStart(2), match.capturedLength(2));
    line.insert(match.capturedStart(2), _tostring<T>(value));
    return;
}

void AlsaProperties::readConf(const std::string& filename)
{
    QFile conf(filename);
    if (!conf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::ostringstream error_message;
        error_message << "Could not open configuration file " << filename.toStdString();
        std::cerr << error_message.str() << std::endl;
        throw std::runtime_error(error_message.str());
    }

    while (!conf.atEnd()) {
        std::string line = conf.readLine().trimmed();

        // Skip comments
        if (line.startsWith('#')) continue;

        if (line.contains("api.alsa.disable-batch")) {
            bool found = parse_line(line, "api.alsa.disable-batch", _batchDisabled);
            if (found) emit batchDisabledChanged(_batchDisabled);
        } else if (line.contains("api.alsa.period-size")) {
            bool found = parse_line(line, "api.alsa.period-size", _periodSize);
            if (found) emit periodSizeChanged(_periodSize);
        }
    }
}

void AlsaProperties::writeUserConf() const
{
    QFileInfo confInfo(userConf);
    QDir folder = QDir(confInfo.absolutePath());
    std::cout << "Creating user conf folder: " << folder.absolutePath().toStdString() << std::endl;
    // This will return true even if the folder already exists.
    if (!folder.mkpath(".")) {
        throw std::runtime_error("Could not create user conf folder");
    }

    // Read file
    QList<std::string> lines;
    {
        QFile userConfF(userConf);
        QFile globalConfF(globalConf);
        QFile *inputFile = &userConfF;
        if (!userConfF.exists()) {
            inputFile = &globalConfF;
        }

        if (!inputFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Could not write user conf file");
        }

        while (!inputFile->atEnd()) {
            QString line = inputFile->readLine();
            QString trimmed = line.trimmed();
            if (trimmed.contains("api.alsa.disable-batch")) {
                parse_and_change_line(line, "api.alsa.disable-batch", this->_batchDisabled);
            } else if (trimmed.contains("api.alsa.period-size")) {
                parse_and_change_line(line, "api.alsa.period-size", this->_periodSize);
            }

            lines.append(line);
        }

        inputFile->close();
    }

    std::cout << "Writing user conf file: " << folder.absolutePath().toStdString() << std::endl;

    {
        // Write with lines changed
        QFile outFile(userConf);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw std::runtime_error("Could not write user conf file");
        }

        QTextStream out(&outFile);
        for(QString &line : lines) {
            out << line;
        }

        outFile.close();
    }

}
