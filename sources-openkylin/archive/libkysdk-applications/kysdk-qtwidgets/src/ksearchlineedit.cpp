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

#include "ksearchlineedit.h"
#include "klineframe.h"
#include "kshadowhelper.h"
#include "ktoolbutton.h"
#include "parmscontroller.h"
#include "themeController.h"
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCompleter>
#include <QCursor>
#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QLinearGradient>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QProxyStyle>
#include <QPushButton>
#include <QStringListModel>
#include <QStyle>
#include <QStyleOption>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QToolTip>
#include <QScrollBar>
#include "accessinfohelper.h"

namespace kdk
{

class MyStyle : public QProxyStyle
{
public:
    MyStyle()
    {
    }
    ~MyStyle()
    {
    }

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override;

    QRect subElementRect(QStyle::SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget) const override;
};

class CursorProxyStyle : public QProxyStyle
{
public:
    CursorProxyStyle()
    {
    }
    ~CursorProxyStyle()
    {
    }

    virtual int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const
    {
        if (metric == QStyle::PM_TextCursorWidth) {
            return 0;
        }
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

class ListViewDelegate : public QStyledItemDelegate, public ThemeController
{
public:
    ListViewDelegate(QObject *parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view,
                      const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

static const int iconMargin = 6;
static const int buttonMargin = 6;
static const int icontextSpacing = 5;

class KSearchLineEditPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSearchLineEdit)

public:
    KSearchLineEditPrivate(KSearchLineEdit *parent);
    void adjustHolderRect();
    void adjustLayout();

public Q_SLOTS:
    void doAnimationFinished();
    void doEditingFinished();

protected:
    void changeTheme();

private:
    KSearchLineEdit *q_ptr;
    QHBoxLayout *m_pWidgetLayout;
    QWidget *m_pWidget;
    QLabel *m_pTextLabel;
    QLabel *m_pIconLabel;
    QPropertyAnimation *m_pAnimation;
    bool m_isSearching;
    QStringListModel *m_pListModel;
    QStringList m_List;
    QCompleter *m_pCompleter;

    QString m_placeHolderText;
    Qt::Alignment m_placeHolderAlignment;
    bool m_hasFocus;
    QRect m_holderRect;
    QRect m_sideRect;
    ListViewDelegate *m_pListViewDelegate;
    bool m_isTranslucent;
    bool m_isTranslucentFlag;
    bool isTextEmpty;
    KToolButton *m_pClearButton;
    KToolButton *m_pCustomButton;
    KVLineFrame *m_pLineFrame;
    QWidget *m_pSubWidget;
    QLinearGradient m_linearGradient;
    bool m_isClearVisible;
    bool m_isGradient;
    bool m_isCustomButtonHover;
    bool m_isClearHover;
    bool m_showCursor;
    bool m_isGradientFlag;
    QTimer *m_timer;
    void sltUpdateCursorRect();
    void init();
    QString m_systemName;
    bool m_contextMenu;
};

KSearchLineEdit::KSearchLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , d_ptr(new KSearchLineEditPrivate(this))
{
    Q_D(KSearchLineEdit);

    setFocusPolicy(Qt::ClickFocus);
    // 应设计师要求，1.2.0.8之后的版本默认显示右键菜单
    // setContextMenuPolicy(Qt::NoContextMenu);
    d->init();
    installEventFilter(this);
    d->m_pCustomButton->installEventFilter(this);

    connect(d->m_pAnimation, &QPropertyAnimation::finished, d, &KSearchLineEditPrivate::doAnimationFinished);
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName" || key == "systemFontSize")
        {
            d->changeTheme();
        }
    });
    connect(this, &KSearchLineEdit::editingFinished, d, &KSearchLineEditPrivate::doEditingFinished);
    connect(this, &KSearchLineEdit::returnPressed, this, &KSearchLineEdit::clearFocus);
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=]() {
        updateGeometry();
    });

    connect(this, &QLineEdit::textChanged, this, [=]() {
        if (!text().isEmpty() && d->m_pClearButton->isEnabled() && hasFocus() && d->m_isClearVisible) {
            d->m_pClearButton->show();
            d->adjustLayout();
        } else {
            d->m_pClearButton->hide();
            d->adjustLayout();
        }
        if(!text().isEmpty())
        {
            d->m_pTextLabel->setVisible(false);
        }
        else
        {
            d->m_pTextLabel->setVisible(true);
        }
        d->adjustHolderRect();
        d->adjustLayout();
        d->doAnimationFinished();
    });
    connect(d->m_pCustomButton, &KToolButton::clicked, this, [=] {
        d->m_isGradient = !d->m_isGradient;
        if (d->m_isGradientFlag && d->m_isGradient) {
            d->m_pCustomButton->setIconColor(true, ThemeController::getPaletteColorFromDT("highlight-active"));
        }
        update();
    });
    connect(d->m_pClearButton, &QPushButton::clicked, this, [=]() {
        this->clear();
    });
    connect(d->m_pAnimation, &QPropertyAnimation::finished, this, [=]() {
        d->sltUpdateCursorRect();
    });

    d->changeTheme();

}

