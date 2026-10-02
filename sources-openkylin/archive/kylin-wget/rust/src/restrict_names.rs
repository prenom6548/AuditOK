use crate::cli::Cli;
use percent_encoding::percent_decode_str;

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum RestrictOs {
    Unix,
    Vms,
    Windows,
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum RestrictCase {
    None,
    Lower,
    Upper,
}

#[derive(Copy, Clone, Debug)]
pub struct RestrictConfig {
    pub os: RestrictOs,
    pub ctrl: bool,
    pub nonascii: bool,
    pub case: RestrictCase,
}

impl RestrictConfig {
    pub fn default_for_build_os() -> Self {
        // GNU: default is based on build OS; on our target linux -> unix
        // (We don't currently build on VMS/MSDOS; keep mapping for completeness.)
        #[cfg(any(windows))]
        let os = RestrictOs::Windows;
        #[cfg(not(any(windows)))]
        let os = RestrictOs::Unix;
        Self {
            os,
            ctrl: true,
            nonascii: false,
            case: RestrictCase::None,
        }
    }
}

pub fn parse_restrict_file_names(val: &str) -> Result<RestrictConfig, String> {
    let mut cfg = RestrictConfig::default_for_build_os();
    for tok in val.split(',').map(|s| s.trim()).filter(|s| !s.is_empty()) {
        match tok {
            "unix" => cfg.os = RestrictOs::Unix,
            "vms" => cfg.os = RestrictOs::Vms,
            "windows" => cfg.os = RestrictOs::Windows,
            "lowercase" => cfg.case = RestrictCase::Lower,
            "uppercase" => cfg.case = RestrictCase::Upper,
            "nocontrol" => cfg.ctrl = false,
            "ascii" => cfg.nonascii = true,
            _ => {
                return Err(crate::i18n::tr_named(
                    "Invalid restriction {tok},\n    use [unix|vms|windows],[lowercase|uppercase],[nocontrol],[ascii].",
                    &[("tok", tok)],
                ));
            }
        }
    }
    Ok(cfg)
}

pub fn effective_restrict_config(cli: &Cli) -> Result<RestrictConfig, String> {
    match cli.restrict_file_names.as_deref() {
        None => Ok(RestrictConfig::default_for_build_os()),
        Some(v) => parse_restrict_file_names(v),
    }
}

fn is_control(b: u8) -> bool {
    b < 0x20 || b == 0x7f
}

fn is_unsafe_char(cfg: RestrictConfig, b: u8) -> bool {
    if cfg.nonascii && b >= 0x80 {
        return true;
    }
    if cfg.ctrl && is_control(b) {
        return true;
    }
    match cfg.os {
        RestrictOs::Unix => b == b'/' || b == 0,
        RestrictOs::Vms => (b < 0x20) || b == b'*' || b == b'?' || b == 0,
        RestrictOs::Windows => match b {
            b'\\' | b'|' | b'/' | b'<' | b'>' | b'?' | b':' | b'*' | b'"' | 0 => true,
            _ => false,
        },
    }
}

fn push_pct(out: &mut Vec<u8>, b: u8) {
    const HEX: &[u8; 16] = b"0123456789ABCDEF";
    out.push(b'%');
    out.push(HEX[(b >> 4) as usize]);
    out.push(HEX[(b & 0x0f) as usize]);
}

pub fn restrict_path_segment(cli: &Cli, segment: &str, escaped: bool) -> Result<String, String> {
    let cfg = effective_restrict_config(cli)?;

    // GNU: if escaped, URL-unescape first, then apply quoting rules.
    let bytes: Vec<u8> = if escaped {
        percent_decode_str(segment).collect()
    } else {
        segment.as_bytes().to_vec()
    };

    // GNU: defang ".." when found as a path component.
    if bytes.as_slice() == b".." {
        return Ok("%2E%2E".to_string());
    }

    let mut out: Vec<u8> = Vec::with_capacity(bytes.len());
    for &b in &bytes {
        if is_unsafe_char(cfg, b) {
            push_pct(&mut out, b);
        } else {
            out.push(b);
        }
    }

    // GNU: inline case transformation after quoting (affects hex digits too).
    if cfg.case != RestrictCase::None {
        for b in &mut out {
            *b = match cfg.case {
                RestrictCase::None => *b,
                RestrictCase::Lower => b.to_ascii_lowercase(),
                RestrictCase::Upper => b.to_ascii_uppercase(),
            }
        }
    }

    Ok(String::from_utf8_lossy(&out).to_string())
}

pub fn port_separator(cli: &Cli) -> Result<char, String> {
    let cfg = effective_restrict_config(cli)?;
    Ok(if cfg.os == RestrictOs::Windows {
        '+'
    } else {
        ':'
    })
}

pub fn query_separator(cli: &Cli) -> Result<char, String> {
    // GNU url.c:FN_QUERY_SEP：unix='?'，vms/windows='@'
    let cfg = effective_restrict_config(cli)?;
    Ok(match cfg.os {
        RestrictOs::Unix => '?',
        RestrictOs::Vms | RestrictOs::Windows => '@',
    })
}
