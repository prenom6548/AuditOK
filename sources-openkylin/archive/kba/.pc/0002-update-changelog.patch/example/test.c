#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <libintl.h>
#include <locale.h>

#include "../include/kba/kba.h"
#include "../lib/pbkdf2_hmac.h"

int uniqueNumbers[3] = {0};

int randnumber() {
    int numbers[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};    
    int count = 0;

    srand((unsigned int)time(NULL));

    for (int i = 9; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }

    while (count < 3) {
        int randomIndex = rand() % 10;
        int randomNumber = numbers[randomIndex];

        int isUnique = 1;
        for (int k = 0; k < count; ++k) {
            if (uniqueNumbers[k] == randomNumber) {
                isUnique = 0;
                break;
            }
        }

        if (isUnique) {
            uniqueNumbers[count++] = randomNumber;
        }
    }

    printf("Three unique random numbers (0-9): ");
    for (int i = 0; i < 3; ++i) {
        printf("%d ", uniqueNumbers[i]);
    }
    printf("\n");

    return 0;
}

SecurityAnswerListNode *bindSecurityQuestionsAnswer(SecurityQuestionListNode *head, int count, int uid, char * enc_answer){
    
    SecurityQuestionListNode *current = head;
    SecurityAnswerListNode *ret = NULL;
    
    unsigned char* key = (unsigned char*)"xyza";

    randnumber();
    sleep(1);

    char hmac[33] = { 0 };
    KPR_PKCS5_PBKDF2_HMAC2((unsigned char*)enc_answer, strlen(enc_answer), key, 4, 2, 32, (unsigned char*)hmac);
    char *hexBlk = Hex(hmac, 32, 0);
        
    for (int i = 0; i < count; i++){
        SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = uid;
        answer.questionId = uniqueNumbers[i];
        strcpy(answer.encryptedAnswer, hexBlk);

        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

    }

    return ret;
}

SecurityAnswerListNode *bindSecurityQuestionsAnswerv2(SecurityQuestionListNode *head, int count, int uid, char * enc_answer){
    
    SecurityQuestionListNode *current = head;
    SecurityAnswerListNode *ret = NULL;
    char *hexBlk = NULL;
    char enc_buf[1024] = {0};
    
    //unsigned char* key = (unsigned char*)"xyza";

    randnumber();
    sleep(1);

    //char hmac[33] = { 0 };
    //char *hexBlk = Hex(hmac, 32, 0);
    int tmpuniqueNumbers[3] = {5,3,7};
        
    for (int i = 0; i < 1; i++){
        sprintf(enc_buf, "%s %d %d", enc_answer, tmpuniqueNumbers[0], i);
        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)enc_buf, strlen(enc_buf), &hexBlk);
        SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = uid;
        answer.questionId = tmpuniqueNumbers[0];
        strcpy(answer.encryptedAnswer, hexBlk);

        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

        sprintf(enc_buf, "%s %d %d", enc_answer, tmpuniqueNumbers[1], i);
        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)enc_buf, strlen(enc_buf), &hexBlk);
        //SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = uid;
        answer.questionId = tmpuniqueNumbers[1];
        strcpy(answer.encryptedAnswer, hexBlk);

        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

        sprintf(enc_buf, "%s %d %d", enc_answer, tmpuniqueNumbers[2], i);
        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)enc_buf, strlen(enc_buf), &hexBlk);
        //SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = uid;
        answer.questionId = tmpuniqueNumbers[2];
        strcpy(answer.encryptedAnswer, hexBlk);

        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

    }

    return ret;
}

SecurityAnswerListNode *_bindSecurityQuestionsAnswerv2(SecurityQuestionListNode *head, int count, int uid, char * enc_answer){
    
    SecurityQuestionListNode *current = head;
    SecurityAnswerListNode *ret = NULL;
    char *hexBlk = NULL;
    char enc_buf[1024] = {0};
    
    //unsigned char* key = (unsigned char*)"xyza";

    randnumber();
    sleep(1);

    //char hmac[33] = { 0 };
    //char *hexBlk = Hex(hmac, 32, 0);
    int tmpuniqueNumbers[3] = {7,3,5};
        
    for (int i = 0; i < 1; i++){
        sprintf(enc_buf, "%s %d %d", enc_answer, tmpuniqueNumbers[0], i);
        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)enc_buf, strlen(enc_buf), &hexBlk);
        SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = uid;
        answer.questionId = tmpuniqueNumbers[0];
        strcpy(answer.encryptedAnswer, hexBlk);

        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

        sprintf(enc_buf, "%s %d %d", enc_answer, tmpuniqueNumbers[1], i);
        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)enc_buf, strlen(enc_buf), &hexBlk);
        //SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = uid;
        answer.questionId = tmpuniqueNumbers[1];
        strcpy(answer.encryptedAnswer, hexBlk);

        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

        sprintf(enc_buf, "%s %d %d", enc_answer, tmpuniqueNumbers[2], i);
        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)enc_buf, strlen(enc_buf), &hexBlk);
        //SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = uid;
        answer.questionId = tmpuniqueNumbers[2];
        strcpy(answer.encryptedAnswer, hexBlk);

        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

    }

    return ret;
}

