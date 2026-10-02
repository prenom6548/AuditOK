use async_trait::async_trait;
use futures_util::{SinkExt, StreamExt};
use kylinbot_api::channel::{Channel, ChannelMessage, SendMessage};
use std::collections::HashMap;
use std::collections::hash_map::Entry;
use std::sync::Arc;
use tokio::sync::RwLock;
use tokio::sync::oneshot;
use tokio_tungstenite::tungstenite::Message;
use tokio_tungstenite::tungstenite::error::{
    Error as TungsteniteError, ProtocolError as TungsteniteProtocolError,
};

const WECOM_WS_URL: &str = "wss://openws.work.weixin.qq.com";
const HEARTBEAT_INTERVAL_SECS: u64 = 30;
const MAX_MISSED_PONG: u32 = 2;
const PENDING_REQ_ID_TTL_SECS: u64 = 300;
const SEND_ACK_TIMEOUT_SECS: u64 = 15;

#[derive(Clone)]
struct PendingReplyTarget {
    req_id: String,
    created_at: std::time::Instant,
}

enum WeComMode {
    Webhook {
        key: String,
    },
    Bot {
        bot_id: String,
        secret: String,
        ws_url: String,
    },
}

pub struct WeComChannel {
    mode: WeComMode,
    allowed_users: Vec<String>,
    pending_req_ids: Arc<RwLock<HashMap<String, PendingReplyTarget>>>,
    pending_send_acks: Arc<RwLock<HashMap<String, oneshot::Sender<anyhow::Result<()>>>>>,
    outgoing_tx: tokio::sync::mpsc::Sender<String>,
    outgoing_rx: tokio::sync::Mutex<tokio::sync::mpsc::Receiver<String>>,
}

impl WeComChannel {
    pub fn new_webhook(key: String, allowed_users: Vec<String>) -> Self {
        let (outgoing_tx, outgoing_rx) = tokio::sync::mpsc::channel(64);
        Self {
            mode: WeComMode::Webhook { key },
            allowed_users,
            pending_req_ids: Arc::new(RwLock::new(HashMap::new())),
            pending_send_acks: Arc::new(RwLock::new(HashMap::new())),
            outgoing_tx,
            outgoing_rx: tokio::sync::Mutex::new(outgoing_rx),
        }
    }

    pub fn new_bot(
        bot_id: String,
        secret: String,
        allowed_users: Vec<String>,
        ws_url: Option<String>,
    ) -> Self {
        let (outgoing_tx, outgoing_rx) = tokio::sync::mpsc::channel(64);
        Self {
            mode: WeComMode::Bot {
                bot_id,
                secret,
                ws_url: ws_url
                    .filter(|u| !u.is_empty())
                    .unwrap_or_else(|| WECOM_WS_URL.to_string()),
            },
            allowed_users,
            pending_req_ids: Arc::new(RwLock::new(HashMap::new())),
            pending_send_acks: Arc::new(RwLock::new(HashMap::new())),
            outgoing_tx,
            outgoing_rx: tokio::sync::Mutex::new(outgoing_rx),
        }
    }

    fn is_user_allowed(&self, user_id: &str) -> bool {
        self.allowed_users.iter().any(|u| u == "*" || u == user_id)
    }

    fn generate_req_id(prefix: &str) -> String {
        let ts = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap_or_default()
            .as_millis();
        let rand_hex = format!("{:08x}", rand_simple());
        format!("{prefix}_{ts}_{rand_hex}")
    }

    fn build_subscribe_frame(bot_id: &str, secret: &str) -> serde_json::Value {
        let req_id = Self::generate_req_id("aibot_subscribe");
        serde_json::json!({
            "cmd": "aibot_subscribe",
            "headers": { "req_id": req_id },
            "body": {
                "bot_id": bot_id,
                "secret": secret,
            }
        })
    }

    fn build_heartbeat_frame() -> serde_json::Value {
        let req_id = Self::generate_req_id("ping");
        serde_json::json!({
            "cmd": "ping",
            "headers": { "req_id": req_id }
        })
    }

    fn build_respond_msg_frame(req_id: &str, content: &str, finish: bool) -> serde_json::Value {
        let stream_id = Self::generate_req_id("stream");
        serde_json::json!({
            "cmd": "aibot_respond_msg",
            "headers": { "req_id": req_id },
            "body": {
                "msgtype": "stream",
                "stream": {
                    "id": stream_id,
                    "finish": finish,
                    "content": content,
                }
            }
        })
    }

    fn build_send_msg_frame(chat_id: &str, content: &str) -> (String, serde_json::Value) {
        let req_id = Self::generate_req_id("aibot_send_msg");
        (
            req_id.clone(),
            serde_json::json!({
                "cmd": "aibot_send_msg",
                "headers": { "req_id": req_id },
                "body": {
                    "chatid": chat_id,
                    "msgtype": "markdown",
                    "markdown": { "content": content }
                }
            }),
        )
    }

