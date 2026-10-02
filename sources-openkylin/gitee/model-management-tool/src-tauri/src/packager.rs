use serde::{Deserialize, Serialize};
use std::fmt;
use std::fs;
use std::io;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, Ordering};

const COPY_BUFFER_SIZE: usize = 1024 * 1024;

const INSTALL_ROOT: &str = "/opt/appdata/kylin-ai/model-repository";

#[derive(Debug)]
pub enum PackageError {
    Io(io::Error),
    Validation(String),
    Cancelled,
}

impl fmt::Display for PackageError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            PackageError::Io(err) => write!(f, "{err}"),
            PackageError::Validation(message) => write!(f, "{message}"),
            PackageError::Cancelled => write!(f, "打包已取消"),
        }
    }
}

impl From<io::Error> for PackageError {
    fn from(value: io::Error) -> Self {
        PackageError::Io(value)
    }
}

pub type PackageResultValue<T> = Result<T, PackageError>;

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum ModelSource {
    Local,
    Cloud,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum ModelKind {
    Llm,
    Asr,
    Tts,
}

impl Default for ModelKind {
    fn default() -> Self {
        ModelKind::Llm
    }
}

impl ModelKind {
    pub fn is_voice(&self) -> bool {
        matches!(self, ModelKind::Asr | ModelKind::Tts)
    }

    pub fn name_prefix(&self) -> &'static str {
        match self {
            ModelKind::Llm => "llm",
            ModelKind::Asr => "asr",
            ModelKind::Tts => "tts",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum AsrMode {
    Streaming,
    Once,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum ModelFileRole {
    Base,
    Lora,
    Mmproj,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct ModelFile {
    pub path: PathBuf,
    pub file_name: String,
    pub role: ModelFileRole,
    pub size: u64,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct EditableMetadata {
    pub model_deb_name: String,
    pub model_display_name: String,
    pub capabilities: Vec<String>,
    pub model_supports_thinking: bool,
    pub model_supports_thinking_toggle: bool,
    pub model_max_token: u64,
    pub config_max_token: u64,
    pub supports_tool_choice: bool,
    pub parameter_count: u64,
}

impl Default for EditableMetadata {
    fn default() -> Self {
        Self {
            model_deb_name: String::new(),
            model_display_name: "Kylin Demo Model".to_string(),
            capabilities: vec!["text".to_string()],
            model_supports_thinking: false,
            model_supports_thinking_toggle: false,
            model_max_token: 4096,
            config_max_token: 4096,
            supports_tool_choice: false,
            parameter_count: 1_000_000_000,
        }
    }
}

impl EditableMetadata {
    pub fn validate(&self, kind: ModelKind) -> PackageResultValue<()> {
        if self.model_display_name.trim().is_empty() {
            return Err(PackageError::Validation("模型名字不能为空".to_string()));
        }
        if self.model_deb_name.trim().is_empty() {
            return Err(PackageError::Validation("模型管理名称不能为空".to_string()));
        }
        // 语音模型确认页不含能力/上下文/参数量字段，跳过这些校验
        if kind.is_voice() {
            return Ok(());
        }
        if self.capabilities.is_empty()
            || self.capabilities.iter().any(|capability| capability.trim().is_empty())
        {
            return Err(PackageError::Validation("模型能力不能为空".to_string()));
        }
        if self.model_max_token == 0 {
            return Err(PackageError::Validation(
                "模型最大上下文长度必须为正整数".to_string(),
            ));
        }
        if self.config_max_token == 0 {
            return Err(PackageError::Validation(
                "配置最大Token数必须为正整数".to_string(),
            ));
        }
        if self.parameter_count == 0 {
            return Err(PackageError::Validation("参数量必须为正整数".to_string()));
        }
        Ok(())
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PackageDraft {
    pub source: ModelSource,
    pub model_name: String,
    #[serde(default)]
    pub model_kind: ModelKind,
    #[serde(default)]
    pub asr_mode: Option<AsrMode>,
    pub brand: String,
    pub base_model: Option<ModelFile>,
    pub lora_models: Vec<ModelFile>,
    pub mmproj_model: Option<ModelFile>,
    pub metadata: EditableMetadata,
    pub output_dir: PathBuf,
    pub config_file: Option<PathBuf>,
    pub description_file: Option<PathBuf>,
    #[serde(default)]
    pub chat_template_file: Option<PathBuf>,
    /// 打包源目录（本地选择目录 / 云端暂存目录）：
    /// 存在时其中除被引用原始配置外的全部文件都会保留进模型包。
    #[serde(default)]
    pub source_dir: Option<PathBuf>,
}

#[derive(Debug, Clone, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct ModelConfigFiles {
    pub config_file: Option<PathBuf>,
    pub description_file: Option<PathBuf>,
    pub chat_template_file: Option<PathBuf>,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct PackageResult {
    pub package_root: PathBuf,
    pub generated_files: Vec<PathBuf>,
}

#[derive(Debug, Clone, Serialize)]
pub struct PackagingProgress {
    pub step: String,
    pub message: String,
    pub progress: u8,
}

pub struct PackagingState {
    pub cancel: std::sync::Arc<AtomicBool>,
}

impl PackagingState {
    pub fn new() -> Self {
        Self {
            cancel: std::sync::Arc::new(AtomicBool::new(false)),
        }
    }
}

impl Default for PackagingState {
    fn default() -> Self {
        Self::new()
    }
}

pub fn is_safe_output_dir(path: &Path) -> bool {
    let candidate = normalize_for_check(path);
    let allowed = ["/tmp", "/var/tmp", "/home", "/root"];
    if allowed.iter().any(|allowed_path| {
        let allowed_path = Path::new(allowed_path);
        candidate == allowed_path || candidate.starts_with(allowed_path)
    }) {
        return true;
    }

    let denied = [
        "/", "/bin", "/boot", "/dev", "/etc", "/lib", "/lib64", "/proc", "/run", "/sbin",
        "/sys", "/usr", "/var", "/opt",
    ];

    !denied.iter().any(|denied_path| {
        let denied_path = Path::new(denied_path);
        if denied_path == Path::new("/") {
            candidate == denied_path
        } else {
            candidate == denied_path || candidate.starts_with(denied_path)
        }
    })
}

fn normalize_for_check(path: &Path) -> PathBuf {
    fs::canonicalize(path).unwrap_or_else(|_| path.to_path_buf())
}

pub fn validate_model_name(name: &str, kind: ModelKind) -> PackageResultValue<()> {
    let prefix = kind.name_prefix();
    let parts = name.split('_').collect::<Vec<_>>();
    if parts.len() != 3 || parts[0] != prefix || parts.iter().any(|part| part.is_empty()) {
        return Err(PackageError::Validation(format!(
            "模型主目录名必须符合 {prefix}_<name>_<version-or-quant> 形式"
        )));
    }
    Ok(())
}

pub fn discover_model_files(paths: &[PathBuf]) -> PackageResultValue<Vec<ModelFile>> {
    let mut files = Vec::new();
    for path in paths {
        discover_path(path, &mut files)?;
    }
    files.sort_by(|left, right| left.file_name.cmp(&right.file_name));
    Ok(files)
}

pub fn discover_config_files(paths: &[PathBuf]) -> ModelConfigFiles {
    let mut found = ModelConfigFiles::default();
    for path in paths {
        let mut candidates: Vec<PathBuf> = Vec::new();
        if path.is_dir() {
            candidates.push(path.to_path_buf());
        } else if let Some(parent) = path.parent() {
            candidates.push(parent.to_path_buf());
            if let Some(grandparent) = parent.parent() {
                candidates.push(grandparent.to_path_buf());
            }
        }
        for dir in candidates {
            let config = dir.join("config.pbtxt");
            if found.config_file.is_none() && config.is_file() {
                found.config_file = Some(config);
            }
            let description = dir.join("model_description.yaml");
            if found.description_file.is_none() && description.is_file() {
                found.description_file = Some(description);
            }
            let chat_template = dir.join("chat_template.jinja");
            if found.chat_template_file.is_none() && chat_template.is_file() {
                found.chat_template_file = Some(chat_template);
            }
        }
        if found.config_file.is_some()
            && found.description_file.is_some()
            && found.chat_template_file.is_some()
        {
            break;
        }
    }
    found
}

fn discover_path(path: &Path, files: &mut Vec<ModelFile>) -> PackageResultValue<()> {
    let metadata = fs::metadata(path)?;
    if metadata.is_file() {
        if let Some(model_file) = classify_model_file(path) {
            files.push(model_file);
        }
        return Ok(());
    }

    if metadata.is_dir() {
        for entry in fs::read_dir(path)? {
            let entry = entry?;
            discover_path(&entry.path(), files)?;
        }
    }
    Ok(())
}

fn classify_model_file(path: &Path) -> Option<ModelFile> {
    let extension = path.extension()?.to_string_lossy().to_ascii_lowercase();
    if !matches!(extension.as_str(), "gguf" | "bin" | "safetensors") {
        return None;
    }

    let file_name = path.file_name()?.to_string_lossy().to_string();
    let lower = file_name.to_ascii_lowercase();
    let role = if lower.starts_with("mmproj") {
        ModelFileRole::Mmproj
    } else if file_name.starts_with("lora_") {
        ModelFileRole::Lora
    } else {
        ModelFileRole::Base
    };

    let size = std::fs::metadata(path).ok()?.len();

    Some(ModelFile {
        path: path.to_path_buf(),
        file_name,
        role,
        size,
    })
}

pub fn generate_package(
    draft: &PackageDraft,
    app: Option<&tauri::AppHandle>,
    cancel: Option<&AtomicBool>,
) -> PackageResultValue<PackageResult> {
    let result = generate_package_inner(draft, app, cancel);
    if matches!(result, Err(PackageError::Cancelled)) {
        cleanup_package_output(draft);
    }
    result
}

fn cleanup_package_output(draft: &PackageDraft) {
    let package_root = draft.output_dir.join(&draft.model_name);
    if let Err(err) = fs::remove_dir_all(&package_root) {
        if err.kind() != io::ErrorKind::NotFound {
            eprintln!("清理打包输出目录失败 {}: {err}", package_root.display());
        }
    }
}

fn generate_package_inner(
    draft: &PackageDraft,
    app: Option<&tauri::AppHandle>,
    cancel: Option<&AtomicBool>,
) -> PackageResultValue<PackageResult> {
    validate_draft(draft)?;
    emit_progress(app, "validate", "验证配置...", 5);

    let package_root = draft.output_dir.join(&draft.model_name);
    let model_dir = package_root.join("1");
    fs::create_dir_all(&model_dir)?;

    let mut generated_files = Vec::new();
    let base_model = draft
        .base_model
        .as_ref()
        .ok_or_else(|| PackageError::Validation("缺少基座模型文件".to_string()))?;

    check_cancel(cancel)?;
    if let Some(source_dir) = draft.source_dir.as_deref().filter(|dir| dir.is_dir()) {
        // 新复制规则：源目录全部文件保留进模型包（排除被引用的原始配置）
        generated_files.extend(copy_source_tree(source_dir, &package_root, draft, app, cancel)?);
    } else {
        copy_model_file(base_model, &model_dir, app, cancel, 5, 50, "复制基座模型")?;
        generated_files.push(model_dir.join(&base_model.file_name));

        let lora_count = draft.lora_models.len();
        for (index, lora) in draft.lora_models.iter().enumerate() {
            check_cancel(cancel)?;
            let total_range: u8 = 15;
            let start_pct = 50 + if lora_count > 0 {
                ((index as u64) * (total_range as u64) / (lora_count as u64)) as u8
            } else {
                0
            };
            let end_pct = 50 + if lora_count > 0 {
                (((index + 1) as u64) * (total_range as u64) / (lora_count as u64)) as u8
            } else {
                0
            };
            copy_model_file(
                lora,
                &model_dir,
                app,
                cancel,
                start_pct,
                end_pct,
                &format!("复制 Lora ({}/{})", index + 1, lora_count),
            )?;
            generated_files.push(model_dir.join(&lora.file_name));
        }

        if let Some(mmproj) = &draft.mmproj_model {
            check_cancel(cancel)?;
            let total_range: u8 = 5;
            copy_model_file(
                mmproj,
                &model_dir,
                app,
                cancel,
                65,
                65 + total_range,
                "复制多模态投射器",
            )?;
            generated_files.push(model_dir.join(&mmproj.file_name));
        }

        if let Some(chat_template) = &draft.chat_template_file {
            if chat_template.is_file() {
                check_cancel(cancel)?;
                emit_progress(app, "chat_template", "复制 chat_template.jinja...", 68);
                let chat_template_name = chat_template
                    .file_name()
                    .map(|name| name.to_string_lossy().to_string())
                    .unwrap_or_else(|| "chat_template.jinja".to_string());
                let target = model_dir.join(&chat_template_name);
                fs::copy(chat_template, &target)?;
                generated_files.push(target);
            }
        }
    }

    check_cancel(cancel)?;
    emit_progress(app, "config", "生成 config.pbtxt...", 70);
    let config_path = package_root.join("config.pbtxt");
    let config_content = match &draft.config_file {
        Some(src) if src.is_file() => {
            let original = fs::read_to_string(src)?;
            render_config_from_original(&original, draft, base_model)
        }
        _ => render_config_pbtxt(draft, base_model),
    };
    fs::write(&config_path, config_content)?;
    generated_files.push(config_path);

    check_cancel(cancel)?;
    emit_progress(app, "description", "生成 model_description.yaml...", 90);
    let description_path = package_root.join("model_description.yaml");
    let description_content = match &draft.description_file {
        Some(src) if src.is_file() => {
            let original = fs::read_to_string(src)?;
            render_description_from_original(&original, draft)
        }
        _ => render_model_description(draft),
    };
    fs::write(&description_path, description_content)?;
    generated_files.push(description_path);

    emit_progress(app, "done", "打包完成", 100);
    Ok(PackageResult {
        package_root,
        generated_files,
    })
}

fn check_cancel(cancel: Option<&AtomicBool>) -> PackageResultValue<()> {
    if let Some(flag) = cancel {
        if flag.load(Ordering::SeqCst) {
            return Err(PackageError::Cancelled);
        }
    }
    Ok(())
}

fn emit_progress(app: Option<&tauri::AppHandle>, step: &str, message: &str, progress: u8) {
    let Some(handle) = app else {
        return;
    };
    let payload = PackagingProgress {
        step: step.to_string(),
        message: message.to_string(),
        progress,
    };
    #[cfg(feature = "tauri-v1")]
    {
        use tauri::Manager;
        let _ = handle.emit_all("packaging-progress", payload);
    }
    #[cfg(feature = "tauri-v2")]
    {
        use tauri::Emitter;
        let _ = handle.emit("packaging-progress", payload);
    }
}

fn validate_draft(draft: &PackageDraft) -> PackageResultValue<()> {
    validate_model_name(&draft.model_name, draft.model_kind)?;
    draft.metadata.validate(draft.model_kind)?;
    if !is_safe_output_dir(&draft.output_dir) {
        return Err(PackageError::Validation(
            "不能选择系统目录作为保存位置".to_string(),
        ));
    }
    if draft.base_model.is_none() {
        return Err(PackageError::Validation("缺少基座模型文件".to_string()));
    }
    Ok(())
}

// 保留源目录全部文件进模型包：相对路径以 1/ 开头的原样映射到包 1/ 下，
// 其余自动前缀 1/；draft 引用的两份原始配置不复制（渲染后输出到包根目录）。
fn copy_source_tree(
    source_dir: &Path,
    package_root: &Path,
    draft: &PackageDraft,
    app: Option<&tauri::AppHandle>,
    cancel: Option<&AtomicBool>,
) -> PackageResultValue<Vec<PathBuf>> {
    let excluded = [draft.config_file.as_deref(), draft.description_file.as_deref()]
        .into_iter()
        .flatten()
        .map(|path| fs::canonicalize(path).unwrap_or_else(|_| path.to_path_buf()))
        .collect::<Vec<_>>();

    let mut entries: Vec<PathBuf> = Vec::new();
    collect_files(source_dir, &mut entries)?;
    entries.sort();

    let model_dir = package_root.join("1");
    let total = entries.len().max(1) as u64;
    let mut generated = Vec::new();
    for (index, file) in entries.iter().enumerate() {
        check_cancel(cancel)?;
        let canonical = fs::canonicalize(file).unwrap_or_else(|_| file.to_path_buf());
        if excluded.contains(&canonical) {
            continue;
        }
        let rel = file.strip_prefix(source_dir).unwrap_or(file);
        let rel_str = rel.to_string_lossy().replace('\\', "/");
        let target = match rel_str.strip_prefix("1/") {
            Some(rest) => model_dir.join(rest),
            None => model_dir.join(rel),
        };
        if let Some(parent) = target.parent() {
            fs::create_dir_all(parent)?;
        }
        let start_pct = 5 + ((index as u64) * 65 / total) as u8;
        let end_pct = (5 + (((index + 1) as u64) * 65 / total) as u8).max(start_pct);
        copy_file_streamed(
            file,
            &target,
            app,
            cancel,
            start_pct,
            end_pct,
            &format!("复制 {rel_str}"),
        )?;
        generated.push(target);
    }
    Ok(generated)
}

fn collect_files(dir: &Path, out: &mut Vec<PathBuf>) -> PackageResultValue<()> {
    for entry in fs::read_dir(dir)? {
        let path = entry?.path();
        if path.is_dir() {
            collect_files(&path, out)?;
        } else if path.is_file() {
            out.push(path);
        }
    }
    Ok(())
}

fn copy_model_file(
    file: &ModelFile,
    model_dir: &Path,
    app: Option<&tauri::AppHandle>,
    cancel: Option<&AtomicBool>,
    progress_start: u8,
    progress_end: u8,
    step_label: &str,
) -> PackageResultValue<()> {
    let target = model_dir.join(&file.file_name);
    copy_file_streamed(
        &file.path,
        &target,
        app,
        cancel,
        progress_start,
        progress_end,
        step_label,
    )
}

fn copy_file_streamed(
    source: &Path,
    target: &Path,
    app: Option<&tauri::AppHandle>,
    cancel: Option<&AtomicBool>,
    progress_start: u8,
    progress_end: u8,
    step_label: &str,
) -> PackageResultValue<()> {
    if target.exists() {
        if let (Ok(source_meta), Ok(target_meta)) = (fs::metadata(source), fs::metadata(target)) {
            if source_meta.len() == target_meta.len() {
                let target_name = target
                    .file_name()
                    .map(|name| name.to_string_lossy().to_string())
                    .unwrap_or_default();
                emit_progress(app, "skip", &format!("跳过已存在: {target_name}"), progress_end);
                return Ok(());
            }
        }
    }

    let source_size = fs::metadata(source)?.len();
    let mut reader = io::BufReader::new(fs::File::open(source)?);
    let mut writer = io::BufWriter::new(fs::File::create(&target)?);
    let mut buffer = vec![0u8; COPY_BUFFER_SIZE];
    let mut copied: u64 = 0;
    let mut last_emitted_pct: u8 = progress_start;

    loop {
        check_cancel(cancel)?;
        let bytes_read = reader.read(&mut buffer)?;
        if bytes_read == 0 {
            break;
        }
        writer.write_all(&buffer[..bytes_read])?;
        copied += bytes_read as u64;

        if source_size > 0 {
            let file_fraction = copied as f64 / source_size as f64;
            let range = (progress_end as f64) - (progress_start as f64);
            let current_pct = progress_start + (file_fraction * range) as u8;
            let current_pct = current_pct.min(progress_end);

            if current_pct > last_emitted_pct || copied >= source_size {
                last_emitted_pct = current_pct;
                let msg = format!(
                    "{} {}/{}",
                    step_label,
                    format_bytes_human(copied),
                    format_bytes_human(source_size)
                );
                emit_progress(app, "copy", &msg, current_pct);
            }
        }
    }

    writer.flush()?;
    Ok(())
}

fn format_bytes_human(bytes: u64) -> String {
    if bytes < 1024 {
        return format!("{} B", bytes);
    }
    let units = ["KB", "MB", "GB", "TB"];
    let mut value = bytes as f64 / 1024.0;
    let mut unit_index = 0;
    while value >= 1024.0 && unit_index < units.len() - 1 {
        value /= 1024.0;
        unit_index += 1;
    }
    format!("{:.1} {}", value, units[unit_index])
}

// chat_template_file 参数值：包内存在 chat_template.jinja 时返回其安装路径，否则为空。
// 优先取 draft.chat_template_file（本地发现/云端暂存路径）；未提供或文件不存在时
// 回退检查源目录根与 1/ 子目录（仓库式目录布局下 jinja 位于 1/，
// discover_config_files 以目录为入参时不会发现它，但 copy_source_tree 会将其复制进包）。
fn chat_template_install_path(draft: &PackageDraft) -> String {
    if let Some(file_name) = draft
        .chat_template_file
        .as_ref()
        .filter(|path| path.is_file())
        .and_then(|path| path.file_name())
    {
        return install_model_path(&draft.model_name, &file_name.to_string_lossy());
    }
    if let Some(source_dir) = draft.source_dir.as_deref() {
        for candidate in ["chat_template.jinja", "1/chat_template.jinja"] {
            if source_dir.join(candidate).is_file() {
                return install_model_path(&draft.model_name, "chat_template.jinja");
            }
        }
    }
    String::new()
}

fn template_parameters(draft: &PackageDraft, base_model: &ModelFile) -> Vec<(&'static str, String)> {
    let model_path = install_model_path(&draft.model_name, &base_model.file_name);
    let lora_paths = draft
        .lora_models
        .iter()
        .map(|file| install_model_path(&draft.model_name, &file.file_name))
        .collect::<Vec<_>>()
        .join(";");
    let mmproj_path = draft
        .mmproj_model
        .as_ref()
        .map(|file| install_model_path(&draft.model_name, &file.file_name))
        .unwrap_or_default();
    let chat_template_path = chat_template_install_path(draft);
    let decrypt_type = if base_model.file_name.ends_with(".gguf") {
        "0".to_string()
    } else {
        "1".to_string()
    };
    if draft.model_kind.is_voice() {
        // 语音模型参数按最新仓库样例（model-repository/asr_Qwen3-ASR-*）取值；
        // 固定参数随模型不同（样例 n_predict：0.6b=512、1.7b=1024），此处取值仅为
        // 无原始配置或原文缺失/值为空时的补充默认值，原文有值一律保留（见 render_config_from_original）
        return vec![
            ("n_threads", "4".to_string()),
            ("n_predict", "1024".to_string()),
            ("n_ctx", "2048".to_string()),
            ("n_batch", "512".to_string()),
            ("n_parallel", "1".to_string()),
            ("cont_batching", "true".to_string()),
            ("model", model_path),
            ("metrics_period_ms", "200".to_string()),
            ("lora", lora_paths),
            ("lora_init_without_apply", "false".to_string()),
            ("model_decrypt_type", decrypt_type),
            ("worker_save_path", String::new()),
            ("use_jinja", "true".to_string()),
            ("chat_template_file", chat_template_path),
            ("mmproj", mmproj_path),
            ("mmproj_use_gpu", "true".to_string()),
            ("llamacpp_verbose", "false".to_string()),
        ];
    }
    // LLM 固定参数：从零生成时的模板值；有原始配置时仅作为缺失/空值的补充默认值
    vec![
        ("n_threads", "1".to_string()),
        ("n_predict", "40960".to_string()),
        ("n_ctx", "128000".to_string()),
        ("n_batch", "512".to_string()),
        ("n_parallel", "1".to_string()),
        ("n_threads_http", "32".to_string()),
        ("cont_batching", "true".to_string()),
        ("model", model_path),
        ("metrics_period_ms", "200".to_string()),
        ("lora", lora_paths),
        ("lora_init_without_apply", "false".to_string()),
        ("model_decrypt_type", decrypt_type),
        ("worker_save_path", String::new()),
        ("use_jinja", "true".to_string()),
        ("chat_template_file", chat_template_path),
        ("mmproj", mmproj_path),
        ("mmproj_use_gpu", "true".to_string()),
        ("llamacpp_verbose", "false".to_string()),
    ]
}

fn render_parameters_block(key: &str, value: &str) -> String {
    format!("parameters: {{\n  key: \"{key}\"\n  value: {{ string_value: \"{value}\" }}\n}}")
}

pub fn render_config_pbtxt(draft: &PackageDraft, base_model: &ModelFile) -> String {
    if draft.model_kind.is_voice() {
        render_config_pbtxt_voice(draft, base_model)
    } else {
        render_config_pbtxt_llm(draft, base_model)
    }
}

// 语音模型 config.pbtxt 模板：按最新仓库样例（model-repository/asr_Qwen3-ASR-*）生成，
// 无 model_transaction_policy，输入张量 12 个（无 system_input，stop_input 为 optional）。
fn render_config_pbtxt_voice(draft: &PackageDraft, base_model: &ModelFile) -> String {
    let params = template_parameters(draft, base_model);
    let params_section = params
        .iter()
        .map(|(key, value)| render_parameters_block(key, value))
        .collect::<Vec<_>>()
        .join("\n");

    format!(
        r#"backend: "llamacpp"
input [
  {{
    name: "request_type"
    data_type: TYPE_INT32
    dims: [ -1 ]
  }},
  {{
    name: "request_id"
    data_type: TYPE_STRING
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "text_input"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }},
  {{
    name: "temperature_input"
    data_type: TYPE_FP32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "top_k_input"
    data_type: TYPE_INT32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "top_p_input"
    data_type: TYPE_FP32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "n_keep_input"
    data_type: TYPE_INT32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "n_predict_input"
    data_type: TYPE_INT32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "cache_prompt_input"
    data_type: TYPE_BOOL
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "stop_input"
    data_type: TYPE_STRING
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "stream_input"
    data_type: TYPE_BOOL
    dims: [ -1 ]
  }},
  {{
    name: "lora_scale_input"
    data_type: TYPE_FP32
    dims: [ -1 ]
    optional: true
  }}
]
output [
  {{
    name: "text_output"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }},
  {{
    name: "token_output"
    data_type: TYPE_INT32
    dims: [ -1 ]
  }},
  {{
    name: "lora_scale_output"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }},
  {{
    name: "metrics_output"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }}
]
instance_group [
  {{
    count: 1
    kind: KIND_CPU
  }}
]
{params_section}
"#
    )
}

fn render_config_pbtxt_llm(draft: &PackageDraft, base_model: &ModelFile) -> String {
    let params = template_parameters(draft, base_model);
    let params_section = params
        .iter()
        .map(|(key, value)| render_parameters_block(key, value))
        .collect::<Vec<_>>()
        .join("\n");

    format!(
        r#"backend: "llamacpp"
model_transaction_policy {{
  decoupled: True
}}
input [
  {{
    name: "request_type"
    data_type: TYPE_INT32
    dims: [ -1 ]
  }},
  {{
    name: "request_id"
    data_type: TYPE_STRING
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "text_input"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }},
  {{
    name: "system_input"
    data_type: TYPE_STRING
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "temperature_input"
    data_type: TYPE_FP32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "top_k_input"
    data_type: TYPE_INT32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "top_p_input"
    data_type: TYPE_FP32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "n_keep_input"
    data_type: TYPE_INT32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "n_predict_input"
    data_type: TYPE_INT32
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "cache_prompt_input"
    data_type: TYPE_BOOL
    dims: [ -1 ]
    optional: true
  }},
  {{
    name: "stop_input"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }},
  {{
    name: "stream_input"
    data_type: TYPE_BOOL
    dims: [ -1 ]
  }},
  {{
    name: "lora_scale_input"
    data_type: TYPE_FP32
    dims: [ -1 ]
    optional: true
  }}
]
output [
  {{
    name: "text_output"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }},
  {{
    name: "token_output"
    data_type: TYPE_INT32
    dims: [ -1 ]
  }},
  {{
    name: "lora_scale_output"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }},
  {{
    name: "metrics_output"
    data_type: TYPE_STRING
    dims: [ -1 ]
  }}
]
instance_group [
  {{
    count: 1
    kind: KIND_CPU
  }}
]
{params_section}
"#
    )
}

pub fn render_config_from_original(original: &str, draft: &PackageDraft, base_model: &ModelFile) -> String {
    let params = template_parameters(draft, base_model);
    let lines = original.lines().collect::<Vec<_>>();
    let mut output_lines: Vec<String> = Vec::new();
    let mut seen: std::collections::HashSet<String> = std::collections::HashSet::new();
    let mut index = 0;
    while index < lines.len() {
        let trimmed = lines[index].trim();
        let rest = trimmed.trim_start_matches("parameters").trim();
        if rest.starts_with('{') || rest.starts_with(':') {
            let mut end = index;
            let mut depth: isize = 0;
            let mut block: Vec<String> = Vec::new();
            while end < lines.len() {
                let line = lines[end];
                block.push(line.to_string());
                depth += line.matches('{').count() as isize;
                depth -= line.matches('}').count() as isize;
                end += 1;
                if depth <= 0 {
                    break;
                }
            }
            let key = block
                .iter()
                .find_map(|line| {
                    line.trim()
                        .strip_prefix("key:")
                        .map(|value| value.trim().trim_matches('"').to_string())
                });
            if let Some(key_name) = key {
                if let Some((_, value)) = params.iter().find(|(name, _)| *name == key_name) {
                    seen.insert(key_name.clone());
                    // 动态 key（安装路径相关）始终覆盖为草稿计算值；固定参数保留原文值，
                    // 仅当 string_value 为空时按模板默认补充（缺失的 key 在文件末尾统一追加）。
                    // 在块内定位含 string_value: 的行，兼容单行 value: { string_value: ... }
                    // 与多行块两种格式（最新仓库样例为多行格式）
                    let is_dynamic = matches!(
                        key_name.as_str(),
                        "model" | "lora" | "mmproj" | "model_decrypt_type"
                    );
                    if let Some(line) =
                        block.iter_mut().find(|line| line.contains("string_value:"))
                    {
                        if is_dynamic || extract_string_value(line).map_or(true, |v| v.is_empty()) {
                            *line = replace_string_value(line, value);
                        }
                    }
                }
            }
            output_lines.extend(block);
            index = end;
        } else {
            output_lines.push(lines[index].to_string());
            index += 1;
        }
    }
    for (name, value) in &params {
        if !seen.contains(*name) {
            output_lines.push(render_parameters_block(name, value));
        }
    }
    let mut result = output_lines.join("\n");
    if !result.ends_with('\n') {
        result.push('\n');
    }
    result
}

fn replace_string_value(line: &str, value: &str) -> String {
    let marker = "string_value:";
    let Some(index) = line.find(marker) else {
        return line.to_string();
    };
    let after = &line[index + marker.len()..];
    let Some(open) = after.find('"') else {
        return line.to_string();
    };
    let open_abs = index + marker.len() + open;
    let after_open = &line[open_abs + 1..];
    let Some(close) = after_open.find('"') else {
        return line.to_string();
    };
    let close_abs = open_abs + 1 + close;
    let mut result = String::from(&line[..open_abs + 1]);
    result.push_str(value);
    result.push_str(&line[close_abs..]);
    result
}

// 提取行内 string_value: "..." 的当前值，用于"值为空按缺失补默认"的判定
fn extract_string_value(line: &str) -> Option<String> {
    let marker = "string_value:";
    let index = line.find(marker)?;
    let after = &line[index + marker.len()..];
    let open = after.find('"')?;
    let rest = &after[open + 1..];
    let close = rest.find('"')?;
    Some(rest[..close].to_string())
}

// 语音模型 model_capability 的补充默认值（无原始配置、或原文缺失/值为空时使用）：
// asr 按确认页识别方式映射 streaming_asr/once_asr，tts 固定 tts；
// 原文已有非空值（含 once_asr/asr/tts 等样例变体）一律保留，不做映射覆盖。
fn voice_capability_value(draft: &PackageDraft) -> &'static str {
    match draft.model_kind {
        ModelKind::Asr => match draft.asr_mode {
            Some(AsrMode::Streaming) => "streaming_asr",
            _ => "once_asr",
        },
        ModelKind::Tts => "tts",
        ModelKind::Llm => "llm",
    }
}

pub fn render_description_from_original(original: &str, draft: &PackageDraft) -> String {
    let metadata = &draft.metadata;
    let voice = draft.model_kind.is_voice();
    let capabilities = json_string_array(&metadata.capabilities);
    let lora_info = if draft.lora_models.is_empty() {
        String::new()
    } else {
        ",\\\"lora\\\":[\\\"GET_LABEL\\\",\\\"INTENT\\\"]".to_string()
    };
    let others = format!(
        r#"{{\"modelMaxToken\":{model_max_token},\"configMaxToken\":{config_max_token},\"supportsToolChoice\":{supports_tool_choice},\"parameterCount\":{parameter_count},\"capabilities\":{capabilities}{lora_info}}}"#,
        model_max_token = metadata.model_max_token,
        config_max_token = metadata.config_max_token,
        supports_tool_choice = metadata.supports_tool_choice,
        parameter_count = metadata.parameter_count,
        capabilities = capabilities,
        lora_info = lora_info,
    );

    let thinking_line = |key: &str, value: bool| -> String {
        if voice {
            format!("{key}: {value}")
        } else {
            format!("{key}: \"{value}\"")
        }
    };

    // 受管字段：始终以草稿/表单值覆盖，原文缺失时在末尾追加。
    // brand 不再取 draft.brand，改为补充类字段（原文保留，缺失/空补 "qwen"）
    let mut managed: Vec<(&str, String)> = vec![
        (
            "model_deb_name",
            format!("model_deb_name: \"{}\"", yaml_escape(&metadata.model_deb_name)),
        ),
        (
            "model_name",
            format!("model_name: \"{}\"", yaml_escape(&draft.model_name)),
        ),
        (
            "model_display_name",
            format!(
                "model_display_name: \"{}\"",
                yaml_escape(&metadata.model_display_name)
            ),
        ),
        (
            "model_is3rdModel",
            "model_is3rdModel: \"true\"".to_string(),
        ),
        (
            "model_supports_thinking",
            thinking_line("model_supports_thinking", metadata.model_supports_thinking),
        ),
        (
            "model_supports_thinking_toggle",
            thinking_line(
                "model_supports_thinking_toggle",
                metadata.model_supports_thinking_toggle,
            ),
        ),
    ];
    // LLM 的 others_info 用表单值覆盖；语音走补充规则（默认带 isBuiltin JSON）
    if !voice {
        managed.push(("others_info", format!("others_info: \"{others}\"")));
    }

    // 补充字段：原文有非空值保留（含行尾注释）；缺失或值为空按最新仓库样例默认值补充。
    // 语音的能力名保留原文变体（once_asr/asr 等）；顶层 isBuiltin、model_is_private
    // 已从最新样例移除：不输出、不追加，原文存在时作为非受管字段原样保留
    let mut supplements: Vec<(&str, String)> = Vec::new();
    if voice {
        supplements.push((
            "model_capability",
            format!("model_capability: [\"{}\"]", voice_capability_value(draft)),
        ));
        supplements.push(("model_internal", "model_internal: \"false\"".to_string()));
    }
    supplements.push(("brand", "brand: \"qwen\"".to_string()));
    if voice {
        supplements.push((
            "others_info",
            r#"others_info: "{\"isBuiltin\":false}""#.to_string(),
        ));
    }

    let mut output = String::new();
    let mut seen: std::collections::HashSet<&str> = std::collections::HashSet::new();
    let mut supplemented: std::collections::HashSet<&str> = std::collections::HashSet::new();
    for line in original.lines() {
        let trimmed = line.trim();
        let mut handled = false;
        for (key, new_line) in &managed {
            if let Some(rest) = trimmed.strip_prefix(key) {
                if rest.trim_start().starts_with(':') {
                    output.push_str(new_line);
                    output.push('\n');
                    seen.insert(key);
                    handled = true;
                    break;
                }
            }
        }
        if handled {
            continue;
        }
        for (key, new_line) in &supplements {
            if let Some(rest) = trimmed.strip_prefix(*key) {
                if rest.trim_start().starts_with(':') {
                    supplemented.insert(key);
                    // 值为空按缺失处理：整行替换为默认值
                    if yaml_value_is_empty(trimmed, key) {
                        output.push_str(new_line);
                    } else {
                        output.push_str(line);
                    }
                    output.push('\n');
                    handled = true;
                    break;
                }
            }
        }
        if !handled {
            output.push_str(line);
            output.push('\n');
        }
    }
    for (key, new_line) in &managed {
        if !seen.contains(key) {
            output.push_str(new_line);
            output.push('\n');
        }
    }
    for (key, new_line) in &supplements {
        if !supplemented.contains(key) {
            output.push_str(new_line);
            output.push('\n');
        }
    }
    output
}

// 判断 YAML 行 `key: <值>` 的值是否为空（无值、"" 或 ''），用于"值为空按缺失补默认"
fn yaml_value_is_empty(trimmed_line: &str, key: &str) -> bool {
    let Some(rest) = trimmed_line.strip_prefix(key) else {
        return false;
    };
    let Some(rest) = rest.trim_start().strip_prefix(':') else {
        return false;
    };
    let value = rest.trim();
    let unquoted = if value.len() >= 2
        && ((value.starts_with('"') && value.ends_with('"'))
            || (value.starts_with('\'') && value.ends_with('\'')))
    {
        &value[1..value.len() - 1]
    } else {
        value
    };
    unquoted.trim().is_empty()
}

pub fn render_model_description(draft: &PackageDraft) -> String {
    if draft.model_kind.is_voice() {
        // 语音模型无原始配置时按最新仓库样例（model-repository/asr_Qwen3-ASR-*）结构生成：
        // brand 固定 "qwen"，others_info 为带 isBuiltin 的 JSON；
        // 顶层 isBuiltin、model_is_private 已从最新样例移除，不再输出
        return format!(
            r#"model_deb_name: "{deb}"
model_name: "{name}"
model_display_name: "{display}"
model_capability: ["{capability}"]
model_is3rdModel: "true"
model_internal: "false"
brand: "qwen"
model_supports_thinking: false
model_supports_thinking_toggle: false
others_info: "{{\"isBuiltin\":false}}"
"#,
            deb = yaml_escape(&draft.metadata.model_deb_name),
            name = yaml_escape(&draft.model_name),
            display = yaml_escape(&draft.metadata.model_display_name),
            capability = voice_capability_value(draft),
        );
    }
    let metadata = &draft.metadata;
    let capabilities = json_string_array(&metadata.capabilities);
    let lora_info = if draft.lora_models.is_empty() {
        String::new()
    } else {
        ",\\\"lora\\\":[\\\"GET_LABEL\\\",\\\"INTENT\\\"]".to_string()
    };
    format!(
        r#"model_deb_name: "{model_deb_name}"
model_name: "{model_name}"
model_display_name: "{model_display_name}"
model_capability: ["llm"]
model_is3rdModel: "true"
model_internal: "false"
brand: "qwen"
model_supports_thinking: "{thinking}"
model_supports_thinking_toggle: "{thinking_toggle}"
model_is_private: "true"
others_info: "{{\"modelMaxToken\":{model_max_token},\"configMaxToken\":{config_max_token},\"supportsToolChoice\":{supports_tool_choice},\"parameterCount\":{parameter_count},\"capabilities\":{capabilities}{lora_info}}}"
"#,
        model_deb_name = yaml_escape(&metadata.model_deb_name),
        model_name = yaml_escape(&draft.model_name),
        model_display_name = yaml_escape(&metadata.model_display_name),
        thinking = metadata.model_supports_thinking,
        thinking_toggle = metadata.model_supports_thinking_toggle,
        model_max_token = metadata.model_max_token,
        config_max_token = metadata.config_max_token,
        supports_tool_choice = metadata.supports_tool_choice,
        parameter_count = metadata.parameter_count,
        capabilities = capabilities,
        lora_info = lora_info,
    )
}

fn install_model_path(model_name: &str, file_name: &str) -> String {
    format!("{INSTALL_ROOT}/{model_name}/1/{file_name}")
}

fn yaml_escape(value: &str) -> String {
    value.replace('\\', "\\\\").replace('"', "\\\"")
}

fn json_string_array(values: &[String]) -> String {
    let values = values
        .iter()
        .map(|value| format!("\\\"{}\\\"", yaml_escape(value)))
        .collect::<Vec<_>>();
    format!("[{}]", values.join(","))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn metadata() -> EditableMetadata {
        EditableMetadata {
            model_deb_name: "ai-kylin-demo-model".to_string(),
            model_display_name: "Demo".to_string(),
            capabilities: vec!["text".to_string(), "tool".to_string()],
            model_supports_thinking: true,
            model_supports_thinking_toggle: false,
            model_max_token: 32768,
            config_max_token: 4096,
            supports_tool_choice: true,
            parameter_count: 9_000_000_000,
        }
    }

    #[test]
    fn validates_metadata() {
        assert!(metadata().validate(ModelKind::Llm).is_ok());
        let mut invalid = metadata();
        invalid.model_max_token = 0;
        assert!(invalid.validate(ModelKind::Llm).is_err());
    }

    #[test]
    fn rejects_system_dirs() {
        assert!(!is_safe_output_dir(Path::new("/")));
        assert!(!is_safe_output_dir(Path::new("/opt/appdata")));
        assert!(is_safe_output_dir(Path::new("/tmp/kylin-model-package")));
    }

    #[test]
    fn validates_model_names() {
        assert!(validate_model_name("llm_demo_0.1", ModelKind::Llm).is_ok());
        assert!(validate_model_name("demo_0.1", ModelKind::Llm).is_err());
        assert!(validate_model_name("asr_Qwen3-ASR-0.6b-1_1.0", ModelKind::Asr).is_ok());
        assert!(validate_model_name("llm_demo_0.1", ModelKind::Asr).is_err());
        assert!(validate_model_name("tts_demo_1.0", ModelKind::Tts).is_ok());
    }

    #[test]
    fn renders_metadata_templates() {
        let draft = PackageDraft {
            source: ModelSource::Local,
            model_name: "llm_demo_0.1".to_string(),
            brand: "Kylin".to_string(),
            base_model: None,
            lora_models: vec![ModelFile {
                path: PathBuf::from("/tmp/lora_modelA_0.1.gguf"),
                file_name: "lora_modelA_0.1.gguf".to_string(),
                role: ModelFileRole::Lora,
                size: 0,
            }],
            mmproj_model: None,
            metadata: metadata(),
            output_dir: PathBuf::from("/tmp"),
            config_file: None,
            description_file: None,
            chat_template_file: None,
            model_kind: ModelKind::Llm,
            asr_mode: None,
            source_dir: None,
        };
        let base = ModelFile {
            path: PathBuf::from("/tmp/base_model_0.1.gguf"),
            file_name: "base_model_0.1.gguf".to_string(),
            role: ModelFileRole::Base,
            size: 0,
        };
        let config = render_config_pbtxt(&draft, &base);
        assert!(config.contains("/opt/appdata/kylin-ai/model-repository/llm_demo_0.1/1/base_model_0.1.gguf"));
        assert!(config.contains("model_decrypt_type"));
        assert!(config.contains("string_value: \"0\""));
        assert!(config.contains("system_input"));
        assert!(config.contains("n_keep_input"));
        assert!(config.contains("n_predict_input"));
        assert!(config.contains("cache_prompt_input"));
        assert!(config.contains("n_threads_http"));
        assert!(config.contains("worker_save_path"));
        assert!(config.contains("use_jinja"));
        assert!(config.contains("chat_template_file"));
        assert!(config.contains("mmproj_use_gpu"));
        assert!(config.contains("string_value: \"40960\""));
        assert!(config.contains("string_value: \"128000\""));
        // LLM 模板 stop_input 仍为必填（无 optional）
        assert!(config.contains(
            "name: \"stop_input\"\n    data_type: TYPE_STRING\n    dims: [ -1 ]\n  },"
        ));


        let description = render_model_description(&draft);
        assert!(description.contains("model_deb_name: \"ai-kylin-demo-model\""));
        assert!(description.contains("model_capability: [\"llm\"]"));
        // brand 不再取 draft.brand，从零生成固定补 "qwen"
        assert!(description.contains("brand: \"qwen\""));
        assert!(!description.contains("Kylin"));
        assert!(description.contains(r#"\"modelMaxToken\":32768"#));
        assert!(description.contains(r#"\"configMaxToken\":4096"#));
        assert!(description.contains(r#"\"supportsToolChoice\":true"#));
        assert!(description.contains(r#"\"capabilities\":[\"text\",\"tool\"]"#));
        assert!(description.contains(r#"\"lora\":[\"GET_LABEL\",\"INTENT\"]"#));
        assert!(!description.contains("isBuiltin"));
    }

    #[test]
    fn renders_config_from_original_keeps_custom_fields() {
        let draft = PackageDraft {
            source: ModelSource::Local,
            model_name: "llm_demo_0.1".to_string(),
            brand: "Kylin".to_string(),
            base_model: None,
            lora_models: vec![ModelFile {
                path: PathBuf::from("/tmp/lora_modelA_0.1.gguf"),
                file_name: "lora_modelA_0.1.gguf".to_string(),
                role: ModelFileRole::Lora,
                size: 0,
            }],
            mmproj_model: None,
            metadata: metadata(),
            output_dir: PathBuf::from("/tmp"),
            config_file: None,
            description_file: None,
            chat_template_file: None,
            model_kind: ModelKind::Llm,
            asr_mode: None,
            source_dir: None,
        };
        let base = ModelFile {
            path: PathBuf::from("/tmp/base_model_0.1.gguf"),
            file_name: "base_model_0.1.gguf".to_string(),
            role: ModelFileRole::Base,
            size: 0,
        };
        let original = r#"backend: "llamacpp"
instance_group [
  {
    count: 1
    kind: KIND_CPU
  }
]
parameters: {
  key: "n_threads"
  value: { string_value: "16" }
}
parameters: {
  key: "use_jinja"
  value: { string_value: "" }
}
parameters: {
  key: "model"
  value: { string_value: "/old/path/old.gguf" }
}
parameters: {
  key: "custom_param"
  value: { string_value: "keep-me" }
}
"#;
        let rendered = render_config_from_original(original, &draft, &base);
        assert!(rendered.contains("custom_param"));
        assert!(rendered.contains("keep-me"));
        // LLM 固定参数同样保留原文值；值为空按缺失补模板默认
        assert!(rendered.contains("key: \"n_threads\"\n  value: { string_value: \"16\" }"));
        assert!(rendered.contains("key: \"use_jinja\"\n  value: { string_value: \"true\" }"));
        assert!(rendered.contains(
            "/opt/appdata/kylin-ai/model-repository/llm_demo_0.1/1/base_model_0.1.gguf"
        ));
        assert!(!rendered.contains("/old/path/old.gguf"));
        assert!(rendered.contains("key: \"lora\""));
        assert!(rendered.contains("key: \"n_ctx\""));
        assert!(rendered.contains("string_value: \"128000\""));
        assert!(rendered.contains("model_decrypt_type"));
        assert!(rendered.contains("string_value: \"0\""));
    }

    #[test]
    fn renders_description_from_original_updates_managed_fields() {
        let mut meta = metadata();
        meta.model_display_name = "New Display".to_string();
        meta.model_supports_thinking = true;
        let draft = PackageDraft {
            source: ModelSource::Cloud,
            model_name: "llm_qwen_1.5".to_string(),
            brand: "Qwen".to_string(),
            base_model: None,
            lora_models: vec![],
            mmproj_model: None,
            metadata: meta,
            output_dir: PathBuf::from("/tmp"),
            config_file: None,
            description_file: None,
            chat_template_file: None,
            model_kind: ModelKind::Llm,
            asr_mode: None,
            source_dir: None,
        };
        let original = r#"model_deb_name: "old"
model_name: "old_name"
model_display_name: "Old Display"
model_capability: ["llm", "image"]
model_is3rdModel: "false"
model_is_private: "true"
brand: "OldBrand"
model_supports_thinking: "false"
model_supports_thinking_toggle: "true"
model_internal: "false"
others_info: "{\"modelMaxToken\":100,\"capabilities\":[\"text\"]}"
"#;
        let rendered = render_description_from_original(&original, &draft);
        assert!(rendered.contains("model_capability: [\"llm\", \"image\"]"));
        assert!(rendered.contains("model_is3rdModel: \"true\""));
        assert!(rendered.contains("model_deb_name: \"ai-kylin-demo-model\""));
        assert!(rendered.contains("model_name: \"llm_qwen_1.5\""));
        assert!(rendered.contains("model_display_name: \"New Display\""));
        // brand 改为补充类字段：原文有值保留，不再被 draft.brand 覆盖
        assert!(rendered.contains("brand: \"OldBrand\""));
        assert!(!rendered.contains("brand: \"Qwen\""));
        assert!(rendered.contains("model_supports_thinking: \"true\""));
        assert!(rendered.contains("model_supports_thinking_toggle: \"false\""));
        assert!(rendered.contains(r#"\"modelMaxToken\":32768"#));
        assert!(rendered.contains(r#"\"configMaxToken\":4096"#));
        assert!(rendered.contains(r#"\"supportsToolChoice\":true"#));
        assert!(!rendered.contains("Old Display"));
        assert!(!rendered.contains("old_name"));
        assert!(!rendered.contains("isBuiltin"));
    }

    fn voice_draft(kind: ModelKind, asr_mode: Option<AsrMode>) -> PackageDraft {
        PackageDraft {
            source: ModelSource::Local,
            model_name: "asr_demo_1.0".to_string(),
            model_kind: kind,
            asr_mode,
            brand: "Kylin".to_string(),
            base_model: None,
            lora_models: vec![],
            mmproj_model: None,
            metadata: metadata(),
            output_dir: PathBuf::from("/tmp"),
            config_file: None,
            description_file: None,
            chat_template_file: None,
            source_dir: None,
        }
    }

    #[test]
    fn renders_voice_config_template_from_sample() {
        let base = ModelFile {
            path: PathBuf::from("/tmp/HiModel_a.gguf"),
            file_name: "HiModel_a.gguf".to_string(),
            role: ModelFileRole::Base,
            size: 0,
        };
        let config = render_config_pbtxt(&voice_draft(ModelKind::Asr, Some(AsrMode::Once)), &base);
        assert!(!config.contains("system_input"));
        assert!(!config.contains("model_transaction_policy"));
        assert!(config.contains("string_value: \"1024\""));
        assert!(config.contains("string_value: \"2048\""));
        assert!(config.contains("stream_input"));
        assert!(config.contains("n_threads_http") == false);
        // 最新样例：语音 stop_input 为 optional
        assert!(config.contains(
            "name: \"stop_input\"\n    data_type: TYPE_STRING\n    dims: [ -1 ]\n    optional: true"
        ));
    }

    #[test]
    fn voice_config_from_original_preserves_fixed_params() {
        let base = ModelFile {
            path: PathBuf::from("/tmp/HiModel_a.gguf"),
            file_name: "HiModel_a.gguf".to_string(),
            role: ModelFileRole::Base,
            size: 0,
        };
        // 最新样例为多行块格式：string_value 独立成行
        let original = "backend: \"llamacpp\"\nparameters: {\n  key: \"n_threads\"\n  value: {\n    string_value: \"8\"\n  }\n}\nparameters: {\n  key: \"n_predict\"\n  value: {\n    string_value: \"\"\n  }\n}\nparameters: {\n  key: \"model\"\n  value: {\n    string_value: \"/old/path/old.gguf\"\n  }\n}\nparameters: {\n  key: \"custom_param\"\n  value: { string_value: \"keep-me\" }\n}\n";
        let rendered = render_config_from_original(
            original,
            &voice_draft(ModelKind::Asr, Some(AsrMode::Once)),
            &base,
        );
        // 固定参数保留原文值（样例 n_predict 随模型不同：0.6b=512、1.7b=1024）
        assert!(rendered.contains("string_value: \"8\""));
        // 值为空按缺失补模板默认
        assert!(rendered.contains("key: \"n_predict\"\n  value: {\n    string_value: \"1024\""));
        // 动态 key 覆盖为新安装路径（多行块同样生效）
        assert!(rendered.contains(
            "string_value: \"/opt/appdata/kylin-ai/model-repository/asr_demo_1.0/1/HiModel_a.gguf\""
        ));
        assert!(!rendered.contains("/old/path/old.gguf"));
        // 非受管参数保留
        assert!(rendered.contains("custom_param"));
        assert!(rendered.contains("keep-me"));
        // 缺失的模板参数按语音默认补充，不追加 LLM 专有参数
        assert!(rendered.contains("key: \"n_ctx\""));
        assert!(rendered.contains("string_value: \"2048\""));
        assert!(rendered.contains("key: \"mmproj\""));
        assert!(!rendered.contains("n_threads_http"));
    }

    #[test]
    fn renders_voice_description_template() {
        let once = render_model_description(&voice_draft(ModelKind::Asr, Some(AsrMode::Once)));
        assert!(once.contains("model_capability: [\"once_asr\"]"));
        assert!(once.contains("model_is3rdModel: \"true\""));
        assert!(once.contains("model_internal: \"false\""));
        assert!(once.contains("brand: \"qwen\""));
        assert!(once.contains("model_supports_thinking: false"));
        assert!(once.contains(r#"others_info: "{\"isBuiltin\":false}""#));
        assert!(once.lines().any(|line| line == r#"others_info: "{\"isBuiltin\":false}""#));
        // 最新样例已移除顶层 isBuiltin 与 model_is_private
        assert!(!once.contains("model_is_private"));
        assert!(!once.lines().any(|line| line.trim_start().starts_with("isBuiltin:")));

        let streaming =
            render_model_description(&voice_draft(ModelKind::Asr, Some(AsrMode::Streaming)));
        assert!(streaming.contains("model_capability: [\"streaming_asr\"]"));

        let tts = render_model_description(&voice_draft(ModelKind::Tts, None));
        assert!(tts.contains("model_capability: [\"tts\"]"));
    }

    #[test]
    fn voice_description_from_original_preserves_and_supplements() {
        let original = "model_deb_name: \"ai-kylin-qwen3-asr-0.6b-1-model\"\nmodel_name: asr_old_1.0\nmodel_capability: [\"asr\"] #engine名\nmodel_is3rdModel: \"true\"\nmodel_internal: \"false\"\nbrand: \"qwen\"\nmodel_supports_thinking: false\nothers_info: ''\nisBuiltin: true\n";
        let rendered = render_description_from_original(
            original,
            &voice_draft(ModelKind::Asr, Some(AsrMode::Once)),
        );
        // 补充类字段原文有值保留（含行尾注释），能力名不做映射覆盖
        assert!(rendered.contains("model_capability: [\"asr\"] #engine名"));
        assert!(rendered.contains("model_internal: \"false\""));
        assert!(rendered.contains("brand: \"qwen\""));
        // others_info 值为空按缺失补最新样例默认
        assert!(rendered.lines().any(|line| line == r#"others_info: "{\"isBuiltin\":false}""#));
        assert!(!rendered.contains("others_info: ''"));
        // 受管字段仍按草稿覆盖
        assert!(rendered.contains("model_name: \"asr_demo_1.0\""));
        assert!(rendered.contains("model_deb_name: \"ai-kylin-demo-model\""));
        assert!(rendered.contains("model_is3rdModel: \"true\""));
        // 顶层 isBuiltin 不再受管：原文值原样保留，缺失也不追加
        assert!(rendered.contains("isBuiltin: true"));
        assert!(!rendered.contains("model_is_private"));

        let tts_rendered =
            render_description_from_original(original, &voice_draft(ModelKind::Tts, None));
        assert!(tts_rendered.contains("model_capability: [\"asr\"]"));
    }

    #[test]
    fn description_supplements_missing_or_empty_fields() {
        let draft = voice_draft(ModelKind::Asr, Some(AsrMode::Once));
        let rendered =
            render_description_from_original("model_name: asr_old_1.0\nbrand: \"\"\n", &draft);
        // brand 空值按缺失补 "qwen"，其余缺失字段按样例默认追加
        assert!(rendered.contains("brand: \"qwen\""));
        assert!(rendered.contains("model_capability: [\"once_asr\"]"));
        assert!(rendered.contains("model_internal: \"false\""));
        assert!(rendered.lines().any(|line| line == r#"others_info: "{\"isBuiltin\":false}""#));
        assert!(!rendered.lines().any(|line| line.trim_start().starts_with("isBuiltin:")));
        assert!(!rendered.contains("model_is_private"));

        // LLM：brand 缺失同样补 "qwen"
        let llm = voice_draft(ModelKind::Llm, None);
        let llm_rendered = render_description_from_original("model_name: \"llm_old_0.1\"\n", &llm);
        assert!(llm_rendered.contains("brand: \"qwen\""));
        assert!(llm_rendered.contains("model_is3rdModel: \"true\""));
    }

    #[test]
    fn chat_template_file_param_filled_with_install_path() {
        let dir = tempfile::tempdir().unwrap();
        std::fs::write(dir.path().join("chat_template.jinja"), "{{ prompt }}").unwrap();
        let base = ModelFile {
            path: PathBuf::from("/tmp/base_model_0.1.gguf"),
            file_name: "base_model_0.1.gguf".to_string(),
            role: ModelFileRole::Base,
            size: 0,
        };
        let mut draft = PackageDraft {
            source: ModelSource::Local,
            model_name: "llm_demo_0.1".to_string(),
            brand: "Kylin".to_string(),
            base_model: None,
            lora_models: vec![],
            mmproj_model: None,
            metadata: metadata(),
            output_dir: PathBuf::from("/tmp"),
            config_file: None,
            description_file: None,
            chat_template_file: Some(dir.path().join("chat_template.jinja")),
            model_kind: ModelKind::Llm,
            asr_mode: None,
            source_dir: None,
        };
        let expected = "/opt/appdata/kylin-ai/model-repository/llm_demo_0.1/1/chat_template.jinja";

        // 从零生成：chat_template_file 输出 jinja 安装路径
        let config = render_config_pbtxt(&draft, &base);
        assert!(config.contains(&format!(
            "key: \"chat_template_file\"\n  value: {{ string_value: \"{expected}\" }}"
        )));

        // 原始配置该 key 值为空：按缺失补安装路径
        let original = "backend: \"llamacpp\"\nparameters: {\n  key: \"chat_template_file\"\n  value: { string_value: \"\" }\n}\n";
        let rendered = render_config_from_original(original, &draft, &base);
        assert!(rendered.contains(expected));

        // 原始配置已有非空值：固定参数保留原文
        let original_custom = "backend: \"llamacpp\"\nparameters: {\n  key: \"chat_template_file\"\n  value: { string_value: \"custom.jinja\" }\n}\n";
        let rendered_custom = render_config_from_original(original_custom, &draft, &base);
        assert!(rendered_custom.contains("custom.jinja"));
        assert!(!rendered_custom.contains(expected));

        // draft 未提供 jinja 路径且无源目录：值为空
        draft.chat_template_file = None;
        let config_empty = render_config_pbtxt(&draft, &base);
        assert!(config_empty.contains(
            "key: \"chat_template_file\"\n  value: { string_value: \"\" }"
        ));

        // 仓库式源目录（jinja 位于 1/ 下）：回退发现并补安装路径
        let repo_dir = tempfile::tempdir().unwrap();
        std::fs::create_dir_all(repo_dir.path().join("1")).unwrap();
        std::fs::write(repo_dir.path().join("1/chat_template.jinja"), "{{ prompt }}").unwrap();
        draft.source_dir = Some(repo_dir.path().to_path_buf());
        let config_repo = render_config_pbtxt(&draft, &base);
        assert!(config_repo.contains(expected));
    }

    #[test]
    fn discovers_config_files_from_directory() {
        let dir = tempfile::tempdir().unwrap();
        std::fs::write(dir.path().join("config.pbtxt"), "backend").unwrap();
        std::fs::write(dir.path().join("model_description.yaml"), "model_name").unwrap();
        std::fs::write(dir.path().join("chat_template.jinja"), "{{ prompt }}").unwrap();
        std::fs::write(dir.path().join("model.gguf"), "weights").unwrap();

        let found = discover_config_files(&[dir.path().join("model.gguf")]);
        assert_eq!(found.config_file, Some(dir.path().join("config.pbtxt")));
        assert_eq!(found.description_file, Some(dir.path().join("model_description.yaml")));
        assert_eq!(
            found.chat_template_file,
            Some(dir.path().join("chat_template.jinja"))
        );
    }
}
