/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../peony-print-pictures/print-pictures-plugin.h"

class PeonyPrintPicturesInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pPeonyPrintPicturesInterface = new Peony::PrintPicturesPlugin();
    }

    static void TearDownTestSuite()
    {
        if (m_pPeonyPrintPicturesInterface) {
            delete m_pPeonyPrintPicturesInterface;
            m_pPeonyPrintPicturesInterface = nullptr;
        }
    }

    static Peony::PrintPicturesPlugin *m_pPeonyPrintPicturesInterface;
};

Peony::PrintPicturesPlugin *PeonyPrintPicturesInterfaceTest::m_pPeonyPrintPicturesInterface = nullptr;

TEST_F(PeonyPrintPicturesInterfaceTest, aaa)
{
    m_pPeonyPrintPicturesInterface->pluginType();
    m_pPeonyPrintPicturesInterface->name();
    m_pPeonyPrintPicturesInterface->description();
    m_pPeonyPrintPicturesInterface->icon();
    m_pPeonyPrintPicturesInterface->setEnable(false);
    m_pPeonyPrintPicturesInterface->setEnable(true);
    m_pPeonyPrintPicturesInterface->isEnable();
}

TEST_F(PeonyPrintPicturesInterfaceTest, bbb)
{
    QString uri = "file://home/kylin/peony-print-pictures/tests/testfile.jpg";
    QStringList uris;
    uris<< uri;

    m_pPeonyPrintPicturesInterface->menuActions(Peony::MenuPluginInterface::SideBar, uri, uris);
    m_pPeonyPrintPicturesInterface->doPrintAction();

    m_pPeonyPrintPicturesInterface->menuActions(Peony::MenuPluginInterface::DirectoryView, uri, uris);
    m_pPeonyPrintPicturesInterface->doPrintAction();

}

TEST_F(PeonyPrintPicturesInterfaceTest, ccc)
{
    QString uri = "/home/kylin/peony-print-pictures/tests/testfile.jpg";
    QStringList uris;
    uris<< uri;

    m_pPeonyPrintPicturesInterface->setSelectionUris(uris);
    m_pPeonyPrintPicturesInterface->doPrintAction();
}

TEST_F(PeonyPrintPicturesInterfaceTest, ddd)
{
    CustomLog log("ddd");
    log.writeLog(LogType::INFO, "ddd");
    log.writeLog(LogType::DEBUG, "ddd");
    log.writeLog(LogType::WARNING, "ddd");
    log.writeLog(LogType::CRITICAL, "ddd");
    log.writeLog(LogType::FATAL, "ddd");
}
