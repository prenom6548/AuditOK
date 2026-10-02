use model_management_tool::cloud::{
    self, CloudDownloadState, CloudModel, CloudModelDescription, DownloadRequest,
};
use model_management_tool::packager::{
    discover_config_files, discover_model_files, generate_package, is_safe_output_dir,
    EditableMetadata, ModelConfigFiles, ModelFile, ModelKind, PackageDraft, PackagingState,
};
use std::sync::atomic::Ordering;

mod dialogs;

#[tauri::command]
fn get_download_directory() -> Result<String, String> {
    dirs::download_dir()
        .map(|p| p.to_string_lossy().to_string())
        .ok_or_else(|| "无法获取下载目录".to_string())
}

#[tauri::command]
fn select_local_model_files(paths: Vec<String>) -> Result<Vec<ModelFile>, String> {
    let path_refs = paths.iter().map(std::path::PathBuf::from).collect::<Vec<_>>();
    discover_model_files(&path_refs).map_err(|err| err.to_string())
}

#[tauri::command]
fn select_local_model_directory(path: String) -> Result<Vec<ModelFile>, String> {
    discover_model_files(&[std::path::PathBuf::from(path)]).map_err(|err| err.to_string())
}

#[tauri::command]
fn inspect_local_path(path: String) -> Result<bool, String> {
    std::fs::metadata(&path)
        .map(|meta| meta.is_dir())
        .map_err(|err| err.to_string())
}

#[tauri::command]
fn find_model_config_files(paths: Vec<String>) -> Result<ModelConfigFiles, String> {
    let path_refs = paths.iter().map(std::path::PathBuf::from).collect::<Vec<_>>();
    Ok(discover_config_files(&path_refs))
}

#[tauri::command]
fn read_model_description_file(path: String) -> Result<CloudModelDescription, String> {
    cloud::read_model_description_file(&path).map_err(|err| err.to_string())
}

#[tauri::command]
async fn select_local_files_dialog(app: tauri::AppHandle) -> Result<Vec<String>, String> {
    dialogs::pick(
        app,
        dialogs::PickerRequest {
            title: "选择模型文件",
            accept_label: "打开",
            folder: false,
            multiple: true,
            filters: dialogs::MODEL_FILE_FILTERS,
        },
    )
    .await
}

#[tauri::command]
async fn select_local_model_directory_dialog(
    app: tauri::AppHandle,
) -> Result<Option<String>, String> {
    let paths = dialogs::pick(
        app,
        dialogs::PickerRequest {
            title: "选择模型目录",
            accept_label: "选择",
            folder: true,
            multiple: false,
            filters: &[],
        },
    )
    .await?;
    Ok(paths.into_iter().next())
}

#[tauri::command]
async fn select_output_directory(app: tauri::AppHandle) -> Result<Option<String>, String> {
    let paths = dialogs::pick(
        app,
        dialogs::PickerRequest {
            title: "选择输出位置",
            accept_label: "选择",
            folder: true,
            multiple: false,
            filters: &[],
        },
    )
    .await?;
    Ok(paths.into_iter().next())
}

