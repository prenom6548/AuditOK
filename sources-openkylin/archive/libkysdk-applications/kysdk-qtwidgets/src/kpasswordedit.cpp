/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kpasswordedit.h"
#include "ktoolbutton.h"
#include "parmscontroller.h"
#include "themeController.h"
#include <QAction>
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPalette>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include "accessinfohelper.h"

namespace kdk
{

class KPasswordEditPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KPasswordEdit)
public:
    KPasswordEditPrivate(KPasswordEdit *parent);
    void adjustLayout();
    void repaintIcon();

protected:
    void changeTheme();

private:
    KPasswordEdit *q_ptr;
    KToolButton *m_pEchoModeBtn;
    LoginState m_state;
    KToolButton *m_pLoadingBtn;
    KToolButton *m_pClearBtn;
    QWidget *m_pWidget;
    QHBoxLayout *m_pWidgetLayout;
    QTimer *m_pTimer;
    bool m_isLoading;
    bool m_hasFocus;
    bool m_useCustomPalette;
    int m_flashState;
};

KPasswordEdit::KPasswordEdit(QWidget *parent)
    : QLineEdit(parent)
    , d_ptr(new KPasswordEditPrivate(this))
{
    Q_D(KPasswordEdit);
    connect(d->m_pEchoModeBtn, &QToolButton::clicked, this, [=]() {
        if (echoMode() == QLineEdit::Password)
            setEchoMode(QLineEdit::Normal);
        else
            setEchoMode(QLineEdit::Password);
        d->repaintIcon();
    });
    connect(this, &QLineEdit::textChanged, this, [=]() {
        if (!text().isEmpty() && d->m_pClearBtn->isEnabled() && hasFocus()) {
            d->m_pClearBtn->show();
            d->adjustLayout();
        } else {
            d->m_pClearBtn->hide();
            d->adjustLayout();
        }
    });
    connect(d->m_pClearBtn, &QPushButton::clicked, this, [=]() {
        this->clear();
    });
    connect(d->m_pTimer, &QTimer::timeout, this, [=]() {
        if (d->m_flashState < 7)
            d->m_flashState++;
        else
            d->m_flashState = 0;
        d->m_pLoadingBtn->setIcon(QIcon::fromTheme(QString("ukui-loading-%1.symbolic").arg(d->m_flashState)));
    });
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
       if(key == "styleName" || key == "iconThemeName"||key == "widgetThemeName")
       {
            d->changeTheme();
       }
    });
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=]() {
        updateGeometry();
    });
    d->repaintIcon();
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, false);
    d->changeTheme();
}

void KPasswordEdit::setState(LoginState state)
{
    Q_D(KPasswordEdit);
    d->m_state = state;

    auto palette = this->palette();
    switch (d->m_state) {
    case Ordinary:
        palette.setBrush(QPalette::Highlight, ThemeController::getCustomColorFromDT("kbrand-normal"));
        setPalette(palette);
        break;
    case LoginSuccess:
        palette.setBrush(QPalette::Highlight, ThemeController::getCustomColorFromDT("ksuccess-normal"));
        setPalette(palette);
        break;
    case LoginFailed:
        palette.setBrush(QPalette::Highlight, ThemeController::getCustomColorFromDT("kerror-normal"));
        setPalette(palette);
        break;
    default:
        break;
    }
}

LoginState KPasswordEdit::state()
{
    Q_D(KPasswordEdit);
    return d->m_state;
}

void KPasswordEdit::setLoading(bool flag)
{
    Q_D(KPasswordEdit);
    d->m_isLoading = flag;
    if (flag) {
        d->m_pLoadingBtn->show();
        d->m_pTimer->start();
        QLineEdit::setEnabled(false);
    } else {
        d->m_pLoadingBtn->hide();
        d->m_pTimer->stop();
        QLineEdit::setEnabled(true);
    }
    d->adjustLayout();
}

bool KPasswordEdit::isLoading()
{
    Q_D(KPasswordEdit);
    return d->m_isLoading;
}

