/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest>
#include <QGuiApplication>

#include "voiceInput.h"

class TestVoiceInput : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void voiceInputInstanceValid();
    void voiceInputInitializesModules();
};

void TestVoiceInput::voiceInputInstanceValid()
{
    QVERIFY(VoiceInput::instance() != nullptr);
}

void TestVoiceInput::voiceInputInitializesModules()
{
    QCOMPARE(VoiceInput::instance(), VoiceInput::instance());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    TestVoiceInput test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_voice_input.moc"
