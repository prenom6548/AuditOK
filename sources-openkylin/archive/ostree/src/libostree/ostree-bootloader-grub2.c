/*
 * Copyright (C) 2014 Colin Walters <walters@verbum.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the licence or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "ostree-bootloader-grub2.h"
#include "ostree-sysroot-private.h"
#include "otutil.h"
#include <gio/gfiledescriptorbased.h>
#include <gio/gunixoutputstream.h>
#include <sys/mount.h>
#include <glib/gstdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
// Written by bootupd
#define BOOTUPD_CONFIG "boot/bootupd-state.json"
// Horrible hack, to avoid including a JSON parser we just grep for this
#define BOOTUPD_CONFIG_STATIC_JSON_FRAGMENT "\"static-configs\""
#define OSLOADER_CFG_PATH "/boot/efi/EFI/BOOT/OSLoader.cfg"

/* Maintain backwards compatibility with legacy GRUB
 * installations that might rely on the -16 suffix
 * for real-mode booting.
 * Allow us to override this at build time if we're
 * using a modern GRUB installation.
 */
//#if !defined(WITH_MODERN_GRUB) && (defined(__i386__) || defined(__x86_64__))
// #define GRUB2_SUFFIX "16"
// #else
// #define GRUB2_SUFFIX ""
// #endif
#define GRUB2_SUFFIX ""
/* Maintain backwards compatibility with legacy GRUB
 * installations that might rely on the -efi suffix
 * for RHEL/fedora-based distros.
 * Allow us to override this at build time if we're
 * using a modern GRUB installation that makes the
 * suffix-less linux/initrd commands synonymous for
 * both EFI and BIOS booting.
 */
#if defined(WITH_MODERN_GRUB) || defined(__aarch64__)
#define GRUB2_EFI_SUFFIX ""
#else
#define GRUB2_EFI_SUFFIX "efi"
#endif

#define LENOVO_X1_CARBON_PARAMETER "video=efifb:off"

struct _OstreeBootloaderGrub2
{
  GObject parent_instance;

  OstreeSysroot *sysroot;
  GFile *config_path_bios_1;
  GFile *config_path_bios_2;
  GFile *config_path_efi;
  gboolean is_efi;
};

#define TITLE_OVERRIDE_CONF "/etc/ostree-release.override"
#define BUILD_STAGE_RC "RC"
#define BUILD_STAGE_RELEASE "Release"

typedef GObjectClass OstreeBootloaderGrub2Class;

static void _ostree_bootloader_grub2_bootloader_iface_init (OstreeBootloaderInterface *iface);
G_DEFINE_TYPE_WITH_CODE (OstreeBootloaderGrub2, _ostree_bootloader_grub2, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (OSTREE_TYPE_BOOTLOADER,
                                                _ostree_bootloader_grub2_bootloader_iface_init));

static gboolean
_ostree_bootloader_grub2_query (OstreeBootloader *bootloader, gboolean *out_is_active,
                                GCancellable *cancellable, GError **error)
{
  OstreeBootloaderGrub2 *self = OSTREE_BOOTLOADER_GRUB2 (bootloader);

  g_autoptr (GFile) bootupd_config
      = g_file_resolve_relative_path (self->sysroot->path, BOOTUPD_CONFIG);
  if (g_file_query_exists (bootupd_config, NULL))
    {
      g_autofree char *bootupd_config_contents = NULL;
      if (!g_file_load_contents (bootupd_config, cancellable, &bootupd_config_contents, NULL, NULL,
                                 error))
        return glnx_prefix_error (error, "Failed to read bootupd config");
      if (strstr (bootupd_config_contents, BOOTUPD_CONFIG_STATIC_JSON_FRAGMENT) != NULL)
        {
          g_debug ("Found static bootupd config");
          *out_is_active = FALSE;
          return TRUE;
        }
    }

  /* Look for the BIOS path first */
  if (g_file_query_exists (self->config_path_bios_1, NULL)
      || g_file_query_exists (self->config_path_bios_2, NULL))
    {
      /* If we found it, we're done */
      *out_is_active = TRUE;
      return TRUE;
    }

  g_autoptr (GFile) efi_basedir
      = g_file_resolve_relative_path (self->sysroot->path, "boot/efi/EFI");

  g_clear_object (&self->config_path_efi);

  if (g_file_query_exists (efi_basedir, NULL))
    {
      g_autoptr (GFileEnumerator) direnum = NULL;

      direnum = g_file_enumerate_children (efi_basedir, OSTREE_GIO_FAST_QUERYINFO,
                                           G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, cancellable, error);
      if (!direnum)
        return FALSE;

      while (TRUE)
        {
          GFileInfo *file_info;
          const char *fname;

          if (!g_file_enumerator_iterate (direnum, &file_info, NULL, cancellable, error))
            return FALSE;
          if (file_info == NULL)
            break;

          fname = g_file_info_get_name (file_info);
          if (strcmp (fname, "BOOT") == 0)
            continue;

          if (g_file_info_get_file_type (file_info) != G_FILE_TYPE_DIRECTORY)
            continue;

          g_autofree char *subdir_grub_cfg
              = g_build_filename (gs_file_get_path_cached (efi_basedir), fname, "grub.cfg", NULL);

          if (g_file_test (subdir_grub_cfg, G_FILE_TEST_EXISTS))
            {
              self->config_path_efi = g_file_new_for_path (subdir_grub_cfg);
              break;
            }
        }

      /* If we found the EFI path, we're done */
      if (self->config_path_efi)
        {
          self->is_efi = TRUE;
          *out_is_active = TRUE;
          return TRUE;
        }
    }
  else
    *out_is_active = FALSE;

  return TRUE;
}

static const char *
_ostree_bootloader_grub2_get_name (OstreeBootloader *bootloader)
{
  return "grub2";
}

