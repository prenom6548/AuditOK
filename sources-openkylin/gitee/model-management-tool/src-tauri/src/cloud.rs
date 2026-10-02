use serde::Deserialize;
use std::sync::atomic::{AtomicBool, Ordering};

use crate::packager::{discover_model_files, ModelFile};

const MANIFEST_URL: &str = "https://archive2.kylinos.cn/LLM/KYLIN/branches/default.json";
const MODEL_BASE_URL: &str = "https://archive2.kylinos.cn/LLM/KYLIN/models";
const CHUNK_SIZE: usize = 1024 * 1024;

/// 云端下载暂存根目录名（`outputDir/.cloud-staging/<subPath>/`），与前端拼接规则一致
pub const STAGING_ROOT_NAME: &str = ".cloud-staging";

pub struct CloudDownloadState {
    pub cancel: std::sync::Arc<AtomicBool>,
    /// 是否有下载任务正在运行，清理暂存前据此等待任务退出，避免边删边写
    pub active: std::sync::Arc<AtomicBool>,
}

impl CloudDownloadState {
    pub fn new() -> Self {
        Self {
            cancel: std::sync::Arc::new(AtomicBool::new(false)),
            active: std::sync::Arc::new(AtomicBool::new(false)),
        }
    }
}

impl Default for CloudDownloadState {
    fn default() -> Self {
        Self::new()
    }
}

#[derive(Debug)]
pub enum CloudError {
    Http(String),
    Io(String),
    Validation(String),
    Cancelled,
}

impl std::fmt::Display for CloudError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            CloudError::Http(message) => write!(f, "{message}"),
            CloudError::Io(message) => write!(f, "{message}"),
            CloudError::Validation(message) => write!(f, "{message}"),
            CloudError::Cancelled => write!(f, "下载已取消"),
        }
    }
}

impl From<reqwest::Error> for CloudError {
    fn from(err: reqwest::Error) -> Self {
        CloudError::Http(err.to_string())
    }
}

impl From<std::io::Error> for CloudError {
    fn from(err: std::io::Error) -> Self {
        CloudError::Io(err.to_string())
    }
}

impl From<serde_json::Error> for CloudError {
    fn from(err: serde_json::Error) -> Self {
        CloudError::Http(format!("清单解析失败: {err}"))
    }
}

impl From<crate::packager::PackageError> for CloudError {
    fn from(err: crate::packager::PackageError) -> Self {
        CloudError::Io(err.to_string())
    }
}

