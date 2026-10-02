//! TLS 信任锚：`--ca-certificate` / `--ca-directory`（GNU wget 语义子集）。

use crate::cli::Cli;
use crate::exit::ExitCode;
use reqwest::Certificate;
use std::fs;

/// 将 PEM 根证书加入 `reqwest` 信任存储（**追加**在默认 Mozilla 根库之后）。
pub fn apply_custom_ca_roots(
    mut builder: reqwest::ClientBuilder,
    cli: &Cli,
) -> Result<reqwest::ClientBuilder, ExitCode> {
    if cli.ca_certificate.is_none() && cli.ca_directory.is_none() {
        return Ok(builder);
    }

    if let Some(ref p) = cli.ca_certificate {
        let data = fs::read(p).map_err(|_| ExitCode::IoFail)?;
        let certs = Certificate::from_pem_bundle(&data).map_err(|_| ExitCode::ParseError)?;
        for c in certs {
            builder = builder.add_root_certificate(c);
        }
    }

    if let Some(ref dir) = cli.ca_directory {
        let entries = fs::read_dir(dir).map_err(|_| ExitCode::IoFail)?;
        let mut paths: Vec<_> = entries.filter_map(|e| e.ok()).map(|e| e.path()).collect();
        paths.sort();
        for path in paths {
            if !path.is_file() {
                continue;
            }
            let data = fs::read(&path).map_err(|_| ExitCode::IoFail)?;
            if let Ok(certs) = Certificate::from_pem_bundle(&data) {
                for c in certs {
                    builder = builder.add_root_certificate(c);
                }
            } else if let Ok(c) = Certificate::from_pem(&data) {
                builder = builder.add_root_certificate(c);
            }
        }
    }

    Ok(builder)
}