static const char *
_ostree_update_vmlinuz_sub(gchar *output_str, const char *title, const char *quoted_uuid,
                            const char *linuxTotal, const char *initrdTotal)
{
  GString *vmlinuz_update_sub = g_string_new(NULL);
  g_auto(GStrv) vmlinuz_tokens = g_strsplit(linuxTotal, " ", -1);
  g_auto(GStrv) initrd_tokens = g_strsplit(initrdTotal, " ", -1);

  if (g_strv_length(vmlinuz_tokens) < 2 || g_strv_length(initrd_tokens) < 2)
    return g_string_free(vmlinuz_update_sub, FALSE);

  char advance_title[256] = {0};
  snprintf(advance_title, sizeof(advance_title), "Advanced options for %s", title);

  g_auto(GStrv) old_output = g_strsplit(output_str, "\n", -1);

  GRegex *regex_title = g_regex_new("(menuentry\\s+')([^']+)('.*)", 0, 0, NULL);
  GRegex *regex_vmlinuz = g_regex_new("/ostree/[^ ]+/vmlinuz[^ ]*", 0, 0, NULL);
  GRegex *regex_initrd = g_regex_new("/ostree/[^ ]+/(initr(amfs|d)[^ ]+)", 0, 0, NULL);

  g_string_append_printf(vmlinuz_update_sub, "submenu %s $menuentry_id_option %s {\n",
                          g_shell_quote(advance_title), quoted_uuid);

  for (int i = 0; i < g_strv_length(vmlinuz_tokens); i++) {
    /* 从内核路径提取版本号 (e.g., 6.6.0-35-generic) */
    g_autofree gchar *kernel_version = NULL;
    if (g_regex_match(regex_vmlinuz, vmlinuz_tokens[i], 0, NULL)) {
      g_autofree char *basename = g_path_get_basename(vmlinuz_tokens[i]);
      if (g_str_has_prefix(basename, "vmlinuz-")) {
          kernel_version = g_strdup(basename + strlen("vmlinuz-")); /* Skip "vmlinuz-" prefix */
      }
    }

    for (int j = 0; old_output[j] != NULL; j++) {
      g_autofree char *replaced_str = NULL;
   
      if (g_regex_match(regex_title, old_output[j], 0, NULL) && kernel_version) {/* 修改菜单标题：添加内核版本 */
        g_autofree char *new_title = g_strdup_printf("%s, %s", title, kernel_version);
        g_autofree char *replacement = g_strdup_printf("\\1%s\\3", new_title);
        replaced_str = g_regex_replace(regex_title, old_output[j], -1, 0, replacement, 0, NULL);
      }else if (g_regex_match(regex_vmlinuz, old_output[j], 0, NULL)) {/*替换内核路径*/
        replaced_str = g_regex_replace(regex_vmlinuz, old_output[j], -1, 0, vmlinuz_tokens[i], 0, NULL);
      } else if (g_regex_match(regex_initrd, old_output[j], 0, NULL)) {/*替换initrd路径*/
        replaced_str = g_regex_replace(regex_initrd, old_output[j], -1, 0, initrd_tokens[i], 0, NULL);
      }

      if (replaced_str) {
        g_string_append(vmlinuz_update_sub, replaced_str);
      } else {
        g_string_append(vmlinuz_update_sub, old_output[j]);
      }
      g_string_append_c(vmlinuz_update_sub, '\n');
    }
  }

  g_string_append(vmlinuz_update_sub, "}\n");

  g_regex_unref(regex_title);
  g_regex_unref(regex_vmlinuz);
  g_regex_unref(regex_initrd);

  return g_string_free(vmlinuz_update_sub, FALSE);
}

static char*
extract_version_part(const char *title){
  if (!title)
    return NULL;
    
  gchar **parts = g_strsplit(title, " ", -1);
  char *result = NULL;

  if (g_strv_length(parts) >= 4) {
    result = g_strdup_printf("%s %s", parts[2], parts[3]);
  } else if (g_strv_length(parts) == 3) {
    result = g_strdup(parts[2]);
  }
  g_strfreev(parts);
  
  return result;
}

