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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kcomboboxwidget.h"
#include "klabel.h"
#include "ktoolbutton.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QList>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProxyStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QWidgetAction>

namespace kdk
{

class Q_DECL_HIDDEN ProxyStyle : public QProxyStyle
{
    Q_OBJECT
public:
    ProxyStyle(QWidget *parent = nullptr)
    {
    }
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const override;
};

class Q_DECL_HIDDEN MenuItem : public QToolButton, public ThemeController
{
    Q_OBJECT
public:
    MenuItem(QString week, int id, QWidget *parent = nullptr);

    void setIconVisible(bool status);

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

Q_SIGNALS:
    void showLabelIcon(int id);
    void hideLabelIcon(int id);

private:
    QLabel *m_pTextLabel;
    QLabel *m_pMenuIconLabel;
    QHBoxLayout *m_pHLayout;
    int m_id = 0;
    ProxyStyle *m_proxyStyle = nullptr;
};

class Q_DECL_HIDDEN KComboBoxWidgetPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KComboBoxWidget)
public:
    KComboBoxWidgetPrivate(KComboBoxWidget *parent);
    void initLayout();
    void setChosenString();
    QStringList reorderListBasedOnSelection(const QStringList stringList, const QStringList chosenList);

private:
    KComboBoxWidget *q_ptr;
    QLabel *m_pMainLabel;
    KLabel *m_pSubLabel;
    QLabel *m_pIconLabel;
    QHBoxLayout *m_pHLayout;
    bool m_mouseEnter = false;
    QStringList m_stringList;
    QStringList m_chosenList;
    QMenu *m_pMenu;
};

KComboBoxWidget::KComboBoxWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KComboBoxWidgetPrivate(this))
{
}

void KComboBoxWidget::setStringList(QStringList list)
{
    Q_D(KComboBoxWidget);
    d->m_stringList = list;
    for (int i = 0; i < list.count(); i++) {
        MenuItem *item = new MenuItem(list[i], i, this);
        connect(item, &MenuItem::showLabelIcon, this, [=](int index) {
            if (!d->m_chosenList.contains(d->m_stringList.at(index)))
                d->m_chosenList.append(d->m_stringList.at(index));
            d->setChosenString();
        });
        connect(item, &MenuItem::hideLabelIcon, this, [=](int index) {
            if (d->m_chosenList.contains(d->m_stringList.at(index)))
                d->m_chosenList.removeOne(d->m_stringList.at(index));
            d->setChosenString();
        });
        QWidgetAction *action = new QWidgetAction(this);
        action->setDefaultWidget(item);
        d->m_pMenu->addAction(action);
    }
}

QStringList KComboBoxWidget::getChosenList()
{
    Q_D(KComboBoxWidget);
    return d->m_chosenList;
}

void KComboBoxWidget::setMainText(QString text)
{
    Q_D(KComboBoxWidget);
    d->m_pMainLabel->setText(text);
}

QMenu *KComboBoxWidget::menu()
{
    Q_D(KComboBoxWidget);
    return d->m_pMenu;
}

void KComboBoxWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(KComboBoxWidget);
    emit comBoxWidgetClicked();
    QWidget::mouseReleaseEvent(event);
    return;
}

void KComboBoxWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(KComboBoxWidget);
}

void KComboBoxWidget::leaveEvent(QEvent *e)
{
    Q_D(KComboBoxWidget);
    d->m_mouseEnter = false;
    repaint();
}

void KComboBoxWidget::enterEvent(QEvent *e)
{
    Q_D(KComboBoxWidget);
    d->m_mouseEnter = true;
    repaint();
}

void KComboBoxWidget::paintEvent(QPaintEvent *event)
{
    Q_D(KComboBoxWidget);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QPainterPath path;
    opt.rect.adjust(0, 0, 0, 0);

    if (!opt.state.testFlag(QStyle::State_Enabled)) {
        p.setBrush(ThemeController::getPaletteColorFromDT("button", QPalette::Disabled));
    } else {

        if (d->m_mouseEnter) {
            p.setBrush(ThemeController::getPaletteColorFromDT("midlight"));
        } else {
            p.setBrush(ThemeController::getPaletteColorFromDT("button"));
        }
    }

    p.setOpacity(1);
    p.setPen(Qt::NoPen);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;
    p.drawRoundedRect(opt.rect, radius, radius);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
}

KComboBoxWidgetPrivate::KComboBoxWidgetPrivate(KComboBoxWidget *parent)
    : q_ptr(parent)
{
    Q_Q(KComboBoxWidget);
    initLayout();
}

void KComboBoxWidgetPrivate::initLayout()
{
    Q_Q(KComboBoxWidget);
    m_pMenu = new QMenu;
    m_pMainLabel = new QLabel();
    m_pMainLabel->setAlignment(Qt::AlignVCenter);

    m_pSubLabel = new KLabel();
    m_pSubLabel->setAlignment(Qt::AlignRight);

    m_pSubLabel->setFixedWidth(165);
    m_pIconLabel = new QLabel();
    QIcon labelIcon = QIcon::fromTheme("ukui-down.symbolic");
    m_pIconLabel->setPixmap(labelIcon.pixmap(QSize(16, 16)));
    m_pIconLabel->setProperty("useIconHighlightEffect", 0x1);
    m_pHLayout = new QHBoxLayout();
    m_pHLayout->setSpacing(0);
    m_pHLayout->setContentsMargins(0, 0, 0, 0);
    q->setContentsMargins(0, 0, 0, 0);

    m_pHLayout->addItem(new QSpacerItem(16, 1, QSizePolicy::Fixed));
    m_pHLayout->addWidget(m_pMainLabel, Qt::AlignVCenter);
    m_pMainLabel->setAlignment(Qt::AlignVCenter);
    m_pHLayout->addItem(new QSpacerItem(12, 20, QSizePolicy::Expanding));
    m_pHLayout->addWidget(m_pSubLabel, Qt::AlignVCenter);
    m_pSubLabel->setAlignment(Qt::AlignVCenter);
    m_pHLayout->addItem(new QSpacerItem(8, 30, QSizePolicy::Minimum));
    m_pHLayout->addWidget(m_pIconLabel, Qt::AlignVCenter);
    m_pHLayout->addItem(new QSpacerItem(8, 5, QSizePolicy::Fixed));

    q->setLayout(m_pHLayout);

    connect(q, &KComboBoxWidget::comBoxWidgetClicked, q, [=] {
        if (!m_pMenu->isVisible()) {
            m_pMenu->exec(q->mapToGlobal(QPoint(0, q->rect().height())));
        } else {
            m_pMenu->hide();
        }
    });
}

