# 安全问题和答案处理库

## 概述

本库提供了一系列函数，用于处理和管理用户安全问题及其答案。这些功能主要用于密码找回流程，通过预设的安全问题和答案来验证用户身份。

## 宏定义

- `KPR_ERROR`: 错误状态码。
- `KPR_SUCCESS`: 成功状态码。
- `KPR_QUESTION_NUM`: 预设安全问题的数量。
- `KPR_QUESION_LEN`: 安全问题文本的最大长度。
- `KPR_MG_QUESION_LEN`: 问题ID的最大长度。
- `KPR_ANSWER_LEN`: 加密后答案的最大长度。

## 数据结构

### 安全问题

- `SecurityQuestion`: 安全问题结构体，包含问题ID和问题文本。

### 安全问题链表节点

- `SecurityQuestionListNode`: 安全问题链表节点，包含`SecurityQuestion`和指向下一个节点的指针。

### 安全问题答案

- `SecurityAnswer`: 安全问题答案结构体，包含用户ID、问题ID和加密后的答案。

### 安全问题答案链表节点

- `SecurityAnswerListNode`: 安全问题答案链表节点，包含`SecurityAnswer`和指向下一个节点的指针。

## 函数接口

### 获取预设的安全问题列表

- **函数名：** `getPresetSecurityQuestions`
- **输入参数：** 无
- **输出参数：** `count` - 问题个数
- **返回值：** 安全问题链表

### 绑定用户安全问题答案

- **函数名：** `setUserSecurityAnswers`
- **输入参数：** `head` - 用户安全问题链表
- **返回值：** 绑定结果，1表示失败，0表示成功

### 检查用户是否绑定安全问题

- **函数名：** `IsUserSecurityAnswers`
- **输入参数：** `uid` - 用户ID
- **返回值：** 绑定状态，1表示已绑定，0表示未绑定

### 获取用户绑定的安全问题列表

- **函数名：** `getUserSecurityQuestions`
- **输入参数：** `uid` - 用户ID
- **输出参数：** `count` - 问题个数
- **返回值：** 用户绑定的安全问题链表

### 校验用户安全问题及答案

- **函数名：** `verifyUserSecurityAnswers`
- **输入参数：** `head` - 用户安全答案链表
- **输出参数：** `token` - API token
- **返回值：** 校验结果，1表示正确，0表示错误

### 安全问题答案加密

- **函数名：** `KPR_PKCS5_PBKDF2_HMAC2`
- **输入参数：** `password`, `plen`, `salt`, `slen`, `iteration_count`, `key_length`
- **输出参数：** `output` - 加密后的密钥
- **返回值：** 加密结果，1表示失败，0表示成功

### 字符串转换为16进制

- **函数名：** `Hex`
- **输入参数：** `cpSrcStr`, `len`, `isUpper`
- **返回值：** 16进制字符串

### 安全问题答案加密 简化版v2

- **函数名：** `KPR_PKCS5_PBKDF2_HMAC2_v2`
    - **输入参数：** `password`, `plen`
    - **输出参数：** `output`
    - **返回值：** 加密结果，1表示失败，0表示成功

### 增加安全问题节点到链表

- **函数名：** `addQuestionNode`
- **输入参数：** `head` - 链表头指针, `data`, `dataSize`
- **返回值：** 操作结果，0表示成功，-1表示失败

### 增加安全问题答案节点到链表

- **函数名：** `addAnswerNode`
- **输入参数：** `head` - 链表头指针, `data`, `dataSize`
- **返回值：** 操作结果，0表示成功，-1表示失败

### 打印安全问题链表

- **函数名：** `printQuestionList`
- **输入参数：** `head` - 链表头指针
- **返回值：** 操作结果，0表示成功，-1表示失败

### 打印安全问题答案链表

- **函数名：** `printAnswerList`
- **输入参数：** `head` - 链表头指针
- **返回值：** 操作结果，0表示成功，-1表示失败

### 重新排序安全问题答案链表

- **函数名：** `sortAnswerList`
- **输入参数：** `head` - 链表头指针
- **返回值：** 操作结果，0表示成功，-1表示失败

### 用户密码修改

- **函数名：** `changePassword`
- **输入参数：** `username`, `password`, `token`
- **返回值：** 操作结果，0表示成功，1表示失败
