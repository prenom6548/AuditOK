/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * 
**/
#define _GNU_SOURCE
#define PAM_SM_AUTH
#include "generic.h"
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <syslog.h>
#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <gio/gio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <security/pam_ext.h>
#include <signal.h>
#include <errno.h>
#include <glib.h>
#include <string.h>

#define USER_CONFIG_FILE "/home/%s/.biometric_auth/ukui_biometric.conf"

#define PAM_CONFIG_FILE "/etc/pam.d/common-auth"
#define PAM_MODULE_NAME "pam_pwdless.so"
#define PWDLESS_CONFIG_FILE "/etc/pwdless/default.conf"
#define GLOBAL_GROUP "global"
#define MFA_AUTH_KEY "mfaAuth"
#define MFA_GROUP "mfa"
#define METHOD_KEY "method"

/* Declare log function */
extern int pam_enable_debug;
extern char *pam_log_prefix;
extern int pam_logger(char *format, ...);


static int ukui_biometric_lock = 0;
int enable_biometric_authentication(pam_handle_t *pamh);
int enable_qrcode_authentication(pam_handle_t *pamh);
int enable_biometric_auth_double();
/* GUI child process alive status */
static int child_alive = 1;
/* Signal handler */
static void signal_handler(int signo)
{
    if (signo == SIGUSR1)
    	child_alive = 0; /* GUI child process has terminated */
    pam_logger("signal_handler is triggered\n");
}

// 检查文件中是否包含指定字符串
static gboolean check_file_contains(const gchar *filename, const gchar *search_str) {
    gchar *content;
    gsize length;
    gboolean result = FALSE;

    if (!g_file_get_contents(filename, &content, &length, NULL)) {
        g_free(content);
        return FALSE;
    }

    if (g_strstr_len(content, length, search_str) != NULL) {
        result = TRUE;
    }

    g_free(content);
    return result;
}

// 检查配置文件中mfaAuth为true且method不为空
static gboolean check_mfa_config(const gchar *config_file) {
    GKeyFile *key_file;
    GError *error = NULL;
    gboolean mfa_auth;
    const gchar *method;
    gboolean result = FALSE;

    key_file = g_key_file_new();

    if (!g_key_file_load_from_file(key_file, config_file, G_KEY_FILE_NONE, &error)) {
        g_error_free(error);
        g_key_file_free(key_file);
        return FALSE;
    }

    // 检查global组下mfaAuth是否为true
    if (g_key_file_has_key(key_file, GLOBAL_GROUP, MFA_AUTH_KEY, NULL)) {
        mfa_auth = g_key_file_get_boolean(key_file, GLOBAL_GROUP, MFA_AUTH_KEY, &error);
        if (error != NULL) {
            g_error_free(error);
            g_key_file_free(key_file);
            return FALSE;
        }

        // 仅当mfaAuth为true时，检查method
        if (mfa_auth) {
            if (g_key_file_has_key(key_file, MFA_GROUP, METHOD_KEY, NULL)) {
                method = g_key_file_get_string(key_file, MFA_GROUP, METHOD_KEY, &error);
                if (error != NULL) {
                    g_error_free(error);
                    g_key_file_free(key_file);
                    return FALSE;
                }

                // 检查method不为空
                if (method != NULL && *method != '\0') {
                    result = TRUE;
                }
            }
        }
    }

    g_key_file_free(key_file);
    return result;
}

// 检查系统是否支持多因子认证
gboolean is_mfa_supported(void) {
    // 第一步：检查PAM配置文件
    if (!check_file_contains(PAM_CONFIG_FILE, PAM_MODULE_NAME)) {
        return FALSE;
    }

    // 第二步：检查配置文件（mfaAuth为true且method不为空）
    return check_mfa_config(PWDLESS_CONFIG_FILE);
}

