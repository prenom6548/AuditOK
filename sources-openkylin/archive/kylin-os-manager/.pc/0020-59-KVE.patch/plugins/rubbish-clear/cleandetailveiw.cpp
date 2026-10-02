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

#include "cleandetailveiw.h"
#include "selectcategorywidget.h"
#include "selectwidget.h"
#include "utils.h"
#include "publicstatus.h"
#include "cleartrash.h"
#include "buried_point.hpp"
#include "toolutils.h"
#include <QDebug>
#include <QtSvg/QSvgRenderer>
#include <QApplication>
#include <QScreen>

#include <gsettingmonitor.h>
#include "frame.h"

namespace KylinRubbishClear
{

CleandetailVeiw::CleandetailVeiw(int theme, int fontSize, QWidget *parent)
    : QWidget(parent), themeColor(theme), mFontSize(fontSize)
{
    InitWidget();
    alertDialog = new KAlertDialog();
    alertDialog->setWindowModality(Qt::ApplicationModal);
}

CleandetailVeiw::~CleandetailVeiw()
{
    if (alertDialog) {
        alertDialog->deleteLater();
        alertDialog = nullptr;
    }
}

void CleandetailVeiw::InitWidget()
{
    main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    QFrame *top_widget = new QFrame(this);

    top_layout = new QHBoxLayout(top_widget);
    top_layout->setSpacing(0);
    top_layout->setContentsMargins(40, 24, 40, 27);
    main_layout->addWidget(top_widget);

    QFrame *spilterLine = new QFrame(this);
    spilterLine->setFixedSize(860, 1);
    spilterLine->setStyleSheet("QFrame{background:rgba(0,0,0,0.1);}");
    //    spilterLine->setFrameShape(QFrame::HLine);
    //    spilterLine->setFrameShadow(QFrame::Plain);
    main_layout->addWidget(spilterLine);

    QFrame *bottom_widget = new QFrame(this);
    //    bottom_widget->setFixedSize(860,400);
    bottom_layout = new QVBoxLayout(bottom_widget);
    bottom_layout->setSpacing(0);
    bottom_layout->setMargin(0);
    bottom_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(bottom_widget);

    cache_files_sizes.clear();

    InitTopWidget();
    InitBottomWidget();

    this->setLayout(main_layout);
}

void CleandetailVeiw::InitTopWidget()
{
    QFrame *frame = new QFrame(this);
    //    frame->setFixedSize(860,100);

    top_icon = new QLabel(frame);
    top_icon->setFixedSize(64, 64);
    top_icon->setScaledContents(true);

    top_tip = new QLabel(frame);
    top_tip->setFixedWidth(432);
    QFont font;
    float tipFontSize = 24 * mFontSize / 11;
    font.setPointSizeF(tipFontSize);
    font.setBold(true);
    top_tip->setFont(font);
    top_tip->setText(tr("Computer scan in progress..."));

    status_tip = new QLabel(frame);
    status_tip->setFixedWidth(432);
    //    status_tip->setStyleSheet("color:rgb(0,0,0,165)");
    statusTipSetText(tr("Computer scan in progress..."));

    cancel_btn = new QPushButton(frame);
    cancel_btn->setText(tr("Cancel"));
    cancel_btn->resize(96, 36);
    connect(cancel_btn, SIGNAL(clicked()), this, SIGNAL(hideThisWidget()));
    cancel_btn->setFocus(Qt::MouseFocusReason);

    onkeyclean = new QPushButton(frame);
    onkeyclean->setText(tr("Cleanup"));
    onkeyclean->resize(96, 36);
    onkeyclean->setProperty("isImportant", true);
    onkeyclean->setVisible(false);
    onkeyclean->setFocus(Qt::MouseFocusReason);
    connect(onkeyclean, &QPushButton::clicked, this, [=] {
        receiveCleanSignal();
    });

    return_btn = new QPushButton(frame);
    return_btn->setText(tr("Return"));
    return_btn->setFixedSize(96, 36);
    return_btn->setVisible(true);
    return_btn->setFocus(Qt::MouseFocusReason);
    connect(return_btn, SIGNAL(clicked()), this, SIGNAL(hideThisWidget()));

    btn_return = new QPushButton(frame);
    btn_return->setText(tr("Finish"));
    btn_return->setFixedSize(96, 36);
    btn_return->setVisible(false);
    btn_return->setFocus(Qt::MouseFocusReason);
    connect(btn_return, SIGNAL(clicked()), this, SIGNAL(hideThisWidget()));

    QVBoxLayout *mVLayout = new QVBoxLayout();
    mVLayout->setSpacing(0);
    mVLayout->setContentsMargins(0, 0, 0, 0);
    mVLayout->addWidget(top_tip);
    mVLayout->addSpacing(4);
    mVLayout->addWidget(status_tip);

    top_layout->addWidget(top_icon);
    top_layout->addSpacing(16);
    top_layout->addLayout(mVLayout);
    top_layout->addStretch();
    top_layout->addWidget(return_btn);
    top_layout->addSpacing(10);
    top_layout->addWidget(cancel_btn);
    top_layout->addWidget(onkeyclean);
    top_layout->addWidget(btn_return);
}

void CleandetailVeiw::InitBottomWidget()
{
    mTipLabelList.clear();

    QFrame *frame = new QFrame(this);
    QHBoxLayout *m_layout = new QHBoxLayout(frame);
    m_layout->setMargin(0);
    m_layout->setContentsMargins(40, 65, 40, 50);

    QFrame *cache_frame = new QFrame(frame);
    QVBoxLayout *cache_layout = new QVBoxLayout(cache_frame);
    cache_icon = new QLabel(cache_frame);

    cache_layout->addWidget(cache_icon);

    cache_tip = new QLabel(cache_frame);
    cache_tip->setText(tr("System cache"));
    cache_tip->adjustSize();
    cache_tip->setWordWrap(true);
    cache_tip->setMinimumWidth(210);
    cache_layout->addWidget(cache_tip);

    QLabel *lable1 = new QLabel(cache_frame);
    QPalette lable1Palette = lable1->palette();
    lable1Palette.setColor(QPalette::Text, qApp->palette().color(QPalette::PlaceholderText));
    lable1->setPalette(lable1Palette);
    lable1->setText(tr("Clear package、thumbnails and browser cache"));
    lable1->setWordWrap(true);
    mTipLabelList.append(lable1);
    cache_layout->addWidget(lable1);

    cache_btn = new KBorderlessButton(cache_frame);
    cache_btn->setObjectName("Cache");
    cache_btn->setText(tr("Details"));
    cache_layout->addWidget(cache_btn);
    cache_layout->addStretch(1);
    cache_btn->setVisible(false);

    connect(cache_btn, SIGNAL(clicked()), this, SLOT(ShowDetailsPage()));

    QFrame *cookie_frame = new QFrame(frame);
    QVBoxLayout *cookie_layout = new QVBoxLayout(cookie_frame);
    cookie_icon = new QLabel(cookie_frame);

    cookie_layout->addWidget(cookie_icon);

    cookie_tip = new QLabel(cookie_frame);
    cookie_tip->setText(tr("Cookies"));
    cookie_tip->setMinimumWidth(210);
    cookie_tip->adjustSize();
    cookie_tip->setWordWrap(true);
    cookie_layout->addWidget(cookie_tip);

    QLabel *lable2 = new QLabel(cookie_frame);
    QPalette lable2Palette = lable2->palette();
    lable2Palette.setColor(QPalette::Text, qApp->palette().color(QPalette::PlaceholderText));
    lable2->setPalette(lable2Palette);
    lable2->setText(tr("Clear internet、games、shopping history, etc"));
    lable2->setWordWrap(true);
    mTipLabelList.append(lable2);
    cookie_layout->addWidget(lable2);

    cookie_btn = new QPushButton(cookie_frame);
    cookie_btn->setObjectName("Cookie");
    cookie_btn->setText(tr("Details"));
    cookie_btn->setFixedSize(96, 36);
    cookie_btn->setStyleSheet("QPushButton{width:80px;height:36px;\
                             background:rgba(231,231,231,1);\
                             border-radius:18px;color:rgba(50,97,247,1);}\
                             QPushButton:hover{width:80px;height:36px;\
                             background:rgba(67,127,240,1);\
                             border-radius:18px;color:white;}");
    cookie_layout->addWidget(cookie_btn);
    cookie_layout->addStretch(1);
    cookie_btn->setVisible(false);

    connect(cookie_btn, SIGNAL(clicked()), this, SLOT(ShowDetailsPage()));

    QFrame *history_frame = new QFrame(frame);
    QVBoxLayout *history_layout = new QVBoxLayout(history_frame);
    history_icon = new QLabel(history_frame);

    history_layout->addWidget(history_icon);

    changeThemeColor(themeColor);

    history_tip = new QLabel(history_frame);
    history_tip->setText(tr("Historical trace"));
    history_tip->adjustSize();
    history_tip->setWordWrap(true);
    history_tip->setMinimumWidth(210);
    history_layout->addWidget(history_tip);

    QLabel *lable3 = new QLabel(history_frame);
    QPalette lable3Palette = lable3->palette();
    lable3Palette.setColor(QPalette::Text, qApp->palette().color(QPalette::PlaceholderText));
    lable3->setPalette(lable3Palette);
    lable3->setText(tr("Clear system usage traces"));
    lable3->setWordWrap(true);
    mTipLabelList.append(lable3);
    history_layout->addWidget(lable3);

    history_layout->addStretch(1);

    m_layout->addWidget(cache_frame);
    m_layout->addWidget(cookie_frame);
    m_layout->addWidget(history_frame);

    bottom_layout->addWidget(frame);
}

void CleandetailVeiw::ResetUI()
{
    qDebug() << "CleandetailVeiw::ResetUI";
    cache_tip->setText(tr("System cache"));
    history_tip->setText(tr("Historical trace"));
    cookie_tip->setText(tr("Cookies"));
    top_tip->setText(tr("Computer scan in progress..."));
    cancel_btn->setVisible(true);
    onkeyclean->setVisible(false);
    return_btn->setVisible(false);
    btn_return->setVisible(false);
    cache_btn->setVisible(false);
    onkeyclean->setEnabled(true);

    cookie_btn->setVisible(false);

    isSystemPix = true; //用来标记小图标是否需要在主题发生变化后进行改变
    isCookiePix = true;
    isTracePix = true;
    changeThemeColor(themeColor);

    cache_list.clear();
    //修改bug，所有缓存的数据都需要复位
    m_selectedAptList.clear();
    trace_system_count = "";
    trace_bash_path = "";
    trace_bash_size = "0";
    cache_flag = true;

    cache_files_sizes.clear();
    cookies_list.clear();
    Cookie_map.clear();
    cache_sum = 0;
    history_sum = 0;
    cookie_sum = 0;

    isCookieFinish = false;
    isOthersFinish = false;

    isLocalCleanFinish = false;
    isOtherCLeanFinish = false;


    isTrashCleanFinish = false;
    isOtherCacheCleanFinish = false;


    isTrashScanFinish = false;
    isOtherCacheScanFinish = false;
}

void CleandetailVeiw::getScanResult(QString msg) {}

void CleandetailVeiw::finishScanResult(QString msg) {}

void CleandetailVeiw::getScanAllResult(QString flag, QString msg) {}

void CleandetailVeiw::OnClikedCleanBtn()
{
    top_tip->setText(tr("Computer cleanup in progress..."));
}
void CleandetailVeiw::slotScanDetailData(const QString status, const QStringList &list)
{
    //    qDebug() << "CleandetailVeiw::slotScanDetailData  list.size()" << list.size();
    if ("Cookies" == status) {
        if (list.size() == 0) {
            cookie_sum = 0;
        } else {
            cookie_sum = list.size();
        }
        cookies_list = list;
    } else if ("Trash" == status) {
        if (list.size() == 0) {
            qDebug() << "CleandetailVeiw::slotScanDetailData Trash is null";
        } else {
            for (int i = 0; i < list.size(); i++) { //当前这个循环只会走一遍
                QString info = list.at(i);
                QStringList detail = info.split(":");
                if (detail.length() > 1) {
                    QString path = detail.at(0);
                    QString sizeSum = detail.at(1);
                    qDebug() << "CleandetailVeiw::slotScanDetailData sizeSum" << sizeSum;
                    qreal size_kb = sizeSum.toDouble() / 1024.0;
                    if (size_kb < 1) {
                        size_kb = 1;
                    }
                    cache_files_sizes.insert(path, size_kb);
                    cache_sum += size_kb;
                    qDebug() << "CleandetailVeiw::slotScanDetailData Trash path:" << path;
                    cache_list.append(path); //是要将扫描到的回收站的全部添加到后边
                } else {
                    qWarning() << "CleandetailVeiw::slotScanDetailData Trash list is err!";
                }
            }
        }
    }
}
void CleandetailVeiw::showReciveData(const QStringList &data)
{
    if (data.at(0).split(".").at(0) == "Belong:Cache" && !data.at(1).isEmpty()) {
        QString cache_name = data.at(1).split(":").at(1);
        statusTipSetText(cache_name);

        QStringList r = data.at(3).split(":").at(1).split(" ");
        qreal size_kb;
        if (QString::compare(r.at(1), "MB") == 0) {
            size_kb = r.at(0).toFloat() * 1024;
            cache_sum += size_kb;
        } else {
            size_kb = r.at(0).toFloat();
            cache_sum += size_kb;
        }
        cache_files_sizes.insert(cache_name, size_kb);

        if (data.at(1).contains(":")) {
            cache_list.append(data.at(1).split(":").at(1));
        }
    } else {
        //        if(data.at(0).split(".").at(0) == "Belong:Cookies" && !data.at(1).isEmpty())
        //        {
        //            if(!data.at(2).isEmpty()){
        //                cookie_sum += data.at(2).split(":").at(1).toInt();
        //            }

        //            if(!Cookie_map.contains(data.at(1).split(":").at(1))){
        //                if(!data.at(2).split(":").at(1).isEmpty())
        //                    Cookie_map.insert(data.at(1).split(":").at(1),data.at(2).split(":").at(1).toInt());
        //            }else{
        //                if(!data.at(2).split(":").at(1).isEmpty()){
        //                    int num = Cookie_map[data.at(1).split(":").at(1)];
        //                    Cookie_map.insert(data.at(1).split(":").at(1),data.at(2).split(":").at(1).toInt() + num);
        //                }
        //            }

        //            if(data.at(1).contains(":") && !data.at(1).split(":").at(1).isEmpty())
        //            {
        //                if(!cookies_list.contains(data.at(1).split(":").at(1)))
        //                    cookies_list.append(data.at(1).split(":").at(1));
        //            }
        //        }
        //----------------------------------------------------------------History Trace---------------------------
        if (data.at(0).split(".").at(0) == "Belong:History" && !data.at(1).isEmpty()) {
            QString r = data.at(1).split(":").at(1);

            history_sum += r.toInt();

            if (data.at(0) == "Belong:History.firefox" && !data.at(1).isEmpty()) {
                if (data.at(1).contains(":")) {
                    if (data.at(1).split(":").at(1).toInt() != 0) {
                        trace_firefox_count = data.at(1).split(":").at(1);
                    }
                }
            } else if (data.at(0) == "Belong:History.chromium" && !data.at(1).isEmpty()) {
                if (data.at(1).contains(":")) {
                    if (data.at(1).split(":").at(1).toInt() != 0) {
                        trace_chromium_count = data.at(1).split(":").at(1);
                    }
                }
            } else if (data.at(0) == "Belong:History.system" && !data.at(1).isEmpty()) {
                if (data.at(1).contains(":")) {
                    if (data.at(1).split(":").at(1).toInt() != 0) {
                        trace_system_count = data.at(1).split(":").at(1);
                    }
                }
            } else if (data.at(0) == "Belong:History.bash" && !data.at(1).isEmpty() && !data.at(2).isEmpty()) {
                if (data.at(1).contains(":")) {
                    trace_bash_size = data.at(1).split(":").at(1);
                    trace_bash_path = data.at(2).split(":").at(1);
                }
            } else if (data.at(0) == "Belong:History.X11") // && !data.at(1).isEmpty() && !data.at(2).isEmpty()
            {

                if (data.at(1).contains(":")) {
                    trace_x11_list.append(data.at(1).split(":").at(1));
                }
            }
        }
    }
}

void CleandetailVeiw::setCacheTip(qreal cache_sum)
{
    if (cache_sum < 1024) {
        cache_tip->setText(tr("Cleanable cache ") + QString::number(int(cache_sum)) + " KB");
    } else {
        cache_tip->setText(tr("Cleanable cache ") + QString::number(cache_sum / 1024, 'f', 0) + " M");
    }
}

void CleandetailVeiw::setCookieTip(int cookie_sum)
{
    cookie_tip->setText(tr("Cleanable browser ") + QString::number(cookie_sum) + tr(" items"));
}
void CleandetailVeiw::slotScanDetailStatus(const QString &str)
{
    qDebug() << "CleandetailVeiw::slotScanDetailStatus" << str;
    if (str == "Complete:Cookies") {
        if (!PublicStatus::getInstance()->getIsCookiesNull()) {
            setCookieTip(cookie_sum);
            //        if(cookie_sum != 0)
            //            cookie_btn->setVisible(true);
            //        else
            //            cookie_btn->setVisible(false);

            QSvgRenderer *svgRender1 = new QSvgRenderer(QString(":/res/light/cookie_blue.svg"));
            QPixmap *icon1 = new QPixmap(32, 32);
            icon1->fill(Qt::transparent); //设置背景透明
            QPainter p1(icon1);
            svgRender1->render(&p1);
            cookie_icon->setPixmap(*icon1);
            isCookiePix = false;
        }
        isCookieFinish = true;
        isAllScanAreFinish();
    } else if (str == "Complete:Trash") {
        isTrashScanFinish = true;
        isAllCacheScanFinish();
        isAllScanAreFinish();
    }
}

void CleandetailVeiw::isAllCacheScanFinish()
{
    if (isTrashScanFinish && isOtherCacheScanFinish) {
        if (PublicStatus::getInstance()->getIsSystemCacheNull()) {
            return;
        }
        setCacheTip(cache_sum);
        if (cache_sum != 0) {
            cache_btn->setVisible(true);
        } else {
            cache_btn->setVisible(false);
        }
        QSvgRenderer *svgRender = new QSvgRenderer(QString(":/res/light/system_blue.svg"));
        QPixmap *icon = new QPixmap(32, 32);
        icon->fill(Qt::transparent); //设置背景透明
        QPainter p(icon);
        svgRender->render(&p);

        cache_icon->setPixmap(*icon);
        isSystemPix = false;
    }
}
void CleandetailVeiw::showReciveStatus(const QString &status)
{
    //    qDebug() << "CleandetailVeiw::showReciveStatus " <<status;
    qDebug() << Q_FUNC_INFO << status;

    if (status == "Complete:History") { //设置有多少个历史记录被扫描
        if (PublicStatus::getInstance()->getIsTraceNull()) {
            return;
        }
        history_tip->setText(QString::number(history_sum) + tr(" historical use traces"));
        QSvgRenderer *svgRender2 = new QSvgRenderer(QString(":/res/light/trace_blue.svg"));
        QPixmap *icon2 = new QPixmap(32, 32);
        icon2->fill(Qt::transparent); //设置背景透明
        QPainter p2(icon2);
        svgRender2->render(&p2);
        history_icon->setPixmap(*icon2);
        isTracePix = false;
    }
    //    else if(status == "Complete:Cookies") {
    //        setCookieTip(cookie_sum);

    //        if(cookie_sum != 0)
    //            cookie_btn->setVisible(true);
    //        else
    //            cookie_btn->setVisible(false);
    //    }
    else if (status == "Complete:Cache") {
        isOtherCacheScanFinish = true;
        isAllCacheScanFinish();
        //        setCacheTip(cache_sum);

        //        if(cache_sum != 0)
        //            cache_btn->setVisible(true);
        //        else
        //            cache_btn->setVisible(false);
    } else if (status == "Complete:All") {
        isOthersFinish = true;
        isAllScanAreFinish();
        /******这个地方做个改动，所有扫描完成后，进行回收站的扫描******/
        //        if(cache_sum == 0 && cookie_sum == 0 && history_sum == 0){
        //            top_tip->setText(tr("There's nothing to clean up."));
        //            cancel_btn->setVisible(false);
        //            onkeyclean->setVisible(false);
        //            return_btn->setVisible(true);
        //        }
        //        else{
        //            top_tip->setText(tr("Complete!"));
        //            status_tip->clear();
        //            cancel_btn->setVisible(false);
        //            onkeyclean->setVisible(true);
        //            return_btn->setVisible(false);
        //            btn_return->setVisible(true);
        //        }

        //        m_selectedAptList = cache_list;
        //        m_selectedCookieList = cookies_list;
    }
}
void CleandetailVeiw::isAllScanAreFinish()
{
    if (isCookieFinish && isOthersFinish && isTrashScanFinish) {
        QString str1 = tr("system cache");
        QString str2 = tr("cookie record");
        QString str3 = tr("history trace");
        QString num1;
        QString num2;
        QString num3;
        if (cache_sum == 0 && cookie_sum == 0 && history_sum == 0) {
            top_tip->setText(tr("There's nothing to clean up."));
            cancel_btn->setVisible(false);
            onkeyclean->setVisible(false);
            return_btn->setVisible(true);

            num1 = "0 KB，";
            num2 = "0" + tr(" item,");
            num3 = "0" + tr(" item");

        } else {
            top_tip->setText(tr("Complete"));
            //            status_tip->clear();
            return_btn->setVisible(true);
            cancel_btn->setVisible(false);
            onkeyclean->setVisible(true);
            btn_return->setVisible(false);
            if (cache_sum < 1024) {
                num1 = QString::number(int(cache_sum)) + " KB，";
            } else {
                num1 = QString::number((cache_sum) / 1024, 'f', 0) + " M，";
            }
            num2 = QString::number(cookie_sum) + tr(" item,");
            num3 = QString::number(history_sum) + tr(" item");
        }
        statusTipSetText(QString("%1 %2%3 %4%5 %6").arg(str1).arg(num1).arg(str2).arg(num2).arg(str3).arg(num3));
        m_selectedAptList = cache_list;
        m_selectedCookieList = cookies_list;
    }
}

void CleandetailVeiw::getCleanResult(QString msg) {}

void CleandetailVeiw::ShowDetailsPage()
{
    QObject *object = QObject::sender();
    QPushButton *button = qobject_cast<QPushButton *>(object);
    QString btn_name = button->objectName();

    SelectWidget *w = nullptr;
    if (QString::compare(btn_name, "Cache") == 0) {
        if (cache_flag) {
            select_cache_apt_list.clear();
            select_cache_apt_list = cache_list;
            cache_flag = false;
        }

        w = new SelectWidget(CleanerModuleID::CacheApt, tr("Cleanable Cache"));
        w->loadData(tr("Cleanable Cache"), select_cache_apt_list, cache_list);
        connect(w, SIGNAL(refreshSelectedItems(CleanerModuleID, QStringList)), this,
                SLOT(onRefreshSelectedItems(CleanerModuleID, QStringList)));
    } else if (QString::compare(btn_name, "Cookie") == 0) {
        if (cache_chromium_flag) {
            select_cookies_list.clear();
            select_cookies_list = cookies_list;
            cache_chromium_flag = false;
        }
        w = new SelectWidget(CleanerModuleID::Cookie, tr("Cleanable Cookie"));
        qDebug() << Q_FUNC_INFO << select_cookies_list << cookies_list;
        w->loadData(tr("Cleanable Cookie"), select_cookies_list, cookies_list);
        connect(w, SIGNAL(refreshSelectedItems(CleanerModuleID, QStringList)), this,
                SLOT(onRefreshSelectedItems(CleanerModuleID, QStringList)));
    }

    if (w != nullptr) {
        w->setAttribute(Qt::WA_DeleteOnClose);

        // 移动到应用中间
        auto frameRect = Frame::geometry();
        auto x = frameRect.x() + frameRect.width() / 2 - w->width() / 2;
        auto y = frameRect.y() + frameRect.height() / 2 - w->height() / 2;
        w->move(x, y);

        w->exec();
    }
}

void CleandetailVeiw::showCleanerData(const QStringList &data) {}
void CleandetailVeiw::slotCleanStatus(const QString status, const QString belong)
{
    if (status == "Complete:cookie" && belong == "cookie") {
        if (PublicStatus::getInstance()->getIsCookiesNull()) {
            return;
        }
        cookie_tip->setText(tr("Clear cookie ") + QString::number(cookie_sum) + tr(" items"));
        QSvgRenderer *svgRender = new QSvgRenderer(QString(":/res/light/status_finish_icon.svg"));
        QPixmap *pixmap = new QPixmap(32, 32);
        pixmap->fill(Qt::transparent); //设置背景透明
        QPainter p(pixmap);
        svgRender->render(&p);

        cookie_icon->setPixmap(*pixmap);
        cookie_icon->setFixedSize(pixmap->size());
        cookie_icon->update();
    } else if (status == "Complete:Trash" && belong == "trash") {
        isTrashCleanFinish = true;
        isAllCacheCleanFinish();
    } else if (status == "Complete:all" && belong == "all") {
        isLocalCleanFinish = true;
        isAllCleanAreFinish();
    }
}
void CleandetailVeiw::isAllCacheCleanFinish()
{
    qDebug() << "CleandetailVeiw::isAllCacheCleanFinish" << isTrashCleanFinish << isOtherCacheCleanFinish;
    if (isTrashCleanFinish && isOtherCacheCleanFinish) {
        if (PublicStatus::getInstance()->getIsSystemCacheNull()) {
            return;
        }
        if (cache_sum < 1024) {
            cache_tip->setText(tr("Clear cache ") + QString::number(int(cache_sum)) + " KB");
        } else {
            cache_tip->setText(tr("Clear cache ") + QString::number((cache_sum) / 1024, 'f', 0) + " M");
        }
        QSvgRenderer *svgRender = new QSvgRenderer(QString(":/res/light/status_finish_icon.svg"));
        QPixmap *pixmap = new QPixmap(32, 32);
        pixmap->fill(Qt::transparent); //设置背景透明
        QPainter p(pixmap);
        svgRender->render(&p);

        cache_icon->setPixmap(*pixmap);
        cache_icon->setFixedSize(pixmap->size());
        cache_icon->update();
    }
}
void CleandetailVeiw::showCleanerStatus(const QString &status, const QString &domain)
{
    qDebug() << "CleandetailVeiw::showCleanerStatus status:" << status << "domain:" << domain;

    if (status == "Complete:file" && domain == "cache") {

        isOtherCacheCleanFinish = true;
        isAllCacheCleanFinish();
    }
    //    else if(status == "Complete:cookie" && domain == "cookie") {
    //        cookie_tip->setText(tr("Clear cookie ")+QString::number(cookie_sum)+tr(" items"));

    //        QSvgRenderer* svgRender = new QSvgRenderer(QString(":/res/svg/finish2 .svg"));
    //        QPixmap *pixmap = new QPixmap(32,32);
    //        pixmap->fill(Qt::transparent);//设置背景透明
    //        QPainter p(pixmap);
    //        svgRender->render(&p);

    //        cookie_icon->setPixmap(*pixmap);
    //        cookie_icon->setFixedSize(pixmap->size());
    //        cookie_icon->update();
    //    }
    else if (status == "Complete:history" && domain == "history") {
        if (PublicStatus::getInstance()->getIsTraceNull()) {
            return;
        }
        history_tip->setText(tr("Clear ") + QString::number(history_sum) + tr(" historical traces"));
        QSvgRenderer *svgRender = new QSvgRenderer(QString(":/res/light/status_finish_icon.svg"));
        QPixmap *pixmap = new QPixmap(32, 32);
        pixmap->fill(Qt::transparent); //设置背景透明
        QPainter p(pixmap);
        svgRender->render(&p);

        history_icon->setPixmap(*pixmap);
        history_icon->setFixedSize(pixmap->size());
        history_icon->update();
    }
}

void CleandetailVeiw::showCleanerError(const QString &status) {}

void CleandetailVeiw::onRefreshSelectedItems(CleanerModuleID id, const QStringList &infos)
{
    //    qDebug() <<"======================"<< Q_FUNC_INFO << infos;
    int new_sum = 0;
    switch (id) {
    case CleanerModuleID::CacheApt:
        select_cache_apt_list = infos;
        m_selectedAptList.clear();
        m_selectedAptList = infos; //选中的需要清理的系统缓存项目
        cache_sum = 0;
        for (QString cache_file : infos) {
            if (cache_files_sizes.contains(cache_file)) {
                cache_sum += cache_files_sizes[cache_file];
            }
        }
        setCacheTip(cache_sum);
        break;
    case CleanerModuleID::Cookie:
        qDebug() << Q_FUNC_INFO << Cookie_map[".163.com"];
        select_cookies_list = infos;
        m_selectedCookieList.clear();
        m_selectedCookieList = infos;
        for (int i = 0; i < select_cookies_list.length(); i++) {
            if (QString::compare(select_cookies_list.at(i), "") == 0)
                continue;

            new_sum += Cookie_map[select_cookies_list.at(i)];
        }
        setCookieTip(new_sum);
        cookie_sum = new_sum;
        break;
    case CleanerModuleID::TraceX11:
        m_selectedTraceX11List.clear();
        m_selectedTraceX11List = infos;
        break;
    default:
        break;
    }
}

void CleandetailVeiw::receiveCleanSignal()
{
    kdk::kabase::BuriedPoint buriedPoint;
    if (buriedPoint.functionBuriedPoint(kdk::kabase::KylinOsManager,
                                        kdk::kabase::BuriedPoint::KylinOsManagerGarbageClean)) {
        qCritical() << "buried point fail!pt:BaseInfo";
    }

    this->getAllSelectedItems();
    if (argsData.empty() && mClearMap.empty()) {
        qDebug() << Q_FUNC_INFO << "no argsData";
        //提示选择为空
        if (alertDialog) {
            alertDialog->show();
        }
    } else {
        qDebug() << Q_FUNC_INFO << argsData;
        Q_EMIT this->startCleanSystem(argsData);
        cache_flag = true;
        onkeyclean->setEnabled(false);
    }
}

void CleandetailVeiw::receivePolicyKitSignal(bool status)
{
    //    qDebug() << "CleandetailVeiw::receivePolicyKitSignal:" <<status;
    qDebug() << Q_FUNC_INFO << status;
    if (status) {
        top_tip->setText(tr("Computer cleanup in progress..."));
        //        QMovie *movie = new QMovie(":/res/gif/clean_anime.gif");
        //        movie->setSpeed(200);
        //        cache_icon->setMovie(movie);
        //        cache_icon->setFixedSize(48,48);

        //        cookie_icon->setMovie(movie);
        //        cookie_icon->setFixedSize(48,48);

        //        history_icon->setMovie(movie);
        //        history_icon->setFixedSize(48,48);

        if (!PublicStatus::getInstance()->getIsSystemCacheNull()) {
            cache_tip->setText(tr("Cleaning up......"));
        }
        if (!PublicStatus::getInstance()->getIsTraceNull()) {
            history_tip->setText(tr("Cleaning up......"));
        }
        if (!PublicStatus::getInstance()->getIsCookiesNull()) {
            cookie_tip->setText(tr("Cleaning up......"));
        }
        //        movie->start();

        cache_btn->setVisible(false);
        cookie_btn->setVisible(false);
        return_btn->setVisible(false);
        //        onkeyclean->setVisible(false);
        btn_return->setVisible(false);

        Q_EMIT this->sigStartCleanSystem(mClearMap);
    } else {
        onkeyclean->setEnabled(true);
    }
}
void CleandetailVeiw::isAllCleanAreFinish()
{
    if (isLocalCleanFinish && isOtherCLeanFinish) {

        top_tip->setText(tr("Clearance completed"));
        statusTipSetText(tr("Computer is very energetic, please keep cleaning habits"));
        cancel_btn->setVisible(false);
        onkeyclean->setVisible(false);
        return_btn->setVisible(false);
        btn_return->setVisible(true);
    }
}
void CleandetailVeiw::showCleanOverStatus()
{
    //    qDebug() << "CleandetailVeiw::showCleanOverStatus";
    qDebug() << Q_FUNC_INFO;
    isOtherCLeanFinish = true;
    isAllCleanAreFinish();
    //    top_tip->setText(tr("Clearance completed!"));
    //    cancel_btn->setVisible(false);
    //    onkeyclean->setVisible(false);
    //    return_btn->setVisible(true);
}

void CleandetailVeiw::getAllSelectedItems()
{
    argsData.clear();
    mClearMap.clear();

    QStringList fileTmp;
    QStringList trashTmp;
    qDebug() << Q_FUNC_INFO << m_selectedAptList;

    for (QString info : m_selectedAptList) {
        if (info != "") {
            if (!info.startsWith("Trash")) {
                fileTmp.append(info);
            } else {
                trashTmp.append(info);
            }
        }
    }

    if (trace_firefox_count > 0) {
        argsData.insert("firefox-history", QStringList() << trace_firefox_count);
    } else if (trace_chromium_count > 0) {
        argsData.insert("chromium-history", QStringList() << trace_chromium_count);
    }

    if (trace_system_count > 0) {
        qDebug() << "CleandetailVeiw::getAllSelectedItems trace_system_count:" << trace_system_count;
        argsData.insert("system-history", QStringList() << trace_system_count);
    }

    if (trace_bash_path > 0) {
        qDebug() << "CleandetailVeiw::getAllSelectedItems  trace_bash_path:" << trace_bash_path;
        argsData.insert("bash-history", QStringList() << trace_bash_path);
    }

    if (fileTmp.length() > 0)
        argsData.insert("file", fileTmp);
    //    if(m_selectedCookieList.length() > 0){
    //        m_selectedCookieList.removeAll("");
    //        argsData.insert("cookie", m_selectedCookieList);
    //    }
    if (m_selectedTraceX11List.length() > 0) {
        argsData.insert("x11-history", m_selectedTraceX11List);
    }
    if (m_selectedCookieList.length() > 0) {
        m_selectedCookieList.removeAll("");
        mClearMap.insert("cookie", m_selectedCookieList);
    }
    if (trashTmp.length() > 0) {
        mClearMap.insert("trash", trashTmp);
    }
}

void CleandetailVeiw::timerUpDate(bool t)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(250);
    connect(timer, &QTimer::timeout, [=] {
        for (int i = 0; i <= 3; i++) {
            this->animationForTip(i);
        }
    });

