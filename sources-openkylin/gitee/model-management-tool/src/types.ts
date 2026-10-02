export type SourceKind = "local" | "cloud";
export type StepKey = "start" | "local" | "cloud" | "confirm" | "packaging" | "result";
export type CloudStatus = "idle" | "selected" | "downloading" | "paused" | "completed" | "unavailable" | "error";
export type CloudListStatus = "loading" | "ready" | "no-network";
export type ResultKind = "success" | "failure";

export interface CloudLogLine {
  time: string;
  message: string;
}

export interface PackagingProgress {
  step: string;
  message: string;
  progress: number;
}

export interface ModelFile {
  path: string;
  file_name: string;
  role: "base" | "lora" | "mmproj";
  size?: number;
}

export interface EditableMetadata {
  model_deb_name: string;
  model_display_name: string;
  capabilities: string[];
  model_supports_thinking: boolean;
  model_supports_thinking_toggle: boolean;
  model_max_token: number;
  config_max_token: number;
  supports_tool_choice: boolean;
  parameter_count: number;
}

export interface PackageDraft {
  source: "local" | "cloud";
  model_name: string;
  model_kind: "llm" | "asr" | "tts";
  asr_mode: "streaming" | "once" | null;
  brand: string;
  base_model: ModelFile | null;
  lora_models: ModelFile[];
  mmproj_model: ModelFile | null;
  metadata: EditableMetadata;
  output_dir: string;
  config_file: string | null;
  description_file: string | null;
  chat_template_file: string | null;
  source_dir: string | null;
}

export interface ModelConfigFiles {
  config_file: string | null;
  description_file: string | null;
  chat_template_file: string | null;
}

export interface PackageResult {
  package_root: string;
  generated_files: string[];
}

export interface CloudFile {
  name: string;
  size: number;
  sha256: string;
}

export interface DownloadFile {
  name: string;
  size: number;
  sha256: string;
}

export interface DownloadRequest {
  sub_path: string;
  files: DownloadFile[];
  staging_dir: string;
}

export interface CloudModel {
  id: string;
  name: string;
  vendor: string;
  parameter_count: number;
  capabilities: string[];
  model_capability: string[];
  sub_path: string;
  description: string;
  files: CloudFile[];
}

export interface CloudModelDescription {
  model_deb_name: string;
  model_name: string;
  model_display_name: string;
  brand: string;
  capabilities: string[];
  model_capability: string[];
  model_supports_thinking: boolean;
  model_supports_thinking_toggle: boolean;
  supports_tool_choice: boolean;
  model_max_token: number | null;
  config_max_token: number | null;
  parameter_count: number | null;
}

export interface CloudDownloadProgress {
  file_name: string;
  downloaded_bytes: number;
  file_size: number;
  file_progress: number;
  overall_downloaded: number;
  overall_size: number;
  overall_progress: number;
}

export interface AppState {
  step: StepKey;
  source: SourceKind | null;
  localFiles: ModelFile[];
  modelDirectory: string | null;
  selectedCloudModel: CloudModel | null;
  cloudStatus: CloudStatus;
  cloudListStatus: CloudListStatus;
  cloudProgress: number;
  cloudFileProgress: number;
  cloudOverallProgress: number;
  cloudCurrentFile: string | null;
  cloudLogLines: CloudLogLine[];
  cloudDownloadSpeed: number;
  cloudFileDownloaded: number;
  cloudFileSize: number;
  cloudOverallDownloaded: number;
  cloudOverallSize: number;
  metadataConfirmed: boolean;
  isBusy: boolean;
  error: string | null;
  result: PackageResult | null;
  resultKind: ResultKind | null;
  packagingError: string | null;
  packagingProgress: number;
  packagingLogLines: CloudLogLine[];
  packagingCancelled: boolean;
}