KSearchLineEdit::~KSearchLineEdit()
{
    Q_D(KSearchLineEdit);
}

void KSearchLineEdit::setEnabled(bool enable)
{
    Q_D(KSearchLineEdit);
    QLineEdit::setEnabled(enable);
    d->changeTheme();
}

bool KSearchLineEdit::isEnabled()
{
    Q_D(KSearchLineEdit);
    return QLineEdit::isEnabled();
}

void KSearchLineEdit::setClearButtonEnabled(bool enable)
{
    Q_D(KSearchLineEdit);
    d->m_isClearVisible = enable;
    QLineEdit::setClearButtonEnabled(false);
}

bool KSearchLineEdit::isClearButtonEnabled() const
{
    Q_D(const KSearchLineEdit);
    return d->m_pClearButton->isHidden();
    //    QLineEdit::isClearButtonEnabled();
}

QString KSearchLineEdit::placeholderText() const
{
    Q_D(const KSearchLineEdit);
    return d->m_placeHolderText;
}

void KSearchLineEdit::setPlaceholderText(const QString &text)
{
    Q_D(KSearchLineEdit);
    d->m_placeHolderText = text;
    d->m_pTextLabel->setText(text);
}

Qt::Alignment KSearchLineEdit::placeholderAlignment() const
{
    Q_D(const KSearchLineEdit);
    return d->m_placeHolderAlignment;
}

void KSearchLineEdit::setPlaceholderAlignment(Qt::Alignment flag)
{
    Q_D(KSearchLineEdit);
    d->m_placeHolderAlignment = flag;
}

Qt::Alignment KSearchLineEdit::alignment() const
{
    return QLineEdit::alignment();
}

void KSearchLineEdit::setAlignment(Qt::Alignment flag)
{
    Q_D(KSearchLineEdit);
    QLineEdit::setAlignment(flag);
}

void KSearchLineEdit::setTranslucent(bool flag)
{
    Q_D(KSearchLineEdit);
    d->m_isTranslucentFlag = flag;
    d->m_isTranslucent = flag;
    d->changeTheme();
    setProperty("needTranslucent", flag);
}

bool KSearchLineEdit::isTranslucent()
{
    Q_D(KSearchLineEdit);
    return d->m_isTranslucent;
}

void KSearchLineEdit::reloadStyle()
{
    Q_D(KSearchLineEdit);
    QAbstractItemView *popup = completer()->popup();
    kdk::effects::KShadowHelper::self()->setWidget(popup);

    popup->setItemDelegate(d->m_pListViewDelegate);
    MyStyle *style = new MyStyle;
    popup->setStyle(style);
}

KToolButton *KSearchLineEdit::customButton()
{
    Q_D(KSearchLineEdit);
    return d->m_pCustomButton;
}

void KSearchLineEdit::setGradation(bool flag)
{
    Q_D(KSearchLineEdit);
    d->m_isGradientFlag = flag;
}

void KSearchLineEdit::setCustomButtonVisible(bool flag)
{
    Q_D(KSearchLineEdit);
    d->m_pCustomButton->setVisible(flag);
    d->m_pLineFrame->setVisible(flag);
    d->adjustLayout();
}

