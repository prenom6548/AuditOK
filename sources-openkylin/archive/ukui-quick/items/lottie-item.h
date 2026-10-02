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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_ITEMS_LOTTIE_ITEM_H
#define UKUI_QUICK_ITEMS_LOTTIE_ITEM_H

#include <QImage>
#include <QQuickItem>

namespace UkuiQuick {

class LottieAttached;
class LottieItemHostAdapter;
class LottieItemPrivate;

class LottieItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(UkuiQuick::LottieItem::HighlightMode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    enum HighlightModeFlag {
        Normal = 0x01,
        Highlight = 0x02,
        AutoHighlight = 0x04,
        ForceHighlight = Highlight | 0x08
    };
    Q_DECLARE_FLAGS(HighlightMode, HighlightModeFlag)
    Q_FLAG(HighlightMode)

    explicit LottieItem(QQuickItem *parent = nullptr);
    ~LottieItem() override;

    QString source() const;
    void setSource(const QString &source);

    HighlightMode mode() const;
    void setMode(HighlightMode mode);

    LottieAttached *lottieAttached() const;
    void notifyLottieChanged();

Q_SIGNALS:
    void sourceChanged();
    void modeChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

private:
    friend class LottieAttached;
    friend class LottieItemHostAdapter;

    void acceptCommittedLottieImage(const QImage &image);
    void attachLottieController(LottieAttached *attached);
    void ensureLottieController();
    void syncLottieHostAdapter();
    void syncLottieHostAdapter(QQuickWindow *window);
    void handleSizeChanged();
    void markTextureChanged();

    LottieItemPrivate *d {nullptr};
};

} // namespace UkuiQuick

Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::LottieItem::HighlightMode)

#endif // UKUI_QUICK_ITEMS_LOTTIE_ITEM_H
