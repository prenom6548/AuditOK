/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "smbclient_helper.h"
#include "common.h"
#include "connect.h"
#include <QDebug>
#include <samba-4.0/libsmbclient.h>

#ifdef USE_SELF_CONTAINED_DEP
    #include "securec.h"
#else
    #include <boundscheck/securec.h>
#endif // USE_SELF_CONTAINED_DEP


void get_auth_data_with_context_fn(SMBCCTX * context,
                                    const char * pServer,
                                    const char * pShare,
                                    char * pWorkgroup,
                                    int maxLenWorkgroup,
                                    char * pUsername,
                                    int maxLenUsername,
                                    char * pPassword,
                                    int maxLenPassword)
{
    SMBAuthData *user_data;
    
    if (context != NULL) {
        user_data = (SMBAuthData *)smbc_getOptionUserData(context);
    }

    if (user_data == nullptr) {
        return ;
    }

    if (user_data->workgroup.isEmpty()) {
        user_data->workgroup = QString(pWorkgroup);
        if (user_data->workgroup.isEmpty()) {
            user_data->workgroup = QString("WORKGROUP");
        }
    }

    if (user_data->username.isEmpty()) {
        user_data->username = QString(pUsername);
        if (user_data->username.isEmpty()) {
            user_data->username = QString("USERNAME");
        }
    }

    if (user_data->password.isEmpty()) {
        user_data->password = QString(pPassword);
        if (user_data->password.isEmpty()) {
            qDebug() << "smb passwd is empty!";
        }
    }

    strncpy_s(pWorkgroup, maxLenWorkgroup, QTC(user_data->workgroup), maxLenWorkgroup);
    strncpy_s(pUsername,  maxLenUsername,  QTC(user_data->username),  maxLenUsername);
    strncpy_s(pPassword,  maxLenPassword,  QTC(user_data->password),  maxLenPassword);

    pWorkgroup[maxLenWorkgroup - 1] = '\0';
    pUsername[maxLenUsername - 1] = '\0';
    pPassword[maxLenPassword - 1] = '\0';
}

int SmbClientHelper::browse(const QString &host)
{
    if (host.isEmpty()) {
        return -1;
    }

    SMBCCTX *context;

    /* Allocate a new context */
    context = smbc_new_context();
    if (!context) {
        qDebug("Could not allocate new smbc context\n");
        return -1;
    }

    /* Initialize the context using the previously specified options */
    if (!smbc_init_context(context)) {
        smbc_free_context(context, 0);
        qDebug("Could not initialize smbc context\n");
        return -1;
    }

    smbc_setOptionUserData(context, &m_data);
    smbc_setFunctionAuthDataWithContext(context,
                                        get_auth_data_with_context_fn);

    /* Tell the compatibility layer to use this context */
    smbc_set_context(context);

    struct smbc_dirent * dirent;
    QString path = QString("smb://%1").arg(host);
    int dir;
    errno = 0;
    if ((dir = smbc_opendir(QTC(path))) < 0)
    {
        qDebug() << dir;
        qDebug("Could not open directory [%s] (%d:%s)\n",
               QTC(path), errno, strerror(errno));
        smbc_free_context(context, 1);
        return errno;
    }

    while ((dirent = smbc_readdir(dir)) != NULL)
    {
        switch(dirent->smbc_type)
        {
            case SMBC_PRINTER_SHARE:
                qDebug("%s\n", dirent->name);
                m_data.smbPrinterList.append(constructSmbDevice(dirent->name));
                break;
            default:
                break;
        }
    }

    smbc_closedir(dir);
    smbc_free_context(context, 1);
    return 0;
}

int SmbClientHelper::cconnect(const QString &host, const QString &share)
{

    bool g_smbAuth;

    QString uri = QString("smb://%1/%2").arg(host).arg(share);
    SMBCCTX *ctx = nullptr;
    SMBCFILE *fd = nullptr;
    ctx = smbc_new_context();
    // smbc_setDebug(ctx, 10);
    errno = 0;
    smbc_setFunctionAuthDataWithContext(ctx,
                                        get_auth_data_with_context_fn);
    // smbc_setOptionUseKerberos(ctx, true);
    if (!smbc_init_context(ctx)) {
        smbc_free_context(ctx, 0);
        qDebug("Could not initialize smbc context\n");
        return 1;
    }

    qDebug("%d\n", errno);
    fd = smbc_getFunctionOpen (ctx)(ctx, QTC(uri), O_RDWR, 511);
    qDebug("%d\n", errno);
    if (!fd) {
        qDebug("--->Could not open dir, auth error!\n");
        return 1;
    }
    return 0;
}

SmbClientHelper::SmbClientHelper(SMBAuthData &data, QObject *parent)
    : m_data(data), QObject(parent)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &SmbClientHelper::dowork);
    connect(this, &SmbClientHelper::finished, m_thread, &QThread::quit);
    connect(this, &SmbClientHelper::finished, this, &SmbClientHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

SmbClientHelper::~SmbClientHelper()
{
}

bool SmbClientHelper::checkSmbServer(const QString &host)
{
    if (!pingCheck(host)) {
        return false;
    }

    if (try_connect(QTC(host), 445) != 0&& try_connect(QTC(host), 139) != 0) {
        return false;
    }

    return true;
}

void SmbClientHelper::dowork()
{
    if (!checkSmbServer(m_data.host)) {
        emit succeed(-1, tr("Cannot connect to %1!").arg(m_data.host));
        return;
    }

    if (m_data.username.isEmpty() && m_data.password.isEmpty()) {
        emit succeed(-2, tr("Wrong user name or password!"));
        return;
    }

    int status = browse(m_data.host);
    
    if (status != 0) {
        emit succeed(-2, tr("Wrong user name or password!"));
        return;
    }

    if (m_data.smbPrinterList.isEmpty()) {
        emit succeed(-3, tr("The samba server shared printer list is empty"));
        return;
    }
    emit succeed(0, "");
    return;
}

QString urlquote(const QString &x)
{
    QString q = encodeUrl(x);
    QByteArray bytes = q.toUtf8();
    for (char c : "/@:")
    {
        QString encoded = QString("%%%1").arg((quint8)c, 2, 16, QLatin1Char('0')).toUpper();
        bytes.replace(QChar(c), encoded.toUtf8());
    }
    return QString::fromUtf8(bytes);
}

QString SmbClientHelper::constructSmbUri(const QString &host,
                                         const QString &share,
                                         const QString &workgroup,
                                         const QString &username,
                                         const QString &password)
{
    // smb://user:passwd@group/host/share
    QString uri_password;
    if (password.size()) {
        uri_password = ":" + urlquote(password);
    }

    if (username.size()) {
        uri_password += "@";
    }

    QString uri = QString("smb://%1%2%3").arg(urlquote(username), uri_password, urlquote(workgroup));

    if (workgroup.size()) {
        uri += "/";
    }

    uri += urlquote(host);
    
    if (share.size()) {
        uri += "/" + urlquote(share);
    }
    return uri;
}


DeviceInformation SmbClientHelper::constructSmbDevice(const QString &share)
{
    DeviceInformation info;
    info.host = m_data.host;
    info.deviceType = DeviceType::SMB;
    info.protocolType = ProtocolType::SMB;
    info.name = share;
    info.uri = constructSmbUri(m_data.host, share, m_data.workgroup, m_data.username, m_data.password);
    return info;
}