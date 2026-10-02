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

#include "xatom-helper.h"

class xatom_helper_test : public QObject
{
    Q_OBJECT

private slots:
    void singletonAndNoopX11DependentMethodsAreSafeOffscreen();
    void motifHintBorderOnlyRequiresDecorationBorder();
};

void xatom_helper_test::singletonAndNoopX11DependentMethodsAreSafeOffscreen()
{
    auto *helper = XAtomHelper::getInstance();

    QVERIFY(helper != nullptr);
    QCOMPARE(XAtomHelper::getInstance(), helper);
    QVERIFY(!XAtomHelper::isFrameLessWindow(0));
    QVERIFY(!helper->isWindowDecorateBorderOnly(0));
    QVERIFY(!helper->isUKUICsdSupported());
    QVERIFY(!helper->isUKUIDecorationWindow(0));

    const UnityCorners corners = helper->getWindowBorderRadius(0);
    QCOMPARE(corners.topLeft, 0UL);
    QCOMPARE(corners.topRight, 0UL);
    QCOMPARE(corners.bottomLeft, 0UL);
    QCOMPARE(corners.bottomRight, 0UL);

    helper->setWindowBorderRadius(0, UnityCorners{1, 2, 3, 4});
    helper->setWindowBorderRadius(0, 5, 6, 7, 8);
    helper->setUKUIDecoraiontHint(0, true);
    helper->setWindowMotifHint(0, MotifWmHints{});

    const MotifWmHints hints = helper->getWindowMotifHint(0);
    QCOMPARE(hints.flags, 0UL);
    QCOMPARE(hints.decorations, 0UL);
}

void xatom_helper_test::motifHintBorderOnlyRequiresDecorationBorder()
{
    auto *helper = XAtomHelper::getInstance();

    MotifWmHints hints;
    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = MWM_DECOR_BORDER;
    QVERIFY(helper->isWindowMotifHintDecorateBorderOnly(hints));

    hints.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
    hints.decorations = MWM_DECOR_BORDER;
    QVERIFY(helper->isWindowMotifHintDecorateBorderOnly(hints));

    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = MWM_DECOR_TITLE;
    QVERIFY(!helper->isWindowMotifHintDecorateBorderOnly(hints));

    hints.flags = 0;
    hints.decorations = MWM_DECOR_BORDER;
    QVERIFY(!helper->isWindowMotifHintDecorateBorderOnly(hints));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QGuiApplication app(argc, argv);
    xatom_helper_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-xatom-helper.moc"
