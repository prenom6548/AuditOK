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
 */
#ifndef TEST_DATAPARSER_H
#define TEST_DATAPARSER_H

#include <QObject>
#include <QtTest/QtTest>
#include "model/dataparser.h"

class TestDataParser : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDataParserConstructor();
    void testDataReplyWithValidData();
    void testDataReplyWithInvalidJson();
    void testDataReplyWithEmptyData();
    void testDataReplyWithoutKylinWeather();
    void testDataReplyWithEmptyWeather();
    void testDataReplyColl();
    void testCurrentWeather();
    void testHourlyWeather();
    void testLifeStyle();
    void testDataReplyL();
    void testDataReplyR();
    void testDataReplyWithIncompleteWeather();
    void testHourlyWeatherInvalidArraySize();
    void testHourlyWeatherWrongKeys();
    void testHourlyWeatherWithEmptyValues();
    void testLifeStyleEmptyObj();
    void testForcastWeatherSevenDays();
    void testDataReplyCollWithInvalidJson();
    void testDataReplyCollWithEmptyData();
    void testDataReplyCollWithEmptyObject();
    void testDataAnalysisWithEmptyData();
    void testDataAnalysisWithEmptyObject();
};

#endif // TEST_DATAPARSER_H