bool KSearchLineEdit::isCustomButtonVisible()
{
    Q_D(KSearchLineEdit);
    return d->m_pCustomButton->isVisible();
}

void KSearchLineEdit::clear()
{
    Q_D(KSearchLineEdit);

    if (this->text().isEmpty())
        return;
    QLineEdit::clear();
    d->isTextEmpty = true;
    if (this->hasFocus()) {
        d->m_pTextLabel->adjustSize();
        d->m_pWidget->update();
    } else {
        d->m_pTextLabel->adjustSize();
        d->m_pAnimation->setStartValue(d->m_sideRect);
        d->m_pAnimation->setEndValue(d->m_holderRect);
        d->m_pAnimation->start();
    }
    d->adjustHolderRect();
    d->adjustLayout();
}

bool KSearchLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KSearchLineEdit);

    QFont font;
    font = QApplication::font();
    QFontMetrics fm(font);

    if (watched == this) {
        switch (event->type()) {
        case QEvent::ContextMenu:
            if (!d->m_contextMenu)
                d->m_contextMenu = true;
            break;
        case QEvent::Show: {
            d->adjustHolderRect();
            if (text().isEmpty())
                d->m_pWidget->setGeometry(d->m_holderRect);
            else
                d->m_pWidget->setGeometry(d->m_sideRect);
            break;
        }
        case QEvent::FocusIn: {
            if (d->m_contextMenu)
                d->m_contextMenu = false;
            d->m_isSearching = true;
            d->m_hasFocus = false;
            if (!this->text().isEmpty()) {
                if (d->isTextEmpty) {
                    d->isTextEmpty = false;
                }
            } else {
                if (d->isTextEmpty) {
                    d->adjustHolderRect();
                    d->m_pAnimation->setStartValue(d->m_holderRect);
                    d->m_pAnimation->setEndValue(d->m_sideRect);
                    d->m_pAnimation->start();
                    d->isTextEmpty = false;
                }
            }
            d->adjustLayout();
            break;
        }
        case QEvent::FocusOut: {
            if (!d->m_contextMenu) {
                d->m_isSearching = false;
                d->m_hasFocus = true;
                if (this->text().isEmpty()) {
                    d->adjustHolderRect();
                    d->m_pAnimation->setStartValue(d->m_sideRect);
                    d->m_pAnimation->setEndValue(d->m_holderRect);
                    d->m_pAnimation->start();
                    d->isTextEmpty = true;
                }
                d->adjustLayout();
            }
            break;
        }
        case QEvent::HoverEnter:
        case QEvent::HoverMove: {
            if (width() - icontextSpacing - d->m_pIconLabel->width() < fm.width(d->m_placeHolderText)) {
                if (d->m_hasFocus)
                    setToolTip(d->m_placeHolderText);
                else
                    setToolTip("");
            }
            break;
        }
        default:
            break;
        }
    }

    if (watched == d->m_pCustomButton) {
        if (!d->m_isGradient) {
            switch (event->type()) {
            case QEvent::HoverEnter:
            case QEvent::HoverMove:
                d->m_pCustomButton->setIconColor(true, ThemeController::getPaletteColorFromDT("windowtext-active"));
                break;
            case QEvent::Leave:
                d->m_pCustomButton->setIconColor(true, ThemeController::getPaletteColorFromDT("mid-active"));
                break;
            default:
                break;
            }
        }
    }

    return QLineEdit::eventFilter(watched, event);
}

