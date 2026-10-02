#ifndef _CS0_ENCODE_H_
#define _CS0_ENCODE_H_

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>


static int enable_log = 0;
static int enable_trace = 0;

#define udf_error(...)   do {                   fprintf(stderr, "udfread ERROR: " __VA_ARGS__); } while (0)
#define udf_log(...)     do { if (enable_log)   fprintf(stderr, "udfread LOG  : " __VA_ARGS__); } while (0)
#define udf_trace(...)   do { if (enable_trace) fprintf(stderr, "udfread TRACE: " __VA_ARGS__); } while (0)



static void *_safe_realloc(void *p, size_t s)
{
    void *result = realloc(p, s);
    if (!result) {
        udf_error("out of memory\n");
        free(p);
    }
    return result;
}


#define utf16lo_to_mutf8(out, out_pos, out_size, ch)    \
  do {                                              \
    if (ch != 0 && ch < 0x80) {                     \
      out[out_pos++] = (uint8_t)ch;                 \
    } else {                                        \
      out_size++;                                   \
      out = (uint8_t *)_safe_realloc(out, out_size);\
      if (!out) return NULL;                        \
                                                    \
      out[out_pos++] = 0xc0 | (ch >> 6);            \
      out[out_pos++] = 0x80 | (ch & 0x3f);          \
    }                                               \
  } while (0)

#define utf16_to_mutf8(out, out_pos, out_size, ch)      \
  do {                                                  \
    if (ch < 0x7ff) {                                   \
      utf16lo_to_mutf8(out, out_pos, out_size, ch);     \
    } else {                                            \
      out_size += 2;                                    \
      out = (uint8_t *)_safe_realloc(out, out_size);    \
      if (!out) return NULL;                            \
                                                        \
      out[out_pos++] = 0xe0 | (ch >> 12);               \
      out[out_pos++] = 0x80 | ((ch >> 6) & 0x3f);       \
      out[out_pos++] = 0x80 | (ch & 0x3f);              \
                                                        \
    }                                                   \
  } while (0)


static size_t encode_utf8(uint8_t *out, const char *in, size_t outlen)
{
    size_t inlen = strlen(in);
    size_t len, i;
    int utf_cnt;
    uint32_t utf_char, max_val;
    unsigned int c;

    len = 1;
    out[0] = 8;
    max_val = 0x7F;

try_again:
    utf_cnt = 0;
    utf_char = 0;

    for (i=0; i<inlen; i++)
    {
        c = in[i];

        /* Complete a multi-byte UTF-8 character */
        if (utf_cnt)
        {
            if ((c & 0xC0) != 0x80)
                goto error_invalid;
            utf_char = (utf_char << 6) | (c & 0x3F);
            if (--utf_cnt)
                continue;
        }
        else
        {
            /* Check for a multi-byte UTF-8 character */
            if (c & 0x80)
            {
                /* Start a multi-byte UTF-8 character */
                if ((c & 0xE0) == 0xC0)
                {
                    utf_char = c & 0x1fU;
                    utf_cnt = 1;
                }
                else if ((c & 0xF0) == 0xE0)
                {
                    utf_char = c & 0x0F;
                    utf_cnt = 2;
                }
                else if ((c & 0xF8) == 0xF0)
                {
                    utf_char = c & 0x07;
                    utf_cnt = 3;
                }
                else if ((c & 0xFC) == 0xF8)
                {
                    utf_char = c & 0x03;
                    utf_cnt = 4;
                }
                else if ((c & 0xFE) == 0xfc)
                {
                    utf_char = c & 0x01;
                    utf_cnt = 5;
                }
                else
                    goto error_invalid;
                continue;
            }
            else
            {
                /* Single byte UTF-8 character (most common) */
                utf_char = c;
            }
        }

        /* Choose no compression if necessary */
        if (utf_char > max_val)
        {
            if (max_val == 0x7F)
            {
                len = 1;
                max_val = 0xFFFF;
                out[0] = 0x10;
                goto try_again;
            }
            return (size_t)-1;
        }

        if (max_val == 0xFFFF)
        {
            if (len + 2 > outlen)
            {
                len = 1;
                max_val = 0xFF;
                out[0] = 0x8;
                goto try_again;
            }
            out[len++] = utf_char >> 8;
        }
        if (len + 1 > outlen)
            return (size_t)-1;
        out[len++] = utf_char & 0xFF;
    }

    if (utf_cnt)
    {
error_invalid:
        exit(1);
    }

    return len;
}



/* Strings, CS0 (UDF 2.1.1) */
static char *_cs0_to_mutf8(const uint8_t *cs0, size_t size)
{
    size_t   out_pos = 0;
    size_t   out_size = size;
    size_t   i;
    uint8_t *out;

    if (size < 1) {
        /* empty string */
        return (char*)calloc(1, 1);
    }

    out = (uint8_t *)malloc(size);
    if (!out) {
        udf_error("out of memory\n");
        return NULL;
    }

    switch (cs0[0]) {
    case 8:
        /*udf_trace("string in utf-8\n");*/
        for (i = 1; i < size; i++) {
            utf16lo_to_mutf8(out, out_pos, out_size, cs0[i]);
        }
        break;
    case 16:
        for (i = 1; i < size - 1; i+=2) {
            uint16_t ch = cs0[i + 1] | (cs0[i] << 8);
            utf16_to_mutf8(out, out_pos, out_size, ch);
        }
        break;
    default:
        udf_error("unregonized string encoding %u\n", cs0[0]);
        free(out);
        return NULL;
    }

    out[out_pos] = 0;
    return (char*)out;
}



#endif
