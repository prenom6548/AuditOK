use anyhow::Context;
use async_trait::async_trait;
use futures_util::{SinkExt, StreamExt};
use kylinbot_api::channel::{Channel, ChannelMessage, SendMessage};
use kylinbot_api::media::{MediaAttachment, MediaKind};
use serde::Deserialize;
use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};
use tokio::sync::RwLock;
use tokio_tungstenite::tungstenite::Message;
use uuid::Uuid;

const DEFAULT_API_GATEWAY_URL: &str = "https://open.e.lanxin.cn/open/apigw";
const APP_TOKEN_ENDPOINT: &str = "/v1/apptoken/create";
const WS_ENDPOINT: &str = "/v1/ws/endpoint/create";
const PRIVATE_MESSAGE_ENDPOINT: &str = "/v1/bot/messages/create";
const GROUP_MESSAGE_ENDPOINT: &str = "/v1/messages/group/create";
const MEDIA_ENDPOINT: &str = "/v1/medias";
const MEDIA_UPLOAD_ENDPOINT: &str = "/v1/medias/create";
const HEARTBEAT_INTERVAL: Duration = Duration::from_secs(30);
const PONG_TIMEOUT: Duration = Duration::from_secs(10);
const IMAGE_URL_DOWNLOAD_TIMEOUT: Duration = Duration::from_secs(15);

// Lansenger upload `type` query parameter.
// Mirrors openclaw-lansenger-channel's mediaTypeFromPath:
//   1 = video, 2 = image, 3 = file (and default fallback).
const LANSENGER_MEDIA_TYPE_VIDEO: u8 = 1;
const LANSENGER_MEDIA_TYPE_IMAGE: u8 = 2;
const LANSENGER_MEDIA_TYPE_FILE: u8 = 3;

const RECONNECT_BACKOFF: &[u64] = &[2, 5, 10, 30, 60];
const TOKEN_REFRESH_SKEW: Duration = Duration::from_secs(300);

#[derive(Debug, Clone)]
struct CachedAppToken {
    value: String,
    refresh_after: Instant,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum LansengerChatType {
    Direct,
    Group,
}

#[derive(Debug, Clone)]
struct LansengerInboundEvent {
    message_id: String,
    sender_id: String,
    reply_target: String,
    content: String,
    timestamp: u64,
    chat_type: LansengerChatType,
    mentions_bot: bool,
    attachments: Vec<MediaAttachment>,
}

#[derive(Debug, Deserialize)]
struct AppTokenResponse {
    #[serde(rename = "errCode")]
    err_code: i64,
    #[serde(rename = "errMsg")]
    err_msg: Option<String>,
    data: Option<AppTokenData>,
}

#[derive(Debug, Deserialize)]
struct AppTokenData {
    #[serde(rename = "appToken")]
    app_token: Option<String>,
    #[serde(rename = "expiresIn")]
    expires_in: Option<u64>,
}

#[derive(Debug, Deserialize)]
struct WsEndpointResponse {
    #[serde(rename = "errCode")]
    err_code: i64,
    #[serde(rename = "errMsg")]
    err_msg: Option<String>,
    data: Option<WsEndpointData>,
}

#[derive(Debug, Deserialize)]
struct WsEndpointData {
    #[serde(rename = "wsEndpoint")]
    ws_endpoint: Option<String>,
}

#[derive(Debug, Deserialize)]
struct LansengerApiResponse {
    #[serde(rename = "errCode")]
    err_code: i64,
    #[serde(rename = "errMsg")]
    err_msg: Option<String>,
}

#[derive(Debug, Deserialize)]
struct UploadMediaResponse {
    #[serde(rename = "errCode")]
    err_code: i64,
    #[serde(rename = "errMsg")]
    err_msg: Option<String>,
    data: Option<UploadMediaData>,
}

#[derive(Debug, Deserialize)]
struct UploadMediaData {
    #[serde(rename = "mediaId")]
    media_id: Option<String>,
}

/// Lansenger / Lanxin channel.
///
/// Inbound messages arrive over a Lansenger-issued WebSocket endpoint. Outbound
/// replies are sent over HTTP using `formatText` first, with a plain `text`
/// fallback for deployments where formatted text is rejected.
pub struct LansengerChannel {
    app_id: String,
    app_secret: String,
    api_gateway_url: String,
    allowed_users: Vec<String>,
    group_allow_from: Vec<String>,
    mention_only: bool,
    proxy_url: Option<String>,
    workspace_dir: Option<PathBuf>,
    app_token: RwLock<Option<CachedAppToken>>,
    chat_types: RwLock<HashMap<String, LansengerChatType>>,
}

impl LansengerChannel {
    pub fn new(
        app_id: String,
        app_secret: String,
        api_gateway_url: String,
        allowed_users: Vec<String>,
        group_allow_from: Vec<String>,
        mention_only: bool,
    ) -> Self {
        Self {
            app_id,
            app_secret,
            api_gateway_url: normalize_api_gateway_url(&api_gateway_url),
            allowed_users,
            group_allow_from,
            mention_only,
            proxy_url: None,
            workspace_dir: None,
            app_token: RwLock::new(None),
            chat_types: RwLock::new(HashMap::new()),
        }
    }

    pub fn from_config(config: &kylinbot_config::schema::LansengerConfig) -> Self {
        Self::new(
            config.app_id.clone(),
            config.app_secret.clone(),
            config.api_gateway_url.clone(),
            config.allowed_users.clone(),
            config.group_allow_from.clone(),
            config.mention_only,
        )
        .with_proxy_url(config.proxy_url.clone())
    }

    pub fn with_workspace_dir(mut self, dir: PathBuf) -> Self {
        self.workspace_dir = Some(dir);
        self
    }

    pub fn with_proxy_url(mut self, proxy_url: Option<String>) -> Self {
        self.proxy_url = proxy_url;
        self
    }

    fn http_client(&self) -> reqwest::Client {
        kylinbot_config::schema::build_channel_proxy_client(
            "channel.lansenger",
            self.proxy_url.as_deref(),
        )
    }

    fn endpoint_url(&self, endpoint: &str) -> String {
        format!("{}{}", self.api_gateway_url, endpoint)
    }

    fn is_user_allowed(&self, user_id: &str) -> bool {
        self.allowed_users
            .iter()
            .any(|allowed| allowed == "*" || allowed == user_id)
    }

    fn is_group_allowed(&self, group_id: &str) -> bool {
        self.group_allow_from
            .iter()
            .any(|allowed| allowed == "*" || allowed == group_id)
    }

