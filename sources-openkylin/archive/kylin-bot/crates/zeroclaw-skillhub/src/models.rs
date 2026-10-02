use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct SkillSearchResult {
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub slug: String,
    #[serde(default)]
    pub description: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub author: String,
    #[serde(default)]
    pub tags: Vec<String>,
    #[serde(default)]
    pub downloads: Option<u64>,
    #[serde(default)]
    pub stars: Option<u64>,
    #[serde(default)]
    pub created_at: Option<String>,
    #[serde(default)]
    pub updated_at: Option<String>,
    #[serde(default)]
    pub profile_url: Option<String>,
    #[serde(default)]
    pub zip_url: Option<String>,
    #[serde(default)]
    pub sha256: Option<String>,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct SkillIndexEntry {
    #[serde(default)]
    pub slug: String,
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub description: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub downloads: u64,
    #[serde(default)]
    pub stars: u64,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct ShowcaseSkill {
    #[serde(default)]
    pub slug: String,
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub description: String,
    #[serde(default, rename = "description_zh")]
    pub description_zh: String,
    #[serde(default)]
    pub downloads: u64,
    #[serde(default)]
    pub stars: u64,
    #[serde(default)]
    pub category: String,
    #[serde(default)]
    pub created_at: i64,
    #[serde(default)]
    pub homepage: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub icon_url: Option<String>,
}

#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct ShowcaseResponse {
    #[serde(default)]
    pub section: String,
    #[serde(default)]
    pub skills: Vec<ShowcaseSkill>,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillDetail {
    #[serde(default)]
    pub latestVersion: SkillDetailLatestVersion,
    #[serde(default)]
    pub owner: SkillDetailOwner,
    #[serde(default)]
    pub securityReports: SkillDetailSecurityReports,
    #[serde(default)]
    pub skill: SkillDetailSkill,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillDetailLatestVersion {
    #[serde(default)]
    pub changelog: String,
    #[serde(default)]
    pub createdAt: i64,
    #[serde(default)]
    pub version: String,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillDetailOwner {
    #[serde(default)]
    pub displayName: String,
    #[serde(default)]
    pub handle: String,
    #[serde(default)]
    pub image: Option<String>,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillDetailSecurityReports {
    #[serde(default)]
    pub keen: SkillDetailSecurityReport,
    #[serde(default)]
    pub sanbu: SkillDetailSecurityReport,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillDetailSecurityReport {
    #[serde(default)]
    pub reportUrl: String,
    #[serde(default)]
    pub status: String,
    #[serde(default)]
    pub statusText: String,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillDetailSkill {
    #[serde(default)]
    pub category: String,
    #[serde(default)]
    pub createdAt: i64,
    #[serde(default)]
    pub displayName: String,
    #[serde(default)]
    pub iconUrl: Option<String>,
    #[serde(default)]
    pub labels: serde_json::Map<String, serde_json::Value>,
    #[serde(default)]
    pub slug: String,
    #[serde(default)]
    pub source: String,
    #[serde(default)]
    pub stats: SkillDetailStats,
    #[serde(default)]
    pub summary: String,
    #[serde(default, rename = "summary_zh")]
    pub summary_zh: String,
    #[serde(default)]
    pub tags: serde_json::Map<String, serde_json::Value>,
    #[serde(default)]
    pub updatedAt: i64,
}

#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillDetailStats {
    #[serde(default)]
    pub comments: u64,
    #[serde(default)]
    pub downloads: u64,
    #[serde(default)]
    pub installs: u64,
    #[serde(default)]
    pub stars: u64,
    #[serde(default)]
    pub versions: u64,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillVersionsResponse {
    #[serde(default)]
    pub slug: String,
    #[serde(default)]
    pub source: String,
    #[serde(default)]
    pub versions: Vec<SkillVersionEntry>,
}

#[allow(non_snake_case)]
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct SkillVersionEntry {
    #[serde(default)]
    pub changelog: String,
    #[serde(default)]
    pub createdAt: i64,
    #[serde(default)]
    pub securityReports: SkillDetailSecurityReports,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub versionId: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct Lockfile {
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub skills: HashMap<String, SkillLockEntry>,
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct SkillLockEntry {
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub slug: String,
    #[serde(default)]
    pub version: String,
    #[serde(default)]
    pub source: String,
    #[serde(default)]
    pub zip_url: String,
    #[serde(default)]
    pub update_url: String,
    #[serde(default)]
    pub sha256: String,
    #[serde(default)]
    pub installed_at: i64,
    #[serde(default)]
    pub detail: Option<SkillDetail>,
}
