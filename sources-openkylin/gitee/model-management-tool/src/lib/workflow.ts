import type {
  AppState,
  CloudDownloadProgress,
  CloudListStatus,
  CloudLogLine,
  CloudModel,
  ModelFile,
  PackagingProgress,
  PackageResult,
  SourceKind,
  StepKey
} from "../types";

export const initialState: AppState = {
  step: "start",
  source: "cloud",
  localFiles: [],
  modelDirectory: null,
  selectedCloudModel: null,
  cloudStatus: "idle",
  cloudListStatus: "loading",
  cloudProgress: 0,
  cloudFileProgress: 0,
  cloudOverallProgress: 0,
  cloudCurrentFile: null,
  cloudLogLines: [],
  cloudDownloadSpeed: 0,
  cloudFileDownloaded: 0,
  cloudFileSize: 0,
  cloudOverallDownloaded: 0,
  cloudOverallSize: 0,
  metadataConfirmed: false,
  isBusy: false,
  error: null,
  result: null,
  resultKind: null,
  packagingError: null,
  packagingProgress: 0,
  packagingLogLines: [],
  packagingCancelled: false
};

const MAX_LOG_LINES = 500;

function appendLog(lines: CloudLogLine[], newLine: CloudLogLine): CloudLogLine[] {
  const updated = [...lines, newLine];
  return updated.length > MAX_LOG_LINES ? updated.slice(updated.length - MAX_LOG_LINES) : updated;
}

export type Action =
  | { type: "choose_source"; source: SourceKind }
  | { type: "select_source"; source: SourceKind }
  | { type: "set_model_directory"; directory: string }
  | { type: "reset_local_selection" }
  | { type: "set_local_files"; files: ModelFile[] }
  | { type: "select_cloud_model"; model: CloudModel }
  | { type: "set_cloud_list_status"; status: CloudListStatus }
  | { type: "start_cloud_download" }
  | { type: "set_cloud_file_progress"; progress: CloudDownloadProgress; speed: number }
  | { type: "append_cloud_log"; line: CloudLogLine }
  | { type: "download_cloud_completed"; files: ModelFile[] }
  | { type: "pause_cloud_download" }
  | { type: "stop_cloud_download" }
  | { type: "cloud_unavailable"; error: string }
  | { type: "confirm_metadata" }
  | { type: "start_packaging" }
  | { type: "set_packaging_progress"; progress: PackagingProgress }
  | { type: "package_success"; result: PackageResult }
  | { type: "package_failed"; error: string }
  | { type: "package_cancelled" }
  | { type: "fail"; error: string }
  | { type: "go"; step: StepKey }
  | { type: "reset" };

