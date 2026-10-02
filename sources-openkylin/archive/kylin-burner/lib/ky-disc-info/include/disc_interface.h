#ifndef _DISC_MULTI_SESSION_INFO_
#define _DISC_MULTI_SESSION_INFO_

#include <string>

int getMultiSeeionParameters(std::string devcie_name, int64_t* last_session_start_pos, int64_t* next_session_start_pos);

#endif