static const char* get_override_title(void)
{
  char *conf_path = TITLE_OVERRIDE_CONF;
  char *title;

  if (access(conf_path, F_OK) != 0)
    return NULL;

  GKeyFile *config = g_key_file_new();
  g_key_file_load_from_file(config, conf_path, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
  char *release_version = g_key_file_get_string(config, "info", "version", NULL);
  g_key_file_free(config);

  title = get_os_version(release_version);
  return title;
}

static const char *get_stage(const char *title)
{
    if (!title)
        return NULL;

    gchar **tokens = g_strsplit(title, " ", -1);
    const char *stage = NULL;

    if (tokens[3]) {
        if (g_str_has_prefix(tokens[3], "rc") || g_str_has_prefix(tokens[3], "RC"))
            stage = BUILD_STAGE_RC;
        else if (g_str_has_prefix(tokens[3], "release") || g_str_has_prefix(tokens[3], "Release"))
            stage = BUILD_STAGE_RELEASE;
    }

    g_strfreev(tokens);
    return stage;
}

static char *parse_title_version(const char *title)
{
    if (!title)
        return NULL;

    gchar **tokens = g_strsplit(title, " ", -1);
    char *version = NULL;

    if (tokens[0] && tokens[1] && tokens[2])
        version = g_strdup(tokens[2]);

    g_strfreev(tokens);
    return version;
}

static gboolean should_override(const char *current_title, const char *override_title)
{
    if (!override_title || *override_title == '\0')
        return FALSE;

    char *curr_ver = parse_title_version(current_title);
    char *ovr_ver  = parse_title_version(override_title);

    const char *curr_stage = get_stage(current_title);
    const char *ovr_stage  = get_stage(override_title);

    gboolean override = FALSE;

    if (curr_ver && ovr_ver && g_strcmp0(curr_ver, ovr_ver) != 0) {
      return FALSE;
    }

    if (curr_stage && ovr_stage) {
        if (g_strcmp0(curr_stage, BUILD_STAGE_RC) == 0 && g_strcmp0(ovr_stage, BUILD_STAGE_RELEASE) == 0) {
            override = TRUE;
        }
    }

    g_free(curr_ver);
    g_free(ovr_ver);

    return override;
}

static gchar* handle_system_restore_opt(const gchar* inputStr) {
  /* 【系统还原选项】 grub条目：System Restore，grub参数：linux行末尾加上restore */

  GString *outputStr = g_string_new(NULL);
  g_auto(GStrv) old_output = g_strsplit(inputStr, "\n", -1);

  GRegex *title_regex = g_regex_new("(menuentry\\s+')([^']*)('\\s+--class)",  G_REGEX_DOTALL, 0, NULL);
  GRegex *regex_vmlinuz = g_regex_new("/ostree/[^ ]+/vmlinuz[^ ]*", 0, 0, NULL);

  for (int index = 0; old_output[index] != NULL; index++) {
    g_autofree char *replaced_str = NULL;
    gboolean is_linux_line = FALSE;

    if(g_regex_match(title_regex, old_output[index], 0, NULL)){
        GMatchInfo *match_info = NULL;
        g_regex_match(title_regex, old_output[index], 0, &match_info);
        gchar *full_title = g_match_info_fetch(match_info, 2);
        char *version_part = extract_version_part(full_title);
        replaced_str = g_regex_replace(title_regex, old_output[index], -1, 0, g_strdup_printf("\\1%s %s\\3", _("System Restore - openkylin v3.0"), version_part ?: ""), 0, NULL);
        g_free(full_title);
        g_match_info_free(match_info);
    }else if (g_regex_match(regex_vmlinuz, old_output[index], 0, NULL)) {
        is_linux_line = TRUE;
    }

    if (replaced_str) {
        g_string_append(outputStr, replaced_str);
    } else {
        g_string_append(outputStr, old_output[index]);
    }

    if(is_linux_line){
      g_string_append(outputStr, "restore");
      is_linux_line = FALSE;
    }

    g_string_append_c(outputStr, '\n');
  }

  g_regex_unref(title_regex);
  g_regex_unref(regex_vmlinuz);

  return g_string_free(outputStr, FALSE);
}

static gchar* handle_restore_menu_opt(const gchar* inputStr) {
  GString *outputStr = g_string_new(NULL);
  g_auto(GStrv) old_output = g_strsplit(inputStr, "\n", -1);

  GRegex *title_regex = g_regex_new("(menuentry\\s+')([^']*)('\\s+--class)",  G_REGEX_DOTALL, 0, NULL);
  GRegex *regex_vmlinuz = g_regex_new("/ostree/[^ ]+/vmlinuz[^ ]*", 0, 0, NULL);

  for (int index = 0; old_output[index] != NULL; index++) {
    g_autofree char *replaced_str = NULL;
    if (g_regex_match(title_regex, old_output[index], 0, NULL)) {
        GMatchInfo *match_info = NULL;
        g_regex_match(title_regex, old_output[index], 0, &match_info);
        gchar *full_title = g_match_info_fetch(match_info, 2);
        char *version_part = extract_version_part(full_title);
        replaced_str = g_regex_replace(title_regex, old_output[index], -1, 0, g_strdup_printf("\\1%s %s\\3", _("System Restore Mode - openkylin v3.0"), version_part ?: ""), 0, NULL);
        g_free(full_title);
        g_match_info_free(match_info);
        g_string_append(outputStr, replaced_str);
        g_string_append_c(outputStr, '\n');
        break;
    }
  }

  g_regex_unref(title_regex);
  g_regex_unref(regex_vmlinuz);

  gchar *current = outputStr->str;
  gsize old_len = strlen("menuentry");
  gsize new_len = strlen("submenu");
  while ((current = strstr(current, "menuentry")) != NULL) {
    gsize pos = current - outputStr->str;
    g_string_erase(outputStr, pos, old_len);
    g_string_insert(outputStr, pos, "submenu");
    current = outputStr->str + pos + new_len;
  }

  const char *second_quote = strchr(outputStr->str, '\'');
  if (second_quote) {
    second_quote = strchr(second_quote + 1, '\''); 
  }
  const char *unrestricted_start = strstr(second_quote + 1, "--unrestricted");
  size_t start_pos = second_quote - outputStr->str + 1;
  size_t end_pos = unrestricted_start - outputStr->str;
  if (end_pos > start_pos) {
    g_string_erase(outputStr, start_pos, end_pos - start_pos);
    g_string_insert_c(outputStr, start_pos, ' ');
  }

  return g_string_free(outputStr, FALSE);
}

/* 
 * @Description: Determine if the deployment is a factory branch
 * @param: OstreeSysroot
 * @return: OstreeBootconfigParser
*/ 
static gboolean
_ostree_bootconfg_parser_is_restore_factory(OstreeSysroot *sysroot,
                                            OstreeBootconfigParser *config)
{
  g_autoptr(GPtrArray) deployments = ostree_sysroot_get_deployments(sysroot);

  if (deployments->len == 0)
  {
    return FALSE;
  }
  else
  {
    for (guint i = 0; i < deployments->len; i++)
    {
      OstreeDeployment *deployment = deployments->pdata[i];
      int deploy_version =atoi(ostree_bootconfig_parser_get (config,OSTREE_COMMIT_META_KEY_VERSION));
      if(ostree_deployment_get_index(deployment) == deployments->len - deploy_version)//找到了conf对应的deplotment
      {
        if(ostree_deployment_is_pinned(deployment))
          return TRUE;
      }
    }
    return FALSE;
  }
}

static char* trim_whitespace(char *str)
{/* 该函数用途：去除字符串两端空白 */

    if (!str) return NULL;
    
    /* 去除前导空白 */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    /* 全空白字符串 */
    if (*start == '\0') {
        return start;
    }
    /* 去除尾部空白 */
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    
    return start;
}

static char* get_value_by_key_from_file(const char* filename, const char* key)
{
    FILE* file = fopen(filename, "r");
    if (!file)
        return NULL;

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char* result = NULL;

    while ((read = getline(&line, &len, file)) != -1) {
        /* 查找键名位置 */
        char* key_start = strstr(line, key);
        if (!key_start) continue;

        /* 验证键名格式 */
        char* colon = strchr(key_start, ':');
        if (!colon) continue;

        /* 检查键名前是否有效 */
        if (key_start != line) {
            char prev_char = *(key_start - 1);
            /* 确保键名是独立单词 */
            if (!isspace((unsigned char)prev_char)) continue;
        }

        /* 提取值部分 */
        char* value_start = colon + 1;
        char* value = trim_whitespace(value_start); /* 去除字符串两端空白 */

        /* 处理可能的引号 */
        size_t val_len = strlen(value);
        if (val_len >= 2 && value[0] == '"' && value[val_len-1] == '"') {
            value[val_len-1] = '\0';
            result = strdup(value + 1);
        } else {
            result = strdup(value);
        }
        break;
    }

    free(line);
    fclose(file);
    return result;
}

static char *get_appScene(void)
{/* 先检查 /etc/LICENS 文件，尝试从中提取 APP_SCENE 字段的值；若未提取到则再检查 /etc/.kylin-osinfo 文件，尝试从中提取 APP_SCENE 字段的值。*/

    char *license_appScene = get_value_by_key_from_file("/etc/LICENSE", "APP_SCENE");
    if(license_appScene){
        g_debug("get %s by APP_SCENE from /etc/LICENSE", license_appScene);
        return license_appScene;
    }
    char *osinfo_appScene = get_value_by_key_from_file("/etc/.kylin-osinfo", "APP_SCENE");
    if(osinfo_appScene){
        g_debug("get %s by APP_SCENE from /etc/.kylin-osinfo", osinfo_appScene);
        return osinfo_appScene;
    }
    
    return NULL;
}

static char* insert_identifier(const char *origin_title, const char* identifier)
{
    const char* ver = strstr(origin_title, "v11");
    return ver ? g_strdup_printf("%.*s%s %s", (int)(ver - origin_title), origin_title, identifier, ver) : g_strdup(origin_title);
}

/* 返回：需调用者使用 g_free() 释放的字符串 */
static char *get_deploy_datetime(OstreeSysroot *sysroot, OstreeBootconfigParser *config){
    g_autoptr(GPtrArray) deployments = ostree_sysroot_get_deployments(sysroot);

    if (deployments->len == 0){
        return NULL;
    }
    /* 获取版本号（添加错误检查） */
    const char *version_str = ostree_bootconfig_parser_get(config, OSTREE_COMMIT_META_KEY_VERSION);
    if (!version_str) {
        g_debug("Missing version metadata in bootconfig");
        return NULL;
    }
    int deploy_version = atoi(version_str);
    g_debug("Matching deployment: total=%d, target_version=%d", deployments->len, deploy_version);
    for (guint i = 0; i < deployments->len; i++)
    {
        OstreeDeployment *deployment = deployments->pdata[i];
        if(ostree_deployment_get_index(deployment) == deployments->len - deploy_version)/* 找到了conf对应的deplotment */
        {
            GKeyFile *origin = ostree_deployment_get_origin (deployment);
            if(origin){/* 获取部署时间 */
                char *deploy_datetime_str = g_key_file_get_string (origin, "origin", "deploy_datetime", NULL);
                g_debug("deploy datetime : %s", deploy_datetime_str);
                return deploy_datetime_str;
            }
        }
    }
    return NULL;
}

//check if lenovo X1 carbon machine
static gboolean
detect_lenovo_x1_carbon(void)
{
  gboolean is_x1_carbon = FALSE;

  g_autoptr(GSubprocess) proc = NULL;
  g_autoptr(GError) error = NULL;
  g_autofree gchar *output = NULL;

  // check manufacturer
  proc = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE,
                          &error,
                          "/usr/sbin/dmidecode", "-t", "2", NULL);
  if (proc) {
    if (g_subprocess_communicate_utf8(proc, NULL, NULL, &output, NULL, &error)) {
      if (g_strstr_len(output, -1, "LXKT-FTD3-X1")) {
        is_x1_carbon = TRUE;
      }
      else {
        g_debug("/usr/sbin/dmidecode info: %s", output);
      }
    }
  }
  g_debug("X1 Carbon detected: %s", is_x1_carbon ? "true" : "false");
  return is_x1_carbon;
}