int enable_biometric_authentication_app()
{
    char conf_file[] = GET_STR(CONFIG_FILE);
    FILE *file;
    char line[1024];
    int i;
    int is_enable = 0;

    if((file = fopen(conf_file, "r")) == NULL){
        pam_logger("open configure file failed: %s\n", strerror(errno));
        return 1;
    }

    while(fgets(line, sizeof(line), file)) {
        i = sscanf(line, "EnableAuthApp=%d\n", &is_enable);
        if(i > 0) {
            pam_logger("EnableAuthApp=%d\n", is_enable);
            break;
        }
    }
    fclose(file);
    return is_enable;
}

/*
 * Check if the service should use biometric authentication
 */
int service_filter(char *service)
{
    //int is_enable = enable_biometric_authentication_app();

    //syslog(LOG_INFO,"is_enable = %d service = %s\n",is_enable,service);
    if (strcmp(service, "lightdm") == 0) {
    	//if(is_enable & 1 == 0)
        //	return 0;
        return 1;
    }
    if (strcmp(service, "ukui-screensaver-qt") == 0){
	//if((is_enable & (1<<1)) == 0)
	//	return 0;
    	return 1;
    }
    if (strcmp(service, "polkit-1") == 0){
	//if((is_enable & (1<<2)) == 0)
	//	return 0;
    	return 1;
    }
    if (strcmp(service, "sudo") == 0 || strcmp(service, "sudo-i") == 0){
	//if((is_enable & (1<<3)) == 0)
	//	return 0;
        return 1;
    }
    if (strcmp(service, "su") == 0){
	//if((is_enable & (1<<4)) == 0)
	//	return 0;
    	return 1;
    }
    if (strcmp(service, "login") == 0){
	//if((is_enable & (1<<5)) == 0)
	//	return 0;
    	return 1;
    }
#ifdef ENABLE_BIOTEST
    if (strcmp(service, "biotest") == 0)
    	return 1;
#endif
    return 0;
}

/*
 * Invoke the PAM conversation function
 */
int call_conversation(pam_handle_t *pamh, int msg_style, char *msg, char *resp, int resp_length)
{
    /* PAM data structures used by conversation */
    const struct pam_message *message[1] = {0};
    struct pam_message *message_tmp = 0;
    struct pam_response *response = 0;
    const struct pam_conv *conv_struct = 0;
    int status = -1;

    status = pam_get_item(pamh, PAM_CONV, (const void **)&conv_struct);
    if (status != PAM_SUCCESS)
    	return PAM_SYSTEM_ERR;

    message_tmp = (struct pam_message *)malloc(sizeof(struct pam_message));

    message_tmp->msg_style = msg_style;
    message_tmp->msg = msg;
    message[0] = message_tmp;
    pam_logger("Call conv callback function\n");
    status = conv_struct->conv(1, message, &response, conv_struct->appdata_ptr);
    pam_logger("Finish conv callback function\n");

    if (resp && response && response->resp && resp_length > 1) {
        snprintf(resp, resp_length - 1, "%s", response->resp); 
    }
    /* Use typecast to suppress gcc warnings */
    if (message[0]) {
        free((void *)message[0]);
    }
    if (response) {
        if (response->resp) {
            free(response->resp);
        }
        free(response);
    }

    return status;
}

/* GUI child process */
void child(char *service, char *username, char *xdisp)
{
    pam_logger("Child process will be replaced.\n");
    int fd = open("/dev/null", O_WRONLY);
    dup2(fd, 2);

    execl("/usr/bin/bioauth", "bioauth",
          "--service", service,
          "--user", username,
    //      "--display", xdisp,
          pam_enable_debug ? "--debug" : "",
          (char *)0);
    /*
     * execl almost always succeed as long as the GUI executable file exists.
     * Though invoking GUI under console will exit with error, the GUI child
     * process won't reach here. Therefore, the following code won't be
     * executed in general.
     */
    pam_logger("Fatal error: execl(gui) failed in child process. "
    	"This is an extremely rare condition. Please ensure that the "
        "biometric-authentication executable file exists.\n");
    pam_logger("Use password as a fallback\n");
    pam_logger("Child _exit with BIO_IGNORE\n");
    /* Child process exits */
    _exit(BIO_IGNORE);
}

