import { invoke } from "@tauri-apps/api/core";
import { listen, type UnlistenFn } from "@tauri-apps/api/event";
import type {
  CloudDownloadProgress,
  CloudModel,
  CloudModelDescription,
  DownloadRequest,
  EditableMetadata,
  ModelConfigFiles,
  ModelFile,
  PackagingProgress,
  PackageDraft,
  PackageResult
} from "../types";

export const isTauriRuntime =
  typeof window !== "undefined" &&
  ("__TAURI_INTERNALS__" in window ||
    "__TAURI__" in window ||
    "__TAURI_IPC__" in window);

export async function selectLocalFilesDialog(): Promise<string[]> {
  if (!isTauriRuntime) {
    return [];
  }
  return invoke<string[]>("select_local_files_dialog");
}

export async function selectLocalModelDirectoryDialog(): Promise<string | null> {
  if (!isTauriRuntime) {
    return null;
  }
  return invoke<string | null>("select_local_model_directory_dialog");
}

export async function selectOutputDirectory(): Promise<string | null> {
  if (!isTauriRuntime) {
    return null;
  }
  return invoke<string | null>("select_output_directory");
}

export async function getDownloadDirectory(): Promise<string> {
  if (!isTauriRuntime) {
    return "/tmp";
  }
  return invoke<string>("get_download_directory");
}

export async function openDirectory(path: string): Promise<void> {
  if (!isTauriRuntime) {
    return;
  }
  await invoke("open_directory", { path });
}

export async function openKylinOsManager(): Promise<void> {
  if (!isTauriRuntime) {
    return;
  }
  await invoke("open_kylin_os_manager");
}

export async function openAiModuleManager(): Promise<void> {
  if (!isTauriRuntime) {
    return;
  }
  await invoke("open_ai_module_manager");
}

export async function cancelCloudDownload(): Promise<void> {
  if (!isTauriRuntime) {
    return;
  }
  await invoke("cancel_cloud_download");
}

export async function cleanCloudStaging(stagingDir: string): Promise<void> {
  if (!isTauriRuntime) {
    return;
  }
  await invoke("clean_cloud_staging", { stagingDir });
}

export async function cancelPackaging(): Promise<void> {
  if (!isTauriRuntime) {
    return;
  }
  await invoke("cancel_packaging");
}

export async function discoverLocalFiles(paths: string[]): Promise<ModelFile[]> {
  if (!isTauriRuntime) {
    return paths.map((path) => {
      const fileName = path.split(/[\\/]/).pop() || path;
      const lower = fileName.toLowerCase();
      let role: "base" | "lora" | "mmproj" = "base";
      if (lower.startsWith("mmproj")) {
        role = "mmproj";
      } else if (fileName.startsWith("lora_")) {
        role = "lora";
      }
      return {
        path,
        file_name: fileName,
        role
      };
    });
  }
  return invoke<ModelFile[]>("select_local_model_files", { paths });
}

export async function inspectLocalPath(path: string): Promise<boolean> {
  if (!isTauriRuntime) {
    return !/\.(gguf|bin|safetensors)$/i.test(path);
  }
  return invoke<boolean>("inspect_local_path", { path });
}

export async function findModelConfigFiles(paths: string[]): Promise<ModelConfigFiles> {
  if (!isTauriRuntime) {
    return { config_file: null, description_file: null, chat_template_file: null };
  }
  return invoke<ModelConfigFiles>("find_model_config_files", { paths });
}

export async function readModelDescriptionFile(path: string): Promise<CloudModelDescription> {
  if (!isTauriRuntime) {
    const response = await fetch(`file://${path}`);
    if (!response.ok) {
      throw new Error(`读取模型描述失败: HTTP ${response.status}`);
    }
    return parseModelDescriptionYaml(await response.text());
  }
  return invoke<CloudModelDescription>("read_model_description_file", { path });
}