void KSearchLineEditPrivate::adjustHolderRect()
{
    Q_Q(KSearchLineEdit);
    m_pWidget->adjustSize();
    auto maxVisualWidth = q->width() - icontextSpacing - m_pIconLabel->width();
    QFont font(QApplication::font());
    QFontMetrics fm(font);
    auto elidedText = fm.elidedText(m_placeHolderText, Qt::ElideRight, maxVisualWidth);
    m_pTextLabel->setText(elidedText);
    m_pWidget->adjustSize();

    if (m_pIconLabel->height() > m_pTextLabel->height())
        m_pWidget->setFixedHeight(m_pIconLabel->height());
    else
        m_pWidget->setFixedHeight(m_pTextLabel->height());

    m_sideRect = m_pWidget->rect();
    if (m_systemName == "ug_CN" || m_systemName == "kk_KZ" || m_systemName == "ky_KG") {
        m_sideRect.moveTop((q->rect().height() - m_pWidget->height()) / 2);
        m_sideRect.moveLeft(q->rect().width() - m_sideRect.width() - iconMargin);
    } else {
        m_sideRect.moveTop((q->rect().height() - m_pWidget->height()) / 2);
        m_sideRect.moveLeft(iconMargin);
    }

    if (m_placeHolderAlignment & Qt::AlignCenter) {
        m_holderRect = m_pWidget->rect();
        m_holderRect.moveCenter(q->rect().center());
    } else if (m_placeHolderAlignment & Qt::AlignLeft) {
        m_holderRect = m_pWidget->rect();
        m_holderRect.moveTop((q->rect().height() - m_pWidget->height()) / 2);
        m_holderRect.moveLeft(iconMargin);
    }

    else if (m_placeHolderAlignment & Qt::AlignRight) {
        m_holderRect = m_pWidget->rect();
        m_holderRect.moveTop((q->height() - m_pWidget->height()) / 2);
        m_holderRect.moveRight(q->width() - iconMargin);
    }
}

void KSearchLineEditPrivate::adjustLayout()
{
    Q_Q(KSearchLineEdit);
    int spacing = 5;
    int width = spacing * 2;
    m_pLineFrame->setFixedHeight(m_pSubWidget->height() - 12);
    if (!m_pClearButton->isHidden())
        width += (spacing + m_pClearButton->iconSize().width());
    if (!m_pLineFrame->isHidden())
        width += (spacing + m_pLineFrame->width());
    if (!m_pCustomButton->isHidden())
        width += (spacing + m_pCustomButton->iconSize().width());
    m_pSubWidget->setFixedSize(width, q->cursorRect().height());
    if (m_systemName == "ug_CN" || m_systemName == "kk_KZ" || m_systemName == "ky_KG") {
        m_pSubWidget->move(0, (q->rect().height() - m_pSubWidget->height()) / 2 + 1);
        q->setTextMargins(m_pSubWidget->width(), 0, m_pWidget->width(), 0);
    } else {
        m_pSubWidget->move(q->width() - m_pSubWidget->width(), (q->rect().height() - m_pSubWidget->height()) / 2 + 1);
        q->setTextMargins(iconMargin + m_pIconLabel->width(), 0, m_pSubWidget->width(), 0);
    }
}

void KSearchLineEdit::resizeEvent(QResizeEvent *event)
{
    Q_D(KSearchLineEdit);
    QLineEdit::resizeEvent(event);
    d->adjustHolderRect();
    if (text().isEmpty() && !hasFocus())
        d->m_pWidget->setGeometry(d->m_holderRect);
    else
        d->m_pWidget->setGeometry(d->m_sideRect);
    d->adjustLayout();
}

void KSearchLineEdit::setVisible(bool visible)
{
    Q_D(KSearchLineEdit);

    QLineEdit::setVisible(visible);
    if (!text().isEmpty())
    {
        d->adjustHolderRect();
        d->m_pWidget->setGeometry(d->m_sideRect);
    }
}

