#ifndef CONSTANT_H
#define CONSTANT_H
#include <QObject>

class Constant
{
public:
    static void getTranslations();
    static QString g_toolbarText;
    static QString g_okText;
    static QString g_cancelText;
    static QString g_cancellingText;
    static QString g_retryText;
    static QString g_migrationErrorText;
    static QString g_viewText;
    static QString g_readLogErrorText;
    static QString g_migrationLogText;
    static QString g_howToSetText;
    static QString g_howToUseImportantText;
    static QString g_winIPText;
    static QString g_winWorkGroupText;
    static QString g_winUserNameText;
    static QString g_winPassWordText;
    static QString g_creatConnectText;
    static QString g_connectSuccessText;
    static QString g_connectingText;
    static QString g_connectFailText;
    static QString g_chooseFileText;
    static QString g_saveAsPathText;
    static QString g_startSaveText;
    static QString g_loadingeText;
    static QString g_chooseFilePathText;
    static QString g_howToUseText;
    static QString g_migrationSuccessText;
    static QString g_creatMigrationLogFailedText;
    static QString g_saveFailedText;
    static QString g_notChooseText;
    static QString g_saveFileFailedText;
    static QString g_creatFileFailedText;
    static QString g_fileErrorText;
    static QString g_smbDisconnectText;
    static QString g_getMigrationTreeText;
};

#define SMB_CONNECT_SUCCESS 1  //连接成功
#define SMB_START_SAVE 2       //开始迁移
#define SMB_LOAD_URLS_START 3  //开始加载子节点
#define SMB_LOAD_URLS_FINISH 4 //加载子节点完成
#define SMB_CREAT_LOG_FINISH 5 //创建日志完成
#define SMB_ERROR_1 -1         // smb初始化失败
#define SMB_ERROR_2 -2         // smb连接失败
#define SMB_ERROR_3 -3         //未选中任何迁移项
#define SMB_ERROR_4 -4         //保存路径不合法或无权限
#define SMB_ERROR_5 -5         //文件下载失败
#define SMB_ERROR_6 -6         //创建迁移日志失败
#define SMB_ERROR_7 -7         // smb连接异常断开

#endif // CONSTANT_H