void handler(int signo)
{
    (void)signo;
    return;
}
/* PAM parent process */
int parent(int pid, pam_handle_t *pamh, int need_call_conv)
{
    pam_logger("Parent process continue running.\n");
    int child_status = -1;
    /*
     * 1. If calling conversation function is not needed, wait the child
     * until it exits.
     * 2. Otherwise, send a text info to application at first and then
     * call the conversation function to request the password. The returned
     * password is unused.
     * Note: During requesting the password, screensaver won't display the
     * prompting message, while greeter will display it into the password
     * entry. If there is a "\n" at the end of the message, it will be
     * displayed on the Label above the password entry.
     */
    if (need_call_conv) {
    	char *lang = getenv("LANG");
    	char *msg1, *msg2;
    	if (lang && !strncmp(lang, "zh_CN", 5))
    		msg1 = "请进行生物识别或点击“切换到密码登录”\n";
    	else
            msg1 = "Use biometric authentication or click "
    					"\"Switch to password\"\n";
    	#ifdef EMPTY_PAM_PWD_PROMPT
    		msg2 = "";
    	#else
    		msg2 = "pam_biometric.so needs a fake ENTER:";
    	#endif

    	if (signal(SIGUSR1, signal_handler) == SIG_ERR)
            pam_logger("Fatal Error. Can't catch SIGUSR1\n");
    reinvoke:
                call_conversation(pamh, PAM_TEXT_INFO, msg1, NULL, 0);
                call_conversation(pamh, PAM_PROMPT_ECHO_OFF, msg2, NULL, 0);
    	/* GUI child process is still alive. This enter is typed by user. */
    	if (child_alive == 1)
    		goto reinvoke;
    	signal(SIGUSR1, SIG_DFL);
    	waitpid(pid, &child_status, 0);
    } else {
        pam_logger("Waiting for the GUI child process to exit...\n");
    	//由于sudo命令在进入pam认证时，会阻塞来自终端的SIGINT以及SIGQUIT信号，导致使用
	//pam认证时，按下Ctrl+C无反应，认证完成后，sudo会退出，这里为了简单，取消了阻塞
	//信号，捕获信号但不做处理，在认证完成后，恢复原本阻塞状态
    	sigset_t mask;
        sigprocmask(SIG_BLOCK,NULL,&mask);
        sigprocmask(SIG_UNBLOCK,&mask,NULL);

        signal(SIGINT,handler);
    
	waitpid(pid, &child_status, 0);
        pam_logger("GUI child process has exited.\n");
        sigprocmask(SIG_SETMASK,&mask,NULL);
    }

    /*
     * Determine the return value of PAM according to the return value of
     * child process.
     */
    int bio_result = BIO_ERROR; /* biometric result code */
    if (WIFEXITED(child_status))
    	bio_result = WEXITSTATUS(child_status);
    else /* This may be because the GUI child process is invoked under console. */
        pam_logger("The GUI-Child process terminate abnormally.\n");

    if (bio_result == BIO_SUCCESS) {
        if(!enable_biometric_authentication(pamh) && !enable_qrcode_authentication(pamh)) {
                pam_logger("disable biometric authentication.\n");
                return PAM_SYSTEM_ERR;
        }
        pam_logger("pam_biometric.so return PAM_SUCCESS\n");
    	return PAM_SUCCESS;
    } else if (bio_result == BIO_IGNORE) {
    	/* Override msg1 to empty the label. We are ready to enter the password module. */
        call_conversation(pamh, PAM_TEXT_INFO, "", NULL, 0);
	ukui_biometric_lock = 1;
        pam_logger("pam_biometric.so return PAM_IGNORE\n");
    	return PAM_IGNORE;
    } else {
        pam_logger("pam_biometric.so return PAM_SYSTEM_ERR\n");
	ukui_biometric_lock = 1;
    	return PAM_SYSTEM_ERR;
    }
}

