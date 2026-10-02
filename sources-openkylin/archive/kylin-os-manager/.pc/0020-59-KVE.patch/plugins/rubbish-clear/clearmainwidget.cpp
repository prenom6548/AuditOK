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

#include <unistd.h>
#include <QDBusInterface>

#include "clearmainwidget.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QScreen>
#include <QVBoxLayout>

#include "cleartrash.h"
#include "publicstatus.h"
#include "selectcategorywidget.h"
#include "kom-ukui-gsettings.h"
#include "kom-utils.h"
#include "frame.h"

namespace KylinRubbishClear
{

ClearMainWidget::ClearMainWidget(int theme, int size, QWidget *parent)
    : QWidget(parent), themeColor(theme), mFontSize(size)

{
    initDataList();
    initMainUi();
    alertDialog = new KAlertDialog();
}

ClearMainWidget::~ClearMainWidget()
{
    argsMap.clear();
    if (alertDialog) {
        alertDialog->deleteLater();
        alertDialog = nullptr;
    }
}

void ClearMainWidget::initMainUi()
{
    //    mBtnSystemCache = new QPushButton(this);
    //    mBtnCookies = new QPushButton(this);
    //    mBtnHistory = new QPushButton(this);

    //    mLabelSystemCache = new QLabel(this);
    //    mLabelCookies = new QLabel(this);
    //    mLabelHistory = new QLabel(this);
    mLabelSlogen = new QLabel(this);
    QFont font;
    font.setPixelSize(36);
    font.setBold(true);
    mLabelSlogen->setFont(font);
    mLabelSlogen->setText(tr("Computer garbage  One key to clean up"));
    // mLabelSlogen->setGeometry(48,88,434,44);
    mLabelSlogen->setFixedSize(728, 50);

    mLabelDescribe = new QLabel(this);
    font.setBold(false);
    font.setPixelSize(16);
    mLabelDescribe->setFont(font);
    //    mLabelDescribe->setStyleSheet("background:transparent;color:#8F9399;");
    mLabelDescribe->setText(tr("Clean regularly to keep your computer light and safe"));
    mLabelDescribe->resize(322, 22);
    connect(kom::UkuiGsettings::getInstance(), &kom::UkuiGsettings::fontSizeChange, this, [&]() {
        QFont font;
        font.setPointSizeF(kom::KomUtils::adaptFontSize(16));
        mLabelDescribe->setFont(font);
    });

    //    mLabelTotal = new QLabel(this);
    //    font.setPixelSize(14);
    //    font.setBold(false);
    //    mLabelTotal->setFont(font);
    //    mLabelTotal->setText(tr("Garbage 2.1 has been cleaned up"));
    //    //mLabelTotal->setGeometry(48,136,140,22);
    //    mLabelTotal->resize(140,22);
    //    mLabelTotal->setVisible(false);

    QVBoxLayout *mTopLayout = new QVBoxLayout;
    mTopLayout->setSpacing(0);
    mTopLayout->setContentsMargins(0, 0, 0, 0);
    mTopLayout->addWidget(mLabelSlogen);
    mTopLayout->addSpacing(8);
    mTopLayout->addWidget(mLabelDescribe);
    //    mTopLayout->addStretch();

    mKindSystemCache = new ClearWidgetKindItem(this);
    QString btnString1 = tr("System cache");
    QString labString1 = tr("Clean up packages, thumbnails, and recycle bin");
    mKindSystemCache->setWidgetItemBtnText(btnString1);
    mKindSystemCache->setWidgetItemTipText(labString1);
    mKindSystemCache->changeSystemFontSize(mFontSize);
    connect(mKindSystemCache, &ClearWidgetKindItem::sigKindItemClick, [this]() {
        slotShowSelectItemDialog("mKindSystemCache");
    });

    mKindCookies = new ClearWidgetKindItem(this);
    QString btnString2 = tr("Cookies");
    QString labString2 = tr("Clean up Internet history,cookies");
    mKindCookies->setWidgetItemBtnText(btnString2);
    mKindCookies->setWidgetItemTipText(labString2);
    mKindCookies->changeSystemFontSize(mFontSize);
    connect(mKindCookies, &ClearWidgetKindItem::sigKindItemClick, [this]() {
        slotShowSelectItemDialog("mKindCookies");
    });

    mKindHistory = new ClearWidgetKindItem(this);
    QString btnString3 = tr("History trace");
    QString labString3 = tr("Clean up system usage traces");
    mKindHistory->setWidgetItemBtnText(btnString3);
    mKindHistory->setWidgetItemTipText(labString3);
    mKindHistory->changeSystemFontSize(mFontSize);
    connect(mKindHistory, &ClearWidgetKindItem::sigKindItemClick, [this]() {
        slotShowSelectItemDialog("mKindHistory");
    });

    mLabelPic = new QLabel(this);
    mLabelPic->setFixedSize(256, 256);
    mLabelPic->setScaledContents(true);

    changeThemeColor(themeColor);

    QVBoxLayout *mMidLayout = new QVBoxLayout;
    mMidLayout->setSpacing(0);
    mMidLayout->setContentsMargins(0, 0, 0, 0);
    mMidLayout->addWidget(mKindSystemCache);
    mMidLayout->addWidget(mKindCookies);
    mMidLayout->addWidget(mKindHistory);
    mMidLayout->setAlignment(mKindSystemCache, Qt::AlignLeft);
    mMidLayout->setAlignment(mKindCookies, Qt::AlignLeft);
    mMidLayout->setAlignment(mKindHistory, Qt::AlignLeft);
    // mMidLayout->setStretch();

    mBtnOneKeyClear = new QPushButton(this);
    mBtnOneKeyClear->setFixedSize(180, 52);
    mBtnOneKeyClear->setProperty("isImportant", true);
    QFont btnFont;
    btnFont.setPixelSize(24);
    mBtnOneKeyClear->setFont(btnFont);
    mBtnOneKeyClear->setText(tr("StartClear"));
    connect(mBtnOneKeyClear, &QPushButton::clicked, this, &ClearMainWidget::slotOnClickedClearBtn);

    QHBoxLayout *mBottomLayout = new QHBoxLayout;
    mBottomLayout->setSpacing(0);
    mBottomLayout->setContentsMargins(0, 0, 0, 0);
    mBottomLayout->addWidget(mBtnOneKeyClear);
    mBottomLayout->addStretch(1);

    QVBoxLayout *mVLayout2 = new QVBoxLayout;
    mVLayout2->setSpacing(0);
    mVLayout2->setContentsMargins(0, 0, 0, 0);
    mVLayout2->addLayout(mMidLayout);
    mVLayout2->addSpacing(15);
    mVLayout2->addLayout(mBottomLayout);

    QHBoxLayout *mMidHLayout = new QHBoxLayout;
    mMidHLayout->setSpacing(0);
    mMidHLayout->setContentsMargins(0, 0, 0, 0);
    mMidHLayout->addLayout(mVLayout2);
    mMidHLayout->addStretch();
    mMidHLayout->addWidget(mLabelPic);

    QVBoxLayout *mMainLayout = new QVBoxLayout;
    mMainLayout->setContentsMargins(40, 0, 40, 40);
    mMainLayout->addStretch();
    mMainLayout->addLayout(mTopLayout);
    mMainLayout->addSpacing(5);
    mMainLayout->addLayout(mMidHLayout);
    mMainLayout->addStretch();

    this->setLayout(mMainLayout);
}

void ClearMainWidget::changeThemeColor(int color)
{
    if (color) {
        QString icon1 = ":/res/dark/system_grey.svg";
        mKindSystemCache->setWidgetItemIcon(icon1);
        QString icon2 = ":/res/dark/cookie_grey.svg";
        mKindCookies->setWidgetItemIcon(icon2);
        QString icon3 = ":/res/dark/trace_grey.svg";
        mKindHistory->setWidgetItemIcon(icon3);

        QString icon4 = ":/res/svg/clear_brush_dark.png";
        QPixmap pix(icon4);
        mLabelPic->setPixmap(pix);
    } else {
        QString icon1 = ":/res/light/system_grey.svg";
        mKindSystemCache->setWidgetItemIcon(icon1);
        QString icon2 = ":/res/light/cookie_grey.svg";
        mKindCookies->setWidgetItemIcon(icon2);
        QString icon3 = ":/res/light/trace_grey.svg";
        mKindHistory->setWidgetItemIcon(icon3);

        QString icon4 = ":/res/svg/clear_brush_light.png";
        QPixmap pix(icon4);
        mLabelPic->setPixmap(pix);
    }
}
void ClearMainWidget::changeSystemSize(int size)
{
    mFontSize = size;
    //    QFont font;

    mKindSystemCache->changeSystemFontSize(mFontSize);
    mKindCookies->changeSystemFontSize(mFontSize);
    mKindHistory->changeSystemFontSize(mFontSize);

    //    float btnSize = 24 * mFontSize / 11;
    //    font.setPixelSize(int(btnSize));
    //    mBtnOneKeyClear->setFont(font);
}

void ClearMainWidget::initDataList()
{
    if (isAdminUser()) {
        cache_list << tr("Cleanup Package Cache") << tr("Cleanup Thumbnails Cache") << tr("Cleanup Qaxbrowser Cache")
                   << tr("Cleanup Trash Box");
        cache_status_list << "apt"
                          << "thumbnails"
                          << "qaxbrowser"
                          << "trash";
    } else {
        cache_list << tr("Cleanup Thumbnails Cache") << tr("Cleanup Qaxbrowser Cache") << tr("Cleanup Trash Box");
        cache_status_list << "thumbnails"
                          << "qaxbrowser"
                          << "trash";
    }

#if 0
    cache_list /* << tr("Cleanup Package Cache") */
               << /* tr("Cleanup Software Center Cache") <<*/ tr("Cleanup Thumbnails Cache")
               << /*tr("Cleanup FireFox Cache") << tr("Cleanup Chromium Cache") << */ tr("Cleanup Qaxbrowser Cache")
               << tr("Cleanup Trash Box");
    cache_status_list /* << "apt" */
                      << /* "software-center" << */ "thumbnails"
                      /*<< "firefox"<< "chromium"*/
                      << "qaxbrowser"
                      << "trash";
#endif
    cookies_list << tr("Cleanup the Cookies saving in Qaxbrowser") << tr("Cleanup the Cookies saving in Firefox");
    /* << tr("Cleanup the Cookies saving in Chromium")*/

    cookies_status_list << "qaxbrowser"
                        << "firefox" /*<< "chromium"*/;
    trace_list << /*tr("Clean up the Firefox Internet records") << tr("Clean up the Chromium Internet records") <<
                                       tr("Clean up the Qaxbrowser Internet records") <<*/
        tr("Clean up the recently opened documents records")
               << tr("Delete the command history") /* << tr("Delete the debug logs")*/;
    trace_status_list << /*"firefox" << "chromium" << "qaxbrowser" <<*/ "system"
                      << "bash" /*<< "X11"*/;

    onRefreshSelectedList();

    m_selectedCache = cache_status_list;
    m_selectedCookie = cookies_status_list;
    m_selectedTrace = trace_status_list;
}

void ClearMainWidget::slotShowSelectItemDialog(QString str)
{
    onRefreshSelectedList();

    SelectCategoryWidget *w = nullptr;
    if (str == "mKindSystemCache") {
        if (flag_cache) {
            m_selectedCache.clear();
            m_selectedCache = cache_status_list;
            flag_cache = false;
        }
        w = new SelectCategoryWidget(CleanerCategoryID::CacheCategory, tr("System cache"), false, this);
        w->loadData(cache_list, m_selectedCache, cache_status_list);
        connect(w, SIGNAL(refreshSelectedItems(CleanerCategoryID, QStringList)), this,
                SLOT(onRefreshSelectedItems(CleanerCategoryID, QStringList)));
    } else if (str == "mKindCookies") {
        if (flag_cookie) {
            m_selectedCookie.clear();
            m_selectedCookie = cookies_status_list;
            flag_cookie = false;
        }
        w = new SelectCategoryWidget(CleanerCategoryID::CookieCategory, tr("Cookies"), false, this);
        w->loadData(cookies_list, m_selectedCookie, cookies_status_list);
        connect(w, SIGNAL(refreshSelectedItems(CleanerCategoryID, QStringList)), this,
                SLOT(onRefreshSelectedItems(CleanerCategoryID, QStringList)));
    } else if (str == "mKindHistory") {
        if (flag_trace) {
            m_selectedTrace.clear();
            m_selectedTrace = trace_status_list;
            flag_trace = false;
        }
        w = new SelectCategoryWidget(CleanerCategoryID::TraceCategory, tr("History trace"), false, this);
        w->loadData(trace_list, m_selectedTrace, trace_status_list);
        connect(w, SIGNAL(refreshSelectedItems(CleanerCategoryID, QStringList)), this,
                SLOT(onRefreshSelectedItems(CleanerCategoryID, QStringList)));
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

void ClearMainWidget::onRefreshSelectedList()
{
    /* 每次点击都更新浏览器的存在数据 */
    Browser_to_judge_existence();

    /* 对firefox浏览器存在添加选择字段，如不存在则去除选择字段 */
    if (firefox) {
        if (!cache_status_list.contains("firefox")) {
            cache_list << tr("Cleanup FireFox Cache");
            cache_status_list << "firefox";
        }
        if (!cookies_status_list.contains("firefox")) {
            cookies_list << tr("Cleanup the Cookies saving in Firefox");
            cookies_status_list << "firefox";
        }
        // if(!trace_status_list.contains("firefox")) {
        //	trace_list << tr("Clean up the Firefox Internet records");
        //	trace_status_list << "firefox";
        //}
    } else {
        if (cache_status_list.contains("firefox")) {
            cache_list.removeOne(tr("Cleanup FireFox Cache"));
            cache_status_list.removeOne("firefox");
        }
        if (cookies_status_list.contains("firefox")) {
            cookies_list.removeOne(tr("Cleanup the Cookies saving in Firefox"));
            cookies_status_list.removeOne("firefox");
        }
        // if(trace_status_list.contains("firefox")) {
        //	trace_list.removeOne(tr("Clean up the Firefox Internet records"));
        //	trace_status_list.removeOne("firefox");
        //}
    }

    /* 对奇安信浏览器存在添加选择字段，如不存在则去除选择字段 */
    if (qaxbrowser) {
        if (!cache_status_list.contains("qaxbrowser")) {
            cache_list << tr("Cleanup Qaxbrowser Cache");
            cache_status_list << "qaxbrowser";
        }
        if (!cookies_status_list.contains("qaxbrowser")) {
            cookies_list << tr("Cleanup the Cookies saving in Qaxbrowser");
            cookies_status_list << "qaxbrowser";
        }
        // if(!trace_status_list.contains("qaxbrowser")) {
        //	trace_list << tr("Clean up the Qaxbrowser Internet records");
        //	trace_status_list << "qaxbrowser";
        //}
    } else {
        if (cache_status_list.contains("qaxbrowser")) {
            cache_list.removeOne(tr("Cleanup Qaxbrowser Cache"));
            cache_status_list.removeOne("qaxbrowser");
        }
        if (cookies_status_list.contains("qaxbrowser")) {
            cookies_list.removeOne(tr("Cleanup the Cookies saving in Qaxbrowser"));
            cookies_status_list.removeOne("qaxbrowser");
        }
        // if(trace_status_list.contains("qaxbrowser")) {
        //	trace_list.removeOne(tr("Clean up the Qaxbrowser Internet records"));
        //	trace_status_list.removeOne("qaxbrowser");
        //}
    }
#if 0
	/* 对google浏览器存在添加选择字段，如不存在则去除选择字段 */
	if (google) {
		if (!cache_status_list.contains("chromium")) {
			cache_list << tr("Cleanup Chromium Cache");
			cache_status_list << "chromium";
		}
		if (!cookies_status_list.contains("chromium")) {
			cookies_list << tr("Cleanup the Cookies saving in Chromium");
			cookies_status_list << "chromium";
		}
		if(!trace_status_list.contains("chromium")) {
			trace_list << tr("Clean up the Chromium Internet records");
			trace_status_list << "chromium";
		}
	} else {
		if (cache_status_list.contains("chromium")) {
			cache_list.removeOne(tr("Cleanup Chromium Cache"));
			cache_status_list.removeOne("chromium");
		}
		if (cookies_status_list.contains("chromium")) {
			cookies_list.removeOne(tr("Cleanup the Cookies saving in Chromium"));
			cookies_status_list.removeOne("chromium");
		}
		if(trace_status_list.contains("chromium")) {
			trace_list.removeOne(tr("Clean up the Chromium Internet records"));
			trace_status_list.removeOne("chromium");
		}
	}
#endif
}

void ClearMainWidget::Browser_to_judge_existence()
{
    QFileInfo fileinfo;

    fileinfo.setFile("/usr/bin/qaxbrowser-safe-stable");
    if (fileinfo.exists())
        qaxbrowser = true;
    else
        qaxbrowser = false;

    fileinfo.setFile("/usr/bin/firefox");
    if (fileinfo.exists()) {
        firefox = true;
    } else {
        fileinfo.setFile("/usr/bin/firefox-esr");
        if (fileinfo.exists())
            firefox = true;
        else
            firefox = false;
    }

#if 0
	fileinfo.setFile("/usr/bin/google-chrome-stable");
	if (fileinfo.exists())
		google = true;
	else
		google = false;


	fileinfo.setFile("/usr/bin/browser360-cn-stable");
	if (fileinfo.exists())
		browser360 = true;
	else
		browser360 = false;
#endif
}
void ClearMainWidget::onRefreshSelectedItems(CleanerCategoryID id, const QStringList &infos)
{
    //    qDebug() << "ClearMainWidget::onRefreshSelectedItems id:" << id;
    switch (id) {
    case CleanerCategoryID::CacheCategory:
        m_selectedCache.clear();
        m_selectedCache = infos;
        //        foreach (QString str, infos) {
        //            qDebug() << "ClearMainWidget::onRefreshSelectedItems" << str;
        //        }
        if (!infos.contains("trash", Qt::CaseInsensitive)) {
            qDebug() << "ClearMainWidget::onRefreshSelectedItems 不包含回收站清理";
            PublicStatus::getInstance()->setIsClearTrash(false);
        } else {
            qDebug() << "ClearMainWidget::onRefreshSelectedItems 包含回收站清理";
            // m_selectedCache.removeAll("trash");
            PublicStatus::getInstance()->setIsClearTrash(true);
        }
        break;
    case CleanerCategoryID::CookieCategory:
        m_selectedCookie.clear();
        m_selectedCookie = infos;
        break;
    case CleanerCategoryID::TraceCategory:
        m_selectedTrace.clear();
        m_selectedTrace = infos;
        break;
    default:
        break;
    }
}

// 开始扫描入口
void ClearMainWidget::slotOnClickedClearBtn()
{
    getAllScanSelectedItems();
    if (argsMap.empty() && mScanSystemMap.empty()) {
        if (alertDialog)
            alertDialog->show();
        resetDefaultStatus();
    } else {
        Q_EMIT hideThisWidget();
        Q_EMIT this->startScanSystem(argsMap);
        Q_EMIT this->sigStartScanSystem(mScanSystemMap);
    }

    // argsMap
    //  Cache:{apt, thumbnails, trash}
    //  History:{system, bash}
    // mScanSystemMap
    //  Trash:{}
    qInfo() << "python scan content: " << argsMap;
    qInfo() << "clear manager scan content: " << mScanSystemMap;
}

void ClearMainWidget::getAllScanSelectedItems()
{
    argsMap.clear();
    mScanSystemMap.clear();

    if (m_selectedCache.length() != m_selectedCache.removeAll("")) {
        argsMap.insert("Cache", m_selectedCache);
    } else {
        PublicStatus::getInstance()->setIsSystemCacheNull(true);
    }
    if (m_selectedTrace.length() != m_selectedTrace.removeAll("")) {
        argsMap.insert("History", m_selectedTrace);
    } else {
        PublicStatus::getInstance()->setIsTraceNull(true);
    }
    if (m_selectedCookie.length() != m_selectedCookie.removeAll("")) {
        mScanSystemMap.insert("Cookies", m_selectedCookie);
    } else {
        PublicStatus::getInstance()->setIsCookiesNull(true);
    }
    if (PublicStatus::getInstance()->getIsClearTrash()) {
        mSelectedTrash.clear();
        mScanSystemMap.insert("Trash", mSelectedTrash);
    }
}

void ClearMainWidget::resetDefaultStatus()
{
    m_selectedCache = cache_status_list;
    m_selectedCookie = cookies_status_list;
    m_selectedTrace = trace_status_list;
    //解决返回后无法扫描回收站的问题
    PublicStatus::getInstance()->setIsClearTrash(true);
    PublicStatus::getInstance()->setIsSystemCacheNull(false);
    PublicStatus::getInstance()->setIsTraceNull(false);
    PublicStatus::getInstance()->setIsCookiesNull(false);
}

bool ClearMainWidget::isAdminUser()
{
    QDBusInterface accounts("org.freedesktop.Accounts", QString("/org/freedesktop/Accounts/User%1").arg(getuid()),
                            "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
    if (!accounts.isValid()) {
        qCritical() << "****** kylin os manager ****** "
                    << "accounts dbus is not vaild";
        return false;
    }

    return accounts.property("AccountType").toBool();
}

} // namespace KylinRubbishClear
