#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <stdlib.h>
#include <QString>
#include <QLibrary>
#include <QDebug>
#include <QUrl>
#include <QObject>
#include <samba-4.0/libsmbclient.h>

#define QTC(x) (qstringTochar(x))
const char *qstringTochar(const QString &qstr)
{
    QByteArray ba = qstr.toLatin1();
    return ba.constData();
}

const char *g_Workgroup = "WORKGROUP";
const char *g_Username = "kylin";
const char *g_Password = "kk123123";


int connect(char *host, char *share);
int browse(char *path);
int main(int argc, const char *argv[])
{
    
    browse("smb://10.41.120.174");
    connect("10.41.120.174", "print$");
    return 0;
}

static void
get_auth_data_with_context_fn(SMBCCTX * context,
                              const char * pServer,
                              const char * pShare,
                              char * pWorkgroup,
                              int maxLenWorkgroup,
                              char * pUsername,
                              int maxLenUsername,
                              char * pPassword,
                              int maxLenPassword)
{
    printf("Authenticating with context %p", context);
    if (context != NULL) {
        char *user_data = (char *)smbc_getOptionUserData(context);
        printf(" with user data %s", user_data);
    }
    printf("\n");

    strncpy(pWorkgroup, g_Workgroup, maxLenWorkgroup);
    pWorkgroup[maxLenWorkgroup - 1] = '\0';

    strncpy(pUsername, g_Username, maxLenUsername);
    pUsername[maxLenUsername - 1] = '\0';

    strncpy(pPassword, g_Password, maxLenPassword);
    pPassword[maxLenPassword - 1] = '\0';
}

int connect(char *host, char *share)
{

    bool g_smbAuth;

    QString uri = QString("smb://%1/%2").arg(host).arg(share);
    SMBCCTX *ctx = nullptr;
    SMBCFILE *fd = nullptr;
    ctx = smbc_new_context();
    // smbc_setDebug(ctx, 10);
    errno = 0;
    smbc_setFunctionAuthDataWithContext(ctx,
                                        get_auth_data_with_context_fn);
    // smbc_setOptionUseKerberos(ctx, true);
    if (!smbc_init_context(ctx)) {
        smbc_free_context(ctx, 0);
        printf("Could not initialize smbc context\n");
        return 1;
    }

    printf("%d\n", errno);
    fd = smbc_getFunctionOpen (ctx)(ctx, qstringTochar(uri), O_RDWR, 511);
    printf("%d\n", errno);
    printf("%s\n", strerror(errno));
    if (!fd) {
        printf("--->Could not open dir, auth error!\n");
        return 1;
    }
    return 0;
}


int browse(char *path) {
    SMBCCTX *context;

    /* Allocate a new context */
    context = smbc_new_context();
    if (!context) {
        printf("Could not allocate new smbc context\n");
        return 1;
    }

    /* Initialize the context using the previously specified options */
    if (!smbc_init_context(context)) {
        smbc_free_context(context, 0);
        printf("Could not initialize smbc context\n");
        return 1;
    }
    smbc_setFunctionAuthDataWithContext(context,
                                        get_auth_data_with_context_fn);

    /* Tell the compatibility layer to use this context */
    smbc_set_context(context);

    struct smbc_dirent * dirent;
    int dir;
    if ((dir = smbc_opendir(path)) < 0)
    {
        printf("Could not open directory [%s] (%d:%s)\n",
               path, errno, strerror(errno));
        smbc_free_context(context, 1);
        return 1;
    }

    while ((dirent = smbc_readdir(dir)) != NULL)
    {
        switch(dirent->smbc_type)
        {
            case SMBC_PRINTER_SHARE:
                printf("%s\n", dirent->name);
                break;
            default:
                break;
        }
    }

    smbc_closedir(dir);
    smbc_free_context(context, 1);
    return 0;
}
