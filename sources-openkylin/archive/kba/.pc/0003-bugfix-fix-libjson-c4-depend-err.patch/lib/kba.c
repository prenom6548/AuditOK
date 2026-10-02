
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <time.h>
#include <assert.h>

#include "common.h"
#include "tdes.h"
#include "pbkdf2_hmac.h"
#include "../include/kba/kba.h"

#define PRESET_SEC_QUESTIONS_FILE  "/etc/kba/question.json"
#define SEC_QUESTIONS_FILE         "/etc/security/questions"
#define SEC_KPRLOCK_FILE           "/tmp/.x8f6bm68fA.lock"
#define PASSWD_FILE                "/etc/passwd"
#define USER_PASSWD_TIP_FILE         "/etc/security/pwdtip"

#define DEFAULT_TALLYDIR "/tmp/.tallylog.d/"
#define PATH_MAX         32

static unsigned char g_key[KPR_KEY_LEN] = {0};

static void transform_string(char* str) {
    for (size_t i = 0; str[i]; ++i) {
        char currentChar = str[i];

        if (isalpha(currentChar)) {
            if (islower(currentChar)) {
                str[i] = ((currentChar - 'a' + 13) % 26) + 'a';
            } else {
                str[i] = ((currentChar - 'A' + 13) % 26) + 'A';
            }
        } else if (isdigit(currentChar)) {
            str[i] = ((currentChar - '0' + 3) % 10) + '0';
        }
    }

    return;
}

static int generate_key(void )
{
    FILE *urandom = NULL;
    unsigned char tmpkey[16] = {0};
    urandom = fopen("/dev/urandom", "r");
    if (!urandom) {
        log_message(_LOG_ERROR, "[%s]:[%d] open /dev/urandom failed.", __FUNCTION__, __LINE__);
        return -1;
    }

    size_t cnt = fread(tmpkey, sizeof(unsigned char), sizeof(tmpkey), urandom);
    if (cnt != sizeof(tmpkey)) {
        log_message(_LOG_ERROR, "[%s]:[%d] read /dev/urandom failed.", __FUNCTION__, __LINE__);
        fclose(urandom);
        return -1;
    }
    fclose(urandom);

    memset(g_key, 0, sizeof(g_key));
    for (int j = 0; j < sizeof(tmpkey); ++j) {
        snprintf(&g_key[j * 2], KPR_KEY_LEN - j * 2, "%02x", tmpkey[j]);
    }

    return 0;
}

int reset_tallylog(uid_t uid) {
    char log_path[PATH_MAX];
    struct tallylog tally;
    int fd, ret;

    snprintf(log_path, sizeof(log_path), "%s.%u", DEFAULT_TALLYDIR, uid);
    if (access(log_path, F_OK) == -1) {
        return -1;
    }

    fd = open(log_path, O_RDWR);
    if (fd == -1) {
        return -1;
    }

    ret = lseek(fd, (off_t)uid * sizeof(tally), SEEK_SET);
    if (ret == (off_t)-1) {
        close(fd);
        return -1;
    }

    memset(&tally, 0, sizeof(tally));
    ret = write(fd, &tally, sizeof(tally));
    if (ret != sizeof(tally)) {
        close(fd);
        return -1;
    }

    if (fsync(fd)) {
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

int check_user_is_valid(const char *username)
{
    int ret = -1;
    char buff[256] = {0};
    FILE* fr = fopen(PASSWD_FILE, "r");
    if( fr == NULL)
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] open %s failed .", PASSWD_FILE , __FUNCTION__, __LINE__);
	    return -1;
    }

    while( fgets(buff, 256, fr) )
    {
        char* ptr = buff;
        char* name = NULL;

        name = strsep(&ptr,":");
        if (name == NULL)
        {
            continue;
        }
        if (strcmp(name, username) == 0){
            ret = 0;
            break;
        }
    }

    fclose (fr);

    return ret;
}

