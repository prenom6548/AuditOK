/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
#ifndef SHADOWDATA_H
#define SHADOWDATA_H

#include <QColor>
#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QSet>
#include <QVector>
namespace UkuiQuick {

class ShadowData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int xOffset READ xOffset WRITE setXOffset NOTIFY xOffsetChanged)
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset NOTIFY yOffsetChanged)
    Q_PROPERTY(int blurRadius READ blurRadius WRITE setBlurRadius NOTIFY blurRadiusChanged)
    Q_PROPERTY(int spread READ spread WRITE setSpread NOTIFY spreadChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(bool isMultiShadow READ isMultiShadow WRITE setIsMultiShadow NOTIFY isMultiShadowChanged)
    Q_PROPERTY(ShadowData* childShadow READ childShadow WRITE setChildShadow NOTIFY childShadowChanged)
public:
    explicit ShadowData(QObject *parent = nullptr);
    ~ShadowData() override;
    ShadowData(int x, int y, int blurRadius, int spread, const QColor &color, bool isMulti = false, ShadowData* child = nullptr, QObject *parent = nullptr);
    ShadowData(const ShadowData &shadow);
    ShadowData &operator= (const ShadowData &shadow);
    bool operator!=(const ShadowData &shadow);
    void setXOffset(int x);
    void setYOffset(int y);
    void setBlurRadius(int radius);
    void setSpread(int spread);
    void setShadowColor(const QColor &color);
    void setIsMultiShadow(bool isMultiShadow);
    void setChildShadow(ShadowData* child);

    int xOffset() const;
    int yOffset() const;
    int blurRadius() const;
    int spread() const;
    QColor shadowColor() const;
    bool isMultiShadow() const;
    ShadowData* childShadow() const;
Q_SIGNALS:
    void xOffsetChanged();
    void yOffsetChanged();
    void blurRadiusChanged();
    void spreadChanged();
    void shadowColorChanged();
    void isMultiShadowChanged();
    void childShadowChanged();
private:
    void setChildShadow(ShadowData *child, QSet<const ShadowData *> &visited);
    bool childShadowWouldCreateCycle(const ShadowData *child) const;
    static bool referencesShadowData(const ShadowData *root, const ShadowData *target, QSet<const ShadowData *> &visited);
    void clearChildShadowConnections();
    void copyChildShadowValues(const ShadowData *child, QSet<const ShadowData *> &visited);
    void connectChildShadowSource(ShadowData *child);
    void deleteChildShadow();

    //水平偏移量
    int m_xOffset = 0;
    //垂直偏移量
    int m_yOffset = 0;
    //阴影模糊半径
    int m_blurRadius = 0;
    //阴影扩展半径
    int m_spread = 0;
    //阴影颜色
    QColor m_color = QColor(0,0,0,0);
    //是否是多重阴影
    bool m_isMultiShadow = false;
    //另一层阴影
    QScopedPointer<ShadowData> m_childShadow;
    QPointer<ShadowData> m_childShadowSource;
    QVector<QMetaObject::Connection> m_childShadowConnections;
};

}
Q_DECLARE_METATYPE(UkuiQuick::ShadowData)

#endif // SHADOWDATA_H