#[derive(Debug, Clone, Deserialize)]
pub struct BranchManifest {
    pub models: std::collections::BTreeMap<String, RemoteModel>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct RemoteModel {
    #[serde(rename = "subPath")]
    pub sub_path: String,
    pub descrip: String,
    pub files: std::collections::BTreeMap<String, RemoteFile>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct RemoteFile {
    pub size: u64,
    pub sha256: String,
    pub time: Option<String>,
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct CloudModel {
    pub id: String,
    pub name: String,
    pub vendor: String,
    pub parameter_count: u64,
    pub capabilities: Vec<String>,
    pub model_capability: Vec<String>,
    pub sub_path: String,
    pub description: String,
    pub files: Vec<CloudFile>,
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct CloudFile {
    pub name: String,
    pub size: u64,
    pub sha256: String,
}

#[derive(Debug, Clone, serde::Deserialize)]
pub struct DownloadRequest {
    pub sub_path: String,
    pub files: Vec<DownloadFile>,
    pub staging_dir: String,
}

#[derive(Debug, Clone, serde::Deserialize)]
pub struct DownloadFile {
    pub name: String,
    pub size: u64,
    pub sha256: String,
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct DownloadStarted {
    pub sub_path: String,
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct DownloadProgress {
    pub sub_path: String,
    pub file_name: String,
    pub downloaded_bytes: u64,
    pub file_size: u64,
    pub file_progress: f64,
    pub overall_downloaded: u64,
    pub overall_size: u64,
    pub overall_progress: f64,
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct DownloadFileDone {
    pub sub_path: String,
    pub file_name: String,
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct DownloadCompleted {
    pub sub_path: String,
    pub files: Vec<ModelFile>,
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct DownloadErrorEvent {
    pub sub_path: String,
    pub file_name: Option<String>,
    pub message: String,
}

fn http_client() -> Result<reqwest::Client, CloudError> {
    reqwest::Client::builder()
        .connect_timeout(std::time::Duration::from_secs(30))
        .build()
        .map_err(CloudError::from)
}

pub async fn fetch_manifest() -> Result<BranchManifest, CloudError> {
    let client = http_client()?;
    let response = client.get(MANIFEST_URL).send().await?;
    if !response.status().is_success() {
        return Err(CloudError::Http(format!(
            "拉取清单失败: HTTP {}",
            response.status()
        )));
    }
    let body = response.bytes().await?;
    let manifest: BranchManifest = serde_json::from_slice(&body)?;
    Ok(manifest)
}

// 列表并发拉取每个模型的 model_description.yaml：
// 既用于取 model_capability（区分语音/大语言模型展示），
// 也复用同一份文本做后摩过滤；拉取失败的模型不展示。
pub async fn list_cloud_models() -> Result<Vec<CloudModel>, CloudError> {
    let manifest = fetch_manifest().await?;
    let entries: Vec<(String, RemoteModel)> = manifest.models.into_iter().collect();
    let descriptions = match http_client() {
        Ok(client) => {
            let fetches = entries.iter().map(|(_, remote)| {
                let client = client.clone();
                let url = format!("{MODEL_BASE_URL}/{}/model_description.yaml", remote.sub_path);
                async move { fetch_description_text(&client, &url).await }
            });
            futures_util::future::join_all(fetches).await
        }
        Err(_) => std::iter::repeat(None).take(entries.len()).collect(),
    };
    let has_houmo = crate::device::has_houmo_card();
    let models = entries
        .into_iter()
        .zip(descriptions)
        .filter(|(_, text)| should_show_model(text.as_deref(), has_houmo))
        .map(|((name, remote), text)| {
            let model_capability = text
                .as_deref()
                .map(|raw| parse_model_description_yaml(raw).model_capability)
                .unwrap_or_default();
            CloudModel {
                id: remote.sub_path.clone(),
                name,
                vendor: "Kylin 云端仓库".to_string(),
                parameter_count: 0,
                capabilities: vec!["text".to_string()],
                model_capability,
                sub_path: remote.sub_path,
                description: remote.descrip,
                files: remote
                    .files
                    .into_iter()
                    .map(|(file_name, file)| CloudFile {
                        name: file_name,
                        size: file.size,
                        sha256: file.sha256,
                    })
                    .collect(),
            }
        })
        .collect();
    Ok(models)
}

async fn fetch_description_text(client: &reqwest::Client, url: &str) -> Option<String> {
    let response = client.get(url).send().await.ok()?;
    if !response.status().is_success() {
        return None;
    }
    response.text().await.ok()
}

pub fn is_houmo_model_description(text: &str) -> bool {
    let model_name = yaml_unquote(yaml_pick_line(text, "model_name"));
    crate::device::is_houmo_model_name(&model_name)
}

// 仓库列表展示规则：描述拉取失败（None）的模型一律不展示；
// 后摩机器只展示后摩模型，非后摩机器只展示普通模型。
pub fn should_show_model(description_text: Option<&str>, has_houmo_card: bool) -> bool {
    match description_text {
        Some(raw) => is_houmo_model_description(raw) == has_houmo_card,
        None => false,
    }
}

#[derive(Debug, Clone, serde::Serialize)]
pub struct CloudModelDescription {
    pub model_deb_name: String,
    pub model_name: String,
    pub model_display_name: String,
    pub brand: String,
    pub capabilities: Vec<String>,
    pub model_capability: Vec<String>,
    pub model_supports_thinking: bool,
    pub model_supports_thinking_toggle: bool,
    pub supports_tool_choice: bool,
    pub model_max_token: Option<u64>,
    pub config_max_token: Option<u64>,
    pub parameter_count: Option<u64>,
}

fn yaml_pick_line(text: &str, key: &str) -> Option<String> {
    let prefix = format!("{key}:");
    for line in text.lines() {
        let trimmed = line.trim();
        if let Some(rest) = trimmed.strip_prefix(&prefix) {
            return Some(rest.trim().to_string());
        }
    }
    None
}

fn yaml_unquote(value: Option<String>) -> String {
    match value {
        Some(s) => {
            let s = s.trim();
            if let Some(stripped) = s.strip_prefix('"') {
                let chars: Vec<char> = stripped.chars().collect();
                let mut i = 0;
                while i < chars.len() {
                    if chars[i] == '\\' && i + 1 < chars.len() {
                        i += 2;
                        continue;
                    }
                    if chars[i] == '"' {
                        let inner: String = chars[..i].iter().collect();
                        return inner.replace("\\\"", "\"").replace("\\\\", "\\");
                    }
                    i += 1;
                }
            }
            if let Some(hash_idx) = s.find(" #") {
                return s[..hash_idx].trim().to_string();
            }
            s.to_string()
        }
        None => String::new(),
    }
}

fn yaml_bool(text: &str, key: &str) -> bool {
    yaml_unquote(yaml_pick_line(text, key)).eq_ignore_ascii_case("true")
}

fn yaml_array(text: &str, key: &str) -> Vec<String> {
    let raw = yaml_unquote(yaml_pick_line(text, key));
    let inner = raw
        .strip_prefix('[')
        .and_then(|s| s.strip_suffix(']'))
        .unwrap_or(&raw);
    inner
        .split(',')
        .map(|item| item.trim().trim_matches('"').trim().to_string())
        .filter(|item| !item.is_empty())
        .collect()
}

fn others_str(others: &serde_json::Value, key: &str) -> Option<String> {
    others.get(key).and_then(|v| v.as_str()).map(|s| s.to_string())
}

fn others_bool(others: &serde_json::Value, key: &str) -> bool {
    match others.get(key) {
        Some(serde_json::Value::Bool(b)) => *b,
        Some(_) => others_str(others, key)
            .map(|s| s.eq_ignore_ascii_case("true"))
            .unwrap_or(false),
        None => false,
    }
}

fn others_num(others: &serde_json::Value, key: &str) -> Option<u64> {
    others
        .get(key)
        .and_then(|v| v.as_u64())
        .or_else(|| {
            others
                .get(key)
                .and_then(|v| v.as_str())
                .and_then(|s| s.parse().ok())
        })
}

fn parse_model_description_yaml(text: &str) -> CloudModelDescription {
    let others_raw = yaml_unquote(yaml_pick_line(text, "others_info"));
    let others: serde_json::Value = serde_json::from_str(&others_raw)
        .unwrap_or_else(|_| serde_json::Value::Object(serde_json::Map::new()));
    let capabilities: Vec<String> = others
        .get("capabilities")
        .and_then(|v| v.as_array())
        .map(|arr| {
            arr.iter()
                .filter_map(|item| item.as_str())
                .map(|s| s.to_string())
                .collect()
        })
        .unwrap_or_default();
    CloudModelDescription {
        model_deb_name: yaml_unquote(yaml_pick_line(text, "model_deb_name")),
        model_name: yaml_unquote(yaml_pick_line(text, "model_name")),
        model_display_name: yaml_unquote(yaml_pick_line(text, "model_display_name")),
        brand: yaml_unquote(yaml_pick_line(text, "brand")),
        capabilities,
        model_capability: yaml_array(text, "model_capability"),
        model_supports_thinking: yaml_bool(text, "model_supports_thinking"),
        model_supports_thinking_toggle: yaml_bool(text, "model_supports_thinking_toggle"),
        supports_tool_choice: others_bool(&others, "supportsToolChoice"),
        model_max_token: others_num(&others, "modelMaxToken"),
        config_max_token: others_num(&others, "configMaxToken"),
        parameter_count: others_num(&others, "parameterCount"),
    }
}

pub fn read_model_description_file(path: &str) -> Result<CloudModelDescription, CloudError> {
    let text = std::fs::read_to_string(path)?;
    Ok(parse_model_description_yaml(&text))
}

pub async fn fetch_model_description(sub_path: &str) -> Result<CloudModelDescription, CloudError> {
    let url = format!("{MODEL_BASE_URL}/{sub_path}/model_description.yaml");
    let client = http_client()?;
    let response = client.get(&url).send().await?;
    if !response.status().is_success() {
        return Err(CloudError::Http(format!(
            "拉取模型描述失败: HTTP {}",
            response.status()
        )));
    }
    let body = response.text().await?;
    Ok(parse_model_description_yaml(&body))
}

fn part_path(staging_dir: &std::path::Path, file_name: &str) -> std::path::PathBuf {
    staging_dir.join(format!("{file_name}.part"))
}

fn final_path(staging_dir: &std::path::Path, file_name: &str) -> std::path::PathBuf {
    staging_dir.join(file_name)
}

fn existing_size(path: &std::path::Path) -> u64 {
    std::fs::metadata(path).map(|meta| meta.len()).unwrap_or(0)
}

fn content_range_start(response: &reqwest::Response) -> Option<u64> {
    let value = response.headers().get(reqwest::header::CONTENT_RANGE)?;
    let text = value.to_str().ok()?;
    parse_content_range_start(text)
}

fn parse_content_range_start(text: &str) -> Option<u64> {
    let range = text.strip_prefix("bytes ")?;
    let start = range.split('-').next()?;
    start.trim().parse().ok()
}

pub async fn download_file<F>(
    url: &str,
    part_path: &std::path::Path,
    have: u64,
    size: u64,
    cancel: Option<&AtomicBool>,
    on_progress: &mut F,
) -> Result<u64, CloudError>
where
    F: FnMut(u64),
{
    if have == size {
        return Ok(have);
    }
    // 残留暂存超过预期大小（远端文件变更或脏数据）：无法续传，删除后从头开始
    if have > size {
        std::fs::remove_file(part_path).ok();
    }

    let client = http_client()?;
    let mut have = if have > size { 0 } else { have };
    // 续传不可信时允许最多一次「从头重下」兜底，避免死循环
    let mut restarts_left = if have == 0 { 1 } else { 2 };

    loop {
        let request = client.get(url).header("Range", format!("bytes={have}-"));
        let response = request.send().await?;
        let status = response.status();
        if !status.is_success() {
            return Err(CloudError::Http(format!("下载失败: HTTP {status}")));
        }
        let supports_range = status == reqwest::StatusCode::PARTIAL_CONTENT;

        if supports_range {
            match content_range_start(&response) {
                Some(start) if start == have => {}
                Some(start) => {
                    // 服务器返回起点与请求偏移不符，续传不可信
                    std::fs::remove_file(part_path).ok();
                    if restarts_left == 0 {
                        return Err(CloudError::Validation(format!(
                            "服务器返回的 Content-Range 起点 {start} 与请求偏移 {have} 不符"
                        )));
                    }
                    have = 0;
                    restarts_left -= 1;
                    continue;
                }
                None => {
                    // 206 但缺少 Content-Range，无法确认起点，按不可续传处理
                    std::fs::remove_file(part_path).ok();
                    if restarts_left == 0 {
                        return Err(CloudError::Validation(
                            "服务器返回 206 但缺少 Content-Range 头".to_string(),
                        ));
                    }
                    have = 0;
                    restarts_left -= 1;
                    continue;
                }
            }
        } else {
            // 200：服务器忽略 Range，从头下载
            std::fs::remove_file(part_path).ok();
        }

        let (mut file, mut downloaded) = if supports_range {
            let file = std::fs::OpenOptions::new()
                .create(true)
                .append(true)
                .open(part_path)?;
            (file, have)
        } else {
            let file = std::fs::OpenOptions::new()
                .create(true)
                .write(true)
                .truncate(true)
                .open(part_path)?;
            (file, 0)
        };

        use futures_util::StreamExt;
        let mut stream = response.bytes_stream();
        use std::io::Write;
        let mut last_report = std::time::Instant::now();
        while let Some(chunk_result) = stream.next().await {
            if let Some(flag) = cancel {
                if flag.load(Ordering::SeqCst) {
                    return Err(CloudError::Cancelled);
                }
            }
            let chunk = chunk_result.map_err(CloudError::from)?;
            let mut write_offset = 0usize;
            while write_offset < chunk.len() {
                let end = std::cmp::min(write_offset + CHUNK_SIZE, chunk.len());
                let slice = &chunk[write_offset..end];
                let written = file.write(slice)?;
                if written == 0 {
                    return Err(CloudError::Io("写入分片为 0 字节".to_string()));
                }
                write_offset += written;
            }
            downloaded += chunk.len() as u64;
            if downloaded > size {
                return Err(CloudError::Validation(format!(
                    "下载数据超出预期大小: {downloaded} > {size}"
                )));
            }
            if last_report.elapsed() >= std::time::Duration::from_millis(200) || downloaded == size {
                on_progress(downloaded);
                last_report = std::time::Instant::now();
            }
        }

        if downloaded != size {
            return Err(CloudError::Validation(format!(
                "下载未完成: {downloaded} / {size}"
            )));
        }

        return Ok(downloaded);
    }
}

#[allow(clippy::too_many_arguments)]
async fn download_one_file<F>(
    url: &str,
    part_path: &std::path::Path,
    final_path: &std::path::Path,
    have: u64,
    size: u64,
    sha256: &str,
    cancel: Option<&AtomicBool>,
    mut on_progress: F,
) -> Result<(), CloudError>
where
    F: FnMut(u64),
{
    let mut have = have;
    loop {
        download_file(url, part_path, have, size, cancel, &mut on_progress).await?;
        if let Err(first_err) = finalize_file(part_path, final_path, sha256, size) {
            // 暂存内容损坏（校验失败）：删除 .part 后从头重下一次
            std::fs::remove_file(part_path).ok();
            if have == 0 {
                return Err(first_err);
            }
            have = 0;
            continue;
        }
        return Ok(());
    }
}

pub fn verify_file(
    final_path: &std::path::Path,
    expected_sha256: &str,
    expected_size: u64,
) -> Result<(), CloudError> {
    use sha2::Digest;
    use std::io::Read;
    let actual_size = std::fs::metadata(final_path)?.len();
    if actual_size != expected_size {
        return Err(CloudError::Validation(format!(
            "文件大小不一致: {actual_size} != {expected_size}"
        )));
    }

    let mut hasher = sha2::Sha256::new();
    let mut file = std::fs::File::open(final_path)?;
    let mut buffer = [0u8; 1024 * 1024];
    loop {
        let read = file.read(&mut buffer)?;
        if read == 0 {
            break;
        }
        hasher.update(&buffer[..read]);
    }
    let digest = hasher.finalize();
    let actual_sha = format!("{digest:x}");
    if !actual_sha.eq_ignore_ascii_case(expected_sha256) {
        return Err(CloudError::Validation(format!(
            "SHA-256 校验失败: {actual_sha} != {expected_sha256}"
        )));
    }
    Ok(())
}

pub fn finalize_file(
    part_path: &std::path::Path,
    final_path: &std::path::Path,
    expected_sha256: &str,
    expected_size: u64,
) -> Result<(), CloudError> {
    verify_file(part_path, expected_sha256, expected_size)?;
    std::fs::rename(part_path, final_path)?;
    Ok(())
}

fn disk_available(path: &std::path::Path) -> Option<u64> {
    let candidate = if path.is_dir() {
        path.to_path_buf()
    } else {
        path.parent()?.to_path_buf()
    };
    fs2::available_space(&candidate).ok()
}

fn is_staging_path(path: &std::path::Path) -> bool {
    path.file_name().map(|name| name == STAGING_ROOT_NAME).unwrap_or(false)
        || path
            .components()
            .any(|component| component.as_os_str() == STAGING_ROOT_NAME)
}

/// 删除暂存目录（`.cloud-staging` 本身或其子目录）。
/// 路径必须位于暂存根目录内，防止误删其他目录；目录不存在视为成功（幂等）。
pub fn clean_staging_dir(staging_dir: &std::path::Path) -> Result<(), CloudError> {
    if !is_staging_path(staging_dir) {
        return Err(CloudError::Validation(format!(
            "非法的暂存目录，拒绝删除: {}",
            staging_dir.display()
        )));
    }
    match std::fs::remove_dir_all(staging_dir) {
        Ok(()) => Ok(()),
        Err(err) if err.kind() == std::io::ErrorKind::NotFound => Ok(()),
        Err(err) => Err(CloudError::Io(format!(
            "清理暂存目录失败 {}: {err}",
            staging_dir.display()
        ))),
    }
}

/// 删除输出目录下的整个 `.cloud-staging` 根目录（应用启动时兜底清理残留）
pub fn clean_staging_root(output_dir: &std::path::Path) -> Result<(), CloudError> {
    clean_staging_dir(&output_dir.join(STAGING_ROOT_NAME))
}

pub async fn download_cloud_model(
    app: tauri::AppHandle,
    request: DownloadRequest,
    cancel: Option<&AtomicBool>,
) -> Result<Vec<ModelFile>, CloudError> {
    let staging_dir = std::path::PathBuf::from(&request.staging_dir);
    std::fs::create_dir_all(&staging_dir)?;

    let download_files: Vec<&DownloadFile> = request.files.iter().collect();

    if download_files.is_empty() {
        return Err(CloudError::Validation(
            "未发现可下载的文件".to_string(),
        ));
    }

    let overall_size: u64 = download_files.iter().map(|file| file.size).sum();
    let remaining_size: u64 = download_files
        .iter()
        .map(|file| {
            let part = part_path(&staging_dir, &file.name);
            file.size.saturating_sub(existing_size(&part))
        })
        .sum();
    if let Some(available) = disk_available(&staging_dir) {
        if remaining_size > available {
            return Err(CloudError::Validation(format!(
                "磁盘可用空间不足: 需要 {remaining_size} 字节，可用 {available} 字节"
            )));
        }
    }

    emit(
        &app,
        "cloud-download-started",
        DownloadStarted {
            sub_path: request.sub_path.clone(),
        },
    );

    let mut overall_downloaded: u64 = 0;
    for file in &download_files {
        if let Some(flag) = cancel {
            if flag.load(Ordering::SeqCst) {
                emit(
                    &app,
                    "cloud-download-error",
                    DownloadErrorEvent {
                        sub_path: request.sub_path.clone(),
                        file_name: Some(file.name.clone()),
                        message: "下载已取消".to_string(),
                    },
                );
                return Err(CloudError::Cancelled);
            }
        }

        let part = part_path(&staging_dir, &file.name);
        let final_p = final_path(&staging_dir, &file.name);
        let have = existing_size(&part);
        let seeded = have.min(file.size);

        if final_p.exists() {
            match verify_file(&final_p, &file.sha256, file.size) {
                Ok(()) => {
                    let _ = std::fs::remove_file(&part);
                    emit(
                        &app,
                        "cloud-download-file-done",
                        DownloadFileDone {
                            sub_path: request.sub_path.clone(),
                            file_name: file.name.clone(),
                        },
                    );
                    overall_downloaded += file.size;
                    emit_progress(
                        &app,
                        &request.sub_path,
                        &file.name,
                        file.size,
                        file.size,
                        overall_downloaded,
                        overall_size,
                    );
                    continue;
                }
                Err(err) => {
                    let _ = std::fs::remove_file(&final_p);
                    let _ = std::fs::remove_file(&part);
                    return Err(CloudError::Validation(format!(
                        "已下载文件 {} 校验失败: {}，已删除请重新下载",
                        file.name, err
                    )));
                }
            }
        }

        overall_downloaded += seeded;
        let file_name = file.name.clone();
        let url = format!("{MODEL_BASE_URL}/{}/{file_name}", request.sub_path);
        let sub_path_for_progress = request.sub_path.clone();
        let file_size = file.size;
        let overall_so_far = overall_downloaded;
        let overall_total = overall_size;
        let app_for_progress = app.clone();
        let result = download_one_file(
            &url,
            &part,
            &final_p,
            have,
            file_size,
            &file.sha256,
            cancel,
            move |downloaded_bytes| {
                emit_progress(
                    &app_for_progress,
                    &sub_path_for_progress,
                    &file_name,
                    downloaded_bytes,
                    file_size,
                    overall_so_far - seeded + downloaded_bytes,
                    overall_total,
                );
            },
        )
        .await;

        match result {
            Ok(()) => {
                overall_downloaded += file_size - seeded;
                emit(
                    &app,
                    "cloud-download-file-done",
                    DownloadFileDone {
                        sub_path: request.sub_path.clone(),
                        file_name: file.name.clone(),
                    },
                );
            }
            Err(err) => {
                let message = err.to_string();
                emit(
                    &app,
                    "cloud-download-error",
                    DownloadErrorEvent {
                        sub_path: request.sub_path.clone(),
                        file_name: Some(file.name.clone()),
                        message,
                    },
                );
                return Err(err);
            }
        }
    }

    let mut paths = Vec::new();
    paths.push(staging_dir);
    let files = discover_model_files(&paths)?;

    emit(
        &app,
        "cloud-download-completed",
        DownloadCompleted {
            sub_path: request.sub_path,
            files: files.clone(),
        },
    );

    Ok(files)
}

fn emit<T: serde::Serialize + Clone>(
    app: &tauri::AppHandle,
    event: &str,
    payload: T,
) {
    #[cfg(feature = "tauri-v1")]
    {
        use tauri::Manager;
        let _ = app.emit_all(event, payload);
    }
    #[cfg(feature = "tauri-v2")]
    {
        use tauri::Emitter;
        let _ = app.emit(event, payload);
    }
}

#[allow(clippy::too_many_arguments)]
fn emit_progress(
    app: &tauri::AppHandle,
    sub_path: &str,
    file_name: &str,
    downloaded_bytes: u64,
    file_size: u64,
    overall_downloaded: u64,
    overall_size: u64,
) {
    let file_progress = if file_size == 0 {
        100.0
    } else {
        ((downloaded_bytes as f64 / file_size as f64) * 100.0).min(100.0)
    };
    let overall_progress = if overall_size == 0 {
        100.0
    } else {
        ((overall_downloaded as f64 / overall_size as f64) * 100.0).min(100.0)
    };
    emit(
        app,
        "cloud-download-progress",
        DownloadProgress {
            sub_path: sub_path.to_string(),
            file_name: file_name.to_string(),
            downloaded_bytes,
            file_size,
            file_progress,
            overall_downloaded,
            overall_size,
            overall_progress,
        },
    );
}

#[cfg(test)]
mod tests {
    use super::*;

    const FIXTURE: &str = r#"{
  "models": {
    "Qwen3.6-35B-A3B": {
      "subPath": "Qwen3.6-35B-A3B",
      "descrip": "Qwen3.6-35B-A3B",
      "files": {
        "config.pbtxt": { "size": 4027, "sha256": "aaa", "time": "2026-07-30 16:49:04" },
        "Qwen3.6-35B-A3B-Q4_K_M.gguf": { "size": 21166757728, "sha256": "bbb", "time": "2026-07-15 11:47:49" }
      }
    }
  }
}"#;

    #[test]
    fn parses_manifest() {
        let manifest: BranchManifest = serde_json::from_str(FIXTURE).unwrap();
        let model = manifest.models.get("Qwen3.6-35B-A3B").unwrap();
        assert_eq!(model.sub_path, "Qwen3.6-35B-A3B");
        let gguf = model.files.get("Qwen3.6-35B-A3B-Q4_K_M.gguf").unwrap();
        assert_eq!(gguf.size, 21166757728);
        assert_eq!(gguf.sha256, "bbb");
    }

    #[test]
    fn computes_part_and_final_paths() {
        let dir = std::path::Path::new("/tmp/stage");
        assert_eq!(part_path(dir, "m.gguf"), std::path::PathBuf::from("/tmp/stage/m.gguf.part"));
        assert_eq!(final_path(dir, "m.gguf"), std::path::PathBuf::from("/tmp/stage/m.gguf"));
    }

    #[test]
    fn existing_size_is_zero_when_missing() {
        assert_eq!(existing_size(std::path::Path::new("/nonexistent/path/12345")), 0);
    }

    #[test]
    fn detects_houmo_model_from_description() {
        let houmo = "model_deb_name: \"llm_Qwen-3.6-35ba3b-1_1.0\"\nmodel_name: \"llm_Qwen-3.6-35ba3b-1_1.0\"\n";
        assert!(is_houmo_model_description(houmo));
        let normal = "model_deb_name: \"llm_Qwen-3.6-35ba3b-0_1.0\"\nmodel_name: llm_Qwen-3.6-35ba3b-0_1.0\n";
        assert!(!is_houmo_model_description(normal));
        assert!(!is_houmo_model_description(""));
    }

    #[test]
    fn shows_model_only_when_houmo_flag_matches_card() {
        let houmo = "model_name: \"llm_Qwen-3.6-35ba3b-1_1.0\"\n";
        let normal = "model_name: llm_Qwen-3.6-35ba3b-0_1.0\n";
        let asr_houmo = "model_name: asr_Qwen3-ASR-0.6b-1_1.0\n";
        assert!(should_show_model(Some(houmo), true));
        assert!(!should_show_model(Some(houmo), false));
        assert!(should_show_model(Some(normal), false));
        assert!(!should_show_model(Some(normal), true));
        assert!(should_show_model(Some(asr_houmo), true));
        assert!(!should_show_model(None, true));
        assert!(!should_show_model(None, false));
    }

    #[test]
    fn parses_supports_tool_choice_from_bool_or_string() {
        let boolean = "others_info: \"{\\\"supportsToolChoice\\\":true,\\\"capabilities\\\":[\\\"text\\\"]}\"\n";
        assert!(parse_model_description_yaml(boolean).supports_tool_choice);
        let string = "others_info: \"{\\\"supportsToolChoice\\\":\\\"true\\\"}\"\n";
        assert!(parse_model_description_yaml(string).supports_tool_choice);
        let boolean_false = "others_info: \"{\\\"supportsToolChoice\\\":false}\"\n";
        assert!(!parse_model_description_yaml(boolean_false).supports_tool_choice);
    }

    #[test]
    fn parses_model_capability_array() {
        let text = "model_name: asr_Qwen3-ASR-0.6b-1_1.0\nmodel_capability: [\"once_asr\"] #模型能力\n";
        let desc = parse_model_description_yaml(text);
        assert_eq!(desc.model_capability, vec!["once_asr".to_string()]);

        let multi = "model_capability: [\"llm\", \"image\"]\n";
        let desc = parse_model_description_yaml(multi);
        assert_eq!(
            desc.model_capability,
            vec!["llm".to_string(), "image".to_string()]
        );

        assert!(parse_model_description_yaml("").model_capability.is_empty());
    }

    #[test]
    fn parses_content_range_start() {
        assert_eq!(parse_content_range_start("bytes 0-1023/2048"), Some(0));
        assert_eq!(parse_content_range_start("bytes 1024-2047/2048"), Some(1024));
        assert_eq!(parse_content_range_start("bytes */2048"), None);
        assert_eq!(parse_content_range_start(""), None);
        assert_eq!(parse_content_range_start("Bogus"), None);
    }
}
