/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: Jing Ding <dingjing@kylinos.cn>
 *
 */
#include "file-operation-error-dialog-base.h"
#include "xatom-helper.h"

#include <QIcon>
#include <QPainter>
#include <QPainter>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QProxyStyle>
#include <QPushButton>
#include <QApplication>
#include <QGSettings>

#ifdef KY_SDK_WAYLANDHELPER
#include <ukuistylehelper/ukuistylehelper.h>
#endif
#include <QX11Info>
#include "xatom-helper.h"

Peony::FileOperationErrorDialogBase::FileOperationErrorDialogBase(QDialog *parent) : QDialog(parent)
{
    setFixedSize (536, 192);
    setAutoFillBackground (true);
    setBackgroundRole (QPalette::Base);

#ifdef KY_SDK_WAYLANDHELPER
    m_wmHelper = new UkuiWindowHelper(this);

#endif
    if (QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    } else {
#ifdef KY_SDK_WAYLANDHELPER
        m_wmHelper->removeTitleBar();
        m_wmHelper->setSkipSwitcher(false);
        m_wmHelper->setSkipTaskBar(true);
        UkuiWindowHelper::States defaultState = UkuiWindowHelper::States(0xff);
        m_wmHelper->setWindowState(defaultState | UkuiWindowHelper::State::KeepAbove);
#endif
    }

    QVBoxLayout* mainLayout = new QVBoxLayout (this);
    mainLayout->setContentsMargins (16, 5, 5, 16);

    QHBoxLayout* headerLayout = new QHBoxLayout;

//    QPushButton* minilize = new QPushButton;
    QPushButton* closebtn = new QPushButton;

//    minilize->setFlat (true);
//    minilize->setFixedSize (36, 36);
//    minilize->setProperty ("isWindowButton", 0x01);
//    minilize->setIconSize (QSize(16, 16));
//    minilize->setIcon (QIcon::fromTheme ("window-minimize-symbolic"));

    closebtn->setFlat (true);
    closebtn->setFixedSize (32, 32);
    closebtn->setProperty ("isWindowButton", 0x02);
    closebtn->setIconSize (QSize(16, 16));
    closebtn->setIcon (QIcon::fromTheme("window-close-symbolic"));
    closebtn->setToolTip(tr("Close"));

    headerLayout->addStretch ();
//    headerLayout->setSpacing (1);
//    headerLayout->addWidget (minilize);
    headerLayout->addWidget (closebtn);

    mainLayout->addLayout (headerLayout);

    QHBoxLayout* contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins (6, 0, 3, 0);
    contentLayout->setAlignment (Qt::AlignTop | Qt::AlignLeft);

    m_tipimage = new QLabel(this);
    m_tipimage->setMargin (0);
    m_tipimage->setMinimumWidth(24);
    m_tipimage->setAlignment (Qt::AlignTop | Qt::AlignHCenter);
    m_tipimage->setPixmap (QIcon::fromTheme ("dialog-warning").pixmap (16, 16));

    m_iconName = "dialog-warning";
    m_imageHeight = 16;

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *settings = new QGSettings("org.ukui.style", QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if("iconThemeName" == key){
                m_tipimage->setPixmap(QIcon::fromTheme(m_iconName).pixmap(m_tipimage->size()));
            }
        });
    }

    contentLayout->addWidget (m_tipimage);

    m_tipcontent = new QLabel(this);
    m_tipcontent->setWordWrap (true);
    m_tipcontent->setMinimumWidth(420);
    m_tipcontent->setMaximumWidth(460);
    m_tipcontent->setAlignment (Qt::AlignLeft | Qt::AlignTop);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setMaximumWidth(480);
    scroll->setWidgetResizable (true);
    scroll->setFrameShape(QFrame::NoFrame);

    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scroll->setWidget(m_tipcontent);

    contentLayout->addWidget (scroll);

    mainLayout->addLayout (contentLayout);

    m_buttonLeft = new QHBoxLayout;
    m_buttonRight = new QHBoxLayout;
    m_buttonRight->setDirection (QHBoxLayout::RightToLeft);
    m_buttonRight->setSpacing(16); /* 按设计稿设置button的space,link to bug#139766 */

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins (0, 0, 16, 3);
    m_buttonLeft->addStretch();

    buttonLayout->addLayout (m_buttonLeft);
    buttonLayout->addStretch ();
    buttonLayout->addLayout (m_buttonRight);

    mainLayout->addStretch ();
    mainLayout->addLayout (buttonLayout);

    setLayout (mainLayout);

    connect (closebtn, &QPushButton::clicked, this, [=] () {
        Q_EMIT cancel ();
        done (QDialog::Rejected);
    });

    connect(this, &FileOperationErrorDialogBase::cancel, this, [=] () {
        done (QDialog::Rejected);
    });
}