    async fn get_app_token(&self) -> anyhow::Result<String> {
        {
            let cached = self.app_token.read().await;
            if let Some(token) = cached.as_ref()
                && Instant::now() < token.refresh_after
            {
                return Ok(token.value.clone());
            }
        }

        let url = format!(
            "{}?grant_type=client_credential&appid={}&secret={}",
            self.endpoint_url(APP_TOKEN_ENDPOINT),
            urlencoding::encode(&self.app_id),
            urlencoding::encode(&self.app_secret)
        );

        let resp = self.http_client().get(url).send().await?;
        let status = resp.status();
        let body = resp.text().await.unwrap_or_default();
        if !status.is_success() {
            anyhow::bail!("Lansenger app token request failed ({status}): {body}");
        }

        let parsed: AppTokenResponse = serde_json::from_str(&body)
            .with_context(|| format!("Lansenger app token response is not JSON: {body}"))?;
        if parsed.err_code != 0 {
            anyhow::bail!(
                "Lansenger app token failed: errCode={} errMsg={}",
                parsed.err_code,
                parsed.err_msg.as_deref().unwrap_or("unknown")
            );
        }

        let data = parsed
            .data
            .ok_or_else(|| anyhow::anyhow!("Lansenger app token response missing data"))?;
        let token = data
            .app_token
            .filter(|value| !value.is_empty())
            .ok_or_else(|| anyhow::anyhow!("Lansenger app token response missing appToken"))?;
        let expires_in = data.expires_in.unwrap_or(7200).max(1);
        let refresh_after = next_token_refresh_deadline(Instant::now(), expires_in);

        {
            let mut cached = self.app_token.write().await;
            *cached = Some(CachedAppToken {
                value: token.clone(),
                refresh_after,
            });
        }

        Ok(token)
    }

    async fn get_ws_endpoint(&self) -> anyhow::Result<String> {
        let body = serde_json::json!({
            "appId": self.app_id,
            "secret": self.app_secret,
        });
        let resp = self
            .http_client()
            .post(self.endpoint_url(WS_ENDPOINT))
            .json(&body)
            .send()
            .await?;
        let status = resp.status();
        let text = resp.text().await.unwrap_or_default();
        if !status.is_success() {
            anyhow::bail!("Lansenger WS endpoint request failed ({status}): {text}");
        }

        let parsed: WsEndpointResponse = serde_json::from_str(&text)
            .with_context(|| format!("Lansenger WS endpoint response is not JSON: {text}"))?;
        if parsed.err_code != 0 {
            anyhow::bail!(
                "Lansenger WS endpoint failed: errCode={} errMsg={}",
                parsed.err_code,
                parsed.err_msg.as_deref().unwrap_or("unknown")
            );
        }

        parsed
            .data
            .and_then(|data| data.ws_endpoint)
            .filter(|url| !url.is_empty())
            .ok_or_else(|| anyhow::anyhow!("Lansenger WS endpoint response missing wsEndpoint"))
    }

    async fn process_raw_message(&self, raw: &str) -> Vec<LansengerInboundEvent> {
        let Ok(ws_msg) = serde_json::from_str::<serde_json::Value>(raw) else {
            tracing::debug!("Lansenger: ignoring non-JSON WS frame");
            return Vec::new();
        };

        let Some(events) = ws_msg.get("events").and_then(|events| events.as_array()) else {
            return Vec::new();
        };

        let mut results = Vec::new();
        for event in events {
            let msg_data = event.get("data").unwrap_or(event);
            let Some(content) = self.extract_text(msg_data).await else {
                continue;
            };

            let sender_id = msg_data
                .get("from")
                .and_then(|value| value.as_str())
                .unwrap_or("")
                .to_string();
            if sender_id.is_empty() {
                tracing::debug!("Lansenger: dropping message without sender");
                continue;
            }

            let chat_type = match msg_data.get("chatType").and_then(|value| value.as_str()) {
                Some("group") => LansengerChatType::Group,
                _ => LansengerChatType::Direct,
            };

            let reply_target = match chat_type {
                LansengerChatType::Group => msg_data
                    .get("conversationId")
                    .and_then(|value| value.as_str())
                    .filter(|value| !value.is_empty())
                    .unwrap_or(&sender_id)
                    .to_string(),
                LansengerChatType::Direct => msg_data
                    .get("conversationId")
                    .and_then(|value| value.as_str())
                    .filter(|value| !value.is_empty())
                    .unwrap_or(&sender_id)
                    .to_string(),
            };

            {
                let mut chat_types = self.chat_types.write().await;
                chat_types.insert(reply_target.clone(), chat_type);
            }

            let message_id = msg_data
                .get("messageId")
                .and_then(|value| value.as_str())
                .filter(|value| !value.is_empty())
                .map(ToString::to_string)
                .unwrap_or_else(|| Uuid::new_v4().to_string());

            let attachments = self.extract_attachments(msg_data).await;
            let content = self
                .content_with_image_markers(&message_id, content, &attachments)
                .await;
            let mentions_bot = self.message_mentions_bot(msg_data, &content);

            results.push(LansengerInboundEvent {
                message_id,
                sender_id,
                reply_target,
                content,
                timestamp: current_unix_secs(),
                chat_type,
                mentions_bot,
                attachments,
            });
        }

        results
    }

    async fn extract_text(&self, msg_data: &serde_json::Value) -> Option<String> {
        let msg_type = msg_data
            .get("msgType")
            .and_then(|value| value.as_str())
            .unwrap_or("text");
        let payload = msg_data
            .get("msgData")
            .unwrap_or(&serde_json::Value::Null);

        let text = match msg_type {
            "text" => payload
                .pointer("/text/content")
                .and_then(|value| value.as_str())
                .map(str::trim)
                .filter(|value| !value.is_empty())
                .map(ToString::to_string),
            "formatText" => payload
                .pointer("/formatText/text")
                .and_then(|value| value.as_str())
                .map(str::trim)
                .filter(|value| !value.is_empty())
                .map(ToString::to_string),
            "image" => Some("[Image]".to_string()),
            "video" => Some("[Video]".to_string()),
            "file" => Some("[File]".to_string()),
            "voice" => Some("[Voice]".to_string()),
            "position" => Some(format_position(payload)),
            "card" => payload
                .pointer("/card/staffId")
                .and_then(|value| value.as_str())
                .map(|staff_id| format!("[Contact Card] {staff_id}"))
                .or_else(|| Some("[Contact Card]".to_string())),
            "sticker" => payload
                .pointer("/sticker/stickerId")
                .and_then(|value| value.as_str())
                .map(|sticker_id| format!("[Sticker] {sticker_id}"))
                .or_else(|| Some("[Sticker]".to_string())),
            _ => None,
        }?;

        Some(text.trim().to_string()).filter(|value| !value.is_empty())
    }