#[tauri::command]
fn open_directory(path: String) -> Result<(), String> {
    #[cfg(target_os = "linux")]
    let cmd = "xdg-open";
    #[cfg(target_os = "macos")]
    let cmd = "open";
    #[cfg(target_os = "windows")]
    let cmd = "explorer";

    std::process::Command::new(cmd)
        .arg(&path)
        .spawn()
        .map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
fn open_kylin_os_manager() -> Result<(), String> {
    std::process::Command::new("kylin-os-manager")
        .arg("--jumpTab=DeviceManager")
        .arg("--pluginMessage=DriverManager")
        .spawn()
        .map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
fn open_ai_module_manager() -> Result<(), String> {
    std::process::Command::new("ukui-control-center")
        .arg("-m")
        .arg("aisubsystem")
        .spawn()
        .map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
fn validate_output_directory(path: String) -> Result<(), String> {
    let path = std::path::PathBuf::from(path);
    if is_safe_output_dir(&path) {
        Ok(())
    } else {
        Err("不能选择系统目录作为保存位置".to_string())
    }
}

#[tauri::command]
async fn list_cloud_models() -> Result<Vec<CloudModel>, String> {
    cloud::list_cloud_models()
        .await
        .map_err(|err| err.to_string())
}

#[tauri::command]
async fn fetch_model_description(sub_path: String) -> Result<CloudModelDescription, String> {
    cloud::fetch_model_description(&sub_path)
        .await
        .map_err(|err| err.to_string())
}

#[tauri::command]
async fn download_cloud_model(
    app: tauri::AppHandle,
    state: tauri::State<'_, CloudDownloadState>,
    request: DownloadRequest,
) -> Result<Vec<ModelFile>, String> {
    let cancel = state.cancel.clone();
    let active = state.active.clone();
    cancel.store(false, Ordering::SeqCst);
    active.store(true, Ordering::SeqCst);
    let result = cloud::download_cloud_model(app, request, Some(&cancel)).await;
    active.store(false, Ordering::SeqCst);
    result.map_err(|err| err.to_string())
}

#[tauri::command]
fn cancel_cloud_download(state: tauri::State<'_, CloudDownloadState>) -> Result<(), String> {
    state.cancel.store(true, Ordering::SeqCst);
    Ok(())
}

#[tauri::command]
async fn clean_cloud_staging(
    state: tauri::State<'_, CloudDownloadState>,
    staging_dir: String,
) -> Result<(), String> {
    let cancel = state.cancel.clone();
    let active = state.active.clone();
    drop(state);
    // 先置取消标志，并等待进行中的下载任务退出（最长 10s），避免边删边写
    cancel.store(true, Ordering::SeqCst);
    for _ in 0..100 {
        if !active.load(Ordering::SeqCst) {
            break;
        }
        tokio::time::sleep(std::time::Duration::from_millis(100)).await;
    }
    cloud::clean_staging_dir(std::path::Path::new(&staging_dir)).map_err(|err| err.to_string())
}

#[tauri::command]
async fn create_package(
    app: tauri::AppHandle,
    state: tauri::State<'_, PackagingState>,
    draft: PackageDraft,
) -> Result<model_management_tool::packager::PackageResult, String> {
    let cancel = state.cancel.clone();
    cancel.store(false, Ordering::SeqCst);
    let app_clone = app.clone();
    tokio::task::spawn_blocking(move || {
        generate_package(&draft, Some(&app_clone), Some(&cancel)).map_err(|err| err.to_string())
    })
    .await
    .map_err(|e| e.to_string())?
}

#[tauri::command]
fn cancel_packaging(state: tauri::State<'_, PackagingState>) -> Result<(), String> {
    state.cancel.store(true, Ordering::SeqCst);
    Ok(())
}

#[tauri::command]
fn validate_metadata(metadata: EditableMetadata, kind: ModelKind) -> Result<(), String> {
    metadata.validate(kind).map_err(|err| err.to_string())
}

fn main() {
    // Tauri 1（麒麟 V10）：没有官方 single-instance 插件，使用底层 single-instance crate。
    // 若已有实例在运行，则直接退出，避免同时打开多个窗口。
    #[cfg(feature = "tauri-v1")]
    let _single_instance_guard = {
        // 单实例标识，与 tauri.conf.json 中的 identifier 保持一致。
        const SINGLE_INSTANCE_ID: &str = "org.openkylin.model-management-tool";
        let instance = single_instance::SingleInstance::new(SINGLE_INSTANCE_ID)
            .expect("failed to create single instance guard");
        if !instance.is_single() {
            std::process::exit(0);
        }
        instance
    };

    #[allow(unused_mut)]
    let mut builder = tauri::Builder::default();
    #[cfg(feature = "tauri-v2")]
    {
        use tauri::Manager;
        // single-instance 插件必须最先注册；再次启动时聚焦已有窗口。
        builder = builder.plugin(tauri_plugin_single_instance::init(|app, _args, _cwd| {
            if let Some(win) = app.get_webview_window("main") {
                let _ = win.unminimize();
                let _ = win.show();
                let _ = win.set_focus();
            }
        }));
    }
    builder
        .setup(|_app| {
            // 启动兜底清理：上次异常退出（强杀/断电/崩溃）残留在下载目录下的 .cloud-staging
            if let Some(dir) = dirs::download_dir() {
                let _ = cloud::clean_staging_root(&dir);
            }
            Ok(())
        })
        .manage(CloudDownloadState::new())
        .manage(PackagingState::new())
        .invoke_handler(tauri::generate_handler![
            select_local_model_files,
            select_local_model_directory,
            inspect_local_path,
            find_model_config_files,
            read_model_description_file,
            select_local_files_dialog,
            select_local_model_directory_dialog,
            select_output_directory,
            open_directory,
            open_kylin_os_manager,
            open_ai_module_manager,
            validate_output_directory,
            get_download_directory,
            list_cloud_models,
            fetch_model_description,
            download_cloud_model,
            cancel_cloud_download,
            clean_cloud_staging,
            create_package,
            cancel_packaging,
            validate_metadata
        ])
        .run(tauri::generate_context!())
        .expect("failed to run model management tool");
}