    fn send_webhook(&self, content: &str) -> anyhow::Result<()> {
        let key = match &self.mode {
            WeComMode::Webhook { key } => key,
            _ => return Err(anyhow::anyhow!("not in webhook mode")),
        };

        let url = format!("https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key={key}");
        let body = serde_json::json!({
            "msgtype": "markdown",
            "markdown": { "content": content }
        });

        let client = reqwest::blocking::Client::new();
        let resp = client.post(&url).json(&body).send()?;

        if !resp.status().is_success() {
            return Err(anyhow::anyhow!("WeCom webhook failed: {}", resp.status()));
        }

        let data: serde_json::Value = resp.json()?;
        let errcode = data.get("errcode").and_then(|e| e.as_i64()).unwrap_or(-1);
        if errcode != 0 {
            let errmsg = data
                .get("errmsg")
                .and_then(|e| e.as_str())
                .unwrap_or("unknown");
            return Err(anyhow::anyhow!("WeCom webhook errcode={errcode}: {errmsg}"));
        }

        Ok(())
    }

    fn pending_reply_target(req_id: &str) -> PendingReplyTarget {
        PendingReplyTarget {
            req_id: req_id.to_string(),
            created_at: std::time::Instant::now(),
        }
    }

    fn pending_req_id_is_fresh(entry: &PendingReplyTarget) -> bool {
        entry.created_at.elapsed().as_secs() < PENDING_REQ_ID_TTL_SECS
    }

    async fn connect_and_auth(
        ws_url: &str,
        bot_id: &str,
        secret: &str,
    ) -> anyhow::Result<(
        futures_util::stream::SplitSink<
            tokio_tungstenite::WebSocketStream<
                tokio_tungstenite::MaybeTlsStream<tokio::net::TcpStream>,
            >,
            Message,
        >,
        futures_util::stream::SplitStream<
            tokio_tungstenite::WebSocketStream<
                tokio_tungstenite::MaybeTlsStream<tokio::net::TcpStream>,
            >,
        >,
    )> {
        let (ws_stream, _) = tokio_tungstenite::connect_async(ws_url).await?;
        let (mut write, mut read) = ws_stream.split();

        let subscribe_frame = Self::build_subscribe_frame(bot_id, secret);
        write
            .send(Message::Text(subscribe_frame.to_string().into()))
            .await?;

        tokio::time::timeout(std::time::Duration::from_secs(15), async {
            while let Some(msg) = read.next().await {
                match msg {
                    Ok(Message::Text(t)) => {
                        if let Ok(v) = serde_json::from_str::<serde_json::Value>(&t) {
                            let errcode = v.get("errcode").and_then(|e| e.as_i64()).unwrap_or(-1);
                            if errcode == 0 {
                                return Ok(());
                            } else {
                                let errmsg = v
                                    .get("errmsg")
                                    .and_then(|e| e.as_str())
                                    .unwrap_or("unknown");
                                return Err(anyhow::anyhow!(
                                    "WeCom auth failed (errcode={errcode}): {errmsg}"
                                ));
                            }
                        }
                    }
                    Ok(Message::Close(_)) => {
                        return Err(anyhow::anyhow!("WeCom: connection closed during auth"));
                    }
                    Err(e) => {
                        return Err(anyhow::anyhow!("WeCom: WebSocket error during auth: {e}"));
                    }
                    _ => continue,
                }
            }
            Err(anyhow::anyhow!("WeCom: WebSocket stream ended during auth"))
        })
        .await
        .map_err(|_| anyhow::anyhow!("WeCom: auth timeout"))??;

        tracing::info!("WeCom: authenticated successfully");
        Ok((write, read))
    }

