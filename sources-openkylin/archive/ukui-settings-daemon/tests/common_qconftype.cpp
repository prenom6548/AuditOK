#include <gtest/gtest.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "QGSettings/qconftype.h"




class qconfTypeTest : public ::testing::Test {
protected:
   void SetUp() override {
       // 初始化测试环境
//       syslog_init("test-category", LOG_LOCAL0);
   }

   void TearDown() override {
       // 清理测试环境
   }
};

TEST_F(qconfTypeTest, qconf_types_convertTest) {

}

TEST_F(qconfTypeTest, qconf_types_collect_from_variantTest) {

}


TEST_F(qconfTypeTest, qconf_types_to_qvariantTest) {

}


TEST_F(qconfTypeTest, qtify_nameTest) {

}


TEST_F(qconfTypeTest, unqtify_nameTest) {

}



