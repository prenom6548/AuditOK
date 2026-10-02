#include "utils.h"

#include <uuid/uuid.h>

namespace utils {

std::string generateUUID()
{
    uuid_t uuid;
    char uuid_str[37];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);
    std::string id = uuid_str;

    return id;
}

}
