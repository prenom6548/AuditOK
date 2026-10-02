#ifndef IJSON_H
#define IJSON_H

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#ifdef UKUI_FUZZ_BUILD
#include <string>
#include <unordered_map>
#endif

namespace UkuiAudioFramwork
{
template <typename T>
class IJson
{
public:
    IJson() = default;

    virtual ~IJson() = default;

public:
    virtual bool isContain(const T&) const = 0;

    virtual void insert(const T&) = 0;

protected:
    void loadJson()
    {
        // GCOVR_EXCL_BR_START
        std::ifstream file(m_fileName);
        if (file.is_open()) {
            /** Initializing nlohmann json without throwing exceptions */
            m_json = nlohmann::json::parse(file, nullptr, false);

            /** Check again whether json is a valid json file to prevent abnormal
             * situations caused by the user's file status */
            if (!m_json.is_object()) {
                m_json = nlohmann::json::object();
            }
        } else {
            m_json = nlohmann::json::object();
        }
        // GCOVR_EXCL_BR_STOP
    }

    void writeToJson()
    {
        // GCOVR_EXCL_BR_START
        std::filesystem::create_directories(std::filesystem::path(m_fileName).parent_path());
#ifdef UKUI_FUZZ_BUILD
        std::ostringstream formatted;
        formatted << std::setw(4) << m_json << std::endl;
        const std::string payload = formatted.str();
        static thread_local std::unordered_map<std::string, std::string> s_last_payload;
        const auto prev = s_last_payload.find(m_fileName);
        if (prev != s_last_payload.end() && prev->second == payload) {
            return;
        }
        std::ofstream outFile(m_fileName);
        if (!outFile) {
            std::cerr << "Unable to open " << m_fileName << " for writing." << std::endl;
            return;
        }
        outFile << payload;
        s_last_payload[m_fileName] = payload;
#else
        std::ofstream outFile(m_fileName);
        if (!outFile) {
            std::cerr << "Unable to open " << m_fileName << " for writing." << std::endl;
            return;
        }
        outFile << std::setw(4) << m_json << std::endl;
#endif
        // GCOVR_EXCL_BR_STOP
    }

protected:
    mutable std::string m_fileName;
    mutable nlohmann::json m_json;
};
}
#endif // IJSON_H
