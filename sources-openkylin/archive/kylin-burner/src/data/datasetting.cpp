/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "datasetting.h"
#include "src/po/translation.h"

#include <pwd.h>
#include <unistd.h>

#include <QDate>
#include <QDebug>
#include <QDir>

DataSetting::DataSetting(QObject *parent) :
    QObject(parent),
    m_iSpeed(0),
    m_SBurnFormat(QString("ISO9660")),
    m_bClean(false),
    m_bCloseSession(false),
    m_bCreateISO(false),
    m_bOpen(false),
    m_bVerficationData(false),
    m_isShowBeforBurn(true)
{
    struct passwd *pwd = ::getpwuid(::getuid());
    if (pwd) {
        m_oPublisher = QString::fromUtf8(pwd->pw_name);
    } else {
        m_oPublisher = QString("KylinBurner");
    }
    m_oPreparer = _("KylinSoft");
    m_oVolumnID = _("KylinBurner");
    m_oCopyright = _("Kylinsoft © %1 All rights.").arg(QDate::currentDate().toString("yyyy"));

    QDir dir;
    dir.setPath(QDir::homePath() + "/.cache/KylinBurner/");
    if (!dir.exists())
        dir.mkpath(dir.absolutePath());
    m_oTemporaryPth = dir.absolutePath();
}

DataSetting::DataSetting(DataSetting &other, QObject *parent)
{
    parent = other.parent();
    m_iSpeed = other.getSpeed();
    m_bClean = other.getClean();
    m_bCreateISO = other.getCreateISO();
    m_bCloseSession = other.getCloseSession();
    m_bVerficationData = other.getVerficationData();
    m_bOpen = other.getOpen();
    m_oPreparer = other.getPreparer();
    m_oPublisher = other.getPublisher();
    m_oVolumnID = other.getVolumnID();
    m_oCopyright = other.getCopyright();
    m_oTemporaryPth = other.getTempPath();
    m_SBurnFormat = other.getBurnFormat();
    m_isShowBeforBurn = other.getIsShowBeforBurn();
}

void DataSetting::load()
{
    speed(m_iSpeed);
    clean(m_bClean);
    createISO(m_bCreateISO);
    closeSession(m_bCloseSession);
    open(m_bOpen);
    preparer(m_oPreparer);
    publisher(m_oPublisher);

    copyright(m_oCopyright);
    tempPath(m_oTemporaryPth);
    burnFormat(m_SBurnFormat);
    IsShowBeforBurn(m_isShowBeforBurn);
}

DataSetting::~DataSetting()
{}