    async fn extract_attachments(&self, msg_data: &serde_json::Value) -> Vec<MediaAttachment> {
        let msg_type = msg_data
            .get("msgType")
            .and_then(|value| value.as_str())
            .unwrap_or("text");
        let media_kind = match msg_type {
            "image" | "video" | "file" | "voice" => msg_type,
            _ => return Vec::new(),
        };
        let ids = extract_media_ids(msg_data, media_kind);

        let mut attachments = Vec::new();
        for media_id in ids {
            match self.download_media_attachment(&media_id, media_kind).await {
                Ok(Some(attachment)) => attachments.push(attachment),
                Ok(None) => {}
                Err(err) => tracing::warn!("Lansenger: failed to download media {media_id}: {err}"),
            }
        }

        attachments
    }

    async fn download_media_attachment(
        &self,
        media_id: &str,
        media_kind: &str,
    ) -> anyhow::Result<Option<MediaAttachment>> {
        let token = self.get_app_token().await?;
        let url = format!(
            "{}/{}/fetch?app_token={}",
            self.endpoint_url(MEDIA_ENDPOINT),
            urlencoding::encode(media_id),
            urlencoding::encode(&token)
        );
        let resp = self.http_client().get(url).send().await?;
        let status = resp.status();
        if !status.is_success() {
            let body = resp.text().await.unwrap_or_default();
            tracing::warn!("Lansenger: media fetch failed ({status}) for {media_id}: {body}");
            return Ok(None);
        }

        let content_type = resp
            .headers()
            .get(reqwest::header::CONTENT_TYPE)
            .and_then(|value| value.to_str().ok())
            .map(ToString::to_string);
        let content_disposition = resp
            .headers()
            .get(reqwest::header::CONTENT_DISPOSITION)
            .and_then(|value| value.to_str().ok())
            .map(ToString::to_string);
        let bytes = resp.bytes().await?.to_vec();
        let file_name = infer_media_file_name(media_id, media_kind, content_type.as_deref(), content_disposition.as_deref());

        Ok(Some(MediaAttachment {
            file_name,
            data: bytes,
            mime_type: content_type,
        }))
    }

    async fn content_with_image_markers(
        &self,
        message_id: &str,
        content: String,
        attachments: &[MediaAttachment],
    ) -> String {
        if attachments
            .iter()
            .all(|attachment| attachment.kind() != MediaKind::Image)
        {
            return content;
        }

        if self.workspace_dir.is_none() {
            tracing::debug!(
                "Lansenger: image attachment received but workspace_dir is not configured"
            );
            return content;
        }

        let mut markers = Vec::new();
        for (idx, attachment) in attachments
            .iter()
            .filter(|attachment| attachment.kind() == MediaKind::Image)
            .enumerate()
        {
            match self
                .save_inbound_image_attachment(message_id, idx, attachment)
                .await
            {
                Ok(path) => markers.push(format!("[IMAGE:{}]", path.display())),
                Err(err) => tracing::warn!(
                    file = %attachment.file_name,
                    error = %err,
                    "Lansenger: failed to save inbound image attachment"
                ),
            }
        }

        if markers.is_empty() {
            return content;
        }

        let marker_text = markers.join("\n");
        let trimmed = content.trim();
        if trimmed.is_empty() || trimmed == "[Image]" {
            marker_text
        } else {
            format!("{marker_text}\n\n{trimmed}")
        }
    }

    async fn save_inbound_image_attachment(
        &self,
        message_id: &str,
        index: usize,
        attachment: &MediaAttachment,
    ) -> anyhow::Result<PathBuf> {
        let workspace_dir = self
            .workspace_dir
            .as_ref()
            .ok_or_else(|| anyhow::anyhow!("workspace_dir is not configured"))?;
        let save_dir = workspace_dir.join("lansenger_files");
        tokio::fs::create_dir_all(&save_dir).await?;

        let file_name = inbound_attachment_file_name(
            message_id,
            index,
            &attachment.file_name,
            attachment.mime_type.as_deref(),
        );
        let local_path = save_dir.join(file_name);
        tokio::fs::write(&local_path, &attachment.data).await?;
        Ok(local_path)
    }

    fn message_mentions_bot(&self, msg_data: &serde_json::Value, content: &str) -> bool {
        if content.contains(&self.app_id) {
            return true;
        }

        let reminders = [
            msg_data.pointer("/msgData/text/reminder"),
            msg_data.pointer("/msgData/formatText/reminder"),
        ];
        reminders.into_iter().flatten().any(|reminder| {
            reminder
                .get("all")
                .and_then(|value| value.as_bool())
                .unwrap_or(false)
                || reminder
                    .get("userIds")
                    .and_then(|value| value.as_array())
                    .is_some_and(|ids| {
                        ids.iter()
                            .any(|id| id.as_str().is_some_and(|id| id == self.app_id))
                    })
        })
    }

    fn should_dispatch(&self, event: &LansengerInboundEvent) -> bool {
        match event.chat_type {
            LansengerChatType::Direct => self.is_user_allowed(&event.sender_id),
            LansengerChatType::Group => {
                self.is_group_allowed(&event.reply_target)
                    && (!self.mention_only || event.mentions_bot)
            }
        }
    }

    async fn send_format_text(&self, recipient: &str, content: &str) -> anyhow::Result<()> {
        let payload = serde_json::json!({
            "formatText": {
                "formatType": 1,
                "text": content,
            },
            "msgType": "formatText",
        });
        self.send_message_payload(recipient, payload).await
    }

    async fn send_plain_text(&self, recipient: &str, content: &str) -> anyhow::Result<()> {
        let payload = serde_json::json!({
            "text": {
                "content": content,
            },
            "msgType": "text",
        });
        self.send_message_payload(recipient, payload).await
    }

    async fn send_message_payload(
        &self,
        recipient: &str,
        msg_data: serde_json::Value,
    ) -> anyhow::Result<()> {
        let token = self.get_app_token().await?;
        let chat_type = self.resolve_chat_type(recipient).await;
        let (endpoint, body) = wrap_message_payload(chat_type, recipient, msg_data);
        let url = format!(
            "{}?app_token={}",
            self.endpoint_url(endpoint),
            urlencoding::encode(&token)
        );
        let resp = self.http_client().post(url).json(&body).send().await?;
        let status = resp.status();
        let text = resp.text().await.unwrap_or_default();
        if !status.is_success() {
            anyhow::bail!("Lansenger send failed ({status}): {text}");
        }

        let parsed: LansengerApiResponse = serde_json::from_str(&text)
            .with_context(|| format!("Lansenger send response is not JSON: {text}"))?;
        if parsed.err_code != 0 {
            anyhow::bail!(
                "Lansenger send failed: errCode={} errMsg={}",
                parsed.err_code,
                parsed.err_msg.as_deref().unwrap_or("unknown")
            );
        }

        Ok(())
    }

    async fn resolve_chat_type(&self, recipient: &str) -> LansengerChatType {
        let chat_types = self.chat_types.read().await;
        chat_types
            .get(recipient)
            .copied()
            .unwrap_or_else(|| {
                if self.is_group_allowed(recipient) {
                    LansengerChatType::Group
                } else {
                    LansengerChatType::Direct
                }
            })
    }

