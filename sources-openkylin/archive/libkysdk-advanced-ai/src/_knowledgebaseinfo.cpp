#include "_knowledgebaseinfo.h"

#include <cstring>

#include "knowledgebaseinfo.h"
#include "utils.h"

KnowledgeBaseInfo *kb_knowledgebase_info_new(const char *name,
                                             const char *description) {
    if (name == nullptr) {
        return nullptr;
    }
    std::string desc = description ? description : "";
    return new KnowledgeBaseInfo{std::string(name), desc};
}

void kb_knowledgebase_info_free(KnowledgeBaseInfo *knowledgebase) {
    if (knowledgebase == nullptr) {
        return;
    }
    delete knowledgebase;
}

const char *kb_knowledgebase_info_get_name(
    const KnowledgeBaseInfo *knowledgebase) {
    if (knowledgebase == nullptr) {
        return nullptr;
    }
    return knowledgebase->name.c_str();
}

const char *kb_knowledgebase_info_get_description(
    const KnowledgeBaseInfo *knowledgebase) {
    if (knowledgebase == nullptr) {
        return nullptr;
    }
    return knowledgebase->description.c_str();
}

const char *kb_knowledgebase_info_get_id(
    const KnowledgeBaseInfo *knowledgebase) {
    if (knowledgebase == nullptr) {
        return nullptr;
    }
    return knowledgebase->knowledgeBaseId.c_str();
}

void kb_knowledgebase_info_set_name(KnowledgeBaseInfo *knowledgebase,
                                    const char *name) {
    if (knowledgebase == nullptr) {
        return;
    }
    knowledgebase->name = name;
}

void kb_knowledgebase_info_set_id(KnowledgeBaseInfo *knowledgebase,
                                  const char *knowledgebase_id) {
    if (knowledgebase == nullptr) {
        return;
    }
    knowledgebase->knowledgeBaseId = knowledgebase_id;
}

void kb_knowledgebase_info_set_description(KnowledgeBaseInfo *knowledgebase,
                                           const char *description) {
    if (knowledgebase == nullptr) {
        return;
    }
    knowledgebase->description = description;
}

KnowledgeBaseInfo **kb_knowledgebase_info_array_new(int capacity) {
    if (capacity <= 0) {
        return nullptr;
    }
    return new KnowledgeBaseInfo *[capacity]();
}

void kb_knowledgebase_info_array_free(
    KnowledgeBaseInfo **knowledgebase_info_array, int count) {
    if (!knowledgebase_info_array || count <= 0) {
        return;
    }
    for (int i = 0; i < count; i++) {
        if (knowledgebase_info_array[i]) {
            delete knowledgebase_info_array[i];
        }
    }
    delete[] knowledgebase_info_array;
}
