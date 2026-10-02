#include "updatenotificationwidget.h"
#include <QMessageBox>
#include <QProcess>
UpdateNotificationWidget::UpdateNotificationWidget(PublicLocalModel &model, QWidget *parent)
    :QWidget(parent)
    ,m_currentState(LatestVersion)//默认为最新版本
    ,m_downloadProgress(0)
    ,m_downloading(false)
{
    m_modelDebName = model.debName;
    // 初始化 UI
    initLayout();
}

void UpdateNotificationWidget::initLayout()
{

    m_progressBar = new QProgressBar(this);
    m_progressBar->setAccessibleName("kylin-ai-subsystem-modelconfig_QProgressBar_m_progressBar");
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedSize(56,16);
    m_progressBar->setObjectName("progressbar");

    m_pauseButton = new QPushButton( this);
    m_pauseButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_m_pauseButton");
    m_pauseButton->setObjectName("puaseButton");
    connect(m_pauseButton,&QPushButton::clicked,this,[=]{

    });
    m_pauseButton->setFixedSize(16,16);
    m_pauseButton->setIcon(QIcon::fromTheme("media-playback-pause-symbolic"));
    m_pauseButton->setIconSize(QSize(16,16));
    m_pauseButton->setProperty("isWindowButton", 0x1);
    m_pauseButton->setFlat(true);
    m_pauseButton->setVisible(false);
    m_pauseButton->setProperty("useButtonPalette", true);


    m_closeButton = new QPushButton(this);
    m_closeButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_m_closeButton");
    connect(m_closeButton,&QPushButton::clicked,this,&UpdateNotificationWidget::slClickExitButton);
    m_closeButton->setObjectName("cancelButton");
    m_closeButton->setFixedSize(16,16);
    m_closeButton->setProperty("isWindowButton", QVariant(0x1));
    m_closeButton->setProperty("useButtonPalette", true);
    m_closeButton->setFlat(true);
    m_closeButton->setIcon(QIcon::fromTheme("application-exit-symbolic"));
    m_closeButton->setIconSize(QSize(16,16));


    // 布局设置
    m_mainHLayout = new QHBoxLayout();
    m_mainHLayout->addWidget(m_progressBar);
    m_mainHLayout->addWidget(m_pauseButton);
    m_mainHLayout->addWidget(m_closeButton);
    this->setLayout(m_mainHLayout);
}

void UpdateNotificationWidget::resetProgressBar()
{
    m_progressBar->setValue(0);
    m_closeButton->setVisible(true);
}

bool UpdateNotificationWidget::checkNetWorkOnline()
{
    CURL *curl;
    CURLcode res;
    long http_code = 0;
    QString targetUrl = getDownloadUrl();
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, targetUrl.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // 只获取头部信息
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 设置超时时间为 5 秒

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code == 200) {
                curl_easy_cleanup(curl);
                return true; // 网络可用
            }
        }
        curl_easy_cleanup(curl);
    }
    KyInfo() << "curl error code: " << res;
    return false; // 网络不可用
}

void UpdateNotificationWidget::setCancelDownloadButtonVisible(bool isVisible)
{
    //在下载返回的进度值大于50时不显示取消下载按钮及进度条显示并显示安装中的提示
    if(m_closeButton->isVisible() == true  && isVisible == false)
    {
        m_progressBar->setValue(100);
        m_progressBar->setVisible(false);
        //设置显示安装中
        emit sigShowIsInstalling();
        m_closeButton->setVisible(isVisible);
    }
}

void UpdateNotificationWidget::setProgressBarVisible(bool isVisible)
{
    if(m_progressBar)
    {
        m_progressBar->setVisible(isVisible);
    }
}

void UpdateNotificationWidget::setCancelDownloadButtenEnabled(bool isEnabled)
{
    if(m_closeButton)
    {
        m_closeButton->setEnabled(isEnabled);
    }
}

QString UpdateNotificationWidget::getDownloadUrl()
{
    QString targetUrl = "";
    if(m_modelDebName.isEmpty()) {
        targetUrl = kaglobal::baiduUrl;
        return targetUrl;
    }
    else {
        QProcess process;
        QString cmd1 = QString("apt show %1 | grep APT-Sources | cut -d ' ' -f 2 ").arg(m_modelDebName);
        process.start("bash", QStringList() << "-c" << cmd1);
        process.waitForFinished();
        QString outPutStr = process.readAllStandardOutput();
        KyInfo() << "ffffffffffffffffffffffffffff:" <<outPutStr;
        QString url;
        if(outPutStr.trimmed().isEmpty())
        {
            url = kaglobal::baiduUrl;
        }
        else if(outPutStr.contains("launchpad")) {
            url = kaglobal::launchpadUrl;
        }
        else if(outPutStr.contains("archive.kylinos.cn") || outPutStr.contains("archive2.kylinos.cn"))
        {
            url = kaglobal::archiveUrl;
        }
        else {
            url  = kaglobal::baiduUrl;
        }
        return url;
    }
}

void UpdateNotificationWidget::slUpdateProgressValue(int value)
{
    //正在安装中不显示进度条进度变化
    bool isInstall =  ModelDownload::getInstance()->getCurrentInstallStatus();
    if(isInstall)
    {
        return;
    }
    if(m_progressBar && m_progressBar->isVisible())
    {
        m_progressBar->setValue(value);
    }

}

void UpdateNotificationWidget::slClickExitButton()
{
    bool isInstall =  ModelDownload::getInstance()->getCurrentInstallStatus();
    //非安装安装状态下可直接取消安装
    if(!isInstall)
    {
        ModelDownload::getInstance()->cancelDownloading();
    }

}

bool UpdateNotificationWidget::getNetStatus()
{
    return m_netStaus;
}
