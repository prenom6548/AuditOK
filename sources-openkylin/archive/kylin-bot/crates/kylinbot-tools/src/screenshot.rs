use async_trait::async_trait;
use serde_json::json;
use std::fmt::Write;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::time::Duration;
use kylinbot_api::tool::{Tool, ToolResult};
use kylinbot_config::policy::SecurityPolicy;

/// Maximum time to wait for a screenshot command to complete.
const SCREENSHOT_TIMEOUT_SECS: u64 = 15;
/// After kylin-screenshot exits, DBus may still be writing the file asynchronously.
const KYLIN_OUTPUT_WAIT_MS: u64 = 3_000;
const KYLIN_OUTPUT_POLL_MS: u64 = 100;
/// Maximum base64 payload size to return (2 MB of base64 ≈ 1.5 MB image).
const MAX_BASE64_BYTES: usize = 2_097_152;

/// Characters unsafe in screenshot filenames and path components.
const SHELL_UNSAFE: &[char] = &[
    '\'', '"', '`', '$', '\\', ';', '|', '&', '\n', '\0', '(', ')',
];

/// Tool for capturing screenshots using platform-native commands.
///
/// macOS: `screencapture`
/// Linux: `kylin-screenshot full` (Kylin desktop screenshot utility)
pub struct ScreenshotTool {
    security: Arc<SecurityPolicy>,
}

impl ScreenshotTool {
    pub fn new(security: Arc<SecurityPolicy>) -> Self {
        Self { security }
    }

    fn path_component_unsafe(value: &str) -> bool {
        value.contains(SHELL_UNSAFE)
    }

    /// Build argv for `kylin-screenshot full`.
    ///
    /// `-N` sets the basename (no extension); `-T` must include the dot (e.g. `.png`),
    /// matching `org.ukui.screenshot` / gsettings conventions in kylin-screenshot.
    fn kylin_screenshot_args(parent: &str, stem: &str) -> Vec<String> {
        vec![
            "kylin-screenshot".into(),
            "full".into(),
            "-p".into(),
            parent.into(),
            "-N".into(),
            stem.into(),
            "-T".into(),
            ".png".into(),
        ]
    }

    async fn wait_for_output_file(path: &Path) -> bool {
        let deadline =
            tokio::time::Instant::now() + Duration::from_millis(KYLIN_OUTPUT_WAIT_MS);
        while tokio::time::Instant::now() < deadline {
            if tokio::fs::metadata(path).await.is_ok() {
                return true;
            }
            tokio::time::sleep(Duration::from_millis(KYLIN_OUTPUT_POLL_MS)).await;
        }
        tokio::fs::metadata(path).await.is_ok()
    }

    /// Determine the screenshot command for the current platform.
    fn screenshot_command(output_path: &Path) -> Option<Vec<String>> {
        if cfg!(target_os = "macos") {
            Some(vec![
                "screencapture".into(),
                "-x".into(), // no sound
                output_path.to_string_lossy().into_owned(),
            ])
        } else if cfg!(target_os = "linux") {
            which::which("kylin-screenshot").ok()?;
            let parent = output_path.parent()?;
            let stem = output_path.file_stem()?;
            let parent_str = parent.to_string_lossy();
            let stem_str = stem.to_string_lossy();
            if Self::path_component_unsafe(&parent_str) || Self::path_component_unsafe(&stem_str) {
                return None;
            }
            Some(Self::kylin_screenshot_args(
                parent_str.as_ref(),
                stem_str.as_ref(),
            ))
        } else {
            None
        }
    }

    fn is_kylin_command(program: &str) -> bool {
        program == "kylin-screenshot"
            || program.ends_with("/kylin-screenshot")
    }

    fn is_kylin_linux() -> bool {
        cfg!(target_os = "linux") && which::which("kylin-screenshot").is_ok()
    }

