#!/bin/bash

#-------------------------- 全局配置 --------------------------#
set -o errexit    # 遇到错误立即退出
set -o nounset    # 遇到未定义变量报错
set -o pipefail   # 管道命令错误传递

#-------------------------- 常量定义 --------------------------#
readonly LOG_FILE="/var/log/audio_config.log"
readonly ALSA_MIXER_PATH="/usr/share/pulseaudio/alsa-mixer"
readonly PULSE_ALSA_PATHS_DIR="${ALSA_MIXER_PATH}/paths"
readonly PULSE_ALSA_PROFILE_SETS_DIR="${ALSA_MIXER_PATH}/profile-sets"
# 系统路径只读时，备用软链目录（与系统下同名文件对应）
readonly PULSE_RESOURCE_DIR="/opt/system/resource/pulseaudio"
readonly PULSE_PATHS_DIR="${PULSE_RESOURCE_DIR}/paths"
readonly PULSE_PROFILE_SETS_DIR="${PULSE_RESOURCE_DIR}/profile-sets"

#--------------------- Pulseaudio 路径定义 --------------------#
# 以下仅为 paths 目录内配置文件名；先尝试系统路径，失败再写入备用目录
ANALOG_INPUT_MIC_CONF="analog-input-mic.conf"
ANALOG_INPUT_LINEIN_CONF="analog-input-linein.conf"
ANALOG_INPUT_REAR_MIC_CONF="analog-input-rear-mic.conf"
ANALOG_INPUT_FRONT_MIC_CONF="analog-input-front-mic.conf"
ANALOG_INPUT_INTERNAL_MIC_CONF="analog-input-internal-mic.conf"

MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-mic-1.conf"
MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-mic-2.conf"
MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-mic-3.conf"
LINEIN_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-linein-1.conf"
LINEIN_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-linein-2.conf"
LINEIN_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-linein-3.conf"
REAR_MIC_BOOST_0="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-rear-mic-0.conf"
REAR_MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-rear-mic-1.conf"
REAR_MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-rear-mic-2.conf"
REAR_MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-rear-mic-3.conf"
FRONT_MIC_BOOST_0="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-front-mic-0.conf"
FRONT_MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-front-mic-1.conf"
FRONT_MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-front-mic-2.conf"
FRONT_MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-front-mic-3.conf"
INTERNAL_MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-internal-mic-1.conf"
INTERNAL_MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-internal-mic-2.conf"
INTERNAL_MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/kylin-paths/analog-input-internal-mic-3.conf"

#-------------------------- 日志函数 --------------------------#
log() {
    local level=$1
    local message=$2
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[${timestamp}] [${level}] ${message}" | tee -a "${LOG_FILE}"
}

#-------------------------- 链接函数 --------------------------#
# 先尝试 PULSE_ALSA_PATHS_DIR，失败再使用 PULSE_PATHS_DIR（备用）
_ukmedia_do_symlink() {
    local target_file=$1
    local link_name=$2
    if [[ -e "$link_name" ]] && ! rm -f "$link_name"; then
        return 1
    fi
    ln -sf "$target_file" "$link_name"
}

ukmedia_symlink_paths() {
    local target_file=$1
    local link_basename=$2
    local primary="${PULSE_ALSA_PATHS_DIR}/${link_basename}"
    local fallback="${PULSE_PATHS_DIR}/${link_basename}"

    if [[ ! -e "$target_file" ]]; then
        log "ERROR" "目标文件不存在: $target_file"
        return 0
    fi

    if _ukmedia_do_symlink "$target_file" "$primary"; then
        log "INFO" "创建链接(系统路径): $primary -> $target_file"
        return 0
    fi

    log "WARN" "系统路径不可写或创建失败，改用备用: $fallback"
    mkdir -p "${PULSE_PATHS_DIR}" || {
        log "ERROR" "无法创建备用目录: ${PULSE_PATHS_DIR}"
        return 0
    }
    if _ukmedia_do_symlink "$target_file" "$fallback"; then
        log "INFO" "创建链接(备用路径): $fallback -> $target_file"
    else
        log "ERROR" "备用路径下链接仍失败: $fallback"
    fi
    return 0
}