    /// Upload a media file to Lansenger and return the resulting `mediaId`.
    ///
    /// `media_type` follows the Lansenger gateway convention: `1` = video,
    /// `2` = image, `3` = file (and the default for unknown extensions).
    pub async fn upload_media(
        &self,
        bytes: Vec<u8>,
        file_name: &str,
        media_type: u8,
    ) -> anyhow::Result<String> {
        let token = self.get_app_token().await?;
        let url = format!(
            "{}?type={}&app_token={}",
            self.endpoint_url(MEDIA_UPLOAD_ENDPOINT),
            media_type,
            urlencoding::encode(&token)
        );

        let part = reqwest::multipart::Part::bytes(bytes).file_name(file_name.to_string());
        let form = reqwest::multipart::Form::new().part("media", part);

        let resp = self.http_client().post(url).multipart(form).send().await?;
        let status = resp.status();
        let text = resp.text().await.unwrap_or_default();
        if !status.is_success() {
            anyhow::bail!("Lansenger upload failed ({status}): {text}");
        }

        let parsed: UploadMediaResponse = serde_json::from_str(&text)
            .with_context(|| format!("Lansenger upload response is not JSON: {text}"))?;
        if parsed.err_code != 0 {
            anyhow::bail!(
                "Lansenger upload failed: errCode={} errMsg={}",
                parsed.err_code,
                parsed.err_msg.as_deref().unwrap_or("unknown")
            );
        }

        parsed
            .data
            .and_then(|data| data.media_id)
            .filter(|value| !value.is_empty())
            .ok_or_else(|| anyhow::anyhow!("Lansenger upload response missing mediaId"))
    }

    /// Send a `text` message that carries one or more media attachments.
    ///
    /// `media_type` is the Lansenger upload-side classification (image / video / file).
    /// Empty `caption` sends a pure media message; non-empty caption is rendered
    /// alongside the media in the receiver's chat.
    pub async fn send_text_with_media(
        &self,
        recipient: &str,
        caption: &str,
        media_type: u8,
        media_ids: &[String],
    ) -> anyhow::Result<()> {
        let payload = serde_json::json!({
            "text": {
                "content": caption,
                "mediaType": media_type,
                "mediaIds": media_ids,
            },
            "msgType": "text",
        });
        self.send_message_payload(recipient, payload).await
    }

    /// Convenience: upload bytes as an image and immediately send it.
    pub async fn send_image_bytes(
        &self,
        recipient: &str,
        bytes: Vec<u8>,
        file_name: &str,
        caption: &str,
    ) -> anyhow::Result<()> {
        let media_id = self
            .upload_media(bytes, file_name, LANSENGER_MEDIA_TYPE_IMAGE)
            .await?;
        self.send_text_with_media(recipient, caption, LANSENGER_MEDIA_TYPE_IMAGE, &[media_id])
            .await
    }

    /// Download an image from `image_url` then upload and send it.
    ///
    /// The URL must be reachable from the running KylinBot host. Rejects responses
    /// whose `Content-Type` is not `image/*` (or `application/octet-stream`).
    pub async fn send_image_url(
        &self,
        recipient: &str,
        image_url: &str,
        caption: &str,
    ) -> anyhow::Result<()> {
        let client = self.http_client();
        let resp = tokio::time::timeout(
            IMAGE_URL_DOWNLOAD_TIMEOUT,
            client.get(image_url).send(),
        )
        .await
        .map_err(|_| {
            anyhow::anyhow!("Lansenger: image URL unreachable (timeout after 15s): {image_url}")
        })??;

        let status = resp.status();
        if !status.is_success() {
            anyhow::bail!("Lansenger: image URL fetch failed ({status}): {image_url}");
        }

        let content_type = resp
            .headers()
            .get(reqwest::header::CONTENT_TYPE)
            .and_then(|value| value.to_str().ok())
            .map(ToString::to_string)
            .unwrap_or_default();
        let ct_lower = content_type.to_ascii_lowercase();
        if !ct_lower.starts_with("image/") && !ct_lower.contains("octet-stream") {
            anyhow::bail!(
                "Lansenger: image URL returned non-image content (content-type: {content_type}): {image_url}"
            );
        }

        let bytes = resp.bytes().await?.to_vec();
        let file_name = infer_media_file_name(
            &Uuid::new_v4().to_string(),
            "image",
            Some(&content_type),
            None,
        );
        self.send_image_bytes(recipient, bytes, &file_name, caption)
            .await
    }

    /// Convenience: upload bytes as a generic file and send it.
    pub async fn send_file_bytes(
        &self,
        recipient: &str,
        bytes: Vec<u8>,
        file_name: &str,
        caption: &str,
    ) -> anyhow::Result<()> {
        let media_type = lansenger_media_type_for_attachment(file_name, None);
        let media_id = self.upload_media(bytes, file_name, media_type).await?;
        self.send_text_with_media(recipient, caption, media_type, &[media_id])
            .await
    }

    /// Upload each attachment from a `SendMessage` and send them grouped by
    /// Lansenger media type, using `caption` (the original message text) on the
    /// first batch only.
    async fn send_attachments(
        &self,
        recipient: &str,
        caption: &str,
        attachments: &[MediaAttachment],
    ) -> anyhow::Result<()> {
        // Group by Lansenger media type so we can batch `mediaIds` per message.
        // Order is preserved: first attachment's media_type defines the first batch.
        let mut batches: Vec<(u8, Vec<String>)> = Vec::new();
        for attachment in attachments {
            let media_type = lansenger_media_type_for_attachment(
                &attachment.file_name,
                attachment.mime_type.as_deref(),
            );
            let media_id = self
                .upload_media(attachment.data.clone(), &attachment.file_name, media_type)
                .await?;
            match batches.last_mut() {
                Some((mt, ids)) if *mt == media_type => ids.push(media_id),
                _ => batches.push((media_type, vec![media_id])),
            }
        }

        for (idx, (media_type, media_ids)) in batches.iter().enumerate() {
            let caption_for_batch = if idx == 0 { caption } else { "" };
            self.send_text_with_media(recipient, caption_for_batch, *media_type, media_ids)
                .await?;
        }
        Ok(())
    }

    async fn resolve_outbound_attachment_markers(
        &self,
        content: &str,
    ) -> anyhow::Result<(String, Vec<MediaAttachment>)> {
        let (text_without_markers, markers) = crate::util::parse_attachment_markers(content);
        if markers.is_empty() {
            return Ok((content.to_string(), Vec::new()));
        }

        let mut attachments = Vec::with_capacity(markers.len());
        for (kind, target) in markers {
            let attachment = self
                .outbound_attachment_from_target(&kind, &target)
                .await
                .with_context(|| {
                    format!("Lansenger: failed to resolve outbound {kind} attachment: {target}")
                })?;
            attachments.push(attachment);
        }

        Ok((text_without_markers, attachments))
    }

