/*
 * Peony-Qt
 *
 * Copyright (C) 2026, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */

#ifndef FILELABELMENU_H
#define FILELABELMENU_H

#include <QMenu>
#include <QStyledItemDelegate>
#include <QList>
#include <QLabel>
class QListWidget;

class LabelItemDelegate : public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

class FileLabelMenu : public QMenu
{
    Q_OBJECT
public:
    explicit FileLabelMenu(QStringList &selections, QWidget *parent = nullptr);

    void addTag(int id, const QString &color, const QString &text, bool checked = false);

    QList<QPair<QString, QString>> selectedTags() const;
    void updateSelections(const QStringList &selections);

protected:
    void loadLabels();
    bool clickItem(int index);
    void clearLabels();

private:
    QLabel *m_titleLabel = nullptr;
    QListWidget *listWidget = nullptr;

    QList<int> m_ids;
    QStringList m_selections;
};

#endif // FILELABELMENU_H
