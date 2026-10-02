/*
 * Copyright 2025 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FLEMINGX_H
#define FLEMINGX_H

#include <QObject>

class FlemingX : public QObject
{
    Q_OBJECT
public:
    explicit FlemingX(int maxbrightness, QObject *parent = nullptr);
    FlemingX() = delete;
    int convert2NodeValue(int UiValue) const;
    int convert2UiValue(int nodeValue) const;

private:
    inline int reverseCalculateUsingFormulaSmall50(const int currentValue) const;
    inline int reverseCalculateUsingFormulaBeyond50(const int currentValue) const;

    inline int calculateUsingFormulaSmall50(const int currentValue) const;
    inline int calculateUsingFormulaBeyond50(const int currentValue) const;

    void initFormulaParam(int maxBrightness);

    void test();
private:
    int m_maxBrightness = 0;

    qreal m_a = 1.94; //[0-50]
    double m_b = 0.017755579807; //[50-100]
    const qreal m_k = 4.0;

    static constexpr double ROUNDING_FACTOR = 0.5;
    static constexpr double POWER_EXPONENT = 2.2;

    static constexpr double MAX_UI = 100;
    const int m_commonThreshold = 50;
    int m_defaultValueThreshold = 101;

};

#endif // FLEMINGX_H