//TEST(PKCS5_PBKDF2_HMAC_Test, BasicTest)
int TEST1(void ) {
    int ret = 0;
    unsigned char* key = (unsigned char*)"key";
    char blk[100] = { 0 };
    strcpy(blk, "The quick brown fox jumps over the lazy dog");
    char hmac[21] = { 0 };
    PKCS5_PBKDF2_HMAC((unsigned char*)blk, 43, key, 3, 10, 20, (unsigned char*)hmac);
    char *hexBlk = Hex(hmac, 20, 0);
    ret = memcmp("2ca6b06c1ed5599af0546fc0067d1712e236b1de", hexBlk,20*2);
    if (ret == 0) {
        printf("BasicTest: success\n");
    }else
    {
        printf("BasicTest: failed\n");
    }
    printf("BasicTest: %s len=%ld\n", hexBlk, strlen(hexBlk));
}

//TEST(PKCS5_PBKDF2_HMAC_Test, BasicTest1)
int TEST2(void ) {
    int ret = 0;
    unsigned char* key = (unsigned char*)"salt";
    char blk[100] = { 0 };
    strcpy(blk, "password");
    char hmac[21] = { 0 };
    PKCS5_PBKDF2_HMAC((unsigned char*)blk, 8, key, 4, 2, 20, (unsigned char*)hmac);
    char *hexBlk = Hex(hmac, 20, 0);
    //EXPECT_EQ(string("ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957"), hexBlk);
    ret = memcmp("ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957", hexBlk,20*2);
    if (ret == 0) {
        printf("BasicTest: success\n");
    }else
    {
        printf("BasicTest: failed\n");
    }
    printf("BasicTest: %s len=%ld\n", hexBlk, strlen(hexBlk));
}

//TEST(PKCS5_PBKDF2_HMAC_Test, BasicTestSha256_1) 
int TEST3(void ) {
    int ret = 0;
    unsigned char* key = (unsigned char*)"salt";
    char blk[100] = { 0 };
    strcpy(blk, "password");
    char hmac[33] = { 0 };
    KPR_PKCS5_PBKDF2_HMAC2((unsigned char*)blk, 8, key, 4, 2, 32, (unsigned char*)hmac);
    char *hexBlk = Hex(hmac, 32, 0);
    //EXPECT_EQ(string("ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43"), hexBlk);
    ret = memcmp("ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43", hexBlk,32*2);
    if (ret == 0) {
        printf("BasicTest: success\n");
    }else
    {
        printf("BasicTest: failed\n");
    }
    printf("BasicTest: %s len=%ld\n", hexBlk, strlen(hexBlk));
}

//TEST(PKCS5_PBKDF2_HMAC_Test, BasicTestSha256_2)
int TEST4(void ) {
    int ret = 0;
    unsigned char* key = (unsigned char*)"salt";
    char blk[100] = { 0 };
    strcpy(blk, "password");
    char hmac[33] = { 0 };
    KPR_PKCS5_PBKDF2_HMAC2((unsigned char*)blk, 8, key, 4, 1, 32, (unsigned char*)hmac);
    char *hexBlk = Hex(hmac, 32, 0);
    //EXPECT_EQ(string("120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b"), hexBlk);
    ret = memcmp("120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b", hexBlk,32*2);
    if (ret == 0) {
        printf("BasicTest: success\n");
    }else
    {
        printf("BasicTest: failed\n");
    }
    printf("BasicTest: %s len=%ld\n", hexBlk, strlen(hexBlk));
}

//TEST(PKCS5_PBKDF2_HMAC_Test, BasicTestSha512_2)
int TEST5(void) {
    int ret = 0;
    unsigned char* key = (unsigned char*)"salt";
    char blk[100] = { 0 };
    strcpy(blk, "password");
    char hmac[65] = { 0 };
    KPR_PKCS5_PBKDF2_HMAC2((unsigned char*)blk, 8, key, 4, 1, 64, (unsigned char*)hmac);
    char *hexBlk = Hex(hmac, 64, 0);
    //EXPECT_EQ(string("120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b4dbf3a2f3dad3377264bb7b8e8330d4efc7451418617dabef683735361cdc18c"), hexBlk);
    ret = memcmp("120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b4dbf3a2f3dad3377264bb7b8e8330d4efc7451418617dabef683735361cdc18c", hexBlk,64*2);
    if (ret == 0) {
        printf("BasicTest: success\n");
    }else
    {
        printf("BasicTest: failed\n");
    }
    printf("BasicTest: %s len=%ld\n", hexBlk, strlen(hexBlk));
}

//TEST(TdesTest, BasicTest) 
int TEST6(void ) {

    char key[16]={"key"};
    char encpt[100];
    char result[100];
    char plain[] = "1234567890";
    int len = strlen(plain);
    cipher2(key, plain, encpt, len);
    decipher2(key, result, encpt, strlen(encpt));
    printf("BasicTest1:\n plain =%s\n encpt=%s\n result=%s\n", plain, encpt, result);

    cipher3(key, plain, encpt,len);
    decipher3(key, result, encpt, strlen(encpt));
    printf("BasicTest2:\n plain =%s\n encpt=%s\n result=%s\n", plain, encpt, result);
}