QString KPasswordEdit::placeholderText()
{
    return QLineEdit::placeholderText();
}

void KPasswordEdit::setPlaceholderText(QString &text)
{
    QLineEdit::setPlaceholderText(text);
}

void KPasswordEdit::setClearButtonEnabled(bool enable)
{
    Q_D(KPasswordEdit);
    d->m_pClearBtn->setEnabled(enable);
}

bool KPasswordEdit::isClearButtonEnabled() const
{
    bool result = d_ptr->m_pClearBtn->isEnabled();
    return result;
}

void KPasswordEdit::setEchoModeBtnVisible(bool enable)
{
    Q_D(KPasswordEdit);
    d->m_pEchoModeBtn->setVisible(enable);
    d->adjustLayout();
}

bool KPasswordEdit::echoModeBtnVisible()
{
    Q_D(KPasswordEdit);
    return d->m_pEchoModeBtn->isVisible();
}

void KPasswordEdit::setClearBtnVisible(bool enable)
{
    Q_D(KPasswordEdit);
    d->m_pClearBtn->setEnabled(enable);
    d->adjustLayout();
}

bool KPasswordEdit::clearBtnVisible()
{
    Q_D(KPasswordEdit);
    return d->m_pClearBtn->isVisible();
}

void KPasswordEdit::setEnabled(bool flag)
{
    Q_D(KPasswordEdit);
    if (!flag) {
        d->m_pClearBtn->hide();
        d->m_pLoadingBtn->hide();
    }
    QLineEdit::setEnabled(flag);
    d->repaintIcon();
}

void KPasswordEdit::setEchoMode(QLineEdit::EchoMode mode)
{
    Q_D(KPasswordEdit);
    QLineEdit::setEchoMode(mode);
    d->repaintIcon();
}

void KPasswordEdit::setUseCustomPalette(bool flag)
{
    Q_D(KPasswordEdit);
    d->m_useCustomPalette = flag;
}

void KPasswordEdit::resizeEvent(QResizeEvent *event)
{
    Q_D(KPasswordEdit);

    QLineEdit::resizeEvent(event);
    d->adjustLayout();
}

bool KPasswordEdit::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KPasswordEdit);
    if (watched == this) {
        if (event->type() == QEvent::FocusIn) {
            d->m_hasFocus = true;
            if (this->text().isEmpty())
                d->m_pClearBtn->hide();
            else {
                if (d->m_pClearBtn->isEnabled())
                    d->m_pClearBtn->show();
            }
            d->adjustLayout();
            d->changeTheme();
        }
        if (event->type() == QEvent::FocusOut) {
            d->m_hasFocus = false;
            d->m_pClearBtn->hide();
            d->adjustLayout();
            d->changeTheme();
        }
    }
    return QLineEdit::eventFilter(watched, event);
}

QSize KPasswordEdit::sizeHint() const
{
    auto size = QLineEdit::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PasswordEditHeight));
    return size;
}