/* Set environment variables related to displaying */
void check_and_set_env(pam_handle_t *pamh, char **xdisp, char **xauth)
{
    *xdisp=getenv("DISPLAY");
    *xauth=getenv("XAUTHORITY");

    if (*xdisp == 0){
    	pam_get_item(pamh, PAM_XDISPLAY, (const void **)xdisp);
    	if (*xdisp)
    		setenv("DISPLAY",*xdisp,-1);
    }
    if (*xauth == 0)
    	setenv("XAUTHORITY", "/var/run/lightdm/root/:0", -1);

    *xdisp=getenv("DISPLAY");
    *xauth=getenv("XAUTHORITY");
    if (*xdisp == 0)
        pam_logger("Warning: DISPLAY env is still empty, "
    		"this is not an error if you are using terminal\n");
    if (*xauth == 0)
        pam_logger("Warning: XAUTHORITY env is still empty, "
    		"this is not an error if you are using terminal\n");

}

/* Biometric processing function for generic purpose */
int biometric_auth_independent(pam_handle_t *pamh , char *service, int need_call_conv)
{
    /* Get the username */
    char *username = 0;
    pam_get_item(pamh, PAM_USER, (const void **)&username);

    /* Check and set environment variables */
    char *xdisp, *xauth;
    check_and_set_env(pamh, &xdisp, &xauth);

    /* Detach child process */
    unsigned int pid;
    pid = fork();
    if (pid < 0) {
        pam_logger("Fork Error!\n");
        return PAM_SYSTEM_ERR;
    } else if (pid != 0) {
        return parent(pid, pamh, need_call_conv);
    } else {
        child(service, username, xdisp);
        pam_logger("Should never reach here.\n");
        return PAM_SYSTEM_ERR;
    }
}

int biometric_auth_embeded(pam_handle_t *pamh)
{
    /*
     * By convention, PAM module sends a string "BIOMETRIC_PAM" to
     * lightdm and this message will be forwarded to greeter. After
     * the authentication is completed, greeter will send a string
     * "BIOMETRIC_IGNORE"/"BIOMETRIC_SUCCESS" to lightdm and then it
     * will be forwarded to PAM module. We can get the authentication
     * status by comparing strings.
     */
    char resp[96] = {0};
    if(enable_biometric_auth_double())
    	call_conversation(pamh, PAM_PROMPT_ECHO_OFF, BIOMETRIC_PAM_DOUBLE, resp, sizeof(resp));
    else
	call_conversation(pamh, PAM_PROMPT_ECHO_OFF, BIOMETRIC_PAM, resp, sizeof(resp));

    if (strcmp(resp, BIOMETRIC_IGNORE) == 0)
        return PAM_IGNORE;
    else if (strcmp(resp, BIOMETRIC_SUCCESS) == 0){
	    if(!enable_biometric_authentication(pamh) && !enable_qrcode_authentication(pamh)) {
            pam_logger("disable biometric authentication.\n");
            return PAM_SYSTEM_ERR;
        }
        return PAM_SUCCESS;
    }
    else if (strcmp(resp, BIOMETRIC_FAILED) == 0)
        return PAM_AUTH_ERR;
    else
        return PAM_SYSTEM_ERR;
}