int get_uid_by_username(const char *username)
{
    int ret = -1;
    char buff[256] = {0};
    FILE* fr = fopen(PASSWD_FILE, "r");
    if( fr == NULL)
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] open %s failed .", PASSWD_FILE , __FUNCTION__, __LINE__);
	    return -1;
    }

    while( fgets(buff, 256, fr) )
    {
        char* ptr = buff;
        char* name = NULL;
        char* userid = NULL;

        name = strsep(&ptr,":");
        assert(name != NULL);
        userid = strsep(&ptr,":");
        assert(userid != NULL);
        userid = strsep(&ptr,":");
        assert(userid != NULL);

        if (strcmp(name, username) == 0){
            ret = atoi(userid);
            break;
        }
    }

    fclose (fr);

    return ret;
}

int check_userID_is_valid(int uid)
{
    int ret = -1;
    char buff[256] = {0};
    FILE* fr = fopen(PASSWD_FILE, "r");
    if( fr == NULL)
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] open %s failed .", PASSWD_FILE , __FUNCTION__, __LINE__);
	    return -1;
    }

    while( fgets(buff, 256, fr) )
    {
        char* ptr = buff;
        char* name = NULL;
        char* userid = NULL;

        name = strsep(&ptr,":");
        assert(name != NULL);
        userid = strsep(&ptr,":");
        assert(userid != NULL);
        userid = strsep(&ptr,":");
        assert(userid != NULL);

        if (uid == atoi(userid)){
            ret = 0;
            break;
        }
    }

    fclose (fr);

    return ret;
}

/*用户答案合并*/
_SecurityAnswer * merge_answer(SecurityAnswerListNode * head , char *localkey){
    int i = 0;
    int ret = 0;
    int flag = 0;
    char buf[1024] = {0};
    char hmac[33] = { 0 };
    unsigned char key[KPR_KEY_LEN] = {0};
    FILE *urandom = NULL;

    if (head == NULL)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] head is NULL.", __FUNCTION__, __LINE__);
        return NULL;
    }

    _SecurityAnswer *ans = malloc(sizeof(_SecurityAnswer));
    if (ans == NULL)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] malloc failed.", __FUNCTION__, __LINE__);
        return NULL;
    }
    memset(ans, 0, sizeof(_SecurityAnswer));
    
    SecurityAnswerListNode *current = head;
    while (current != NULL) {
        if ((current->answer.questionId < 0) || (current->answer.questionId > 10) || \
            (current->answer.encryptedAnswer == NULL)){
            flag = 1;
            break;
        }
    
        ans->user_id = current->answer.userId;
        ans->merge_question[i] = (char)(current->answer.questionId + '0');
        memcpy(&buf[i*64], current->answer.encryptedAnswer, sizeof(current->answer.encryptedAnswer));
        
        i++;
        current = current->next;
    }

    if (flag == 1) 
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] questionId is out of range.", __FUNCTION__, __LINE__);
        goto FINAL;
    }
    
    if(localkey == NULL){
        unsigned char tmpkey[16] = {0};
        urandom = fopen("/dev/urandom", "r");
        if (!urandom) {
            log_message(_LOG_ERROR, "[%s]:[%d] open /dev/urandom failed.", __FUNCTION__, __LINE__);
            goto FINAL;
        }

        size_t cnt = fread(tmpkey, sizeof(unsigned char), sizeof(tmpkey), urandom);
        if (cnt != sizeof(tmpkey)) {
            log_message(_LOG_ERROR, "[%s]:[%d] read /dev/urandom failed.", __FUNCTION__, __LINE__);
            goto FINAL;
        }
        fclose(urandom);

        for (int j = 0; j < sizeof(tmpkey); ++j) {
            snprintf(&key[j * 2], KPR_KEY_LEN - j * 2, "%02x", tmpkey[j]);
        }

        memcpy(ans->key, key, sizeof(key));
    }else {
        memcpy(key, localkey, sizeof(key));
    }

    ans->merge_question[i] = '\0';
    buf[i*64] = '\0';
    
    ret = KPR_PKCS5_PBKDF2_HMAC2((unsigned char*)buf, strlen(buf), key, sizeof(key), 2, 32, (unsigned char*)hmac);
    if (ret != 0)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] KPR_PKCS5_PBKDF2_HMAC2 failed.", __FUNCTION__, __LINE__);
        goto FINAL;
    }
    
    char *hexBlk = Hex(hmac, 32, 0);
    if(hexBlk == NULL){
        log_message(_LOG_ERROR, "[%s]:[%d] Hex failed.", __FUNCTION__, __LINE__);
        goto FINAL;
    }
    memcpy(ans->enc2_answer, hexBlk, 32*2);
    return ans;

