/*
 *
 * Copyright (C) 2003-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef ISO9660_H
#define ISO9660_H

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QStringList>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef HAVE_STAT64
#define k3b_struct_stat struct stat64
#define k3b_stat        ::stat64
#define k3b_lstat       ::lstat64
#else
#define k3b_struct_stat struct stat
#define k3b_stat        ::stat
#define k3b_lstat       ::lstat
#endif

struct iso_directory_record;
struct el_torito_boot_descriptor;
struct iso_primary_descriptor;
typedef long sector_t;


class Iso9660;
class Iso9660Backend;

/**
 * Simplyfied primary descriptor which just contains the fields
 * used by K3b.
 */
class  Iso9660SimplePrimaryDescriptor
{
public:
    /**
     * Creates an empty descriptor
     */
    Iso9660SimplePrimaryDescriptor();

    QString volumeId;
    QString systemId;
    QString volumeSetId;
    QString publisherId;
    QString preparerId;
    QString applicationId;
    int volumeSetSize;
    int volumeSetNumber;
    long logicalBlockSize;
    long long volumeSpaceSize;
    long long getUsedCapacity() { return logicalBlockSize* volumeSpaceSize; }
    long long getVolumSpacesSize() {return volumeSpaceSize; }
    QString getVolumeID() {return volumeId;}
};


/**
 * This class is based on the KIso class by
 * Gy�gy Szombathelyi <gyurco@users.sourceforge.net>.
 * A lot has been changed and bugfixed.
 * The API has been improved to be useful.
 *
 * Due to the stupid Qt which does not support large files as default
 * we cannot use QIODevice with DVDs! That's why we have our own
 * reading code which is not allowed by KArchive (which is limited to int
 * by the way... who the hell designed this?)
 * I also removed the KArchive inheritance because of the named reasons.
 * So this stuff contains a lot KArchive code which has been made usable.
 *
 * That does not mean that this class is well designed. No, it's not. :)
 *
 * Opening a Iso9660 object should be fast since creation of the directory
 * and file entries is not done until a call to Iso9660Directory::entries.
 */
class  Iso9660
{
public:

    /**
     * Directly specify the backend to read from.
     * Iso9660 will take ownership of the backend and delete it.
     */
    explicit Iso9660( Iso9660Backend* );

    /**
     * If the .iso is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~Iso9660();

    /**
     * Set where to start reading in the source.
     */
    void setStartSector( unsigned int startSector );

    /**
     * If set to true before opening Iso9660 will ignore RR and joliet extensions
     * and only create plain iso9660 names.
     */
    void setPlainIso9660( bool );

    bool plainIso9660() const;

    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the Iso9660Directory/Iso9660File entries.
     */
    bool open();

    bool isOpen() const;

    /**
     * Closes everything.
     * This is also called in the destructor
     */
    void close();

    /**
     * @param sector startsector
     * @param len number of sectors
     * @return number of sectors read or -1 on error
     */
    int read( unsigned int sector, char* data, int len );

    /**
     * The name of the os file, as passed to the constructor
     * Null if you did not use the QString constructor.
     */
    QString fileName() { return m_filename; }

    const Iso9660SimplePrimaryDescriptor& primaryDescriptor() const;

    void debug() const;

private:

    void createSimplePrimaryDesc( struct iso_primary_descriptor* desc );

    // only used for creation
    static int read_callback( char* buf, sector_t start, int len, void* udata );

    bool m_rr;


private:
    QString m_filename;

    class Private;
    Private * d;
};


#endif // ISO9660_H
