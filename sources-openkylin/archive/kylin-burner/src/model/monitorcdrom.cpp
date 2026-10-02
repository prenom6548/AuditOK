#include "monitorcdrom.h"
#include "src/frame/logger.h"
#include "src/po/translation.h"

#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <fstream>
#include <QCoreApplication>

#define CDROM "/proc/sys/dev/cdrom/info"

MonitorCDROM *MonitorCDROM::m_oInstance = nullptr;
QMutex        MonitorCDROM::m_oMutex;

MonitorCDROM::MonitorCDROM(QObject *parent) :
    QThread(parent),
    m_bAppRunning(true),
    m_iPreception(-1)
{
    LOGGER->registerName(this, _("Monitor"));
}

MonitorCDROM *MonitorCDROM::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new MonitorCDROM();
        }
    }
    return m_oInstance;
}

void MonitorCDROM::stop()
{
    m_bAppRunning = false;
    if (isRunning())
    {
        terminate();
        wait();
    }
    delete this;
}

void MonitorCDROM::run()
{
    QString rcvMsg;
    QStringList splits;
    char buffer[256];

    update();
    if (m_oBlocks.size() > 0)
    {
        for (int i = 0; i < m_oBlocks.size(); ++i)
        {
            LOGGER->info(_("Calling MAIN to add a CDROM, block name is %1").arg(m_oBlocks.at(i)));
            emit addCDROM(m_oBlocks.at(i));
        }
    }

    if (preception() < 0) return;
    fcntl(m_iPreception, F_SETFL, fcntl(m_iPreception, F_GETFL, 0));
    while (m_bAppRunning)
    {
        QCoreApplication::processEvents();
        //QThread::msleep(200);
        ::memset(buffer, 0, 256);
        if (::read(m_iPreception, buffer, 256) < 0) continue;
        rcvMsg = QString::fromUtf8(buffer);
        splits.clear();
        if (rcvMsg.contains("block/"))
        {
            if (rcvMsg.startsWith("add@/devices")) // add
            {
                splits = rcvMsg.split("block/");
                if (isCDROM(splits.last()))
                {
                    LOGGER->info(QString("( MonitorCDROM::run ) ADD Device - Calling MAIN to add new inserted CDROM, block name is /dev/%1").arg(splits.last()));
                    emit addCDROM(QString("/dev/%1").arg(splits.last()));
                }
            }
            else if (rcvMsg.startsWith("change@/devices")) // change
            {
                splits = rcvMsg.split("block/");
                if (isCDROM(splits.last()))
                {
                    LOGGER->info(QString("( MonitorCDROM::run ) CHANGE Device - Calling MAIN to change old CDROM, block name is /dev/%1").arg(splits.last()));
                    emit changeCDROM(QString("/dev/%1").arg(splits.last()));
                }
            }
            else if (rcvMsg.startsWith("remove@/devices")) // remove
            {
                splits = rcvMsg.split("block/");
                if (isCDROM(splits.last()))
                {
                    LOGGER->info(QString("( MonitorCDROM::run ) AEMOVE Device - Calling MAIN to remove old CDROM, block name is /dev/%1").arg(splits.last()));
                    emit removeCDROM(QString("/dev/%1").arg(splits.last()));
                }
            }
            else
            {
            }
        }
    }
}

int MonitorCDROM::preception()
{
    const int bufferSize = 16 * 1024 * 1024;
    struct sockaddr_nl netlink;

    ::memset(&netlink, 0, sizeof(struct sockaddr_nl));
    netlink.nl_family = AF_NETLINK;
    netlink.nl_pid = ::getpid();
    netlink.nl_groups = -1;

    m_iPreception = ::socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (m_iPreception < 0)
    {
        LOGGER->error(_("Build netlink socket error. %1").arg(::strerror(errno)));
        return -1;
    }

    setsockopt(m_iPreception, SOL_SOCKET, SO_RCVBUFFORCE, &bufferSize, sizeof(bufferSize));
    if (::bind(m_iPreception, (struct sockaddr *)(&netlink), sizeof(struct sockaddr_nl)) < 0)
    {
        LOGGER->error(_("Bind netlink socket error. %1").arg(::strerror(errno)));
        ::close(m_iPreception);
        return -1;
    }

    return m_iPreception;
}
/******************************************************
 * 从CDROM文件中更新块设备变量m_oBlocks
 * 读/proc/sys/dev/cdrom/info获得srN-->/dev/srN-->加入m_oBlocks
 ******************************************************/
void MonitorCDROM::update()
{
    std::ifstream in;
    std::string str;
    QString line;
    QStringList splits;

    //QThread::msleep(200);
    in.open(CDROM);
    if (!in.is_open()) // open failed.
    {
        LOGGER->error(_("Open system path %1 failed.%2").arg(CDROM).arg(::strerror(errno)));
        return;
    }

    while (!line.startsWith("drive name:") && std::getline(in, str))
    {
        line.clear();
        line = QString::fromStdString(str);
    }
    if (!line.startsWith("drive name:")) // search block name failed.
    {
        LOGGER->error(_("Cannot search block from %1").arg(CDROM));
        return;
    }

    splits = line.split("\t");
    if (splits.size() < 3) // no block name
    {
        LOGGER->warn(_("There is no block in current system"));
        return;
    }

    m_oBlocks.clear();
    for (int i = 2; i < splits.size(); ++i)
    {
        if (splits.at(i).isEmpty()) continue;
        line.clear();
        line = QString("/dev/%1").arg(splits.at(i));
        LOGGER->info(_("Update block named %1 into application").arg(line));
        m_oBlocks << line;
    }
}

bool MonitorCDROM::isCDROM(QString block)
{
    QString blockName;

    update();
    if (m_oBlocks.size() == 0) return false;

    blockName = block;
    if (!blockName.startsWith("/dev/"))
    {
        blockName = QString("/dev/%1").arg(block);
    }

    if (-1 == m_oBlocks.indexOf(blockName))
    {
        return false;
    }
    return true;
}

MonitorCDROM::~MonitorCDROM()
{}