FINAL:
    log_message(_LOG_ERROR, "[%s]:[%d] Final.", __FUNCTION__, __LINE__);
    free(ans);
    ans = NULL;
    if(urandom != NULL){
        fclose(urandom);
    }

    return ans;
}

/*
获取预设的安全问题列表，问题个数
* 输入参数：
*  无
* 输出参数：
*  type： 问题个数

* 返回值：
*  安全问题列表
*/
SecurityQuestionListNode *getPresetSecurityQuestions( int *count ){

    SecurityQuestionListNode *list = NULL;
    SecurityQuestionListNode *current = NULL;
    
    if (access(PRESET_SEC_QUESTIONS_FILE, F_OK) != 0)
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] %s not exist.", __FUNCTION__, __LINE__, PRESET_SEC_QUESTIONS_FILE);
        return NULL;
    }

    if (0 != parse_question_form_jsonfile(PRESET_SEC_QUESTIONS_FILE, &list)){
        log_message(_LOG_ERROR, "[%s]:[%d] parse_question_form_jsonfile failed.", __FUNCTION__, __LINE__);
        return NULL;
    }
    
    current = list;
    while (current != NULL) {
        *count += 1;      
        current = current->next;
    }

    return list;
}

/*
2、绑定安全问题，传入用户id和用户选择的问题及答案信息，返回绑定结构
* 输入参数：
*  type: 用户安全问题列表
* 输出参数：
*  type： 无

* 返回值：
*  1---绑定失败
*  0---绑定成功
*/
int setUserSecurityAnswers( SecurityAnswerListNode * head ){
    int ret = 0;
    int flag = 0;
    _SecurityAnswerListNode * history = NULL;
    _SecurityAnswerListNode * new = NULL; 

    if (head == NULL) {
        log_message(_LOG_ERROR, "[%s]:[%d] head is NULL.", __FUNCTION__, __LINE__);
        return 1;
    }
    
    #if 0
    ret = sortAnswerList(&head);
    if (ret != 0) {
        log_message(_LOG_ERROR, "[%s]:[%d] sortAnswerList failed.", __FUNCTION__, __LINE__);
        return 1;
    }
    #endif

    _SecurityAnswer *ans = merge_answer(head, NULL);
    
    if (ans == NULL) {
        log_message(_LOG_ERROR, "[%s]:[%d] ans is NULL.", __FUNCTION__, __LINE__);
        return 1;
    }

    if (0 != check_userID_is_valid(ans->user_id)){
        log_message(_LOG_ERROR, "[%s]:[%d] user_id[%d] is invalid.", __FUNCTION__, __LINE__, ans->user_id);
        return 1;
    }

    if (access(SEC_QUESTIONS_FILE, F_OK) == 0)
    {   
        ret = parse_answer_form_jsonfile(SEC_QUESTIONS_FILE, &history);
        if (ret != 0) {
            log_message(_LOG_ERROR, "[%s]:[%d] parse_answer_form_jsonfile failed.", __FUNCTION__, __LINE__);
            return 1;
        }
        flag = IsUserSecurityAnswers(ans->user_id);
        if (flag == 1) {
            /* 用户已绑定安全问题 */
            _updateAnswerNode(&history,(void *)ans, sizeof(_SecurityAnswer));
            ret = save_answer_to_jsonfile(SEC_QUESTIONS_FILE, history);
            if (ret != 0) {
                log_message(_LOG_ERROR, "[%s]:[%d] save_answer_to_jsonfile failed.", __FUNCTION__, __LINE__);
                return 1;
            }
        }else{
            /* 用户未绑定安全问题 */
            _addAnswerNode(&history, (void *)ans, sizeof(_SecurityAnswer));
            ret = save_answer_to_jsonfile(SEC_QUESTIONS_FILE, history);
            if (ret != 0) {
                log_message(_LOG_ERROR, "[%s]:[%d] save_answer_to_jsonfile failed.", __FUNCTION__, __LINE__);
                return 1;
            }
        }
        
    }else{
        /* 首次保存安全问题 */
        _addAnswerNode(&new, (void *)ans, sizeof(_SecurityAnswer));
        ret = save_answer_to_jsonfile(SEC_QUESTIONS_FILE, new);
        if (ret != 0) {
            log_message(_LOG_ERROR, "[%s]:[%d] save_answer_to_jsonfile failed.", __FUNCTION__, __LINE__);
            return 1;
        }
    }

    return ret;
}

