use model_management_tool::cloud::{finalize_file, verify_file};
use model_management_tool::packager::{
    generate_package, EditableMetadata, ModelFile, ModelFileRole, ModelKind, ModelSource,
    PackageDraft,
};
use sha2::{Digest, Sha256};
use std::fs;
use std::path::PathBuf;

fn write_temp_file(dir: &std::path::Path, name: &str, content: &[u8]) -> PathBuf {
    let path = dir.join(name);
    fs::write(&path, content).unwrap();
    path
}

fn sha256_of(content: &[u8]) -> String {
    let mut hasher = Sha256::new();
    hasher.update(content);
    format!("{:x}", hasher.finalize())
}

fn sample_metadata() -> EditableMetadata {
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
fn verify_file_passes_with_correct_sha256() {
    let dir = tempfile::tempdir().unwrap();
    let content = b"hello world\n";
    let path = write_temp_file(dir.path(), "model.gguf", content);
    let sha = sha256_of(content);
    assert!(verify_file(&path, &sha, content.len() as u64).is_ok());
}

#[test]
fn verify_file_rejects_wrong_sha256() {
    let dir = tempfile::tempdir().unwrap();
    let content = b"hello world\n";
    let path = write_temp_file(dir.path(), "model.gguf", content);
    assert!(verify_file(&path, "deadbeef", content.len() as u64).is_err());
}

#[test]
fn verify_file_rejects_wrong_size() {
    let dir = tempfile::tempdir().unwrap();
    let content = b"hello world\n";
    let path = write_temp_file(dir.path(), "model.gguf", content);
    let sha = sha256_of(content);
    assert!(verify_file(&path, &sha, 999).is_err());
}

#[test]
fn finalize_file_renames_on_success() {
    let dir = tempfile::tempdir().unwrap();
    let content = b"hello world\n";
    let part = write_temp_file(dir.path(), "model.gguf.part", content);
    let final_p = dir.path().join("model.gguf");
    let sha = sha256_of(content);

    assert!(finalize_file(&part, &final_p, &sha, content.len() as u64).is_ok());

    assert!(final_p.exists());
    assert!(!part.exists());
}

#[test]
fn finalize_file_keeps_part_on_failure() {
    let dir = tempfile::tempdir().unwrap();
    let content = b"hello world\n";
    let part = write_temp_file(dir.path(), "model.gguf.part", content);
    let final_p = dir.path().join("model.gguf");

    assert!(finalize_file(&part, &final_p, "bogus", content.len() as u64).is_err());

    assert!(part.exists());
    assert!(!final_p.exists());
}

#[test]
fn generates_package_for_cloud_source() {
    let staging = tempfile::tempdir().unwrap();
    let base_name = "base_model_0.1.gguf";
    let base_path = write_temp_file(staging.path(), base_name, b"weights");
    let out = tempfile::tempdir().unwrap();

    let draft = PackageDraft {
        source: ModelSource::Cloud,
        model_name: "llm_demo_0.1".to_string(),
        brand: "Kylin".to_string(),
        base_model: Some(ModelFile {
            path: base_path.clone(),
            file_name: base_name.to_string(),
            role: ModelFileRole::Base,
            size: 7,
        }),
        lora_models: vec![],
        mmproj_model: None,
        metadata: sample_metadata(),
        output_dir: out.path().to_path_buf(),
        config_file: None,
        description_file: None,
        chat_template_file: None,
        model_kind: ModelKind::Llm,
        asr_mode: None,
        source_dir: None,
    };

    let result = generate_package(&draft, None, None).expect("cloud source should package");
    assert!(result.package_root.join("1").join(base_name).exists());
    assert!(result.package_root.join("config.pbtxt").exists());
    let desc_path = result.package_root.join("model_description.yaml");
    assert!(desc_path.exists());
    let desc = fs::read_to_string(&desc_path).expect("read description");
    assert!(
        desc.contains("model_is3rdModel: \"true\""),
        "cloud model description must set model_is3rdModel to true"
    );
}

#[test]
fn cloud_source_no_longer_blocked() {
    let staging = tempfile::tempdir().unwrap();
    let base_name = "base_model_0.1.gguf";
    let base_path = write_temp_file(staging.path(), base_name, b"weights");
    let out = tempfile::tempdir().unwrap();

    let mut draft = PackageDraft {
        source: ModelSource::Cloud,
        model_name: "llm_demo_0.1".to_string(),
        brand: "Kylin".to_string(),
        base_model: Some(ModelFile {
            path: base_path,
            file_name: base_name.to_string(),
            role: ModelFileRole::Base,
            size: 7,
        }),
        lora_models: vec![],
        mmproj_model: None,
        metadata: sample_metadata(),
        output_dir: out.path().to_path_buf(),
        config_file: None,
        description_file: None,
        chat_template_file: None,
        model_kind: ModelKind::Llm,
        asr_mode: None,
        source_dir: None,
    };

    let result = generate_package(&draft, None, None);
    assert!(result.is_ok(), "cloud source must not be rejected");

    draft.source = ModelSource::Local;
    let result_local = generate_package(&draft, None, None);
    assert!(result_local.is_ok(), "local source still works");
}

#[test]
fn clean_staging_dir_removes_only_staging_paths() {
    let root = tempfile::tempdir().unwrap();
    let staging = root.path().join(".cloud-staging").join("model_a");
    fs::create_dir_all(&staging).unwrap();
    fs::write(staging.join("model.gguf.part"), b"data").unwrap();

    assert!(model_management_tool::cloud::clean_staging_dir(&staging).is_ok());
    assert!(!staging.exists());
    // 幂等：目录不存在时不报错
    assert!(model_management_tool::cloud::clean_staging_dir(&staging).is_ok());

    // 非暂存路径拒绝删除
    let other = root.path().join("models");
    fs::create_dir_all(&other).unwrap();
    assert!(model_management_tool::cloud::clean_staging_dir(&other).is_err());
    assert!(other.exists());
}

#[test]
fn clean_staging_root_removes_whole_staging_dir() {
    let root = tempfile::tempdir().unwrap();
    let staging_root = root.path().join(".cloud-staging");
    fs::create_dir_all(staging_root.join("a").join("b")).unwrap();
    fs::write(staging_root.join("a").join("model.bin"), b"weights").unwrap();

    assert!(model_management_tool::cloud::clean_staging_root(root.path()).is_ok());
    assert!(!staging_root.exists());
    assert!(root.path().exists());
}
