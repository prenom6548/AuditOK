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

TEST(WechatOpsConfigureTest, ConfigureSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
	GError *err = NULL;
	g_key_file_load_from_file(bioconf, "/etc/biometric-auth/biometric-drivers.conf", G_KEY_FILE_KEEP_COMMENTS, &err);
    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    
    free(dev);
    g_key_file_free(bioconf);
}

TEST(WechatOpsDriverInitTest, DriverInitSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    int ret = bio_drv_wechat_ops_driver_init(dev);
    EXPECT_EQ(ret, 0);
    free(dev);
}

TEST(WechatOpsDiscoverTest, DiscoverSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    int ret = bio_drv_wechat_ops_discover(dev);
    EXPECT_EQ(ret, 1);
    free(dev);
}

TEST(WechatOpsFreeTest, FreeSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    bio_drv_wechat_ops_free(dev);
    free(dev);
}

TEST(WechatOpsOpenTest, OpenSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
	GError *err = NULL;
	g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);
    free(dev);
}

TEST(WechatOpsCloseTest, CloseSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    bio_drv_wechat_ops_close(dev);
    // 检查是否正确释放了 frameFD

    free(dev);
    g_key_file_free(bioconf);
}


TEST(WechatOpsGetFeatureListTest, GetFeatureListSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    feature_info * ret3 = bio_drv_wechat_ops_get_feature_list(dev,ACTION_START,1000,0,-1);

    free(dev);
    g_key_file_free(bioconf);
}

TEST(WechatOpsStopByUserTest, StopByUserSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    bio_drv_wechat_ops_stop_by_user(dev,5000);
    free(dev);
}

TEST(WechatOpsGetOpsResultMesgTest, GetOpsResultMesgSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    const char * ret3 = bio_drv_wechat_ops_get_ops_result_mesg(dev);

    free(dev);
    g_key_file_free(bioconf);
}

TEST(WechatOpsGetDevStatusMesgTest, GetDevStatusMesgSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    const char * ret3 = bio_drv_wechat_ops_get_dev_status_mesg(dev);

    free(dev);
    g_key_file_free(bioconf);
}

TEST(WechatOpsGetNotifyMidMesgTest, GetNotifyMidMesgSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    const char * ret3 = bio_drv_wechat_ops_get_notify_mid_mesg(dev);

    free(dev);
    g_key_file_free(bioconf);
}

TEST(WechatOpsClean, CleanSuccess){
    bio_dev *dev = (bio_dev *)malloc(sizeof(bio_dev));
    GKeyFile * bioconf = NULL;
    bioconf = g_key_file_new();
    GError *err = NULL;
    g_key_file_load_from_file(bioconf, DRIVERS_CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS, &err);

    int ret = ops_configure(dev, bioconf);
    EXPECT_EQ(ret, 0);
    int ret2 = bio_drv_wechat_ops_open(dev);
    EXPECT_EQ(ret2, 0);

    int ret3 = bio_drv_wechat_ops_clean(dev,ACTION_START,1000,1,1);
    free(dev);
}