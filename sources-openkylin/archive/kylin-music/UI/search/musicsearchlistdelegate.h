/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 */

#include <QScopedPointer>
#include <QStyledItemDelegate>
#include <QListView>

class MusicSearchListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit MusicSearchListDelegate(QWidget *parent = Q_NULLPTR);
    ~MusicSearchListDelegate() override;

Q_SIGNALS:
    void SearchClear() const;

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
#ifndef WHITE_TEST
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
#endif
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QPixmap m_shadowImg;
};
