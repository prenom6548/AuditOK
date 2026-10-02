use anyhow::{Context, Result};
use reqwest::Client;
use tempfile::NamedTempFile;

use crate::extract;
use crate::models::{
    ShowcaseResponse, SkillDetail, SkillIndexEntry, SkillSearchResult, SkillVersionsResponse,
};

const SKILLHUB_API_BASE: &str = "https://api.skillhub.cn/api/v1";
const MAX_DOWNLOAD_BYTES: u64 = 50 * 1024 * 1024;

#[derive(Debug, Clone)]
pub struct ApiClient {
    client: Client,
    base_url: String,
}

impl Default for ApiClient {
    fn default() -> Self {
        Self {
            client: Client::builder()
                .user_agent("zeroclaw-skillhub/0.1.0")
                .build()
                .expect("failed to build HTTP client"),
            base_url: SKILLHUB_API_BASE.to_string(),
        }
    }
}

impl ApiClient {
    pub fn new() -> Self {
        Self::default()
    }

    fn extract_display_name(item: &serde_json::Value, slug: &str) -> String {
        item.get("displayName")
            .or(item.get("name"))
            .and_then(|v| v.as_str())
            .unwrap_or(slug)
            .to_string()
    }

    fn extract_summary(item: &serde_json::Value) -> String {
        item.get("summary")
            .or(item.get("description"))
            .and_then(|v| v.as_str())
            .unwrap_or("")
            .to_string()
    }

    fn extract_version(item: &serde_json::Value) -> String {
        item.get("latestVersion")
            .and_then(|v| v.get("version"))
            .or(item.get("version"))
            .and_then(|v| v.as_str())
            .unwrap_or("")
            .to_string()
    }

    fn extract_downloads(item: &serde_json::Value) -> u64 {
        item.get("stats")
            .and_then(|s| s.get("downloads"))
            .or(item.get("downloads"))
            .and_then(|v| v.as_u64())
            .unwrap_or(0)
    }

    fn extract_stars(item: &serde_json::Value) -> u64 {
        item.get("stats")
            .and_then(|s| s.get("stars"))
            .or(item.get("stars"))
            .and_then(|v| v.as_u64())
            .unwrap_or(0)
    }

    fn extract_items_array(data: &serde_json::Value) -> Vec<&serde_json::Value> {
        data.get("items")
            .or(data.get("skills"))
            .or(data.get("results"))
            .and_then(|v| v.as_array())
            .map(|arr| arr.iter().collect())
            .unwrap_or_default()
    }

    pub async fn search(&self, query: &str, limit: Option<u32>) -> Result<Vec<SkillSearchResult>> {
        let mut url = format!("{}/search?q={}", self.base_url, urlencoding::encode(query));
        if let Some(n) = limit {
            url.push_str(&format!("&limit={n}"));
        }
        let resp = self.client.get(&url).send().await;

        let resp = match resp {
            Ok(r) if r.status().is_success() => r,
            Ok(r) if r.status().as_u16() == 404 => return Ok(Vec::new()),
            Ok(r) => {
                anyhow::bail!("search API returned HTTP {} for url ({})", r.status(), url)
            }
            Err(e) => {
                anyhow::bail!("search request failed: {e}")
            }
        };

        let body = resp.text().await?;

        if body.trim().is_empty() || body.trim() == "null" || body.trim() == "[]" {
            return Ok(Vec::new());
        }

        let data: serde_json::Value =
            serde_json::from_str(&body).context("parse search response failed")?;

        let items = Self::extract_items_array(&data);

        let results = items
            .iter()
            .filter_map(|item| {
                let slug = item.get("slug")?.as_str()?.to_string();
                Some(SkillSearchResult {
                    name: Self::extract_display_name(item, &slug),
                    slug: slug.clone(),
                    description: Self::extract_summary(item),
                    version: Self::extract_version(item),
                    author: item
                        .get("author")
                        .and_then(|v| v.as_str())
                        .unwrap_or("")
                        .to_string(),
                    tags: item
                        .get("tags")
                        .and_then(|v| v.as_object())
                        .map(|obj| {
                            obj.keys()
                                .filter(|k| *k != "latest")
                                .map(String::from)
                                .collect()
                        })
                        .or_else(|| {
                            item.get("tags").and_then(|v| v.as_array()).map(|arr| {
                                arr.iter()
                                    .filter_map(|v| v.as_str().map(String::from))
                                    .collect()
                            })
                        })
                        .unwrap_or_default(),
                    downloads: Some(Self::extract_downloads(item)),
                    stars: Some(Self::extract_stars(item)),
                    created_at: item
                        .get("createdAt")
                        .and_then(|v| v.as_i64())
                        .map(|ts| ts.to_string()),
                    updated_at: item
                        .get("updatedAt")
                        .and_then(|v| v.as_i64())
                        .map(|ts| ts.to_string()),
                    profile_url: item
                        .get("profileUrl")
                        .or(item.get("homepage"))
                        .and_then(|v| v.as_str())
                        .map(String::from),
                    zip_url: item
                        .get("zipUrl")
                        .and_then(|v| v.as_str())
                        .map(String::from),
                    sha256: item
                        .get("sha256")
                        .and_then(|v| v.as_str())
                        .map(String::from),
                })
            })
            .collect();

        Ok(results)
    }

