#ifndef __KYLIN_PASSWDRESET_H__
#define __KYLIN_PASSWDRESET_H__

#include <bits/types.h>
#include <linux/limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KPR_ERROR          (-1)
#define KPR_SUCCESS        (0)

#define KPR_QUESTION_NUM    10
#define KPR_QUESION_LEN     128
#define KPR_MG_QUESION_LEN  16
#define KPR_ANSWER_LEN      65
#define KPR_KEY_LEN         33


#define KPR_DEFAULT_PASSWD_TIMEOUT  3600

/*安全问题*/
typedef struct {
    int questionId; 
    char questionText[KPR_QUESION_LEN];  
} SecurityQuestion;

/*安全问题链表*/
typedef struct SecurityQuestionListNode {
    SecurityQuestion question;
    struct SecurityQuestionListNode *next;
} SecurityQuestionListNode;

/*安全问题答案*/
typedef struct {  
    int userId;  
    int questionId;
    char encryptedAnswer[KPR_ANSWER_LEN];  
} SecurityAnswer; 

/*安全问题答案链表*/
typedef struct SecurityAnswerListNode {
    SecurityAnswer answer;
    struct SecurityAnswerListNode *next;
} SecurityAnswerListNode;

/*
获取预设的安全问题列表，问题个数
* 输入参数：
*  无
* 输出参数：
*  type： 问题个数

* 返回值：
*  安全问题列表
*/
SecurityQuestionListNode *getPresetSecurityQuestions( int *count );

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
int setUserSecurityAnswers( SecurityAnswerListNode * head );

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
int IsUserSecurityAnswers( int uid );

/*
获取用户绑定的安全问题列表
* 输入参数：
*  type: 用户ID
* 输出参数：
*  type： 问题个数

* 返回值：
*  安全问题列表
*/
SecurityQuestionListNode * getUserSecurityQuestions( int uid, int *count );

/*
校验用户的安全问题及对应答案
* 输入参数：
*  type: 用户安全答案列表
* 输出参数：
*  type： token---API token

* 返回值：
*  1---安全问题答案正确
*  0---安全问题答案错误
*/
int verifyUserSecurityAnswers( SecurityAnswerListNode *remote , char ** token);

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
    unsigned char *output);

/*
字符串转换为16进制
* 输入参数：
*  type: cpSrcStr---输入字符串
*  type: len---输入字符串长度
*  type: isUpper---是否大写
* 输出参数：
*  type： 无

* 返回值：
*  16进制字符串
*/
char *Hex(const char* cpSrcStr, int len, int isUpper);

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
int KPR_PKCS5_PBKDF2_HMAC2_v2(const unsigned char *password, size_t plen, char **output);

/*
增加安全问题节点到链表
* 输入参数：
*  type: head---链表头指针
*  type: data---节点数据
*  type: dataSize---节点数据长度
* 输出参数：
*  type： 无

* 返回值：
*  0---成功
*  -1---失败
*/
int addQuestionNode(SecurityQuestionListNode **head, const void *data, size_t dataSize);

/*
增加安全问题答案节点到链表
* 输入参数：
*  type: head---链表头指针
*  type: data---节点数据
*  type: dataSize---节点数据长度
* 输出参数：
*  type： 无

* 返回值：
*  0---成功
*  -1---失败
*/
int addAnswerNode(SecurityAnswerListNode **head, const void *data, size_t dataSize);

/*
打印安全问题链表
* 输入参数：
*  type: head---链表头指针
* 输出参数：
*  type： 无

* 返回值：
*  0---成功
*  -1---失败
*/
int printQuestionList(SecurityQuestionListNode *head);

/*
打印安全问题答案链表
* 输入参数：
*  type: head---链表头指针
* 输出参数：
*  type： 无

* 返回值：
*  0---成功
*  -1---失败
*/
int printAnswerList(SecurityAnswerListNode *head);

/*
重新排序安全问题答案链表
* 输入参数：
*  type: head---链表头指针
* 输出参数：
*  type： 无

* 返回值：
*  0---成功
*  -1---失败
*/
int sortAnswerList(SecurityAnswerListNode **head);

/*
逆序安全问题答案链表
* 输入参数：
*  type: head---链表头指针
* 输出参数：
*  type： 无

* 返回值：无
*/
void reverseAnswerList(SecurityAnswerListNode **head);

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
int changePassword(const char *username, const char *password, char *token);

#ifdef __cplusplus
}
#endif

#endif
