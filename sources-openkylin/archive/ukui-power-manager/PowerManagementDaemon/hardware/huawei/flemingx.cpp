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

#include "flemingx.h"
#include <cmath>
#include <QtMath>
#include <QDebug>


/*
  - flemingX的OLED调光策略
    - 设置值 = b * UI值^2.2+k；(50~100)
    - 设置值 = a * UI值 + k；(0~50)
    - （UI值，设置值）带入计算。
      - （0，4）带入公式1计算出K，带入50，得出设置1。
      - （100，450），把设置1带入公式2，50带入UI值，k带入得出a。
    - 然后50%时UI值时两边的值一样。
    - 需要对公式逆向出默认值。
*/


FlemingX::FlemingX(int maxbrightness, QObject *parent):
    QObject(parent)
{
    initFormulaParam(maxbrightness);
}

int FlemingX::convert2NodeValue(int UiValue) const
{
    return UiValue > m_commonThreshold ?
                 calculateUsingFormulaBeyond50(UiValue) : calculateUsingFormulaSmall50(UiValue);
}

int FlemingX::convert2UiValue(int nodeValue) const
{
    return nodeValue > m_defaultValueThreshold ?
               reverseCalculateUsingFormulaBeyond50(nodeValue):reverseCalculateUsingFormulaSmall50(nodeValue);
}

void FlemingX::initFormulaParam(int maxBrightness)
{
    m_b = (maxBrightness - m_k)/(std::pow(MAX_UI, POWER_EXPONENT));
    m_defaultValueThreshold = m_b * std::pow(m_commonThreshold, POWER_EXPONENT) + m_k;
    m_a = (m_defaultValueThreshold - m_k) / m_commonThreshold;

    qDebug()<< __func__ << m_a << m_b << m_k << m_defaultValueThreshold;
}

void FlemingX::test()
{
    for(int circle = 0; circle <= 100; circle++) {

        int value = convert2NodeValue(circle);
        int defautValue = convert2UiValue(value);

        qDebug()<<__func__<<__LINE__<<circle<<value<<defautValue;
    }
}

inline int FlemingX::reverseCalculateUsingFormulaSmall50(int currentValue) const
{
    int ret = (currentValue* 1.0 - m_k)/m_a + ROUNDING_FACTOR; // Adding 0.5 for rounding
    return ret;
}

inline int FlemingX::reverseCalculateUsingFormulaBeyond50(int currentValue) const
{
    qreal temp = (currentValue - m_k) / m_b;
    return static_cast<int>(std::pow(temp, 1.0 / POWER_EXPONENT) + ROUNDING_FACTOR);
}

inline int FlemingX::calculateUsingFormulaSmall50(int currentValue) const
{
    int ret = m_a * currentValue * 1.0 + m_k  + ROUNDING_FACTOR;
    return ret;
}

inline int FlemingX::calculateUsingFormulaBeyond50(int currentValue) const
{
    qreal ret = m_b * std::pow(currentValue* 1.0, POWER_EXPONENT) + m_k  + ROUNDING_FACTOR;
    return ret;
}


