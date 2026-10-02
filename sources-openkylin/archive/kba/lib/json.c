
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <json-c/json.h>
//#include <mcheck.h>
#include "../include/kba/kba.h"
#include "common.h"

int save_answer_to_jsonfile(char * jsonfile, _SecurityAnswerListNode * head) {
    if (head == NULL)
    {
        fprintf(stderr, "Error: answer list is empty\n");
        return 1;
    }
    
    _SecurityAnswerListNode *current = head;
    //mtrace();
    // 创建 JSON 对象
    struct json_object *array = json_object_new_array();

    while (current != NULL) {

        struct json_object *qa = json_object_new_object();
        // 添加数据到 qa
        json_object_object_add(qa, "userid", json_object_new_int(current->answer.user_id));
        json_object_object_add(qa, "questions", json_object_new_string(current->answer.merge_question));
        json_object_object_add(qa, "answer", json_object_new_string(current->answer.enc2_answer));
        json_object_object_add(qa, "key", json_object_new_string(current->answer.key));
        json_object_array_add(array, qa);

        current = current->next;
    }

    // 创建 JSON 字符串
    const char *json_str = json_object_to_json_string_ext(array, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);

    mode_t original_umask = umask(0);
    umask(0077);
    // 将 JSON 字符串写入文件
    FILE *file = fopen(jsonfile, "w");
    if (file != NULL) {
        fprintf(file, "%s", json_str);
        fclose(file);
    } else {
        perror("Error opening file");
    }
    umask(original_umask);

    // 释放内存
    json_object_put(array);

    return 0;

}

/*生成问题列表模板*/
int init_question_to_jsonfile(void) {
    SecurityQuestion question;
    
    //mtrace();
    // 创建 JSON 对象
    struct json_object *array = json_object_new_array();

    for(int i=0; i<10; i++) {
        struct json_object *qa = json_object_new_object();

        memset(&question, 0, sizeof(SecurityQuestion));    
        question.questionId = i+1;
        strcpy(question.questionText, "What is your favorite color?");
        // 添加数据到 qa
        json_object_object_add(qa, "questionid", json_object_new_int(question.questionId));
        json_object_object_add(qa, "questionText", json_object_new_string(question.questionText));
        json_object_array_add(array, qa);
    }

    // 创建 JSON 字符串
    const char *json_str = json_object_to_json_string_ext(array, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);

    // 打印 JSON 字符串
    printf("JSON String:\n%s\n", json_str);

    // 将 JSON 字符串写入文件
    FILE *file = fopen("../data/question_001.json", "w");
    if (file != NULL) {
        fprintf(file, "%s", json_str);
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // 释放内存
    json_object_put(array);

    return 0;

}

int parse_question_form_jsonfile(char * jsonfile, SecurityQuestionListNode ** head) {
    struct json_object *parsed_json;
    struct json_object *question;
    int i, count;
    int ret = 0;

    if (jsonfile == NULL)
    {
        fprintf(stderr, "Error: jsonfile is empty\n");
        return 1;
    }

    // 读取JSON文件
    FILE *file = fopen(jsonfile, "r");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }
    char buffer[65536];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    if (bytesRead == 0)
    {   
        fprintf(stderr, "Error file is empty\n");
        return 1;
    }

    // 解析JSON
    parsed_json = json_tokener_parse(buffer);
    if (!parsed_json) {
        fprintf(stderr, "Error parsing JSON\n");
        return 1;
    }

    // 获取问题数组
    count = json_object_array_length(parsed_json);

    // 遍历问题数组
    for (i = 0; i < count; i++) {
        question = json_object_array_get_idx(parsed_json, i);
        struct json_object *id = NULL;
        struct json_object *q = NULL;
        json_object_object_get_ex(question, "questionid", &id);
        json_object_object_get_ex(question, "questionText", &q);
        
        SecurityQuestion sec_question;
        memset(&sec_question, 0, sizeof(SecurityQuestion));

        if(id == NULL || q == NULL){
            fprintf(stderr, "Error parsing JSON\n");
            ret = 1;
            break;
        }

        sec_question.questionId = json_object_get_int(id);
        strcpy(sec_question.questionText, json_object_get_string(q));

        addQuestionNode(head, (void *)&sec_question, sizeof(SecurityQuestion));
    }

    // 释放内存
    json_object_put(parsed_json);

    return ret;
}