/*
获取用户是否已绑定安全问题
* 输入参数：
*  type: 用户ID
* 输出参数：
*  type： 无

* 返回值：
*  1---用户已绑定
*  0---用户未绑定
*/
int IsUserSecurityAnswers(int uid){
    int ret = 0;
    _SecurityAnswerListNode *list = NULL;

    if (0 != check_userID_is_valid(uid)){
        log_message(_LOG_ERROR, "[%s]:[%d] uid[%d] is invalid.", __FUNCTION__, __LINE__, uid);
        return 0;
    }
    
    if (access(SEC_QUESTIONS_FILE, F_OK) == 0)
    {
        ret = parse_answer_form_jsonfile(SEC_QUESTIONS_FILE, &list);
        if ((ret != 0) || (list == NULL)) {
            log_message(_LOG_ERROR, "[%s]:[%d] parse_answer_form_jsonfile failed.", __FUNCTION__, __LINE__);
            return 0;
        }

        _SecurityAnswerListNode *current = list;
        while (current != NULL) {            
            if (current->answer.user_id == uid){
                ret = 1;
            }

            current = current->next;
        }
    }

    return ret;
}

/*
获取用户绑定的安全问题列表
* 输入参数：
*  type: 用户ID
* 输出参数：
*  type： 问题个数

* 返回值：
*  安全问题列表
*/
SecurityQuestionListNode * getUserSecurityQuestions( int uid, int *count ){
    int questions = 0;
    int cnt = 0;
    int div = 100;
    SecurityQuestionListNode *preset = NULL;
    SecurityQuestionListNode *cur = NULL;
    SecurityQuestionListNode *ret = NULL;
    _SecurityAnswerListNode *answer = NULL;

    if (0 != check_userID_is_valid(uid)){
        log_message(_LOG_ERROR, "[%s]:[%d] uid[%d] is invalid.", __FUNCTION__, __LINE__, uid);
        return NULL;
    }
    
    if (access(SEC_QUESTIONS_FILE, F_OK) == 0)
    {   
        int tt = 0;
        tt = parse_answer_form_jsonfile(SEC_QUESTIONS_FILE, &answer);
        if ((tt != 0) || (answer == NULL)) {
            log_message(_LOG_ERROR, "[%s]:[%d] parse_answer_form_jsonfile failed.", __FUNCTION__, __LINE__);
            return NULL;
        }
    }

    preset = getPresetSecurityQuestions(&cnt);
    if (preset == NULL) {
        log_message(_LOG_ERROR, "[%s]:[%d] getPresetSecurityQuestions failed.", __FUNCTION__, __LINE__);
        return NULL;
    }
    
    _SecurityAnswerListNode *current = answer;
    while (current != NULL) {
        if (current->answer.user_id == uid){
            SecurityQuestion sec_question;
            memset(&sec_question, 0, sizeof(SecurityQuestion));
            questions = atoi(current->answer.merge_question);

            for (size_t i = 0; i < 3; i++)
            {   
                sec_question.questionId = questions / div;
                questions %= div;
                div /= 10;

                cur = preset;
                while (cur != NULL)
                {
                    if (cur->question.questionId == sec_question.questionId){
                        strcpy(sec_question.questionText, cur->question.questionText);
                        addQuestionNode(&ret, (void *)&sec_question, sizeof(SecurityQuestion));
                    }

                    cur = cur->next;
                }                
            }
        }

        current = current->next;
    }
    *count = 3;

    return ret;
}

