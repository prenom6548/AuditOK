#include <iostream>
#include <string>
#include <stdio.h>


#include "inttypes.h"
#include "udfappender.h"
#include "ecma167.h"
#include "interface.h"
#include "udfread.h"
#include "blockinput.h"

using namespace  std;

//init blank write udf1.02 image
int init_udf102_image(std::string data_path, std::string volume_id, std::string output_path){
    //init UDF1.02 burn
    UdfAppender udf_appender;
    udf_appender.SetOffsetIndex(0);
    bool is_ok = udf_appender.PreAppendBuild(data_path, volume_id, output_path);
    if(is_ok){
        udf_appender.InitBurnUDF();
        return 0;
    }
    return -1;
}

int build_append_udf102_image(std::string device_path, int disc_offset, std::string data_path, std::string volume_id, std::string output_path){

    UdfAppender udf_appender;
    udfread *udf = (udfread *)calloc(1, sizeof(udfread));
    if (!udf) {
        return -1;
    }

    if (udfread_open(udf, device_path.c_str()) < 0) {
        fprintf(stderr, "udfread_open failed\n");
        udfread_close(udf);
        return -2;
    }

    UDFDIR *root  = udfread_opendir(udf, "/");
    if (!root) {
        fprintf(stderr, "error opening root directory\n");
        udfread_close(udf);
        return -3;
    }

    FileNode* prev_file_tree = new FileNode();
    read_file_tree(udf,prev_file_tree, udf->input->udfdisc.fsd.rootDirectoryICB);

    udf_appender.SetOffsetIndex(disc_offset);
    udf_appender.SetUdfImageHeader(udf->input->udfdisc);
    udf_appender.SetLVID(udf->input->lvid);
    udf_appender.SetFileTree(prev_file_tree);
    udf_appender.PreAppendBuild(data_path, volume_id, output_path);
    udf_appender.BuildAppendUDF();
    udfread_close(udf);

    return 0;
}