    async fn run_listen_loop(
        &self,
        tx: &tokio::sync::mpsc::Sender<ChannelMessage>,
        outgoing_rx: &mut tokio::sync::mpsc::Receiver<String>,
    ) -> anyhow::Result<()> {
        let (bot_id, secret, ws_url) = match &self.mode {
            WeComMode::Bot {
                bot_id,
                secret,
                ws_url,
            } => (bot_id.clone(), secret.clone(), ws_url.clone()),
            _ => return Err(anyhow::anyhow!("not in bot mode")),
        };

        let (mut write, mut read) = Self::connect_and_auth(&ws_url, &bot_id, &secret).await?;

        tracing::info!("WeCom: listening for messages...");

        let mut missed_pong: u32 = 0;
        let mut heartbeat_interval =
            tokio::time::interval(std::time::Duration::from_secs(HEARTBEAT_INTERVAL_SECS));

        loop {
            tokio::select! {
                msg = read.next() => {
                    match msg {
                        Some(Ok(Message::Text(t))) => {
                            let frame: serde_json::Value = match serde_json::from_str(&t) {
                                Ok(v) => v,
                                Err(_) => continue,
                            };

                            if self.try_complete_send_ack(&frame).await {
                                continue;
                            }

                            let errcode = frame.get("errcode").and_then(|e| e.as_i64());
                            if let Some(code) = errcode
                                && code == 0
                            {
                                missed_pong = 0;
                                continue;
                            }

                            let cmd = frame.get("cmd").and_then(|c| c.as_str()).unwrap_or("");

                            match cmd {
                                "aibot_msg_callback" => {
                                    self.handle_msg_callback(&frame, tx, &mut write).await?;
                                }
                                "aibot_event_callback" => {
                                    self.handle_event_callback(&frame, &mut write).await?;
                                }
                                _ => {
                                    tracing::debug!("WeCom: unhandled cmd: {cmd}");
                                }
                            }
                        }
                        Some(Ok(Message::Ping(data))) => {
                            if let Err(e) = write.send(Message::Pong(data)).await {
                                tracing::warn!("WeCom: failed to send pong: {e}");
                                break;
                            }
                        }
                        Some(Ok(Message::Close(_))) => {
                            tracing::info!("WeCom: server closed connection");
                            self.fail_pending_send_acks("WeCom: server closed connection").await;
                            break;
                        }
                        Some(Err(e)) if is_recoverable_wecom_ws_disconnect(&e) => {
                            tracing::debug!("WeCom: WebSocket disconnected: {e}");
                            self.fail_pending_send_acks(&format!("WeCom: WebSocket disconnected: {e}")).await;
                            break;
                        }
                        Some(Err(e)) => {
                            tracing::warn!("WeCom: WebSocket error: {e}");
                            self.fail_pending_send_acks(&format!("WeCom: WebSocket error: {e}")).await;
                            break;
                        }
                        None => {
                            self.fail_pending_send_acks("WeCom: WebSocket stream ended").await;
                            break;
                        }
                        _ => {}
                    }
                }
                outgoing = outgoing_rx.recv() => {
                    match outgoing {
                        Some(frame_str) => {
                            if let Err(e) = write.send(Message::Text(frame_str.into())).await {
                                tracing::warn!("WeCom: failed to send outgoing frame: {e}");
                                self.fail_pending_send_acks(&format!("WeCom: failed to send outgoing frame: {e}")).await;
                                break;
                            }
                        }
                        None => {
                            self.fail_pending_send_acks("WeCom: outgoing queue closed").await;
                            break;
                        }
                    }
                }
                _ = heartbeat_interval.tick() => {
                    if missed_pong >= MAX_MISSED_PONG {
                        tracing::warn!("WeCom: too many missed heartbeats, reconnecting");
                        self.fail_pending_send_acks("WeCom: too many missed heartbeats").await;
                        break;
                    }
                    missed_pong += 1;
                    let ping = Self::build_heartbeat_frame();
                    if let Err(e) = write.send(Message::Text(ping.to_string().into())).await {
                        tracing::warn!("WeCom: failed to send heartbeat: {e}");
                        self.fail_pending_send_acks(&format!("WeCom: failed to send heartbeat: {e}")).await;
                        break;
                    }
                }
            }
        }

        Ok(())
    }

    async fn handle_msg_callback(
        &self,
        frame: &serde_json::Value,
        tx: &tokio::sync::mpsc::Sender<ChannelMessage>,
        write: &mut futures_util::stream::SplitSink<
            tokio_tungstenite::WebSocketStream<
                tokio_tungstenite::MaybeTlsStream<tokio::net::TcpStream>,
            >,
            Message,
        >,
    ) -> anyhow::Result<()> {
        let body = match frame.get("body") {
            Some(b) => b,
            None => return Ok(()),
        };

        let msg_type = body.get("msgtype").and_then(|m| m.as_str()).unwrap_or("");

        tracing::debug!("WeCom: received msg_type={msg_type}, body={body}");

        if msg_type == "event" {
            return self.handle_event_callback(frame, write).await;
        }

        let content = match msg_type {
            "text" => body
                .get("text")
                .and_then(|t| t.get("content"))
                .and_then(|c| c.as_str())
                .unwrap_or("")
                .trim()
                .to_string(),
            "mixed" => {
                if let Some(items) = body
                    .get("mixed")
                    .and_then(|m| m.get("msg_item").or_else(|| m.get("items")))
                    .and_then(|items| items.as_array())
                {
                    let mut parts = vec![];
                    for item in items {
                        let item_type = item.get("msgtype").and_then(|m| m.as_str()).unwrap_or("");
                        if let Some(text) = item
                            .get("text")
                            .and_then(|t| t.get("content"))
                            .and_then(|c| c.as_str())
                        {
                            parts.push(text.to_string());
                        } else if item_type == "voice" {
                            let voice_text = item
                                .get("voice")
                                .and_then(|v| v.get("content"))
                                .and_then(|c| c.as_str())
                                .unwrap_or("")
                                .trim()
                                .to_string();
                            if !voice_text.is_empty() {
                                parts.push(voice_text);
                            }
                        } else if let Some(marker) = extract_media_marker_from_item(item).await {
                            parts.push(marker);
                        }
                    }
                    parts.join("\n")
                } else {
                    String::new()
                }
            }
            "image" | "file" | "video" => {
                match extract_media_marker_from_body(body, msg_type).await {
                    Some(marker) => marker,
                    None => match msg_type {
                        "image" => "[图片]".to_string(),
                        "file" => {
                            let name = body
                                .get("file")
                                .and_then(|f| f.get("filename"))
                                .and_then(|n| n.as_str())
                                .unwrap_or("文件");
                            format!("[文件: {name}]")
                        }
                        "video" => "[视频]".to_string(),
                        _ => String::new(),
                    },
                }
            }
            "voice" => {
                let voice_text = body
                    .get("voice")
                    .and_then(|v| v.get("content"))
                    .and_then(|c| c.as_str())
                    .unwrap_or("")
                    .trim()
                    .to_string();
                if voice_text.is_empty() {
                    "[语音]".to_string()
                } else {
                    voice_text
                }
            }
            _ => return Ok(()),
        };

        if content.is_empty() {
            return Ok(());
        }

        let sender_id = body
            .get("from")
            .and_then(|f| f.get("userid"))
            .and_then(|u| u.as_str())
            .unwrap_or("unknown");

        if !self.is_user_allowed(sender_id) {
            tracing::warn!("WeCom: ignoring message from unauthorized user: {sender_id}");
            return Ok(());
        }

        let chat_type = body
            .get("chattype")
            .and_then(|c| c.as_str())
            .unwrap_or("single");

        let reply_target = if chat_type == "group" {
            body.get("chatid")
                .and_then(|c| c.as_str())
                .unwrap_or(sender_id)
                .to_string()
        } else {
            sender_id.to_string()
        };

        let msg_id = body.get("msgid").and_then(|m| m.as_str()).unwrap_or("");

        let req_id = frame
            .get("headers")
            .and_then(|h| h.get("req_id"))
            .and_then(|r| r.as_str())
            .unwrap_or("");

        {
            let mut pending = self.pending_req_ids.write().await;
            let entry = Self::pending_reply_target(req_id);
            pending.insert(reply_target.clone(), entry.clone());
            pending.insert(sender_id.to_string(), entry);
        }

        let channel_msg = ChannelMessage {
            id: msg_id.to_string(),
            sender: sender_id.to_string(),
            reply_target: reply_target.clone(),
            content,
            channel: "wecom".to_string(),
            timestamp: body
                .get("create_time")
                .and_then(|t| t.as_u64())
                .unwrap_or(0),
            thread_ts: None,
            interruption_scope_id: None,
            attachments: vec![],
        };

        if tx.send(channel_msg).await.is_err() {
            tracing::warn!("WeCom: channel message receiver closed");
        }

        Ok(())
    }

