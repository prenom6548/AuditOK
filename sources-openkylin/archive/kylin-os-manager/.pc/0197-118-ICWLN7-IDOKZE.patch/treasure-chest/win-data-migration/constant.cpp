#include "constant.h"

QString Constant::g_toolbarText;
QString Constant::g_okText;
QString Constant::g_cancelText;
QString Constant::g_cancellingText;
QString Constant::g_retryText;
QString Constant::g_migrationErrorText;
QString Constant::g_viewText;
QString Constant::g_readLogErrorText;
QString Constant::g_migrationLogText;
QString Constant::g_howToSetText;
QString Constant::g_howToUseImportantText;
QString Constant::g_winIPText;
QString Constant::g_winWorkGroupText;
QString Constant::g_winUserNameText;
QString Constant::g_winPassWordText;
QString Constant::g_creatConnectText;
QString Constant::g_connectSuccessText;
QString Constant::g_connectingText;
QString Constant::g_connectFailText;
QString Constant::g_chooseFileText;
QString Constant::g_saveAsPathText;
QString Constant::g_startSaveText;
QString Constant::g_loadingeText;
QString Constant::g_chooseFilePathText;
QString Constant::g_howToUseText;
QString Constant::g_migrationSuccessText;
QString Constant::g_creatMigrationLogFailedText;
QString Constant::g_saveFailedText;
QString Constant::g_notChooseText;
QString Constant::g_saveFileFailedText;
QString Constant::g_creatFileFailedText;
QString Constant::g_fileErrorText;
QString Constant::g_smbDisconnectText;
QString Constant::g_getMigrationTreeText;
QString Constant::g_creatListText;
QString Constant::g_statusResetText;

void Constant::getTranslations()
{
    g_toolbarText = QObject::tr("Win Data Migration");                              //"win迁移工具"
    g_okText = QObject::tr("ok");                                                   //"确定"
    g_cancelText = QObject::tr("cancel");                                           //"取消"
    g_cancellingText = QObject::tr("Canceling file migration, please wait ……");   //"取消中"
    g_retryText = QObject::tr("retry");                                             //"重试"
    g_migrationErrorText = QObject::tr("failed to migrate files");                  //"未成功迁移的文件"
    g_viewText = QObject::tr("view");                                               //"查看"
    g_readLogErrorText = QObject::tr("failed to read the migration log:");          //"读取迁移日志失败："
    g_migrationLogText = QObject::tr("migration log");                              //"迁移日志"
    g_howToSetText = QObject::tr("how to set the win");                             //"win端如何设置"
    g_howToUseImportantText = QObject::tr("how to set the win (required reading)"); //"win端如何设置（必读）"
    g_winIPText = QObject::tr("IP address");                                        //"IP地址"
    g_winWorkGroupText = QObject::tr("win work group");                             //"win工作组"
    g_winUserNameText = QObject::tr("win user name");                               //"win用户名"
    g_winPassWordText = QObject::tr("win password");                                //"win密码"
    g_creatConnectText = QObject::tr("establish connection");                       //"建立连接"
    g_connectSuccessText = QObject::tr("successful connection");                    //"连接成功"
    g_connectingText = QObject::tr("connecting...");                                //"正在连接中..."
    g_connectFailText = QObject::tr("connection failed, please try again");         //"连接失败，请重试"
    g_chooseFileText = QObject::tr("select the files you want to migrate");         //"选择要迁移的文件"
    g_saveAsPathText = QObject::tr("file migration to");                            //"文件迁移至"
    g_startSaveText = QObject::tr("start migration");                               //"开始迁移"
    g_loadingeText = QObject::tr("[loading]");                                      //"加载中"
    g_chooseFilePathText = QObject::tr("select migration directory");               //"选择迁移目录"
    /*"<h2>win端设置</h2>"
      "<p>第一步：两台电脑需在同一局域网网络内</p>"
      "<p>第二步：win电脑需将文件夹或文件，设置为共享文件夹</p>"
      "<p>1、选择要共享的文件夹"
      "<br>2、鼠标右键点击属性，并选择共享页签，点击高级共享设置，勾选共享此文件夹，点击应用</p>"*/
    g_howToUseText = QObject::tr("<h2>win Settings </h2>"
                                 "<p> Step 1: Both computers need to be in the same LAN network </p>"
                                 "<p> Step 2: win computer needs to set the folder or file as a shared folder </p>"
                                 "<div>1. Select the folder to share.</div>"
                                 "<div>2. Right click on Properties and select the Sharing tab. Click on Advanced Sharing Settings, check Share this folder, and then click Apply.</div>");
				//  Right click on Properties and select the Sharing tab. Click on Advanced Sharing Settings, check Share this folder, and then click Apply
    g_migrationSuccessText = QObject::tr("migration success");                      //"迁移完成"
    g_creatMigrationLogFailedText = QObject::tr("failed to create migration logs"); //"创建迁移日志失败"
    g_saveFailedText = QObject::tr("files failed to be migrated,network or file error,please check and try again");   //"个文件迁移失败.网络或源文件异常,请检查后重试"
    g_notChooseText = QObject::tr("no migration items are selected");               //"未选中任何迁移项"
    g_saveFileFailedText = QObject::tr("save path is invalid or unauthorized");     //"保存路径不合法或无权限"
    g_creatFileFailedText = QObject::tr("file creation failure");                   //"创建文件失败"
    g_fileErrorText = QObject::tr("file integrity check fails");                    //"文件完整性校验不通过"
    g_smbDisconnectText = QObject::tr("smb disconnected abnormally");               //"smb连接异常中断"
    g_getMigrationTreeText = QObject::tr("loading migration file tree");            //"读取迁移文件树"
    g_creatListText = QObject::tr("creating queue");                                //"创建迁移队列"
    g_statusResetText = QObject::tr("migration status resetting...");               //"迁移状态重置中..."
}
