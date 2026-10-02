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

#ifndef USERMENU_H
#define USERMENU_H

#include <QMenu>
#include <QPushButton>

class QStyleOptionMenuItem;

class UserMenu : public QMenu
{
    Q_OBJECT
    friend class UserMenuStyle;
public:
    explicit UserMenu(QWidget *parent = nullptr);
    ~UserMenu();
    void addMiniButton(QAction *action, const QString& btnText, const QSize& btnSize);

private Q_SLOTS:
    void updateAllButtonPos();

Q_SIGNALS:
    void miniButtonClicked(QAction *action, QPushButton *button);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int menuItemWidthWithMiniButton(const QStyleOptionMenuItem *option, int baseWidth) const;

    QMap<QAction*, QPushButton*> m_actionBtnMap;
};

#endif // USERMENU_H