    async fn handle_event_callback(
        &self,
        frame: &serde_json::Value,
        write: &mut futures_util::stream::SplitSink<
            tokio_tungstenite::WebSocketStream<
                tokio_tungstenite::MaybeTlsStream<tokio::net::TcpStream>,
            >,
            Message,
        >,
    ) -> anyhow::Result<()> {
        let body = match frame.get("body") {
            Some(b) => b,
            None => return Ok(()),
        };

        let event_type = body
            .get("event")
            .and_then(|e| e.get("eventtype"))
            .and_then(|t| t.as_str())
            .unwrap_or("");

        match event_type {
            "enter_chat" => {
                let req_id = frame
                    .get("headers")
                    .and_then(|h| h.get("req_id"))
                    .and_then(|r| r.as_str())
                    .unwrap_or("");

                if !req_id.is_empty() {
                    let welcome = serde_json::json!({
                        "cmd": "aibot_respond_welcome_msg",
                        "headers": { "req_id": req_id },
                        "body": {
                            "msgtype": "text",
                            "text": { "content": "你好！我是麒灵，有什么可以帮你的？" }
                        }
                    });
                    if let Err(e) = write.send(Message::Text(welcome.to_string().into())).await {
                        tracing::warn!("WeCom: failed to send welcome message: {e}");
                    }
                }
            }
            "disconnected_event" => {
                tracing::info!("WeCom: received disconnected_event, connection replaced");
            }
            _ => {
                tracing::debug!("WeCom: unhandled event type: {event_type}");
            }
        }

        Ok(())
    }

    fn frame_req_id(frame: &serde_json::Value) -> Option<&str> {
        frame
            .get("headers")
            .and_then(|headers| headers.get("req_id"))
            .and_then(|req_id| req_id.as_str())
    }

    fn frame_errmsg(frame: &serde_json::Value) -> &str {
        frame
            .get("errmsg")
            .and_then(|errmsg| errmsg.as_str())
            .unwrap_or("unknown")
    }

    async fn try_complete_send_ack(&self, frame: &serde_json::Value) -> bool {
        let Some(req_id) = Self::frame_req_id(frame) else {
            return false;
        };

        let tx = {
            let mut pending = self.pending_send_acks.write().await;
            pending.remove(req_id)
        };

        let Some(tx) = tx else {
            return false;
        };

        let result = match frame.get("errcode").and_then(|errcode| errcode.as_i64()) {
            Some(0) => Ok(()),
            Some(code) => Err(anyhow::anyhow!(
                "WeCom send failed (errcode={code}): {}",
                Self::frame_errmsg(frame)
            )),
            None => Ok(()),
        };
        let _ = tx.send(result);
        true
    }

    async fn fail_pending_send_acks(&self, reason: &str) {
        let pending = {
            let mut pending = self.pending_send_acks.write().await;
            std::mem::take(&mut *pending)
        };

        for (_, tx) in pending {
            let _ = tx.send(Err(anyhow::anyhow!(reason.to_string())));
        }
    }
}

fn rand_simple() -> u32 {
    use std::time::SystemTime;
    let t = SystemTime::now()
        .duration_since(SystemTime::UNIX_EPOCH)
        .unwrap_or_default();
    (t.as_nanos() as u32)
        .wrapping_mul(1664525)
        .wrapping_add(1013904223)
}

fn is_recoverable_wecom_ws_disconnect(error: &TungsteniteError) -> bool {
    match error {
        TungsteniteError::ConnectionClosed
        | TungsteniteError::Protocol(TungsteniteProtocolError::ResetWithoutClosingHandshake) => {
            true
        }
        TungsteniteError::Io(error) => matches!(
            error.kind(),
            std::io::ErrorKind::ConnectionReset
                | std::io::ErrorKind::ConnectionAborted
                | std::io::ErrorKind::BrokenPipe
                | std::io::ErrorKind::UnexpectedEof
                | std::io::ErrorKind::TimedOut
        ),
        _ => false,
    }
}

