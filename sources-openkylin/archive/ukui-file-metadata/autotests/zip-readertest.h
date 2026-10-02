/*
 *
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
 */

#ifndef UKUI_FILE_METADATA_ZIPREADERTEST_H
#define UKUI_FILE_METADATA_ZIPREADERTEST_H

#include <QObject>

namespace UkuiFileMetadata {

class ZipReaderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testReadEntryReturnsFullContent();
    void testProcessEntryCanStopEarly();
    void testCp437EntryNamesUseZipDefaultCodec();
    void testDefaultFallbackCodecsDoNotDependOnLocaleLanguage();
    void testUnicodePathExtraFieldTakesPrecedence();
    void testInvalidUnicodePathExtraFieldFallsBackToConfiguredCodec();
    void testUtf8EntryNamesRoundTrip();
    void testLegacyEntryNamesUseFallbackCodecs();
};

}

#endif // UKUI_FILE_METADATA_ZIPREADERTEST_H