// check if it is a Great Wall D3000M machine
static gboolean
detect_greatwall_d3000m(void)
{
  gboolean is_greatwall = FALSE;
  gboolean has_d3000m = FALSE;

  g_autoptr(GSubprocess) proc = NULL;
  g_autoptr(GError) error = NULL;
  g_autofree gchar *output = NULL;

  // check manufacturer
  proc = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE,
                          &error,
                          "/usr/sbin/dmidecode", "-t", "1", NULL);
  if (proc)
  {
    if (g_subprocess_communicate_utf8(proc, NULL, NULL, &output, NULL, &error))
    {
      if (g_strstr_len(output, -1, "GreatWall") ||
          g_strstr_len(output, -1, "Great Wall"))
      {
        is_greatwall = TRUE;
      }
      else
      {
        syslog(LOG_INFO, "/usr/sbin/dmidecode info: %s", output);
      }
    }
  }

  // check CPU
  if (is_greatwall)
  {
    g_autofree gchar *cpuinfo_content = NULL;
    gsize length = 0;
    if (g_file_get_contents("/proc/cpuinfo", &cpuinfo_content, &length, &error))
    {
      g_autofree gchar *lower_content = g_ascii_strdown(cpuinfo_content, -1);
      if (g_strstr_len(lower_content, -1, "d3000m"))
      {
        has_d3000m = TRUE;
      }
    }
  }
  gboolean result = is_greatwall && has_d3000m;
  syslog(LOG_INFO, "GreatWall D3000M detection: %s; is_greatwall: %s; has_d3000m: %s",
         result ? "detected" : "not detected",
         is_greatwall ? "true" : "false",
         has_d3000m ? "true" : "false");

  return result;
}

// check whether the file is a PE32+ executable file
static gboolean
is_pe32_executable(const char *filepath)
{
  gboolean result = FALSE;
  g_autofree char *command = NULL;
  FILE *fp;
  char buffer[256];

  command = g_strdup_printf("file -b '%s'", filepath);
  fp = popen(command, "r");
  if (fp)
  {
    if (fgets(buffer, sizeof(buffer), fp))
    {
      if (strstr(buffer, "MS-DOS executable") ||
          strstr(buffer, "PE32+ executable"))
      {
        result = TRUE;
      }
    }
    pclose(fp);
  }

  return result;
}

// check if the file is a gzip-compressed file
static gboolean
is_gzip_compressed(const char *filepath)
{
  gboolean result = FALSE;
  g_autofree char *command = NULL;
  FILE *fp;
  char buffer[256];

  command = g_strdup_printf("file -b '%s'", filepath);
  fp = popen(command, "r");
  if (fp)
  {
    if (fgets(buffer, sizeof(buffer), fp))
    {
      if (strstr(buffer, "gzip compressed data"))
      {
        result = TRUE;
      }
    }
    pclose(fp);
  }

  return result;
}

// extract the kernel from the gzip file
static gboolean
extract_kernel_from_gzip(const char *src_path, const char *dst_path)
{
  int src_fd = -1, dst_fd = -1;
  FILE *src_fp = NULL;
  unsigned char buffer[4];
  off_t offset = 0;
  gboolean success = FALSE;
  ssize_t bytes_read;

  // open the source file
  src_fd = open(src_path, O_RDONLY);
  if (src_fd < 0)
  {
    syslog(LOG_ERR, "Failed to open source file %s: %s", src_path, strerror(errno));
    return FALSE;
  }

  // search for the gzip magic number (0x1f 0x8b 0x08)
  while (1)
  {
    bytes_read = read(src_fd, buffer, 3);
    if (bytes_read < 3)
    {
      break;
    }

    if (buffer[0] == 0x1f && buffer[1] == 0x8b && buffer[2] == 0x08)
    {
      lseek(src_fd, offset, SEEK_SET);
      break;
    }

    offset++;
    lseek(src_fd, offset, SEEK_SET);
  }

  if (bytes_read < 3)
  {
    syslog(LOG_ERR, "GZIP magic number not found in %s", src_path);
    close(src_fd);
    return FALSE;
  }

  // open the target file
  dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (dst_fd < 0)
  {
    syslog(LOG_ERR, "Failed to create destination file %s: %s", dst_path, strerror(errno));
    close(src_fd);
    return FALSE;
  }

  // decompress and write to the target file
  g_autofree char *command = g_strdup_printf("dd if='%s' bs=1 skip=%ld 2>/dev/null | zcat > '%s'",
                                             src_path, offset, dst_path);
  int ret = system(command);
  if (ret == 0)
  {
    success = TRUE;
    syslog(LOG_INFO, "Successfully extracted kernel from gzip to %s", dst_path);
  }
  else
  {
    syslog(LOG_ERR, "Failed to extract kernel using zcat, return code: %d", ret);
  }

  close(src_fd);
  close(dst_fd);
  return success;
}

// copy file
static gboolean
copy_file(const char *src_path, const char *dst_path)
{
  int src_fd = -1, dst_fd = -1;
  char buffer[4096];
  ssize_t bytes_read, bytes_written;

  src_fd = open(src_path, O_RDONLY);
  if (src_fd < 0)
  {
    syslog(LOG_ERR, "Failed to open source file %s: %s", src_path, strerror(errno));
    return FALSE;
  }

  dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (dst_fd < 0)
  {
    syslog(LOG_ERR, "Failed to create destination file %s: %s", dst_path, strerror(errno));
    close(src_fd);
    return FALSE;
  }

  while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0)
  {
    bytes_written = write(dst_fd, buffer, bytes_read);
    if (bytes_written != bytes_read)
    {
      syslog(LOG_ERR, "Write error to %s", dst_path);
      close(src_fd);
      close(dst_fd);
      return FALSE;
    }
  }

  close(src_fd);
  close(dst_fd);

  if (bytes_read < 0)
  {
    syslog(LOG_ERR, "Read error from %s: %s", src_path, strerror(errno));
    return FALSE;
  }

  syslog(LOG_INFO, "Successfully copied %s to %s", src_path, dst_path);
  return TRUE;
}