fn decrypt_wecom_media(encrypted: &[u8], aes_key_b64: &str) -> anyhow::Result<Vec<u8>> {
    use aes::cipher::{BlockDecryptMut, KeyIvInit};
    use base64::Engine;

    type Aes256CbcDec = cbc::Decryptor<aes::Aes256>;

    let padded_key = if aes_key_b64.ends_with('=') {
        aes_key_b64.to_string()
    } else {
        format!("{aes_key_b64}=")
    };
    let key = base64::engine::general_purpose::STANDARD.decode(&padded_key)?;
    if key.len() < 16 {
        return Err(anyhow::anyhow!("WeCom: AES key too short"));
    }
    let iv = &key[..16];

    let decryptor = Aes256CbcDec::new_from_slices(&key, iv)?;
    let mut buffer = encrypted.to_vec();
    let decrypted_padded = decryptor
        .decrypt_padded_mut::<aes::cipher::block_padding::NoPadding>(&mut buffer)
        .map_err(|e| anyhow::anyhow!("WeCom: AES decrypt failed: {e}"))?;

    let decrypted = pkcs7_unpad(decrypted_padded, 32)?;

    Ok(decrypted.to_vec())
}

fn pkcs7_unpad(data: &[u8], block_size: usize) -> anyhow::Result<&[u8]> {
    if data.is_empty() {
        return Err(anyhow::anyhow!("WeCom: empty data for pkcs7 unpad"));
    }
    let pad = data[data.len() - 1] as usize;
    if pad < 1 || pad > block_size {
        return Err(anyhow::anyhow!("WeCom: invalid pkcs7 padding value: {pad}"));
    }
    if pad > data.len() {
        return Err(anyhow::anyhow!("WeCom: invalid pkcs7 padding length"));
    }
    for i in 0..pad {
        if data[data.len() - 1 - i] as usize != pad {
            return Err(anyhow::anyhow!("WeCom: invalid pkcs7 padding byte"));
        }
    }
    Ok(&data[..data.len() - pad])
}

async fn download_and_decrypt_media(url: &str, aes_key: &str) -> anyhow::Result<Vec<u8>> {
    let resp = reqwest::get(url).await?;
    if !resp.status().is_success() {
        return Err(anyhow::anyhow!(
            "WeCom: media download failed: {}",
            resp.status()
        ));
    }
    let encrypted = resp.bytes().await?;
    decrypt_wecom_media(&encrypted, aes_key)
}

async fn extract_media_marker_from_body(
    body: &serde_json::Value,
    msg_type: &str,
) -> Option<String> {
    let media_obj = body.get(msg_type)?;
    let url = media_obj.get("url").and_then(|u| u.as_str())?;
    let aes_key = media_obj
        .get("aeskey")
        .or_else(|| media_obj.get("aes_key"))
        .and_then(|k| k.as_str())?;

    let data = match download_and_decrypt_media(url, aes_key).await {
        Ok(d) => d,
        Err(e) => {
            tracing::warn!("WeCom: failed to download/decrypt {msg_type}: {e}");
            return None;
        }
    };

    let (file_name, marker_prefix) = match msg_type {
        "image" => {
            let fmt = media_obj
                .get("format")
                .and_then(|f| f.as_str())
                .unwrap_or("jpg");
            (format!("image.{fmt}"), "IMAGE")
        }
        "file" => {
            let name = media_obj
                .get("filename")
                .and_then(|n| n.as_str())
                .unwrap_or("file.dat");
            (name.to_string(), "DOCUMENT")
        }
        "video" => {
            let fmt = media_obj
                .get("format")
                .and_then(|f| f.as_str())
                .unwrap_or("mp4");
            (format!("video.{fmt}"), "VIDEO")
        }
        _ => ("unknown".to_string(), "DOCUMENT"),
    };

    save_media_to_workspace(&file_name, &data)
        .await
        .map(|path| format!("[{marker_prefix}:{}]", path.display()))
}

async fn extract_media_marker_from_item(item: &serde_json::Value) -> Option<String> {
    let item_type = item.get("msgtype").and_then(|m| m.as_str()).unwrap_or("");

    let (media_obj, file_name, marker_prefix) = match item_type {
        "image" => {
            let image = item.get("image")?;
            let fmt = image
                .get("format")
                .and_then(|f| f.as_str())
                .unwrap_or("jpg");
            (image, format!("image.{fmt}"), "IMAGE")
        }
        "file" => {
            let file = item.get("file")?;
            let name = file
                .get("filename")
                .and_then(|n| n.as_str())
                .unwrap_or("file.dat");
            (file, name.to_string(), "DOCUMENT")
        }
        "video" => {
            let video = item.get("video")?;
            let fmt = video
                .get("format")
                .and_then(|f| f.as_str())
                .unwrap_or("mp4");
            (video, format!("video.{fmt}"), "VIDEO")
        }
        _ => return None,
    };

    let url = media_obj.get("url").and_then(|u| u.as_str())?;
    let aes_key = media_obj
        .get("aeskey")
        .or_else(|| media_obj.get("aes_key"))
        .and_then(|k| k.as_str())?;

    let data = match download_and_decrypt_media(url, aes_key).await {
        Ok(d) => d,
        Err(e) => {
            tracing::warn!("WeCom: failed to download/decrypt {item_type}: {e}");
            return None;
        }
    };

    save_media_to_workspace(&file_name, &data)
        .await
        .map(|path| format!("[{marker_prefix}:{}]", path.display()))
}

