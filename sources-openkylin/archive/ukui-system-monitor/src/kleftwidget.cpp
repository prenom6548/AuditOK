/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "kleftwidget.h"
#include "macro.h"
#include "util.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QStringList>
#include <QScrollBar>

#include <kysdk/kysdk-system/libkyrtinfo.h>

#define PERSONALSIE_SCHEMA     "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define CONTAIN_PERSONALSIE_TRAN_KEY   "transparency"

typedef struct _net
{
    guint64 last_in, last_out;
    gint64 time;
} net;
net netinfo;

inline void getNetworkBytesData(unsigned long long int &receiveBytes, unsigned long long int &sendBytes, unsigned long long int &receiveRateBytes, unsigned long long int &sendRateBytes)
{
    QStringList netlist;
    guint32 i;
    guint64 in = 0, out = 0;
    gint64 time;
    guint64 din = 0, dout = 0;
    netlist = netInfo::getAllCards();

    for (i = 0; i < netlist.count(); ++i) {
        const QString &netCard = netlist.at(i);
        std::string stdstr = netCard.toStdString();
        const char *ncName = stdstr.c_str();
        in  += kdk_nc_get_rx_bytes(ncName);
        out += kdk_nc_get_tx_bytes(ncName);
    }

    time = g_get_real_time();

    if (in >= netinfo.last_in && out >= netinfo.last_out && netinfo.time != 0) {
        float dtime;
        dtime = (double)(time - netinfo.time) / G_USEC_PER_SEC;
        din   = static_cast<guint64>((in  - netinfo.last_in)  / dtime);
        dout  = static_cast<guint64>((out - netinfo.last_out) / dtime);
    } else {
        /* Don't calc anything if new data is less than old (interface
           removed, counters reset, ...) or if it is the first time */
        din  = 0;
        dout = 0;
    }

    netinfo.last_in  = in;
    netinfo.last_out = out;
    netinfo.time     = time;

    receiveBytes = in;
    sendBytes = out;
    receiveRateBytes = din;
    sendRateBytes = dout;
}

KLeftWidget::KLeftWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setFixedWidth(LEFT_WIDGET_WIDTH);
    m_prevCpuData = cpuInfo::getAllCpuUsageData();
    initUI();
    initConnections();
}

KLeftWidget::~KLeftWidget()
{
}

void KLeftWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(16);
    m_itemsLayout = new QVBoxLayout();
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(0);
    m_resLayout = new QVBoxLayout();
    m_resLayout->setContentsMargins(0, 0, 0, 0);
    m_resLayout->setSpacing(12);
    m_layoutOptions = new QVBoxLayout();
    m_layoutOptions->setContentsMargins(10, 0, 4, 20);
    m_layoutOptions->setSpacing(16);

    // 初始化控件
    m_frameOptions = new QFrame();
    m_frameOptions->setLayout(m_layoutOptions);
    m_scrollOptions = new QScrollArea();
    m_scrollOptions->setContentsMargins(0, 0, 0, 0);
    m_scrollOptions->setBackgroundRole(QPalette::Window);
    m_scrollOptions->setAutoFillBackground(true);
    m_scrollOptions->setFrameStyle(0);
    m_scrollOptions->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollOptions->setWidgetResizable(true);
    m_scrollOptions->setWidget(m_frameOptions);
    m_scrollOptions->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::transparent);
    m_scrollOptions->setPalette(pal);
    m_scrollOptions->viewport()->setPalette(pal);

    m_scrollOptions->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_scrollOptions->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_itemsLayout->setAlignment(Qt::AlignTop);
    QStandardItem *processItem = new QStandardItem(QIcon::fromTheme("ukui-xtjs-all-process-symbolic"), tr("Processes"));
    QStandardItem *serviceItem = new QStandardItem(QIcon::fromTheme("ukui-xtjs-process-symbolic"), tr("Services"));
    QStandardItem *disksItem = new QStandardItem(QIcon::fromTheme("drive-harddisk-symbolic"), tr("Disks"));
    m_buttonBar = new KNavigationBar(this);
    m_buttonBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_buttonBar->setMinimumHeight(120);
    m_buttonBar->addItem(processItem);
    m_buttonBar->addItem(serviceItem);
    m_buttonBar->addItem(disksItem);
    m_buttonBar->listview()->setCurrentIndex(processItem->index());

    QWidget *buttonBarWidget = new QWidget();
    buttonBarWidget->setFixedWidth(LEFT_BUTTON_WIDTH);
    buttonBarWidget->setLayout(m_itemsLayout);

    m_itemsLayout->addWidget(m_buttonBar);
    m_layoutOptions->addWidget(buttonBarWidget);
    // 资源布局
    m_layoutOptions->addLayout(m_resLayout);
    m_resLayout->setAlignment(Qt::AlignBottom);
    m_cpuHisWidget = new CpuHistoryWidget();
    m_cpuHisWidget->setFixedWidth(LEFT_BUTTON_WIDTH);
    m_resLayout->addWidget(m_cpuHisWidget);

    m_memHisWidget = new MemHistoryWidget();
    m_memHisWidget->setFixedWidth(LEFT_BUTTON_WIDTH);
    m_resLayout->addWidget(m_memHisWidget);

    m_netHisWidget = new NetHistoryWidget();
    m_netHisWidget->setFixedWidth(LEFT_BUTTON_WIDTH);
    m_resLayout->addWidget(m_netHisWidget);

    m_pDiskHisWidget = new DiskHistoryWidget();
    m_pDiskHisWidget->setFixedWidth(LEFT_BUTTON_WIDTH);
    m_resLayout->addWidget(m_pDiskHisWidget);

    m_mainLayout->addWidget(m_scrollOptions);
    this->setLayout(m_mainLayout);
    m_updateStatusTimer = new QTimer(this);
}

void KLeftWidget::initConnections()
{
    connect(m_buttonBar->listview(), &QListView::clicked, this, [=](const QModelIndex &index){
        m_cpuHisWidget->setClickedFlags(false);
        m_memHisWidget->setClickedFlags(false);
        m_netHisWidget->setClickedFlags(false);
        m_pDiskHisWidget->setClickedFlags(false);
        Q_EMIT switchPage(index.row());
    });
    connect(m_cpuHisWidget, &CpuHistoryWidget::clicked, this, [=](){
        Q_EMIT switchPage(m_buttonBar->model()->rowCount());
    });
    connect(m_memHisWidget, &MemHistoryWidget::clicked, this, [=](){
        Q_EMIT switchPage(m_buttonBar->model()->rowCount()+1);
    });
    connect(m_netHisWidget, &NetHistoryWidget::clicked, this, [=](){
        Q_EMIT switchPage(m_buttonBar->model()->rowCount()+2);
    });

    connect(m_pDiskHisWidget, &DiskHistoryWidget::clicked, this, [=](){
        Q_EMIT switchPage(m_buttonBar->model()->rowCount()+3);
    });

    connect(m_updateStatusTimer, SIGNAL(timeout()), this, SLOT(onResourceUpdate()));
    connect(this, &KLeftWidget::updateCpuStatus, m_cpuHisWidget, &CpuHistoryWidget::onUpdateData);
    connect(this, &KLeftWidget::updateMemStatus, m_memHisWidget, &MemHistoryWidget::onUpdateMemData);
    connect(this, &KLeftWidget::updateSwapStatus, m_memHisWidget, &MemHistoryWidget::onUpdateSwapData);
    connect(this, &KLeftWidget::onUpdateNetStatus, m_netHisWidget, &NetHistoryWidget::onUpdateData);
    connect(this, &KLeftWidget::onUpdateDiskStatus, m_pDiskHisWidget, &DiskHistoryWidget::onUpdateDiskData);
    m_updateStatusTimer->start(2000);
}

