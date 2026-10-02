#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <biometric_common.h> 
#include <biometric_stroge.h>
#include <biometric_version.h>
#include "wechat_internal.h"
#include "wechat_realization.h"
#include "wechat_type.h"
#define CONFIG_DIR "/etc/biometric-auth"
#define DRIVERS_CONFIG_FILE	CONFIG_DIR "/biometric-drivers.conf"
extern "C"{
    int ops_configure(bio_dev * dev, GKeyFile * conf);
}

TEST(WechatOpsEnrollTest, EnrollSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    int ret3 = bio_drv_wechat_ops_enroll(dev,ACTION_START,1003,1,"Index1");
    EXPECT_EQ(ret3, 0);

    free(dev);
    g_key_file_free(bioconf);
}


