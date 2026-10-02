#ifndef MEDIACONTROLER_H
#define MEDIACONTROLER_H

#include <glib.h>
#include <gio/gio.h>
#include <vector>
#include <string>

namespace UkuiAudioFramwork
{
class MediaControler
{
public:
    static MediaControler& getInstance();

public:
    void pauseAllPlayers();

private:
    std::vector<std::string> getMprisPlayers(GDBusConnection*) const;

private:
    MediaControler() = default;
    MediaControler(const MediaControler&) = delete;
    MediaControler(MediaControler&&) = delete;
    MediaControler operator=(const MediaControler&) = delete;
    MediaControler operator=(MediaControler&&) = delete;
    virtual ~MediaControler() = default;
};
}
#endif // MEDIACONTROLER_H