KPasswordEditPrivate::KPasswordEditPrivate(KPasswordEdit *parent)
    : q_ptr(parent)
    , m_useCustomPalette(false)
{
    Q_Q(KPasswordEdit);
    m_flashState = 0;
    m_state = Ordinary;
    m_hasFocus = q->hasFocus();
    m_isLoading = false;
    m_pTimer = new QTimer(this);
    m_pTimer->setInterval(100);
    q->QLineEdit::setEchoMode(QLineEdit::Password);

    QPalette btnPalette;
    btnPalette.setBrush(QPalette::Active, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Button, Qt::transparent);

    btnPalette.setBrush(QPalette::Active, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Highlight, Qt::transparent);

    m_pEchoModeBtn = new KToolButton(q);
    m_pEchoModeBtn->setAutoFillBackground(true);
    m_pEchoModeBtn->setPalette(btnPalette);
    m_pEchoModeBtn->setType(KToolButtonType::Background);
    m_pEchoModeBtn->setAttribute(Qt::WA_TranslucentBackground,true);
    m_pEchoModeBtn->setIconSize(QSize(16, 16));
    m_pEchoModeBtn->setFixedSize(QSize(16, 16));
    m_pEchoModeBtn->setFocusPolicy(Qt::NoFocus);
    m_pEchoModeBtn->setCursor(Qt::ArrowCursor);
    m_pEchoModeBtn->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
    m_pLoadingBtn = new KToolButton(q);
    m_pLoadingBtn->setAutoFillBackground(true);
    m_pLoadingBtn->setPalette(btnPalette);
    m_pLoadingBtn->setType(KToolButtonType::Background);
    m_pLoadingBtn->setAttribute(Qt::WA_TranslucentBackground,true);
    m_pLoadingBtn->setIconSize(QSize(16, 16));
    m_pLoadingBtn->setFixedSize(QSize(16, 16));
    m_pLoadingBtn->setFocusPolicy(Qt::NoFocus);
    m_pLoadingBtn->setCursor(Qt::ArrowCursor);
    m_pLoadingBtn->setIcon(QIcon::fromTheme("ukui-loading-0"));
    m_pLoadingBtn->hide();
    m_pClearBtn = new KToolButton(q);
    m_pClearBtn->setAutoFillBackground(true);
    m_pClearBtn->setPalette(btnPalette);
    m_pClearBtn->setType(KToolButtonType::Background);
    m_pClearBtn->setAttribute(Qt::WA_TranslucentBackground,true);
    m_pClearBtn->setIconSize(QSize(16, 16));
    m_pClearBtn->setFixedSize(QSize(16, 16));
    m_pClearBtn->setFocusPolicy(Qt::NoFocus);
    m_pClearBtn->setCursor(Qt::ArrowCursor);
    m_pClearBtn->setIcon(QIcon::fromTheme("application-exit-symbolic"));
    m_pClearBtn->setVisible(false);
    m_pWidget = new QWidget(q);
    m_pWidgetLayout = new QHBoxLayout(m_pWidget);
    m_pWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pWidgetLayout->setSpacing(5);
    if (ThemeController::systemLang())
        m_pWidget->setLayoutDirection(Qt::RightToLeft);
    else
        m_pWidget->setLayoutDirection(Qt::LeftToRight);
    m_pWidgetLayout->addWidget(m_pLoadingBtn);
    m_pWidgetLayout->addWidget(m_pClearBtn);
    m_pWidgetLayout->addWidget(m_pEchoModeBtn);
    m_pWidgetLayout->addSpacing(5);
    m_pWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setParent(parent);

    KDK_ALL_INFO_FORMAT(m_pEchoModeBtn,"");
    KDK_ALL_INFO_FORMAT(m_pLoadingBtn,"");
    KDK_ALL_INFO_FORMAT(m_pClearBtn,"");
    KDK_ALL_INFO_FORMAT(m_pWidget,"");
}

void KPasswordEditPrivate::adjustLayout()
{
    Q_Q(KPasswordEdit);
    int spacing = 5;
    int width = spacing;
    if (!m_pEchoModeBtn->isHidden())
        width += (spacing + m_pEchoModeBtn->iconSize().width());
    if (!m_pClearBtn->isHidden())
        width += (spacing + m_pClearBtn->iconSize().width());
    if (!m_pLoadingBtn->isHidden())
        width += (spacing + m_pLoadingBtn->iconSize().width());
    m_pWidget->setFixedSize(width, q->height());
    if (ThemeController::systemLang()) {
        m_pWidget->move(spacing, 0);
        q->setTextMargins(m_pWidget->width(), 0, 0, 0);
    } else {
        m_pWidget->move(q->width() - m_pWidget->width(), 0);
        q->setTextMargins(0, 0, m_pWidget->width(), 0);
    }
}