/*
校验用户的安全问题及对应答案
* 输入参数：
*  tyep: 用户安全答案列表
* 输出参数：
*  type： token---API token

* 返回值：
*  1---安全问题答案正确
*  0---安全问题答案错误
*/
int verifyUserSecurityAnswers( SecurityAnswerListNode *remote , char ** token){
    int ret = 0;
    //int cnt = 0;
    char *encpt = NULL;

    _SecurityAnswerListNode *local = NULL;

    log_message(_LOG_INFO, "[%s]:[%d] start verifying the answer to the questions.", __FUNCTION__, __LINE__);
    
    if (remote == NULL){
        log_message(_LOG_ERROR, "[%s]:[%d] remote is NULL.", __FUNCTION__, __LINE__);
        return 0;
    }

    if (access(SEC_QUESTIONS_FILE, F_OK) == 0)
    {
        ret = parse_answer_form_jsonfile(SEC_QUESTIONS_FILE, &local);
        if ((ret != 0) || (local == NULL)) {
            log_message(_LOG_ERROR, "[%s]:[%d] parse_answer_form_jsonfile failed.", __FUNCTION__, __LINE__);
            return 0;
        }
    }else{
        log_message(_LOG_ERROR, "[%s]:[%d] file %s don't exist.",SEC_QUESTIONS_FILE , __FUNCTION__, __LINE__);
        return 0;
    }

    char *localkey = NULL;
    _SecurityAnswerListNode *currentlocal = local;
    SecurityAnswerListNode *currentremote = remote;
    while (currentlocal != NULL && currentremote != NULL)
    {   
        if (currentlocal->answer.user_id == currentremote->answer.userId){
            localkey = currentlocal->answer.key;
            break;
        }
        currentlocal = currentlocal->next;
    }

    if (localkey == NULL)
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] localkey is NULL.", __FUNCTION__, __LINE__);
        return 0;
    }

    #if 0
    ret = sortAnswerList(&remote);
    if (ret != 0) {
        log_message(_LOG_ERROR, "[%s]:[%d] sortAnswerList failed.", __FUNCTION__, __LINE__);
        return 0;
    }
    #endif
    /* workround for remote list order problem */
    reverseAnswerList(&remote);

    _SecurityAnswer *ans = merge_answer(remote, localkey);
    if (ans == NULL) {
        log_message(_LOG_ERROR, "[%s]:[%d] merge_answer failed.", __FUNCTION__, __LINE__);
        return 0;
    }

    if (0 != check_userID_is_valid(ans->user_id)){
        log_message(_LOG_ERROR, "[%s]:[%d] user_id[%d] is invalid.", __FUNCTION__, __LINE__, ans->user_id);
        return 0;
    }

    currentlocal = local;
    while (currentlocal != NULL)
    {   
        if (currentlocal->answer.user_id == ans->user_id){
            printf("questions: [%s] [%s]\n", currentlocal->answer.merge_question, ans->merge_question);
            printf("answer:    [%s] [%s]\n", currentlocal->answer.enc2_answer, ans->enc2_answer);
            
            if ((memcmp(currentlocal->answer.enc2_answer, ans->enc2_answer,sizeof(ans->enc2_answer)) == 0) &&\
                (memcmp(currentlocal->answer.merge_question, ans->merge_question, sizeof(ans->merge_question)) == 0)){
                ret = 1;
            }
        }
        currentlocal = currentlocal->next;
    }

    if(ret == 1){
        char plain[128] = {0};

        if (generate_key() == -1){
            log_message(_LOG_ERROR, "[%s]:[%d] generate_key failed.", __FUNCTION__, __LINE__);
            return 0;
        }
        
        sprintf(plain, "%d %ld %s", ans->user_id, time(NULL), ans->enc2_answer);
        int len = strlen(plain);
        encpt = (char *)malloc(4096);
        if (encpt == NULL){
            log_message(_LOG_ERROR, "[%s]:[%d] malloc failed. ", __FUNCTION__, __LINE__);
            return 0;
        }
        memset(encpt, 0, 4096);
        cipher2(g_key, plain, encpt, len);
        *token = encpt;

        log_message(_LOG_INFO, "[%s]:[%d] verification question answer passed!", __FUNCTION__, __LINE__);
    }else{
        log_message(_LOG_WARN, "[%s]:[%d] verification question answer failed!", __FUNCTION__, __LINE__);
    }
    
    return ret;
}