void KSearchLineEdit::paintEvent(QPaintEvent *event)
{
    Q_D(KSearchLineEdit);
    QLineEdit::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    painter.setClipRect(QRect(), Qt::NoClip);

    d->m_linearGradient = QLinearGradient(QPointF(rect().left(), rect().center().y()), QPointF(rect().right(), rect().center().y()));

//    if ((hasFocus() ||(completer() && completer()->popup() && completer()->popup()->isVisible()) ) && d->m_showCursor)
//    {
//        QRect rect = this->cursorRect();
//        int height = rect.height() - 4;
//        painter.fillRect(QRect(rect.center().x(),rect.center().y() - height/2 - 1 , 1, height ),ThemeController::getPaletteColorFromDT("windowtext-active"));
//    }

    QPen pen = painter.pen();
    pen.setWidth(2);
    if (isEnabled()) {
        d->m_pClearButton->setEnabled(true);
        d->m_pCustomButton->setEnabled(true);
        if (d->m_isGradientFlag && d->m_isGradient) {
            d->m_linearGradient.setColorAt(0, QColor(232, 136, 248));
            d->m_linearGradient.setColorAt(0.4, QColor(138, 65, 255));
            d->m_linearGradient.setColorAt(0.7, QColor(78, 117, 255));
            d->m_linearGradient.setColorAt(1, QColor(0, 54, 245));
            pen.setBrush(d->m_linearGradient);
        } else {
            pen.setBrush(ThemeController::getPaletteColorFromDT("highlight-active"));
        }
    } else {
        pen.setBrush(ThemeController::getPaletteColorFromDT("button-disable"));
        d->m_pClearButton->setEnabled(false);
        d->m_pCustomButton->setEnabled(false);
    }
    if (!hasFocus() && !d->m_isGradient)
        painter.setPen(Qt::NoPen);
    else
        painter.setPen(pen);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;
    painter.drawRoundedRect(rect().adjusted(1, 3, -1, -3), radius, radius);
}

QSize KSearchLineEdit::sizeHint() const
{
    auto size = QLineEdit::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_SearchLineEditHeight));
    return size;
}

KSearchLineEditPrivate::KSearchLineEditPrivate(KSearchLineEdit *parent)
    : q_ptr(parent)
    , m_placeHolderText(tr("Search"))
    , m_placeHolderAlignment(Qt::AlignCenter)
    , m_hasFocus(true)
    , isTextEmpty(true)
    , m_isTranslucent(false)
    , m_isTranslucentFlag(false)
    , m_isClearVisible(false)
    , m_isGradient(false)
    , m_isCustomButtonHover(false)
    , m_isClearHover(false)
    , m_contextMenu(false)
    , m_isGradientFlag(false)
{
    Q_Q(KSearchLineEdit);
    q->setContentsMargins(0,2,0,2);
    m_pCompleter = new QCompleter(this);
    m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_pListModel = new QStringListModel(m_List, this);
    m_pCompleter->setModel(m_pListModel);
    q->setCompleter(m_pCompleter);

    QAbstractItemView *popup = m_pCompleter->popup();
    popup->setAttribute(Qt::WA_TranslucentBackground, true);
    kdk::effects::KShadowHelper::self()->setWidget(popup);
    m_pListViewDelegate = new ListViewDelegate(popup);
    popup->setItemDelegate(m_pListViewDelegate);
    MyStyle *style = new MyStyle;
    popup->setStyle(style);
    popup->setContentsMargins(0,1,0,1);
    setParent(parent);
}

void KSearchLineEditPrivate::doAnimationFinished()
{
    Q_Q(KSearchLineEdit);
    if (m_isSearching) {
        if (m_systemName == "ug_CN" || m_systemName == "kk_KZ" || m_systemName == "ky_KG")
            q->setTextMargins(m_pSubWidget->width(), 0, iconMargin + m_pIconLabel->width(), 0);
        else
            q->setTextMargins(iconMargin + m_pIconLabel->width(), 0, m_pSubWidget->width(), 0);
    }
}

void KSearchLineEditPrivate::doEditingFinished()
{
    Q_Q(KSearchLineEdit);
    QString text = q->text();
    if (QString::compare(text.trimmed(), "") != 0) {
        bool flag = m_List.contains(text, Qt::CaseInsensitive);
        if (!flag) {
            m_List.append(text);
            m_pListModel->setStringList(m_List);
        }
    }
}

