use std::path::Path;

const SYS_PCI_DEVICES: &str = "/sys/bus/pci/devices";

// kyml 设备白名单，内容取自 /usr/share/kyml/kyml_whitelists.txt（内容固定），
// 直接内置以避免依赖 kyml 包安装。
const KYML_WHITELIST: &str = "\
# 格式: vendor_id|device_id|device_type(0:CPU, 1:GPU, 2:NPU)|supportShareKytensor|supportStandardGGUF|supportedModels|llamacpp_plugin_name|kyml_plugin_name|vendor|model
0x10de|0x2231|1|1|1|1,1|cuda|nvml|NVIDIA|RTX A5000
0x10de|0x2685|1|1|1|1,1|cuda|nvml|NVIDIA|GeForce RTX 4090 D
0x10de|0x2684|1|1|1|1,1|cuda|nvml|NVIDIA|GeForce RTX 4090
0x10de|0x2702|1|1|1|1,1|cuda|nvml|NVIDIA|GeForce RTX 4080
0x10de|0x2704|1|1|1|1,1|cuda|nvml|NVIDIA|GeForce RTX 4080
0x10de|0x2504|1|1|1|1,1|cuda|nvml|NVIDIA|GeForce RTX 3060
0x10de|0x21c4|1|1|1|1,0|cuda|nvml|NVIDIA|GeForce GTX 1660 SUPER
0x6766|0x3d06|1|0|1|1,0|ponn|glenfly|Glenfly|Arise-GT-10C0t
0x6766|0x3d07|1|0|1|1,0|ponn|glenfly|Glenfly|Arise-GT-2030
0x1f6b|0x0c00|2|1|0|0,1|houmo|houmo|HOUMO.AI|LQ50
";

// 白名单行格式（共 10 段，'|' 分隔）：
// vendor_id|device_id|device_type|supportShareKytensor|supportStandardGGUF|supportedModels|llamacpp_plugin_name|kyml_plugin_name|vendor|model
const WHITELIST_FIELD_COUNT: usize = 10;
const WHITELIST_FIELD_KYML_PLUGIN: usize = 7;
const WHITELIST_FIELD_VENDOR: usize = 8;

fn parse_hex_u16(token: &str) -> Option<u16> {
    let token = token.trim();
    let token = token
        .strip_prefix("0x")
        .or_else(|| token.strip_prefix("0X"))
        .unwrap_or(token);
    u16::from_str_radix(token, 16).ok()
}

fn read_hex_file(dir: &Path, name: &str) -> Option<u16> {
    let text = std::fs::read_to_string(dir.join(name)).ok()?;
    parse_hex_u16(text.trim())
}

struct WhitelistEntry {
    vendor_id: u16,
    device_id: u16,
}

fn parse_whitelist_line(line: &str) -> Option<WhitelistEntry> {
    let line = line.trim();
    if line.is_empty() || line.starts_with('#') || line.starts_with("//") {
        return None;
    }
    let tokens: Vec<&str> = line.split('|').collect();
    if tokens.len() < WHITELIST_FIELD_COUNT {
        return None;
    }
    let vendor = tokens[WHITELIST_FIELD_VENDOR].trim().to_lowercase();
    let plugin = tokens[WHITELIST_FIELD_KYML_PLUGIN].trim().to_lowercase();
    if !vendor.contains("houmo") && !plugin.contains("houmo") {
        return None;
    }
    Some(WhitelistEntry {
        vendor_id: parse_hex_u16(tokens[0])?,
        device_id: parse_hex_u16(tokens[1])?,
    })
}

fn pci_has_device(sys_pci: &Path, vendor_id: u16, device_id: u16) -> bool {
    let Ok(entries) = std::fs::read_dir(sys_pci) else {
        return false;
    };
    for entry in entries.flatten() {
        let path = entry.path();
        if read_hex_file(&path, "vendor") == Some(vendor_id)
            && read_hex_file(&path, "device") == Some(device_id)
        {
            return true;
        }
    }
    false
}

pub fn has_houmo_card_paths(sys_pci: &Path) -> bool {
    for line in KYML_WHITELIST.lines() {
        let Some(entry) = parse_whitelist_line(line) else {
            continue;
        };
        if pci_has_device(sys_pci, entry.vendor_id, entry.device_id) {
            return true;
        }
    }
    false
}

// 测试用：设置 MMT_FORCE_HOUMO_CARD=1/0 可跳过 PCI 扫描，强制判定有/无后摩卡。
const FORCE_HOUMO_CARD_ENV: &str = "MMT_FORCE_HOUMO_CARD";

