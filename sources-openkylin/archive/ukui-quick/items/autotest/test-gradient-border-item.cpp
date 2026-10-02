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
#include <QImage>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSignalSpy>
#include <QtTest>

#include "gradient-border-item.h"

using namespace UkuiQuick;

class TestableGradientBorderItem : public GradientBorderItem
{
public:
    using GradientBorderItem::GradientBorderItem;
    using GradientBorderItem::componentComplete;
    using GradientBorderItem::paint;
};

namespace {

class WarningCollector
{
public:
    WarningCollector()
    {
        Q_ASSERT(s_current == nullptr);
        s_current = this;
        m_previous = qInstallMessageHandler(&WarningCollector::handler);
    }

    ~WarningCollector()
    {
        qInstallMessageHandler(m_previous);
        s_current = nullptr;
    }

    QStringList warnings() const
    {
        return m_warnings;
    }

private:
    static void handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
    {
        if (s_current && type == QtWarningMsg) {
            s_current->m_warnings.append(message);
            return;
        }
        if (s_current && s_current->m_previous) {
            s_current->m_previous(type, context, message);
        }
    }

    static WarningCollector *s_current;
    QtMessageHandler m_previous = nullptr;
    QStringList m_warnings;
};

WarningCollector *WarningCollector::s_current = nullptr;

QImage paintItem(TestableGradientBorderItem &item)
{
    QImage image(item.width(), item.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    item.paint(&painter);
    painter.end();

    return image;
}

}

class gradient_border_item_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndNoopSetters();
    void borderColorChangesOnlyEmitOnPointerChange();
    void borderColorWithoutQmlEngineDoesNotWarn();
    void componentCompleteWithoutQmlEngineDoesNotWarn();
    void specificBorderAnimationStartsAndStops();
    void paintUsesCustomPureBorderColor();
    void paintUsesCustomGradientBorderColor();
    void paintUsesSpecificBorderGradient();
};

void gradient_border_item_test::defaultsAndNoopSetters()
{
    TestableGradientBorderItem item;

    QCOMPARE(item.borderWidth(), 2.0);
    QCOMPARE(item.borderRadius(), 6.0);
    QCOMPARE(item.deg(), 0.0);
    QVERIFY(!item.enableSpecificBorder());
    QVERIFY(item.borderColor() == nullptr);

    QSignalSpy widthSpy(&item, &GradientBorderItem::borderWidthChanged);
    QSignalSpy radiusSpy(&item, &GradientBorderItem::borderRadiusChanged);
    QSignalSpy degSpy(&item, &GradientBorderItem::degChanged);
    QSignalSpy enableSpy(&item, &GradientBorderItem::enableSpecificBorderChanged);

    item.setBorderWidth(2.0);
    item.setBorderRadius(6.0);
    item.setDeg(0.0);
    item.setEnableSpecificBorder(false);

    QCOMPARE(widthSpy.count(), 0);
    QCOMPARE(radiusSpy.count(), 0);
    QCOMPARE(degSpy.count(), 0);
    QCOMPARE(enableSpy.count(), 0);

    item.setBorderWidth(4.0);
    item.setBorderRadius(10.0);
    item.setDeg(30.0);

    QCOMPARE(item.borderWidth(), 4.0);
    QCOMPARE(item.borderRadius(), 10.0);
    QCOMPARE(item.deg(), 30.0);
    QCOMPARE(widthSpy.count(), 1);
    QCOMPARE(radiusSpy.count(), 1);
    QCOMPARE(degSpy.count(), 1);
}

void gradient_border_item_test::borderColorChangesOnlyEmitOnPointerChange()
{
    TestableGradientBorderItem item;
    GradientColor first(QColor(Qt::red));
    GradientColor second(QColor(Qt::blue));
    QSignalSpy colorSpy(&item, &GradientBorderItem::borderColorChanged);

    item.setBorderColor(nullptr);
    QCOMPARE(colorSpy.count(), 0);

    item.setBorderColor(&first);
    QCOMPARE(item.borderColor(), &first);
    QCOMPARE(colorSpy.count(), 1);

    item.setBorderColor(&first);
    QCOMPARE(colorSpy.count(), 1);

    item.setBorderColor(&second);
    QCOMPARE(item.borderColor(), &second);
    QCOMPARE(colorSpy.count(), 2);
}

void gradient_border_item_test::borderColorWithoutQmlEngineDoesNotWarn()
{
    TestableGradientBorderItem item;
    GradientColor color(QColor(Qt::red));
    WarningCollector collector;

    item.setBorderColor(&color);

    QVERIFY2(collector.warnings().isEmpty(), qPrintable(collector.warnings().join(QStringLiteral(" | "))));
}

void gradient_border_item_test::componentCompleteWithoutQmlEngineDoesNotWarn()
{
    TestableGradientBorderItem item;
    WarningCollector collector;

    item.componentComplete();

    QVERIFY2(collector.warnings().isEmpty(), qPrintable(collector.warnings().join(QStringLiteral(" | "))));
}

void gradient_border_item_test::specificBorderAnimationStartsAndStops()
{
    TestableGradientBorderItem item;
    QSignalSpy enableSpy(&item, &GradientBorderItem::enableSpecificBorderChanged);

    item.setEnableSpecificBorder(true);

    auto *animation = item.findChild<QPropertyAnimation *>();
    QVERIFY(animation != nullptr);
    QTRY_COMPARE(animation->state(), QAbstractAnimation::Running);
    QVERIFY(item.enableSpecificBorder());
    QCOMPARE(enableSpy.count(), 1);

    item.setEnableSpecificBorder(true);
    QCOMPARE(enableSpy.count(), 1);

    item.setEnableSpecificBorder(false);
    QCOMPARE(animation->state(), QAbstractAnimation::Stopped);
    QVERIFY(!item.enableSpecificBorder());
    QCOMPARE(enableSpy.count(), 2);
}

void gradient_border_item_test::paintUsesCustomPureBorderColor()
{
    TestableGradientBorderItem item;
    GradientColor color(QColor(Qt::red));

    item.setWidth(60);
    item.setHeight(30);
    item.setBorderWidth(4.0);
    item.setBorderColor(&color);

    const QImage image = paintItem(item);
    const QColor pixel = image.pixelColor(30, 2);

    QVERIFY(pixel.alpha() > 0);
    QVERIFY(pixel.red() >= pixel.blue());
}

void gradient_border_item_test::paintUsesCustomGradientBorderColor()
{
    TestableGradientBorderItem item;
    GradientColor color(0, QColor(Qt::red), QColor(Qt::blue), QColor(Qt::white));

    item.setWidth(60);
    item.setHeight(30);
    item.setBorderWidth(4.0);
    item.setBorderColor(&color);

    QVERIFY(color.colorType() == DtColorType::Gradient);

    const QImage image = paintItem(item);

    QVERIFY(image.pixelColor(10, 2).alpha() > 0);
    QVERIFY(image.pixelColor(50, 2).alpha() > 0);
}

void gradient_border_item_test::paintUsesSpecificBorderGradient()
{
    TestableGradientBorderItem item;

    item.setWidth(60);
    item.setHeight(30);
    item.setBorderWidth(4.0);
    item.setEnableSpecificBorder(true);

    const QImage image = paintItem(item);

    QVERIFY(image.pixelColor(30, 2).alpha() > 0);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    gradient_border_item_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-gradient-border-item.moc"