void KSearchLineEditPrivate::changeTheme()
{
    Q_Q(KSearchLineEdit);
    if (m_isTranslucentFlag) {
        if (ThemeController::widgetTheme() == ClassicTheme) {
            m_isTranslucent = false;
            setProperty("needTranslucent", false);
        } else {
            m_isTranslucent = true;
            setProperty("needTranslucent", true);
        }
    } else {
        m_isTranslucent = false;
        setProperty("needTranslucent", false);
    }
    adjustHolderRect();
    if (q->text().isEmpty())
        m_pWidget->setGeometry(m_holderRect);
    else
        m_pWidget->setGeometry(m_sideRect);

    QPixmap pixmap = QIcon::fromTheme("search-symbolic").pixmap(QSize(16, 16));
    initThemeStyle();

    if (!q->isEnabled()) {
        QPalette palette = q->palette();
        //        palette.setColor(QPalette::Text,q->palette().color(QPalette::Disabled,QPalette::ButtonText));
        palette.setColor(QPalette::Text, ThemeController::getPaletteColorFromDT("buttontext-disable"));
        m_pTextLabel->setPalette(palette);
    } else {
        QPalette palette = q->palette();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        palette.setColor(QPalette::Text, ThemeController::getPaletteColorFromDT("placeholdertext-active"));
#else if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        palette.setColor(QPalette::Text, ThemeController::getPaletteColorFromDT("shadow-active"));
#endif

        m_pTextLabel->setPalette(palette);
    }

    if (ThemeController::themeMode() == LightTheme) {
        m_pIconLabel->setPixmap(pixmap);
        auto palette = qApp->palette();
        palette.setColor(QPalette::Base, Qt::transparent);
        m_pCompleter->popup()->setPalette(palette);
        m_pCompleter->popup()->setBackgroundRole(QPalette::Base);
        m_pCompleter->popup()->setAutoFillBackground(true);

        QPalette lineFramePalette = qApp->palette();
        lineFramePalette.setColor(QPalette::Base, QColor(207, 207, 207));
        m_pLineFrame->setPalette(lineFramePalette);
    } else {
        m_pIconLabel->setPixmap(drawSymbolicColoredPixmap(pixmap));

        auto palette = qApp->palette();
        palette.setColor(QPalette::Base, Qt::transparent);
        m_pCompleter->popup()->setPalette(palette);
        m_pCompleter->popup()->setBackgroundRole(QPalette::Base);
        m_pCompleter->popup()->setAutoFillBackground(true);

        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Text,ThemeController::getCustomColorFromDT("kfont-white"));
        m_pTextLabel->setPalette(pal);

        QPalette lineFramePalette = qApp->palette();
        lineFramePalette.setColor(QPalette::Base, ThemeController::getCustomColorFromDT("kgray-13"));
        m_pLineFrame->setPalette(lineFramePalette);
    }
    m_pClearButton->setIconColor(true, ThemeController::getPaletteColorFromDT("buttontext-active"));

    q->update();
}

void KSearchLineEditPrivate::sltUpdateCursorRect()
{
    Q_Q(KSearchLineEdit);
    QRect rect = q->cursorRect();
    //fix 428661
    q->update(rect.adjusted(-2, 0, 2, 0));
}

