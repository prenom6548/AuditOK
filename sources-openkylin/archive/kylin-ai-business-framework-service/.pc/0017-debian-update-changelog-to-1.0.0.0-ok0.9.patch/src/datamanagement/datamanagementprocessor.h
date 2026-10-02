/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATAMANAGEMENTPROCESSOR_H
#define DATAMANAGEMENTPROCESSOR_H

#include "datamanagement/datamanagementprocessorglue.h"

#include <string>

class DataManagementProcessor
{
public:
    explicit DataManagementProcessor(GDBusConnection &connection);
    ~DataManagementProcessor();

private:
    void exportSkeleton();
    void unexportSkeleton();
    void connectSignal();

    static bool handleSimilaritySearch(AisdkDataManagementProcessor *delegate,
                                       GDBusMethodInvocation *invocation,
                                       gchar *searchConditions,
                                       int uid,
                                       gpointer userData);
    static bool handleAddTextFiles(AisdkDataManagementProcessor *delegate,
                                   GDBusMethodInvocation *invocation,
                                   gchar *fileinfos,
                                   int uid,
                                   gpointer userData);
    static bool handleAddImageFiles(AisdkDataManagementProcessor *delegate,
                                    GDBusMethodInvocation *invocation,
                                    gchar *fileinfos,
                                    int uid,
                                    gpointer userData);
    static bool handleDeleteFiles(AisdkDataManagementProcessor *delegate,
                                  GDBusMethodInvocation *invocation,
                                  gchar *fileinfos,
                                  int uid,
                                  gpointer userData);
    static bool handleUpdateFilesName(AisdkDataManagementProcessor *delegate,
                                      GDBusMethodInvocation *invocation,
                                      gchar *fileinfos,
                                      int uid,
                                      gpointer userData);
    static bool handleUpdateFilesContent(AisdkDataManagementProcessor *delegate,
                                         GDBusMethodInvocation *invocation,
                                         gchar *fileinfos,
                                         int uid,
                                         gpointer userData);
    static bool handleGetAllFileinfos(AisdkDataManagementProcessor *delegate,
                                      GDBusMethodInvocation *invocation,
                                      int uid,
                                      gpointer userData);

    AisdkDataManagementProcessor *delegate_ = nullptr;

private:
    bool isExported_ = false;
    GDBusConnection &connection_;
    static const std::string objectPath_;
};

#endif // DATAMANAGEMENTPROCESSOR_H
