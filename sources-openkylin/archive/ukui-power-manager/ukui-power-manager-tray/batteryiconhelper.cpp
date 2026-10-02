/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "batteryiconhelper.h"

#include <QColor>
#include <QDebug>
#include <QImage>
#include <QVariant>

#include "upm-gsettings/upm_gsettings.h"

// 主题图标真实填充色（ukui-icon-theme-default）：
//   level-0:         无填充
//   level-10:        红  #FF0000
//   level-20:        橙  #F9960C
//   level-30~100:    绿  #2AAA00

static bool isBatteryLevelIconName(const QString &iconName)
{
    return iconName.startsWith(QStringLiteral("battery-level-"))
        && iconName.endsWith(QStringLiteral("-symbolic"));
}

// 绿色检测：#2AAA00 (42,170,0)
static bool isNearBatteryGreen(const QColor &c)
{
    return c.alpha() > 8
        && qAbs(c.red()   - 42)  <= 40
        && qAbs(c.green() - 170) <= 40
        && qAbs(c.blue()  - 0)   <= 40;
}

// 红色检测：#FF0000，用 HSV（h≈0°，高饱和度）
static bool isNearBatteryRed(const QColor &c)
{
    if (c.alpha() <= 8) {
        return false;
    }
    int h, s, v;
    c.getHsv(&h, &s, &v);
    if (h < 0) {
        return false;
    }
    return ((h <= 15) || (h >= 345)) && s > 180 && v > 150;
}

// 橙色检测：#F9960C (249,150,12)
static bool isNearBatteryOrange(const QColor &c)
{
    return c.alpha() > 8
        && qAbs(c.red()   - 249) <= 40
        && qAbs(c.green() - 150) <= 40
        && qAbs(c.blue()  - 12)  <= 40;
}

enum class SourceColor { Red, Orange, Green };

static QPixmap recolorBatteryFill(const QPixmap &src,
                                  const QColor &target,
                                  SourceColor from)
{
    if (src.isNull()) {
        return src;
    }

    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    int replacedPixels = 0;
    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const QColor cur = QColor::fromRgba(line[x]);
            bool match = false;
            switch (from) {
            case SourceColor::Red:    match = isNearBatteryRed(cur);    break;
            case SourceColor::Orange: match = isNearBatteryOrange(cur); break;
            case SourceColor::Green:  match = isNearBatteryGreen(cur);  break;
            }
            if (!match) { continue; }
            QColor out = target;
            out.setAlpha(cur.alpha());
            line[x] = out.rgba();
            ++replacedPixels;
        }
    }

    const char *fromStr = (from == SourceColor::Red)    ? "red"
                        : (from == SourceColor::Orange) ? "orange"
                                                        : "green";
    qDebug() << "[BatteryIconColor] recolorBatteryFill:"
             << "size=" << src.size()
             << "from=" << fromStr
             << "target=" << target.name(QColor::HexRgb)
             << "replacedPixels=" << replacedPixels;

    QPixmap out = QPixmap::fromImage(img);
    out.setDevicePixelRatio(src.devicePixelRatio());
    return out;
}

static QIcon buildRecoloredIcon(const QIcon &baseIcon,
                                const QColor &target,
                                SourceColor from)
{
    if (baseIcon.isNull()) {
        return QIcon();
    }
    QIcon out;
    const QList<int> sizes = {16, 20, 22, 24, 32};
    for (int s : sizes) {
        const QPixmap pm = baseIcon.pixmap(s, s);
        if (!pm.isNull()) {
            out.addPixmap(recolorBatteryFill(pm, target, from));
        }
    }
    return out;
}

QIcon buildBatteryLevelIcon(const QString &iconName,
                            double batteryPercentage,
                            const QString &context)
{
    const QString logPrefix = context.isEmpty()
        ? QStringLiteral("[BatteryIconColor]")
        : QStringLiteral("[BatteryIconColor]") + context;

    QIcon icon = QIcon::fromTheme(iconName);
    if (icon.isNull()) {
        qDebug() << logPrefix << "theme icon is null, skip recolor" << iconName;
        return icon;
    }
    if (!isBatteryLevelIconName(iconName)) {
        return icon;
    }

    // 从 gsettings 读取低电量阈值（直接读，不走缓存，避免延迟）
    const int percentageLow = UpmGsettings::self()->percentageLowValue();

    const int colorPct  = static_cast<int>(batteryPercentage);
    const int iconLevel = qRound(batteryPercentage / 10.0) * 10;

    // 极低：<= 10%（percentageAction 固定为 10%）
    const bool inCritical = (colorPct <= 10);
    // 低电量：> 10% 且 <= percentageLow
    const bool inLow      = (!inCritical && colorPct <= percentageLow);

    const char *tier = inCritical ? "critical(red)"
                     : (inLow    ? "low(orange)"
                                 : "normal(green)");

    qDebug() << logPrefix << "evaluate:"
             << "iconName=" << iconName
             << "colorPct=" << colorPct
             << "iconLevel=" << iconLevel
             << "percentageLow=" << percentageLow
             << "tier=" << tier;

    // ── iconLevel == 10（主题原色：红）──────────────────────────────
    if (iconLevel == 10) {
        if (inCritical) {
            // 极低电量，红色原图不动
            qDebug() << logPrefix << "level10 inCritical: keep red";
            return icon;
        }
        // 不在极低区间：需要改掉红色
        const QColor target = inLow ? QColor(QStringLiteral("#F9960C"))
                                    : QColor(QStringLiteral("#2AAA00"));
        qDebug() << logPrefix << "level10 !inCritical: red ->" << target.name(QColor::HexRgb);
        const QIcon recolored = buildRecoloredIcon(icon, target, SourceColor::Red);
        if (!recolored.isNull()) { return recolored; }
        qDebug() << logPrefix << "recolor failed, fallback to original";
        return icon;
    }

    // ── iconLevel == 20（主题原色：橙）──────────────────────────────
    if (iconLevel == 20) {
        if (inLow) {
            // 低电量区间，橙色原图不动
            qDebug() << logPrefix << "level20 inLow: keep orange";
            return icon;
        }
        // 不在低电量区间：橙色 → 绿色
        qDebug() << logPrefix << "level20 !inLow: orange -> #2AAA00";
        const QIcon recolored = buildRecoloredIcon(icon, QColor(QStringLiteral("#2AAA00")),
                                                   SourceColor::Orange);
        if (!recolored.isNull()) { return recolored; }
        qDebug() << logPrefix << "recolor failed, fallback to original";
        return icon;
    }

    // ── iconLevel >= 30（主题原色：绿）──────────────────────────────
    if (iconLevel >= 30) {
        if (inLow) {
            // 低电量区间：绿色 → 橙色
            qDebug() << logPrefix << "level" << iconLevel << "inLow: green -> #F9960C";
            const QIcon recolored = buildRecoloredIcon(icon, QColor(QStringLiteral("#F9960C")),
                                                       SourceColor::Green);
            if (!recolored.isNull()) { return recolored; }
            qDebug() << logPrefix << "recolor failed, fallback to original";
        } else {
            qDebug() << logPrefix << "level" << iconLevel << "!inLow: keep green";
        }
        return icon;
    }

    // iconLevel == 0：无填充，直接返回
    qDebug() << logPrefix << "level0: no fill, keep as-is";
    return icon;
}