# 先尝试 PULSE_ALSA_PROFILE_SETS_DIR，失败再使用 PULSE_PROFILE_SETS_DIR（备用）
ukmedia_symlink_profile_sets() {
    local target_file=$1
    local link_basename=$2
    local primary="${PULSE_ALSA_PROFILE_SETS_DIR}/${link_basename}"
    local fallback="${PULSE_PROFILE_SETS_DIR}/${link_basename}"

    if [[ ! -e "$target_file" ]]; then
        log "ERROR" "目标文件不存在: $target_file"
        return 0
    fi

    if _ukmedia_do_symlink "$target_file" "$primary"; then
        log "INFO" "创建链接(系统 profile-sets): $primary -> $target_file"
        return 0
    fi

    log "WARN" "系统 profile-sets 不可写或创建失败，改用备用: $fallback"
    mkdir -p "${PULSE_PROFILE_SETS_DIR}" || {
        log "ERROR" "无法创建备用目录: ${PULSE_PROFILE_SETS_DIR}"
        return 0
    }
    if _ukmedia_do_symlink "$target_file" "$fallback"; then
        log "INFO" "创建链接(备用 profile-sets): $fallback -> $target_file"
    else
        log "ERROR" "备用 profile-sets 下链接仍失败: $fallback"
    fi
    return 0
}

