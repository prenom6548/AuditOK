/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "about.h"
#include "xatom-helper.h"
#include "utils.h"

#include "wayland/ukui/ukui-decoration-manager.h"
#include "wayland/xdg/XdgManager.h"

#include "sessionsettings.h"

#include <QApplication>
#include <QLabel>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QVariant>
#include <QX11Info>
#include <QMouseEvent>
#include <QSettings>

About::About(const QString &icon, QWidget *parent)
    : QDialog(parent)
    , m_icon(icon)
{
#ifdef KYLIN_V10
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
#else
    this->setWindowFlags(windowFlags() | Qt::Tool);
#endif

    this->setBackgroundRole(QPalette::Base);//this->setBackgroundRole(QPalette::Window);
    this->setAutoFillBackground(true);
    //this->setWindowIcon(QIcon(m_icon));//wayland下设置无效
    //this->setWindowTitle(tr("KMRE"));
    this->setFocusPolicy(Qt::NoFocus);
    this->setMouseTracking(true);
    this->setFixedSize(420, 450);

#ifndef KYLIN_V10
    // 添加窗管协议
    if (SessionSettings::getInstance().windowUsePlatformX11() &&
            QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
#endif

    initUI();
    initConnection();

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        installEventFilter(this);
    }
}

About::~About()
{

}

void About::initUI()
{
    m_mainVLayout = new QVBoxLayout();
    m_titleLayout = new QHBoxLayout();
    m_centerIconLayout = new QHBoxLayout();
    m_centerTitleLayout = new QHBoxLayout();
    m_centerVersionLayout = new QHBoxLayout();
    m_detailLayout = new QHBoxLayout();
    m_developerLayout = new QHBoxLayout();

    m_mainVLayout->setContentsMargins(8, 4, 4, 4);
    m_mainVLayout->setSpacing(10);
    m_titleLayout->setSpacing(0);

    QIcon titleIcon = QIcon::fromTheme("kmre");
    if (titleIcon.isNull()) {
        titleIcon = QIcon(":/res/kmre.svg");
    }

    m_iconLabel  = new QLabel();
    m_iconLabel->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(24, 24))));

    m_titleLabel = new QLabel(tr("KMRE"));

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);
    m_closeBtn->setFlat(true);
    m_closeBtn->setFixedSize(30, 30);
    m_closeBtn->setToolTip(tr("Close"));

    m_titleLayout->addWidget(m_iconLabel);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_closeBtn);

    m_centerIcon = new QLabel();
    m_centerIcon->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(96, 96))));

    m_centerIconLayout->addStretch();
    m_centerIconLayout->addWidget(m_centerIcon);
    m_centerIconLayout->addStretch();

    m_centerTitleLabel = new QLabel(tr("KMRE"));
    m_centerTitleLabel->setFixedHeight(28);

    m_centerTitleLayout->addStretch();
    m_centerTitleLayout->addWidget(m_centerTitleLabel);
    m_centerTitleLayout->addStretch();

    m_versionLabel = new QLabel(tr("Version: ") + getKmreVersion());
    m_versionLabel->setFixedHeight(24);

    m_centerVersionLayout->addStretch();
    m_centerVersionLayout->addWidget(m_versionLabel);
    m_centerVersionLayout->addStretch();

    m_detailTextEdit = new QTextEdit(tr("KMRE is a Android App compatible system environment created by Kylin team for Kylin OS, which is used to meet the diversified needs of users for application software. KMRE enables users to install and run Android apps in Kylin OS, such as games, wechat, QQ, stock, video apps, etc."));
    m_detailTextEdit->setReadOnly(true);
    m_detailTextEdit->setFrameShape(QFrame::NoFrame);
    m_detailLayout->addSpacing(32);
    m_detailLayout->addWidget(m_detailTextEdit);
    m_detailLayout->addSpacing(32);

    m_developerLabel = new QLabel(tr("Service and Support:"));

    m_developerLayout->addSpacing(32);
    m_developerLayout->addWidget(m_developerLabel);

    m_developerEmailBtn = new QPushButton("support@kylinos.cn");
    m_developerEmailBtn->setFocusPolicy(Qt::NoFocus);
    m_developerEmailBtn->setContentsMargins(0,0,0,0);
    m_developerEmailBtn->setCursor(QCursor(Qt::PointingHandCursor));
    m_developerEmailBtn->setStyleSheet("QPushButton{background: transparent;border-radius: 4px;text-decoration: underline;} ");

    m_developerLayout->addWidget(m_developerEmailBtn);
    m_developerLayout->setAlignment(Qt::AlignLeft);
    connect(m_developerEmailBtn, &QPushButton::clicked, this,[=] {
        QDesktopServices::openUrl(QUrl(QLatin1String("mailto:support@kylinos.cn")));
    });

    m_mainVLayout->addLayout(m_titleLayout);
    m_mainVLayout->addLayout(m_centerIconLayout);
    m_mainVLayout->addLayout(m_centerTitleLayout);
    m_mainVLayout->addLayout(m_centerVersionLayout);
    m_mainVLayout->addLayout(m_detailLayout);
    m_mainVLayout->addLayout(m_developerLayout);
    m_mainVLayout->addSpacing(20);

    this->setLayout(m_mainVLayout);
}

