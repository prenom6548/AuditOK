/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 */

#include "user-menu.h"

#include <QEvent>
#include <QFontMetrics>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionButton>

namespace {

constexpr int kMiniButtonLeftSpacing = 8;
constexpr int kMiniButtonRightMargin = 8;
const QSize kMiniButtonMinimumSize(64, 32);

int miniButtonPreferredWidth(const QPushButton *button)
{
    if (!button) {
        return kMiniButtonMinimumSize.width();
    }

    const QFontMetrics fontMetrics(button->font());
    const int textWidth = fontMetrics.horizontalAdvance(button->text());
    const int minimumWidth = button->minimumWidth();

    QStyleOptionButton option;
    option.initFrom(button);
    option.text = button->text();
    option.rect = QRect(0, 0, minimumWidth, button->height());
    const QRect contentRect = button->style()->subElementRect(QStyle::SE_PushButtonContents, &option, button);
    const int styleHorizontalMargin = qMax(0, option.rect.width() - contentRect.width());

    return qMax(minimumWidth, textWidth + styleHorizontalMargin);
}

void updateMiniButtonWidth(QPushButton *button)
{
    if (!button) {
        return;
    }

    button->setFixedWidth(miniButtonPreferredWidth(button));
}

int availableActionRightEdge(const QMenu *menu, const QRect &actionRect)
{
    if (!menu) {
        return actionRect.left() + actionRect.width();
    }

    const int panelWidth = menu->style()->pixelMetric(QStyle::PM_MenuPanelWidth, nullptr, menu);
    const int menuRightEdge = (menu->isVisible() ? menu->width() : menu->sizeHint().width()) - panelWidth;
    return qMax(actionRect.left() + actionRect.width(), menuRightEdge);
}

int menuItemTextRightEdge(const QMenu *menu, const QStyleOptionMenuItem *option)
{
    if (!menu || !option) {
        return 0;
    }

    const QFontMetrics fontMetrics(option->font);
    const int textWidth = fontMetrics.horizontalAdvance(option->text);

    int textStart = menu->style()->pixelMetric(QStyle::PM_MenuHMargin, option, menu);
    if (option->menuHasCheckableItems || option->checkType != QStyleOptionMenuItem::NotCheckable) {
        textStart += menu->style()->pixelMetric(QStyle::PM_IndicatorWidth, option, menu)
                + menu->style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, option, menu);
    }

    const int iconWidth = option->maxIconWidth > 0
            ? option->maxIconWidth
            : (option->icon.isNull() ? 0 : menu->style()->pixelMetric(QStyle::PM_SmallIconSize, option, menu));
    if (iconWidth > 0) {
        textStart += iconWidth + menu->style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, option, menu);
    }

    return textStart + textWidth;
}

}

class UserMenuStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize,
                           const QWidget *widget = nullptr) const override
    {
        QSize size = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);

        if (type == QStyle::CT_MenuItem) {
            const UserMenu *menu = qobject_cast<const UserMenu *>(widget);
            const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option);
            if (menu && menuItem) {
                size.setWidth(menu->menuItemWidthWithMiniButton(menuItem, size.width()));
            }
        }

        return size;
    }
};

UserMenu::UserMenu(QWidget *parent) : QMenu(parent)
{
    auto menuStyle = new UserMenuStyle;
    menuStyle->setParent(this);
    setStyle(menuStyle);
    connect(this, &UserMenu::aboutToShow, this, &UserMenu::updateAllButtonPos);
}

UserMenu::~UserMenu()
{
    m_actionBtnMap.clear();
}

int UserMenu::menuItemWidthWithMiniButton(const QStyleOptionMenuItem *option, int baseWidth) const
{
    int requiredWidth = 0;

    for (auto it = m_actionBtnMap.constBegin(); it != m_actionBtnMap.constEnd(); ++it) {
        QAction *action = it.key();
        QPushButton *button = it.value();
        if (!action || !button || !option || action->text() != option->text) {
            continue;
        }

        requiredWidth = qMax(requiredWidth, menuItemTextRightEdge(this, option)
                + kMiniButtonLeftSpacing
                + miniButtonPreferredWidth(button)
                + kMiniButtonRightMargin);
    }

    return qMax(baseWidth, requiredWidth);
}

void UserMenu::addMiniButton(QAction *action, const QString &btnText, const QSize &btnSize)
{
    if (!action || action->parent() != this) {
        return;
    }

    const QSize minimumButtonSize = btnSize.expandedTo(kMiniButtonMinimumSize);
    QPushButton *miniBtn = new QPushButton(this);
    miniBtn->setText(btnText);
    miniBtn->setMinimumSize(minimumButtonSize);
    miniBtn->setFixedHeight(minimumButtonSize.height());
    updateMiniButtonWidth(miniBtn);
    miniBtn->setVisible(false);
    miniBtn->setAttribute(Qt::WA_NoMousePropagation);
    miniBtn->installEventFilter(this);

    connect(miniBtn, &QPushButton::clicked, this, [=]() {
        emit miniButtonClicked(action, miniBtn);
    });

    m_actionBtnMap[action] = miniBtn;
    updateGeometry();
}

void UserMenu::updateAllButtonPos()
{
    QMap<QAction*, QPushButton*>::iterator it;
    for (it = m_actionBtnMap.begin(); it != m_actionBtnMap.end(); ++it) {
        QAction *action = it.key();
        QPushButton *btn = it.value();

        if (!btn) continue;
        QRect actionRect = this->actionGeometry(action);
        if (actionRect.isEmpty()) {
            btn->setVisible(false);
            continue;
        }

        updateMiniButtonWidth(btn);
        int btnX = availableActionRightEdge(this, actionRect) - kMiniButtonRightMargin - btn->width();
        int btnY = actionRect.top() + (actionRect.height() - btn->height()) / 2;
        btn->setGeometry(btnX, btnY, btn->width(), btn->height());
        btn->setVisible(true);

        btn->raise();
    }
}

void UserMenu::resizeEvent(QResizeEvent *event)
{
    QMenu::resizeEvent(event);
    updateAllButtonPos();
}

bool UserMenu::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        QPushButton *btn = qobject_cast<QPushButton*>(watched);
        if (btn) {
            this->setActiveAction(nullptr);
        }
    }
    return QMenu::eventFilter(watched, event);
}
