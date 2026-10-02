#include "upload_helper.h"
#include "common.h"
#include <QThread>
#include <QTimer>

UploadHelper::UploadHelper(DeviceInformation info, int ppdRank, QObject *parent)
    : m_info(info), m_ppdRank(ppdRank), QObject(parent)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &UploadHelper::dowork);

    connect(this, &UploadHelper::finished, m_thread, &QThread::quit);
    connect(this, &UploadHelper::finished, this, &UploadHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

UploadHelper::~UploadHelper()
{

}

void UploadHelper::dowork()
{
    postDeviceInfo(m_info, m_ppdRank);


    finished();
}