#-------------------------- 硬件信息获取 --------------------------#
get_hw_info() {
    # 缓存硬件信息避免重复查询
    local -g _vendor_id
    local -g _subsystem_id
    local -g _system_product_name
    local -g _baseboard_product_name

    if [[ -z "${_vendor_id:-}" ]]; then
        _vendor_id=$(grep 'Vendor Id' /proc/asound/card*/codec#0 2>/dev/null || true)
    fi

    if [[ -z "${_subsystem_id:-}" ]]; then
        _subsystem_id=$(grep 'Subsystem Id' /proc/asound/card*/codec#0 2>/dev/null || true)
    fi

    if [[ -z "${_system_product_name:-}" ]]; then
        _system_product_name=$(dmidecode -s system-product-name 2>/dev/null || true)
    fi

    if [[ -z "${_baseboard_product_name:-}" ]]; then
        _baseboard_product_name=$(dmidecode -s baseboard-product-name 2>/dev/null || true)
    fi
}

handle_realtek_alc222() {
    log "INFO" "检测到Realtek ALC222声卡"

    ukmedia_symlink_paths "$FRONT_MIC_BOOST_0" "$ANALOG_INPUT_FRONT_MIC_CONF"
    ukmedia_symlink_paths "$REAR_MIC_BOOST_0" "$ANALOG_INPUT_REAR_MIC_CONF"
}


handle_realtek_alc897() {
    log "INFO" "检测到Realtek ALC897声卡"

    if [[ "${_baseboard_product_name}" == "FD2000ZX200MB1" ]] ||
       [[ "${_baseboard_product_name}" == "CE520F" ]]; then
        log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用配置: Front Mic Boost = 1, Rear Mic Boot = 1"
        ukmedia_symlink_paths "$FRONT_MIC_BOOST_0" "$ANALOG_INPUT_FRONT_MIC_CONF"
        ukmedia_symlink_paths "$REAR_MIC_BOOST_0" "$ANALOG_INPUT_REAR_MIC_CONF"

    else
        log "INFO" "未识别到基板型号 ${_baseboard_product_name}，使用通用配置: Front Mic Boost = 1, Rear Mic Boot = 1"
        ukmedia_symlink_paths "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
        ukmedia_symlink_paths "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"
    fi
}

handle_realtek_alc269() {
    log "INFO" "检测到Realtek ALC269声卡"

    if [[ "${_baseboard_product_name}" == "CE720F" ]] ||
       [[ "${_baseboard_product_name}" == "B52-ZK1" ]] ||
       [[ "${_baseboard_product_name}" == "MAXHUB-NZ61-6780A" ]]; then
        log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用配置: Internal Mic Boost = 1"
        ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_1" "$ANALOG_INPUT_INTERNAL_MIC_CONF"

    elif [[ "${_baseboard_product_name}" == "BM6J80" ]]; then
	log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用配置: Internal Mic Boost = 2"
        ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"

    else
        log "INFO" "未识别到基板型号 ${_baseboard_product_name}，使用通用配置: Front Mic Boost = 1, Rear Mic Boost = 1"
	ukmedia_symlink_paths "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
	ukmedia_symlink_paths "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"
    fi
}

handle_realtek_alc623() {
    log "INFO" "检测到Realtek ALC623声卡"
    log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用通用配置"
    ukmedia_symlink_paths "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
    ukmedia_symlink_paths "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"
}

handle_realtek_alc662() {
    log "INFO" "检测到Realtek ALC662声卡"

    if [[ "${_baseboard_product_name}" == "LM-LS3A4000-7A1000-1w-V01-pc_A2005" ]]; then
        log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用配置: Front Mic Boost = 0, Rear Mic Boot = 2"
        ukmedia_symlink_paths "$REAR_MIC_BOOST_2" "$ANALOG_INPUT_REAR_MIC_CONF"

    else
        log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用通用配置"
	ukmedia_symlink_paths "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
	ukmedia_symlink_paths "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"
    fi
}

handle_conexant_cx20632() {
    log "INFO" "检测到Conexant CX20632声卡"

    if [[ "${_baseboard_product_name}" == "EAECIS-FT_D2000_X100-1W-V0.1-NF14" ]]; then
        log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用配置: Internal Mic Boost = merge, Mic Boot = merge"
        ukmedia_symlink_paths "$MIC_BOOST_3" "$ANALOG_INPUT_MIC_CONF"
        ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_3" "$ANALOG_INPUT_INTERNAL_MIC_CONF"

    else
        log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用通用配置: Internal Mic Boost = 2, Mic Boot = merge"
        ukmedia_symlink_paths "$MIC_BOOST_3" "$ANALOG_INPUT_MIC_CONF"
        ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
    fi
}

handle_conexant_cx11880() {
    log "INFO" "检测到Conexant CX11880声卡"
    log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用通用配置"
    ukmedia_symlink_paths "$MIC_BOOST_3" "$ANALOG_INPUT_MIC_CONF"
    ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
}

handle_conexant_sn6140() {
    log "INFO" "检测到Conexant SN6140声卡"
    log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用通用配置"
    ukmedia_symlink_paths "$MIC_BOOST_2" "$ANALOG_INPUT_MIC_CONF"
    ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
}

handle_conexant_sn6180() {
    log "INFO" "检测到Conexant SN6180声卡"
    log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用通用配置"
    ukmedia_symlink_paths "$MIC_BOOST_2" "$ANALOG_INPUT_MIC_CONF"
    ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
}

handle_realtek_alc3328() {
    log "INFO" "检测到Realtek ALC3328声卡"

    if [[ "${_baseboard_product_name}" == "FD2000ZX200MB1" ]]; then
        log "INFO" "识别到基板型号 ${_baseboard_product_name}，使用配置: Internal Mic Boost = 1, Mic Boot = 1"
        ukmedia_symlink_paths "$MIC_BOOST_1" "$ANALOG_INPUT_MIC_CONF"
        ukmedia_symlink_paths "$INTERNAL_MIC_BOOST_1" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
    fi
}

#-------------------------- 特殊硬件处理 --------------------------#
handle_special_hardware() {

    # 华为特殊处理
    if [[ -f "/usr/share/pulseaudio/alsa-mixer/profile-sets/pulse-huawei.conf" ]]; then
        [[ -f "/etc/pulse/daemon-i2s-huawei.conf" ]] && \
            mv -f "/etc/pulse/daemon-i2s-huawei.conf" "/etc/pulse/daemon.conf"
    fi

    if [[ $(dmidecode | grep -c "UNIS D3830 G3") -ne 0 ]]; then
       log "INFO" "UNIS D3830 G3 udev 自定义配置文件"
       ukmedia_symlink_profile_sets \
           "/usr/share/pulseaudio/alsa-mixer/profile-sets/UNIS-D3830-G3-audio.conf" \
           "alc4080-mh243a-usb-audio.conf"
    fi

    if [[ `dmidecode |grep -c "706-LS3A5000-4-V1.0-B40L-41A1"` != "0" ]]; then
        log "INFO" "特殊硬件处理 AutoMute处理"
        ukmedia_symlink_paths "/usr/share/pulseaudio/alsa-mixer/paths/analog-output-headphones-enabled-AutoMute.conf" \
                        "analog-output-headphones.conf"
    fi
}

#-------------------------- 主逻辑 --------------------------#
main() {
    log "INFO" "====== 开始音频配置 ======"

    # 获取硬件信息
    get_hw_info
    log "DEBUG" "硬件信息:
    Vendor ID: ${_vendor_id}
    Subsystem ID: ${_subsystem_id}
    System Product: ${_system_product_name}
    Baseboard Product: ${_baseboard_product_name}"

    # 声卡类型判断
    case "${_vendor_id}" in
	*"0x10ec0222"*) handle_realtek_alc222 ;;
	*"0x10ec0257"*) handle_realtek_alc3328 ;;
        *"0x10ec0269"*) handle_realtek_alc269 ;;
        *"0x10ec0623"*) handle_realtek_alc623 ;;
        *"0x10ec0662"*) handle_realtek_alc662 ;;
        *"0x10ec0897"*) handle_realtek_alc897 ;;
        *"0x14f15098"*) handle_conexant_cx20632 ;;
	*"0x14f11f86"*) handle_conexant_cx11880 ;;
	*"0x14f120d1"*) handle_conexant_sn6180 ;;
	*"0x14f11f87"*) handle_conexant_sn6140 ;;
        *)
            log "INFO" "暂未适配的声卡型号: ${_vendor_id}"
            ;;
    esac

    # 特殊硬件处理
    handle_special_hardware

    log "INFO" "====== 音频配置完成 ======"
    return 0
}

#-------------------------- 执行入口 --------------------------#
main "$@"
