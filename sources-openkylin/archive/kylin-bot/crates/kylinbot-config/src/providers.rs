use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use kylinbot_macros::Configurable;

use super::schema::{EmbeddingRouteConfig, ModelProviderConfig, ModelRouteConfig};

/// Top-level `[providers]` section. Wraps model provider profiles, routing rules,
/// and an optional fallback reference.
#[derive(Debug, Clone, Serialize, Deserialize, Configurable, Default)]
#[cfg_attr(feature = "schema-export", derive(schemars::JsonSchema))]
#[prefix = "providers"]
pub struct ProvidersConfig {
    /// Key of the provider entry to use when no route matches.
    /// Optional — if unset, requests without a matching route fail at runtime.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub fallback: Option<String>,

    /// Named model provider profiles keyed by id.
    #[serde(default, skip_serializing_if = "HashMap::is_empty")]
    #[nested]
    pub models: HashMap<String, ModelProviderConfig>,

    /// Model routing rules — route `hint:<name>` to specific provider+model combos.
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    pub model_routes: Vec<ModelRouteConfig>,

    /// Embedding routing rules — route `hint:<name>` to specific provider+model combos.
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    pub embedding_routes: Vec<EmbeddingRouteConfig>,
}

impl ProvidersConfig {
    /// Drop `[providers.models.*]` shells that contain no model / provider label / endpoint / api key,
    /// so deletes from the dashboard do not leave stale empty table headers in `config.toml`.
    pub fn prune_empty_model_profiles(&mut self) {
        self.models
            .retain(|_profile_key, profile| profile.has_identity_fields());
    }

    pub fn fallback_provider(&self) -> Option<&ModelProviderConfig> {
        self.fallback
            .as_deref()
            .and_then(|name| self.models.get(name))
    }
    pub fn fallback_provider_mut(&mut self) -> Option<&mut ModelProviderConfig> {
        let name = self.fallback.clone()?;
        self.models.get_mut(&name)
    }

    /// Return the first concrete `model` string available for use as a default.
    ///
    /// Resolution order:
    ///
    /// 1. The fallback provider's `model` field, if set.
    /// 2. The first entry in `models` (iteration order) that has `model` set.
    ///
    /// Returns `None` only when no provider entry has any model configured at all.
    /// Callers should treat `None` as a configuration error and surface it rather
    /// than silently substituting a hardcoded model identifier.
    pub fn resolve_default_model(&self) -> Option<String> {
        if let Some(model) = self
            .fallback_provider()
            .and_then(|e| e.model.as_deref())
            .map(str::trim)
            .filter(|m| !m.is_empty())
        {
            return Some(model.to_string());
        }

        self.models
            .values()
            .filter_map(|entry| entry.model.as_deref().map(str::trim))
            .find(|m| !m.is_empty())
            .map(ToString::to_string)
    }

    /// True when `key` is a runtime-only mirror inserted by
    /// `Config::apply_named_model_provider_profile()` (for example
    /// `custom:http://localhost:11434` cloned from `[providers.models.ollama]`).
    ///
    /// Dashboard list endpoints use this to hide alias rows that are not
    /// user-authored TOML entries.
    pub fn is_runtime_model_alias(&self, key: &str) -> bool {
        let key = key.trim();
        if key.is_empty() {
            return false;
        }

        if let Some(url) = key.strip_prefix("custom:") {
            let normalized = normalize_provider_base_url(url);
            if normalized.is_empty() {
                return false;
            }
            return self.models.iter().any(|(profile_key, profile)| {
                profile_key != key
                    && !profile_key.starts_with("custom:")
                    && profile
                        .base_url
                        .as_deref()
                        .map(normalize_provider_base_url)
                        .filter(|value| !value.is_empty())
                        == Some(normalized.clone())
            });
        }

        if key == "openai-codex" {
            return self.models.iter().any(|(profile_key, profile)| {
                profile_key != key && profile_uses_responses_api(profile)
            });
        }

        // Mirror the same rule used when inserting name aliases during load:
        // only when `name` differs from the profile's config key.
        self.models.iter().any(|(profile_key, profile)| {
            profile_key != key
                && !profile_key.starts_with("custom:")
                && profile
                    .name
                    .as_deref()
                    .map(str::trim)
                    .filter(|value| !value.is_empty())
                    .is_some_and(|name| {
                        name.eq_ignore_ascii_case(key) && !name.eq_ignore_ascii_case(profile_key)
                    })
        })
    }
}

fn normalize_provider_base_url(raw: &str) -> String {
    raw.trim().trim_end_matches('/').to_string()
}

fn profile_uses_responses_api(profile: &ModelProviderConfig) -> bool {
    matches!(
        profile
            .wire_api
            .as_deref()
            .map(str::trim)
            .unwrap_or("")
            .to_ascii_lowercase()
            .as_str(),
        "responses" | "openai-responses" | "open-ai-responses"
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    fn ollama_profile() -> ModelProviderConfig {
        ModelProviderConfig {
            base_url: Some("http://localhost:11434".to_string()),
            model: Some("qwen2.5:1.5b".to_string()),
            ..Default::default()
        }
    }

    #[test]
    fn custom_base_url_mirror_is_runtime_alias() {
        let mut cfg = ProvidersConfig::default();
        let profile = ollama_profile();
        cfg.models.insert("ollama".into(), profile.clone());
        cfg.models
            .insert("custom:http://localhost:11434".into(), profile);
        assert!(cfg.is_runtime_model_alias("custom:http://localhost:11434"));
        assert!(!cfg.is_runtime_model_alias("ollama"));
    }

    #[test]
    fn ollama_with_matching_name_field_stays_visible_when_custom_mirror_exists() {
        let mut cfg = ProvidersConfig::default();
        let profile = ModelProviderConfig {
            base_url: Some("http://localhost:11434".to_string()),
            model: Some("qwen2.5:1.5b".to_string()),
            name: Some("ollama".to_string()),
            ..Default::default()
        };
        cfg.models.insert("ollama".into(), profile.clone());
        cfg.models
            .insert("custom:http://localhost:11434".into(), profile);
        assert!(
            !cfg.is_runtime_model_alias("ollama"),
            "canonical ollama entry must remain visible even when custom: mirror shares name"
        );
        assert!(cfg.is_runtime_model_alias("custom:http://localhost:11434"));
    }

    #[test]
    fn standalone_custom_provider_is_not_runtime_alias() {
        let mut cfg = ProvidersConfig::default();
        cfg.models
            .insert("custom:http://localhost:11434".into(), ollama_profile());
        assert!(!cfg.is_runtime_model_alias("custom:http://localhost:11434"));
    }

    #[test]
    fn profile_name_mirror_is_runtime_alias() {
        let mut cfg = ProvidersConfig::default();
        cfg.models.insert(
            "primary".into(),
            ModelProviderConfig {
                name: Some("alias-name".into()),
                base_url: Some("https://example.invalid/v1".into()),
                model: Some("primary-model".into()),
                ..Default::default()
            },
        );
        cfg.models.insert(
            "alias-name".into(),
            ModelProviderConfig {
                name: Some("alias-name".into()),
                base_url: Some("https://example.invalid/v1".into()),
                model: Some("primary-model".into()),
                ..Default::default()
            },
        );
        assert!(cfg.is_runtime_model_alias("alias-name"));
        assert!(!cfg.is_runtime_model_alias("primary"));
    }
}
