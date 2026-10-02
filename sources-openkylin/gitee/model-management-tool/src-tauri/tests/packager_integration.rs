use model_management_tool::packager::{
    discover_model_files, generate_package, AsrMode, EditableMetadata, ModelFileRole, ModelKind,
    ModelSource, PackageDraft, PackageError,
};
use std::fs;
use std::sync::atomic::AtomicBool;

fn metadata() -> EditableMetadata {
    EditableMetadata {
        model_deb_name: "ai-kylin-demo-model".to_string(),
        model_display_name: "Demo".to_string(),
        capabilities: vec!["text".to_string()],
        model_supports_thinking: false,
        model_supports_thinking_toggle: false,
        model_max_token: 4096,
        config_max_token: 4096,
        supports_tool_choice: false,
        parameter_count: 1_000_000_000,
    }
}

#[test]
fn discovers_directory_and_generates_package() {
    let source = tempfile::tempdir().expect("source tempdir");
    let output = tempfile::tempdir().expect("output tempdir");
    let base = source.path().join("base_model_0.1.gguf");
    let lora = source.path().join("lora_modelA_0.1.gguf");
    let ignored = source.path().join("notes.txt");
    fs::write(&base, "base").expect("base file");
    fs::write(&lora, "lora").expect("lora file");
    fs::write(&ignored, "ignored").expect("ignored file");

    let files = discover_model_files(&[source.path().to_path_buf()]).expect("discover files");
    assert_eq!(files.len(), 2);

    let base_model = files
        .iter()
        .find(|file| file.role == ModelFileRole::Base)
        .cloned()
        .expect("base model");
    let lora_models = files
        .iter()
        .filter(|file| file.role == ModelFileRole::Lora)
        .cloned()
        .collect::<Vec<_>>();

    let mmproj_models = files
        .iter()
        .filter(|file| file.role == ModelFileRole::Mmproj)
        .cloned()
        .collect::<Vec<_>>();

    let draft = PackageDraft {
        source: ModelSource::Local,
        model_name: "llm_demo_0.1".to_string(),
        brand: "Kylin".to_string(),
        base_model: Some(base_model),
        lora_models,
        mmproj_model: mmproj_models.into_iter().next(),
        metadata: metadata(),
        output_dir: output.path().to_path_buf(),
        config_file: None,
        description_file: None,
        chat_template_file: None,
        model_kind: ModelKind::Llm,
        asr_mode: None,
        source_dir: None,
    };

    let result = generate_package(&draft, None, None).expect("package generated");
    assert!(result.package_root.join("1/base_model_0.1.gguf").exists());
    assert!(result.package_root.join("1/lora_modelA_0.1.gguf").exists());
    assert!(result.package_root.join("config.pbtxt").exists());
    assert!(result.package_root.join("model_description.yaml").exists());

    let config = fs::read_to_string(result.package_root.join("config.pbtxt")).expect("config");
    assert!(config.contains("/opt/appdata/kylin-ai/model-repository/llm_demo_0.1/1/base_model_0.1.gguf"));
    assert!(config.contains("system_input"));
    assert!(config.contains("n_threads_http"));
    assert!(config.contains("mmproj_use_gpu"));
    assert!(config.contains("string_value: \"40960\""));
    assert!(config.contains("string_value: \"128000\""));

    let desc = fs::read_to_string(result.package_root.join("model_description.yaml")).expect("desc");
    assert!(desc.contains("model_deb_name:"));
    assert!(desc.contains("configMaxToken"));
    assert!(desc.contains(r#",\"lora\":[\"GET_LABEL\",\"INTENT\"]"#));
}

#[test]
fn uses_original_config_files_when_provided() {
    let source = tempfile::tempdir().expect("source tempdir");
    let output = tempfile::tempdir().expect("output tempdir");
    let base = source.path().join("base_model_0.1.gguf");
    fs::write(&base, "base").expect("base file");

    let original_config = r#"backend: "llamacpp"
input [
  {
    name: "request_type"
    data_type: TYPE_INT32
    dims: [ -1 ]
  }
]
parameters: {
  key: "model"
  value: { string_value: "/custom/install/path/model.gguf" }
}
parameters: {
  key: "vendor_custom"
  value: { string_value: "original" }
}
"#;
    let original_desc = r#"model_deb_name: "orig"
model_name: "orig_model"
model_display_name: "Original Display"
model_capability: ["llm"]
model_is3rdModel: "false"
model_internal: "false"
brand: "OrigBrand"
model_supports_thinking: "false"
model_supports_thinking_toggle: "false"
model_is_private: "true"
others_info: "{\"modelMaxToken\":8192,\"configMaxToken\":2048,\"supportsToolChoice\":\"false\",\"parameterCount\":1000000000,\"capabilities\":[\"text\"]}"
"#;
    fs::write(source.path().join("config.pbtxt"), original_config).expect("config");
    fs::write(source.path().join("model_description.yaml"), original_desc).expect("desc");
    fs::write(source.path().join("chat_template.jinja"), "{{ prompt }}").expect("chat template");

    let base_model = discover_model_files(&[source.path().to_path_buf()])
        .expect("discover files")
        .into_iter()
        .find(|file| file.role == ModelFileRole::Base)
        .expect("base model");

    let draft = PackageDraft {
        source: ModelSource::Local,
        model_name: "llm_orig_0.1".to_string(),
        brand: "Kylin".to_string(),
        base_model: Some(base_model),
        lora_models: vec![],
        mmproj_model: None,
        metadata: metadata(),
        output_dir: output.path().to_path_buf(),
        config_file: Some(source.path().join("config.pbtxt")),
        description_file: Some(source.path().join("model_description.yaml")),
        chat_template_file: Some(source.path().join("chat_template.jinja")),
        model_kind: ModelKind::Llm,
        asr_mode: None,
        source_dir: None,
    };

    let result = generate_package(&draft, None, None).expect("package generated");

    let chat_template = fs::read_to_string(result.package_root.join("1/chat_template.jinja"))
        .expect("chat template copied next to model files");
    assert_eq!(chat_template, "{{ prompt }}");

    let config = fs::read_to_string(result.package_root.join("config.pbtxt")).expect("config");
    assert!(config.contains("vendor_custom"), "custom parameters kept");
    assert!(config.contains("original"), "custom parameter value kept");
    assert!(config.contains(
        "/opt/appdata/kylin-ai/model-repository/llm_orig_0.1/1/base_model_0.1.gguf"
    ), "model path rewritten to install root");
    assert!(!config.contains("/custom/install/path/model.gguf"), "old path removed");
    assert!(config.contains("key: \"n_ctx\""), "missing params appended");
    assert!(config.contains("string_value: \"128000\""));
    assert!(config.contains(
        "/opt/appdata/kylin-ai/model-repository/llm_orig_0.1/1/chat_template.jinja"
    ), "chat_template_file filled with jinja install path");

    let desc = fs::read_to_string(result.package_root.join("model_description.yaml")).expect("desc");
    assert!(desc.contains("model_capability: [\"llm\"]"), "untouched fields kept");
    assert!(
        desc.contains("model_is3rdModel: \"true\""),
        "model_is3rdModel forced to true"
    );
    assert!(desc.contains("model_display_name: \"Demo\""), "confirmed display name saved");
    assert!(!desc.contains("Original Display"));
    assert!(desc.contains("model_name: \"llm_orig_0.1\""));
    assert!(desc.contains(r#"\"modelMaxToken\":4096"#));
    assert!(desc.contains(r#"\"supportsToolChoice\":false"#));
}

#[test]
fn cancel_flag_aborts_packaging() {
    let source = tempfile::tempdir().expect("source tempdir");
    let output = tempfile::tempdir().expect("output tempdir");
    let base = source.path().join("base_model_0.1.gguf");
    fs::write(&base, "base").expect("base file");

    let base_model = discover_model_files(&[source.path().to_path_buf()])
        .expect("discover files")
        .into_iter()
        .find(|file| file.role == ModelFileRole::Base)
        .expect("base model");

    let draft = PackageDraft {
        source: ModelSource::Local,
        model_name: "llm_demo_0.1".to_string(),
        brand: "Kylin".to_string(),
        base_model: Some(base_model),
        lora_models: vec![],
        mmproj_model: None,
        metadata: metadata(),
        output_dir: output.path().to_path_buf(),
        config_file: None,
        description_file: None,
        chat_template_file: None,
        model_kind: ModelKind::Llm,
        asr_mode: None,
        source_dir: None,
    };

    let cancel = AtomicBool::new(true);
    let result = generate_package(&draft, None, Some(&cancel));
    assert!(matches!(result, Err(PackageError::Cancelled)));
    assert!(
        !output.path().join("llm_demo_0.1").exists(),
        "取消打包后应清理输出位置已生成的文件"
    );
}

#[test]
fn cancel_after_partial_copy_removes_output_dir() {
    let source = tempfile::tempdir().expect("source tempdir");
    let output = tempfile::tempdir().expect("output tempdir");
    let base = source.path().join("base_model_0.1.gguf");
    fs::write(&base, vec![0u8; 4 * 1024 * 1024]).expect("base file");

    let base_model = discover_model_files(&[source.path().to_path_buf()])
        .expect("discover files")
        .into_iter()
        .find(|file| file.role == ModelFileRole::Base)
        .expect("base model");

    let draft = PackageDraft {
        source: ModelSource::Local,
        model_name: "llm_demo_0.1".to_string(),
        brand: "Kylin".to_string(),
        base_model: Some(base_model),
        lora_models: vec![],
        mmproj_model: None,
        metadata: metadata(),
        output_dir: output.path().to_path_buf(),
        config_file: None,
        description_file: None,
        chat_template_file: None,
        model_kind: ModelKind::Llm,
        asr_mode: None,
        source_dir: None,
    };

    // 预先创建输出目录，模拟复制过程中被取消（已产生部分文件）
    let package_root = output.path().join("llm_demo_0.1");
    fs::create_dir_all(package_root.join("1")).expect("create partial output");
    fs::write(package_root.join("1/base_model_0.1.gguf"), "partial").expect("partial file");

    let cancel = AtomicBool::new(true);
    let result = generate_package(&draft, None, Some(&cancel));
    assert!(matches!(result, Err(PackageError::Cancelled)));
    assert!(!package_root.exists(), "取消后部分生成的文件应被删除");
}

#[test]
fn copies_source_tree_into_package() {
    let source = tempfile::tempdir().unwrap();
    fs::write(source.path().join("model.gguf"), "weights").expect("base");
    fs::write(source.path().join("config.pbtxt"), "backend: \"llamacpp\"\n").expect("config");
    fs::write(source.path().join("model_description.yaml"), "model_name: x\n").expect("desc");
    fs::create_dir_all(source.path().join("1")).expect("version dir");
    fs::write(source.path().join("1/mmproj_a.gguf"), "mmproj").expect("mmproj");
    fs::write(source.path().join("1/chat_template.jinja"), "{{ prompt }}").expect("jinja");
    fs::create_dir_all(source.path().join("docs")).expect("docs dir");
    fs::write(source.path().join("docs/readme.txt"), "hi").expect("readme");

    let output = tempfile::tempdir().unwrap();
    let base_model = discover_model_files(&[source.path().to_path_buf()])
        .expect("discover files")
        .into_iter()
        .find(|file| file.role == ModelFileRole::Base)
        .expect("base model");

    let draft = PackageDraft {
        source: ModelSource::Local,
        model_name: "asr_demo_1.0".to_string(),
        model_kind: ModelKind::Asr,
        asr_mode: Some(AsrMode::Once),
        brand: "Kylin".to_string(),
        base_model: Some(base_model),
        lora_models: vec![],
        mmproj_model: None,
        metadata: metadata(),
        output_dir: output.path().to_path_buf(),
        config_file: Some(source.path().join("config.pbtxt")),
        description_file: Some(source.path().join("model_description.yaml")),
        chat_template_file: None,
        source_dir: Some(source.path().to_path_buf()),
    };

    let result = generate_package(&draft, None, None).expect("package generated");
    assert!(result.package_root.join("1/model.gguf").exists());
    assert!(result.package_root.join("1/mmproj_a.gguf").exists());
    assert!(result.package_root.join("1/docs/readme.txt").exists());
    assert!(result.package_root.join("1/chat_template.jinja").exists());
    assert!(!result.package_root.join("1/config.pbtxt").exists());
    assert!(!result.package_root.join("1/model_description.yaml").exists());
    assert!(result.package_root.join("config.pbtxt").exists());
    assert!(result.package_root.join("model_description.yaml").exists());
    // draft 未提供 jinja 路径时回退检查源目录 1/，config 补安装路径
    let config = fs::read_to_string(result.package_root.join("config.pbtxt")).unwrap();
    assert!(config.contains(
        "/opt/appdata/kylin-ai/model-repository/asr_demo_1.0/1/chat_template.jinja"
    ));
    let description = fs::read_to_string(result.package_root.join("model_description.yaml")).unwrap();
    // 原文缺失的字段按最新语音样例默认补充；顶层 isBuiltin / model_is_private 不再输出
    assert!(description.contains("model_capability: [\"once_asr\"]"));
    assert!(description.contains("model_is3rdModel: \"true\""));
    assert!(description.contains("model_internal: \"false\""));
    assert!(description.contains("brand: \"qwen\""));
    assert!(description.contains(r#"others_info: "{\"isBuiltin\":false}"#));
    assert!(
        !description.lines().any(|line| line.trim_start().starts_with("isBuiltin:")),
        "语音模型描述不应再输出顶层 isBuiltin"
    );
    assert!(
        !description.contains("model_is_private"),
        "语音模型描述不应再输出 model_is_private"
    );
}