/*
安全问题答案加密
* 输入参数：
*  type: password---用户输入的问题答案
*  type: plen---用户输入的问题答案长度
*  type: salt---盐值
*  type: slen---盐值长度
*  type: iteration_count---迭代次数
*  type: key_length---密钥长度
* 输出参数：
*  type： output---密钥
* 返回值：
*  1---加密失败
*  0---加密成功
*/
int KPR_PKCS5_PBKDF2_HMAC2(const unsigned char *password, size_t plen,
    const unsigned char *salt, size_t slen,
    const unsigned long iteration_count, const unsigned long key_length,
    unsigned char *output) {
    int ret = 0;
    if (password == NULL || plen == 0 || salt == NULL || slen == 0 || iteration_count == 0 || key_length == 0 )
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] parameter is NULL.", __FUNCTION__, __LINE__);
        return 1;
    }
    
    PKCS5_PBKDF2_HMAC2((unsigned char*)password, plen, salt, slen, iteration_count, key_length, (unsigned char*)output);
    
    return ret;
}

char *Hex(const char* cpSrcStr, int len, int isUpper) {
    if (cpSrcStr == NULL) {
        return "";
    }
    
    char * ret = (char *)malloc(len * 2 + 1);
    if (ret == NULL) {
        return "";
    }
    memset(ret, 0, len * 2);

    const char* hex = isUpper ? "0123456789ABCDEF" : "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        unsigned char v = (unsigned char)(cpSrcStr[i]);
        ret[2*i]  = hex[(v >> 4) & 0x0f];
        ret[2*i+1] = hex[ v & 0x0f];
    }
    ret[len * 2] = '\0';

    return ret;
}

/*
安全问题答案加密 简化版v2
* 输入参数：
*  type: password---用户输入的问题答案
*  type: plen---用户输入的问题答案长度
* 输出参数：
*  type： output---密钥
* 返回值：
*  1---加密失败
*  0---加密成功
*/
int KPR_PKCS5_PBKDF2_HMAC2_v2(const unsigned char *password, size_t plen, char **output) {
    int iteration_count = 2;
    int key_length = 32;
    char hmac[33] = {0};
    unsigned char key[1024] = {0};

    if (password == NULL || plen == 0 )
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] password or plen is NULL.", __FUNCTION__, __LINE__);
        return 1;
    }

    memcpy(key, password, plen);
    transform_string(key);

    PKCS5_PBKDF2_HMAC2((unsigned char*)password, plen, key, strlen(key), iteration_count, key_length, (unsigned char*)hmac);
    *output = Hex(hmac, 32, 0);
    //printf("hmac: %s\n", *output);
    
    return 0;
}

