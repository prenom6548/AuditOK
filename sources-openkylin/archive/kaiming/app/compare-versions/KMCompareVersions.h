/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

 #ifndef __KMCOMPAREVERSIONS_H__
 #define __KMCOMPAREVERSIONS_H__
 
 #include <string>
 #include <memory>
 
 #include "common/KMSubCommand.h"

 class KMCompareVersions : public KMSubCommand
 {
     DECLARE_DYNCREATE(KMCompareVersions);
 
 public:
     explicit KMCompareVersions();
     virtual ~KMCompareVersions();
 
     int dispose(int argc, char **argv) override;
 
 private:
     void init(int argc, char **argv);
     int run();
 
 private:
     class Options;
     class Private;
     std::unique_ptr<Private> d;
 };
 
 #endif // !__KMCOMPAREVERSIONS_H__
 