void KPasswordEditPrivate::repaintIcon()
{
    Q_Q(KPasswordEdit);
    if (ThemeController::themeMode() == LightTheme) {
        m_pClearBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("application-exit-symbolic").pixmap(16, 16), ThemeController::getPaletteColorFromDT("buttontext-active")));
        if (q->echoMode() == QLineEdit::Password)
            m_pEchoModeBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("ukui-eye-hidden-symbolic").pixmap(16, 16), ThemeController::getPaletteColorFromDT("buttontext-active")));
        else
            m_pEchoModeBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("ukui-eye-display-symbolic").pixmap(16, 16), ThemeController::getPaletteColorFromDT("buttontext-active")));
    } else {
        m_pClearBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("application-exit-symbolic").pixmap(16, 16), ThemeController::getPaletteColorFromDT("buttontext-active")));
        if (q->echoMode() == QLineEdit::Password) {
            if (q->isEnabled())
                m_pEchoModeBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("ukui-eye-hidden-symbolic").pixmap(16, 16), ThemeController::getPaletteColorFromDT("buttontext-active")));
            else
                m_pEchoModeBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("ukui-eye-hidden-symbolic").pixmap(16, 16), ThemeController::getCustomColorFromDT("kgray-17")));
        } else {
            if (q->isEnabled())
                m_pEchoModeBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("ukui-eye-display-symbolic").pixmap(16, 16), ThemeController::getPaletteColorFromDT("buttontext-active")));
            else
                m_pEchoModeBtn->setIcon(drawColoredPixmap(QIcon::fromTheme("ukui-eye-display-symbolic").pixmap(16, 16), ThemeController::getCustomColorFromDT("kgray-17")));
        }
    }
}

void KPasswordEditPrivate::changeTheme()
{
    Q_Q(KPasswordEdit);
    initThemeStyle();
    if (m_useCustomPalette)
        return;
    repaintIcon();
    QPalette palette = q->palette();
    if (q->hasFocus()) {
        QColor color = ThemeController::getPaletteColorFromDT("base-active");
        palette.setBrush(QPalette::Button, color);
        if (ThemeController::themeMode() == LightTheme)
            palette.setBrush(QPalette::Text, ThemeController::getCustomColorFromDT("kgray-17"));
        else
            palette.setBrush(QPalette::Active, QPalette::Text, ThemeController::getCustomColorFromDT("kwhite"));
        q->setPalette(palette);
    } else {
        if (ThemeController::themeMode() == LightTheme) {
            palette.setBrush(QPalette::Active, QPalette::Button, ThemeController::getCustomColorFromDT("kgray-6"));
            palette.setBrush(QPalette::Inactive, QPalette::Button, ThemeController::getCustomColorFromDT("kgray-6"));
            palette.setBrush(QPalette::Disabled, QPalette::Button, ThemeController::getCustomColorFromDT("kgray-5"));
            palette.setBrush(QPalette::Active, QPalette::Text, QColor(140, 140, 140));
            palette.setBrush(QPalette::Inactive, QPalette::Text, QColor(140, 140, 140));
            palette.setBrush(QPalette::Disabled, QPalette::Text, ThemeController::getCustomColorFromDT("kgray-11"));
            q->setPalette(palette);
        } else {
            palette.setBrush(QPalette::Active, QPalette::Button, ThemeController::getCustomColorFromDT("kgray-16"));
            palette.setBrush(QPalette::Inactive, QPalette::Button, ThemeController::getCustomColorFromDT("kgray-16"));
            palette.setBrush(QPalette::Disabled, QPalette::Button, ThemeController::getCustomColorFromDT("kgray-17"));
            palette.setBrush(QPalette::Active, QPalette::Text, ThemeController::getCustomColorFromDT("kgray-13"));
            palette.setBrush(QPalette::Inactive, QPalette::Text, ThemeController::getCustomColorFromDT("kgray-13"));
            palette.setBrush(QPalette::Disabled, QPalette::Text, ThemeController::getCustomColorFromDT("kgray-15"));
            q->setPalette(palette);
        }
    }
}
}

#include "kpasswordedit.moc"
#include "moc_kpasswordedit.cpp"