void About::initConnection()
{
    connect(m_closeBtn, &QPushButton::clicked, this, [=]() {
        this->close();
    });
}

QString About::getKmreVersion()
{
    QString version;
    QStringList options;

    QString confName = "/usr/share/kmre/kmre-update.conf";
    if (QFile::exists(confName)) {
        QSettings settings(confName, QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.beginGroup("update");
        version = settings.value("update_revision", QString("")).toString();
        settings.endGroup();
    }

    if (!version.isEmpty() && !version.isNull()) {
        return QString("2.0-%1").arg(version);
    }

    //__mips__  __sw_64__
#ifdef __x86_64__
    options << "-W" << "-f=${Version}\n" << "kylin-kmre-image-data-x64";
#elif __aarch64__
    options << "-W" << "-f=${Version}\n" << "kylin-kmre-image-data";
#endif

    if (options.length() > 0) {
        QByteArray data;
        QProcess process;
        process.start("dpkg-query", options);
        process.waitForFinished();
        process.waitForReadyRead();
        data = process.readAllStandardOutput();
        version = QString(data);
        version.replace(QString("\n"), QString(""));
        return version;
    }

    return qApp->applicationVersion();
}

void About::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    // wayland 窗口无边框，需要在窗口show()之后执行
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        UKUIDecorationManager::getInstance()->removeHeaderBar(windowHandle());
        UKUIDecorationManager::getInstance()->setCornerRadius(this->windowHandle(), 12, 12, 12, 12);
    }
}

bool About::eventFilter(QObject *watched, QEvent *event)
{
    if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
        return false;
    }

    if(watched == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                if (SessionSettings::getInstance().hasWaylandUKUIDecorationSupport()) {
                    UKUIDecorationManager::getInstance()->moveWindow(this->windowHandle());
                } else if (SessionSettings::getInstance().supportsWaylandXdgShell()) {
                    XdgManager::getInstance()->moveWindow(this->windowHandle());
                }
            }
        }
    }
    return false;
}

/*
About::About(const QString &icon, QWidget *parent)
    : QScrollArea(parent)
{
    this->setWindowFlags(Qt::Tool);
    this->setAttribute(Qt::WA_ShowModal, true);
    this->setWidgetResizable(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setFrameStyle(QFrame::NoFrame);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    this->setContentsMargins(0, 0, 0, 0);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowIcon(QIcon(icon));//wayland下设置无效
    this->setWindowTitle(tr("About"));
    this->setFixedSize(500, 300);
    this->setMinimumWidth(500);

    QLabel *label = new QLabel;
    label->setWordWrap(true);
    label->setMargin(20);
    this->setWidget(label);

    const QString &text = QString("<body style=\"line-height: 18px;font-size:12px;font-style: normal;\">"
                                  "<div style=\"font-style: medium;font-size: 16px;\">%1</div>"
                                  "<ul><li>%2</li></ul>"
                                  "<div style=\"font-style: medium;font-size: 16px;\">%3</div>"
                                  "<ul><li>%4</li></ul>"
                                  "<div style=\"font-style: medium;font-size: 16px;\">%5</div>"
                                  "<ul><li><a href= \"mailto://support@kylinos.cn\" style=\"color: palette(buttonText)\"> support@kylinos.cn</a ></li></ul>"
                                  "</body>")

            .arg(tr("About us"))
            .arg(tr("Kmre is a Android App compatible system environment created by Kylin team for Kylin OS, which is used to meet the diversified needs of users for application software. Kmre enables users to install and run Android apps in Kylin OS, such as games, wechat, QQ, stock, video apps, etc."))
            .arg(tr("Copyright information"))
            .arg(tr("Copyright 2020-2021. kylinos.cn. All Rights Reserved."))
            .arg(tr("Service & Support Team"));// Service & Support
//            .arg(link("www.kylinos.cn"));

    label->setText(text);
    label->setContextMenuPolicy(Qt::NoContextMenu);
    connect(label, &QLabel::linkActivated, this, [=](const QString url) {
        QDesktopServices::openUrl(QUrl(url));
    });
}

QString About::link(const QString &url, QString name)
{
    if (name.isEmpty()) {
        name = url;
    }

    return QString("<a href=\"" + url + "\">" + name + "</a>");
}
*/