#define _(string) gettext(string)

int main(int argc, char *argv[])
{
    int ret=0;
    int count = 0;
    char *token = NULL;
    SecurityQuestionListNode *preset_list = NULL;
    SecurityAnswerListNode *zky_ans_list = NULL;
    SecurityAnswerListNode *zky_ans_list1 = NULL;
    SecurityAnswerListNode *user1_list = NULL; /*uid == 1001*/
    SecurityAnswerListNode *user2_list = NULL; /*uid == 1002*/
    SecurityQuestionListNode *zky_qst_list = NULL; 

    bindtextdomain("kba", "/usr/share/locale");
	textdomain("kba");
	setlocale(LC_ALL, "zh_CN.UTF-8");

    if(geteuid() != 0)
    {
        printf("kba-test: Permission denied, current euid is %d. \n",geteuid());
        return -1;
    }
    
    //init_question_to_jsonfile();

    /* 接口1--获取预设的安全问题列表，问题个数10个 */
    preset_list = getPresetSecurityQuestions(&count);
    printf("count = %d\n", count);

    printQuestionList(preset_list);
    
    SecurityQuestionListNode *current = preset_list;
    while (current != NULL) {
        printf("questionId: %d, questionText: %s\n", current->question.questionId, _(current->question.questionText));
        current = current->next;
    }
    printf("-------------接口1测试完毕-------------\n");

    //user1_list = bindSecurityQuestionsAnswerv2(preset_list, 3, 1001, "usr1xxx");
    //printAnswerList(user1_list);
    /*接口2*/
    //setUserSecurityAnswers(user1_list);

    zky_ans_list = bindSecurityQuestionsAnswerv2(preset_list, 3, 1000, "zky123456");
    printAnswerList(zky_ans_list);
    
    /*接口2*/
    setUserSecurityAnswers(zky_ans_list);

    //user2_list = bindSecurityQuestionsAnswerv2(preset_list, 3, 1002, "usr2xxxxx");
    //printAnswerList(user2_list);
    /*接口2*/
    //setUserSecurityAnswers(user2_list);
    printf("-------------接口2测试完毕-------------\n");

    /*接口3*/
    ret = IsUserSecurityAnswers(1000);
    if (ret == 1){
        printf("user zky has security answer\n");
    }
    
    ret = IsUserSecurityAnswers(1001);
    if (ret == 1){
        printf("user usr1 has security answer\n");
    }
    
    //ret = IsUserSecurityAnswers(1002);
    //if (ret == 1){
    //    printf("user use2 has security answer\n");
    //}
    printf("-------------接口3测试完毕-------------\n");

    /*接口4*/
    if (ret == 1){
        zky_qst_list = getUserSecurityQuestions(1000, &count);
        printQuestionList(zky_qst_list);
    }
    printf("-------------接口4测试完毕-------------\n");

    zky_ans_list1 = _bindSecurityQuestionsAnswerv2(preset_list, 3, 1000, "zky123456");
    printAnswerList(zky_ans_list1);

    /*接口5*/
    if (ret == 1){
        int is_ok = verifyUserSecurityAnswers(zky_ans_list1, &token);
        printf("verifyUserSecurityAnswers: %s\n", is_ok == 1 ? "success" : "failed");
    }
    printf("token: %s\n", token);
    printf("-------------接口5测试完毕-------------\n");

    #if 0
    /*测试加密函数*/
    //TEST1();
    //TEST2();
    //TEST3();
    //TEST4();
    //TEST5();
    //TEST6();

    /*答案链表排序*/
    SecurityAnswer answer[3];
    answer[0].userId = 1000;
    answer[1].userId = 1000;
    answer[2].userId = 1000;
    answer[0].questionId = 9;
    memcpy(answer[0].encryptedAnswer, (unsigned char*)"0234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef", 64);
    answer[1].questionId = 5;
    memcpy(answer[1].encryptedAnswer, (unsigned char*)"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef", 64);
    answer[2].questionId = 0;
    memcpy(answer[2].encryptedAnswer, (unsigned char*)"2234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef", 64);
    SecurityAnswerListNode *answer_list = NULL;
    addAnswerNode(&answer_list, (void *)&answer[0], sizeof(answer[0]));
    addAnswerNode(&answer_list, (void *)&answer[1], sizeof(answer[1]));
    addAnswerNode(&answer_list, (void *)&answer[2], sizeof(answer[2]));

    sortAnswerList(&answer_list);
    printAnswerList(answer_list);
    #endif
    
    /*接口6*/
    sleep(20);
    ret = changePassword("test", "Kylin@123", token);
    printf("changePassword: %s\n", ret == 0 ? "success" : "failed");
    sleep(20);
    ret = changePassword("test", "Kylin@123", token);
    printf("changePassword: %s\n", ret == 0 ? "success" : "failed");
    printf("-------------接口6测试完毕-------------\n");

    return 0;
}
