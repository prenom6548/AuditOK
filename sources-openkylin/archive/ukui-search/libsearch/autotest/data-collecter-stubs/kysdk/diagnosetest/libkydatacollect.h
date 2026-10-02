#ifndef LIBKYDATACOLLECT_H
#define LIBKYDATACOLLECT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    KEVENTSOURCE_DESKTOP = 1,
    KEVENT_CLICK = 2
};

typedef struct KCustomProperty {
    char *key;
    char *value;
} KCustomProperty;

typedef struct KTrackData {
    int source;
    int event;
} KTrackData;

KTrackData *kdk_dia_data_init(int source, int event);
void kdk_dia_append_custom_property(KTrackData *node, KCustomProperty *property, int count);
void kdk_dia_upload_default(KTrackData *node, char *eventName, char *pluginName);
void kdk_dia_data_free(KTrackData *node);

#ifdef __cplusplus
}
#endif

#endif // LIBKYDATACOLLECT_H