    /// Execute the screenshot capture and return the result.
    async fn capture(&self, args: serde_json::Value) -> anyhow::Result<ToolResult> {
        let timestamp = chrono::Utc::now().format("%Y%m%d_%H%M%S");
        let filename = args
            .get("filename")
            .and_then(|v| v.as_str())
            .map_or_else(|| format!("screenshot_{timestamp}.png"), String::from);

        // Sanitize filename to prevent path traversal
        let safe_name = PathBuf::from(&filename).file_name().map_or_else(
            || format!("screenshot_{timestamp}.png"),
            |n| n.to_string_lossy().to_string(),
        );

        if Self::path_component_unsafe(&safe_name) {
            return Ok(ToolResult {
                success: false,
                output: String::new(),
                error: Some("Filename contains characters unsafe for shell execution".into()),
            });
        }

        let output_path = self.security.workspace_dir.join(&safe_name);
        if Self::is_kylin_linux() {
            if let Err(e) = tokio::fs::create_dir_all(&self.security.workspace_dir).await {
                return Ok(ToolResult {
                    success: false,
                    output: String::new(),
                    error: Some(format!(
                        "Failed to create screenshot workspace directory: {e}"
                    )),
                });
            }
        }

        let Some(mut cmd_args) = Self::screenshot_command(&output_path) else {
            let error = if cfg!(target_os = "linux") {
                if which::which("kylin-screenshot").is_err() {
                    "kylin-screenshot not found in PATH. Install Kylin Screenshot.".into()
                } else {
                    "Invalid screenshot output path".into()
                }
            } else {
                "Screenshot not supported on this platform".into()
            };
            return Ok(ToolResult {
                success: false,
                output: String::new(),
                error: Some(error),
            });
        };

        // macOS region flags
        if cfg!(target_os = "macos")
            && let Some(region) = args.get("region").and_then(|v| v.as_str())
        {
            match region {
                "selection" => cmd_args.insert(1, "-s".into()),
                "window" => cmd_args.insert(1, "-w".into()),
                _ => {} // ignore unknown regions
            }
        }

        let program = cmd_args.remove(0);
        let mut command = tokio::process::Command::new(&program);
        command.args(&cmd_args);
        if Self::is_kylin_command(&program) {
            command.env("QT_WAYLAND_USE_WLR_SCREENCOPY", "1");
        }

        let result = tokio::time::timeout(
            Duration::from_secs(SCREENSHOT_TIMEOUT_SECS),
            command.output(),
        )
        .await;

        match result {
            Ok(Ok(output)) => {
                if !output.status.success() {
                    let stderr = String::from_utf8_lossy(&output.stderr);
                    let error = if Self::is_kylin_command(&program) {
                        format!(
                            "Screenshot failed (kylin-screenshot): {stderr}. \
                             Ensure DISPLAY/WAYLAND and desktop session are available."
                        )
                    } else {
                        format!("Screenshot command failed: {stderr}")
                    };
                    return Ok(ToolResult {
                        success: false,
                        output: String::new(),
                        error: Some(error),
                    });
                }

                let file_ready = if Self::is_kylin_command(&program) {
                    Self::wait_for_output_file(&output_path).await
                } else {
                    tokio::fs::metadata(&output_path).await.is_ok()
                };
                if !file_ready {
                    let error = if Self::is_kylin_command(&program) {
                        "Screenshot failed (kylin-screenshot): output file was not created. \
                         Ensure org.dharkael.kylinscreenshot is running, WAYLAND/X11 session is \
                         available, and QT_WAYLAND_USE_WLR_SCREENCOPY=1 on Wayland."
                            .into()
                    } else {
                        format!(
                            "Screenshot command succeeded but file not found: {}",
                            output_path.display()
                        )
                    };
                    return Ok(ToolResult {
                        success: false,
                        output: String::new(),
                        error: Some(error),
                    });
                }

                Self::read_and_encode(&output_path).await
            }
            Ok(Err(e)) => Ok(ToolResult {
                success: false,
                output: String::new(),
                error: Some(format!("Failed to execute screenshot command: {e}")),
            }),
            Err(_) => Ok(ToolResult {
                success: false,
                output: String::new(),
                error: Some(format!(
                    "Screenshot timed out after {SCREENSHOT_TIMEOUT_SECS}s"
                )),
            }),
        }
    }