int parse_answer_form_jsonfile(char * jsonfile, _SecurityAnswerListNode ** head) {
    struct json_object *parsed_json = NULL;
    struct json_object *answerinfo = NULL;
    int i, count = 0;
    int ret = 0;

    if ( jsonfile == NULL )
    {
        fprintf(stderr, "Error: jsonfile is empty\n");
        return 1;
    }

    // 读取JSON文件
    FILE *file = fopen(jsonfile, "r");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }
    char buffer[65536] = {0};
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    if (bytesRead == 0)
    {   
        fprintf(stderr, "Error file is empty\n");
        return 1;
    }

    // 解析JSON
    parsed_json = json_tokener_parse(buffer);
    if (!parsed_json) {
        fprintf(stderr, "Error parsing JSON\n");
        return 1;
    }

    count = json_object_array_length(parsed_json);

    // 遍历问题数组
    for (i = 0; i < count; i++) {
        answerinfo = json_object_array_get_idx(parsed_json, i);
        struct json_object *uid, *questions, *answer, *key;
        json_object_object_get_ex(answerinfo, "userid", &uid);
        json_object_object_get_ex(answerinfo, "questions", &questions);
        json_object_object_get_ex(answerinfo, "answer", &answer);
        json_object_object_get_ex(answerinfo, "key", &key);

        if (uid == NULL || questions == NULL || answer == NULL || key == NULL)
        {
            fprintf(stderr, "Error parsing JSON\n");
            ret = 1;
            break;
        }
        
        _SecurityAnswer sec_answer;
        memset(&sec_answer, 0, sizeof(_SecurityAnswer));

        sec_answer.user_id = json_object_get_int(uid);
        strcpy(sec_answer.merge_question, json_object_get_string(questions));
        strcpy(sec_answer.enc2_answer, json_object_get_string(answer));
        strcpy(sec_answer.key, json_object_get_string(key));
        _addAnswerNode(head, (void *)&sec_answer, sizeof(_SecurityAnswer));
    }

    // 释放内存
    json_object_put(parsed_json);

    return ret;
}

void save_users_to_json(const User *users, int count, const char *filename) {
    struct json_object *root = json_object_new_object();
    struct json_object *users_array = json_object_new_array();

    for (int i = 0; i < count; i++) {
        struct json_object *user_obj = json_object_new_object();
        json_object_object_add(user_obj, "uid", json_object_new_int(users[i].uid));
        json_object_object_add(user_obj, "passwordHint", json_object_new_string(users[i].password_hint));
        json_object_array_add(users_array, user_obj);
    }

    json_object_object_add(root, "users", users_array);

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Failed to open file");
        json_object_put(root);
        return;
    }

    char *json_str = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    fprintf(fp, "%s", json_str);
    fclose(fp);
    json_object_put(root);
}

int load_users_from_json(User *users, int *count, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Failed to open file");
        *count = 0;
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc(fsize + 1);
    fread(buffer, 1, fsize, fp);
    buffer[fsize] = '\0';
    fclose(fp);

    struct json_object *root = json_tokener_parse(buffer);
    free(buffer);

    if (!root) {
        fprintf(stderr, "Error parsing JSON\n");
        return -1;
    }

    struct json_object *users_array = json_object_object_get(root, "users");
    *count = json_object_array_length(users_array);

    for (int i = 0; i < *count; i++) {
        struct json_object *user_obj = json_object_array_get_idx(users_array, i);
        users[i].uid = json_object_get_int(json_object_object_get(user_obj, "uid"));
        const char *hint = json_object_get_string(json_object_object_get(user_obj, "passwordHint"));
        strncpy(users[i].password_hint, hint, sizeof(users[i].password_hint) - 1);
        users[i].password_hint[sizeof(users[i].password_hint) - 1] = '\0'; // Ensure null-termination
    }

    json_object_put(root);
    return 0;
}