export function reducer(state: AppState, action: Action): AppState {
  switch (action.type) {
    case "choose_source":
      return {
        ...state,
        source: action.source,
        step: action.source === "local" ? "local" : "cloud",
        error: null,
        result: null
      };
    case "select_source":
      return {
        ...state,
        source: action.source,
        error: null,
        result: null
      };
    case "set_model_directory":
      return {
        ...state,
        modelDirectory: action.directory
      };
    case "reset_local_selection":
      return {
        ...state,
        localFiles: [],
        modelDirectory: null,
        metadataConfirmed: false,
        error: null
      };
    case "set_local_files":
      return {
        ...state,
        source: "local",
        step: "local",
        localFiles: action.files,
        metadataConfirmed: false,
        error: null
      };
    case "select_cloud_model":
      return {
        ...state,
        source: "cloud",
        selectedCloudModel: action.model,
        cloudStatus: "selected",
        cloudProgress: 0,
        cloudFileProgress: 0,
        cloudOverallProgress: 0,
        cloudCurrentFile: null,
        cloudLogLines: [],
        cloudDownloadSpeed: 0,
        cloudFileDownloaded: 0,
        cloudFileSize: 0,
        cloudOverallDownloaded: 0,
        cloudOverallSize: 0,
        error: null
      };
    case "set_cloud_list_status":
      return {
        ...state,
        cloudListStatus: action.status
      };
    case "start_cloud_download":
      return {
        ...state,
        cloudStatus: "downloading",
        cloudCurrentFile: null,
        cloudLogLines: [],
        cloudDownloadSpeed: 0,
        isBusy: true,
        error: null
      };
    case "set_cloud_file_progress":
      return {
        ...state,
        cloudCurrentFile: action.progress.file_name,
        cloudFileProgress: action.progress.file_progress,
        cloudFileDownloaded: action.progress.downloaded_bytes,
        cloudFileSize: action.progress.file_size,
        cloudOverallProgress: action.progress.overall_progress,
        cloudOverallDownloaded: action.progress.overall_downloaded,
        cloudOverallSize: action.progress.overall_size,
        cloudProgress: action.progress.overall_progress,
        cloudDownloadSpeed: action.speed,
        cloudLogLines: appendLog(state.cloudLogLines, {
          time: new Date().toLocaleTimeString(),
          message: `${action.progress.file_name} ${action.progress.downloaded_bytes}/${action.progress.file_size} bytes (${action.progress.file_progress.toFixed(2)}%)`
        })
      };
    case "append_cloud_log":
      return {
        ...state,
        cloudLogLines: appendLog(state.cloudLogLines, action.line)
      };
    case "download_cloud_completed":
      return {
        ...state,
        source: "cloud",
        localFiles: action.files,
        cloudStatus: "completed",
        cloudProgress: 100,
        cloudOverallProgress: 100,
        cloudFileProgress: 100,
        cloudCurrentFile: null,
        cloudDownloadSpeed: 0,
        isBusy: false,
        error: null
      };
    case "pause_cloud_download":
      return {
        ...state,
        cloudStatus: "paused",
        cloudDownloadSpeed: 0,
        isBusy: false,
        error: null
      };
    case "stop_cloud_download":
      return {
        ...state,
        cloudStatus: "selected",
        cloudProgress: 0,
        cloudFileProgress: 0,
        cloudOverallProgress: 0,
        cloudCurrentFile: null,
        cloudLogLines: [],
        cloudDownloadSpeed: 0,
        cloudFileDownloaded: 0,
        cloudFileSize: 0,
        cloudOverallDownloaded: 0,
        cloudOverallSize: 0,
        isBusy: false,
        error: null
      };
    case "cloud_unavailable":
      return {
        ...state,
        cloudStatus: "error",
        cloudDownloadSpeed: 0,
        isBusy: false,
        error: action.error
      };
    case "confirm_metadata":
      return {
        ...state,
        step: "packaging",
        metadataConfirmed: true,
        error: null
      };
    case "start_packaging":
      return {
        ...state,
        step: "packaging",
        isBusy: true,
        packagingProgress: 0,
        packagingLogLines: [],
        packagingCancelled: false,
        packagingError: null,
        resultKind: null,
        error: null
      };
    case "set_packaging_progress":
      return {
        ...state,
        packagingProgress: action.progress.progress,
        packagingLogLines: appendLog(state.packagingLogLines, {
          time: new Date().toLocaleTimeString(),
          message: action.progress.message
        })
      };
    case "package_success":
      return {
        ...state,
        step: "result",
        isBusy: false,
        result: action.result,
        resultKind: "success",
        packagingProgress: 100,
        error: null
      };
    case "package_failed":
      return {
        ...state,
        step: "result",
        isBusy: false,
        resultKind: "failure",
        packagingError: action.error,
        error: action.error
      };
    case "package_cancelled":
      return {
        ...state,
        step: "confirm",
        isBusy: false,
        result: null,
        resultKind: null,
        packagingCancelled: true,
        packagingProgress: 0,
        error: null
      };
    case "fail":
      return {
        ...state,
        isBusy: false,
        error: action.error
      };
    case "go":
      return {
        ...state,
        step: action.step,
        error: null
      };
    case "reset":
      return initialState;
    default:
      return state;
  }
}

export function splitPathInput(input: string): string[] {
  return input
    .split(/\r?\n/)
    .map((item) => item.trim())
    .filter(Boolean);
}

const VERSION_SEGMENT = /^\d+(?:\.\d+)*$/;

export function isValidModelDirectoryName(name: string): boolean {
  const parts = name.trim().split("_");
  return parts.length === 3 && parts[0] === "llm" && parts.every((part) => part.length > 0);
}

export function deriveModelDirectoryName(stem: string): string {
  const trimmed = stem.trim();
  if (isValidModelDirectoryName(trimmed)) {
    return trimmed;
  }

  const parts = trimmed.split("_").filter((part) => part.length > 0);
  const body = ["llm", "asr", "tts"].includes(parts[0]?.toLowerCase() ?? "")
    ? parts.slice(1)
    : parts;
  if (body.length === 0) {
    return "llm_demo_0.1";
  }

  const last = body[body.length - 1];
  const hasVersion = body.length >= 2 && VERSION_SEGMENT.test(last);
  const core = (hasVersion ? body.slice(0, -1) : body).join("-");
  return `llm_${core}_${hasVersion ? last : "1.0"}`;
}

