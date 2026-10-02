//! Unified peer address for TCP and Unix-domain gateway listeners.
//!
//! Axum's [`ConnectInfo`] type must match the [`axum::extract::connect_info::Connected`]
//! implementation for each listener. We use a newtype around [`SocketAddr`] so local
//! processes connecting over a Unix socket are treated as loopback for rate limits
//! and admin localhost checks.

use axum::extract::connect_info::Connected;
use axum::serve::IncomingStream;
use std::net::SocketAddr;
use std::ops::Deref;
use tokio::net::TcpListener;

/// Peer address presented to gateway handlers (`ConnectInfo<GatewayPeerAddr>`).
///
/// TCP connections use the real remote [`SocketAddr`]. Unix-domain connections are mapped
/// to `127.0.0.1:0` so existing localhost and rate-key logic continues to work.
#[derive(Clone, Copy, Debug)]
pub struct GatewayPeerAddr(pub SocketAddr);

impl Deref for GatewayPeerAddr {
    type Target = SocketAddr;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

/// Used by the manual TLS accept path, which calls `make_service` with a plain [`SocketAddr`]
/// (same contract as Axum's `Connected<SocketAddr>` for [`SocketAddr`]).
impl Connected<SocketAddr> for GatewayPeerAddr {
    fn connect_info(remote_addr: SocketAddr) -> Self {
        Self(remote_addr)
    }
}

impl Connected<IncomingStream<'_, TcpListener>> for GatewayPeerAddr {
    fn connect_info(stream: IncomingStream<'_, TcpListener>) -> Self {
        Self(*stream.remote_addr())
    }
}

#[cfg(unix)]
impl Connected<IncomingStream<'_, tokio::net::UnixListener>> for GatewayPeerAddr {
    fn connect_info(_stream: IncomingStream<'_, tokio::net::UnixListener>) -> Self {
        Self(SocketAddr::from(([127, 0, 0, 1], 0)))
    }
}