// create the OSLoader configuration file
static gboolean
create_osloader_config(const char *linux_cmdline)
{
  const char *osloader_cfg_path = OSLOADER_CFG_PATH;
  int fd;
  ssize_t bytes_written;

  // create directory (if it does not exist)
  g_autofree char *dir_path = g_path_get_dirname(osloader_cfg_path);
  g_mkdir_with_parents(dir_path, 0755);

  fd = open(osloader_cfg_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
  {
    syslog(LOG_ERR, "Failed to create OSLoader.cfg: %s", strerror(errno));
    return FALSE;
  }

  // write the first line
  const char *line1 = "\\EFI\\BOOT\\vmlinux\n";
  bytes_written = write(fd, line1, strlen(line1));

  // write the second line
  g_autofree char *line2 = g_strdup_printf("initrd=\\EFI\\BOOT\\initrd.img %s\n", linux_cmdline);
  bytes_written += write(fd, line2, strlen(line2));

  close(fd);

  if (bytes_written < 0)
  {
    syslog(LOG_ERR, "Failed to write OSLoader.cfg: %s", strerror(errno));
    return FALSE;
  }

  syslog(LOG_INFO, "Successfully created OSLoader.cfg");
  return TRUE;
}

// OSLoader processing function
static void
process_first_boot_entry_for_osloader(const char *kernel_path,
                                      const char *initrd_path,
                                      const char *linux_cmdline)
{
  g_autofree char *full_kernel_path = NULL;
  g_autofree char *full_initrd_path = NULL;

  full_kernel_path = g_strdup_printf("/boot%s", kernel_path);
  full_initrd_path = g_strdup_printf("/boot%s", initrd_path);

  syslog(LOG_INFO, "Processing OSLoader for GreatWall D3000M:");
  syslog(LOG_INFO, "  Kernel path: %s", full_kernel_path);
  syslog(LOG_INFO, "  Initrd path: %s", full_initrd_path);
  syslog(LOG_INFO, "  Cmdline: %s", linux_cmdline);

  if (!g_file_test(full_kernel_path, G_FILE_TEST_EXISTS) ||
      !g_file_test(full_initrd_path, G_FILE_TEST_EXISTS))
  {
    syslog(LOG_ERR, "Kernel or initrd file not found for OSLoader");
    return;
  }

  // handle kernel files
  const char *vmlinux_dst = "/boot/efi/EFI/BOOT/vmlinux";
  gboolean kernel_processed = FALSE;

  if (is_gzip_compressed(full_kernel_path))
  {
    syslog(LOG_INFO, "Extracting kernel from gzip...");
    kernel_processed = extract_kernel_from_gzip(full_kernel_path, vmlinux_dst);
  }
  else if (is_pe32_executable(full_kernel_path))
  {
    syslog(LOG_INFO, "Copying PE32+ kernel...");
    kernel_processed = copy_file(full_kernel_path, vmlinux_dst);
  }
  else
  {
    syslog(LOG_ERR, "Unknown kernel type: %s", full_kernel_path);
    return;
  }

  if (!kernel_processed)
  {
    syslog(LOG_ERR, "Failed to process kernel file");
    return;
  }

  // copy the initrd file
  const char *initrd_dst = "/boot/efi/EFI/BOOT/initrd.img";
  if (!copy_file(full_initrd_path, initrd_dst))
  {
    syslog(LOG_ERR, "Failed to copy initrd file");
    return;
  }

  // create OSLoader configuration
  create_osloader_config(linux_cmdline);
}

// check whether the file OSLoader.cfg exists and is not empty
static gboolean
osloader_cfg_exists(void)
{
  const char *osloader_cfg_path = OSLOADER_CFG_PATH;
  g_autofree char *content = NULL;
  gsize length = 0;

  if (!g_file_test(osloader_cfg_path, G_FILE_TEST_EXISTS)) {
    syslog(LOG_DEBUG, "OSLoader.cfg does not exist, will create new one");
    return FALSE;
  }

  // check if the file is empty
  if (g_file_get_contents(osloader_cfg_path, &content, &length, NULL)) {
    g_strstrip(content);
    
    if (length > 0 && strlen(content) > 0) {
      syslog(LOG_INFO, "OSLoader.cfg already exists and not empty, skipping");
      return TRUE;
    } else {
      syslog(LOG_INFO, "OSLoader.cfg exists but is empty, will recreate");
      return FALSE;
    }
  }

  return FALSE;
}

/* This implementation is quite complex; see this issue for
 * a starting point:
 * https://github.com/ostreedev/ostree/issues/717
 */
gboolean
_ostree_bootloader_grub2_generate_config (OstreeSysroot *sysroot, int bootversion, int target_fd,
                                          GCancellable *cancellable, GError **error)
{
  /* So... yeah.  Just going to hardcode these. */
  #if defined(__aarch64__) || defined(__arm__)
    static const char hardcoded_video[] = "";
  #else
    static const char hardcoded_video[] = "load_video\n"
      "set gfxpayload=keep\n";
  #endif
  static const char hardcoded_insmods[] = "insmod gzio\n";
  const char *grub2_boot_device_id = g_getenv ("GRUB2_BOOT_DEVICE_ID");
  const char *grub2_prepare_root_cache = g_getenv ("GRUB2_PREPARE_ROOT_CACHE");
  const char *grub2_cmdline_linux_default = g_getenv ("GRUB_CMDLINE_LINUX_DEFAULT");
  const char *grub2_cmdline_linux_security = g_getenv ("GRUB_CMDLINE_LINUX_SECURITY");
  const char *grub2_cmdline_linux_upgrade = g_getenv ("GRUB_CMDLINE_LINUX_UPGRADE");
  //新增获取是否禁用还原选项的变量
  const char *grub_restore_disabled = g_getenv ("GRUB_RESTORE_DISABLED");
  gboolean is_restore_disabled = (grub_restore_disabled != NULL && g_strcmp0(grub_restore_disabled,"yes")==0);
  const char *grub2_title_override = get_override_title();
  const char *grub2_cmdline_linux_lenovo_extra = LENOVO_X1_CARBON_PARAMETER;

  /* We must have been called via the wrapper script */
  g_assert (grub2_boot_device_id != NULL);
  g_assert (grub2_prepare_root_cache != NULL);

  /* Passed from the parent */
  gboolean is_efi = g_getenv ("_OSTREE_GRUB2_IS_EFI") != NULL;

  g_autoptr (GOutputStream) out_stream = g_unix_output_stream_new (target_fd, FALSE);

  g_autoptr (GPtrArray) loader_configs = NULL;
  if (!_ostree_sysroot_read_boot_loader_configs (sysroot, bootversion, &loader_configs, cancellable,
                                                 error))
    return FALSE;
  /* New addition:Determine whether it is necessary to add the "/boot" perfix before the kernel path*/
  gboolean need_boot_prefix = FALSE;

  g_autofree char *sysroot_path = g_file_get_path(ostree_sysroot_get_path(sysroot));

  if (sysroot_path)
  {
      g_autofree char *boot_path = g_build_filename(sysroot_path, "boot", NULL);
      struct stat boot_stat, root_stat;
      if(g_stat(boot_path, &boot_stat)==0 && g_stat(sysroot_path, &root_stat)==0)
      {
        if (boot_stat.st_dev == root_stat.st_dev)
        {
          need_boot_prefix = TRUE; 
        }
      }

  }
  /*Left bracket {flag bit*/
  gboolean left_brace = FALSE;
  /*Right bracket} flag bit*/
  gboolean right_brace = FALSE;
  /*multiple_vmlinuz flag bit*/
  gboolean multiple_vmlinuz_brace = FALSE;
  g_autoptr (GString) output = g_string_new ("");
  /*Restore is used to save and factory restore fields*/
  g_autoptr (GString) restore_menu = g_string_new ("");
  g_autoptr (GString) restore = g_string_new ("");
  g_autoptr (GString) system_restore = g_string_new ("");
  /*Temporarily save the content of the output*/
  g_autoptr (GString) tmp = g_string_new ("");

  /* For OSLoader creation */
  const char *first_kernel = NULL;
  const char *first_initrd = NULL;
  const char *first_options = NULL;

  for (guint i = 0; i < loader_configs->len; i++)
    {
      OstreeBootconfigParser *config = loader_configs->pdata[i];
      const char *title;
      const char *options;
      const char *kernel;
      const char *initrd;
      const char *devicetree;
      const char *linuxTotal;
      const char *initrdTotal;
      char *quoted_title = NULL;
      char *uuid = NULL;
      char *quoted_uuid = NULL;
      char *version_part = NULL;

      title = ostree_bootconfig_parser_get (config, "title");
      if (!title)
        title = "(Untitled)";
      if (grub2_title_override && *grub2_title_override != '\0'
            && should_override(title, grub2_title_override)) {
        title = grub2_title_override;
        g_debug("RC to Release override grub title %s\n", title);
      }

      g_debug("final show title : %s\n", title);
      version_part = extract_version_part(title);
      g_autofree char *app_scene = get_appScene();
      if(app_scene && strcmp(app_scene, "EDU") == 0){/* 如果是教育版，需要添加教育版标识 */
        title = g_strdup_printf("%s %s", _("openkylin v3.0 EDU"), version_part);
        g_debug("insert EDU identifier, tiltle:%s\n", title);
      }
      else{
        title = g_strdup_printf("%s %s", _("openkylin v3.0"), version_part);
      }

      /*Determine if the deployment corresponding to the current configuration is factory made*/
      if (_ostree_bootconfg_parser_is_restore_factory(sysroot, config)) 
      {
          if(is_restore_disabled)
          {
              continue;
          }
        char restore_title[100] = {0};
        sprintf(restore_title, "%s %s", _("Factory Restore - openkylin v3.0"), version_part);
        title = restore_title;
        /*Set the boot for restoring the factory part, first save the original output in the temporary variable tmp, and then set the output to empty
        After obtaining all the factory recovery guides, set Restore to output to save the factory recovery string, and restore the output to tmp content.*/
        g_string_set_size(tmp,0);
        g_string_append (tmp, output->str);
        g_string_set_size(output,0);
      }
      else if(grub2_cmdline_linux_upgrade != NULL && strcmp(grub2_cmdline_linux_upgrade, "") != 0) //表示处于更新升级
      {
        char upgrade_title[100] = {0};
        sprintf(upgrade_title, "Upgrade to %s", title);
        title = upgrade_title;
      }else if(i > 0){
          g_autofree char* deploy_datetime_str = get_deploy_datetime(sysroot, config);
          if(deploy_datetime_str){
              char historical_subitme_title[100] = {0};
              sprintf(historical_subitme_title, "%s (%s)", title, deploy_datetime_str);
              title = historical_subitme_title;
              g_debug("historical version subitme title : %s", title);
          }
      }

      kernel = ostree_bootconfig_parser_get(config, "linux");
      if (first_kernel == NULL)
        first_kernel = kernel;
      quoted_title = g_shell_quote (title);
      uuid = g_strdup_printf ("ostree-%u-%s", (guint)i, grub2_boot_device_id);
      quoted_uuid = g_shell_quote (uuid);
      g_string_append_printf (output, "menuentry %s --class gnu-linux --class gnu --class os --unrestricted %s {\n", quoted_title, quoted_uuid);

      /* Hardcoded sections */
      g_string_append (output, hardcoded_video);
      g_string_append (output, hardcoded_insmods);
      g_string_append (output, grub2_prepare_root_cache);
      g_string_append_c (output, '\n');

      if (!kernel)
      {
        g_free(uuid);
        g_free(quoted_title);
        g_free(quoted_uuid);
        return glnx_throw(error, "No \"linux\" key in bootloader config");
      }
      g_string_append (output, "linux");
      if (is_efi)
        g_string_append (output, GRUB2_EFI_SUFFIX);
      else
        g_string_append (output, GRUB2_SUFFIX);
      g_string_append_c (output, ' ');
      if (need_boot_prefix) {
        g_string_append (output, "/boot");
      }
      g_string_append (output, kernel);

      options = ostree_bootconfig_parser_get (config, "options");
      if (options)
        {
          if (first_options == NULL)
            first_options = options;
          g_string_append_c (output, ' ');
          g_string_append (output, options);
          
          g_string_append_c (output, ' ');
          g_string_append (output, grub2_cmdline_linux_default);

          g_string_append_c (output, ' ');
          g_string_append (output, grub2_cmdline_linux_security);

          if (detect_lenovo_x1_carbon()) {
            g_string_append_c (output, ' ');
            g_string_append (output, grub2_cmdline_linux_lenovo_extra);
          }
        }
      if (_ostree_bootconfg_parser_is_restore_factory(sysroot, config))
      {
        g_string_append (output, " factory-restore");
      }else{
        g_string_append_c (output, ' ');
        g_string_append (output, grub2_cmdline_linux_upgrade);
      }
      g_string_append_c (output, '\n');

      initrd = ostree_bootconfig_parser_get (config, "initrd");
      if (initrd)
        {
          if (first_initrd == NULL)
            first_initrd = initrd;
          g_string_append (output, "initrd");
          if (is_efi)
            g_string_append (output, GRUB2_EFI_SUFFIX);
          else
            g_string_append (output, GRUB2_SUFFIX);
          g_string_append_c (output, ' ');
          if (need_boot_prefix) {
            g_string_append (output, "/boot");
          }
          g_string_append (output, initrd);
          g_string_append_c (output, '\n');
        }

      devicetree = ostree_bootconfig_parser_get (config, "devicetree");
      if (devicetree)
        {
          g_string_append (output, "devicetree");
          g_string_append_c (output, ' ');
          g_string_append (output, devicetree);
          g_string_append_c (output, '\n'); 
        }

      g_string_append (output, "}\n");

      if(i == 0&&!is_restore_disabled){/* 处理系统还原grub选项内容 */
        g_autofree gchar *system_restore_opt = handle_system_restore_opt(output->str);
        if (system_restore_opt) {
          g_string_append(system_restore, system_restore_opt);
        }
        g_autofree gchar *restore_menu_opt = handle_restore_menu_opt(output->str);
        if (restore_menu_opt) {
          g_string_append(restore_menu, restore_menu_opt);
        }
      }

      linuxTotal = ostree_bootconfig_parser_get (config, "linuxTotal");
      initrdTotal = ostree_bootconfig_parser_get (config, "initrdTotal");

      if (!multiple_vmlinuz_brace && linuxTotal && initrdTotal && output->str != NULL)
      {
          multiple_vmlinuz_brace = TRUE;
          g_autofree char *vmlinuz_sub = _ostree_update_vmlinuz_sub(output->str, title , quoted_uuid, linuxTotal, initrdTotal);
          if (vmlinuz_sub)
            g_string_append(output, vmlinuz_sub);
      }

      /*Determine whether to increase{*/
      if (!left_brace && loader_configs->len > 2 && !_ostree_bootconfg_parser_is_restore_factory(sysroot,config))
      {
        char historty_title[1024] = {0};
        sprintf(historty_title, "%s %s", _("Historical Version -"), title);
        char *quoted_my_sub = NULL;
        quoted_my_sub = g_shell_quote(historty_title);
        g_string_append_printf(output, "submenu %s  --unrestricted %s {\n", quoted_my_sub, quoted_uuid);
        g_free(quoted_my_sub);
        left_brace = TRUE;
      }
      g_free(uuid);
      g_free(quoted_title);
      g_free(quoted_uuid);

      /*The factory version writing is complete. The output stores the factory version,\
       while the tmp stores the previously traversed version. Reset back*/
      if (_ostree_bootconfg_parser_is_restore_factory(sysroot, config)) 
      {
        g_string_append(restore,output->str);
        g_string_set_size(output,0);
        g_string_append(output,tmp->str);
      }
    }

    /*Determine whether to increase}*/
    if (left_brace && !right_brace && loader_configs->len > 2)
    {
      g_string_append(output, " }\n");
      right_brace = TRUE;
    }

    g_string_append_c (output, '\n');
    if (restore_menu->len&&!is_restore_disabled)
    {
      g_string_append(output, restore_menu->str);
    }

    /* 系统还原 */
    if(system_restore->len&&!is_restore_disabled){
      g_string_append(output, system_restore->str);
    }
    /* 出厂还原 */
    if (restore->len&&!is_restore_disabled)
    {
      g_string_append(output, restore->str);
    }
    if(!is_restore_disabled)
    {
        g_string_append(output, "}\n");
    }

  gsize bytes_written;
  if (!g_output_stream_write_all (out_stream, output->str, output->len, &bytes_written, cancellable,
                                  error))
    return FALSE;

  // /* Create OSLoader for GreatWall D3000M */
  // if (detect_greatwall_d3000m() &&
  //     first_kernel && first_initrd && first_options &&
  //     !osloader_cfg_exists())
  // {

  //   syslog(LOG_INFO, "GreatWall D3000M detected, creating OSLoader files");

  //   /* Build complete command line */
  //   g_autoptr(GString) cmdline = g_string_new(first_options);

  //   if (grub2_cmdline_linux_default)
  //   {
  //     g_string_append_printf(cmdline, " %s", grub2_cmdline_linux_default);
  //   }

  //   if (grub2_cmdline_linux_security)
  //   {
  //     g_string_append_printf(cmdline, " %s", grub2_cmdline_linux_security);
  //   }

  //   if (grub2_cmdline_linux_upgrade)
  //   {
  //     g_string_append_printf(cmdline, " %s", grub2_cmdline_linux_upgrade);
  //   }

  //   /* Process OSLoader files */
  //   process_first_boot_entry_for_osloader(first_kernel, first_initrd, cmdline->str);
  // }

  return TRUE;
}

typedef struct
{
  const char *root;
  const char *bootversion_str;
  gboolean is_efi;
} Grub2ChildSetupData;

/* Post-fork, pre-exec child setup for grub2-mkconfig */
static void
grub2_child_setup (gpointer user_data)
{
  Grub2ChildSetupData *cdata = user_data;

  setenv ("_OSTREE_GRUB2_BOOTVERSION", cdata->bootversion_str, TRUE);
  /* We have to pass our state (whether or not we're using EFI) to the child */
  if (cdata->is_efi)
    setenv ("_OSTREE_GRUB2_IS_EFI", "1", TRUE);

  /* Everything below this is dealing with the chroot case; if
   * we're not doing that, return early.
   */
  if (!cdata->root)
    return;

  /* TODO: investigate replacing this with bwrap */
  if (chdir (cdata->root) != 0)
    {
      perror ("chdir");
      _exit (1);
    }

  if (unshare (CLONE_NEWNS) != 0)
    {
      perror ("CLONE_NEWNS");
      _exit (1);
    }

  if (mount (NULL, "/", "none", MS_REC | MS_PRIVATE, NULL) < 0)
    {
      perror ("Failed to make / a private mount");
      _exit (1);
    }

  if (mount (".", ".", NULL, MS_BIND | MS_PRIVATE, NULL) < 0)
    {
      perror ("mount (MS_BIND)");
      _exit (1);
    }

  if (mount (cdata->root, "/", NULL, MS_MOVE, NULL) < 0)
    {
      perror ("failed to MS_MOVE to /");
      _exit (1);
    }

  if (chroot (".") != 0)
    {
      perror ("chroot");
      _exit (1);
    }
}

/* Main entrypoint for writing GRUB configuration. */
static gboolean
_ostree_bootloader_grub2_write_config (OstreeBootloader *bootloader, int bootversion,
                                       GPtrArray *new_deployments, GCancellable *cancellable,
                                       GError **error)
{
  OstreeBootloaderGrub2 *self = OSTREE_BOOTLOADER_GRUB2 (bootloader);

  /* Autotests can set this envvar to select which code path to test, useful for OS installers as
   * well */
  gboolean use_system_grub2_mkconfig = TRUE;
#ifdef USE_BUILTIN_GRUB2_MKCONFIG
  use_system_grub2_mkconfig = FALSE;
#endif
  const gchar *grub_exec = g_getenv ("OSTREE_GRUB2_EXEC");
  if (grub_exec)
    {
      if (g_str_has_suffix (grub_exec, GRUB2_MKCONFIG_PATH))
        use_system_grub2_mkconfig = TRUE;
      else
        use_system_grub2_mkconfig = FALSE;
    }
  else
    grub_exec = use_system_grub2_mkconfig ? GRUB2_MKCONFIG_PATH
                                          : TARGET_PREFIX "/lib/ostree/ostree-grub-generator";

  g_autofree char *grub2_mkconfig_chroot = NULL;
  if (use_system_grub2_mkconfig && ostree_sysroot_get_booted_deployment (self->sysroot) == NULL
      && g_file_has_parent (self->sysroot->path, NULL))
    {
      OstreeDeployment *tool_deployment;
      g_autoptr (GFile) tool_deployment_root = NULL;

      g_assert_cmpint (new_deployments->len, >, 0);
      tool_deployment = new_deployments->pdata[0];

      /* Sadly we have to execute code to generate the bootloader configuration.
       * If we're in a booted deployment, we just don't chroot.
       *
       * In the case of an installer, use the first deployment root (which
       * will most likely be the only one.
       *
       * This all only applies if we're not using the builtin
       * generator, which handles being run outside of the root.
       */
      tool_deployment_root
          = ostree_sysroot_get_deployment_directory (self->sysroot, tool_deployment);
      grub2_mkconfig_chroot = g_file_get_path (tool_deployment_root);
    }

  g_debug ("Using grub2-mkconfig chroot: %s\n", grub2_mkconfig_chroot);

  g_autoptr (GFile) new_config_path = NULL;
  g_autoptr (GFile) config_path_efi_dir = NULL;
  if (self->is_efi)
    {
      config_path_efi_dir = g_file_get_parent (self->config_path_efi);
      new_config_path = g_file_get_child (config_path_efi_dir, "grub.cfg.new");
      /* We use grub2-mkconfig to write to a temporary file first */
      if (!ot_gfile_ensure_unlinked (new_config_path, cancellable, error))
        return FALSE;
    }
  else
    {
      new_config_path = ot_gfile_resolve_path_printf (self->sysroot->path,
                                                      "boot/loader.%d/grub.cfg", bootversion);
    }

  const char *grub_argv[4] = { NULL, "-o", NULL, NULL };
  Grub2ChildSetupData cdata = {
    NULL,
  };
  grub_argv[0] = grub_exec;
  grub_argv[2] = gs_file_get_path_cached (new_config_path);

  GSpawnFlags grub_spawnflags = G_SPAWN_SEARCH_PATH;
  if (!g_getenv ("OSTREE_DEBUG_GRUB2"))
    grub_spawnflags |= G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL;
  cdata.root = grub2_mkconfig_chroot;
  g_autofree char *bootversion_str = g_strdup_printf ("%u", (guint)bootversion);
  cdata.bootversion_str = bootversion_str;
  cdata.is_efi = self->is_efi;
  /* Note in older versions of the grub2 package, this script doesn't even try
     to be atomic; it just does:

     cat ${grub_cfg}.new > ${grub_cfg}
     rm -f ${grub_cfg}.new

     Upstream is fixed though.
  */
  int grub2_estatus;
  if (!g_spawn_sync (NULL, (char **)grub_argv, NULL, grub_spawnflags, grub2_child_setup, &cdata,
                     NULL, NULL, &grub2_estatus, error))
    return FALSE;
  if (!g_spawn_check_exit_status (grub2_estatus, error))
    {
      g_prefix_error (error, "%s: ", grub_argv[0]);
      return FALSE;
    }

  /* Now let's fdatasync() for the new file */
  {
    glnx_autofd int new_config_fd = -1;
    if (!glnx_openat_rdonly (AT_FDCWD, gs_file_get_path_cached (new_config_path), TRUE,
                             &new_config_fd, error))
      return FALSE;

    if (fdatasync (new_config_fd) < 0)
      return glnx_throw_errno_prefix (error, "fdatasync");
  }

  if (self->is_efi)
    {
      g_autoptr (GFile) config_path_efi_old
          = g_file_get_child (config_path_efi_dir, "grub.cfg.old");

      /* copy current to old */
      if (!ot_gfile_ensure_unlinked (config_path_efi_old, cancellable, error))
        return FALSE;
      if (!g_file_copy (self->config_path_efi, config_path_efi_old, G_FILE_COPY_OVERWRITE,
                        cancellable, NULL, NULL, error))
        return FALSE;

      /* NOTE: NON-ATOMIC REPLACEMENT; WE can't do anything else on FAT;
       * see https://bugzilla.gnome.org/show_bug.cgi?id=724246
       */
      if (!ot_gfile_ensure_unlinked (self->config_path_efi, cancellable, error))
        return FALSE;
      if (rename (gs_file_get_path_cached (new_config_path),
                  gs_file_get_path_cached (self->config_path_efi))
          < 0)
        return glnx_throw_errno_prefix (error, "rename");
    }

  return TRUE;
}

static gboolean
_ostree_bootloader_grub2_is_atomic (OstreeBootloader *bootloader)
{
  OstreeBootloaderGrub2 *self = OSTREE_BOOTLOADER_GRUB2 (bootloader);
  return !self->is_efi;
}

static void
_ostree_bootloader_grub2_finalize (GObject *object)
{
  OstreeBootloaderGrub2 *self = OSTREE_BOOTLOADER_GRUB2 (object);

  g_clear_object (&self->sysroot);
  g_clear_object (&self->config_path_bios_1);
  g_clear_object (&self->config_path_bios_2);
  g_clear_object (&self->config_path_efi);

  G_OBJECT_CLASS (_ostree_bootloader_grub2_parent_class)->finalize (object);
}

void
_ostree_bootloader_grub2_init (OstreeBootloaderGrub2 *self)
{
}

static void
_ostree_bootloader_grub2_bootloader_iface_init (OstreeBootloaderInterface *iface)
{
  iface->query = _ostree_bootloader_grub2_query;
  iface->get_name = _ostree_bootloader_grub2_get_name;
  iface->write_config = _ostree_bootloader_grub2_write_config;
  iface->is_atomic = _ostree_bootloader_grub2_is_atomic;
}

void
_ostree_bootloader_grub2_class_init (OstreeBootloaderGrub2Class *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = _ostree_bootloader_grub2_finalize;
}

OstreeBootloaderGrub2 *
_ostree_bootloader_grub2_new (OstreeSysroot *sysroot)
{
  OstreeBootloaderGrub2 *self = g_object_new (OSTREE_TYPE_BOOTLOADER_GRUB2, NULL);
  self->sysroot = g_object_ref (sysroot);
  /* Used by (at least) Debian */
  self->config_path_bios_1
      = g_file_resolve_relative_path (self->sysroot->path, "boot/grub/grub.cfg");
  /* Used by (at least) Fedora */
  self->config_path_bios_2
      = g_file_resolve_relative_path (self->sysroot->path, "boot/grub2/grub.cfg");
  return self;
}
