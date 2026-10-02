#ifndef fidoukey_INTERNAL_H
#define fidoukey_INTERNAL_H

#include <stdio.h>

#include "fidoukey_type.h"

#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <limits.h>

#ifdef __cplusplus
extern "C"{
#endif

struct enroll_response
{
    uint8_t flags;
    uint8_t *public_key;
    size_t public_key_len;
    uint8_t *key_handle;
    size_t key_handle_len;
    uint8_t *signature;
    size_t signature_len;
    uint8_t *attestation_cert;
    size_t attestation_cert_len;
    uint8_t *authdata;
    size_t authdata_len;
};

struct pk {
  void *ptr;
  int type;
};

// 定义操作结果提示
enum gdxfp_ops_mid {
    MID_OPS_EXTENDED_MESSAGE = OPS_COMM_MAX + 1,
    MID_OPS_EXCEED_ENROOL_MAX,
};

void *fidoukey_buf_alloc(unsigned long size);
void fidoukey_buf_free(void *buf);
void fidoukey_buf_clean(void *buf, unsigned long size);

char *fidoukey_face_feature_capture(bio_dev *dev);
size_t strlcpy(char *dst, const char *src, size_t siz);
int random_bytes(void *buf, size_t cnt);
int verify_assert(int type, const unsigned char *authdata_ptr, size_t authdata_len,
    const unsigned char *sig_ptr, size_t sig_len, bool up, bool uv, int ext,
    const char *key, unsigned char *cdh,char *username);
int parse_pk(int old, char* type, const char *pk,
                    struct pk *out);
int b64_decode(const char *in, void **ptr, size_t *len);
char *cose_string(int type);


#ifdef __cplusplus
}
#endif

#endif // DRIVER_INTERNAL_H
