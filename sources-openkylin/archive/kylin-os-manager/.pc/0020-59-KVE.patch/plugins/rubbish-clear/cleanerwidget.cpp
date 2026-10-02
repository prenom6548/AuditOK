/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "cleanerwidget.h"
#include "dbus/threadpool.h"
#include "clearmanager.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include "gsettings.hpp"

using namespace kdk::kabase;

namespace KylinRubbishClear
{


CleanerWidget::CleanerWidget(QWidget *parent) : QWidget(parent)
{
    //    this->setFixedSize(860, 460);
    //    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);
    //    this->setStyleSheet("QWidget{background: #ffffff; border:
    //    none;/*border-bottom-right-radius:10px;border-bottom-left-radius:6px*/}"); this->setAutoFillBackground(true);
    //    QPalette palette;
    //    palette.setBrush(QPalette::Window, QBrush(Qt::white));
    //    this->setPalette(palette);

    initThemeGetting();

    statked_widget = new QStackedWidget(this);
    main_widget = new ClearMainWidget(themeColor, mFontSize, this);
    detailview = new CleandetailVeiw(themeColor, mFontSize, this);
    statked_widget->addWidget(main_widget);
    statked_widget->addWidget(detailview);

    QVBoxLayout *layout1 = new QVBoxLayout();
    layout1->addWidget(statked_widget);
    layout1->setSpacing(0);
    layout1->setMargin(0);
    layout1->setContentsMargins(0, 0, 0, 0);

    setLayout(layout1);
    startDbusDaemon();
}

CleanerWidget::~CleanerWidget() {}

void CleanerWidget::paintEvent(QPaintEvent *event) {}

void CleanerWidget::resetSkin(QString skin)
{
    //    if(main_widget != NULL)
    //        main_widget->resetCurrentSkin(skin);
}

void CleanerWidget::displayDetailPage()
{
    //    qDebug() << "CleanerWidget::displayDetailPage";
    statked_widget->setCurrentIndex(1);
    detailview->ResetUI();
}

void CleanerWidget::displayMainPage()
{
    statked_widget->setCurrentIndex(0);
    main_widget->resetDefaultStatus();
}
//启动dbus服务
void CleanerWidget::startDbusDaemon()
{
    m_dataWorker = new DataWorker(this->desktop);                  //这里传入的字符串，用来判定版本
    QThread *w_thread = ThreadPool::Instance()->createNewThread(); //线程池创建新的线程
    m_dataWorker->moveToThread(w_thread);
    connect(w_thread, &QThread::started, m_dataWorker, &DataWorker::doWork /*, Qt::QueuedConnection*/);
    connect(m_dataWorker, &DataWorker::dataLoadFinished, this,
            &CleanerWidget::onInitDataFinished /*, Qt::BlockingQueuedConnection*/);
    connect(w_thread, &QThread::finished, this, [=] {
        w_thread->deleteLater();
        qDebug() << "DataWorker thread finished......";
    });
    w_thread->start();
}

void CleanerWidget::onInitDataFinished()
{
    //    qDebug() << "CleanerWidget::onInitDataFinished";
    qDebug() << Q_FUNC_INFO;
    connect(main_widget, SIGNAL(hideThisWidget()), this, SLOT(displayDetailPage()));
    connect(detailview, SIGNAL(hideThisWidget()), this, SLOT(displayMainPage()));
    //点击主页面开始清理后，开始扫描
    connect(main_widget, SIGNAL(startScanSystem(QMap<QString, QVariant>)), m_dataWorker,
            SLOT(onStartScanSystem(QMap<QString, QVariant>)));
    //报告具体垃圾清理时扫描的进度，包括 扫描完成
    connect(m_dataWorker, SIGNAL(tellCleanerDetailStatus(QString)), detailview, SLOT(showReciveStatus(QString)),
            Qt::BlockingQueuedConnection);
    //这个是扫描完成后，点击开始清理的信号
    connect(detailview, SIGNAL(startCleanSystem(QMap<QString, QVariant>)), m_dataWorker,
            SLOT(onStartCleanSystem(QMap<QString, QVariant>)));
    //传递扫描到的具体信息
    connect(m_dataWorker, SIGNAL(tellCleanerDetailData(QStringList)), detailview, SLOT(showReciveData(QStringList)),
            Qt::BlockingQueuedConnection);
    //这个是展示清理时各个组件是否清理完成的
    connect(m_dataWorker, SIGNAL(tellCleanerMainStatus(QString, QString)), detailview,
            SLOT(showCleanerStatus(QString, QString)), Qt::BlockingQueuedConnection);
    //展示清理时的小项目动画
    connect(m_dataWorker, SIGNAL(policykitCleanSignal(bool)), detailview, SLOT(receivePolicyKitSignal(bool)));
    //清理完成
    connect(m_dataWorker, SIGNAL(sendCleanOverSignal()), detailview, SLOT(showCleanOverStatus()));


    //这些暂时没啥用
    connect(m_dataWorker, SIGNAL(tellCleanerMainData(QStringList)), detailview, SLOT(showCleanerData(QStringList)),
            Qt::BlockingQueuedConnection);
    connect(m_dataWorker, SIGNAL(sendCleanErrorSignal(QString)), detailview, SLOT(showCleanerError(QString)),
            Qt::BlockingQueuedConnection);

    /****************新增加的本地处理项***************/
    ClearManager *manager = ClearManager::getInstance();
    //点击主页面开始清理后，开始扫描
    connect(main_widget, &ClearMainWidget::sigStartScanSystem, manager, &ClearManager::slotScanSystem);
    //扫描回收站进度
    connect(manager, &ClearManager::sigCleanTrashStatus, detailview, &CleandetailVeiw::statusTipSetText);
    //传递扫描到的具体信息
    connect(manager, &ClearManager::sigScanDetailData, detailview, &CleandetailVeiw::slotScanDetailData);
    //报告具体扫描的进度，包括 扫描完成
    connect(manager, &ClearManager::sigScanDetailStatus, detailview, &CleandetailVeiw::slotScanDetailStatus);
    //这个是扫描完成后，点击开始清理的信号
    connect(detailview, &CleandetailVeiw::sigStartCleanSystem, manager, &ClearManager::slotCleanSystm);
    //这个是展示清理时各个组件是否清理完成的
    connect(manager, &ClearManager::sigCleanStatus, detailview, &CleandetailVeiw::slotCleanStatus);
}

void CleanerWidget::initThemeGetting()
{
    QString themeStr = kdk::kabase::Gsettings::getSystemTheme().toString();
    if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
        themeColor = 1;
    } else {
        themeColor = 0;
    }

    connect(Gsettings::getPoint(), &Gsettings::systemThemeChange, this, [this]() {
        QString themeStr = kdk::kabase::Gsettings::getSystemTheme().toString();
        qDebug() << "ToolBoxWidget::initThemeGetting theme change :" << themeStr;
        if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
            themeColor = 1;
        } else {
            themeColor = 0;
        }
        changeThemeColor(themeColor);
    });

    //字体的
    mFontSize = Gsettings::getSystemFontSize().toInt();
    qDebug() << "ToolBoxWidget::initThemeGetting :system font size :" << mFontSize;

    connect(Gsettings::getPoint(), &Gsettings::systemFontSizeChange, this, [this]() {
        mFontSize = Gsettings::getSystemFontSize().toInt();
        changeSystemFontSize(mFontSize);
    });
}

void CleanerWidget::changeThemeColor(int color)
{
    qDebug() << "CleanerWidget::changeThemeColor" << color;
    main_widget->changeThemeColor(color);
    detailview->changeThemeColor(color);
}

void CleanerWidget::changeSystemFontSize(int size)
{
    main_widget->changeSystemSize(size);
    detailview->changeSystemSize(size);
}
} // namespace KylinRubbishClear
