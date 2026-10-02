#ifndef FIDOUKEY_REALIZATION_H
#define FIDOUKEY_REALIZATION_H

#include "biometric_common.h"


#ifdef __cplusplus
extern "C"{
#endif
int fidoukey_ops_configure(bio_dev *dev, GKeyFile * conf);

int bio_drv_fidoukey_ops_driver_init(bio_dev *dev);

void bio_drv_fidoukey_ops_free(bio_dev *dev);

int bio_drv_fidoukey_ops_discover(bio_dev *dev);

int bio_drv_fidoukey_ops_open(bio_dev *dev);

void bio_drv_fidoukey_ops_close(bio_dev *dev);

char *bio_drv_fidoukey_ops_capture(bio_dev *dev, OpsActions action);

int bio_drv_fidoukey_ops_enroll (bio_dev *dev, OpsActions action, int uid, int idx, char * bio_idx_name);

int bio_drv_fidoukey_ops_verify (bio_dev *dev, OpsActions action, int uid, int idx);

int bio_drv_fidoukey_ops_identify (bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);

feature_info *bio_drv_fidoukey_ops_search (bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);

int bio_drv_fidoukey_ops_clean (bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);

feature_info *bio_drv_fidoukey_ops_get_feature_list (bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);

int bio_drv_fidoukey_ops_stop_by_user (bio_dev *dev, int waiting_ms);

const char *bio_drv_fidoukey_ops_get_dev_status_mesg (bio_dev *dev);

const char *bio_drv_fidoukey_ops_get_ops_result_mesg (bio_dev *dev);

const char *bio_drv_fidoukey_ops_get_notify_mid_mesg (bio_dev *dev);

void bio_drv_fidoukey_ops_attach(bio_dev *dev);

void bio_drv_fidoukey_ops_detach(bio_dev *dev);


#ifdef __cplusplus
}
#endif


#endif // SEETAFACE_REALIZATION_H