async fn save_media_to_workspace(file_name: &str, data: &[u8]) -> Option<std::path::PathBuf> {
    let home = directories::UserDirs::new()?.home_dir().to_path_buf();
    let save_dir = home.join(".zeroclaw").join("workspace").join("wecom_files");
    if let Err(e) = tokio::fs::create_dir_all(&save_dir).await {
        tracing::warn!("WeCom: failed to create wecom_files directory: {e}");
        return None;
    }

    let ts = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap_or_default()
        .as_millis();
    let unique_name = format!("{ts}_{file_name}");
    let file_path = save_dir.join(&unique_name);

    if let Err(e) = tokio::fs::write(&file_path, data).await {
        tracing::warn!(
            "WeCom: failed to save media to {}: {e}",
            file_path.display()
        );
        return None;
    }

    tracing::debug!("WeCom: saved media to {}", file_path.display());
    Some(file_path)
}

#[async_trait]
impl Channel for WeComChannel {
    fn name(&self) -> &str {
        "wecom"
    }

    async fn send(&self, message: &SendMessage) -> anyhow::Result<()> {
        match &self.mode {
            WeComMode::Webhook { .. } => self.send_webhook(&message.content),
            WeComMode::Bot { .. } => {
                let reply_req_id = {
                    let mut pending = self.pending_req_ids.write().await;
                    match pending.entry(message.recipient.clone()) {
                        Entry::Occupied(entry) if Self::pending_req_id_is_fresh(entry.get()) => {
                            Some(entry.get().req_id.clone())
                        }
                        Entry::Occupied(entry) => {
                            entry.remove();
                            None
                        }
                        Entry::Vacant(_) => None,
                    }
                };

                let frame = if let Some(req_id) = reply_req_id {
                    (
                        None,
                        Self::build_respond_msg_frame(&req_id, &message.content, true),
                    )
                } else {
                    let (req_id, frame) =
                        Self::build_send_msg_frame(&message.recipient, &message.content);
                    (Some(req_id), frame)
                };

                let ack_rx = if let Some(req_id) = frame.0.as_ref() {
                    let (ack_tx, ack_rx) = oneshot::channel();
                    let mut pending = self.pending_send_acks.write().await;
                    match pending.entry(req_id.clone()) {
                        Entry::Vacant(slot) => {
                            slot.insert(ack_tx);
                            Some(ack_rx)
                        }
                        Entry::Occupied(_) => {
                            return Err(anyhow::anyhow!(
                                "WeCom: duplicate pending send req_id: {req_id}"
                            ));
                        }
                    }
                } else {
                    None
                };

                if self.outgoing_tx.send(frame.1.to_string()).await.is_err() {
                    if let Some(req_id) = frame.0.as_ref() {
                        let mut pending = self.pending_send_acks.write().await;
                        pending.remove(req_id);
                    }
                    return Err(anyhow::anyhow!("WeCom: outgoing channel closed"));
                }

                let Some(ack_rx) = ack_rx else {
                    return Ok(());
                };

                match tokio::time::timeout(
                    std::time::Duration::from_secs(SEND_ACK_TIMEOUT_SECS),
                    ack_rx,
                )
                .await
                {
                    Ok(Ok(result)) => result,
                    Ok(Err(_)) => Err(anyhow::anyhow!("WeCom: send ack channel closed")),
                    Err(_) => {
                        if let Some(req_id) = frame.0.as_ref() {
                            let mut pending = self.pending_send_acks.write().await;
                            pending.remove(req_id);
                        }
                        Err(anyhow::anyhow!(
                            "WeCom: timed out waiting for send ack after {SEND_ACK_TIMEOUT_SECS}s"
                        ))
                    }
                }?;
                Ok(())
            }
        }
    }

    async fn listen(&self, tx: tokio::sync::mpsc::Sender<ChannelMessage>) -> anyhow::Result<()> {
        match &self.mode {
            WeComMode::Webhook { .. } => {
                tracing::info!("WeCom: webhook mode does not support listening, skipping");
                loop {
                    tokio::time::sleep(std::time::Duration::from_secs(3600)).await;
                }
            }
            WeComMode::Bot { .. } => loop {
                let mut outgoing_rx = self.outgoing_rx.lock().await;
                match self.run_listen_loop(&tx, &mut outgoing_rx).await {
                    Ok(()) => {
                        tracing::info!("WeCom: WebSocket connection closed, reconnecting...");
                        tokio::time::sleep(std::time::Duration::from_secs(3)).await;
                    }
                    Err(e) => {
                        tracing::error!("WeCom: WebSocket error: {e}, reconnecting in 5s...");
                        tokio::time::sleep(std::time::Duration::from_secs(5)).await;
                    }
                }
            },
        }
    }