    async fn outbound_attachment_from_target(
        &self,
        kind: &str,
        target: &str,
    ) -> anyhow::Result<MediaAttachment> {
        let target = target.strip_prefix("file://").unwrap_or(target).trim();
        if is_http_url(target) {
            return self.download_outbound_attachment(kind, target).await;
        }

        let path = Path::new(target);
        let data = tokio::fs::read(path)
            .await
            .with_context(|| format!("unable to read local attachment path {}", path.display()))?;
        let file_name = path
            .file_name()
            .and_then(|name| name.to_str())
            .filter(|name| !name.is_empty())
            .unwrap_or("attachment.dat")
            .to_string();

        Ok(MediaAttachment {
            mime_type: marker_fallback_mime_type(kind),
            file_name,
            data,
        })
    }

    async fn download_outbound_attachment(
        &self,
        kind: &str,
        url: &str,
    ) -> anyhow::Result<MediaAttachment> {
        let resp = tokio::time::timeout(
            IMAGE_URL_DOWNLOAD_TIMEOUT,
            self.http_client().get(url).send(),
        )
        .await
        .map_err(|_| {
            anyhow::anyhow!("Lansenger: attachment URL unreachable (timeout after 15s): {url}")
        })??;

        let status = resp.status();
        if !status.is_success() {
            anyhow::bail!("Lansenger: attachment URL fetch failed ({status}): {url}");
        }

        let content_type = resp
            .headers()
            .get(reqwest::header::CONTENT_TYPE)
            .and_then(|value| value.to_str().ok())
            .map(ToString::to_string);
        let content_disposition = resp
            .headers()
            .get(reqwest::header::CONTENT_DISPOSITION)
            .and_then(|value| value.to_str().ok())
            .map(ToString::to_string);

        if matches!(kind, "IMAGE" | "PHOTO")
            && let Some(content_type) = content_type.as_deref()
        {
            let lower = content_type.to_ascii_lowercase();
            if !lower.starts_with("image/") && !lower.contains("octet-stream") {
                anyhow::bail!(
                    "Lansenger: image attachment URL returned non-image content (content-type: {content_type}): {url}"
                );
            }
        }

        let file_name = content_disposition
            .as_deref()
            .and_then(extract_filename_from_disposition)
            .or_else(|| file_name_from_url(url))
            .unwrap_or_else(|| {
                infer_media_file_name(
                    &Uuid::new_v4().to_string(),
                    marker_media_kind(kind),
                    content_type.as_deref(),
                    None,
                )
            });
        let data = resp.bytes().await?.to_vec();

        Ok(MediaAttachment {
            mime_type: content_type.or_else(|| marker_fallback_mime_type(kind)),
            file_name,
            data,
        })
    }

    async fn listen_once(&self, tx: tokio::sync::mpsc::Sender<ChannelMessage>) -> anyhow::Result<()> {
        let ws_url = self.get_ws_endpoint().await?;
        tracing::info!("Lansenger: connecting to WebSocket endpoint");
        let (ws_stream, _) = kylinbot_config::schema::ws_connect_with_proxy(
            &ws_url,
            "channel.lansenger",
            self.proxy_url.as_deref(),
        )
        .await?;
        let (mut write, mut read) = ws_stream.split();
        let mut heartbeat = tokio::time::interval(HEARTBEAT_INTERVAL);
        heartbeat.set_missed_tick_behavior(tokio::time::MissedTickBehavior::Skip);
        let mut last_pong = Instant::now();

        loop {
            tokio::select! {
                _ = heartbeat.tick() => {
                    if last_pong.elapsed() > HEARTBEAT_INTERVAL + PONG_TIMEOUT {
                        anyhow::bail!("Lansenger WebSocket pong timeout");
                    }
                    write.send(Message::Ping(Vec::new().into())).await?;
                }
                frame = read.next() => {
                    match frame {
                        Some(Ok(Message::Text(text))) => {
                            let events = self.process_raw_message(&text).await;
                            for event in events {
                                if !self.should_dispatch(&event) {
                                    tracing::debug!(
                                        "Lansenger: dropping unauthorized or unmentioned message from {} to {}",
                                        event.sender_id,
                                        event.reply_target
                                    );
                                    continue;
                                }
                                let message = ChannelMessage {
                                    id: event.message_id,
                                    sender: event.sender_id,
                                    reply_target: event.reply_target,
                                    content: event.content,
                                    channel: "lansenger".to_string(),
                                    timestamp: event.timestamp,
                                    thread_ts: None,
                                    interruption_scope_id: None,
                                    attachments: event.attachments,
                                };
                                if tx.send(message).await.is_err() {
                                    anyhow::bail!("Lansenger message receiver closed");
                                }
                            }
                        }
                        Some(Ok(Message::Ping(data))) => {
                            write.send(Message::Pong(data)).await?;
                        }
                        Some(Ok(Message::Pong(_))) => {
                            last_pong = Instant::now();
                        }
                        Some(Ok(Message::Close(_))) => {
                            anyhow::bail!("Lansenger WebSocket closed");
                        }
                        Some(Ok(_)) => {}
                        Some(Err(err)) => {
                            anyhow::bail!("Lansenger WebSocket error: {err}");
                        }
                        None => {
                            anyhow::bail!("Lansenger WebSocket stream ended");
                        }
                    }
                }
            }
        }
    }
}

#[async_trait]
impl Channel for LansengerChannel {
    fn name(&self) -> &str {
        "lansenger"
    }

    async fn send(&self, message: &SendMessage) -> anyhow::Result<()> {
        let (content, mut attachments) = self
            .resolve_outbound_attachment_markers(&message.content)
            .await?;
        attachments.extend(message.attachments.iter().cloned());

        if !attachments.is_empty() {
            return self
                .send_attachments(&message.recipient, &content, &attachments)
                .await;
        }

        if let Err(err) = self
            .send_format_text(&message.recipient, &content)
            .await
        {
            tracing::info!("Lansenger: formatText failed ({err}); falling back to text");
            self.send_plain_text(&message.recipient, &content).await?;
        }
        Ok(())
    }

    async fn listen(&self, tx: tokio::sync::mpsc::Sender<ChannelMessage>) -> anyhow::Result<()> {
        let mut backoff_idx = 0usize;
        loop {
            match self.listen_once(tx.clone()).await {
                Ok(()) => return Ok(()),
                Err(err) => {
                    if tx.is_closed() {
                        return Err(err);
                    }
                    let delay = RECONNECT_BACKOFF
                        .get(backoff_idx)
                        .copied()
                        .unwrap_or_else(|| *RECONNECT_BACKOFF.last().unwrap_or(&60));
                    tracing::warn!("Lansenger: listener ended: {err}; reconnecting in {delay}s");
                    tokio::time::sleep(Duration::from_secs(delay)).await;
                    backoff_idx = (backoff_idx + 1).min(RECONNECT_BACKOFF.len().saturating_sub(1));
                }
            }
        }
    }

