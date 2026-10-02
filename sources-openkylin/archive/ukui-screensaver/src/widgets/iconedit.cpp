/* iconedit.cpp
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include <QHBoxLayout>
#include <QWidget>
#include <QPainter>
#include <QtConcurrent/QtConcurrentRun>
#include <QFile>
#include <QKeyEvent>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QRegExp>
#include <QRegExpValidator>
#include <QFontMetrics>
#include <QCursor>
#include <QToolTip>
#include "commonfunc.h"
#include "utils.h"

IconEdit::IconEdit(LockDialogModel *model, QWidget *parent, int editType) : QWidget(parent), m_modelLockDialog(model)
{
    this->setFixedSize(240, 40);
    m_editType = editType;
    m_edit = new QLineEdit(this);
    m_edit->setProperty("needTranslucent", false);

    m_capslockStateHelper = CapslockStateHelper::instance(this);
    connect(m_capslockStateHelper, &CapslockStateHelper::capslockChanged, this, [=](bool state) {
        qDebug() << " CapsLock changed !!! "
                 << " state = " << state;
        m_capsState = state;
        onCapsChanged(m_capsState);
    });
    m_capsIcon = new QLabel(this);
    m_capsIcon->setPixmap(QIcon::fromTheme("ukui-capslock-symbolic").pixmap(QSize(16, 16)));
    m_capsIcon->setVisible(m_capsState);

    m_modeButton = new KyIconButton(EDITBUTTON, this);
    m_modeButton->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
    m_modeButton->setFlat(true);
    m_modeButton->setFixedSize(24, 14);
    m_modeButton->setIconSize(QSize(16, 16));
    m_modeButton->setFocusPolicy(Qt::NoFocus);
    m_modeButton->installEventFilter(this);
    connect(m_modeButton, &QPushButton::clicked, this, [=]() {
        if (m_edit->echoMode() == QLineEdit::Password) {
            setType(QLineEdit::Normal);
        } else {
            setType(QLineEdit::Password);
        }
    });

    m_iconButton = new QPushButton(this);
    m_iconButton->setFixedSize(24, 24);
    m_iconButton->setProperty("isRoundButton", true);
    m_iconButton->setProperty("isImportant", true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 1, 8, 1);
    layout->setSpacing(2);
    layout->addStretch();
    layout->addWidget(m_capsIcon);
    layout->addWidget(m_modeButton);
    layout->addWidget(m_iconButton);
    m_edit->setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法
    QRegExp inputRegExp("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\/?:;\"'|\{} ]*$");
    QRegExpValidator *inputLimits = new QRegExpValidator(inputRegExp, this);

    if (m_editType == CONFIDENTIALEDIT) {
        m_iconButton->hide();
        m_modeButton->hide();
        setType(QLineEdit::Normal);
        m_edit->setAttribute(Qt::WA_InputMethodEnabled, true);
    } else if (m_editType == RESETPWDEDIT) {
        m_iconButton->hide();
        setType(QLineEdit::Password);
        m_edit->setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法
        m_edit->setValidator(inputLimits);
    } else if (m_editType == KYLINIDEDIT) {
        m_iconButton->hide();
        m_modeButton->hide();
        setType(QLineEdit::Normal);
        m_edit->setAttribute(Qt::WA_InputMethodEnabled, false);
    } else {
        connect(m_edit, &QLineEdit::returnPressed, this, &IconEdit::clicked_cb);
        m_edit->setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法
        m_edit->setValidator(inputLimits);
    }
    connect(m_iconButton, &QPushButton::clicked, this, &IconEdit::clicked_cb);
    connect(m_edit, &QLineEdit::textChanged, this, [=](const QString &text) { Q_EMIT textChanged(text); });
    //    connect(m_edit, &QLineEdit::textChanged, this, [=](QString text){
    //        if (!text.isEmpty()) {
    //            setLayoutDirection(Qt::LeftToRight);
    //        } else if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
    //            setLayoutDirection(Qt::RightToLeft);
    //        }
    //    });

    m_edit->setContextMenuPolicy(Qt::NoContextMenu); // 禁用右键菜单
    m_edit->installEventFilter(this);
    m_iconButton->setFocusPolicy(Qt::TabFocus);
    m_iconButton->installEventFilter(this);
    m_iconButton->setToolTip(tr("OK"));
    this->setFocusProxy(m_edit);

    QLocale local;
    systemLang = local.name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        setLayoutDirection(Qt::RightToLeft);
    } else {
        setLayoutDirection(Qt::LeftToRight);
    }
}

void IconEdit::resizeEvent(QResizeEvent *event)
{
    if (m_widgetSize == event->size()) {
        return;
    }
    m_widgetSize = event->size();
    // 设置输入框中文件输入区，不让输入的文字在被隐藏在按钮下
    updateEditTextMargins();
    m_edit->setFixedSize(size());
}

void IconEdit::hideEvent(QHideEvent *event)
{
    if (m_edit) {
        m_edit->hide();
    }
    QWidget::hideEvent(event);
}

void IconEdit::showEvent(QShowEvent *event)
{
    if (m_edit) {
        m_edit->show();
    }
    QWidget::showEvent(event);
}

bool IconEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_edit) {
        if (event->type() == 6) { // 禁止复制粘贴功能。
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut)
                || keyEvent->matches(QKeySequence::Paste)) {
                event->ignore();
                return true;
            } else if (
                (keyEvent->modifiers() & Qt::MetaModifier) || (keyEvent->modifiers() & Qt::AltModifier)
                || (keyEvent->modifiers() & Qt::ControlModifier)) { // 当meta或者alt键或ctrl键被按下时，记录日志
                qInfo() << "Key Modifiers event:" << keyEvent->modifiers();
            }
        }
        if (event->type() == QEvent::MouseButtonPress) {
            Q_EMIT clickedPassword();
        } else if (event->type() == QEvent::ToolTip) {
            m_isShowEditToolTip = true;
            measurePlaceholderText();
        } else if (event->type() == QEvent::Leave) {
            hideKToolTip();
            m_isShowEditToolTip = false;
        }
        if (event->type() == 23) {
            update();
        } else if (event->type() == QEvent::MouseButtonPress) {
            update();
        } else if (event->type() == QEvent::MouseButtonRelease) { // 禁用鼠标中键
            QMouseEvent *mouseevent = static_cast<QMouseEvent *>(event);
            if (mouseevent->button() == Qt::MidButton) {
                event->ignore();
                return true;
            }
        }
    }
    if (obj == m_modeButton) {
        if (event->type() == 2) {
        }
    }
    if (obj == m_iconButton) {
        if (event->type() == QEvent::FocusIn) {
            setIconButtonSize(QSize(26, 26));
        } else if (event->type() == QEvent::FocusOut) {
            setIconButtonSize(QSize(24, 24));
        }
    }
    if (event->type() == QEvent::MouseButtonPress) {
        Q_EMIT mousePressed();
    }
    if (obj == m_iconButton && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            m_iconButton->click();
            return true;
        }
    }
    return false;
}

void IconEdit::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        refreshTranslate();
    }
}
void IconEdit::refreshTranslate()
{
    QLocale local;
    if (systemLang == local.name()) {
        return;
    }
    systemLang = local.name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        setLayoutDirection(Qt::RightToLeft);
        m_edit->setAlignment(Qt::AlignRight);
    } else {
        setLayoutDirection(Qt::LeftToRight);
        m_edit->setAlignment(Qt::AlignLeft);
    }
    updateEditTextMargins();
    if (m_iconButton) {
        m_iconButton->setToolTip(tr("OK"));
    }
}

void IconEdit::setIcon(const QIcon &icon, const QSize &size)
{
    QPixmap pixmap = icon.pixmap(size.width(), size.height());
    m_iconButton->setIcon(pixmap);
    m_iconButton->setIconSize(QSize(size.width(), size.height()));
    m_iconButton->setText("");
    m_sizeIconBtn = size;
    m_icon = icon;
    m_iconText = "";
}

void IconEdit::clear()
{
    m_edit->setText("");
    setPrompt("");
}

void IconEdit::clearText()
{
    m_edit->setText("");
}

void IconEdit::setPrompt(const QString &prompt)
{
    m_edit->setPlaceholderText(prompt);
}

const QString IconEdit::text()
{
    return m_edit->text();
}

void IconEdit::setType(QLineEdit::EchoMode type)
{
    m_edit->setEchoMode(type);
    if (type == 0) {
        m_modeButton->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
    } else {
        m_modeButton->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
    }
}

void IconEdit::readOnly(bool enabled)
{
    m_edit->setEnabled(!enabled);
    m_edit->setReadOnly(enabled);
    if (!enabled) {
        m_edit->setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法
    }
}

void IconEdit::setLocked(bool lock)
{
    m_iconButton->blockSignals(lock);
}

void IconEdit::setModeBtnVisible(bool visible)
{
    m_modeButton->setVisible(visible);
}

void IconEdit::startWaiting()
{
    m_edit->setReadOnly(true);
    // m_iconButton->setEnabled(false);

    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(150);
        connect(m_timer, &QTimer::timeout, this, &IconEdit::updatePixmap);
    }

    // 这里将QIcon转QPixmap传入(24,24)时，实际得到的pixmap大小为(24,24)乘上缩放比例，导致计算pixmap位置错误，因此需要重新设置一次大小
    // 更好的解决方案应该是重写IconButton的setIcon函数?直接处理图片大小与icon大小一致?
    //    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(16,16).scaled(16,16, Qt::IgnoreAspectRatio,
    //    Qt::SmoothTransformation);
    m_waitingPixmap = getLoadingIcon(16);
    m_iconButton->setIcon(m_waitingPixmap);
    m_iconButton->setIconSize(QSize(16, 16));
    m_timer->start();
}

void IconEdit::stopWaiting()
{
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
    }
    m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_edit->setReadOnly(false);
    m_edit->setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法
    if (!m_icon.isNull()) {
        QPixmap pixmap = m_icon.pixmap(m_sizeIconBtn.width(), m_sizeIconBtn.height());
        m_iconButton->setIcon(pixmap);
        m_iconButton->setIconSize(QSize(m_sizeIconBtn.width(), m_sizeIconBtn.height()));
    } else {
        m_iconButton->setText(m_iconText);
    }
}

void IconEdit::updateUI()
{   
    setIconButtonSize(m_iconButton->hasFocus() ? QSize(26, 26) : QSize(24, 24));
    if (!m_icon.isNull()) {
        QPixmap pixmap = m_icon.pixmap(m_sizeIconBtn.width(), m_sizeIconBtn.height());
        m_iconButton->setIcon(pixmap);
        m_iconButton->setIconSize(QSize(m_sizeIconBtn.width(), m_sizeIconBtn.height()));
    } else {
        m_iconButton->setText(m_iconText);
    }
    m_modeButton->setFixedSize(QSize(24, 24));
    m_modeButton->setIconSize(QSize(16, 16));
    m_capsIcon->setPixmap(QIcon::fromTheme("ukui-capslock-symbolic").pixmap(QSize(16, 16)));
}

void IconEdit::setFontSize(double fontSize)
{
    QFont font = qApp->font();
    font.setPointSize(fontSize);
    m_edit->setFont(font);
}

void IconEdit::updatePixmap()
{
    m_iconButton->setEnabled(true);
    m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    m_iconButton->setIcon(QIcon(m_waitingPixmap));
    m_iconButton->setIconSize(QSize(16, 16));
}

void IconEdit::clicked_cb()
{
    if (m_edit && !m_edit->isReadOnly() && isVisible()) {
        startWaiting();
        emit clicked(m_edit->text());
    }
}

void IconEdit::onCapsChanged(bool state)
{
    if (QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        m_capsState = checkCapsLockState();
    } else {
        m_capsState = state;
    }
    m_capsIcon->setVisible(m_capsState);
    updateEditTextMargins();
    int marginsWidth = 0;
    if (m_editType == CONFIDENTIALEDIT || m_editType == KYLINIDEDIT) {
        marginsWidth = 0;
    } else if (m_editType == RESETPWDEDIT) {
        marginsWidth = m_modeButton->width();
    } else {
        marginsWidth = m_iconButton->width() + m_modeButton->width();
    }
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        m_edit->setTextMargins(m_capsState ? marginsWidth + m_capsIcon->width() + 10 : marginsWidth + 6, 1, 1, 1);
    } else {
        m_edit->setTextMargins(1, 1, m_capsState ? marginsWidth + m_capsIcon->width() + 10 : marginsWidth + 6, 1);
    }
}

void IconEdit::measurePlaceholderText()
{
    if (m_edit && m_isShowEditToolTip) {
        QString strPlaceholderText = m_edit->placeholderText();
        if (!strPlaceholderText.isEmpty() && m_edit->text().isEmpty()) {
            int nAlignWidth = 16;
            if (m_editType == NORMALPWDEDIT) { // qss影响
                nAlignWidth = 15;
            }
            int nTextWidth
                = m_edit->width() - m_edit->textMargins().left() - m_edit->textMargins().right() - nAlignWidth;
            QFontMetrics fontMetrics(m_edit->font());
            // 如果当前字体下，字符串长度大于指定宽度
            int nActualWidth = fontMetrics.width(strPlaceholderText);
            // qDebug() << "edit text width:" << m_edit->width() << nTextWidth << nActualWidth << strPlaceholderText;
            if (nActualWidth > nTextWidth) {
                setKToolTip(strPlaceholderText);
            } else {
                hideKToolTip();
            }
        } else {
            hideKToolTip();
        }
    }
}

void IconEdit::setIconButtonSize(const QSize &size)
{
    if (!m_iconButton || m_iconButton->size() == size) {
        return;
    }
    m_iconButton->setFixedSize(size);
    updateEditTextMargins();
}
void IconEdit::updateEditTextMargins()
{
    if (!m_edit) {
        return;
    }
    int marginsWidth = 0;
    if (m_editType == CONFIDENTIALEDIT || m_editType == KYLINIDEDIT) {
        marginsWidth = 0;
    } else if (m_editType == RESETPWDEDIT) {
        marginsWidth = m_modeButton->width();
    } else {
        marginsWidth = m_iconButton->width() + m_modeButton->width();
    }
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        m_edit->setAlignment(Qt::AlignRight);
        m_edit->setTextMargins(m_capsState ? marginsWidth + m_capsIcon->width() + 10 : marginsWidth + 6, 1, 1, 1);
    } else {
        m_edit->setAlignment(Qt::AlignLeft);
        m_edit->setTextMargins(1, 1, m_capsState ? marginsWidth + m_capsIcon->width() + 10 : marginsWidth + 6, 1);
    }
}

void IconEdit::setKToolTip(const QString &text)
{
    if (!text.isEmpty()) {
        QPoint point = QCursor::pos();
        QToolTip::showText(point, text);
    }
}

void IconEdit::hideKToolTip()
{
    if (QToolTip::isVisible()) {
        QToolTip::hideText();
    }
}
