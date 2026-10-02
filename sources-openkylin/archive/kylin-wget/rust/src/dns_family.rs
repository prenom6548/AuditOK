//! `--inet4-only` / `--inet6-only`：在 DNS 结果上按 IPv4/IPv6 过滤（GNU `opt.ipv4_only` / `opt.ipv6_only`）。

use crate::cli::Cli;
use crate::exit::ExitCode;
use reqwest::dns::{Addrs, Name, Resolve, Resolving};
use std::io::{Error, ErrorKind};
use std::net::{Ipv4Addr, Ipv6Addr, SocketAddr};
use std::sync::Arc;
use tokio::net::lookup_host;
use url::Url;

/// 字面量 IP 不走自定义 `Resolve`，须与 GNU 一样拒绝「错误族」的主机形式。
pub(crate) fn enforce_literal_host_policy(cli: &Cli, url: &Url) -> Result<(), ExitCode> {
    let Some(host) = url.host_str() else {
        return Ok(());
    };

    if cli.inet6_only && host.parse::<Ipv4Addr>().is_ok() {
        return Err(ExitCode::NetworkFail);
    }
    if cli.inet4_only && host.parse::<Ipv6Addr>().is_ok() {
        return Err(ExitCode::NetworkFail);
    }
    Ok(())
}

type DynErr = Box<dyn std::error::Error + Send + Sync>;

#[derive(Clone, Copy, Debug)]
pub(crate) enum AddrFamilyPolicy {
    Inet4Only,
    Inet6Only,
}

/// 先走系统异步解析（`tokio::net::lookup_host`），再只保留指定地址族。
#[derive(Clone)]
pub(crate) struct FamilyFilterResolver {
    policy: AddrFamilyPolicy,
}

impl FamilyFilterResolver {
    pub fn arc(policy: AddrFamilyPolicy) -> Arc<Self> {
        Arc::new(Self { policy })
    }
}

impl Resolve for FamilyFilterResolver {
    fn resolve(&self, name: Name) -> Resolving {
        let host = name.as_str().to_string();
        let policy = self.policy;
        Box::pin(async move {
            let mut addrs: Vec<SocketAddr> = match lookup_host((host.as_str(), 0u16)).await {
                Ok(iter) => iter.collect(),
                Err(e) => return Err(Box::new(e) as DynErr),
            };
            addrs.sort_by_key(|a| match a.ip() {
                std::net::IpAddr::V4(v) => (0u8, v.octets().to_vec()),
                std::net::IpAddr::V6(v) => (1u8, v.octets().to_vec()),
            });
            let filtered: Vec<SocketAddr> = addrs
                .into_iter()
                .filter(|a| match policy {
                    AddrFamilyPolicy::Inet4Only => a.is_ipv4(),
                    AddrFamilyPolicy::Inet6Only => a.is_ipv6(),
                })
                .collect();
            if filtered.is_empty() {
                return Err(Box::new(Error::new(
                    ErrorKind::AddrNotAvailable,
                    "no addresses in requested address family",
                )) as DynErr);
            }
            Ok(Box::new(filtered.into_iter()) as Addrs)
        })
    }
}