    if (t) {
        timer->start();
    } else {
        timer->stop();
    }
}

void CleandetailVeiw::animationForTip(int f)
{
    switch (f) {
    case 0:
        cache_tip->setText(tr("Cleaning up"));
        history_tip->setText(tr("Cleaning up"));
        cookie_tip->setText(tr("Cleaning up"));
        break;
    case 1:
        cache_tip->setText(tr("Cleaning up.."));
        history_tip->setText(tr("Cleaning up.."));
        cookie_tip->setText(tr("Cleaning up.."));
        break;
    case 2:
        cache_tip->setText(tr("Cleaning up...."));
        history_tip->setText(tr("Cleaning up...."));
        cookie_tip->setText(tr("Cleaning up...."));
        break;
    case 3:
        cache_tip->setText(tr("Cleaning up......"));
        history_tip->setText(tr("Cleaning up......"));
        cookie_tip->setText(tr("Cleaning up......"));
        break;
    default:
        break;
    }
}
void CleandetailVeiw::changeThemeColor(int color)
{
    themeColor = color;
    if (isSystemPix) {
        QString iconPath1;
        if (color) {
            iconPath1 = ":/res/dark/system_grey.svg";
        } else {
            iconPath1 = ":/res/light/system_grey.svg";
        }
        QSvgRenderer *svgRender = new QSvgRenderer(iconPath1);
        QPixmap *icon = new QPixmap(32, 32);
        icon->fill(Qt::transparent); //设置背景透明
        QPainter p(icon);
        svgRender->render(&p);

        cache_icon->setPixmap(*icon);
        cache_icon->setFixedSize(icon->size());
    }

    if (isCookiePix) {
        QString iconPath2;
        if (color) {
            iconPath2 = ":/res/dark/cookie_grey.svg";
        } else {
            iconPath2 = ":/res/light/cookie_grey.svg";
        }
        QSvgRenderer *svgRender1 = new QSvgRenderer(iconPath2);
        QPixmap *icon1 = new QPixmap(32, 32);
        icon1->fill(Qt::transparent); //设置背景透明
        QPainter p1(icon1);
        svgRender1->render(&p1);

        cookie_icon->setPixmap(*icon1);
        cookie_icon->setFixedSize(icon1->size());
    }
    if (isTracePix) {
        QString iconPath3;
        if (color) {
            iconPath3 = ":/res/dark/trace_grey.svg";
        } else {
            iconPath3 = ":/res/light/trace_grey.svg";
        }
        QSvgRenderer *svgRender2 = new QSvgRenderer(iconPath3);
        QPixmap *icon2 = new QPixmap(32, 32);
        icon2->fill(Qt::transparent); //设置背景透明
        QPainter p2(icon2);
        svgRender2->render(&p2);

        history_icon->setPixmap(*icon2);
        history_icon->setFixedSize(icon2->size());
    }
    if (color) {
        QPixmap pix(":/res/svg/clear_icon_dark.svg");
        top_icon->setPixmap(pix);
    } else {
        QPixmap pix(":/res/svg/clear_icon_light.svg");
        top_icon->setPixmap(pix);
    }
}

void CleandetailVeiw::changeSystemSize(int size)
{
    QVariant fontSize = kdk::GsettingMonitor::getSystemFontSize();
    mFontSize = fontSize.toDouble();

    QFont font;
    font.setPointSizeF(24 * fontSize.toDouble() / 11);
    font.setBold(true);
    top_tip->setFont(font);

    font.setBold(false);
    font.setPointSizeF(14 * fontSize.toDouble() / 11);
    status_tip->setFont(font);

    ToolUtils::setToolTipAfterChangeFont(status_tip, mStrStatusTip);
}

void CleandetailVeiw::statusTipSetText(QString text)
{
    mStrStatusTip = text;
    status_tip->setText(text);
    ToolUtils::setToolTipAfterChangeFont(status_tip, mStrStatusTip);
}

} // namespace KylinRubbishClear