    async fn health_check(&self) -> bool {
        self.get_app_token().await.is_ok()
    }
}

fn normalize_api_gateway_url(url: &str) -> String {
    let trimmed = url.trim().trim_end_matches('/');
    if trimmed.is_empty() {
        DEFAULT_API_GATEWAY_URL.to_string()
    } else {
        trimmed.to_string()
    }
}

fn next_token_refresh_deadline(now: Instant, expires_in_secs: u64) -> Instant {
    let ttl = Duration::from_secs(expires_in_secs.max(1));
    let refresh_in = ttl
        .checked_sub(TOKEN_REFRESH_SKEW)
        .unwrap_or(Duration::from_secs(1));
    now + refresh_in
}

fn wrap_message_payload(
    chat_type: LansengerChatType,
    recipient: &str,
    msg_data: serde_json::Value,
) -> (&'static str, serde_json::Value) {
    let msg_type = msg_data
        .get("msgType")
        .and_then(|value| value.as_str())
        .unwrap_or("text")
        .to_string();

    let mut data = serde_json::Map::new();
    if let Some(object) = msg_data.as_object() {
        for (key, value) in object {
            if key != "msgType" {
                data.insert(key.clone(), value.clone());
            }
        }
    }

    match chat_type {
        LansengerChatType::Group => (
            GROUP_MESSAGE_ENDPOINT,
            serde_json::json!({
                "groupId": recipient,
                "msgType": msg_type,
                "msgData": serde_json::Value::Object(data),
            }),
        ),
        LansengerChatType::Direct => (
            PRIVATE_MESSAGE_ENDPOINT,
            serde_json::json!({
                "userIdList": [recipient],
                "msgType": msg_type,
                "msgData": serde_json::Value::Object(data),
            }),
        ),
    }
}

fn format_position(payload: &serde_json::Value) -> String {
    let position = payload.get("position").unwrap_or(&serde_json::Value::Null);
    let mut parts = Vec::new();
    if let Some(name) = position.get("name").and_then(|value| value.as_str()) {
        parts.push(name.to_string());
    }
    if let Some(address) = position.get("address").and_then(|value| value.as_str()) {
        parts.push(address.to_string());
    }
    if let (Some(latitude), Some(longitude)) = (
        position.get("latitude").and_then(|value| value.as_f64()),
        position.get("longitude").and_then(|value| value.as_f64()),
    ) {
        parts.push(format!("{latitude},{longitude}"));
    }
    if let Some(link) = position.get("link").and_then(|value| value.as_str()) {
        parts.push(link.to_string());
    }

    if parts.is_empty() {
        "[Location]".to_string()
    } else {
        format!("[Location] {}", parts.join(" "))
    }
}

fn extract_media_ids(msg_data: &serde_json::Value, media_kind: &str) -> Vec<String> {
    let empty = serde_json::Value::Null;
    let payload = msg_data.get("msgData").unwrap_or(&empty);
    let mut ids = Vec::new();

    for base in [payload, msg_data] {
        for field in ["mediaIds", "mediaId", "media_id", "mediaIdList"] {
            let nested_pointer = format!("/{media_kind}/{field}");
            collect_media_id_value(base.pointer(&nested_pointer), &mut ids);

            let flat_pointer = format!("/{field}");
            collect_media_id_value(base.pointer(&flat_pointer), &mut ids);
        }
    }

    ids
}

fn collect_media_id_value(value: Option<&serde_json::Value>, ids: &mut Vec<String>) {
    match value {
        Some(serde_json::Value::String(id)) => push_media_id(id, ids),
        Some(serde_json::Value::Number(id)) => push_media_id(&id.to_string(), ids),
        Some(serde_json::Value::Array(values)) => {
            for value in values {
                collect_media_id_value(Some(value), ids);
            }
        }
        _ => {}
    }
}

fn push_media_id(id: &str, ids: &mut Vec<String>) {
    let trimmed = id.trim();
    if !trimmed.is_empty() && !ids.iter().any(|existing| existing == trimmed) {
        ids.push(trimmed.to_string());
    }
}

fn inbound_attachment_file_name(
    message_id: &str,
    index: usize,
    file_name: &str,
    mime_type: Option<&str>,
) -> String {
    let safe_message_id = sanitize_path_component(message_id)
        .unwrap_or_else(|| Uuid::new_v4().simple().to_string());
    let mut safe_file_name = sanitize_path_component(file_name).unwrap_or_else(|| {
        format!(
            "image_{}.{}",
            Uuid::new_v4().simple(),
            image_extension_from_mime(mime_type)
        )
    });

    if Path::new(&safe_file_name).extension().is_none() {
        safe_file_name.push('.');
        safe_file_name.push_str(image_extension_from_mime(mime_type));
    }

    format!("{}_{}_{}", safe_message_id, index + 1, safe_file_name)
}

fn sanitize_path_component(value: &str) -> Option<String> {
    let base = value
        .rsplit(['/', '\\'])
        .next()
        .unwrap_or(value)
        .trim()
        .trim_matches('.');
    if base.is_empty() {
        return None;
    }

    let mut sanitized = String::new();
    for ch in base.chars().take(120) {
        if ch.is_ascii_alphanumeric() || matches!(ch, '.' | '-' | '_') {
            sanitized.push(ch);
        } else {
            sanitized.push('_');
        }
    }

    let sanitized = sanitized.trim_matches(['.', '_']).to_string();
    if sanitized.is_empty() {
        None
    } else {
        Some(sanitized)
    }
}

fn image_extension_from_mime(mime_type: Option<&str>) -> &'static str {
    match mime_type.unwrap_or("").to_ascii_lowercase().as_str() {
        mime if mime.contains("png") => "png",
        mime if mime.contains("gif") => "gif",
        mime if mime.contains("webp") => "webp",
        mime if mime.contains("bmp") => "bmp",
        mime if mime.contains("jpeg") || mime.contains("jpg") => "jpg",
        _ => "jpg",
    }
}

