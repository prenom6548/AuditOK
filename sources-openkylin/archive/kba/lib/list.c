#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "../include/kba/kba.h"

SecurityQuestionListNode* createQuestionNode(const void *data, size_t dataSize) {
    SecurityQuestionListNode *newNode = (SecurityQuestionListNode*)malloc(sizeof(SecurityQuestionListNode));
    if (newNode == NULL) {
        return NULL;
    }

    memcpy(&newNode->question, data, dataSize);
    newNode->next = NULL;
    return newNode;
}

SecurityAnswerListNode* createAnswerNode(const void *data, size_t dataSize) {
    SecurityAnswerListNode *newNode = (SecurityAnswerListNode*)malloc(sizeof(SecurityAnswerListNode));
    if (newNode == NULL) {
        return NULL;
    }

    memcpy(&newNode->answer, data, dataSize);
    newNode->next = NULL;
    return newNode;
}

_SecurityAnswerListNode* _createAnswerNode(const void *data, size_t dataSize) {
    _SecurityAnswerListNode *newNode = (_SecurityAnswerListNode*)malloc(sizeof(_SecurityAnswerListNode));
    if (newNode == NULL) {
        return NULL;
    }

    memcpy(&newNode->answer, data, dataSize);
    newNode->next = NULL;
    return newNode;
}

void freeQuestionList(SecurityQuestionListNode *head) {
    SecurityQuestionListNode *current = head;
    while (current != NULL) {
        SecurityQuestionListNode *next = current->next;
        free(current);
        current = next;
    }
}

void freeAnswerList(SecurityAnswerListNode *head) {
    SecurityAnswerListNode *current = head;
    while (current != NULL) {
        SecurityAnswerListNode *next = current->next;
        free(current);
        current = next;
    }
}

void _freeAnswerList(_SecurityAnswerListNode *head) {
    _SecurityAnswerListNode *current = head;
    while (current != NULL) {
        _SecurityAnswerListNode *next = current->next;
        free(current);
        current = next;
    }
}

int addQuestionNode(SecurityQuestionListNode **head, const void *data, size_t dataSize) {
    if (head == NULL || data == NULL) {
        return -1;
    }
    SecurityQuestionListNode *newNode = createQuestionNode(data, dataSize);
    if (newNode == NULL) {
        return -1;
    }
    if (*head == NULL) {
        *head = newNode;
    } else {
        SecurityQuestionListNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    return 0;
}

int addAnswerNode(SecurityAnswerListNode **head, const void *data, size_t dataSize) {
    if (head == NULL || data == NULL) {
        return -1;
    }
    SecurityAnswerListNode *newNode = createAnswerNode(data, dataSize);
    if (newNode == NULL) {
        return -1;
    }
    if (*head == NULL) {
        *head = newNode;
    } else {
        SecurityAnswerListNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    return 0;
}

int _addAnswerNode(_SecurityAnswerListNode **head, const void *data, size_t dataSize) {
    if (head == NULL || data == NULL) {
        return -1;
    }
    _SecurityAnswerListNode *newNode = _createAnswerNode(data, dataSize);
    if (newNode == NULL) {
        return -1;
    }
    if (*head == NULL) {
        *head = newNode;
    } else {
        _SecurityAnswerListNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    return 0;
}

int _updateAnswerNode(_SecurityAnswerListNode **head, const void *data, size_t dataSize) {
    if (head == NULL || data == NULL || dataSize == 0) {
        return -1;
    }

    _SecurityAnswerListNode *current = *head;
    while (current != NULL) {    
        if (current->answer.user_id == ((_SecurityAnswer*)data)->user_id) {
            memcpy(&current->answer, data, dataSize);
        }
        current = current->next;
    }
    return 0;
}

int printQuestionList(SecurityQuestionListNode *head) {
    if (head == NULL) {
        return -1;
    }
    SecurityQuestionListNode *current = head;
    while (current != NULL) {
        printf("questionId: %d, questionText: %s\n", current->question.questionId, current->question.questionText);

        current = current->next;
    }
    return 0;
}

int printAnswerList(SecurityAnswerListNode *head) {
    if (head == NULL) {
        return -1;
    }
    SecurityAnswerListNode *current = head;
    while (current != NULL) {
        printf("uid: = %d, queston: %d, answer: %s\n",current->answer.userId, current->answer.questionId, current->answer.encryptedAnswer);
        
        current = current->next;
    }
    return 0;
}

int _printAnswerList(_SecurityAnswerListNode *head) {
    if (head == NULL) {
        return -1;
    }
    _SecurityAnswerListNode *current = head;
    while (current != NULL) {
        printf("uid: = %d, questions: %s, enc_answer: %s\n",current->answer.user_id, current->answer.merge_question, current->answer.enc2_answer);
        
        current = current->next;
    }
    return 0;
}

void clearQuestionList(SecurityQuestionListNode *head) {
    if (head == NULL) {
        return;
    }
    SecurityQuestionListNode *current = head->next;
    while (current != NULL) {
        SecurityQuestionListNode *next = current->next;
        free(current);
        current = next;
    }
    head->next = NULL;
}

void clearAnswerList(SecurityAnswerListNode *head) {
    if (head == NULL) {
        return;
    }
    SecurityAnswerListNode *current = head->next;
    while (current != NULL) {
        SecurityAnswerListNode *next = current->next;
        free(current);
        current = next;
    }
    head->next = NULL;
}

void _clearAnswerList(_SecurityAnswerListNode *head) {
    if (head == NULL) {
        return;
    }
    _SecurityAnswerListNode *current = head->next;
    while (current != NULL) {
        _SecurityAnswerListNode *next = current->next;
        free(current);
        current = next;
    }
    head->next = NULL;
}

int sortAnswerList(SecurityAnswerListNode **head) {
    SecurityAnswerListNode *current, *tail = NULL;
    int swapped = 0;

    if (*head == NULL)
    {
        return -1;
    }
    
    do {
        swapped = 0;
        current = *head; 
        tail = NULL; 

        while (current != NULL && current->next != NULL) {

            if (current->answer.questionId > current->next->answer.questionId) {
                SecurityAnswer tempAnswer = current->answer;
                current->answer = current->next->answer;
                current->next->answer = tempAnswer;

                swapped = 1;
            }

            tail = current;
            current = current->next;
        }
    } while (swapped);

    return 0;
}

void reverseAnswerList(SecurityAnswerListNode **head) {
    SecurityAnswerListNode *prev = NULL;
    SecurityAnswerListNode *current = *head;
    SecurityAnswerListNode *next = NULL;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    *head = prev;
}
