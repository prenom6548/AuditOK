#include <glib.h>
#include <gio/gio.h>

typedef struct {
    gchar *device;
    gboolean can_eject;
    gboolean drive_can_stop;
} UkuiTestFakeVolume;

typedef struct {
    gchar *uri;
    gboolean root_available;
} UkuiTestFakeMount;

static gboolean g_monitor_available = FALSE;
static gint g_monitor_marker = 0;
static GList *g_fake_volumes = NULL;
static GList *g_fake_mounts = NULL;

static void ukui_test_free_fake_volume(gpointer data)
{
    UkuiTestFakeVolume *volume = data;
    if (!volume) {
        return;
    }
    g_free(volume->device);
    g_free(volume);
}

static void ukui_test_free_fake_mount(gpointer data)
{
    UkuiTestFakeMount *mount = data;
    if (!mount) {
        return;
    }
    g_free(mount->uri);
    g_free(mount);
}

void ukui_test_gio_reset(void)
{
    g_monitor_available = FALSE;
    g_list_free_full(g_fake_volumes, ukui_test_free_fake_volume);
    g_fake_volumes = NULL;
    g_list_free_full(g_fake_mounts, ukui_test_free_fake_mount);
    g_fake_mounts = NULL;
}

void ukui_test_gio_clear_volumes(void)
{
    g_list_free_full(g_fake_volumes, ukui_test_free_fake_volume);
    g_fake_volumes = NULL;
}

void ukui_test_gio_set_monitor_available(int available)
{
    g_monitor_available = available ? TRUE : FALSE;
}

void ukui_test_gio_add_volume(const char *device, int can_eject, int drive_can_stop)
{
    UkuiTestFakeVolume *volume = g_new0(UkuiTestFakeVolume, 1);
    volume->device = g_strdup(device);
    volume->can_eject = can_eject ? TRUE : FALSE;
    volume->drive_can_stop = drive_can_stop ? TRUE : FALSE;
    g_fake_volumes = g_list_append(g_fake_volumes, volume);
}

GMount *ukui_test_gio_create_mount(const char *uri, int root_available)
{
    UkuiTestFakeMount *mount = g_new0(UkuiTestFakeMount, 1);
    mount->uri = g_strdup(uri);
    mount->root_available = root_available ? TRUE : FALSE;
    g_fake_mounts = g_list_append(g_fake_mounts, mount);
    return (GMount *)mount;
}

GVolumeMonitor *g_volume_monitor_get(void)
{
    return g_monitor_available ? (GVolumeMonitor *)&g_monitor_marker : NULL;
}

GList *g_volume_monitor_get_volumes(GVolumeMonitor *monitor)
{
    (void)monitor;
    return g_fake_volumes;
}

gulong g_signal_connect_data(gpointer instance,
                             const gchar *detailed_signal,
                             GCallback c_handler,
                             gpointer data,
                             GClosureNotify destroy_data,
                             GConnectFlags connect_flags)
{
    (void)instance;
    (void)detailed_signal;
    (void)c_handler;
    (void)data;
    (void)destroy_data;
    (void)connect_flags;
    return 1;
}

void g_signal_handler_disconnect(gpointer instance, gulong handler_id)
{
    (void)instance;
    (void)handler_id;
}

void g_object_unref(gpointer object)
{
    (void)object;
}

char *g_volume_get_identifier(GVolume *volume, const char *kind)
{
    (void)kind;
    return g_strdup(((UkuiTestFakeVolume *)volume)->device);
}

gboolean g_volume_can_eject(GVolume *volume)
{
    return ((UkuiTestFakeVolume *)volume)->can_eject;
}

GDrive *g_volume_get_drive(GVolume *volume)
{
    return (GDrive *)volume;
}

gboolean g_drive_can_stop(GDrive *drive)
{
    return ((UkuiTestFakeVolume *)drive)->drive_can_stop;
}

GFile *g_mount_get_root(GMount *mount)
{
    if (!mount) {
        return NULL;
    }
    return ((UkuiTestFakeMount *)mount)->root_available ? (GFile *)mount : NULL;
}

char *g_file_get_uri(GFile *file)
{
    return g_strdup(((UkuiTestFakeMount *)file)->uri);
}

char *g_filename_from_uri(const char *uri, char **hostname, GError **error)
{
    (void)hostname;
    (void)error;
    if (g_str_has_prefix(uri, "file://")) {
        return g_strdup(uri + 7);
    }
    return NULL;
}
