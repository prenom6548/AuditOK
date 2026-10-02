#include "disc_interface.h"
#include "diskinfo.h"
#include "device.h"

int getMultiSeeionParameters(std::string devcie_name, int64_t* last_session_start_pos, int64_t* next_session_start_pos)
{
    if(devcie_name.empty() || (last_session_start_pos == NULL) || (next_session_start_pos == NULL)){
        return -1;
    }

    Device::Device  device(devcie_name.c_str());
    Device::DiskInfo info = device.diskInfo();
    Device::Toc toc = device.readToc();
    if( toc.isEmpty() ) {
        return -2;
    }

    unsigned long lastSessionStart = 0;
    unsigned long nextSessionStart = 0;

    // FIXME: Does BD-RE really behave like DVD+RW here?
    if( info.mediaType() & (Device::MEDIA_DVD_PLUS_RW|
                            Device::MEDIA_DVD_PLUS_RW_DL|
                            Device::MEDIA_DVD_RW_OVWR|
                            Device::MEDIA_BD_RE|
                            Device::MEDIA_DVD_RAM|
                            Device::MEDIA_BD_R_RRM) ) {
        lastSessionStart = 0;

        return -3;
    }
    else {
        nextSessionStart = device.nextWritableAddress();
        lastSessionStart = toc.last().firstSector().lba();
    }

    if (info.mediaType() & Device::MEDIA_DVD_ALL ) {
        // pad to closest 32K boundary
        nextSessionStart += 15;
        nextSessionStart /= 16;
        nextSessionStart *= 16;

        // growisofs does it. I actually do not know yet why.... :(
        lastSessionStart += 16;
    }
    *last_session_start_pos = lastSessionStart;
    *next_session_start_pos = nextSessionStart;

    printf("lastSessionSatrt is: %ld nextSessionStart is:%ld", lastSessionStart,nextSessionStart);
    return 0;
}