void get_greeter_session(char buf[], int len)
{
    GPtrArray *args_ps = NULL;
    gchar *stdout_ps = NULL;
    gchar *greeter_name = NULL;

    args_ps = g_ptr_array_new ();
    g_ptr_array_add(args_ps, (gpointer)"/usr/bin/ps");
    g_ptr_array_add(args_ps, (gpointer)"-aux");
    g_ptr_array_add(args_ps, (gpointer)"--columns");
    g_ptr_array_add(args_ps, (gpointer)"256");
    g_ptr_array_add(args_ps, NULL);

    if (!g_spawn_sync(NULL, (char**)args_ps->pdata, NULL, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &stdout_ps, NULL, NULL, NULL)) {
        if (stdout_ps)
            g_free(stdout_ps);
        g_ptr_array_free (args_ps, TRUE);
        return ;
    }
    if (stdout_ps) {
        gchar **lines = NULL;
        lines = g_strsplit(stdout_ps, "\n", 0);
        for (int n = 0; lines[n] != NULL; n++) {
            if (lines[n][0] == '\0')
                continue;
            gchar *res = g_strstr_len(lines[n], 1024, "greeter-session");
            if (res) {
                gchar **greeter_args = g_strsplit(res, " ", 0);
                int arg_index = 0;
                for (int m = 0; greeter_args[m] != NULL; m++) {
                    if (greeter_args[m][0] == '\0')
                        continue;
                    // greeter-session arg1
                    if (arg_index == 1) {
                        gchar *greeter = g_strrstr_len(greeter_args[m], 256, "/");
                        if (greeter && strnlen(greeter,1024) > 1) {
                            greeter_name = g_strdup(greeter + 1);
                            break;
                        }
                    }
                    arg_index ++;
                }
                g_strfreev(greeter_args);
                if (greeter_name) {
                    break;
                }
            }
        }
        g_strfreev(lines);
        g_free(stdout_ps);
    }
    g_ptr_array_free (args_ps, TRUE);
    if (greeter_name) {
        g_strlcpy(buf, greeter_name, len);
        g_free(greeter_name);
    } else {
        GPtrArray *args_pidof = NULL;
        gchar *stdout_pidof = NULL;

        args_pidof = g_ptr_array_new ();
        g_ptr_array_add(args_pidof, (gpointer)"/usr/bin/pidof");
        g_ptr_array_add(args_pidof, (gpointer)"ukui-greeter");
        g_ptr_array_add(args_pidof, NULL);

        if (!g_spawn_sync(NULL, (char**)args_pidof->pdata, NULL, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &stdout_pidof, NULL, NULL, NULL)) {
            if (stdout_pidof)
                g_free(stdout_pidof);
            g_ptr_array_free (args_pidof, TRUE);

		//如果ukui-greeter进程不存在，进而判断ukui-screensaver-backend进程
	    args_pidof = g_ptr_array_new ();
            g_ptr_array_add(args_pidof, (gpointer)"/usr/bin/pidof");
            g_ptr_array_add(args_pidof, (gpointer)"ukui-screensaver-backend");
            g_ptr_array_add(args_pidof, NULL);

	    if (!g_spawn_sync(NULL, (char**)args_pidof->pdata, NULL, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &stdout_pidof, NULL, NULL, NULL)) {
                if (stdout_pidof) {
                    g_free(stdout_pidof);
                    stdout_pidof = NULL;
                }
                g_ptr_array_free (args_pidof, TRUE);
	    }
	    
            if (stdout_pidof) {
                if (strnlen(stdout_pidof,1024) > 0 && atoi(stdout_pidof) > 0) {
                    g_strlcpy(buf, "ukui-screensaver-backend", len);
                }
                g_free(stdout_pidof);
            }

	    g_ptr_array_free (args_pidof, TRUE);

            return ;
        }
        if (stdout_pidof) {
            if (strnlen(stdout_pidof,1024) > 0 && atoi(stdout_pidof) > 0) {
                g_strlcpy(buf, "ukui-greeter", len);
            }
            g_free(stdout_pidof);
        }
        g_ptr_array_free (args_pidof, TRUE);
    }
}

#define UKUI_POLKIT_AGENT_BIN "polkit-ukui-authentication-agent-1"

static const char *exe_basename(const char *path)
{
    const char *base = strrchr(path, '/');

    return base ? base + 1 : path;
}

static int is_ukui_polkit_agent_pid(pid_t pid)
{
    char path[64];
    char exe[512];
    ssize_t exe_len;
    char *deleted;

    if (pid <= 1)
        return 0;

    snprintf(path, sizeof(path), "/proc/%d/exe", (int)pid);
    exe_len = readlink(path, exe, sizeof(exe) - 1);
    if (exe_len <= 0) {
        pam_logger("readlink %s failed: %s\n", path, strerror(errno));
        return 0;
    }
    exe[exe_len] = '\0';

    deleted = strstr(exe, " (deleted)");
    if (deleted)
        *deleted = '\0';

    pam_logger("polkit agent candidate pid=%d exe=%s\n", (int)pid, exe);
    return strcmp(exe_basename(exe), UKUI_POLKIT_AGENT_BIN) == 0;
}

