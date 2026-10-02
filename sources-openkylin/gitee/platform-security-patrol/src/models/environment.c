#include "models/environment.h"
#include "utils/string_utils.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

#define MODULE "environment"

/* CPU 信息辅助函数 */
static void cpu_info_init(cpu_info_t *cpu)
{
    if (!cpu) return;
    
    cpu->vendor = NULL;
    cpu->model = NULL;
    // cpu->flags = NULL;
    // cpu->flags_count = 0;
}

static void cpu_info_free(cpu_info_t *cpu)
{
    if (!cpu) return;
    
    free(cpu->vendor);
    free(cpu->model);
    
    // if (cpu->flags) {
    //     for (size_t i = 0; i < cpu->flags_count; i++) {
    //         free(cpu->flags[i]);
    //     }
    //     free(cpu->flags);
    // }
    
    cpu->vendor = NULL;
    cpu->model = NULL;
    // cpu->flags = NULL;
    // cpu->flags_count = 0;
}

/* 固件信息辅助函数 */
static void firmware_info_init(firmware_info_t *firmware)
{
    if (!firmware) return;
    
    firmware->type = NULL;
    firmware->vendor = NULL;
    firmware->version = NULL;
    // firmware->secure_firmware_version = NULL;
    // firmware->tpm_firmware_version = NULL;
}

static void firmware_info_free(firmware_info_t *firmware)
{
    if (!firmware) return;
    
    free(firmware->type);
    free(firmware->vendor);
    free(firmware->version);
    // free(firmware->secure_firmware_version);
    // free(firmware->tpm_firmware_version);
    
    firmware->type = NULL;
    firmware->vendor = NULL;
    firmware->version = NULL;
    // firmware->secure_firmware_version = NULL;
    // firmware->tpm_firmware_version = NULL;
}

/* OS 信息辅助函数 */
static void os_info_init(os_info_t *os)
{
    if (!os) return;
    
    os->distribution = NULL;
    os->version = NULL;
    os->kernel = NULL;
    // os->kernel_config = NULL;
    // os->config_count = 0;
    // os->modules = NULL;
    // os->modules_count = 0;
}

static void os_info_free(os_info_t *os)
{
    if (!os) return;
    
    free(os->distribution);
    free(os->version);
    free(os->kernel);
    
    // if (os->kernel_config) {
    //     for (size_t i = 0; i < os->config_count; i++) {
    //         free(os->kernel_config[i]);
    //     }
    //     free(os->kernel_config);
    // }
    
    // if (os->modules) {
    //     for (size_t i = 0; i < os->modules_count; i++) {
    //         free(os->modules[i]);
    //     }
    //     free(os->modules);
    // }
    
    os->distribution = NULL;
    os->version = NULL;
    os->kernel = NULL;
    // os->kernel_config = NULL;
    // os->config_count = 0;
    // os->modules = NULL;
    // os->modules_count = 0;
}

environment_t* environment_create(void)
{
    environment_t *env = (environment_t*)calloc(1, sizeof(environment_t));
    if (!env) {
        logger_error(MODULE, "Failed to allocate memory for environment");
        return NULL;
    }
    
    cpu_info_init(&env->cpu);
    firmware_info_init(&env->firmware);
    os_info_init(&env->os);
    env->raw = NULL;
    
    return env;
}

void environment_free(environment_t *env)
{
    if (!env) return;
    
    cpu_info_free(&env->cpu);
    firmware_info_free(&env->firmware);
    os_info_free(&env->os);
    
    if (env->raw) {
        free(env->raw);
    }
    
    free(env);
    logger_debug(MODULE, "Environment object freed");
}

void environment_set_cpu_vendor(environment_t *env, const char *vendor)
{
    if (!env || !vendor) return;
    
    free(env->cpu.vendor);
    env->cpu.vendor = string_utils_strdup(vendor);
    
    if (env->cpu.vendor) {
        logger_debug(MODULE, "Set CPU vendor: %s", vendor);
    }
}

void environment_set_cpu_model(environment_t *env, const char *model)
{
    if (!env || !model) return;
    
    free(env->cpu.model);
    env->cpu.model = string_utils_strdup(model);
    
    if (env->cpu.model) {
        logger_debug(MODULE, "Set CPU model: %s", model);
    }
}

void environment_set_firmware_type(environment_t *env, const char *type)
{
    if (!env || !type) return;
    
    free(env->firmware.type);
    env->firmware.type = string_utils_strdup(type);
    
    if (env->firmware.type) {
        logger_debug(MODULE, "Set firmware type: %s", type);
    }
}

void environment_set_firmware_vendor(environment_t *env, const char *vendor)
{
    if (!env || !vendor) return;
    
    free(env->firmware.vendor);
    env->firmware.vendor = string_utils_strdup(vendor);
    
    if (env->firmware.vendor) {
        logger_debug(MODULE, "Set firmware vendor: %s", vendor);
    }
}

void environment_set_firmware_version(environment_t *env, const char *version)
{
    if (!env || !version) return;
    
    free(env->firmware.version);
    env->firmware.version = string_utils_strdup(version);
    
    if (env->firmware.version) {
        logger_debug(MODULE, "Set firmware version: %s", version);
    }
}

// void environment_set_secure_firmware_version(environment_t *env, const char *version)
// {
//     if (!env || !version) return;
    
//     free(env->firmware.secure_firmware_version);
//     env->firmware.secure_firmware_version = string_utils_strdup(version);
    
//     if (env->firmware.secure_firmware_version) {
//         logger_debug(MODULE, "Set secure firmware version: %s", version);
//     }
// }

// void environment_set_tpm_firmware_version(environment_t *env, const char *version)
// {
//     if (!env || !version) return;
    
//     free(env->firmware.tpm_firmware_version);
//     env->firmware.tpm_firmware_version = string_utils_strdup(version);
    
//     if (env->firmware.tpm_firmware_version) {
//         logger_debug(MODULE, "Set TPM firmware version: %s", version);
//     }
// }

void environment_set_os_distribution(environment_t *env, const char *distro)
{
    if (!env || !distro) return;
    
    free(env->os.distribution);
    env->os.distribution = string_utils_strdup(distro);
    
    if (env->os.distribution) {
        logger_debug(MODULE, "Set OS distribution: %s", distro);
    }
}

void environment_set_os_version(environment_t *env, const char *version)
{
    if (!env || !version) return;
    
    free(env->os.version);
    env->os.version = string_utils_strdup(version);
    
    if (env->os.version) {
        logger_debug(MODULE, "Set OS version: %s", version);
    }
}

void environment_set_os_kernel(environment_t *env, const char *kernel)
{
    if (!env || !kernel) return;
    
    free(env->os.kernel);
    env->os.kernel = string_utils_strdup(kernel);
    
    if (env->os.kernel) {
        logger_debug(MODULE, "Set OS kernel: %s", kernel);
    }
}