void KSearchLineEditPrivate::init()
{
    Q_Q(KSearchLineEdit);
    QPalette btnPalette;
    btnPalette.setBrush(QPalette::Active, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Button, Qt::transparent);

    btnPalette.setBrush(QPalette::Active, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Highlight, Qt::transparent);

    m_pClearButton = new KToolButton(q);
    m_pClearButton->setType(KToolButtonType::Flat);
    m_pClearButton->setAutoFillBackground(true);
    m_pClearButton->setPalette(btnPalette);
    m_pClearButton->setIconSize(QSize(16, 16));
    m_pClearButton->setFixedSize(QSize(16, 16));
    m_pClearButton->setFocusPolicy(Qt::NoFocus);
    m_pClearButton->setCursor(Qt::ArrowCursor);
    m_pClearButton->setIcon(QIcon::fromTheme("application-exit-symbolic"));
    m_pClearButton->setVisible(false);

    m_pLineFrame = new KVLineFrame();
    m_pLineFrame->setFollowPalette(false);
    m_pLineFrame->setVisible(false);

    m_pCustomButton = new KToolButton(q);
    m_pCustomButton->setCheckable(true);
    m_pCustomButton->setType(KToolButtonType::Flat);
    m_pCustomButton->setAutoFillBackground(true);
    m_pCustomButton->setPalette(btnPalette);
    m_pCustomButton->setIconSize(QSize(16, 16));
    m_pCustomButton->setFixedSize(QSize(16, 16));
    m_pCustomButton->setFocusPolicy(Qt::NoFocus);
    m_pCustomButton->setCursor(Qt::ArrowCursor);
    m_pCustomButton->setVisible(false);
    m_pCustomButton->setIcon(QIcon());

    m_pSubWidget = new QWidget(q);
    QHBoxLayout *layout = new QHBoxLayout(m_pSubWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(iconMargin);
    layout->addWidget(m_pClearButton);
    layout->addWidget(m_pLineFrame);
    layout->addWidget(m_pCustomButton);
    layout->addSpacing(iconMargin);
    m_pSubWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_isSearching = false;
    m_pTextLabel = new QLabel(tr("Search"));
    m_pIconLabel = new QLabel;
    m_pIconLabel->setScaledContents(true);
    QPixmap pixmap = QIcon::fromTheme("search-symbolic").pixmap(QSize(16, 16));
    m_pIconLabel->setPixmap(pixmap);
    m_pIconLabel->setFixedSize(QSize(16, 16));
    m_pWidget = new QWidget(q);
    m_pWidget->setFocusPolicy(Qt::NoFocus);
    m_pWidgetLayout = new QHBoxLayout();
    m_pWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pWidgetLayout->setMargin(0);
    m_pWidgetLayout->setSpacing(0);
    m_pWidgetLayout->addWidget(m_pIconLabel, Qt::AlignVCenter);
    m_pWidgetLayout->addSpacing(icontextSpacing);
    m_pWidgetLayout->addWidget(m_pTextLabel, Qt::AlignVCenter);
    m_pWidgetLayout->addStretch();
    m_pWidgetLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    m_pWidget->setLayout(m_pWidgetLayout);
    m_pAnimation = new QPropertyAnimation(m_pWidget, "geometry");
    m_pAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_pAnimation->setDuration(100);

    m_systemName = QLocale::system().name();
    if (m_systemName == "ug_CN" || m_systemName == "kk_KZ" || m_systemName == "ky_KG")
        q->setAlignment(Qt::AlignRight);
    else
        q->setAlignment(Qt::AlignLeft);

    q->setTextMargins(iconMargin + m_pIconLabel->width(), 0, m_pSubWidget->width(), 0);

    KDK_ALL_INFO_FORMAT(m_pWidget,"");
    KDK_ALL_INFO_FORMAT(m_pTextLabel,"");
    KDK_ALL_INFO_FORMAT(m_pIconLabel,"");
    KDK_OBJ_INFO_FORMAT(m_pAnimation);
    KDK_OBJ_INFO_FORMAT(m_pListModel);
    KDK_OBJ_INFO_FORMAT(m_pCompleter);
    KDK_OBJ_INFO_FORMAT(m_pListViewDelegate);
    KDK_ALL_INFO_FORMAT(m_pClearButton,"");
    KDK_ALL_INFO_FORMAT(m_pCustomButton,"");
    KDK_ALL_INFO_FORMAT(m_pLineFrame,"");
    KDK_ALL_INFO_FORMAT(m_pSubWidget,"");
}

ListViewDelegate::ListViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y() + 1);
    rect.setWidth(option.rect.width());
    rect.setHeight(option.rect.height());

    qreal radius;
    if (ThemeController::widgetTheme() == ClassicTheme) {
        if (painter->pen().width() == 1)
            painter->translate(0.5, 0.5);
        radius = 0;
    } else {
        radius = ThemeController::getRadiusFromDT("kradius-normal");
        if (radius == -1)
            radius = 6;
    }
    QPainterPath path;
    path.moveTo(rect.topRight() - QPointF(radius, 0));
    path.lineTo(rect.topLeft() + QPointF(radius, 0));
    path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
    path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
    path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
    path.lineTo(rect.bottomRight() - QPointF(radius, 0));
    path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
    path.lineTo(rect.topRight() + QPointF(0, radius));
    path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

    painter->setRenderHint(QPainter::Antialiasing);
    QColor color = Qt::transparent;
    QColor fontColor = ThemeController::getPaletteColorFromDT("buttontext-active");

    if (ThemeController::themeMode() == ThemeFlag::LightTheme) {
        if (!(option.state & QStyle::State_Enabled)) {
            color = ThemeController::getCustomColorFromDT("kcomponent-disable");
        } else if (((option.state & QStyle::State_HasFocus) || (option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver))) {
            if (option.state.testFlag(QStyle::State_HasFocus) && option.state.testFlag(QStyle::State_Selected)) // QStyle::State_Enabled
            {

                color = ThemeController::getPaletteColorFromDT("highlight-active");
                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
            } else if ((option.state & QStyle::State_MouseOver)) {
                color = ThemeController::getPaletteColorFromDT("highlight-active");
                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
            } else {
                color = ThemeController::getPaletteColorFromDT("windowtext-active");
            }
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(color);
            painter->drawPath(path);
            painter->restore();
        }
    } else {

        if (!(option.state & QStyle::State_Enabled)) {
            color = ThemeController::getCustomColorFromDT("kcomponent-disable");
        } else if (((option.state & QStyle::State_HasFocus) || (option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver))) {
            if (option.state.testFlag(QStyle::State_HasFocus) && option.state.testFlag(QStyle::State_Selected)) // QStyle::State_Enabled
            {
                color = ThemeController::getPaletteColorFromDT("highlight-active");
                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
            } else if ((option.state & QStyle::State_MouseOver)) {
                color = ThemeController::getPaletteColorFromDT("highlight-active");
                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
            } else {
                color = ThemeController::getPaletteColorFromDT("windowtext-active");
            }
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(color);
            painter->drawPath(path);
            painter->restore();
        }
    }
    QPen pen;
    pen.setWidth(1);
    pen.setColor(fontColor);
    painter->setPen(pen);
    QFontMetrics fm = option.fontMetrics;
    auto str = index.model()->data(index, Qt::DisplayRole).toString();
    if (fm.width(str) >= rect.width() - 12) {
        str = fm.elidedText(str, Qt::ElideRight, rect.width() - 12);
    }
    painter->drawText(rect.adjusted(12, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, str);
}