export function deriveVoiceModelDirectoryName(stem: string, kind: "asr" | "tts"): string {
  const prefix = kind === "tts" ? "tts" : "asr";
  const trimmed = stem.trim();
  const allParts = trimmed.split("_").filter((part) => part.length > 0);
  if (allParts.length === 3 && allParts[0] === prefix) {
    return trimmed;
  }

  const body = ["llm", "asr", "tts"].includes(allParts[0]?.toLowerCase() ?? "")
    ? allParts.slice(1)
    : allParts;
  if (body.length === 0) {
    return `${prefix}_demo_1.0`;
  }

  const last = body[body.length - 1];
  const hasVersion = body.length >= 2 && VERSION_SEGMENT.test(last);
  const core = (hasVersion ? body.slice(0, -1) : body).join("-");
  return `${prefix}_${core}_${hasVersion ? last : "1.0"}`;
}

export function pickBaseModel(files: ModelFile[]): ModelFile | null {
  return files.find((file) => file.role === "base") ?? null;
}

export function pickLoraModels(files: ModelFile[]): ModelFile[] {
  return files.filter((file) => file.role === "lora");
}

export function pickMmprojModel(files: ModelFile[]): ModelFile | null {
  return files.find((file) => file.role === "mmproj") ?? null;
}

export type ModelKind = "llm" | "asr" | "tts";
export type AsrMode = "streaming" | "once";

export function detectModelKind(capabilities: string[]): ModelKind {
  const values = capabilities.map((cap) => cap.toLowerCase());
  if (values.some((cap) => cap.includes("asr"))) return "asr";
  if (values.some((cap) => cap.includes("tts"))) return "tts";
  return "llm";
}

export function detectAsrMode(capabilities: string[]): AsrMode {
  return capabilities.some((cap) => cap.toLowerCase().includes("streaming_asr"))
    ? "streaming"
    : "once";
}

export function cloudRowModelDesc(kind: ModelKind): string {
  switch (kind) {
    case "asr":
      return "语音识别模型";
    case "tts":
      return "语音合成模型";
    default:
      return "通用语言大模型";
  }
}

const NETWORK_ERROR_PATTERNS = [
  "error sending request",
  "error trying to connect",
  "error decoding response body",
  "dns error",
  "failed to lookup address",
  "name or service not known",
  "temporary failure in name resolution",
  "connection refused",
  "connection reset",
  "connection closed",
  "connection aborted",
  "network is unreachable",
  "network unreachable",
  "no route to host",
  "host is unreachable",
  "timed out",
  "timeout",
  "tls",
  "ssl",
  "certificate",
  "socket hang up",
  "broken pipe",
  "unexpected eof",
  "incomplete message",
  "channel closed"
];

export function isNetworkErrorMessage(message: string): boolean {
  const lower = message.toLowerCase();
  return NETWORK_ERROR_PATTERNS.some((pattern) => lower.includes(pattern));
}

export function formatCloudDownloadError(message: string): string {
  if (!message.trim()) {
    return "下载异常";
  }
  if (isNetworkErrorMessage(message) || message.includes("下载未完成")) {
    return "网络连接异常";
  }
  const lower = message.toLowerCase();
  if (lower.includes("no space left on device") || message.includes("磁盘可用空间不足")) {
    return "磁盘空间不足";
  }
  if (lower.includes("permission denied")) {
    return "文件写入权限不足";
  }
  if (lower.includes("read-only file system")) {
    return "文件系统只读";
  }
  if (lower.includes("no such file or directory")) {
    return "文件或目录不存在";
  }
  if (message.includes("Content-Range") || message.includes("206")) {
    return "服务器响应异常";
  }
  const httpStatus = message.match(/HTTP\s+(\d{3})/);
  if (httpStatus) {
    return `服务器响应异常（HTTP ${httpStatus[1]}）`;
  }
  if (
    message.includes("校验失败") ||
    message.includes("文件大小不一致") ||
    message.includes("下载数据超出预期大小")
  ) {
    return "文件校验失败";
  }
  if (/[一-鿿]/.test(message)) {
    return message;
  }
  return "下载异常，请稍后重试";
}

export function parentDirectory(path: string): string {
  const trimmed = path.replace(/[/\\]+$/, "");
  const index = Math.max(trimmed.lastIndexOf("/"), trimmed.lastIndexOf("\\"));
  if (index < 0) {
    return trimmed;
  }
  const parent = trimmed.slice(0, index);
  if (!parent) {
    return trimmed.slice(0, 1);
  }
  if (/^[A-Za-z]:$/.test(parent)) {
    return `${parent}\\`;
  }
  return parent;
}
