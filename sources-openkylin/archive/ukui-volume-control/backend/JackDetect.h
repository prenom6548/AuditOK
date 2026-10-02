#pragma once
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <alsa/asoundlib.h>

namespace UkuiAudioFramwork
{
enum class AlsaJackType
{
    ALSA_JACK_FRONTMIC,
    ALSA_JACK_HEADPHONEMIC,
    ALSA_JACK_HEADSETMIC,
    ALSA_JACK_INTERNALMIC,
    ALSA_JACK_LINEIN,
    ALSA_JACK_INPUTMIC,
    ALSA_JACK_REARMIC,
    ALSA_JACK_HEADPHONE2,
    ALSA_JACK_HEADPHONE,
    ALSA_JACK_LINEOUT,
    ALSA_JACK_SPEAKER,
    ALSA_JACK_HDMI,
    ALSA_JACK_UNKNOEW
};

struct AlsaJack
{
    // uint32_t card;
    uint32_t numid;
    std::string name; // jack name
    AlsaJackType type;
    bool plugin;
};

struct AlsaCard
{
    std::string name;
    std::vector<AlsaJack> jacks;
};

class JackDetect
{
public:
    using EventCallback = std::function<void(const std::string &card,
                                             const std::string &jack,
                                             AlsaJackType type,
                                             bool plugged)>;

    explicit JackDetect(EventCallback cb);

    virtual ~JackDetect();

public:
    void start();

    void stop();


private:
    void loop();

    void scanCards();

    std::vector<std::string> listSoundCards();

    bool jackIsPlugged(snd_ctl_elem_id_t*, const char*);

    void enumJacks(const std::string&, JackDetect::EventCallback);

    AlsaJackType getJackTypeByName(const std::string&) const;

private:
    EventCallback m_callback;
    std::thread m_worker;
    std::atomic<bool> m_bRunning {false};
    int m_inotifyFd {-1};
    std::vector<int> m_watchDescriptors;
    std::vector<AlsaCard> m_cards;

    std::unordered_map<std::string, AlsaJackType> m_jackMap {
        {"Front Mic Jack", AlsaJackType::ALSA_JACK_FRONTMIC},
        {"Front Mic Phantom Jack", AlsaJackType::ALSA_JACK_FRONTMIC},
        {"Headphone Mic Jack", AlsaJackType::ALSA_JACK_HEADPHONEMIC},
        {"Headset Mic Jack", AlsaJackType::ALSA_JACK_HEADSETMIC},
        {"Headset Mic Phantom Jack", AlsaJackType::ALSA_JACK_HEADSETMIC},
        {"Internal Mic Phantom Jack", AlsaJackType::ALSA_JACK_INTERNALMIC},
        {"Line Phantom Jack", AlsaJackType::ALSA_JACK_LINEIN},
        {"Line - Input Jack", AlsaJackType::ALSA_JACK_LINEIN},
        {"Mic Jack", AlsaJackType::ALSA_JACK_INPUTMIC},
        {"Mic Phantom Jack", AlsaJackType::ALSA_JACK_INPUTMIC},
        {"Mic - Input Jack", AlsaJackType::ALSA_JACK_INPUTMIC},
        {"Rear Mic Jack", AlsaJackType::ALSA_JACK_REARMIC},
        {"Rear Mic - Input Jack", AlsaJackType::ALSA_JACK_REARMIC},
        {"Rear Mic Phantom Jack", AlsaJackType::ALSA_JACK_REARMIC},
        {"Front Headphone,1 Jack", AlsaJackType::ALSA_JACK_HEADPHONE2},
        {"Front Headphone Surround Jack", AlsaJackType::ALSA_JACK_HEADPHONE2},
        {"Dock Headphone Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Dock Headphone Phantom Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Front Headphone Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Front Headphone Front Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Front Headphone Phantom Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Headphone Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Headphone Phantom Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Headphone Mic Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Headphone - Output Jack", AlsaJackType::ALSA_JACK_HEADPHONE},
        {"Line Out Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out Phantom Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Front Line Out Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Front Line Out Phantom Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Rear Line Out Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out Front Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out Front Phantom Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out CLFE Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out CLFE Phantom Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out Surround Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out Surround Phantom Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out Side Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Line Out Side Phantom Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Dock Line Out Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Dock Line Out Phantom Jack", AlsaJackType::ALSA_JACK_LINEOUT},
        {"Speaker Jack", AlsaJackType::ALSA_JACK_SPEAKER},
        {"Speaker Phantom Jack", AlsaJackType::ALSA_JACK_SPEAKER},
        {"Speaker Front Phantom Jack", AlsaJackType::ALSA_JACK_SPEAKER},
        {"Speaker - Output Jack", AlsaJackType::ALSA_JACK_SPEAKER},
        {"HDMI/DP", AlsaJackType::ALSA_JACK_HDMI}
    };
};
}