export async function validateMetadata(
  metadata: EditableMetadata,
  kind: "llm" | "asr" | "tts" = "llm"
): Promise<void> {
  if (!isTauriRuntime) {
    if (!metadata.model_display_name.trim()) {
      throw new Error("模型名字不能为空");
    }
    if (kind === "llm" && !metadata.capabilities.length) {
      throw new Error("模型能力不能为空");
    }
    return;
  }
  await invoke("validate_metadata", { metadata, kind });
}

const CLOUD_MANIFEST_PATH = "/kylin-cloud/LLM/KYLIN/branches/default.json";

interface RemoteFileEntry {
  size: number;
  sha256: string;
  time?: string;
}
interface RemoteModelEntry {
  subPath: string;
  descrip: string;
  files: Record<string, RemoteFileEntry>;
}
interface BranchManifest {
  models: Record<string, RemoteModelEntry>;
}

function mapManifestToCloudModels(manifest: BranchManifest): CloudModel[] {
  const entries = Object.entries(manifest.models ?? {});
  return entries.map(([name, remote]) => ({
    id: remote.subPath,
    name,
    vendor: "Kylin 云端仓库",
    parameter_count: 0,
    capabilities: ["text"],
    model_capability: [],
    sub_path: remote.subPath,
    description: remote.descrip,
    files: Object.entries(remote.files ?? {}).map(([fileName, file]) => ({
      name: fileName,
      size: file.size,
      sha256: file.sha256
    }))
  }));
}

export async function listCloudModels(): Promise<CloudModel[]> {
  if (!isTauriRuntime) {
    const response = await fetch(CLOUD_MANIFEST_PATH, { headers: { Accept: "application/json" } });
    if (!response.ok) {
      throw new Error(`拉取清单失败: HTTP ${response.status}`);
    }
    const manifest = (await response.json()) as BranchManifest;
    return mapManifestToCloudModels(manifest);
  }
  return invoke<CloudModel[]>("list_cloud_models");
}

const MODEL_BASE_URL = "https://archive2.kylinos.cn/LLM/KYLIN/models";

export async function fetchModelDescription(subPath: string): Promise<CloudModelDescription> {
  if (!isTauriRuntime) {
    const response = await fetch(`${MODEL_BASE_URL}/${subPath}/model_description.yaml`);
    if (!response.ok) {
      throw new Error(`拉取模型描述失败: HTTP ${response.status}`);
    }
    return parseModelDescriptionYaml(await response.text());
  }
  return invoke<CloudModelDescription>("fetch_model_description", { subPath });
}

