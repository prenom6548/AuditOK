#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>

#include "../include/kba/kba.h"

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */


#define MAX_LOG_SIZE 1024 // 日志缓冲区最大长度

// 日志级别，对应syslog中的LOG_ERR、LOG_WARNING、LOG_NOTICE、LOG_INFO、LOG_DEBUG
enum LogLevel { _LOG_ERROR, _LOG_WARN, _LOG_INFO, _LOG_NOTICE, _LOG_DEBUG };

/*安全问题答案——内部保存使用*/
typedef struct {  
    int user_id;
    char merge_question[KPR_MG_QUESION_LEN];
    char enc2_answer[KPR_ANSWER_LEN];
    char key[KPR_KEY_LEN];
} _SecurityAnswer;

typedef struct _SecurityAnswerListNode {
    _SecurityAnswer answer;
    struct _SecurityAnswerListNode *next;
} _SecurityAnswerListNode;

struct tallylog {
    char        fail_line[52];  /* rhost or tty of last failure */
    uint16_t    reserved;       /* 保留，用于将来的扩展 */
    uint16_t    fail_cnt;       /* 自上次成功以来的失败次数 */
    uint64_t    fail_time;      /* 最后一次失败的时间 */
};

int init_question_to_jsonfile(void) ;
int save_answer_to_jsonfile(char * jsonfile, _SecurityAnswerListNode * head);
int parse_question_form_jsonfile(char * jsonfile, SecurityQuestionListNode ** head);
int parse_answer_form_jsonfile(char * jsonfile, _SecurityAnswerListNode ** head);
void save_users_to_json(const User *users, int count, const char *filename);
int load_users_from_json(User *users, int *count, const char *filename);

int _addAnswerNode(_SecurityAnswerListNode **head, const void *data, size_t dataSize) ;
int _updateAnswerNode(_SecurityAnswerListNode **head, const void *data, size_t dataSize);
int _printAnswerList(_SecurityAnswerListNode *head);

void log_message(enum LogLevel level, const char *format, ...);

#ifdef  __cplusplus
}
#endif /* __cplusplus */
#endif

