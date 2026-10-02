/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 * Unified QBrush / DT gradient parsing for QML controls and private C++ types.
 */

#ifndef DTBRUSHUTIL_H
#define DTBRUSHUTIL_H

#include <QBrush>
#include <QColor>

namespace UKUIQQC2Style {

struct DtColorInfo {
    bool isSolidPattern = true;
    QColor startColor;
    QColor endColor;
};

class DtBrushUtil
{
public:
  DtBrushUtil() = delete;

  static DtColorInfo fromBrush(const QBrush &brush);
  static bool isSolidPattern(const QBrush &brush);
  /// 对齐 ThemeController::getCustomColorFromDT / getPaletteColorFromDT 的 brush.color()
  static QColor brushColor(const QBrush &brush);
  static QColor startColor(const QBrush &brush);
  static QColor endColor(const QBrush &brush);
};

} // namespace UKUIQQC2Style

#endif // DTBRUSHUTIL_H