function parseModelDescriptionYaml(text: string): CloudModelDescription {
  const pick = (key: string): string | null => {
    const match = text.match(new RegExp(`^${key}:\\s*(.+)$`, "m"));
    return match ? match[1].trim() : null;
  };
  const unquote = (value: string | null): string => {
    if (!value) return "";
    const s = value.trim();
    if (s.startsWith('"')) {
      let i = 1;
      while (i < s.length) {
        if (s[i] === "\\" && i + 1 < s.length) {
          i += 2;
          continue;
        }
        if (s[i] === '"') {
          return s.substring(1, i).replace(/\\"/g, '"').replace(/\\\\/g, "\\");
        }
        i++;
      }
    }
    const hashIdx = s.indexOf(" #");
    if (hashIdx > 0) return s.substring(0, hashIdx).trim();
    return s;
  };
  const asBool = (value: string | null): boolean => unquote(value).toLowerCase() === "true";
  const asArray = (value: string | null): string[] => {
    const raw = unquote(value);
    const inner = raw.startsWith("[") && raw.endsWith("]") ? raw.slice(1, -1) : raw;
    return inner
      .split(",")
      .map((item) => item.trim().replace(/^"|"$/g, "").trim())
      .filter(Boolean);
  };
  let others: Partial<{
    supportsToolChoice: string | boolean;
    capabilities: string[];
    modelMaxToken: number;
    configMaxToken: number;
    parameterCount: number;
    lora: string[];
  }> = {};
  const othersRaw = unquote(pick("others_info"));
  try {
    others = othersRaw ? JSON.parse(othersRaw) : {};
  } catch {
    others = {};
  }
  const capabilities = Array.isArray(others.capabilities) ? others.capabilities : [];
  return {
    model_deb_name: unquote(pick("model_deb_name")),
    model_name: unquote(pick("model_name")),
    model_display_name: unquote(pick("model_display_name")),
    brand: unquote(pick("brand")),
    capabilities,
    model_capability: asArray(pick("model_capability")),
    model_supports_thinking: asBool(pick("model_supports_thinking")),
    model_supports_thinking_toggle: asBool(pick("model_supports_thinking_toggle")),
    supports_tool_choice: String(others.supportsToolChoice ?? "false").toLowerCase() === "true",
    model_max_token: typeof others.modelMaxToken === "number" ? others.modelMaxToken : null,
    config_max_token: typeof others.configMaxToken === "number" ? others.configMaxToken : null,
    parameter_count: typeof others.parameterCount === "number" ? others.parameterCount : null
  };
}

export async function downloadCloudModel(request: DownloadRequest): Promise<ModelFile[]> {
  if (!isTauriRuntime) {
    throw new Error("云端下载需要在 Tauri 桌面端运行");
  }
  return invoke<ModelFile[]>("download_cloud_model", { request });
}

export interface CloudDownloadCompletedPayload {
  sub_path: string;
  files: ModelFile[];
}

export interface CloudDownloadErrorPayload {
  sub_path: string;
  file_name: string | null;
  message: string;
}

export async function listenCloudDownloadStarted(
  handler: (subPath: string) => void
): Promise<UnlistenFn> {
  if (!isTauriRuntime) {
    return Promise.resolve(noopUnlisten);
  }
  return listen<{ sub_path: string }>("cloud-download-started", (event) => {
    handler(event.payload.sub_path);
  });
}

export async function listenCloudDownloadProgress(
  handler: (progress: CloudDownloadProgress) => void
): Promise<UnlistenFn> {
  if (!isTauriRuntime) {
    return Promise.resolve(noopUnlisten);
  }
  return listen<CloudDownloadProgress>("cloud-download-progress", (event) => {
    handler(event.payload);
  });
}

export async function listenCloudDownloadFileDone(
  handler: (fileName: string) => void
): Promise<UnlistenFn> {
  if (!isTauriRuntime) {
    return Promise.resolve(noopUnlisten);
  }
  return listen<{ file_name: string }>("cloud-download-file-done", (event) => {
    handler(event.payload.file_name);
  });
}

export async function listenCloudDownloadCompleted(
  handler: (payload: CloudDownloadCompletedPayload) => void
): Promise<UnlistenFn> {
  if (!isTauriRuntime) {
    return Promise.resolve(noopUnlisten);
  }
  return listen<CloudDownloadCompletedPayload>("cloud-download-completed", (event) => {
    handler(event.payload);
  });
}

export async function listenCloudDownloadError(
  handler: (payload: CloudDownloadErrorPayload) => void
): Promise<UnlistenFn> {
  if (!isTauriRuntime) {
    return Promise.resolve(noopUnlisten);
  }
  return listen<CloudDownloadErrorPayload>("cloud-download-error", (event) => {
    handler(event.payload);
  });
}

export async function listenPackagingProgress(
  handler: (progress: PackagingProgress) => void
): Promise<UnlistenFn> {
  if (!isTauriRuntime) {
    return Promise.resolve(noopUnlisten);
  }
  return listen<PackagingProgress>("packaging-progress", (event) => {
    handler(event.payload);
  });
}

const noopUnlisten: UnlistenFn = () => undefined;

export async function createPackage(draft: PackageDraft): Promise<PackageResult> {
  if (!isTauriRuntime) {
    return {
      package_root: `${draft.output_dir}/${draft.model_name}`,
      generated_files: [
        `${draft.output_dir}/${draft.model_name}/1/${draft.base_model?.file_name ?? "base_model.gguf"}`,
        `${draft.output_dir}/${draft.model_name}/config.pbtxt`,
        `${draft.output_dir}/${draft.model_name}/model_description.yaml`
      ]
    };
  }
  return invoke<PackageResult>("create_package", { draft });
}