void KComboBoxWidgetPrivate::setChosenString()
{
    Q_Q(KComboBoxWidget);
    QStringList list = reorderListBasedOnSelection(m_stringList, m_chosenList);
    QString text;
    for (int i = 0; i < list.count(); i++) {
        text.append(list.at(i));
        if (i < list.count() - 1)
            text.append(" ");
    }

    m_pSubLabel->setText(text);
}

QStringList KComboBoxWidgetPrivate::reorderListBasedOnSelection(const QStringList stringList, const QStringList chosenList)
{
    QStringList result;
    QVector<bool> isSelected(chosenList.size(), false);

    for (const QString &selected : stringList) {
        int index = chosenList.indexOf(selected);
        if (index != -1) {
            result.append(selected);
            isSelected[index] = true;
        }
    }

    for (int i = 0; i < chosenList.size(); ++i) {
        if (!isSelected[i]) {
            result.append(chosenList.at(i));
        }
    }

    return result;
}

void ProxyStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_ToolButton: {
        if (const QStyleOptionComplex *f = qstyleoption_cast<const QStyleOptionComplex *>(option)) {
            const bool hover = f->state & State_MouseOver;
            if (hover) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                QStyleOption opt;
                QColor color = ThemeController::getPaletteColorFromDT("highlight");
                painter->setBrush(color);
                int radius = ThemeController::getRadiusFromDT("kradius-normal");
                if (radius == -1)
                    radius = 6;
                painter->drawRoundedRect(option->rect, radius, radius);
                painter->restore();
                return;
            }
        }
        break;
    }
    default:
        break;
    }
}

MenuItem::MenuItem(QString week, int id, QWidget *parent)
    : QToolButton(parent)
    , m_id(id)
{
    m_proxyStyle = new ProxyStyle(this);
    this->setStyle(m_proxyStyle);
    m_pTextLabel = new QLabel();
    m_pTextLabel->setText(week);
    this->setAutoRaise(true); // 设置默认状态背景透明

    m_pMenuIconLabel = new QLabel();

    m_pMenuIconLabel->setFixedSize(16, 16);
    QIcon labelIcon = QIcon::fromTheme("object-select-symbolic");
    m_pMenuIconLabel->setPixmap(labelIcon.pixmap(QSize(16, 16)));
    m_pMenuIconLabel->setProperty("useIconHighlightEffect", 0x8);
    m_pMenuIconLabel->setFixedSize(16, 16);

    m_pHLayout = new QHBoxLayout();
    m_pHLayout->setContentsMargins(17, 0, 0, 0);
    this->setLayout(m_pHLayout);
    m_pHLayout->addWidget(m_pMenuIconLabel);
    m_pHLayout->addWidget(m_pTextLabel);

    QSizePolicy policy = m_pMenuIconLabel->sizePolicy();
    policy.setRetainSizeWhenHidden(true);
    m_pMenuIconLabel->setSizePolicy(policy);
    m_pMenuIconLabel->setVisible(false);

    connect(m_gsetting, &QGSettings::changed, this, [=](const QString &key) {
        if (ThemeController::themeMode() == DarkTheme) {
            QPalette palette;
            palette.setColor(QPalette::Active, QPalette::ButtonText, Qt::white);
            m_pTextLabel->setPalette(palette);
        } else {
            QPalette palette;
            palette.setColor(QPalette::Active, QPalette::ButtonText, Qt::black);
            m_pTextLabel->setPalette(palette);
        }
    });
}

void MenuItem::setIconVisible(bool status)
{
    m_pMenuIconLabel->setVisible(status);
}

void MenuItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!this->m_pMenuIconLabel->isVisible()) {
            m_pMenuIconLabel->setVisible(true);
            emit showLabelIcon(m_id);
        } else {
            m_pMenuIconLabel->setVisible(false);
            emit hideLabelIcon(m_id);
        }
    }
    QToolButton::mouseReleaseEvent(event);
}

void MenuItem::enterEvent(QEvent *e)
{
    QApplication::postEvent(m_pMenuIconLabel, new QEvent(QEvent::Enter));

    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::ButtonText, Qt::white);
    m_pTextLabel->setPalette(palette);
}

void MenuItem::leaveEvent(QEvent *e)
{
    if (ThemeController::themeMode() == LightTheme) {
        QPalette palette;
        palette.setColor(QPalette::Active, QPalette::ButtonText, Qt::black);
        m_pTextLabel->setPalette(palette);
    }
    QApplication::postEvent(m_pMenuIconLabel, new QEvent(QEvent::Leave));
}

}

#include "kcomboboxwidget.moc"
#include "moc_kcomboboxwidget.cpp"
