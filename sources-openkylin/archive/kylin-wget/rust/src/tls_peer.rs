//! 读取 HTTPS 服务端证书签发者（GNU wget 证书错误日志格式）。

use rustls::client::danger::{HandshakeSignatureValid, ServerCertVerified, ServerCertVerifier};
use rustls::pki_types::{CertificateDer, IpAddr, ServerName, UnixTime};
use rustls::{DigitallySignedStruct, Error, SignatureScheme};
use std::sync::{Arc, Mutex};
use tokio::net::TcpStream;
use tokio_rustls::TlsConnector;
use x509_parser::certificate::X509Certificate;
use x509_parser::prelude::FromDer;
use x509_parser::oid_registry::{
    Oid, OID_X509_COMMON_NAME, OID_X509_COUNTRY_NAME, OID_X509_LOCALITY_NAME,
    OID_X509_ORGANIZATION_NAME, OID_X509_ORGANIZATIONAL_UNIT, OID_X509_STATE_OR_PROVINCE_NAME,
};

#[derive(Debug)]
struct CaptureCertVerifier {
    der: Arc<Mutex<Option<Vec<u8>>>>,
}

impl ServerCertVerifier for CaptureCertVerifier {
    fn verify_server_cert(
        &self,
        end_entity: &CertificateDer<'_>,
        _intermediates: &[CertificateDer<'_>],
        _server_name: &ServerName<'_>,
        _ocsp: &[u8],
        _now: UnixTime,
    ) -> Result<ServerCertVerified, Error> {
        *self.der.lock().unwrap() = Some(end_entity.as_ref().to_vec());
        Ok(ServerCertVerified::assertion())
    }

    fn verify_tls12_signature(
        &self,
        _message: &[u8],
        _cert: &CertificateDer<'_>,
        _dss: &DigitallySignedStruct,
    ) -> Result<HandshakeSignatureValid, Error> {
        Ok(HandshakeSignatureValid::assertion())
    }

    fn verify_tls13_signature(
        &self,
        _message: &[u8],
        _cert: &CertificateDer<'_>,
        _dss: &DigitallySignedStruct,
    ) -> Result<HandshakeSignatureValid, Error> {
        Ok(HandshakeSignatureValid::assertion())
    }

    fn supported_verify_schemes(&self) -> Vec<SignatureScheme> {
        vec![
            SignatureScheme::RSA_PKCS1_SHA256,
            SignatureScheme::RSA_PKCS1_SHA384,
            SignatureScheme::RSA_PKCS1_SHA512,
            SignatureScheme::RSA_PSS_SHA256,
            SignatureScheme::RSA_PSS_SHA384,
            SignatureScheme::RSA_PSS_SHA512,
            SignatureScheme::ECDSA_NISTP256_SHA256,
            SignatureScheme::ECDSA_NISTP384_SHA384,
            SignatureScheme::ECDSA_NISTP521_SHA512,
            SignatureScheme::ED25519,
            SignatureScheme::ED448,
        ]
    }
}

fn attr_short_name(oid: &Oid) -> &'static str {
    if oid == &OID_X509_COMMON_NAME {
        "CN"
    } else if oid == &OID_X509_ORGANIZATION_NAME {
        "O"
    } else if oid == &OID_X509_ORGANIZATIONAL_UNIT {
        "OU"
    } else if oid == &OID_X509_COUNTRY_NAME {
        "C"
    } else if oid == &OID_X509_STATE_OR_PROVINCE_NAME {
        "ST"
    } else if oid == &OID_X509_LOCALITY_NAME {
        "L"
    } else {
        "OID"
    }
}

/// GNU：`CN=Wget Rust Test CA`（`=` 两侧无空格，多 RDN 以 `, ` 连接）。
pub fn format_issuer_dn_gnu(der: &[u8]) -> Option<String> {
    let (_, cert) = X509Certificate::from_der(der).ok()?;
    let mut parts = Vec::new();
    for rdn in cert.issuer().iter() {
        for attr in rdn.iter() {
            let key = attr_short_name(attr.attr_type());
            let val = attr.as_str().ok()?.to_string();
            parts.push(format!("{key}={val}"));
        }
    }
    if parts.is_empty() {
        None
    } else {
        Some(parts.join(", "))
    }
}

fn server_name_for_host(host: &str) -> Option<ServerName<'static>> {
    if let Ok(ip) = host.parse::<std::net::IpAddr>() {
        return Some(ServerName::IpAddress(IpAddr::from(ip)));
    }
    ServerName::try_from(host.to_string()).ok()
}

/// TCP + TLS（不校验证书）读取 leaf 证书并解析 **issuer** DN。
pub async fn fetch_peer_certificate_issuer(host: &str, port: u16) -> Option<String> {
    let addr = format!("{host}:{port}");
    let tcp = TcpStream::connect(&addr).await.ok()?;
    let captured = Arc::new(Mutex::new(None));
    let verifier = Arc::new(CaptureCertVerifier {
        der: captured.clone(),
    });
    let config = rustls::ClientConfig::builder()
        .dangerous()
        .with_custom_certificate_verifier(verifier)
        .with_no_client_auth();
    let connector = TlsConnector::from(Arc::new(config));
    let sn = server_name_for_host(host)?;
    let _tls = connector.connect(sn, tcp).await.ok()?;
    let der = captured.lock().ok()?.take()?;
    format_issuer_dn_gnu(&der)
}