/// Classify an attachment into a Lansenger upload `type`:
/// `1` = video, `2` = image, `3` = file (default).
///
/// Mirrors openclaw-lansenger-channel's `mediaTypeFromPath`, with an extra
/// MIME-prefix check so callers that only know `Content-Type` also work.
fn lansenger_media_type_for_attachment(file_name: &str, mime_type: Option<&str>) -> u8 {
    if let Some(mime) = mime_type {
        let lower = mime.to_ascii_lowercase();
        if lower.starts_with("image/") {
            return LANSENGER_MEDIA_TYPE_IMAGE;
        }
        if lower.starts_with("video/") {
            return LANSENGER_MEDIA_TYPE_VIDEO;
        }
        if lower.starts_with("audio/") {
            return LANSENGER_MEDIA_TYPE_FILE;
        }
    }

    let ext = file_name
        .rsplit_once('.')
        .map(|(_, ext)| ext.to_ascii_lowercase())
        .unwrap_or_default();

    match ext.as_str() {
        "jpg" | "jpeg" | "png" | "webp" | "gif" | "bmp" | "heic" | "tiff" => {
            LANSENGER_MEDIA_TYPE_IMAGE
        }
        "mp4" | "mov" | "avi" | "mkv" | "webm" | "3gp" => LANSENGER_MEDIA_TYPE_VIDEO,
        _ => LANSENGER_MEDIA_TYPE_FILE,
    }
}

fn infer_media_file_name(
    media_id: &str,
    media_kind: &str,
    content_type: Option<&str>,
    content_disposition: Option<&str>,
) -> String {
    if let Some(file_name) = content_disposition.and_then(extract_filename_from_disposition)
        && !file_name.is_empty()
    {
        return file_name;
    }

    let ext = match content_type.unwrap_or("").to_ascii_lowercase().as_str() {
        content_type if content_type.contains("png") => "png",
        content_type if content_type.contains("gif") => "gif",
        content_type if content_type.contains("webp") => "webp",
        content_type if content_type.contains("jpeg") || content_type.contains("jpg") => "jpg",
        content_type if content_type.contains("mp4") => "mp4",
        content_type if content_type.contains("wav") => "wav",
        content_type if content_type.contains("pdf") => "pdf",
        _ => match media_kind {
            "image" => "jpg",
            "video" => "mp4",
            "voice" => "amr",
            _ => "dat",
        },
    };

    format!("lansenger_{media_kind}_{media_id}.{ext}")
}

fn extract_filename_from_disposition(disposition: &str) -> Option<String> {
    for part in disposition.split(';') {
        let part = part.trim();
        if let Some(value) = part.strip_prefix("filename=") {
            return Some(value.trim_matches('"').to_string());
        }
    }
    None
}

fn is_http_url(target: &str) -> bool {
    target.starts_with("http://") || target.starts_with("https://")
}

fn file_name_from_url(url: &str) -> Option<String> {
    let parsed = reqwest::Url::parse(url).ok()?;
    let segment = parsed
        .path_segments()?
        .filter(|segment| !segment.is_empty())
        .next_back()?;
    let decoded = urlencoding::decode(segment)
        .map(|value| value.into_owned())
        .unwrap_or_else(|_| segment.to_string());
    if decoded.is_empty() {
        None
    } else {
        Some(decoded)
    }
}

fn marker_fallback_mime_type(kind: &str) -> Option<String> {
    match kind {
        "IMAGE" | "PHOTO" => Some("image/jpeg".to_string()),
        "VIDEO" => Some("video/mp4".to_string()),
        "AUDIO" | "VOICE" => Some("audio/mpeg".to_string()),
        _ => None,
    }
}

fn marker_media_kind(kind: &str) -> &'static str {
    match kind {
        "IMAGE" | "PHOTO" => "image",
        "VIDEO" => "video",
        "AUDIO" | "VOICE" => "voice",
        _ => "file",
    }
}