int excute_cmd(char * strcmd)
{
    int status = 0;
    if (!strcmd){
        return 1;
    }

    status = system(strcmd);
    if (status == -1)
    {
        return 1;
    }

    /*
    **WIFEXITED(status)用来指出子进程是否为正常退出的，如果是，它会返回一个非零值。
    */
    if (WIFEXITED(status))
    {
        /*当WIFEXITED返回非零值时，我们可以用这个宏来提取子进程的返回值，
        **如果子进程调用exit(5)退出，WEXITSTATUS(status)就会返回5；
        **如果子进程调用exit(7)，WEXITSTATUS(status)就会返回7。
        **请注意，如果进程不是正常退出的，也就是说，WIFEXITED返回0，这个值就毫无意义。
        **/
        if (WEXITSTATUS(status) == 0)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
}

/*
修改用户密码
* 输入参数：
*  type: username---用户名
*  type: password---用户密码
*  type: token---API token
* 输出参数：
*  type： 无
* 返回值：
*  1---修改失败
*  0---修改成功
*/
int changePassword(const char *username, const char *password, char *token){
    int ret = 0;
    int token_match = 0;
    int olduid = 0;
    int oldtime = 0;
    int curtime = time(NULL);
    char result[4096] = {0};
    char *cipher = NULL;
    _SecurityAnswerListNode *local = NULL;

    if (username == NULL || password == NULL || token == NULL){
        log_message(_LOG_ERROR, "[%s]:[%d] username/password/token is NULL.", __FUNCTION__, __LINE__);
        return 1;
    }

    log_message(_LOG_WARN, "[%s]:[%d] start changing user:[%s] password.", __FUNCTION__, __LINE__, username);

    if (check_user_is_valid(username) != 0){
        log_message(_LOG_ERROR, "[%s]:[%d] %s is not exist.", __FUNCTION__, __LINE__, username);
        return 1;
    }

    if(geteuid() != 0)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] Permission denied, current euid is %d.", __FUNCTION__, __LINE__, geteuid);
        return 1;
    }

    decipher2(g_key, result, token, strlen(token));
    memset(g_key, 0, sizeof(g_key));

    char *p = strtok(result, " ");
    if (p == NULL)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] uid is empty.", __FUNCTION__, __LINE__);
        return 1;
    }
    
    olduid = atoi(p);
    p = strtok(NULL, " ");
    if (p == NULL)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] time is empty.", __FUNCTION__, __LINE__);
        return 1;
    }
    oldtime = atoi(p);
    
    p = strtok(NULL, " ");
    if (p == NULL)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] cipher is empty.", __FUNCTION__, __LINE__);
        return 1;
    }
    cipher = p;

    if (olduid != get_uid_by_username(username)){
        log_message(_LOG_ERROR, "[%s]:[%d] uid[%d != %d] is difference.", __FUNCTION__, __LINE__, olduid, get_uid_by_username(username));
        return 1;
    }

    if ((curtime - oldtime) > KPR_DEFAULT_PASSWD_TIMEOUT ){
        log_message(_LOG_ERROR, "[%s]:[%d] time is expired.", __FUNCTION__, __LINE__);
        return 1;
    }
    
    if (access(SEC_QUESTIONS_FILE, F_OK) == 0)
    {
        ret = parse_answer_form_jsonfile(SEC_QUESTIONS_FILE, &local);
        if (ret != 0){
            log_message(_LOG_ERROR, "[%s]:[%d] parse_answer_form_jsonfile failed.", __FUNCTION__, __LINE__);
            return 1;
        }
    }else{
        log_message(_LOG_ERROR, "[%s]:[%d] can not find file %s",  __FUNCTION__, __LINE__, SEC_QUESTIONS_FILE);
        return 1;
    }

    _SecurityAnswerListNode *currentlocal = local;
    while (currentlocal != NULL)
    {   
        if (currentlocal->answer.user_id == olduid){
            printf("questions: [%s]\n", currentlocal->answer.merge_question);
            printf("answer:    [%s]\n", currentlocal->answer.enc2_answer);
            
            if (memcmp(currentlocal->answer.enc2_answer, cipher,sizeof(currentlocal->answer.enc2_answer)) == 0){
                token_match = 1;
                break;
            }
        }
        currentlocal = currentlocal->next;
    }

    if (token_match == 0)
    {   
        log_message(_LOG_ERROR, "[%s]:[%d] token is not match.", __FUNCTION__, __LINE__);
        return 1;
    }

    if (reset_tallylog(get_uid_by_username(username)) != 0){
        log_message(_LOG_ERROR, "[%s]:[%d] reset_tallylog failed.", __FUNCTION__, __LINE__);
        return 1;
    }
    
    char *cmd = (char *)malloc(strlen(username) + strlen(password) + 20);
    if (cmd == NULL)
    {
        log_message(_LOG_ERROR, "[%s]:[%d] malloc failed", __FUNCTION__, __LINE__);
        return 1;
    }

    memset(cmd, 0, strlen(username) + strlen(password) + 20);
    sprintf(cmd, "echo '%s:%s' |chpasswd", username, password);

    ret = excute_cmd(cmd);
    if (ret != 0)
    {
        log_message(_LOG_WARN, "[%s]:[%d] change user: [%s] password failed.", __FUNCTION__, __LINE__, username);
    }else
    {
        log_message(_LOG_INFO, "[%s]:[%d] change user: [%s] password success.", __FUNCTION__, __LINE__, username);
    }
    
    free(cmd);
    cmd = NULL;

    return ret;
}