    pub async fn showcase(&self, scene: &str) -> Result<ShowcaseResponse> {
        let url = format!(
            "{}/showcase/{}",
            self.base_url,
            urlencoding::encode(scene)
        );
        let resp = self.client.get(&url).send().await;

        let resp = match resp {
            Ok(r) if r.status().is_success() => r,
            Ok(r) if r.status().as_u16() == 404 => {
                return Ok(ShowcaseResponse::default());
            }
            Ok(r) => {
                anyhow::bail!(
                    "showcase API returned HTTP {} for url ({})",
                    r.status(),
                    url
                )
            }
            Err(e) => {
                anyhow::bail!("showcase request failed: {e}")
            }
        };

        let data: ShowcaseResponse = resp
            .json()
            .await
            .context("parse showcase response failed")?;

        Ok(data)
    }

    pub async fn index(&self) -> Result<Vec<SkillIndexEntry>> {
        let url = format!("{}/skills", self.base_url);
        let resp = self.client.get(&url).send().await;

        let resp = match resp {
            Ok(r) if r.status().is_success() => r,
            Ok(r) if r.status().as_u16() == 404 => return Ok(Vec::new()),
            Ok(r) => {
                anyhow::bail!("index API returned HTTP {} for url ({})", r.status(), url)
            }
            Err(e) => {
                anyhow::bail!("index request failed: {e}")
            }
        };

        let body = resp.text().await?;

        if body.trim().is_empty() || body.trim() == "null" {
            return Ok(Vec::new());
        }

        let data: serde_json::Value =
            serde_json::from_str(&body).context("parse index response failed")?;

        let items = Self::extract_items_array(&data);

        let skills = items
            .iter()
            .filter_map(|item| {
                let slug = item.get("slug")?.as_str()?.to_string();
                Some(SkillIndexEntry {
                    slug: slug.clone(),
                    name: Self::extract_display_name(item, &slug),
                    description: Self::extract_summary(item),
                    version: Self::extract_version(item),
                    downloads: Self::extract_downloads(item),
                    stars: Self::extract_stars(item),
                })
            })
            .collect();

        Ok(skills)
    }

    pub async fn download(&self, slug: &str) -> Result<(NamedTempFile, String)> {
        let url = format!(
            "{}/download?slug={}",
            self.base_url,
            urlencoding::encode(slug)
        );
        let resp = self
            .client
            .get(&url)
            .send()
            .await
            .context("download request failed")?
            .error_for_status()
            .context("download API error")?;

        let content_length = resp.content_length().unwrap_or(0) as u64;
        if content_length > MAX_DOWNLOAD_BYTES {
            anyhow::bail!(
                "download too large: {} bytes (max {} MB)",
                content_length,
                MAX_DOWNLOAD_BYTES / (1024 * 1024)
            );
        }

        let tmp = NamedTempFile::new().context("create temp file")?;
        let mut writer = tokio::fs::File::from_std(tmp.reopen()?);

        use tokio::io::AsyncWriteExt;
        let bytes = resp.bytes().await.context("read download body")?;
        writer.write_all(&bytes).await.context("write temp file")?;
        writer.flush().await?;

        let sha256 = extract::sha256_file(tmp.path())?;
        Ok((tmp, sha256))
    }

    pub async fn try_download(&self, url: &str) -> Result<(NamedTempFile, String)> {
        let resp = self
            .client
            .get(url)
            .send()
            .await
            .context("download request failed")?;
        if !resp.status().is_success() {
            anyhow::bail!("download failed: HTTP {}", resp.status());
        }
        let bytes = resp.bytes().await?;
        let mut tmp = NamedTempFile::new()?;
        std::io::Write::write_all(&mut tmp, &bytes)?;
        let sha = extract::sha256_bytes(&bytes);
        Ok((tmp, sha))
    }

    pub async fn detail(&self, slug: &str) -> Result<SkillDetail> {
        let url = format!(
            "{}/skills/{}",
            self.base_url,
            urlencoding::encode(slug)
        );
        let resp = self
            .client
            .get(&url)
            .send()
            .await
            .context("detail request failed")?
            .error_for_status()
            .context("detail API error")?;

        let data: SkillDetail = resp
            .json()
            .await
            .context("parse detail response failed")?;

        Ok(data)
    }

    pub async fn versions(&self, slug: &str) -> Result<SkillVersionsResponse> {
        let url = format!(
            "{}/skills/{}/versions",
            self.base_url,
            urlencoding::encode(slug)
        );
        let resp = self
            .client
            .get(&url)
            .send()
            .await
            .context("versions request failed")?
            .error_for_status()
            .context("versions API error")?;

        let data: SkillVersionsResponse = resp
            .json()
            .await
            .context("parse versions response failed")?;

        Ok(data)
    }

    pub async fn fetch_json(&self, url: &str, timeout_secs: u64) -> Result<serde_json::Value> {
        let resp = self
            .client
            .get(url)
            .timeout(std::time::Duration::from_secs(timeout_secs))
            .send()
            .await
            .context("fetch request failed")?
            .error_for_status()
            .context("fetch API error")?
            .json::<serde_json::Value>()
            .await
            .context("parse fetch response failed")?;
        Ok(resp)
    }
}