fn current_unix_secs() -> u64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn normalize_api_gateway_url_uses_default_for_empty_value() {
        assert_eq!(normalize_api_gateway_url(""), DEFAULT_API_GATEWAY_URL);
        assert_eq!(
            normalize_api_gateway_url("https://example.test/open/apigw/"),
            "https://example.test/open/apigw"
        );
    }

    #[test]
    fn wrap_message_payload_uses_private_shape_for_direct_chats() {
        let (_, payload) = wrap_message_payload(
            LansengerChatType::Direct,
            "user-1",
            serde_json::json!({
                "formatText": {"formatType": 1, "text": "hello"},
                "msgType": "formatText"
            }),
        );

        assert_eq!(payload["userIdList"], serde_json::json!(["user-1"]));
        assert_eq!(payload["msgType"], "formatText");
        assert!(payload["msgData"].get("msgType").is_none());
    }

    #[test]
    fn wrap_message_payload_uses_group_shape_for_group_chats() {
        let (endpoint, payload) = wrap_message_payload(
            LansengerChatType::Group,
            "group-1",
            serde_json::json!({
                "text": {"content": "hello"},
                "msgType": "text"
            }),
        );

        assert_eq!(endpoint, GROUP_MESSAGE_ENDPOINT);
        assert_eq!(payload["groupId"], "group-1");
        assert_eq!(payload["msgData"]["text"]["content"], "hello");
    }

    #[tokio::test]
    async fn process_raw_message_extracts_text_event() {
        let channel = LansengerChannel::new(
            "app-1".into(),
            "secret".into(),
            DEFAULT_API_GATEWAY_URL.into(),
            vec!["user-1".into()],
            vec![],
            false,
        );
        let raw = serde_json::json!({
            "events": [{
                "data": {
                    "messageId": "msg-1",
                    "chatType": "p2p",
                    "from": "user-1",
                    "conversationId": "user-1",
                    "msgType": "text",
                    "msgData": {
                        "text": {"content": " hello "}
                    }
                }
            }]
        });

        let events = channel.process_raw_message(&raw.to_string()).await;

        assert_eq!(events.len(), 1);
        assert_eq!(events[0].message_id, "msg-1");
        assert_eq!(events[0].sender_id, "user-1");
        assert_eq!(events[0].reply_target, "user-1");
        assert_eq!(events[0].content, "hello");
        assert!(channel.should_dispatch(&events[0]));
    }

    #[tokio::test]
    async fn process_raw_message_applies_group_policy_and_mentions() {
        let channel = LansengerChannel::new(
            "bot-app".into(),
            "secret".into(),
            DEFAULT_API_GATEWAY_URL.into(),
            vec![],
            vec!["group-1".into()],
            true,
        );
        let raw = serde_json::json!({
            "events": [{
                "data": {
                    "messageId": "msg-1",
                    "chatType": "group",
                    "from": "user-1",
                    "conversationId": "group-1",
                    "msgType": "formatText",
                    "msgData": {
                        "formatText": {
                            "text": "hello",
                            "reminder": {"userIds": ["bot-app"]}
                        }
                    }
                }
            }]
        });

        let events = channel.process_raw_message(&raw.to_string()).await;

        assert_eq!(events.len(), 1);
        assert_eq!(events[0].chat_type, LansengerChatType::Group);
        assert!(events[0].mentions_bot);
        assert!(channel.should_dispatch(&events[0]));
    }

    #[test]
    fn media_type_for_attachment_classifies_images() {
        assert_eq!(
            lansenger_media_type_for_attachment("photo.png", None),
            LANSENGER_MEDIA_TYPE_IMAGE
        );
        assert_eq!(
            lansenger_media_type_for_attachment("photo.JPG", None),
            LANSENGER_MEDIA_TYPE_IMAGE
        );
        assert_eq!(
            lansenger_media_type_for_attachment("unknown.bin", Some("image/webp")),
            LANSENGER_MEDIA_TYPE_IMAGE
        );
    }

    #[test]
    fn media_type_for_attachment_classifies_videos_and_files() {
        assert_eq!(
            lansenger_media_type_for_attachment("clip.mp4", None),
            LANSENGER_MEDIA_TYPE_VIDEO
        );
        assert_eq!(
            lansenger_media_type_for_attachment("clip", Some("video/mp4")),
            LANSENGER_MEDIA_TYPE_VIDEO
        );
        assert_eq!(
            lansenger_media_type_for_attachment("report.pdf", None),
            LANSENGER_MEDIA_TYPE_FILE
        );
        assert_eq!(
            lansenger_media_type_for_attachment("voice.webm", Some("audio/webm")),
            LANSENGER_MEDIA_TYPE_FILE
        );
        assert_eq!(
            lansenger_media_type_for_attachment("noext", None),
            LANSENGER_MEDIA_TYPE_FILE
        );
    }

    #[test]
    fn extract_media_ids_accepts_common_lansenger_shapes() {
        let raw = serde_json::json!({
            "mediaId": "top-level",
            "msgData": {
                "image": {
                    "mediaIds": ["media-1", ""],
                    "mediaId": "media-2",
                    "media_id": "media-3",
                    "mediaIdList": [42]
                }
            }
        });

        assert_eq!(
            extract_media_ids(&raw, "image"),
            vec!["media-1", "media-2", "media-3", "42", "top-level"]
        );
    }

    #[tokio::test]
    async fn content_with_image_markers_saves_image_to_workspace() {
        let temp = tempfile::tempdir().unwrap();
        let channel = LansengerChannel::new(
            "app-1".into(),
            "secret".into(),
            DEFAULT_API_GATEWAY_URL.into(),
            vec!["user-1".into()],
            vec![],
            false,
        )
        .with_workspace_dir(temp.path().to_path_buf());
        let attachment = MediaAttachment {
            file_name: "../photo name.png".to_string(),
            data: b"png-bytes".to_vec(),
            mime_type: Some("image/png".to_string()),
        };

        let content = channel
            .content_with_image_markers("msg/1", "[Image]".to_string(), &[attachment])
            .await;

        let path = content
            .strip_prefix("[IMAGE:")
            .and_then(|value| value.strip_suffix(']'))
            .expect("expected an image marker");
        let path = std::path::PathBuf::from(path);
        assert!(path.to_string_lossy().contains("lansenger_files"));
        assert_eq!(std::fs::read(path).unwrap(), b"png-bytes");
    }

    #[tokio::test]
    async fn resolve_outbound_attachment_markers_reads_local_files() {
        let temp = tempfile::tempdir().unwrap();
        let report_path = temp.path().join("report.pdf");
        std::fs::write(&report_path, b"pdf-bytes").unwrap();
        let channel = LansengerChannel::new(
            "app-1".into(),
            "secret".into(),
            DEFAULT_API_GATEWAY_URL.into(),
            vec!["user-1".into()],
            vec![],
            false,
        );

        let content = format!("ready\n[DOCUMENT:{}]", report_path.display());
        let (cleaned, attachments) = channel
            .resolve_outbound_attachment_markers(&content)
            .await
            .unwrap();

        assert_eq!(cleaned, "ready");
        assert_eq!(attachments.len(), 1);
        assert_eq!(attachments[0].file_name, "report.pdf");
        assert_eq!(attachments[0].data, b"pdf-bytes");
        assert_eq!(attachments[0].mime_type, None);
    }

    #[tokio::test]
    async fn resolve_outbound_attachment_markers_uses_marker_kind_for_media_type() {
        let temp = tempfile::tempdir().unwrap();
        let image_path = temp.path().join("generated-file");
        std::fs::write(&image_path, b"image-bytes").unwrap();
        let channel = LansengerChannel::new(
            "app-1".into(),
            "secret".into(),
            DEFAULT_API_GATEWAY_URL.into(),
            vec!["user-1".into()],
            vec![],
            false,
        );

        let content = format!("[IMAGE:{}]", image_path.display());
        let (_, attachments) = channel
            .resolve_outbound_attachment_markers(&content)
            .await
            .unwrap();

        assert_eq!(attachments.len(), 1);
        assert_eq!(attachments[0].file_name, "generated-file");
        assert_eq!(attachments[0].mime_type.as_deref(), Some("image/jpeg"));
        assert_eq!(
            lansenger_media_type_for_attachment(
                &attachments[0].file_name,
                attachments[0].mime_type.as_deref(),
            ),
            LANSENGER_MEDIA_TYPE_IMAGE
        );
    }

    #[test]
    fn wrap_message_payload_carries_media_ids_for_text_media() {
        let (endpoint, payload) = wrap_message_payload(
            LansengerChatType::Direct,
            "user-1",
            serde_json::json!({
                "text": {
                    "content": "see attached",
                    "mediaType": LANSENGER_MEDIA_TYPE_IMAGE,
                    "mediaIds": ["media-abc"],
                },
                "msgType": "text",
            }),
        );

        assert_eq!(endpoint, PRIVATE_MESSAGE_ENDPOINT);
        assert_eq!(payload["msgType"], "text");
        assert_eq!(payload["userIdList"], serde_json::json!(["user-1"]));
        assert_eq!(payload["msgData"]["text"]["content"], "see attached");
        assert_eq!(
            payload["msgData"]["text"]["mediaType"],
            LANSENGER_MEDIA_TYPE_IMAGE
        );
        assert_eq!(
            payload["msgData"]["text"]["mediaIds"],
            serde_json::json!(["media-abc"])
        );
        // top-level msgType should not leak into msgData
        assert!(payload["msgData"].get("msgType").is_none());
    }

    #[test]
    fn wrap_message_payload_group_text_media_uses_group_endpoint() {
        let (endpoint, payload) = wrap_message_payload(
            LansengerChatType::Group,
            "group-1",
            serde_json::json!({
                "text": {
                    "content": "",
                    "mediaType": LANSENGER_MEDIA_TYPE_FILE,
                    "mediaIds": ["m1", "m2"],
                },
                "msgType": "text",
            }),
        );

        assert_eq!(endpoint, GROUP_MESSAGE_ENDPOINT);
        assert_eq!(payload["groupId"], "group-1");
        assert_eq!(
            payload["msgData"]["text"]["mediaIds"],
            serde_json::json!(["m1", "m2"])
        );
    }
}
