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
#include "detectpagewidget.h"
#include <kysdk/applications/gsettingmonitor.h>
#include <QWindow>
#include "include/theme.h"

DetectPageWidget::DetectPageWidget(QWidget *parent) : QWidget(parent),
    m_detectPageIcon(new QLabel(this)),
    m_detectPageText(new QLabel(this)),
    m_detectPageVLayout(new QVBoxLayout(this))
{
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland")
    {
        isWayland = true;
    }
    else
    {
        isWayland = false;
    }
    installEventFilter(this);

    if (QGSettings::isSchemaInstalled(UKUI_THEME_GSETTING_PATH)) {
        m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    }

    if (QGSettings::isSchemaInstalled(FITCONTROLTRANS)) {
        m_pGsettingControlTrans = new QGSettings(FITCONTROLTRANS);
    }

    setupGui();
    initConnect();
}

void DetectPageWidget::setupGui()
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    qDebug()<<"isWayland ==="<<isWayland;
    if(isWayland) {
        QPalette p = palette();
        m_backColor = p.color(QPalette::Window);
    }else{
        this->setProperty("useSystemStyleBlur", true);
        setAutoFillBackground(true);
        setBackgroundRole(QPalette::Window);
    }

    getFileListNum();

    waitingTimer = new QTimer();
    setWaitTimerStart();

    m_detectPageIcon->setFixedSize(DetectPageIconSize);
    m_detectPageText->setEnabled(false);
    m_detectPageText->setText(tr("Detect scanners, please waiting"));

    m_detectPageVLayout->setSpacing(0);
    m_detectPageVLayout->addStretch();
    m_detectPageVLayout->addWidget(m_detectPageIcon, 0,  Qt::AlignCenter);
    m_detectPageVLayout->addSpacing(13);
    m_detectPageVLayout->addWidget(m_detectPageText, 0,  Qt::AlignCenter);
    m_detectPageVLayout->addStretch();
    m_detectPageVLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_detectPageVLayout);

    transChange();
}

void DetectPageWidget::initConnect()
{
    connect(waitingTimer, SIGNAL(timeout()), this, SLOT(showWaitImages()));
    connect(g_user_signal, &GlobalUserSignal::detectPageWaitTimerStartSignal, this, &DetectPageWidget::setWaitTimerStart);
    connect(g_user_signal, &GlobalUserSignal::detectPageWaitTimerStopSignal, this, &DetectPageWidget::setWaitTimerStop);
    connect(m_pGsettingControlTrans, &QGSettings::changed, this, &DetectPageWidget::changeControlTransSlot);
    connect(g_user_signal, &GlobalUserSignal::windowFocousSignal, this, [=](bool window) {
        if (window) {
            qDebug() << "设置透明";
            m_backColor.setAlpha(static_cast<int>(m_tran));
        } else {
            qDebug() << "设置完全不透明";
            m_backColor.setAlpha(255);
        }
    });
}

void DetectPageWidget::changeControlTransSlot(const QString &key)
{
    if (key == "transparency" || key == "effect") {
        transChange();
    }
}

void DetectPageWidget::transChange()
{
    m_effect = m_pGsettingControlTrans->get("effect").toBool();
    m_tran = m_pGsettingControlTrans->get("transparency").toDouble() * 255;
    qDebug () << m_effect <<m_tran;
    this->update();
}

void DetectPageWidget::getFileListNum()
{
    path = ":/loading/";

    num = GetFileList(path).size();

    count = 0;
}

QFileInfoList DetectPageWidget::GetFileList(QString path)
{
    QDir dir(path);
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i != folder_list.size(); i++) {
        QString name = folder_list.at(i).absoluteFilePath();
        QFileInfoList child_file_list = GetFileList(name);
        file_list.append(child_file_list);
    }

    return file_list;
}

void DetectPageWidget::showWaitImages()
{
    fileinfo = GetFileList(path).at(count);
    m_detectPageIcon->setScaledContents(true);
    themeColor = m_themeData->get("styleName").toString();
    if (themeColor.compare("ukui-dark", Qt::CaseInsensitive) == 0 || themeColor.compare("ukui-black", Qt::CaseInsensitive) == 0) {
        m_detectPageIcon->setPixmap(SVGHandler::loadSvgColor(fileinfo.filePath(), "white", 70));
    } else {
        m_detectPageIcon->setPixmap(SVGHandler::loadSvgColor(fileinfo.filePath(), "black", 70));
    }

    ++count;

    if (count == num) {
        count = 0;
    }
}

void DetectPageWidget::setWaitTimerStart()
{
    waitingTimer->start(100);
}

void DetectPageWidget::setWaitTimerStop()
{
    waitingTimer->stop();
}


void DetectPageWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), m_backColor);
    QWidget::paintEvent(event);
}
bool DetectPageWidget::eventFilter(QObject *obj, QEvent *event){
    if (event->type() == QEvent::WindowActivate) {
        m_backColor.setAlpha(static_cast<int>(m_tran));
    } else if (event->type() == QEvent::WindowDeactivate) {
        m_backColor.setAlpha(255);
    }
    return QObject::eventFilter(obj,event);
}
