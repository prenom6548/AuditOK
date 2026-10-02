#include <QTimer>
#include <QDateTime>
#include <QThread>
#include "cupsconnection4ppds.h"
#include "ukui_printer.h"

//QMutex CupsConnection4PPDs::m_mutex;
CupsConnection4PPDs::CupsConnection4PPDs()
{
    qDebug() << "CUPS开始建立" << QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    m_server = cupsServer();

    m_ippPortNum = ippPort();

    m_encryptionType = cupsEncryption();

    int cancel = 0;
    m_ppdRequestConnection = httpConnect2(m_server, m_ippPortNum, nullptr, AF_UNSPEC, (http_encryption_t)m_encryptionType, 1, 30000, &cancel);

    if (!m_ppdRequestConnection)
    {
        throw std::runtime_error("Failed to construct ppdRequestConnection!");
    }
    else
    {
        qDebug() << "CUPS服务ppdRequestConnection建立成功!";
    }


    qDebug() << "CUPS建立完成" << QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
}

CupsConnection4PPDs::~CupsConnection4PPDs()
{
    delete m_server;
}
http_t* CupsConnection4PPDs::getCupsConnection() const
{
    return m_ppdRequestConnection;
}