void KLeftWidget::onSwichPanel(int nIndex)
{
    setleftClickedStatus(nIndex);
    m_buttonBar->listview()->setCurrentIndex(m_buttonBar->model()->index(nIndex,0));
}

void KLeftWidget::setleftClickedStatus(int nIndex)
{
    qDebug() << "row count = " << m_buttonBar->model()->rowCount() << nIndex;

    switch (nIndex) {
    case 3:
        m_memHisWidget->setClickedFlags(false);
        m_netHisWidget->setClickedFlags(false);
        m_pDiskHisWidget->setClickedFlags(false);
        break;
    case 4:
        m_cpuHisWidget->setClickedFlags(false);
        m_netHisWidget->setClickedFlags(false);
        m_pDiskHisWidget->setClickedFlags(false);
        break;
    case 5:
        m_cpuHisWidget->setClickedFlags(false);
        m_memHisWidget->setClickedFlags(false);
        m_pDiskHisWidget->setClickedFlags(false);
        break;
    case 6:
        m_cpuHisWidget->setClickedFlags(false);
        m_memHisWidget->setClickedFlags(false);
        m_netHisWidget->setClickedFlags(false);
        break;
    default:
        m_cpuHisWidget->setClickedFlags(false);
        m_memHisWidget->setClickedFlags(false);
        m_netHisWidget->setClickedFlags(false);
        m_pDiskHisWidget->setClickedFlags(false);
        break;
    }
}

int KLeftWidget::currentIndex()
{
    return m_buttonBar->listview()->currentIndex().row();
}

void KLeftWidget::onResourceUpdate()
{
    QVector<CpuUsageData> currentData = cpuInfo::getAllCpuUsageData();
    QVector<double> usage = cpuInfo::calculateCpuUsage(m_prevCpuData, currentData);
    m_prevCpuData = currentData;
    emit updateCpuStatus(usage);

    //swap and memory data get signal
    qreal swapTotal = kdk_rti_get_mem_swap_total_KiB();
    qreal swapUsed =kdk_rti_get_mem_swap_usage_KiB();

    unsigned long long lluSwapSize = 0, lluSwapUsed = 0;
    if (getSwaponData(lluSwapSize, lluSwapUsed)) {
        swapUsed = lluSwapUsed*1024;
        swapTotal = lluSwapSize*1024;
    }

    qreal memUsed = kdk_rti_get_mem_res_usage_KiB();
    qreal memTotal = kdk_rti_get_mem_res_total_KiB();

    emit updateMemStatus(memUsed, memTotal);
    emit updateSwapStatus(swapUsed, swapTotal);

    qDebug() << Q_FUNC_INFO << "swapUsed = " << swapUsed << ";swapTotal = " << swapTotal;

    //net work data get signal
    getNetworkBytesData(m_totalRecvBytes, m_totalSentBytes, m_rateRecvBytes, m_rateSentBytes);
    emit onUpdateNetStatus(m_totalSentBytes, m_rateSentBytes, m_totalRecvBytes, m_rateRecvBytes);
    emit onUpdateDiskStatus();
}

bool KLeftWidget::getSwaponData(unsigned long long &swapSize, unsigned long long &swapUsed)
{
    bool bSucessed = false;
    QProcess proc;
    QStringList options;
    options << "-s";
    proc.start("swapon", options);
    proc.waitForFinished();
    QString dpkgInfo = proc.readAll();
    QStringList infoList = dpkgInfo.split("\n");
    for (int n = 0; n < infoList.size(); n++) {
        QString strInfoLine = infoList[n];
        if (strInfoLine.contains("swap_file")) {
            QStringList lineInfoList = strInfoLine.split(QRegExp("[\\s]+"));
            if (lineInfoList.size() >= 4) {
                lineInfoList[2] = lineInfoList[2].trimmed();
                lineInfoList[3] = lineInfoList[3].trimmed();
                swapSize = lineInfoList[2].toULongLong();
                swapUsed = lineInfoList[3].toULongLong();
                bSucessed = true;
            }
            break;
        }
    }
    return bSucessed;
}
