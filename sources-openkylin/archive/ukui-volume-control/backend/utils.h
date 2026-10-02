#pragma once

#include <spa/utils/dict.h>
#include <spa/pod/pod.h>
#include <iostream>

namespace UkuiAudioFramwork
{
void print_dict(const struct spa_dict* dictionary);

uint32_t spa_dict_get_u32(const spa_dict* props, const char* key);

std::ostream& operator<< (std::ostream& out, const spa_pod& pod);
}