Peony::FileOperationErrorDialogBase::~FileOperationErrorDialogBase()
{

}

void Peony::FileOperationErrorDialogBase::adjustTextContent()
{
    auto text = m_tipcontent->text();
    if (text.startsWith("<p>") && text.endsWith("</p>")) {
        text.remove(text.length() - 4, 4);
        text.remove(0, 3);
    }

    // 文本顶端显示，不设置上边距
    m_tipcontent->setContentsMargins(0, 0, 0, 0);

    // 获取首行文本的高度（单行文本高度），图标只与首行文本垂直居中对齐
    QFontMetrics fm = fontMetrics();
    int firstLineHeight = fm.height();

    //在wayland高缩放下存在获取异常问题，需要应用记录
    int imageHeight = m_imageHeight;
    // 如果首行文本高度为0，不进行对齐计算
    if (firstLineHeight <= 0) {
        m_tipimage->setContentsMargins(0, 0, 0, 0);
        return;
    }

    // 计算图标的上边距，使图标的中心点与首行文本的中心点对齐
    // 首行文本中心点位置：firstLineHeight / 2（因为文本上边距为0）
    // 图标中心点应该对齐到：firstLineHeight / 2
    // 所以图标上边距应该是：firstLineHeight / 2 - imageHeight / 2
    int imageMargin = (firstLineHeight - imageHeight) / 2;
    //根据字体修正视觉显示效果
    if (firstLineHeight > 24) {
        imageMargin = imageMargin + 2;
    } else {
        imageMargin = imageMargin + 1;
    }

    m_tipimage->setContentsMargins(0, qMax(imageMargin, 0), 0, 0);
}

void Peony::FileOperationErrorDialogBase::setText(QString text)
{
    if (!text.isNull () && !text.isEmpty ()) {
        m_tipcontent->setText (text);
        adjustTextContent();
    }
}

void Peony::FileOperationErrorDialogBase::setIcon(QString iconName)
{
    m_iconName = iconName;
    if (!iconName.isNull () && !iconName.isEmpty ()) {
        m_imageHeight = iconName.contains("dialog-warning") ? 16 : 64;
        if (iconName.contains("dialog-question") ) {
            m_imageHeight = 16;
        }
        m_tipimage->setPixmap (QIcon::fromTheme (iconName).pixmap (m_imageHeight, m_imageHeight));
    }
    int pimageMargin = qMax(m_tipcontent->contentsMargins().top() - (m_tipimage->pixmap()->height()-fontMetrics().height())/2,  0);
    m_tipimage->setContentsMargins(0, pimageMargin, 0, 0);
}

QPushButton *Peony::FileOperationErrorDialogBase::addButton(QString name)
{
    if (!name.isNull () && !name.isEmpty ()) {
        QPushButton* b = new QPushButton(name);
        b->setContentsMargins(0, 0, 0, 0);
        int buttonSize = qMax(96, b->sizeHint().width());
        b->setFixedWidth(buttonSize);

        connect(this, &FileOperationErrorDialogBase::fontChanged, b, [=]{
            int buttonSize = qMax(96, b->sizeHint().width());
            b->setFixedWidth(buttonSize);
        });

        m_buttonRight->addWidget (b, Qt::AlignRight | Qt::AlignVCenter);
        return b;
    }

    return nullptr;
}

QCheckBox *Peony::FileOperationErrorDialogBase::addCheckBoxLeft(QString name)
{
    if (!name.isNull () && !name.isEmpty ()) {
        QCheckBox* b = new QCheckBox(name);
        m_buttonLeft->addWidget (b, Qt::AlignLeft | Qt::AlignVCenter);
        return b;
    }

    return nullptr;
}

bool Peony::FileOperationErrorDialogBase::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange
        || event->type() == QEvent::ApplicationFontChange
#ifdef HAVE_DEVICE_PIXEL_RATIO_CHANGE
        || event->type() == QEvent::DevicePixelRatioChange
#endif
        ) {
        adjustTextContent();
        Q_EMIT this->fontChanged();
    }
    return QDialog::event(event);
}
