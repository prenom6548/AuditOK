/*
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */
#include <stdio.h>
#include <string.h>

#include <gtest/gtest.h>
#include "../cpp_stub/src/stub.h"
extern "C"
{
#include "../../log/libkylog.h"
#include "../../log/core.h"
#include "../../log/klog_mqueue.h"
}

TEST(TestSuite1, kylog_1)
{
	assert(kdk_logger_init("logtest.conf") == 0);

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
	
	emptyMessageQueue();

	destroyKLogger();
}

TEST(TestSuite2, kylog_2)
{   
	assert(kdk_logger_init(NULL) == 0);
	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
}

TEST(TestSuite3, kylog_autowrap)
{
	// assert(kdk_logger_init("../log/logtest.conf") == 0);

    kdk_logger_set_autowrap(1);

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
}

TEST(TestSuite4, kylog_setdir)
{
	// assert(kdk_logger_init("../log/logtest.conf") == 0);
	assert(kdk_logger_setdir("/tmp/kylog") == 0);

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
}

TEST(TestSuite5, levelBasedStorage)
{
	logger->levelBasedStorage = 1;

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
	logger->levelBasedStorage = 0;
}

TEST(TestSuite6, async)
{
	logger->stype = ASYNC;
	// assert(kdk_logger_init(NULL) == 0);

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
	logger->stype = SYNC;
}

TEST(TestSuite7, otype_stdout)
{
	logger->otype = OUT_STDOUT;

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
	logger->otype = OUT_SPECFILE;
}

TEST(TestSuite8, otype_syslog)
{
	logger->otype = OUT_SYSLOG;

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
	logger->otype = OUT_SPECFILE;
}

TEST(TestSuite9, commonfp_null)
{
	FILE *fp = logger->fp.commonfp;
	logger->fp.commonfp = NULL;

	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");
	logger->fp.commonfp = fp;
}