/*
 * Decide whether BIOMETRIC_PAM should be sent for polkit-1.
 * Only ukui-polkit should receive it; other agents must be ignored.
 *
 * polkit >=127 uses a socket-activated helper: stdin is the AF_UNIX
 * connection from the agent, so SO_PEERCRED gives the agent pid.
 * Legacy setuid helper is forked by the agent, so PPID is the agent.
 */
int enable_by_polkit()
{
    struct ucred cred;
    socklen_t len = sizeof(cred);

    memset(&cred, 0, sizeof(cred));
    if (getsockopt(STDIN_FILENO, SOL_SOCKET, SO_PEERCRED, &cred, &len) == 0
            && cred.pid > 1) {
        pam_logger("SO_PEERCRED pid=%d uid=%d\n", (int)cred.pid, (int)cred.uid);
        if (is_ukui_polkit_agent_pid(cred.pid))
            return 1;
    } else {
        pam_logger("SO_PEERCRED unavailable: %s\n", strerror(errno));
    }

    pam_logger("fallback to PPID=%d\n", (int)getppid());
    if (is_ukui_polkit_agent_pid(getppid()))
        return 1;

    return 0;
}

int enable_biometric_authentication(pam_handle_t *pamh)
{
    char *username = NULL;
    int is_found = 0;
    int is_auth_enable = 0;
    pam_get_item(pamh, PAM_USER, (const void **)&username);
    if (username) {
        char conf_file_user[256];
        snprintf(conf_file_user, 255, USER_CONFIG_FILE, username);
        FILE *file = NULL;
        char line[1024], is_enable[16];
        int i;
        if((file = fopen(conf_file_user, "r")) == NULL){
            pam_logger("open configure file failed: %s\n", strerror(errno));
        } else {
            while(fgets(line, sizeof(line), file)) {
                i = sscanf(line, "EnableAuth=%15s\n", is_enable);
                if(i > 0) {
                    pam_logger("EnableAuth=%s\n", is_enable);
                    is_found = 1;
                    break;
                }
            }
            fclose(file);
            if(!strcmp(is_enable, "true"))
                is_auth_enable = 1;
        }
    }
    if (is_found != 0) {
        return is_auth_enable;
    }

    char conf_file[] = GET_STR(CONFIG_FILE);
    FILE *file;
    char line[1024], is_enable[16];
    int i;

    if((file = fopen(conf_file, "r")) == NULL){
        pam_logger("open configure file failed: %s\n", strerror(errno));
        return 0;
    }
    while(fgets(line, sizeof(line), file)) {
        i = sscanf(line, "EnableAuth=%15s\n",  is_enable);
        if(i > 0) {
            pam_logger("EnableAuth=%s\n", is_enable);
            break;
        }
    }
    
    fclose(file);
    if(!strcmp(is_enable, "true"))
        return 1;
    return 0;
}

int enable_qrcode_authentication(pam_handle_t *pamh)
{
    char *username = NULL;
    int is_found = 0;
    int is_auth_enable = 0;
    pam_get_item(pamh, PAM_USER, (const void **)&username);
    if (username) {
        char conf_file_user[256];
        snprintf(conf_file_user, 255, USER_CONFIG_FILE, username);
        FILE *file = NULL;
        char line[1024], is_enable[16];
        int i;
        if((file = fopen(conf_file_user, "r")) == NULL){
            pam_logger("open configure file failed: %s\n", strerror(errno));
        } else {
            while(fgets(line, sizeof(line), file)) {
                i = sscanf(line, "EnableQRCode=%15s\n", is_enable);
                if(i > 0) {
                    pam_logger("EnableQRCode=%s\n", is_enable);
                    is_found = 1;
                    break;
                }
            }
            fclose(file);
            if(!strcmp(is_enable, "true"))
                is_auth_enable = 1;
        }
    }
    if (is_found != 0) {
        return is_auth_enable;
    }

    char conf_file[] = GET_STR(CONFIG_FILE);
    FILE *file;
    char line[1024], is_enable[16];
    int i;

    if((file = fopen(conf_file, "r")) == NULL){
        pam_logger("open configure file failed: %s\n", strerror(errno));
        return 0;
    }
    while(fgets(line, sizeof(line), file)) {
        i = sscanf(line, "EnableQRCode=%15s\n",  is_enable);
        if(i > 0) {
            pam_logger("EnableQRCode=%s\n", is_enable);
            break;
        }
    }
    
    fclose(file);
    if(!strcmp(is_enable, "true"))
        return 1;
    return 0;
}

