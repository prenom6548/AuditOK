//! HTTP client for communicating with the KylinBot gateway.

use anyhow::{Context, Result};

pub struct GatewayClient {
    pub base_url: String,
    pub token: Option<String>,
    client: reqwest::Client,
}

impl GatewayClient {
    pub fn new(base_url: &str, token: Option<&str>) -> Self {
        let client = reqwest::Client::builder()
            .timeout(std::time::Duration::from_secs(10))
            .build()
            .unwrap_or_default();
        Self {
            base_url: base_url.to_string(),
            token: token.map(String::from),
            client,
        }
    }

    pub fn auth_header(&self) -> Option<String> {
        self.token.as_ref().map(|t| format!("Bearer {t}"))
    }

    pub async fn get_status(&self) -> Result<serde_json::Value> {
        let mut req = self.client.get(format!("{}/api/status", self.base_url));
        if let Some(auth) = self.auth_header() {
            req = req.header("Authorization", auth);
        }
        let resp = req.send().await.context("status request failed")?;
        Ok(resp.json().await?)
    }

    pub async fn get_health(&self) -> Result<bool> {
        match self
            .client
            .get(format!("{}/health", self.base_url))
            .send()
            .await
        {
            Ok(resp) => Ok(resp.status().is_success()),
            Err(_) => Ok(false),
        }
    }

    pub async fn get_devices(&self) -> Result<serde_json::Value> {
        let mut req = self.client.get(format!("{}/api/devices", self.base_url));
        if let Some(auth) = self.auth_header() {
            req = req.header("Authorization", auth);
        }
        let resp = req.send().await.context("devices request failed")?;
        Ok(resp.json().await?)
    }

    pub async fn initiate_pairing(&self) -> Result<serde_json::Value> {
        let mut req = self
            .client
            .post(format!("{}/api/pairing/initiate", self.base_url));
        if let Some(auth) = self.auth_header() {
            req = req.header("Authorization", auth);
        }
        let resp = req.send().await.context("pairing request failed")?;
        Ok(resp.json().await?)
    }

    pub async fn requires_pairing(&self) -> Result<bool> {
        let resp = self
            .client
            .get(format!("{}/health", self.base_url))
            .send()
            .await
            .context("health request failed")?;
        let body: serde_json::Value = resp.json().await?;
        Ok(body["require_pairing"].as_bool().unwrap_or(false))
    }

    pub async fn request_new_paircode(&self) -> Result<String> {
        let resp = self
            .client
            .post(format!("{}/admin/paircode/new", self.base_url))
            .send()
            .await
            .context("paircode request failed")?;
        let body: serde_json::Value = resp.json().await?;
        body["pairing_code"]
            .as_str()
            .map(String::from)
            .context("no pairing_code in response")
    }

    pub async fn pair_with_code(&self, code: &str) -> Result<String> {
        let resp = self
            .client
            .post(format!("{}/pair", self.base_url))
            .header("X-Pairing-Code", code)
            .send()
            .await
            .context("pair request failed")?;
        if !resp.status().is_success() {
            anyhow::bail!("pair request returned {}", resp.status());
        }
        let body: serde_json::Value = resp.json().await?;
        body["token"]
            .as_str()
            .map(String::from)
            .context("no token in pair response")
    }

    pub async fn validate_token(&self) -> Result<bool> {
        let mut req = self.client.get(format!("{}/api/status", self.base_url));
        if let Some(auth) = self.auth_header() {
            req = req.header("Authorization", auth);
        }
        match req.send().await {
            Ok(resp) => Ok(resp.status().is_success()),
            Err(_) => Ok(false),
        }
    }

    pub async fn auto_pair(&self) -> Result<String> {
        let code = self.request_new_paircode().await?;
        self.pair_with_code(&code).await
    }

    pub async fn send_webhook_message(&self, message: &str) -> Result<serde_json::Value> {
        let mut req = self
            .client
            .post(format!("{}/webhook", self.base_url))
            .json(&serde_json::json!({ "message": message }));
        if let Some(auth) = self.auth_header() {
            req = req.header("Authorization", auth);
        }
        let resp = req.send().await.context("webhook request failed")?;
        Ok(resp.json().await?)
    }
}
