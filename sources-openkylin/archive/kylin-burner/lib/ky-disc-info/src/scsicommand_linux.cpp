#include "scsicommand.h"
#include "device.h"

#include <QDebug>

#include <sys/ioctl.h>
#undef __STRICT_ANSI__
#include <linux/cdrom.h>
#define __STRICT_ANSI__
#include <scsi/sg.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>

class Device::ScsiCommand::Private
{
public:
    struct cdrom_generic_command cmd;
    struct request_sense sense;

#ifdef SG_IO
    bool useSgIo;
    struct sg_io_hdr sgIo;
#endif
};

QString Device::commandString( const unsigned char& command )
{
    if( command == MMC_BLANK )
        return "BLANK";
    if( command == MMC_CLOSE_TRACK_SESSION )
        return "CLOSE TRACK/SESSION";
    if( command == MMC_ERASE )
        return "ERASE";
    if( command == MMC_FORMAT_UNIT )
        return "FORMAT UNIT";
    if( command == MMC_GET_CONFIGURATION )
        return "GET CONFIGURATION";
    if( command == MMC_GET_EVENT_STATUS_NOTIFICATION )
        return "GET EVENT STATUS NOTIFICATION";
    if( command == MMC_GET_PERFORMANCE )
        return "GET PERFORMANCE";
    if( command == MMC_INQUIRY )
        return "INQUIRY";
    if( command == MMC_LOAD_UNLOAD_MEDIUM )
        return "LOAD/UNLOAD MEDIUM";
    if( command == MMC_MECHANISM_STATUS )
        return "MECHANISM STATUS";
    if( command == MMC_MODE_SELECT )
        return "MODE SELECT";
    if( command == MMC_MODE_SENSE )
        return "MODE SENSE";
    if( command == MMC_PAUSE_RESUME )
        return "PAUSE/RESUME";
    if( command == MMC_PLAY_AUDIO_10 )
        return "PLAY AUDIO (10)";
    if( command == MMC_PLAY_AUDIO_12 )
        return "PLAY AUDIO (12)";
    if( command == MMC_PLAY_AUDIO_MSF )
        return "PLAY AUDIO (MSF)";
    if( command == MMC_PREVENT_ALLOW_MEDIUM_REMOVAL )
        return "PREVENT ALLOW MEDIUM REMOVAL";
    if( command == MMC_READ_10 )
        return "READ (10)";
    if( command == MMC_READ_12 )
        return "READ (12)";
    if( command == MMC_READ_BUFFER )
        return "READ BUFFER";
    if( command == MMC_READ_BUFFER_CAPACITY )
        return "READ BUFFER CAPACITY";
    if( command == MMC_READ_CAPACITY )
        return "READ CAPACITY";
    if( command == MMC_READ_CD )
        return "READ CD";
    if( command == MMC_READ_CD_MSF )
        return "READ CD MSF";
    if( command == MMC_READ_DISC_INFORMATION )
        return "READ DISC INFORMATION";
    if( command == MMC_READ_DVD_STRUCTURE )
        return "READ DVD STRUCTURE";
    if( command == MMC_READ_FORMAT_CAPACITIES )
        return "READ FORMAT CAPACITIES";
    if( command == MMC_READ_SUB_CHANNEL )
        return "READ SUB-CHANNEL";
    if( command == MMC_READ_TOC_PMA_ATIP )
        return "READ TOC/PMA/ATIP";
    if( command == MMC_READ_TRACK_INFORMATION )
        return "READ TRACK INFORMATION";
    if( command == MMC_REPAIR_TRACK )
        return "REPAIR TRACK";
    if( command == MMC_REPORT_KEY )
        return "REPORT KEY";
    if( command == MMC_REQUEST_SENSE )
        return "REQUEST SENSE";
    if( command == MMC_RESERVE_TRACK )
        return "RESERVE TRACK";
    if( command == MMC_SCAN )
        return "SCAN";
    if( command == MMC_SEEK_10 )
        return "SEEK (10)";
    if( command == MMC_SEND_CUE_SHEET )
        return "SEND CUE SHEET";
    if( command == MMC_SEND_DVD_STRUCTURE )
        return "SEND DVD STRUCTURE";
    if( command == MMC_SEND_KEY )
        return "SEND KEY";
    if( command == MMC_SEND_OPC_INFORMATION )
        return "SEND OPC INFORMATION";
    if( command == MMC_SET_SPEED )
        return "SET SPEED";
    if( command == MMC_SET_READ_AHEAD )
        return "SET READ AHEAD";
    if( command == MMC_SET_STREAMING )
        return "SET STREAMING";
    if( command == MMC_START_STOP_UNIT )
        return "START STOP UNIT";
    if( command == MMC_STOP_PLAY_SCAN )
        return "STOP PLAY/SCAN";
    if( command == MMC_SYNCHRONIZE_CACHE )
        return "SYNCHRONIZE CACHE";
    if( command == MMC_TEST_UNIT_READY )
        return "TEST UNIT READY";
    if( command == MMC_VERIFY_10 )
        return "VERIFY (10)";
    if( command == MMC_WRITE_10 )
        return "WRITE (10)";
    if( command == MMC_WRITE_12 )
        return "WRITE (12)";
    if( command == MMC_WRITE_AND_VERIFY_10 )
        return "WRITE AND VERIFY (10)";
    if( command == MMC_WRITE_BUFFER )
        return "WRITE BUFFER";

    return "unknown";
}