int enable_biometric_auth_double()
{
    char conf_file[] = GET_STR(CONFIG_FILE);
    FILE *file;
    char line[1024], is_enable[16];
    int i;


    if((file = fopen(conf_file, "r")) == NULL){
        pam_logger("open configure file failed: %s\n", strerror(errno));
        return 0;
    }
    while(fgets(line, sizeof(line), file)) {
        i = sscanf(line, "DoubleAuth=%s\n",  is_enable);
        if(i > 0) {
            pam_logger("DoubleAuth=%s\n", is_enable);
            break;
        }
    }

    fclose(file);
    if(!strcmp(is_enable, "true"))
        return 1;
    return 0;
}

/*
 * SPI interface
 */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc,
        const char **argv)
{
    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "debug") == 0) {
            pam_enable_debug = 1;
            pam_log_prefix = "PAM_BIO";
        }
    }
    
    pam_logger("Invoke libpam_biometric.so module\n");

    char *service = 0;

    gboolean use_mfa = is_mfa_supported();

    if((!enable_biometric_authentication(pamh) && !enable_qrcode_authentication(pamh)) || ukui_biometric_lock || use_mfa) {
        pam_logger("disable biometric authentication.\n");
    	return PAM_IGNORE;
    }
    pam_logger("enable biometric authentication.\n");

    pam_get_item(pamh, PAM_SERVICE, (const void **)&service);

    /* Service filter */
    if (!service_filter(service)){
        pam_logger("Service <%s> should not use biometric-authentication\n", service);
    	return PAM_IGNORE;
    }

    /* Different services use different processing function */
    if (strcmp(service, "lightdm") == 0) {
        char buf[128];
        get_greeter_session(buf, sizeof(buf));
        pam_logger("current greeter: %s\n", buf);

        if(strcmp(buf, "ukui-greeter") == 0 || strcmp(buf, "ukui-greeter-wayland") == 0 || strcmp(buf, "ukui-session") == 0 || strcmp(buf,"ukui-screensaver-backend"))
            return biometric_auth_embeded(pamh);
//        else
//            return biometric_auth_independent(pamh, "lightdm", 1);
    }
    else if (strcmp(service, "ukui-screensaver-qt")==0)
        return biometric_auth_embeded(pamh);
    else if (strcmp(service, "polkit-1") == 0){
        if(enable_by_polkit())
            return biometric_auth_embeded(pamh);
        else
            pam_logger("[PAM_BIOMETRIC]: It's not polkit-ukui-authentication-agent-1.\n");
    }
    else if (strcmp(service, "sudo") == 0 || strcmp(service, "sudo-i") == 0)
        return biometric_auth_independent(pamh, "sudo", 0);
    else if (strcmp(service, "login") == 0)
        return biometric_auth_independent(pamh, "login", 0);
    else if (strcmp(service, "su") == 0)
        return biometric_auth_independent(pamh, "su", 0);
//    else if (strcmp(service, "mate-screensaver") == 0)
//        return biometric_auth_independent(pamh, "mate-screensaver", 1);
    #ifdef ENABLE_BIOTEST
    else if (strcmp(service, "biotest") == 0)
        return biometric_auth_independent(pamh, "biotest", 1);
    #endif
    else
        pam_logger("Service <%s> slip through the service filter\n", service);
    return PAM_IGNORE;
}

int
pam_sm_setcred (pam_handle_t *pamh, int flags ,
        int argc , const char **argv )
{
    return PAM_SUCCESS;
}