    /// Read the screenshot file and return base64-encoded result.
    async fn read_and_encode(output_path: &Path) -> anyhow::Result<ToolResult> {
        // Check file size before reading to prevent OOM on large screenshots
        const MAX_RAW_BYTES: u64 = 1_572_864; // ~1.5 MB (base64 expands ~33%)
        if let Ok(meta) = tokio::fs::metadata(output_path).await
            && meta.len() > MAX_RAW_BYTES
        {
            return Ok(ToolResult {
                success: true,
                output: format!(
                    "Screenshot saved to: {}\nSize: {} bytes (too large to base64-encode inline)",
                    output_path.display(),
                    meta.len(),
                ),
                error: None,
            });
        }

        match tokio::fs::read(output_path).await {
            Ok(bytes) => {
                use base64::Engine;
                let size = bytes.len();
                let mut encoded = base64::engine::general_purpose::STANDARD.encode(&bytes);
                let truncated = if encoded.len() > MAX_BASE64_BYTES {
                    let mut boundary = MAX_BASE64_BYTES.min(encoded.len());
                    while boundary > 0 && !encoded.is_char_boundary(boundary) {
                        boundary -= 1;
                    }
                    encoded.truncate(boundary);
                    true
                } else {
                    false
                };

                let mut output_msg = format!(
                    "Screenshot saved to: {}\nSize: {size} bytes\nBase64 length: {}",
                    output_path.display(),
                    encoded.len(),
                );
                if truncated {
                    output_msg.push_str(" (truncated)");
                }
                let mime = match output_path.extension().and_then(|e| e.to_str()) {
                    Some("jpg" | "jpeg") => "image/jpeg",
                    Some("bmp") => "image/bmp",
                    Some("gif") => "image/gif",
                    Some("webp") => "image/webp",
                    _ => "image/png",
                };
                let _ = write!(output_msg, "\ndata:{mime};base64,{encoded}");

                Ok(ToolResult {
                    success: true,
                    output: output_msg,
                    error: None,
                })
            }
            Err(e) => Ok(ToolResult {
                success: false,
                output: format!("Screenshot saved to: {}", output_path.display()),
                error: Some(format!("Failed to read screenshot file: {e}")),
            }),
        }
    }
}

#[async_trait]
impl Tool for ScreenshotTool {
    fn name(&self) -> &str {
        "screenshot"
    }

    fn description(&self) -> &str {
        "Capture a screenshot of the current screen. Returns the file path and base64-encoded PNG data."
    }

    fn parameters_schema(&self) -> serde_json::Value {
        json!({
            "type": "object",
            "properties": {
                "filename": {
                    "type": "string",
                    "description": "Optional filename (default: screenshot_<timestamp>.png). Saved in workspace."
                },
                "region": {
                    "type": "string",
                    "description": "Optional region for macOS: 'selection' for interactive crop, 'window' for front window. Ignored on Linux."
                }
            }
        })
    }