QString Device::ScsiCommand::senseKeyToString( int key )
{
    switch( key ) {
    case 0x0:
        return "NO SENSE (2)";
    case 0x1:
        return "RECOVERED ERROR (1)";
    case 0x2:
        return "NOT READY (2)";
    case 0x3:
        return "MEDIUM ERROR (3)";
    case 0x4:
        return "HARDWARE ERROR (4)";
    case 0x5:
        return "ILLEGAL REQUEST (5)";
    case 0x6:
        return "UNIT ATTENTION (6)";
    case 0x7:
        return "DATA PROTECT (7)";
    case 0x8:
        return "BLANK CHECK (8)";
    case 0x9:
        return "VENDOR SPECIFIC (9)";
    case 0xA:
        return "COPY ABORTED (A)";
    case 0xB:
        return "ABORTED COMMAND (B)";
    case 0xC:
        return "0xC is obsolete... ??";
    }

    return "unknown";
}


void Device::ScsiCommand::debugError( int command, int errorCode, int senseKey, int asc, int ascq ) {
    if( m_printErrors ) {
        qDebug() << "(Device::ScsiCommand) failed: ";
        qDebug() << "                           command:    " << QString("%1 (%2)")
            .arg( commandString( command ) )
            .arg( QString::number(command, 16) ) ;
        qDebug()  << "                           errorcode:  " << QString::number(errorCode, 16);
        qDebug()  << "                           sense key:  " << senseKeyToString(senseKey);
        qDebug()  << "                           asc:        " << QString::number(asc, 16);
        qDebug()  << "                           ascq:       " << QString::number(ascq, 16);
    }
}




Device::ScsiCommand::ScsiCommand( const Device* dev )
    : d(new Private),
      m_device(dev),
      m_printErrors(true)
{
    clear();
}


Device::ScsiCommand::~ScsiCommand()
{
    delete d;
}

#ifdef SG_IO
static bool useSgIo()
{
    struct utsname buf;
    uname( &buf );
    // was CDROM_SEND_PACKET declared dead in 2.5?
    return ( strcmp( buf.release, "2.5.43" ) >=0 );
}
#endif





void Device::ScsiCommand::clear()
{
    ::memset( &d->cmd, 0, sizeof(struct cdrom_generic_command) );
    ::memset( &d->sense, 0, sizeof(struct request_sense) );

    d->cmd.quiet = 1;
    d->cmd.sense = &d->sense;

#ifdef SG_IO
    d->useSgIo = useSgIo();
    ::memset( &d->sgIo, 0, sizeof(struct sg_io_hdr) );
#endif
}


unsigned char& Device::ScsiCommand::operator[]( size_t i )
{
#ifdef SG_IO
    if( d->sgIo.cmd_len < i+1 )
        d->sgIo.cmd_len = i+1;
#endif
    return d->cmd.cmd[i];
}


int Device::ScsiCommand::transport( TransportDirection dir,
                                         void* data,
                                         size_t len )
{
    bool needToClose = false;
    int deviceHandle = -1;
    if( m_device ) {
        m_device->usageLock();
        if( !m_device->isOpen() ) {
            needToClose = true;
        }
        if ( !m_device->open( dir == TR_DIR_WRITE ) ) {
            m_device->usageUnlock();
            return -1;
        }
        deviceHandle = m_device->handle();
    }

    if( deviceHandle == -1 ) {
        return -1;
    }

    int i = -1;

#ifdef SG_IO
    if( d->useSgIo ) {
        d->sgIo.interface_id= 'S';
        d->sgIo.mx_sb_len = sizeof( struct request_sense );
        d->sgIo.cmdp      = d->cmd.cmd;
        d->sgIo.sbp       = (unsigned char*)&d->sense;
        d->sgIo.flags     = SG_FLAG_LUN_INHIBIT|SG_FLAG_DIRECT_IO;
        d->sgIo.dxferp    = data;
        d->sgIo.dxfer_len = len;
        d->sgIo.timeout   = 5000;
        if( dir == TR_DIR_READ )
            d->sgIo.dxfer_direction = SG_DXFER_FROM_DEV;
        else if( dir == TR_DIR_WRITE )
            d->sgIo.dxfer_direction = SG_DXFER_TO_DEV;
        else
            d->sgIo.dxfer_direction = SG_DXFER_NONE;

        i = ::ioctl( deviceHandle, SG_IO, &d->sgIo );

        if( ( d->sgIo.info&SG_INFO_OK_MASK ) != SG_INFO_OK )
            i = -1;
    }
    else {
#endif
        d->cmd.buffer = (unsigned char*)data;
        d->cmd.buflen = len;
        if( dir == TR_DIR_READ )
            d->cmd.data_direction = CGC_DATA_READ;
        else if( dir == TR_DIR_WRITE )
            d->cmd.data_direction = CGC_DATA_WRITE;
        else
            d->cmd.data_direction = CGC_DATA_NONE;

        i = ::ioctl( deviceHandle, CDROM_SEND_PACKET, &d->cmd );
#ifdef SG_IO
    }
#endif

    if( needToClose )
        m_device->close();

    if ( m_device ) {
        m_device->usageUnlock();
    }

    if( i ) {
        debugError( d->cmd.cmd[0],
                    d->sense.error_code,
                    d->sense.sense_key,
                    d->sense.asc,
                    d->sense.ascq );

        int errCode =
            ((d->sense.error_code<<24) & 0xF000) |
            ((d->sense.sense_key<<16)  & 0x0F00) |
            ((d->sense.asc<<8)         & 0x00F0) |
            ((d->sense.ascq)           & 0x000F);

        return( errCode != 0 ? errCode : 1 );
    }
    else
        return 0;
}
