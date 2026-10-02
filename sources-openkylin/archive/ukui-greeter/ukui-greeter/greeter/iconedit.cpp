/* iconedit.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "iconedit.h"
#include "imageutil.h"
#include <QHBoxLayout>
#include <QWidget>
#include <QX11Info>
#include <QPainter>
#include <QtConcurrent/QtConcurrentRun>
#include <QFile>
#include <QKeyEvent>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include "xeventmonitor.h"
#include "common/configuration.h"

IconEdit::IconEdit(QWidget *parent)
    : QWidget(parent),
      m_timer(nullptr),
      m_configuration(Configuration::instance())
{
    m_edit = new QLineEdit(this);
    m_edit->setObjectName(QStringLiteral("passwdEdit"));

    m_capsIcon = new QSvgWidget(this);
    m_capsIcon->setObjectName(QStringLiteral("capsIconLabel"));
    m_capsIcon->load(QString(":/images/capslock.svg"));

    m_iconButton = new QPushButton(this);
    m_iconButton->setObjectName(QStringLiteral("loginButton"));

    m_modeButton = new ModeButton(this);
    m_modeButton->setObjectName(QStringLiteral("echoModeButton"));
    m_modeButton->setFixedSize(24, 14);
    m_modeButton->setIconSize(QSize(16, 16));

    QHBoxLayout *layout = new QHBoxLayout(this);

    QtConcurrent::run([=](){
        layout->setContentsMargins(1, 1, 8, 1);
        layout->setSpacing(0);
        layout->addStretch();
        layout->addWidget(m_capsIcon);
        layout->addWidget(m_modeButton);
        layout->addWidget(m_iconButton);

        m_edit->setAttribute(Qt::WA_InputMethodEnabled, false); //禁用输入法
        m_edit->setContextMenuPolicy(Qt::NoContextMenu);    //禁用右键菜单
        m_edit->installEventFilter(this);
        //m_edit->setMaxLength(1000);

        m_iconButton->setFixedSize(24, 24);
        m_iconButton->setIconSize(QSize(24, 24));
        m_iconButton->setFocusPolicy(Qt::NoFocus);
        m_iconButton->setCursor(QCursor(Qt::PointingHandCursor));
        m_iconButton->installEventFilter(this);

        m_modeButton->setCheckable(true);
        m_modeButton->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
        m_modeButton->setFocusPolicy(Qt::NoFocus);
        m_modeButton->setCursor(Qt::PointingHandCursor);
        m_modeButton->installEventFilter(this);
        connect(m_modeButton, &QPushButton::clicked, this, [&](bool checked){
            setType(checked ? QLineEdit::Normal : QLineEdit::Password);
        });

        connect(m_edit, &QLineEdit::returnPressed, this, &IconEdit::clicked_cb);
        connect(m_iconButton, &QPushButton::clicked, this, &IconEdit::clicked_cb);
        if(QX11Info::isPlatformX11()){
	    connect(XEventMonitor::instance(), SIGNAL(keyRelease(const QString &)),
                this, SLOT(onGlobalKeyRelease(const QString &)));
        }
        setFocusProxy(m_edit);
        setCapsState(checkCapsState());
        setFocusin(0);
    });
}

bool IconEdit::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_edit){
        if(event->type() == QEvent::KeyPress){
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut) || keyEvent->matches(QKeySequence::Paste)){
                event->ignore();
                return true;
            }else if((keyEvent->modifiers() & Qt::MetaModifier) || (keyEvent->modifiers() & Qt::AltModifier)){//当meta或者alt键被按下时，忽略按键事件
                event->ignore();
                return true;
            }
        }
        if(event->type() == 2)
        {
            setFocusin(0);
            Q_EMIT clickedPassword(true);
        }
    }else if(obj == m_iconButton){
        if(m_timer && m_timer->isActive())
        {
            return false;
        }
        if(event->type() == QEvent::HoverEnter){
            setIcon(QIcon(":/images/login-button-hover.svg"));
        }
        if(event->type() == 2)
        {
            setFocusin(0);
            Q_EMIT clickedPassword(true);
        }
        else if(event->type() == QEvent::HoverLeave){
            setIcon(QIcon(":/images/login-button.svg"));
        } else if(event->type() == QEvent::MouseButtonRelease) { //禁用鼠标中键
            QMouseEvent *mouseevent = static_cast<QMouseEvent*>(event);
            if(mouseevent->button() == Qt::MidButton){
                event->ignore();
                return true;
            }
        } else if (event->type() == QEvent::ToolTip) {
            Q_EMIT setKToolTip(tr("OK"));
        } else if (event->type() == QEvent::Leave) {
            Q_EMIT hideKTooltip();
        }
    }else if(obj == m_modeButton){
        if(event->type() == 2)
        {
            setFocusin(0);
            Q_EMIT clickedPassword(true);
        }
    }

    return false;
}

void IconEdit::setType(QLineEdit::EchoMode type)
{
    m_edit->setEchoMode(type);
    if(type == 0){
       m_modeButton->setChecked(true);
       m_modeButton->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
    }
    else{
       m_modeButton->setChecked(false);
       m_modeButton->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
    }
}

void IconEdit::setFontSize(int curFontSize)
{
    QFont sysFont;
    sysFont = qApp->font();
    double m_ptToPx = 1.0;
    m_ptToPx = m_configuration->getPtToPx();
    sysFont.setPointSize((16 + curFontSize) *m_ptToPx);
    m_edit->setFont(sysFont);
}

void IconEdit::clearText()
{
    m_edit->setText("");
}

void IconEdit::setModeBtnVisible(bool visible)
{
    m_modeButton->setVisible(visible);
}

void IconEdit::resizeEvent(QResizeEvent *)
{
    // 设置输入框中文件输入区，不让输入的文字在被隐藏在按钮下
    int w = m_iconButton->width() + m_modeButton->width();
    m_edit->setTextMargins(1, 1, checkCapsState() ? w + m_capsIcon->width() + 10 : w + 6, 1);
    m_edit->setFixedSize(size());
}

void IconEdit::clicked_cb()
{
//    m_iconButton->setFocus();       //按回车后输入框光标会消失或者不再闪烁，先让其他控件获取焦点，就会解决该问题
    //startWaiting();
    emit clicked(m_edit->text());
}

void IconEdit::setFocusin(int target)
{
    switch (target) {
    case 0:
        m_edit->setFocus();
        m_iconButton->setStyleSheet("background:#3D6BE5; border-radius:12px; border:2px solid #3D6BE5;");
        m_iconButton->setIconSize(QSize(24, 24));
        break;
    case 1:
        m_edit->clearFocus();
        m_iconButton->setFocus();
        m_iconButton->setStyleSheet("background:#3D6BE5; border-radius:12px; border:2px solid #000000;");
        m_iconButton->setIconSize(QSize(22, 22));
        break;
    default:
        m_iconButton->setStyleSheet("background:#3D6BE5; border-radius:12px; border:2px solid #3D6BE5;");
        m_iconButton->setIconSize(QSize(24, 24));
        m_edit->clearFocus();
        break;
    }
}

void IconEdit::onGlobalKeyRelease(const QString &key)
{
    if(key == "Caps_Lock")
    {
        setCapsState(!m_capsIcon->isVisible());
    }
    else if(key == "Num_Lock")
    {
        bool numlockState = m_configuration->getLastNumLock();
        m_configuration->saveLastNumLock(!numlockState);
    }
}

void IconEdit::setCapsState(bool capsState)
{
    m_capsIcon->setVisible(capsState);
    int w = m_iconButton->width() + m_modeButton->width();
    m_edit->setTextMargins(1, 1, capsState ? w + m_capsIcon->width() + 10 : w + 6, 1);
    m_edit->setFixedSize(size());
}

void IconEdit::setIcon(const QString &text)
{
    m_iconButton->setIcon(QIcon());
    m_iconButton->setText(text);
    m_iconText = text;
    m_icon = QIcon();
}

void IconEdit::setIcon(const QIcon &icon)
{
    m_iconButton->setIcon(icon);
    m_iconButton->setText("");
    m_iconButton->setIconSize(QSize(24, 24));
    m_icon = icon;
    m_iconText = "";
}

void IconEdit::clear()
{
    m_edit->setText("");
    setPrompt("");
}

void IconEdit::setPrompt(const QString &prompt)
{
   // m_edit->setPlaceholderText(prompt);
    if(prompt == "Password: " || prompt == "密码：")
        m_edit->setPlaceholderText(tr("Password: "));
    else if(prompt == "Username" || prompt == "用户名")
        m_edit->setPlaceholderText(tr("Username"));
    else
        m_edit->setPlaceholderText(prompt);
}

const QString IconEdit::text()
{
    return m_edit->text();
}

void IconEdit::startWaiting()
{
    m_edit->setReadOnly(true);
    //m_iconButton->setEnabled(false);

    if(!m_timer)
    {
        m_timer = new QTimer(this);
        m_timer->setInterval(150);
        connect(m_timer, &QTimer::timeout, this, &IconEdit::updatePixmap);
    }


//    m_waitingPixmap.load(":/images/waiting.png");
//    m_iconButton->setIconSize(m_iconButton->size());

    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(16,16);
    m_waitingPixmap = ImageUtil::drawSymbolicColoredPixmap(icon, "white");
    m_iconButton->setIcon(QIcon(m_waitingPixmap));
    m_iconButton->setIconSize(QSize(16, 16));
    m_timer->start();
}

void IconEdit::readOnly(bool enabled)
{
    m_edit->setEnabled(!enabled);
    m_edit->setReadOnly(enabled);
}

void IconEdit::setLocked(bool lock)
{
    m_iconButton->blockSignals(lock);
}
void IconEdit::refreshTranslate()
{
     if(m_edit->placeholderText() == "Password: " || m_edit->placeholderText() == "密码：")
         m_edit->setPlaceholderText(tr("Password: "));
     if(m_edit->placeholderText() == "Username" || m_edit->placeholderText() == "用户名")
         m_edit->setPlaceholderText(tr("Username"));
}

void IconEdit::stopWaiting()
{
    if(m_timer && m_timer->isActive())
    {
        m_timer->stop();
    }
    m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_edit->setReadOnly(false);
    //m_iconButton->setEnabled(true);
    if(!m_icon.isNull()) {
        m_iconButton->setIcon(m_icon);
        m_iconButton->setIconSize(QSize(24, 24));
    }
    else
        m_iconButton->setText(m_iconText);
}

void IconEdit::updatePixmap()
{
    m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    m_iconButton->setIcon(QIcon(m_waitingPixmap));
    m_iconButton->setIconSize(QSize(16, 16));
}
