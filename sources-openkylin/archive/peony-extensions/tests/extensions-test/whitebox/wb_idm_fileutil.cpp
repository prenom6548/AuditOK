/*
 * 白盒：FileInfoJob 静态路径、FileEventHandlerImpl 远程入口。
 */
#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>

#include "file/fileinfojob.h"
#include "file/fileinfo.h"
#include "file/qtremoteobject/fileeventhandlerimpl.h"

using namespace UKUI::IDM;

TEST(IdmFileUtilWhitebox, file_info_job_static_match_empty_conditions)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("m.txt"));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("z");
    f.close();

    const QFileInfo qfi(path);
    EXPECT_TRUE(FileInfoJob::infoMatchCondtions(qfi, {}, {}));

    auto info = FileInfo::fromPath(path, false);
    ASSERT_FALSE(info.isNull());
    EXPECT_TRUE(FileInfoJob::queryInfoSyncWithConditions(info, {}, {}));
}

TEST(IdmFileUtilWhitebox, file_event_handler_impl_handle_event)
{
    FileEventHandlerImpl impl;
    impl.handleFileEvent(0, QStringLiteral("/a"), QStringLiteral("/b"));
    impl.handleFileEvent(101, QStringLiteral("x"), QString());
}