    async fn execute(&self, args: serde_json::Value) -> anyhow::Result<ToolResult> {
        if !self.security.can_act() {
            return Ok(ToolResult {
                success: false,
                output: String::new(),
                error: Some("Action blocked: autonomy is read-only".into()),
            });
        }
        self.capture(args).await
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use kylinbot_config::autonomy::AutonomyLevel;
    use kylinbot_config::policy::SecurityPolicy;

    fn test_security() -> Arc<SecurityPolicy> {
        Arc::new(SecurityPolicy {
            autonomy: AutonomyLevel::Full,
            workspace_dir: std::env::temp_dir(),
            ..SecurityPolicy::default()
        })
    }

    #[test]
    fn screenshot_tool_name() {
        let tool = ScreenshotTool::new(test_security());
        assert_eq!(tool.name(), "screenshot");
    }

    #[test]
    fn screenshot_tool_description() {
        let tool = ScreenshotTool::new(test_security());
        assert!(!tool.description().is_empty());
        assert!(tool.description().contains("screenshot"));
    }

    #[test]
    fn screenshot_tool_schema() {
        let tool = ScreenshotTool::new(test_security());
        let schema = tool.parameters_schema();
        assert!(schema["properties"]["filename"].is_object());
        assert!(schema["properties"]["region"].is_object());
    }

    #[test]
    fn screenshot_tool_spec() {
        let tool = ScreenshotTool::new(test_security());
        let spec = tool.spec();
        assert_eq!(spec.name, "screenshot");
        assert!(spec.parameters.is_object());
    }

    #[test]
    fn kylin_screenshot_args_format() {
        let cmd = ScreenshotTool::kylin_screenshot_args("/tmp", "my_screenshot");
        assert_eq!(cmd[0], "kylin-screenshot");
        assert_eq!(cmd[1], "full");
        assert_eq!(cmd[2], "-p");
        assert_eq!(cmd[3], "/tmp");
        assert_eq!(cmd[4], "-N");
        assert_eq!(cmd[5], "my_screenshot");
        assert_eq!(cmd[6], "-T");
        assert_eq!(cmd[7], ".png");
    }

    #[test]
    #[cfg(target_os = "linux")]
    fn linux_screenshot_command_builds_kylin_argv() {
        if which::which("kylin-screenshot").is_err() {
            return;
        }
        let cmd = ScreenshotTool::screenshot_command(Path::new("/tmp/foo.png")).unwrap();
        assert_eq!(cmd[0], "kylin-screenshot");
        assert_eq!(cmd[1], "full");
        assert!(cmd.contains(&"-p".to_string()));
        assert!(cmd.contains(&"/tmp".to_string()));
        assert!(cmd.contains(&"-N".to_string()));
        assert!(cmd.contains(&"foo".to_string()));
    }

    #[test]
    #[cfg(target_os = "macos")]
    fn macos_screenshot_command_exists() {
        let cmd = ScreenshotTool::screenshot_command(Path::new("/tmp/test.png"));
        assert!(cmd.is_some());
        let args = cmd.unwrap();
        assert_eq!(args[0], "screencapture");
    }

    #[test]
    #[cfg(target_os = "linux")]
    fn linux_screenshot_command_missing_kylin() {
        if which::which("kylin-screenshot").is_ok() {
            return;
        }
        assert!(ScreenshotTool::screenshot_command(Path::new("/tmp/test.png")).is_none());
    }

    #[tokio::test]
    async fn screenshot_rejects_shell_injection_filename() {
        let tool = ScreenshotTool::new(test_security());
        let result = tool
            .execute(json!({"filename": "test'injection.png"}))
            .await
            .unwrap();
        assert!(!result.success);
        assert!(result.error.unwrap().contains("unsafe for shell execution"));
    }

    #[test]
    #[cfg(target_os = "linux")]
    fn linux_screenshot_command_contains_output_components() {
        if which::which("kylin-screenshot").is_err() {
            return;
        }
        let cmd = ScreenshotTool::screenshot_command(Path::new("/tmp/my_screenshot.png")).unwrap();
        let joined = cmd.join(" ");
        assert!(joined.contains("/tmp"));
        assert!(joined.contains("my_screenshot"));
    }

    #[test]
    #[cfg(target_os = "macos")]
    fn macos_screenshot_command_contains_output_path() {
        let cmd = ScreenshotTool::screenshot_command(Path::new("/tmp/my_screenshot.png")).unwrap();
        let joined = cmd.join(" ");
        assert!(joined.contains("/tmp/my_screenshot.png"));
    }
}