int set_password_hint(int uid, const char *password_hint) {
    User users[KPR_USER_MAX] = {0};
    int user_count = 0;
    memset(users, 0, sizeof(users));

    if (check_userID_is_valid(uid) != 0) {
        log_message(_LOG_ERROR, "[%s]:[%d] uid[%d] is invalid.", __FUNCTION__, __LINE__, uid);
        return -1;
    }

    if (access(USER_PASSWD_TIP_FILE, F_OK) == 0) {
        if (load_users_from_json(users, &user_count, USER_PASSWD_TIP_FILE) != 0) {
            log_message(_LOG_ERROR, "[%s]:[%d] load_users_from_json failed.", __FUNCTION__, __LINE__);
            return -1;
        }
    }

    int found = 0;
    for (int i = 0; i < user_count; i++) {
        if (users[i].uid == uid) {
            strncpy(users[i].password_hint, password_hint, sizeof(users[i].password_hint) - 1);
            users[i].password_hint[sizeof(users[i].password_hint) - 1] = '\0'; // Ensure null-termination
            found = 1;
            break;
        }
    }

    if (!found) {
        // If the user is not found, add a new user
        User new_user = {0};
        new_user.uid = uid;
        strncpy(new_user.password_hint, password_hint, sizeof(new_user.password_hint) - 1);
        new_user.password_hint[sizeof(new_user.password_hint) - 1] = '\0'; // Ensure null-termination
        users[user_count++] = new_user;
    }

    save_users_to_json(users, user_count, USER_PASSWD_TIP_FILE);
    return 0;
}

int get_password_hint(int uid, char *password_hint) {
    User users[KPR_USER_MAX] = {0};
    int user_count = 0;
    memset(users, 0, sizeof(users));

    if (check_userID_is_valid(uid) != 0) {
        log_message(_LOG_ERROR, "[%s]:[%d] uid[%d] is invalid.", __FUNCTION__, __LINE__, uid);
        return -1;
    }

    if (access(USER_PASSWD_TIP_FILE, F_OK) == 0) {
        if (load_users_from_json(users, &user_count, USER_PASSWD_TIP_FILE) != 0) {
            log_message(_LOG_ERROR, "[%s]:[%d] load_users_from_json failed.", __FUNCTION__, __LINE__);
            return -1;
        }
    }

    for (int i = 0; i < user_count; i++) {
        if (users[i].uid == uid) {
            strncpy(password_hint, users[i].password_hint, sizeof(users[i].password_hint) - 1);
            password_hint[sizeof(users[i].password_hint) - 1] = '\0'; // Ensure null-termination
            return 0;
        }
    }

    return -1; // User not found
}