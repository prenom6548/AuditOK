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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QGuiApplication>
#include <QtTest>

#include "views/tooltip-dialog.h"

using namespace UkuiQuick;

class tooltip_utils_test : public QObject
{
    Q_OBJECT

private slots:
    void shortTextIsReturnedAsIs();
    void latinTextWrapsAfterSixtyGraphemes();
    void cjkTextWrapsAfterThirtyGraphemes();
    void existingNewlineResetsWrapCounter();
};

void tooltip_utils_test::shortTextIsReturnedAsIs()
{
    const QString text = QStringLiteral("short tooltip text");

    QCOMPARE(TooltipUtils::instance().processedText(text), text);
}

void tooltip_utils_test::latinTextWrapsAfterSixtyGraphemes()
{
    const QString text(61, QLatin1Char('a'));
    const QString processed = TooltipUtils::instance().processedText(text);
    QString normalized = processed;

    QCOMPARE(processed.count(QLatin1Char('\n')), 1);
    QCOMPARE(processed.indexOf(QLatin1Char('\n')), 60);
    QCOMPARE(normalized.remove(QLatin1Char('\n')), text);
}

void tooltip_utils_test::cjkTextWrapsAfterThirtyGraphemes()
{
    const QString text = QStringLiteral("你").repeated(31);
    const QString processed = TooltipUtils::instance().processedText(text);
    QString normalized = processed;

    QCOMPARE(processed.count(QLatin1Char('\n')), 1);
    QCOMPARE(processed.indexOf(QLatin1Char('\n')), 30);
    QCOMPARE(normalized.remove(QLatin1Char('\n')), text);
}

void tooltip_utils_test::existingNewlineResetsWrapCounter()
{
    const QString text = QStringLiteral("你").repeated(20) + QLatin1Char('\n') + QStringLiteral("你").repeated(20);
    const QString processed = TooltipUtils::instance().processedText(text);

    QCOMPARE(processed.count(QLatin1Char('\n')), 1);
    QCOMPARE(processed, text);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    tooltip_utils_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-tooltip-utils.moc"
