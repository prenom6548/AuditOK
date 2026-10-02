#ifndef _KY_UDFT_TOOLS_INTERFACE_
#define _KY_UDFT_TOOLS_INTERFACE_
#include <string>

using namespace  std;

//init blank write udf1.02 image
int init_udf102_image(std::string data_path, std::string volume_id, std::string output_path);

int build_append_udf102_image(std::string device_path, int disc_offset, std::string data_path, std::string volume_id, std::string output_path);

#endif
