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
