// Linux 原生模态文件对话框。
//
// rfd（tauri-plugin-dialog 与 Tauri 1 dialog 的底层实现）的 GTK3 后端完全忽略 set_parent，
// 既不设置 WM_TRANSIENT_FOR 也不设置 modal，还会另起一个独立 GTK 线程运行对话框，
// 因此对话框表现为与主窗口同级的独立窗口。这里改为在主线程直接创建 GtkFileChooserDialog，
// 通过 set_transient_for + set_modal 使其成为主窗口的真模态窗口（自动居中并置顶于主窗口，
// 且对话框关闭前主窗口不接收输入）。

pub struct PickerRequest {
    pub title: &'static str,
    pub accept_label: &'static str,
    pub folder: bool,
    pub multiple: bool,
    pub filters: &'static [(&'static str, &'static [&'static str])],
}

pub const MODEL_FILE_FILTERS: &[(&str, &[&str])] = &[
    ("模型文件", &["gguf", "bin", "safetensors"]),
    ("所有文件", &["*"]),
];

#[cfg(target_os = "linux")]
use gtk::prelude::*;
#[cfg(target_os = "linux")]
use tokio::sync::oneshot;

#[cfg(all(target_os = "linux", feature = "tauri-v2"))]
fn main_gtk_window(app: &tauri::AppHandle) -> Option<gtk::ApplicationWindow> {
    use tauri::Manager;
    app.get_webview_window("main")
        .and_then(|window| window.gtk_window().ok())
}

#[cfg(all(target_os = "linux", feature = "tauri-v1"))]
fn main_gtk_window(app: &tauri::AppHandle) -> Option<gtk::ApplicationWindow> {
    use tauri::Manager;
    app.get_window("main")
        .and_then(|window| window.gtk_window().ok())
}

// gtk 0.15（Tauri 1）的 add_filter 接收 &FileFilter，gtk 0.18（Tauri 2）接收 FileFilter。
#[cfg(all(target_os = "linux", feature = "tauri-v1"))]
fn add_filter(chooser: &gtk::FileChooserWidget, filter: &gtk::FileFilter) {
    chooser.add_filter(filter);
}

#[cfg(all(target_os = "linux", feature = "tauri-v2"))]
fn add_filter(chooser: &gtk::FileChooserWidget, filter: &gtk::FileFilter) {
    chooser.add_filter(filter.clone());
}

#[cfg(target_os = "linux")]
fn build_filter(name: &str, patterns: &[&str]) -> gtk::FileFilter {
    let filter = gtk::FileFilter::new();
    filter.set_name(Some(name));
    for pattern in patterns {
        if *pattern == "*" {
            filter.add_pattern("*");
        } else {
            filter.add_pattern(&format!("*.{}", pattern));
        }
    }
    filter
}

#[cfg(target_os = "linux")]
const DIALOG_WIDTH: i32 = 960;
#[cfg(target_os = "linux")]
const DIALOG_HEIGHT: i32 = 680;

#[cfg(target_os = "linux")]
pub async fn pick(app: tauri::AppHandle, request: PickerRequest) -> Result<Vec<String>, String> {
    let (tx, rx) = oneshot::channel::<Vec<String>>();
    let handle = app.clone();
    app.run_on_main_thread(move || {
        let parent = main_gtk_window(&handle);
        let action = if request.folder {
            gtk::FileChooserAction::SelectFolder
        } else {
            gtk::FileChooserAction::Open
        };
        // 用 GtkDialog + GtkFileChooserWidget 而不是 GtkFileChooserDialog：后者会读取并回写
        // 全局 org.gtk.Settings.FileChooser 的 window-size，对话框会以上次记录的尺寸打开
        // （可能接近全屏），还会改写其它 GTK 应用的对话框尺寸。
        let mut builder = gtk::Dialog::builder()
            .title(request.title)
            .modal(true)
            .use_header_bar(1);
        if let Some(parent) = parent.as_ref() {
            builder = builder.transient_for(parent);
        }
        let dialog = builder.build();
        dialog.set_default_size(DIALOG_WIDTH, DIALOG_HEIGHT);

        let chooser = gtk::FileChooserWidget::new(action);
        chooser.set_select_multiple(request.multiple);
        let filters: Vec<gtk::FileFilter> = request
            .filters
            .iter()
            .map(|(name, patterns)| build_filter(name, patterns))
            .collect();
        for filter in &filters {
            add_filter(&chooser, filter);
        }
        if let Some(filter) = filters.first() {
            chooser.set_filter(filter);
        }
        dialog.content_area().pack_start(&chooser, true, true, 0);

        dialog.add_button("取消", gtk::ResponseType::Cancel);
        let accept = dialog.add_button(request.accept_label, gtk::ResponseType::Accept);
        accept.style_context().add_class("suggested-action");
        accept.grab_default();
        dialog.show_all();

        let paths = if dialog.run() == gtk::ResponseType::Accept {
            chooser
                .filenames()
                .into_iter()
                .map(|path| path.to_string_lossy().to_string())
                .collect()
        } else {
            Vec::new()
        };
        unsafe { dialog.destroy() };
        let _ = tx.send(paths);
    })
    .map_err(|err| err.to_string())?;

    rx.await.map_err(|err| err.to_string())
}

#[cfg(not(target_os = "linux"))]
pub async fn pick(_app: tauri::AppHandle, _request: PickerRequest) -> Result<Vec<String>, String> {
    Err("当前平台不支持文件选择对话框".to_string())
}
