#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>
#include "fidoukey_internal.h"

#ifndef RANDOM_DEV
#define RANDOM_DEV "/dev/urandom"
#endif



// 分配空间
void *fidoukey_buf_alloc(unsigned long size)
{
    void *buf = malloc(size);
    memset(buf, 0, size);

    return buf;
}

// 释放空间
void fidoukey_buf_free(void *buf)
{
    if (buf != NULL)
        free(buf);

    return;
}

// 清零空间
void fidoukey_buf_clean(void *buf, unsigned long size)
{
    memset(buf, 0, size);

    return;
}

size_t
strlcpy(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0)
    {
        while (--n != 0)
        {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0'; /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return (s - src - 1); /* count does not include NUL */
}

int hex_decode(const char *ascii_hex, unsigned char **blob,
                      size_t *blob_len)
{
    *blob = NULL;
    *blob_len = 0;

    if (ascii_hex == NULL || (strlen(ascii_hex) % 2) != 0)
        return (0);

    *blob_len = strlen(ascii_hex) / 2;
    *blob = calloc(1, *blob_len);
    if (*blob == NULL)
        return (0);

    for (size_t i = 0; i < *blob_len; i++)
    {
        unsigned int c;
        int n = -1;
        int r = sscanf(ascii_hex, "%02x%n", &c, &n);
        if (r != 1 || n != 2 || c > UCHAR_MAX)
        {
            free(*blob);
            *blob = NULL;
            *blob_len = 0;
            return (0);
        }
        (*blob)[i] = (unsigned char)c;
        ascii_hex += n;
    }

    return (1);
}

int cose_type(const char *str, int *type)
{
    if (strcasecmp(str, "es256") == 0)
    {
        *type = COSE_ES256;
    }
    else if (strcasecmp(str, "rs256") == 0)
    {
        *type = COSE_RS256;
    }
    else if (strcasecmp(str, "eddsa") == 0)
    {
        *type = COSE_EDDSA;
    }
    else
    {
        *type = 0;
        return 0;
    }

    return 1;
}

int random_bytes(void *buf, size_t cnt)
{
    int fd;
    ssize_t n;

    fd = open(RANDOM_DEV, O_RDONLY);
    if (fd < 0)
        return (0);

    n = read(fd, buf, cnt);
    close(fd);
    if (n < 0 || (size_t)n != cnt)
        return (0);

    return (1);
}

void reset_pk(struct pk *pk)
{
    if (pk->type == COSE_ES256)
    {
        es256_pk_free((es256_pk_t **)&pk->ptr);
    }
    else if (pk->type == COSE_RS256)
    {
        rs256_pk_free((rs256_pk_t **)&pk->ptr);
    }
    else if (pk->type == COSE_EDDSA)
    {
        eddsa_pk_free((eddsa_pk_t **)&pk->ptr);
    }
    memset(pk, 0, sizeof(*pk));
}

int parse_pk(int old, char* type, const char *pk,
                    struct pk *out) {
  unsigned char *buf = NULL;
    size_t buf_len;
    int ok = 0;
    int r;

    reset_pk(out);

    if (old)
    {
        if (!hex_decode(pk, &buf, &buf_len))
        {
            printf("Failed to decode public key");
            goto err;
        }
    }
    else
    {
        bio_print_debug("decode buf\n");
        if (!b64_decode(pk, (void **)&buf, &buf_len))
        {
            bio_print_error("Failed to decode public key");
            goto err;
        }
    }

    if (!cose_type(type, &out->type))
    {
        bio_print_error("Unknown COSE type '%s'", type);
        goto err;
    }

    // For backwards compatibility, failure to pack the public key is not
    // returned as an error.  Instead, it is handled by fido_verify_assert().
    if (out->type == COSE_ES256)
    {
        if ((out->ptr = es256_pk_new()) == NULL)
        {
            printf("Failed to allocate ES256 public key");
            goto err;
        }
        if (old)
        {
            printf("old\n");
            //   r = translate_old_format_pubkey(out->ptr, buf, buf_len);
        }
        else
        {
            printf("es256 pk from ptr\n");
            r = es256_pk_from_ptr(out->ptr, buf, buf_len);
        }
        if (r != FIDO_OK)
        {
            printf("Failed to convert ES256 public key");
        }
    }
    else if (out->type == COSE_RS256)
    {
        if ((out->ptr = rs256_pk_new()) == NULL)
        {
            printf("Failed to allocate RS256 public key");
            goto err;
        }
        r = rs256_pk_from_ptr(out->ptr, buf, buf_len);
        if (r != FIDO_OK)
        {
            printf("Failed to convert RS256 public key");
        }
    }
    else if (out->type == COSE_EDDSA)
    {
        if ((out->ptr = eddsa_pk_new()) == NULL)
        {
            printf("Failed to allocate EDDSA public key");
            goto err;
        }
        r = eddsa_pk_from_ptr(out->ptr, buf, buf_len);
        if (r != FIDO_OK)
        {
            printf("Failed to convert EDDSA public key");
        }
    }
    else
    {
        printf("COSE type '%s' not handled", type);
        goto err;
    }

    ok = 1;
err:
    free(buf);

    return ok;
}

char *cose_string(int type)
{
    switch (type)
    {
    case COSE_ES256:
        return "es256";
    case COSE_RS256:
        return "rs256";
    case COSE_EDDSA:
        return "eddsa";
    default:
        return "unknown";
    }
}

int
verify_assert(int type, const unsigned char *authdata_ptr, size_t authdata_len,
    const unsigned char *sig_ptr, size_t sig_len, bool up, bool uv, int ext,
    const char *key, unsigned char *cdh,char *username)
{
	fido_assert_t	*assert = NULL;
    EC_KEY *ec = NULL;
    RSA *rsa = NULL;
    EVP_PKEY *eddsa = NULL;
    es256_pk_t *es256_pk = NULL;
    // es384_pk_t	*es384_pk = NULL;
    rs256_pk_t *rs256_pk = NULL;
    eddsa_pk_t *eddsa_pk = NULL;
    struct pk pks;

    // unsigned char cdh[32];
    void *pk;
    int r;
    memset(&pks, 0, sizeof(pks));

    if (!parse_pk(0, "eddsa", key, &pks))
    {
        bio_print_error("Failed to parse public key");
        return 0;
    }
    /* credential pubkey */
    bio_print_debug("key:%s\n", key);
    bio_print_debug("%d,%d\n", authdata_len, sig_len);

    if ((assert = fido_assert_new()) == NULL)
    	bio_print_error("fido_assert_new");

    /* client data hash */
    // r = fido_assert_set_clientdata_hash(assert, cdh, sizeof(cdh));
    r = fido_assert_set_clientdata_hash(assert, cd, sizeof(cd));
    if (r != FIDO_OK)
    	bio_print_error("fido_assert_set_clientdata: %s (0x%x)", fido_strerr(r), r);

    // /* relying party */
    r = fido_assert_set_rp(assert, username);
    if (r != FIDO_OK)
    	bio_print_error("fido_assert_set_rp: %s (0x%x)", fido_strerr(r), r);
    // /* authdata */
    r = fido_assert_set_count(assert, 1);
    if (r != FIDO_OK)
    	bio_print_error("fido_assert_set_count: %s (0x%x)", fido_strerr(r), r);
    r = fido_assert_set_authdata(assert, 0, authdata_ptr, authdata_len);
    if (r != FIDO_OK)
    	bio_print_error("fido_assert_set_authdata: %s (0x%x)", fido_strerr(r), r);
    // /* extension */
    r = fido_assert_set_extensions(assert, ext);
    if (r != FIDO_OK)
    	bio_print_error("fido_assert_set_extensions: %s (0x%x)", fido_strerr(r),
    	    r);

    // /* user presence */
    if (up && (r = fido_assert_set_up(assert, FIDO_OPT_TRUE)) != FIDO_OK)
    	bio_print_error("fido_assert_set_up: %s (0x%x)", fido_strerr(r), r);

    // /* user verification */
    if (uv && (r = fido_assert_set_uv(assert, FIDO_OPT_TRUE)) != FIDO_OK)
    	bio_print_error("fido_assert_set_uv: %s (0x%x)", fido_strerr(r), r);

    /* sig */
    r = fido_assert_set_sig(assert, 0, sig_ptr, sig_len);
    if (r != FIDO_OK)
    	bio_print_error("fido_assert_set_sig: %s (0x%x)", fido_strerr(r), r);

    r = fido_assert_verify(assert, 0, pks.type, pks.ptr);
    if (r != FIDO_OK)
        {bio_print_error("fido_assert_verify: %s (0x%x)", fido_strerr(r), r);
        return -1;}
    if(r==FIDO_OK)
        {printf("success\n");
        return 0;}
    // printf("success\n");

    es256_pk_free(&es256_pk);
    // es384_pk_free(&es384_pk);
    rs256_pk_free(&rs256_pk);
    eddsa_pk_free(&eddsa_pk);

    fido_assert_free(&assert);
}

int b64_decode(const char *in, void **ptr, size_t *len) {
  BIO *bio_mem = NULL;
  BIO *bio_b64 = NULL;
  size_t alloc_len;
  int n;
  int ok = 0;

  if (in == NULL || ptr == NULL || len == NULL || strlen(in) > INT_MAX)
    return (0);

  *ptr = NULL;
  *len = 0;

  bio_b64 = BIO_new(BIO_f_base64());
  if (bio_b64 == NULL)
    goto fail;

  bio_mem = BIO_new_mem_buf((const void *) in, -1);
  if (bio_mem == NULL)
    goto fail;

  BIO_set_flags(bio_b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(bio_b64, bio_mem);

  alloc_len = strlen(in);
  *ptr = calloc(1, alloc_len);
  if (*ptr == NULL)
    goto fail;

  n = BIO_read(bio_b64, *ptr, (int) alloc_len);
  if (n < 0 || BIO_eof(bio_b64) == 0)
    goto fail;

  *len = (size_t) n;
  ok = 1;

fail:
  BIO_free(bio_b64);
  BIO_free(bio_mem);

  if (!ok) {
    free(*ptr);
    *ptr = NULL;
    *len = 0;
  }

  return (ok);
}