fn parse_forced_flag(value: &str) -> Option<bool> {
    match value.trim() {
        "1" => Some(true),
        "0" => Some(false),
        _ => None,
    }
}

pub fn has_houmo_card() -> bool {
    if let Some(forced) = std::env::var(FORCE_HOUMO_CARD_ENV)
        .ok()
        .and_then(|value| parse_forced_flag(&value))
    {
        return forced;
    }
    has_houmo_card_paths(Path::new(SYS_PCI_DEVICES))
}

// model_name 固定格式：llm_<核心名>_<版本>，如 llm_Qwen-3.6-35ba3b-1_1.0。
// 去掉前缀 llm_ 与版本后缀后，核心名以 -1 结尾为后摩模型，-0 为普通模型。
pub fn is_houmo_model_name(model_name: &str) -> bool {
    let name = model_name.trim();
    let body = name.strip_prefix("llm_").unwrap_or(name);
    let core = body.rsplit_once('_').map(|(core, _version)| core).unwrap_or(body);
    core.ends_with("-1")
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    fn write_pci_device(sys: &Path, addr: &str, vendor: &str, device: &str) {
        let dir = sys.join(addr);
        fs::create_dir_all(&dir).unwrap();
        fs::write(dir.join("vendor"), format!("{vendor}\n")).unwrap();
        fs::write(dir.join("device"), format!("{device}\n")).unwrap();
    }

    #[test]
    fn parses_hex_tokens() {
        assert_eq!(parse_hex_u16("0x1f6b"), Some(0x1f6b));
        assert_eq!(parse_hex_u16("1f6b"), Some(0x1f6b));
        assert_eq!(parse_hex_u16("0X0C00"), Some(0x0c00));
        assert_eq!(parse_hex_u16("zzz"), None);
    }

    #[test]
    fn parses_only_houmo_whitelist_lines() {
        let mut entries = KYML_WHITELIST.lines().filter_map(parse_whitelist_line);
        let entry = entries.next().expect("whitelist should contain a houmo entry");
        assert_eq!(entry.vendor_id, 0x1f6b);
        assert_eq!(entry.device_id, 0x0c00);
        assert!(entries.next().is_none());
    }

    #[test]
    fn detects_houmo_card_by_pci_scan() {
        let tmp = tempfile::tempdir().unwrap();
        let sys = tmp.path().join("pci");
        fs::create_dir_all(&sys).unwrap();
        write_pci_device(&sys, "0000:00:00.0", "0x8086", "0x1234");
        assert!(!has_houmo_card_paths(&sys));
        write_pci_device(&sys, "0000:01:00.0", "0x1f6b", "0x0c00");
        assert!(has_houmo_card_paths(&sys));
    }

    #[test]
    fn nvidia_device_is_not_houmo_card() {
        let tmp = tempfile::tempdir().unwrap();
        let sys = tmp.path().join("pci");
        fs::create_dir_all(&sys).unwrap();
        write_pci_device(&sys, "0000:01:00.0", "0x10de", "0x2684");
        assert!(!has_houmo_card_paths(&sys));
    }

    #[test]
    fn unknown_houmo_device_id_is_not_matched() {
        let tmp = tempfile::tempdir().unwrap();
        let sys = tmp.path().join("pci");
        fs::create_dir_all(&sys).unwrap();
        write_pci_device(&sys, "0000:01:00.0", "0x1f6b", "0xffff");
        assert!(!has_houmo_card_paths(&sys));
    }

    #[test]
    fn missing_sysfs_means_no_card() {
        let tmp = tempfile::tempdir().unwrap();
        assert!(!has_houmo_card_paths(&tmp.path().join("no-sysfs")));
    }

    #[test]
    fn parses_forced_houmo_card_flag() {
        assert_eq!(parse_forced_flag("1"), Some(true));
        assert_eq!(parse_forced_flag(" 0 "), Some(false));
        assert_eq!(parse_forced_flag("true"), None);
        assert_eq!(parse_forced_flag(""), None);
    }

    #[test]
    fn identifies_houmo_model_name() {
        assert!(is_houmo_model_name("llm_Qwen-3.6-35ba3b-1_1.0"));
        assert!(!is_houmo_model_name("llm_Qwen-3.6-35ba3b-0_1.0"));
        assert!(!is_houmo_model_name("llm_demo_0.1"));
        assert!(!is_houmo_model_name(""));
        assert!(!is_houmo_model_name("llm_Qwen-3.6-35ba3b-10_1.0"));
    }
}