bool ListViewDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!event || !view)
            return false;
        if (event->type() == QEvent::ToolTip) {
            auto str = index.model()->data(index, Qt::DisplayRole).toString();
            QFontMetrics fm(option.font);
            //显示ToolTip
            int width = 12;
            if(view->verticalScrollBar()->isVisible())
                width += view->verticalScrollBar()->width();
            if (fm.horizontalAdvance(str) >= option.rect.width() - width) {
                QToolTip::showText(event->globalPos(), str, view);
            } else {
                QToolTip::hideText();
            }
            return true;
        } else if (event->type() == QEvent::Leave) {
            QToolTip::hideText();
            return true;
        }
        return QStyledItemDelegate::helpEvent(event, view, option, index);

}

QSize ListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size;
    size.setWidth(option.widget->width());
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_SearchLineEditItemHeight));
    return size;
}

void MyStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_ShapedFrame: {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);
        QColor color;
        if (ThemeController::themeMode() == LightTheme) {
            color = ThemeController::getCustomColorFromDT("kgray-17");
            color.setAlphaF(0.15);
            painter->setBrush(ThemeController::getCustomColorFromDT("kwhite"));
        } else {
            color = ThemeController::getCustomColorFromDT("kgray-3");
            painter->setBrush(color);
        }
        painter->setPen(color);
        int radius = ThemeController::getRadiusFromDT("kradius-menu");
        painter->drawRoundedRect(option->rect, radius, radius);
        break;
    }
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

QRect MyStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{

    switch (element) {
    case SE_ShapedFrameContents:
        return QProxyStyle::subElementRect(element, option, widget).adjusted(0, 4, 0, 2);
    default:
        return QProxyStyle::subElementRect(element, option, widget);
    }
}

}
#include "ksearchlineedit.moc"
#include "moc_ksearchlineedit.cpp"