    async fn health_check(&self) -> bool {
        match &self.mode {
            WeComMode::Webhook { key } => !key.is_empty(),
            WeComMode::Bot {
                bot_id,
                secret,
                ws_url,
            } => match Self::connect_and_auth(ws_url, bot_id, secret).await {
                Ok(_) => true,
                Err(e) => {
                    tracing::warn!("WeCom health check failed: {e}");
                    false
                }
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn wecom_webhook_channel_name() {
        let ch = WeComChannel::new_webhook("key123".into(), vec![]);
        assert_eq!(ch.name(), "wecom");
    }

    #[test]
    fn wecom_bot_channel_name() {
        let ch = WeComChannel::new_bot("bot1".into(), "secret1".into(), vec![], None);
        assert_eq!(ch.name(), "wecom");
    }

    #[test]
    fn wecom_user_allowed_wildcard() {
        let ch = WeComChannel::new_webhook("key".into(), vec!["*".into()]);
        assert!(ch.is_user_allowed("anyone"));
    }

    #[test]
    fn wecom_user_allowed_specific() {
        let ch = WeComChannel::new_bot("bot".into(), "secret".into(), vec!["user123".into()], None);
        assert!(ch.is_user_allowed("user123"));
        assert!(!ch.is_user_allowed("other"));
    }

    #[test]
    fn wecom_user_denied_empty() {
        let ch = WeComChannel::new_webhook("key".into(), vec![]);
        assert!(!ch.is_user_allowed("anyone"));
    }

    #[test]
    fn wecom_bot_default_ws_url() {
        let ch = WeComChannel::new_bot("bot".into(), "secret".into(), vec![], None);
        assert!(ch.is_user_allowed("anyone") == false);
    }

    #[test]
    fn wecom_bot_custom_ws_url() {
        let ch = WeComChannel::new_bot(
            "bot".into(),
            "secret".into(),
            vec!["*".into()],
            Some("wss://custom.example.com".into()),
        );
        assert!(ch.is_user_allowed("anyone"));
    }

    #[test]
    fn wecom_config_serde() {
        let toml_str = r#"
bot_id = "bot_abc"
secret = "sec_xyz"
allowed_users = ["user1", "*"]
"#;
        let config: kylinbot_config::schema::WeComConfig = toml::from_str(toml_str).unwrap();
        assert_eq!(config.bot_id.as_deref(), Some("bot_abc"));
        assert_eq!(config.secret.as_deref(), Some("sec_xyz"));
        assert_eq!(config.allowed_users, vec!["user1", "*"]);
    }

    #[test]
    fn wecom_config_serde_defaults() {
        let toml_str = "";
        let config: kylinbot_config::schema::WeComConfig = toml::from_str(toml_str).unwrap();
        assert!(!config.enabled);
        assert!(config.webhook_key.is_none());
        assert!(config.bot_id.is_none());
        assert!(config.secret.is_none());
        assert!(config.allowed_users.is_empty());
    }

    #[test]
    fn wecom_config_webhook_serde() {
        let toml_str = r#"
enabled = true
webhook_key = "wh_key_123"
"#;
        let config: kylinbot_config::schema::WeComConfig = toml::from_str(toml_str).unwrap();
        assert!(config.enabled);
        assert_eq!(config.webhook_key.as_deref(), Some("wh_key_123"));
    }

    #[test]
    fn wecom_build_subscribe_frame() {
        let frame = WeComChannel::build_subscribe_frame("bot_123", "sec_456");
        assert_eq!(frame["cmd"], "aibot_subscribe");
        assert_eq!(frame["body"]["bot_id"], "bot_123");
        assert_eq!(frame["body"]["secret"], "sec_456");
        assert!(frame["headers"]["req_id"].is_string());
    }

    #[test]
    fn wecom_build_heartbeat_frame() {
        let frame = WeComChannel::build_heartbeat_frame();
        assert_eq!(frame["cmd"], "ping");
        assert!(frame["headers"]["req_id"].is_string());
    }

    #[test]
    fn wecom_build_respond_msg_frame() {
        let frame = WeComChannel::build_respond_msg_frame("req_abc", "hello world", true);
        assert_eq!(frame["cmd"], "aibot_respond_msg");
        assert_eq!(frame["headers"]["req_id"], "req_abc");
        assert_eq!(frame["body"]["stream"]["content"], "hello world");
        assert_eq!(frame["body"]["stream"]["finish"], true);
    }

    #[test]
    fn wecom_build_send_msg_frame_returns_req_id_and_payload() {
        let (req_id, frame) = WeComChannel::build_send_msg_frame("chat_123", "hello");
        assert!(req_id.starts_with("aibot_send_msg_"));
        assert_eq!(frame["cmd"], "aibot_send_msg");
        assert_eq!(frame["headers"]["req_id"], req_id);
        assert_eq!(frame["body"]["chatid"], "chat_123");
        assert_eq!(frame["body"]["markdown"]["content"], "hello");
    }

    #[tokio::test]
    async fn wecom_try_complete_send_ack_reports_success() {
        let ch = WeComChannel::new_bot("bot".into(), "secret".into(), vec!["*".into()], None);
        let (tx, rx) = oneshot::channel();
        {
            let mut pending = ch.pending_send_acks.write().await;
            pending.insert("req_ok".into(), tx);
        }

        let handled = ch
            .try_complete_send_ack(&serde_json::json!({
                "headers": { "req_id": "req_ok" },
                "errcode": 0
            }))
            .await;

        assert!(handled);
        assert!(rx.await.unwrap().is_ok());
    }

    #[tokio::test]
    async fn wecom_try_complete_send_ack_reports_error() {
        let ch = WeComChannel::new_bot("bot".into(), "secret".into(), vec!["*".into()], None);
        let (tx, rx) = oneshot::channel();
        {
            let mut pending = ch.pending_send_acks.write().await;
            pending.insert("req_err".into(), tx);
        }

        let handled = ch
            .try_complete_send_ack(&serde_json::json!({
                "headers": { "req_id": "req_err" },
                "errcode": 93000,
                "errmsg": "chat not found"
            }))
            .await;

        assert!(handled);
        let err = rx.await.unwrap().unwrap_err().to_string();
        assert!(err.contains("93000"));
        assert!(err.contains("chat not found"));
    }

    #[tokio::test]
    async fn wecom_fail_pending_send_acks_notifies_waiters() {
        let ch = WeComChannel::new_bot("bot".into(), "secret".into(), vec!["*".into()], None);
        let (tx, rx) = oneshot::channel();
        {
            let mut pending = ch.pending_send_acks.write().await;
            pending.insert("req_fail".into(), tx);
        }

        ch.fail_pending_send_acks("socket disconnected").await;

        let err = rx.await.unwrap().unwrap_err().to_string();
        assert!(err.contains("socket disconnected"));
        assert!(ch.pending_send_acks.read().await.is_empty());
    }

    #[tokio::test]
    async fn wecom_send_ignores_stale_pending_req_id_for_proactive_send() {
        let ch = Arc::new(WeComChannel::new_bot(
            "bot".into(),
            "secret".into(),
            vec!["*".into()],
            None,
        ));
        {
            let mut pending = ch.pending_req_ids.write().await;
            pending.insert(
                "chat_123".into(),
                PendingReplyTarget {
                    req_id: "stale_req".into(),
                    created_at: std::time::Instant::now()
                        - std::time::Duration::from_secs(PENDING_REQ_ID_TTL_SECS + 1),
                },
            );
        }

        let ch_for_task = Arc::clone(&ch);
        let outgoing = tokio::spawn(async move {
            let mut rx = ch_for_task.outgoing_rx.lock().await;
            let frame_str = rx.recv().await.expect("outgoing frame should be queued");
            let frame: serde_json::Value =
                serde_json::from_str(&frame_str).expect("frame should be valid json");
            assert_eq!(frame["cmd"], "aibot_send_msg");
            let req_id = frame["headers"]["req_id"]
                .as_str()
                .expect("send frame should include req_id");
            ch_for_task
                .try_complete_send_ack(&serde_json::json!({
                    "headers": { "req_id": req_id },
                    "errcode": 0
                }))
                .await;
        });

        ch.send(&SendMessage::new("hello", "chat_123"))
            .await
            .expect("send should fall back to proactive chat send");
        outgoing.await.unwrap();
    }

    #[test]
    fn wecom_pkcs7_unpad_valid() {
        let data = vec![0x41u8, 0x42, 0x43, 0x04, 0x04, 0x04, 0x04];
        let result = pkcs7_unpad(&data, 32).unwrap();
        assert_eq!(result, &[0x41, 0x42, 0x43]);
    }

    #[test]
    fn wecom_pkcs7_unpad_empty_data() {
        let data: Vec<u8> = vec![];
        let result = pkcs7_unpad(&data, 32);
        assert!(result.is_err());
    }

    #[test]
    fn wecom_pkcs7_unpad_invalid_padding_value() {
        let data = vec![0x41u8, 0x00];
        let result = pkcs7_unpad(&data, 32);
        assert!(result.is_err());
    }

    #[test]
    fn wecom_pkcs7_unpad_padding_exceeds_length() {
        let data = vec![0x41u8, 0x05];
        let result = pkcs7_unpad(&data, 32);
        assert!(result.is_err());
    }

    #[test]
    fn wecom_pkcs7_unpad_inconsistent_padding_bytes() {
        let data = vec![0x41u8, 0x03, 0x02];
        let result = pkcs7_unpad(&data, 32);
        assert!(result.is_err());
    }

    #[test]
    fn wecom_generate_req_id_format() {
        let id = WeComChannel::generate_req_id("test_prefix");
        assert!(id.starts_with("test_prefix_"));
    }

    #[test]
    fn wecom_ws_reset_without_close_is_recoverable() {
        let error =
            TungsteniteError::Protocol(TungsteniteProtocolError::ResetWithoutClosingHandshake);

        assert!(is_recoverable_wecom_ws_disconnect(&error));
    }

    #[test]
    fn wecom_ws_connection_reset_io_is_recoverable() {
        let error = TungsteniteError::Io(std::io::Error::from(std::io::ErrorKind::ConnectionReset));

        assert!(is_recoverable_wecom_ws_disconnect(&error));
    }

    #[test]
    fn wecom_webhook_health_check_with_key() {
        let ch = WeComChannel::new_webhook("valid_key".into(), vec![]);
        let rt = tokio::runtime::Runtime::new().unwrap();
        let healthy = rt.block_on(ch.health_check());
        assert!(healthy);
    }

    #[test]
    fn wecom_webhook_health_check_empty_key() {
        let ch = WeComChannel::new_webhook("".into(), vec![]);
        let rt = tokio::runtime::Runtime::new().unwrap();
        let healthy = rt.block_on(ch.health_check());
        assert!(!healthy);
    }
}
