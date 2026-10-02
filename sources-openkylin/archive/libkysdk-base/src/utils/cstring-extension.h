/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */


#ifndef KYSDK_BASE_UTILS_STREXT_H__
#define KYSDK_BASE_UTILS_STREXT_H__


/** @defgroup C语言字符串扩展模块
  * @{
  */


/**
 * @file cstring-extension.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief KYSDK C语言工具模块字符串扩展
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (c) 2021
 * @since 1.0.0
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <locale.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief 对给定的字符串进行strip操作，删减字符串前后的指定字符；注意该操作会修改原字符串
 * 
 * @param str 需要进行strip的字符串指针
 * @param ch 需要删除的字符
 */
static inline void strstrip(char *str, char ch)
{
    if (strnlen(str, STRNLEN_MAX) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && *startPos == ch)
        startPos++;
    if (*startPos == '\0')
    {
        str[0] = 0;
        return;
    }

    char *endPos = str + strnlen(str, STRNLEN_MAX) - 1;
    while (endPos != str && *endPos == ch)
        endPos --;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

/**
 * @brief 删除给定字符串前后的空格、制表符、换行符，注意该操作会修改原字符串
 * 
 * @param str 需要进行strip操作的字符串指针
 */
static inline void strstripspace(char *str)
{
    if (strnlen(str, STRNLEN_MAX) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && isspace(*startPos))
        startPos++;
    if (*startPos == '\0')
    {
        str[0] = 0;
        return;
    }

    char *endPos = str + strnlen(str, STRNLEN_MAX) - 1;
    while (endPos != str && isspace(*endPos))
        endPos --;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

/**
 * @brief 删除给定字符串前后的空格和水平制表符（tab），注意该操作会修改原字符串
 * 
 * @param str 需要修改的字符串指针
 */
static inline void strstripblank(char *str)
{
    if (strnlen(str, STRNLEN_MAX) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && isblank(*startPos))
        startPos++;
    if (*startPos == '\0')
    {
        str[0] = 0;
        return;
    }

    char *endPos = str + strnlen(str, STRNLEN_MAX) - 1;
    while (endPos != str && isblank(*endPos))
        endPos --;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

/**
 * @brief 跳过字符串前的所有空格、制表符、换行符；该操作不会修改原字符串
 * 
 * @param p 指向原字符串的指针
 * @return const char* 指向跳过space后的字符串指针
 */
static inline const char *strskipspace(const char *p)
{
    while (isspace(*p))
		++p;
	return p;
}

/**
 * @brief 跳过字符串前的所有空格和水平制表符（tab）；该操作不会修改原字符串
 * 
 * @param p 指向原字符串的指针
 * @return const char* 指向跳过space后的字符串指针
 */
static inline const char *strskipblank(const char *p)
{
	while (isblank(*p))
		++p;
	return p;
}

/**
 * @brief 将字符串中的所有小写字母转化为大写字母；注意该操作会修改原字符串
 * 
 * @param str 需要操作的字符串指针
 */
static inline void str2upper(char *str)
{
    char *pos = str;
    while (*pos)
    {
        if (isalpha(*pos))
            *pos &= 0xDF;
        pos ++;
    }
}

/**
 * @brief 将字符串中的所有大写字母转化为小写字母；注意该操作会修改原字符串
 * 
 * @param str 需要操作的字符串指针
 */
static inline void str2lower(char *str)
{
    char *pos = str;
    while (*pos)
    {
        if (isalpha(*pos))
            *pos |= 0x20;
        pos ++;
    }
}

/**
 * @brief 在给定的字符串中查找给定字符第一次出现的位置；计数从0开始
 * 
 * @param str 原字符串
 * @param ch 需要查找的字符
 * @return int 第一次出现的位置，若未找到给定的字符，则返回-1
 */
static inline int strfirstof(const char* str, char ch)
{
    const char* pos = str;
    while (*pos)
    {
        if (*pos == ch)
            return pos - str;
        
        pos ++;
    }
    return -1;
}

/**
 * @brief 在给定的字符串中查找给定字符最后一次出现的位置；计数从0开始
 * 
 * @param str 原字符串
 * @param ch 需要查找的字符
 * @return int 最后一次出现的位置，若未找到给定的字符，则返回-1
 */
static inline int strlastof(const char* str, char ch)
{
    const char* pos = str;
    while (*pos)
        pos ++;
    pos --;
    while (pos != str)
    {
        if (*pos == ch)
            return pos - str;
	pos --;
    }
    return -1;
}

/**
 * @brief 判断str是否以prefix开头；注意该函数区分大小写
 * 
 * @param str 原字符串
 * @param prefix 需要匹配的字符串前缀
 * @return int 若str以prefix开头，则返回0；否则返回1
 */
static inline int strstartswith(const char *str, const char *prefix)
{
    size_t sz = prefix ? strnlen(prefix, STRNLEN_MAX) : 0;

    if (str && sz && strncmp(str, prefix, sz) == 0)
        return 0;
    return 1;
}

/**
 * @brief 判断str是否以prefix开头；不区分大小写
 * 
 * @param str 原字符串
 * @param prefix 需要匹配的字符串前缀
 * @return int 若str以prefix开头，则返回0；否则返回1
 */
static inline int strstartswith_nocase(const char *str, const char *prefix)
{
    size_t sz = prefix ? strnlen(prefix, STRNLEN_MAX) : 0;

    if (str && sz && strncasecmp(str, prefix, sz) == 0)
        return 0;
    return 1;
}

/**
 * @brief 判断str是否以postfix结尾；注意该函数区分大小写
 * 
 * @param str 原字符串
 * @param postfix 需要匹配的字符串后缀
 * @return int 若str以postfix结尾，则返回0；否则返回1
 */
static inline int strendwith(const char *str, const char *postfix)
{
    size_t sl = str ? strnlen(str, STRNLEN_MAX) : 0;
	size_t pl = postfix ? strnlen(postfix, STRNLEN_MAX) : 0;

	if (pl == 0)
		return 0;
	if (sl < pl)
		return 1;
	if (memcmp(str + sl - pl, postfix, pl) != 0)
		return 1;
	return 0;   
}

static inline int strendwith_nocase(const char *str, const char *postfix)
{
    return 0;
}

/**
 * @brief 统计给定字符在字符串中出现的次数
 * 
 * @param str 原字符串
 * @param ch 需要统计的字符
 * @return size_t 字符出现的次数
 */
static inline size_t strcounts(const char *str, char ch)
{
    const char *p = str;
    int counts = 0;
    while (*p)
    {
        if (*p == ch)
            counts ++;
        p ++;
    }

    return counts;
}

/**
 * @brief 对原字符串以给定的分隔符进行分割，注意该函数会修改原字符串
 * 
 * @param str 需要分割的字符串
 * @param delim 分隔符
 * @return char** 分割后的字符串列表，以NULL结尾。存储分割后所有字符串的字符串列表本身是由alloc申请的内存，因此当使用
 * 完成后应当被free；而分割出来的各个字符串不是申请的内存，而是分别指向了原字符串中的特定位置，因此他们不需要被分别free
 */
static inline char** strsplit(char *str, char delim)
{
    size_t size = strcounts(str, delim) + 1;
    char **res = (char **)calloc(1, sizeof(char *) * (size + 1));
	if (!res)
		return NULL;
    if (size < 2)
    {
        res[0] = str;
        return res;
    }
    char *leftstr;
    res[0] = strtok_r(str, &delim, &leftstr);
    for (size_t i = 1; i < size; i ++)
    {
        res[i] = strtok_r(NULL, &delim, &leftstr);
    }

    return res;
}

/**
 * @brief 判断字符串str是否是数字
 * 
 * @param str 字符串
 * @return bool 是-true，否-false
 */
static inline bool strisdigit(char *str)
{
    if(!str)
        return false;
    const char* pos = str;
    while (*pos)
    {
        if (!isdigit(*pos))
        {
            return false;
        }
        pos ++;
    }
    return true;
}

/**
 * @brief 判断字符串str是否是空格符
 * 
 * @param str 字符串
 * @return bool 是-true，否-false
 */
static inline bool strisspace(char *str)
{
    if(!str)
        return false;
    
    size_t space_size = strlen(str);
    if(!space_size)
        return false;

    const char* pos = str;
    while (*pos)
    {
        if (*pos != 0x20)
        {
            return false;
        }
        pos ++;
    }
    return true;
}

/**
 * @brief 判断字符串str是否是空白符
 * 
 * @param str 字符串
 * @return bool 是-true，否-false
 */
static inline bool striswhitespace(char *str)
{
    if(!str)
        return true;
    
    size_t space_size = strlen(str);
    if(!space_size)
        return true;
    
    const char* pos = str;
    while (*pos)
    {
        if (!isspace(*pos))
        {
            return false;
        }
        pos ++;
    }
    return true;
}

/**
 * @brief 判断字符串str是否是字母
 * 
 * @param str 字符串
 * @return bool 是-true，否-false
 */
static inline bool strisalpha(char *str)
{
    if(!str)
        return false;

    size_t space_size = strlen(str);
    if(0 == space_size)
        return false;

    const char* pos = str;
    while (*pos)
    {
        if (!isalpha(*pos))
        {
            return false;
        }
        pos ++;
    }
    return true;
}

// 定义 Unicode 范围结构体
typedef struct {
    unsigned int start;
    unsigned int end;
    const char* language;
} UnicodeBlock;

// 定义语言的 Unicode 范围
static UnicodeBlock language_blocks[] = {
    {0x0590, 0x05FF, "he"},    // 希伯来语
    {0x0600, 0x06FF, "ar"},    // 阿拉伯语 (基本)
    {0x0750, 0x077F, "ar"},    // 阿拉伯语 (扩展)
    {0xFE70, 0xFEFF, "ar"},    // 阿拉伯语 (表示形式)
    {0x0400, 0x04FF, "kk/ky"}, // 哈萨克语/吉尔吉斯语 (西里尔字母)
	{0x4E00, 0x9FFF, "zh"},        // 中文 (汉字基础)
    {0x3400, 0x4DBF, "zh"},        // 中文 (汉字扩展 A)
    {0x20000, 0x2A6DF, "zh"},      // 中文 (汉字扩展 B)
    {0x2A700, 0x2B73F, "zh"},      // 中文 (汉字扩展 C)
    {0x2B740, 0x2B81F, "zh"},      // 中文 (汉字扩展 D)
    {0x2B820, 0x2CEAF, "zh"},      // 中文 (汉字扩展 E)
	{0xF900, 0xFAFF, "zh"},        // 中文 (中日韩统一表意字符)
	{0x3040, 0x309F, "ja"},        // 日文平假名
    {0x30A0, 0x30FF, "ja"},        // 日文片假名
    {0x1100, 0x11FF, "ko"},        // 韩文音节
    {0x3130, 0x318F, "ko"},        // 韩文字母
    {0xA960, 0xA97F, "ko"},        // 韩文兼容字母
    {0xAC00, 0xD7AF, "ko"},        // 韩文音节
    {0x3130, 0x318F, "ko"}         // 韩文兼容音节
};

// 查找字符对应的语言
static inline const char* detect_language(unsigned int codepoint) 
{
    int num_blocks = sizeof(language_blocks) / sizeof(UnicodeBlock);

    // 遍历 Unicode 块，查找字符所属的语言范围
    for (int i = 0; i < num_blocks; i++)
	{
        if (codepoint >= language_blocks[i].start && codepoint <= language_blocks[i].end) 
		{
            return language_blocks[i].language;
        }
    }
    return NULL;
}

/**
 * @brief 判断字符串str是否是表意文字
 * 
 * @param str 字符串
 * @return bool 是-true，否-false
 */
static inline bool strisideograph(char *str)
{
	setlocale(LC_ALL, "");
	int zh_counts = 0;
	int ja_counts = 0;
	int ko_counts = 0;

    size_t len = mbstowcs(NULL, str, 0) + 1;  // 获取宽字符长度
    wchar_t* wide_str = (wchar_t*) malloc(len * sizeof(wchar_t));
    mbstowcs(wide_str, str, len);

    // 遍历字符串并检测每个字符的语言
    for (const wchar_t* p = wide_str; *p != L'\0'; ++p) 
	{
        unsigned int codepoint = (unsigned int) *p;
        const char* language = detect_language(codepoint);
		if(language)
		{
			if (strcmp(language, "zh") == 0)
			{
				zh_counts++;
			}
			else if (strcmp(language, "ja") == 0)
			{
				ja_counts++;
			} 
			else if (strcmp(language, "ko") == 0) 
			{
				ko_counts++;
			}
		}
    }

	if(ja_counts != 0 || ko_counts != 0)
	{
		return false;
	}
	else if(ja_counts == 0 && ko_counts == 0 && zh_counts != 0)
	{
		return true;
	}

	return false;
}

/**
 * @brief 判断字符串str是否是从右到左语言的字符
 * 
 * @param str 字符串
 * @return bool 是-true，否-false
 */
static inline bool strisrtl(char *str)
{
	setlocale(LC_ALL, "");
	int ar_counts = 0;
	int he_counts = 0;
	int kk_counts = 0;

    size_t len = mbstowcs(NULL, str, 0) + 1;  // 获取宽字符长度
    wchar_t* wide_str = (wchar_t*) malloc(len * sizeof(wchar_t));
    mbstowcs(wide_str, str, len);

    // 遍历字符串并检测每个字符的语言
    for (const wchar_t* p = wide_str; *p != L'\0'; ++p) 
	{
        unsigned int codepoint = (unsigned int) *p;
        const char* language = detect_language(codepoint);
		if(language)
		{
			if (strcmp(language, "ar") == 0)
			{
				ar_counts++;
			} 
			else if (strcmp(language, "he") == 0)
			{
				he_counts++;
			} 
			else if (strcmp(language, "kk/ky") == 0) 
			{
				kk_counts++;
			}
		}
    }

	if(ar_counts == 0 && he_counts == 0 && kk_counts == 0)
	{
		return false;
	}
	return true;
}

#ifdef __cplusplus
}
#endif

#endif
/**
  * @}
  */
