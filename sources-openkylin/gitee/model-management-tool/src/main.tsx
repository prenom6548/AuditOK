import React, { useEffect, useMemo, useReducer, useRef, useState } from "react";
import { createRoot } from "react-dom/client";

import {
  AlertTriangle,
  Check,
  CheckCircle2,
  Circle,
  Cloud,
  Download,
  FolderOpen,
  FolderSearch,
  Info,
  Minus,
  PackageCheck,
  RotateCcw,
  Square,
  WifiOff,
  X,
  XCircle
} from "lucide-react";
import {
  cancelCloudDownload,
  cancelPackaging,
  cleanCloudStaging,
  createPackage,
  discoverLocalFiles,
  downloadCloudModel,
  findModelConfigFiles,
  listCloudModels,
  listenCloudDownloadCompleted,
  listenCloudDownloadProgress,
  listenPackagingProgress,
  openDirectory,
  readModelDescriptionFile,
  inspectLocalPath,
  selectLocalModelDirectoryDialog,
  selectOutputDirectory,
  getDownloadDirectory,
  isTauriRuntime,
  openKylinOsManager,
  openAiModuleManager,
  validateMetadata
} from "./lib/api";
import { getCurrentWindow } from "@tauri-apps/api/window";
import { cloudRowModelDesc, deriveModelDirectoryName, deriveVoiceModelDirectoryName, detectModelKind, formatCloudDownloadError, initialState, parentDirectory, pickBaseModel, pickLoraModels, pickMmprojModel, reducer } from "./lib/workflow";
import type { ModelKind } from "./lib/workflow";
import type {
  CloudLogLine,
  CloudModel,
  EditableMetadata,
  PackageDraft,
  ResultKind,
  StepKey
} from "./types";
import "./styles.css";

const CAPABILITY_OPTIONS = [
  { label: "文本", value: "text" },
  { label: "推理", value: "reasoning" },
  { label: "视觉", value: "image" },
  { label: "语音", value: "audio" },
  { label: "视频", value: "video" },
  { label: "向量化", value: "embedding" },
  { label: "工具", value: "tool" },
  { label: "联网", value: "web" }
];

function capabilityLabel(value: string): string {
  return CAPABILITY_OPTIONS.find((option) => option.value === value)?.label ?? value;
}

const defaultMetadata: EditableMetadata = {
  model_deb_name: "",
  model_display_name: "Kylin Demo Model",
  capabilities: [],
  model_supports_thinking: false,
  model_supports_thinking_toggle: false,
  model_max_token: 32768,
  config_max_token: 32768,
  supports_tool_choice: false,
  parameter_count: 9000000000
};

const stepLabels = ["选择模型", "确认信息", "执行打包", "打包结果"];

type LocalErrorKind = "single-file" | "multi" | "no-model" | "no-base";

const LOCAL_ERROR_MESSAGES: Record<LocalErrorKind, string> = {
  "single-file": "仅支持上传文件目录，单个文件无法上传",
  multi: "一次仅支持上传一个文件目录",
  "no-model": "目录内未检测到.gguf 模型文件，请重新上传",
  "no-base": "未发现基座模型文件，请重新上传"
};

const LOCAL_ERROR_BAR_KINDS: LocalErrorKind[] = ["single-file", "multi"];

function formatFailureReason(reason: string): string {
  return /空间不足|没有空间|no space left|enospc/i.test(reason) ? "磁盘空间不足" : reason;
}

function stepIndex(step: StepKey): number {
  switch (step) {
    case "start":
      return 0;
    case "local":
    case "cloud":
      return 0;
    case "confirm":
      return 1;
    case "packaging":
      return 2;
    case "result":
      return 3;
    default:
      return 0;
  }
}

function App() {
  const [state, dispatch] = useReducer(reducer, initialState);
  const [cloudModels, setCloudModels] = useState<CloudModel[]>([]);
  const [metadata, setMetadata] = useState<EditableMetadata>(defaultMetadata);
  const [modelCapability, setModelCapability] = useState<string[]>([]);
  const [modelName, setModelName] = useState("llm_demo_0.1");
  const [brand, setBrand] = useState("Kylin");
  const [outputDir, setOutputDir] = useState("");
  const [isDragOver, setIsDragOver] = useState(false);
  const [showCancelPackagingDialog, setShowCancelPackagingDialog] = useState(false);
  const [showExitDialog, setShowExitDialog] = useState(false);
  const [confirmInfoConfirmed, setConfirmInfoConfirmed] = useState(false);
  const [confirmError, setConfirmError] = useState<string | null>(null);
  const [configFilePath, setConfigFilePath] = useState<string | null>(null);
  const [descriptionFilePath, setDescriptionFilePath] = useState<string | null>(null);
  const [chatTemplateFilePath, setChatTemplateFilePath] = useState<string | null>(null);
  const [localModelType, setLocalModelType] = useState<"llm" | "voice" | null>("llm");
  const [localErrorKind, setLocalErrorKind] = useState<LocalErrorKind | null>(null);
  const [selectedLocalPath, setSelectedLocalPath] = useState<string | null>(null);
  const [selectedOutputDir, setSelectedOutputDir] = useState<string | null>(null);
  const [elapsedTime, setElapsedTime] = useState("00:00");
  const downloadPausedRef = useRef(false);
  const downloadSessionRef = useRef(0);
  const cloudFetchSessionRef = useRef(0);
  const stepRef = useRef(state.step);
  stepRef.current = state.step;
  const packagingActiveRef = useRef(false);
  const pendingExitRef = useRef(false);
  // 当前云端下载的暂存目录（打包源文件），需要在停止下载/打包成功/回首页/关闭窗口时清理
  const stagingRef = useRef<{ dir: string; subPath: string } | null>(null);
  const [packagingSteps, setPackagingSteps] = useState<{ name: string; status: "waiting" | "active" | "done" }[]>([
    { name: "读取模型文件", status: "waiting" },
    { name: "整理模型信息", status: "waiting" },
    { name: "生成模型包", status: "waiting" },
    { name: "输出打包结果", status: "waiting" }
  ]);

  const baseModel = useMemo(() => pickBaseModel(state.localFiles), [state.localFiles]);
  const loraModels = useMemo(() => pickLoraModels(state.localFiles), [state.localFiles]);
  const mmprojModel = useMemo(() => pickMmprojModel(state.localFiles), [state.localFiles]);
  const totalFileSize = useMemo(
    () => state.localFiles.reduce((sum, f) => sum + (f.size ?? 0), 0),
    [state.localFiles]
  );

  async function fetchCloudModels() {
    const session = ++cloudFetchSessionRef.current;
    const hasCached = cloudModels.length > 0;
    dispatch({ type: "set_cloud_list_status", status: "loading" });
    try {
      const models = await listCloudModels();
      if (session !== cloudFetchSessionRef.current) return;
      setCloudModels(models);
      dispatch({ type: "set_cloud_list_status", status: "ready" });
    } catch {
      if (session !== cloudFetchSessionRef.current) return;
      if (hasCached) {
        dispatch({ type: "set_cloud_list_status", status: "ready" });
      } else {
        dispatch({ type: "set_cloud_list_status", status: "no-network" });
      }
    }
  }

  useEffect(() => {
    void (async () => {
      try {
        const dir = await getDownloadDirectory();
        setOutputDir(dir);
      } catch {
        setOutputDir("/tmp/kylin-model-packages");
      }
    })();
  }, []);

  useEffect(() => {
    if (state.step !== "cloud") return;
    if (state.cloudStatus === "downloading" || state.cloudStatus === "paused") return;
    void fetchCloudModels();
  }, [state.step]);

  useEffect(() => {
    const root = document.documentElement;
    const updateScale = () => {
      const styles = getComputedStyle(root);
      const panelW = parseFloat(styles.getPropertyValue("--panel-w"));
      const panelH = parseFloat(styles.getPropertyValue("--panel-h"));
      const scale = Math.min(
        window.innerWidth / (Number.isFinite(panelW) && panelW > 0 ? panelW : 896),
        window.innerHeight / (Number.isFinite(panelH) && panelH > 0 ? panelH : 688)
      );
      root.style.setProperty("--scale", scale.toFixed(4));
    };
    updateScale();
    window.addEventListener("resize", updateScale);
    return () => window.removeEventListener("resize", updateScale);
  }, []);

  useEffect(() => {
    if (!isTauriRuntime) return;
    let unlisten: (() => void) | undefined;
    let disposed = false;
    void getCurrentWindow()
      .onCloseRequested((event) => {
        // 始终接管原生关闭请求（Alt+F4/窗口管理器关闭），由 routeCloseRequest 路由后
        // 显式 destroy：v2 API 未拦截时包装器会自动 destroy（依赖额外权限），
        // v1 包装器会再次 close，两版语义不同，统一 preventDefault 避免歧义。
        event.preventDefault();
        routeCloseRequest();
      })
      .then((fn) => {
        if (disposed) fn();
        else unlisten = fn;
      })
      .catch(() => undefined);
    return () => {
      disposed = true;
      unlisten?.();
    };
  }, []);

  useEffect(() => {
    if (state.step === "start") {
      setConfigFilePath(null);
      setDescriptionFilePath(null);
      setChatTemplateFilePath(null);
    }
  }, [state.step]);

  useEffect(() => {
    if (state.step !== "packaging") return;
    setElapsedTime("00:00");
    const start = Date.now();
    const id = setInterval(() => {
      const elapsed = Math.floor((Date.now() - start) / 1000);
      const mm = String(Math.floor(elapsed / 60)).padStart(2, "0");
      const ss = String(elapsed % 60).padStart(2, "0");
      setElapsedTime(`${mm}:${ss}`);
    }, 1000);
    return () => clearInterval(id);
  }, [state.step]);

  useEffect(() => {
    if (state.step !== "packaging") return;
    setPackagingSteps([
      { name: "读取模型文件", status: "waiting" },
      { name: "整理模型信息", status: "waiting" },
      { name: "生成模型包", status: "waiting" },
      { name: "输出打包结果", status: "waiting" }
    ]);
    const timers: ReturnType<typeof setTimeout>[] = [];
    const stepDurations = [800, 1000, 1200, 600];
    let elapsed = 0;
    for (let i = 0; i < 4; i++) {
      timers.push(setTimeout(() => {
        setPackagingSteps((prev) => prev.map((s, idx) => idx === i ? { ...s, status: "active" } : s));
      }, elapsed));
      elapsed += stepDurations[i];
      timers.push(setTimeout(() => {
        setPackagingSteps((prev) => prev.map((s, idx) => idx === i ? { ...s, status: "done" } : s));
      }, elapsed));
    }
    return () => timers.forEach(clearTimeout);
  }, [state.step]);

  useEffect(() => {
    // 打包已结束（成功/失败/取消）时，未关闭的取消/退出确认框直接隐藏
    if (state.step === "packaging") return;
    setShowCancelPackagingDialog(false);
    setShowExitDialog(false);
  }, [state.step]);

  useEffect(() => {
    let unlistenTauriDrop: (() => void) | undefined;
    import("@tauri-apps/api/webview")
      .then(({ getCurrentWebview }) =>
        getCurrentWebview().onDragDropEvent((event) => {
          const payload = event.payload as { type: string; paths?: string[] };
          if (payload.type === "over") {
            setIsDragOver(true);
          }
          if (payload.type === "leave") {
            setIsDragOver(false);
          }
          if (payload.type === "drop") {
            setIsDragOver(false);
            if (payload.paths?.length) {
              void loadLocalPaths(payload.paths);
            }
          }
        })
      )
      .then((unlisten) => {
        unlistenTauriDrop = unlisten;
      })
      .catch(() => {
        unlistenTauriDrop = undefined;
      });

    const onDragOver = (event: DragEvent) => {
      event.preventDefault();
      setIsDragOver(true);
    };
    const onDragLeave = () => setIsDragOver(false);
    const onDrop = async (event: DragEvent) => {
      event.preventDefault();
      setIsDragOver(false);
      const paths = Array.from(event.dataTransfer?.files ?? [])
        .map((file) => (file as File & { path?: string }).path || file.name)
        .filter(Boolean);
      if (paths.length > 0) {
        await loadLocalPaths(paths);
      }
    };

    window.addEventListener("dragover", onDragOver);
    window.addEventListener("dragleave", onDragLeave);
    window.addEventListener("drop", onDrop);
    return () => {
      window.removeEventListener("dragover", onDragOver);
      window.removeEventListener("dragleave", onDragLeave);
      window.removeEventListener("drop", onDrop);
      unlistenTauriDrop?.();
    };
  }, []);

  async function loadLocalPaths(paths: string[]) {
    // 单目录语义：新的选择/拖入直接替换旧结果
    setLocalErrorKind(null);
    setSelectedLocalPath(null);
    dispatch({ type: "reset_local_selection" });
    dispatch({ type: "choose_source", source: "local" });
    if (paths.length > 1) {
      setLocalErrorKind("multi");
      return;
    }
    const target = paths[0];
    if (!target) return;
    let isDir = false;
    try {
      isDir = await inspectLocalPath(target);
    } catch {
      isDir = false;
    }
    if (!isDir) {
      setLocalErrorKind("single-file");
      return;
    }
    setSelectedLocalPath(target);
    dispatch({ type: "set_model_directory", directory: target });
    try {
      const files = await discoverLocalFiles([target]);
      if (files.length === 0) {
        setLocalErrorKind("no-model");
        return;
      }
      if (!files.some((file) => file.role === "base")) {
        setLocalErrorKind("no-base");
        return;
      }
      dispatch({ type: "set_local_files", files });

      try {
        const configFiles = await findModelConfigFiles([target]);
        setConfigFilePath(configFiles.config_file);
        setDescriptionFilePath(configFiles.description_file);
        setChatTemplateFilePath(configFiles.chat_template_file);
      } catch {
        // 无配置文件时保持默认值，goToConfirm 会重新读取
      }
    } catch {
      setLocalErrorKind("no-model");
    }
  }

  async function handleSelectLocalDirectory() {
    try {
      const path = await selectLocalModelDirectoryDialog();
      if (path) {
        await loadLocalPaths([path]);
      }
    } catch {
      // 对话框异常不影响主流程
    }
  }

  function takeStagingDir(): string | null {
    const dir = stagingRef.current?.dir ?? null;
    stagingRef.current = null;
    return dir;
  }

  async function cleanStagingQuietly(dir?: string | null) {
    const target = dir ?? stagingRef.current?.dir ?? null;
    if (!target) return;
    try {
      await cleanCloudStaging(target);
    } catch {
      // 清理暂存失败不影响主流程
    }
  }

  async function destroyWindowWithCleanup() {
    // 关闭应用前清理暂存（下载中会先由后端取消下载任务并等待其退出）
    await cleanStagingQuietly(takeStagingDir());
    destroyWindow();
  }

  async function handleSelectCloudModel(model: CloudModel) {
    const prev = stagingRef.current;
    if (prev && prev.subPath !== model.sub_path) {
      // 切换选中其他模型：旧模型的暂存目录不再使用，清理
      stagingRef.current = null;
      await cleanStagingQuietly(prev.dir);
    }
    dispatch({ type: "select_cloud_model", model });
  }

  async function handleCloudDownload() {
    if (!state.selectedCloudModel) return;
    const model = state.selectedCloudModel;
    downloadPausedRef.current = false;
    const session = ++downloadSessionRef.current;
    dispatch({ type: "start_cloud_download" });
    let prevBytes = 0;
    let prevTime = Date.now();
    let unlistenProgress: () => void = () => undefined;
    let unlistenCompleted: () => void = () => undefined;
    let completedEventReceived = false;
    try {
      unlistenProgress = await listenCloudDownloadProgress((progress) => {
        const now = Date.now();
        const elapsed = (now - prevTime) / 1000;
        const speed = elapsed > 0 ? (progress.overall_downloaded - prevBytes) / elapsed : 0;
        prevBytes = progress.overall_downloaded;
        prevTime = now;
        dispatch({ type: "set_cloud_file_progress", progress, speed });
      });
      unlistenCompleted = await listenCloudDownloadCompleted(() => {
        completedEventReceived = true;
      });
      const request = {
        sub_path: model.sub_path,
        files: model.files.map((file) => ({ name: file.name, size: file.size, sha256: file.sha256 })),
        staging_dir: `${outputDir}/.cloud-staging/${model.sub_path}`
      };
      stagingRef.current = { dir: request.staging_dir, subPath: model.sub_path };
      const files = await downloadCloudModel(request);
      if (session !== downloadSessionRef.current) return;
      if (!completedEventReceived) {
        await new Promise((resolve) => setTimeout(resolve, 50));
      }
      dispatch({ type: "download_cloud_completed", files });

      const stagingConfigPath = `${request.staging_dir}/config.pbtxt`;
      const stagingDescriptionPath = `${request.staging_dir}/model_description.yaml`;
      const stagingChatTemplatePath = `${request.staging_dir}/chat_template.jinja`;
      setConfigFilePath(stagingConfigPath);
      setDescriptionFilePath(stagingDescriptionPath);
      setChatTemplateFilePath(stagingChatTemplatePath);
    } catch (error) {
      if (session !== downloadSessionRef.current || downloadPausedRef.current) return;
      dispatch({ type: "cloud_unavailable", error: String(error) });
    } finally {
      unlistenProgress();
      unlistenCompleted();
    }
  }

  async function handlePauseDownload() {
    if (state.cloudStatus !== "downloading") return;
    downloadPausedRef.current = true;
    try {
      await cancelCloudDownload();
    } catch {
      // ignore
    }
    dispatch({ type: "pause_cloud_download" });
  }

  async function handleResumeDownload() {
    await handleCloudDownload();
  }

  async function handleStopDownload() {
    downloadPausedRef.current = true;
    downloadSessionRef.current += 1;
    try {
      await cancelCloudDownload();
    } catch {
      // ignore
    }
    // 点击 x 停止下载：不保留续传，清理暂存目录（后端会等待下载任务退出后再删）
    await cleanStagingQuietly(takeStagingDir());
    dispatch({ type: "stop_cloud_download" });
  }

  async function confirmMetadata() {
    try {
      await validateMetadata(metadata, effectiveKind);
      dispatch({ type: "confirm_metadata" });
    } catch (error) {
      dispatch({ type: "fail", error: String(error) });
    }
  }

  async function packageLocalModel() {
    if (!baseModel) {
      dispatch({ type: "fail", error: "缺少基座模型文件" });
      return;
    }
    if (!outputDir) {
      dispatch({ type: "fail", error: "请先选择输出位置" });
      return;
    }
    const draft: PackageDraft = {
      source: state.source === "cloud" ? "cloud" : "local",
      model_name: modelName,
      model_kind: effectiveKind,
      asr_mode: effectiveKind === "asr" ? "once" : null,
      brand,
      base_model: baseModel,
      lora_models: loraModels,
      mmproj_model: mmprojModel,
      metadata: { ...metadata, model_deb_name: modelName },
      output_dir: outputDir,
      config_file: configFilePath,
      description_file: descriptionFilePath,
      chat_template_file: chatTemplateFilePath,
      source_dir: state.source === "cloud" ? stagingRef.current?.dir ?? null : selectedLocalPath
    };
    try {
      dispatch({ type: "start_packaging" });
      packagingActiveRef.current = true;
      const unlistenProgress = await listenPackagingProgress((progress) => {
        dispatch({ type: "set_packaging_progress", progress });
      });
      try {
        const result = await createPackage(draft);
        if (draft.source === "cloud") {
          // 打包成功：模型文件已复制进模型包，清理暂存目录
          await cleanStagingQuietly(takeStagingDir());
        }
        if (!pendingExitRef.current) {
          dispatch({ type: "package_success", result });
        }
      } catch (error) {
        const msg = String(error);
        if (!pendingExitRef.current) {
          if (msg.includes("取消")) {
            dispatch({ type: "package_cancelled" });
          } else {
            dispatch({ type: "package_failed", error: msg });
          }
        }
      } finally {
        unlistenProgress();
        packagingActiveRef.current = false;
        if (pendingExitRef.current) {
          await destroyWindowWithCleanup();
        }
      }
    } catch (error) {
      packagingActiveRef.current = false;
      if (pendingExitRef.current) {
        await destroyWindowWithCleanup();
      } else {
        dispatch({ type: "package_failed", error: String(error) });
      }
    }
  }

  async function handleCancelPackaging() {
    setShowCancelPackagingDialog(false);
    try {
      await cancelPackaging();
    } catch {
      // ignore
    }
  }

  function routeCloseRequest() {
    if (pendingExitRef.current) return; // 已确认退出，等待打包清理完成后关闭窗口
    if (stepRef.current === "packaging") {
      setShowExitDialog(true);
      return;
    }
    // 下载中/下载完成后关闭应用：先清理暂存目录（后端会取消进行中的下载），再销毁窗口
    void destroyWindowWithCleanup();
  }

  async function handleConfirmExit() {
    setShowExitDialog(false);
    if (packagingActiveRef.current) {
      // 打包进行中：先取消打包（后端会清理已生成的文件），任务返回后再关闭窗口
      pendingExitRef.current = true;
      try {
        await cancelPackaging();
      } catch {
        // ignore
      }
    } else {
      destroyWindow();
    }
  }

  async function handleOpenDirectory() {
    const packageRoot = state.result?.package_root;
    if (!packageRoot) {
      return;
    }
    try {
      await openDirectory(parentDirectory(packageRoot));
    } catch {
      // ignore
    }
  }

  function resetAll() {
    // 回到首页：清理云端下载暂存目录残留（幂等，打包成功后已清理过则跳过）
    void cleanStagingQuietly(takeStagingDir());
    downloadPausedRef.current = false;
    downloadSessionRef.current += 1;
    packagingActiveRef.current = false;
    pendingExitRef.current = false;
    setMetadata(defaultMetadata);
    setModelCapability([]);
    setModelName("llm_demo_0.1");
    setBrand("Kylin");
    setConfirmInfoConfirmed(false);
    setConfirmError(null);
    setConfigFilePath(null);
    setDescriptionFilePath(null);
    setChatTemplateFilePath(null);
    setSelectedOutputDir(null);
    setLocalModelType("llm");
    setLocalErrorKind(null);
    setSelectedLocalPath(null);
    setShowCancelPackagingDialog(false);
    setShowExitDialog(false);
    setElapsedTime("00:00");
    setPackagingSteps([
      { name: "读取模型文件", status: "waiting" },
      { name: "整理模型信息", status: "waiting" },
      { name: "生成模型包", status: "waiting" },
      { name: "输出打包结果", status: "waiting" }
    ]);
    dispatch({ type: "reset" });
  }

  async function goToConfirm() {
    setConfirmInfoConfirmed(false);
    setLocalErrorKind(null);

    let newMetadata: EditableMetadata = { ...defaultMetadata };
    let newModelName = "llm_demo_0.1";
    let newBrand = "Kylin";
    let stem = "";

    if (state.source === "local") {
      const base = baseModel;
      if (base) {
        stem = base.file_name.replace(/\.(gguf|bin|safetensors)$/i, "");
        if (stem) {
          newMetadata = { ...newMetadata, model_display_name: stem };
        }
      }
    }

    let caps: string[] = [];
    let descModelName = "";
    if (descriptionFilePath) {
      try {
        const desc = await readModelDescriptionFile(descriptionFilePath);
        caps = desc.model_capability;
        descModelName = desc.model_name;
        newMetadata = {
          ...newMetadata,
          model_display_name: desc.model_display_name || newMetadata.model_display_name,
          capabilities: desc.capabilities.length ? desc.capabilities : newMetadata.capabilities,
          model_supports_thinking: desc.model_supports_thinking,
          model_supports_thinking_toggle: desc.model_supports_thinking_toggle,
          supports_tool_choice: desc.supports_tool_choice,
          model_max_token: desc.model_max_token ?? newMetadata.model_max_token,
          config_max_token: desc.config_max_token ?? newMetadata.config_max_token,
          parameter_count: desc.parameter_count ?? newMetadata.parameter_count
        };
        if (desc.brand) newBrand = desc.brand;
      } catch {
        // keep defaults/inferred values
      }
    }

    const descKind = detectModelKind(caps);
    // 本地暂不支持打包语音模型，本地来源一律按大语言模型处理
    const kind: ModelKind = state.source === "local" ? "llm" : descKind;

    if (state.source === "local" && descKind !== "llm") {
      // 目录原始配置是语音结构（config.pbtxt 张量/参数、description 的 model_capability 均与 LLM 不兼容），
      // 全部丢弃，打包时按 LLM 模板从零生成
      setConfigFilePath(null);
      setDescriptionFilePath(null);
      setChatTemplateFilePath(null);
    }

    if (kind === "llm") {
      if (descModelName) newModelName = deriveModelDirectoryName(descModelName);
      else if (stem) newModelName = deriveModelDirectoryName(stem);
    } else {
      // 语音模型固定按 asr 类型打包，目录名同样归一化为 asr_<name>_<version> 三段形式，
      // 已符合格式的名字原样保留，避免非规范名导致打包校验失败
      newModelName = deriveVoiceModelDirectoryName(descModelName || stem, "asr");
    }
    setModelCapability(caps);

    setMetadata(newMetadata);
    setModelName(newModelName);
    setBrand(newBrand);
    dispatch({ type: "go", step: "confirm" });
  }

  function handleMetadataChange(patch: Partial<EditableMetadata>) {
    setMetadata((prev) => ({ ...prev, ...patch }));
    setConfirmInfoConfirmed(false);
  }

  function handleConfirmCheckboxChange(checked: boolean) {
    if (!checked) {
      setConfirmInfoConfirmed(false);
      return;
    }
    if (!metadata.model_display_name.trim()) {
      setConfirmError("请填写模型名称");
      return;
    }
    if (effectiveKind === "llm" && !metadata.capabilities.length) {
      setConfirmError(metaLocked ? "模型描述文件缺少模型能力，请重新下载模型" : "请选择模型能力");
      return;
    }
    setConfirmInfoConfirmed(true);
  }

  const disabled = state.isBusy;
  const active = stepIndex(state.step);
  const descKind = detectModelKind(modelCapability);
  // 本地暂不支持打包语音模型，本地来源确认页一律按大语言模型展示
  const confirmKind: ModelKind = state.source === "local" ? "llm" : descKind;
  const effectiveKind: ModelKind = confirmKind === "llm" ? "llm" : "asr";
  const isVoiceConfirm = confirmKind !== "llm";
  // 仓库模型的描述文件信息完整，确认页只允许改模型名字，其余字段只读展示
  const metaLocked = state.source === "cloud";

  return (
    <main className="stage">
      {state.step === "start" && (
        <Panel title="选择模型来源" subtitle="支持从模型仓库获取模型，或使用本地已有的模型文件，在本机完成标准化打包。" active={active} onRequestClose={routeCloseRequest}>
          <PanelBody>
            <button
              className={`source-card ${state.source === "cloud" ? "selected" : ""}`}
              onClick={() => dispatch({ type: "select_source", source: "cloud" })}
            >
              <strong className="source-card-title">从模型仓库获取</strong>
              <span className="source-card-desc">填写仓库模型地址并选择目标模型，工具将在本地拉取模型文件。</span>
              <ul className="source-features">
                <li><Check size={16} className="feature-check" /> 适合在线获取开源模型</li>
                <li><Check size={16} className="feature-check" /> 支持查看模型基本信息</li>
                <li><Check size={16} className="feature-check" /> 自动展示获取进度</li>
              </ul>
              <span className="source-link">需要联网访问模型仓库</span>
            </button>
            <button
              className={`source-card ${state.source === "local" ? "selected" : ""}`}
              onClick={() => dispatch({ type: "select_source", source: "local" })}
            >
              <strong className="source-card-title">选择本地模型</strong>
              <span className="source-card-desc">选择已保存在本机的模型目录或模型文件，直接进入信息确认和打包。</span>
              <ul className="source-features">
                <li><Check size={16} className="feature-check" /> 无需重新下载模型</li>
                <li><Check size={16} className="feature-check" /> 支持目录或文件选择</li>
                <li><Check size={16} className="feature-check" /> 模型数据仅在本机处理</li>
              </ul>
              <span className="source-link">&nbsp;</span>
            </button>
          </PanelBody>
          <PanelFoot>
            <button
              className="btn btn-primary"
              onClick={() => dispatch({ type: "go", step: state.source === "cloud" ? "cloud" : "local" })}
              disabled={!state.source}
            >
              下一步
            </button>
          </PanelFoot>
        </Panel>
      )}

      {state.step === "local" && (
        <Panel title="本地模型" subtitle="选择本机已有的大语言模型目录。" active={active} onRequestClose={routeCloseRequest}>
          <PanelBody>
            <div className="local-left">
              {/* 因系统模型加载问题，本地暂不支持打包语音模型，暂时隐藏模型类型选择，固定按大语言模型处理
              <section className="card local-type-card">
                <span className="local-type-label"><em>*</em>选择模型类型：</span>
                <label className="local-type-option">
                  <input
                    type="radio"
                    name="local-model-type"
                    checked={localModelType === "llm"}
                    onChange={() => setLocalModelType("llm")}
                    disabled={disabled}
                  />
                  <span className="radio-dot" />
                  <span>大语言模型</span>
                </label>
                <label className="local-type-option">
                  <input
                    type="radio"
                    name="local-model-type"
                    checked={localModelType === "voice"}
                    onChange={() => setLocalModelType("voice")}
                    disabled={disabled}
                  />
                  <span className="radio-dot" />
                  <span>语音模型</span>
                </label>
              </section>
              */}
              <section className="card drop-card">
                <div className={`drop-zone ${isDragOver ? "dragging" : ""}`}>
                  <button className="btn btn-action btn-lg" onClick={handleSelectLocalDirectory} disabled={disabled}>
                    选择模型目录
                  </button>
                  <span className="drop-hint">请先将 .gguf 文件整理到一个文件目录，再拖入或点击上传</span>
                </div>
              </section>
              {localErrorKind && LOCAL_ERROR_BAR_KINDS.includes(localErrorKind) && (
                <div className="local-error-bar">
                  <XCircle size={16} className="local-error-bar-icon" />
                  <span>{LOCAL_ERROR_MESSAGES[localErrorKind]}</span>
                  <button className="local-error-close" onClick={() => setLocalErrorKind(null)}>
                    <X size={14} />
                  </button>
                </div>
              )}
              {selectedLocalPath && (localErrorKind === null || !LOCAL_ERROR_BAR_KINDS.includes(localErrorKind)) && (
                <div className={`dir-card ${localErrorKind ? "warn" : "ok"}`}>
                  <strong className="dir-card-title">目录名称</strong>
                  {localErrorKind ? (
                    <div className="dir-card-warn-row">
                      <AlertTriangle size={16} className="dir-card-warn-icon" />
                      <span>{LOCAL_ERROR_MESSAGES[localErrorKind]}</span>
                      <button className="local-error-close" onClick={() => setLocalErrorKind(null)}>
                        <X size={14} />
                      </button>
                    </div>
                  ) : (
                    <div className="dir-card-ok-row">
                      <span>{selectedLocalPath}{totalFileSize > 0 ? `  ${formatBytes(totalFileSize)}` : ""}</span>
                      <CheckCircle2 size={20} className="dir-card-check" />
                    </div>
                  )}
                </div>
              )}
            </div>
            <aside className="card-detail">
              <strong className="card-title">{selectedLocalPath ? "模型读取结果" : "已选模型"}</strong>
              {selectedLocalPath === null ? (
                <span className="card-sub detail-empty">暂无</span>
              ) : (
                <div className="model-status-rows">
                  <div className="status-row">
                    <span>模型目录</span>
                    <strong>已选择</strong>
                  </div>
                  <div className="status-row">
                    <span>模型文件</span>
                    {state.localFiles.length > 0 ? <strong>已发现</strong> : <strong className="status-warn">未发现</strong>}
                  </div>
                  <div className="status-row">
                    <span>基础信息</span>
                    <strong>待确认</strong>
                  </div>
                  <div className="status-row">
                    <span>文件大小</span>
                    <strong>{totalFileSize > 0 ? formatBytes(totalFileSize) : "-"}</strong>
                  </div>
                </div>
              )}
              <div className="tip-box-local">
                下一步将确认读取到的模型信息，确认后可操作打包。
              </div>
            </aside>
          </PanelBody>
          <PanelFoot>
            <button className="btn btn-gray" onClick={resetAll} disabled={disabled}>
              取消
            </button>
            <button className="btn btn-gray" onClick={() => dispatch({ type: "go", step: "start" })} disabled={disabled}>
              上一步
            </button>
            <button
              className="btn btn-primary"
              onClick={() => goToConfirm()}
              disabled={!baseModel || !localModelType || disabled}
            >
              下一步
            </button>
          </PanelFoot>
        </Panel>
      )}

      {state.step === "cloud" && (
        <Panel title="模型仓库" subtitle="连接模型仓库，查找并选择需要打包的开源模型。" active={active} onRequestClose={routeCloseRequest}>
          <PanelBody>
            {state.cloudListStatus === "no-network" ? (
              <>
                <section className="card cloud-list cloud-empty-card">
                  <strong className="card-title">模型列表</strong>
                  <div className="cloud-empty-inner">
                    <span className="card-sub">未获取到模型列表，请检查网络后重试</span>
                    <button className="btn btn-primary" onClick={() => void fetchCloudModels()}>
                      重试
                    </button>
                  </div>
                </section>
                <aside className="cloud-detail">
                  <strong className="card-title">已选模型</strong>
                  <span className="detail-empty">暂无</span>
                </aside>
              </>
            ) : (
              <>
                <section className="card cloud-list">
                  <strong className="card-title" style={{ marginBottom: 8 }}>模型列表</strong>
                  <div className="cloud-grid">
                    {cloudModels.map((model) => {
                      const totalSize = model.files.reduce((sum, file) => sum + file.size, 0);
                      const isSelected = state.selectedCloudModel?.id === model.id;
                      return (
                        <button
                          key={model.id}
                          className={`model-row ${isSelected ? "selected" : ""} ${state.cloudStatus === "downloading" && !isSelected ? "disabled-row" : ""}`}
                          onClick={() => state.cloudStatus !== "downloading" && handleSelectCloudModel(model)}
                          disabled={disabled || state.cloudStatus === "downloading"}
                        >
                          <div className="model-row-text">
                            <strong>{model.name}</strong>
                            <span className="model-row-desc">{cloudRowModelDesc(detectModelKind(model.model_capability))} 版本 1.0 约 {formatBytes(totalSize)}</span>
                          </div>
                          {isSelected && <Circle size={20} className="model-row-radio" fill="#3790fa" stroke="#3790fa" />}
                        </button>
                      );
                    })}
                  </div>
                </section>
                <aside className="cloud-detail">
                  <strong className="card-title">已选模型</strong>
                  {state.selectedCloudModel ? (
                    <div className="cloud-detail-rows">
                      <div className="detail-row">
                        <span>模型名称</span>
                        <strong>{state.selectedCloudModel.name}</strong>
                      </div>
                      <div className="detail-row">
                        <span>模型版本</span>
                        <strong>1.0</strong>
                      </div>
                      <div className="detail-row">
                        <span>模型大小</span>
                        <strong>{formatBytes(state.selectedCloudModel.files.reduce((s, f) => s + f.size, 0))}</strong>
                      </div>
                      <div className="detail-row">
                        <span>获取方式</span>
                        <strong>模型仓库</strong>
                      </div>
                    </div>
                  ) : (
                    <span className="detail-empty">暂无</span>
                  )}

                  {state.cloudStatus === "error" && state.error && (
                    <div className="cloud-error-toast">
                      <XCircle size={16} className="cloud-error-icon" fill="#f43f5e" color="#fff" />
                      <div className="cloud-error-text">
                        <strong className="cloud-error-title">模型下载失败</strong>
                        <span className="cloud-error-detail">{formatCloudDownloadError(state.error)}</span>
                      </div>
                      <button className="cloud-error-close" onClick={() => dispatch({ type: "fail", error: "" })}>
                        <X size={14} />
                      </button>
                    </div>
                  )}
                  {(state.cloudStatus === "downloading" || state.cloudStatus === "paused") && (
                    <div className="cloud-download-tip">
                      <span>
                        {state.cloudStatus === "downloading"
                          ? `ⓘ 模型下载中...${state.cloudOverallProgress.toFixed(0)}%`
                          : `ⓘ 模型下载暂停...${state.cloudOverallProgress.toFixed(0)}%`}
                      </span>
                      {state.cloudStatus === "downloading" ? (
                        <button className="cloud-download-pause" onClick={handlePauseDownload}>
                          暂停
                        </button>
                      ) : (
                        <button className="cloud-download-pause" onClick={handleResumeDownload}>
                          继续
                        </button>
                      )}
                      <button className="cloud-download-close" onClick={handleStopDownload}>
                        <X size={14} />
                      </button>
                    </div>
                  )}
                  {state.cloudStatus === "completed" && (
                    <div className="cloud-complete-toast">
                      <CheckCircle2 size={16} className="cloud-complete-icon" />
                      <span>模型下载完成！</span>
                      <button className="cloud-complete-close" onClick={() => dispatch({ type: "go", step: "confirm" })}>
                        <X size={14} />
                      </button>
                    </div>
                  )}
                  {state.cloudStatus === "selected" && state.selectedCloudModel && (
                    <div className="tip-box-local">
                      确认后将开始下载模型文件，请确保网络连接正常，且本地存储空间充足
                    </div>
                  )}
                </aside>
              </>
            )}
          </PanelBody>
          <PanelFoot>
            <button className="btn btn-gray" onClick={resetAll} disabled={disabled}>
              取消
            </button>
            <button className="btn btn-gray" onClick={() => dispatch({ type: "go", step: "start" })} disabled={disabled}>
              上一步
            </button>
            {state.cloudStatus === "completed" ? (
              <button className="btn btn-primary" onClick={goToConfirm}>
                下一步
              </button>
            ) : (
              <button
                className="btn btn-primary"
                onClick={handleCloudDownload}
                disabled={!state.selectedCloudModel || disabled || state.cloudStatus === "downloading"}
              >
                {state.cloudStatus === "error" ? "重新下载" : "下一步"}
              </button>
            )}
          </PanelFoot>
        </Panel>
      )}

      {state.step === "confirm" && (
        <Panel title="确认模型信息" subtitle="确认模型基础信息和运行说明，相关信息将随模型包一同输出。" active={active} onRequestClose={routeCloseRequest}>
          <PanelBody>
            <section className="confirm-left">
              {isVoiceConfirm ? (
                <div className="form-grid-1col">
                  <TextField label="模型名称" value={metadata.model_display_name} onChange={(value) => handleMetadataChange({ model_display_name: value })} disabled={disabled} />
                  <TextField label="任务类型" value="语音识别" disabled />
                  <TextField label="识别方式" value="非流式" disabled />
                </div>
              ) : (
              <div className="form-grid-2col">
                <TextField label="模型名字" value={metadata.model_display_name} onChange={(value) => handleMetadataChange({ model_display_name: value })} disabled={disabled} />
                <BoolSelect
                  label="是否支持深度思考"
                  value={metadata.model_supports_thinking}
                  onChange={(value) => handleMetadataChange({ model_supports_thinking: value })}
                  disabled={disabled || metaLocked}
                />
                <NumberField
                  label="模型最大上下文长度"
                  value={metadata.model_max_token}
                  onChange={(value) => handleMetadataChange({ model_max_token: value })}
                  disabled={disabled || metaLocked}
                />
                <BoolSelect
                  label="深度思考是否支持关闭"
                  value={metadata.model_supports_thinking_toggle}
                  onChange={(value) => handleMetadataChange({ model_supports_thinking_toggle: value })}
                  disabled={disabled || metaLocked}
                />
                <BoolSelect
                  label="是否支持工具调用"
                  value={metadata.supports_tool_choice}
                  onChange={(value) => handleMetadataChange({ supports_tool_choice: value })}
                  disabled={disabled || metaLocked}
                />
                <NumberField
                  label="参数量"
                  value={metadata.parameter_count}
                  onChange={(value) => handleMetadataChange({ parameter_count: value })}
                  disabled={disabled || metaLocked}
                />
                <div className="field capability-field" style={{ gridColumn: "1 / -1" }}>
                  <span>模型能力</span>
                  <CapabilitySelect
                    value={metadata.capabilities}
                    onChange={(caps) => handleMetadataChange({ capabilities: caps })}
                    disabled={disabled || metaLocked}
                  />
                </div>
              </div>
              )}
            </section>
            <aside className="cloud-detail confirm-summary">
              <strong className="card-title">模型摘要</strong>
              <div className="summary-model-card">
                <div className="summary-model-info">
                  <div className="summary-model-name">{metadata.model_display_name}</div>
                  <div className="summary-model-sub">模型文件已获取</div>
                </div>
                <CheckCircle2 size={20} className="summary-check" />
              </div>
              <div className="cloud-detail-rows">
                <div className="detail-row">
                  <span>模型类型</span>
                  <strong>{isVoiceConfirm ? "语音模型" : "大语言模型"}</strong>
                </div>
                <div className="detail-row">
                  <span>模型大小</span>
                  <strong>{formatBytes(totalFileSize)}</strong>
                </div>
                <div className="detail-row">
                  <span>信息状态</span>
                  <strong className={confirmInfoConfirmed ? "status-confirmed" : "status-pending"}>
                    {confirmInfoConfirmed ? "已确认" : "待确认"}
                  </strong>
                </div>
                <div className="detail-row">
                  <span>下一步</span>
                  <strong>执行本地打包</strong>
                </div>
                {confirmInfoConfirmed && (
                  <div className="detail-row">
                    <span>输出位置</span>
                    <strong
                      style={{ display: "flex", alignItems: "center", gap: 4, cursor: "pointer" }}
                      onClick={async () => {
                        const dir = await selectOutputDirectory();
                        if (dir) setOutputDir(dir);
                      }}
                    >
                      {outputDir || "选择目录"}
                      <FolderOpen size={14} />
                    </strong>
                  </div>
                )}
              </div>
            </aside>
          </PanelBody>
          <PanelFoot>
            <label className="confirm-checkbox">
              <input
                type="checkbox"
                checked={confirmInfoConfirmed}
                onChange={(event) => handleConfirmCheckboxChange(event.target.checked)}
                disabled={disabled}
              />
              <span>我已确认以上模型信息</span>
            </label>
            <button className="btn btn-gray" onClick={resetAll} disabled={disabled}>
              取消
            </button>
            <button className="btn btn-gray" onClick={() => dispatch({ type: "go", step: state.source === "cloud" ? "cloud" : "local" })} disabled={disabled}>
              上一步
            </button>
            <button
              className="btn btn-primary"
              onClick={() => void packageLocalModel()}
              disabled={!confirmInfoConfirmed || !outputDir || disabled}
            >
              {confirmInfoConfirmed ? "开始打包" : "下一步"}
            </button>
          </PanelFoot>
        </Panel>
      )}

      {state.step === "packaging" && (
        <Panel title="正在打包模型" subtitle="工具正在本机生成标准模型包，请勿关闭工具或移动模型文件" active={active} onRequestClose={routeCloseRequest}>
          <PanelBody>
            <section className="card packaging-left-card">
              <div className="packaging-model-header">
                <strong className="packaging-model-name">{metadata.model_display_name}</strong>
                <span className="packaging-model-output">输出文件：{metadata.model_display_name}</span>
              </div>
              <div className="packaging-progress-section">
                <strong className="packaging-progress-title">正在生成标准模型包</strong>
                <div className="packaging-progress-bar-row">
                  <div className="progress-track" style={{ height: 12 }}>
                    <div className="progress-fill" style={{ width: `${state.packagingProgress}%` }} />
                  </div>
                  <span className="packaging-progress-percent">{state.packagingProgress}%</span>
                </div>
                <span className="packaging-progress-desc">正在整理模型文件和配置信息</span>
              </div>
              <div className="packaging-steps-list">
                {packagingSteps.map((step) => (
                  <div key={step.name} className="packaging-step-row">
                    <span>{step.name}</span>
                    <span className={`packaging-step-status packaging-step-${step.status}`}>
                      {step.status === "done" ? "已完成" : step.status === "active" ? "进行中" : "等待中"}
                    </span>
                  </div>
                ))}
              </div>
            </section>
            <aside className="cloud-detail packaging-info-card">
              <strong className="card-title">打包信息</strong>
              <div className="cloud-detail-rows">
                <div className="detail-row">
                  <span>模型名称</span>
                  <strong>{metadata.model_display_name}</strong>
                </div>
                <div className="detail-row">
                  <span>输出位置</span>
                  <strong>{outputDir}</strong>
                </div>
                <div className="detail-row">
                  <span>模型大小</span>
                  <strong>{totalFileSize > 0 ? formatBytes(totalFileSize) : "22.6 GB"}</strong>
                </div>
                <div className="detail-row">
                  <span>已用时间</span>
                  <strong>{elapsedTime}</strong>
                </div>
              </div>
              <div className="tip-box-local">
                模型较大，打包时间可能受磁盘读写速度影响。
              </div>
            </aside>
          </PanelBody>
          <PanelFoot>
            <button
              className="btn btn-gray"
              onClick={() => setShowCancelPackagingDialog(true)}
            >
              取消
            </button>
            <button className="btn btn-primary" disabled>
              打包中
            </button>
          </PanelFoot>
        </Panel>
      )}

      {state.step === "result" && state.resultKind === "success" && state.result && (
        <Panel title="打包结果" subtitle="模型包已完成输出。" active={active} resultKind={state.resultKind} onRequestClose={routeCloseRequest}>
          <PanelBody colCenter>
            <div className="result-center">
              <div className="result-icon result-icon-success">
                <Check size={32} strokeWidth={3} />
              </div>
              <strong className="result-title">模型打包成功</strong>
              <span className="result-filename">{state.result.package_root.split("/").pop()}</span>
              <div className="result-detail-table">
                <div className="result-detail-row">
                  <span>模型名称</span>
                  <strong>{metadata.model_display_name}</strong>
                </div>
                <div className="result-detail-row">
                  <span>标准模型包输出位置</span>
                  <strong>{state.result.package_root}</strong>
                </div>
                <div className="result-detail-row">
                  <span>模型大小</span>
                  <strong>{totalFileSize > 0 ? formatBytes(totalFileSize) : "22.6 GB"}</strong>
                </div>
                <div className="result-detail-row">
                  <span>完成时间</span>
                  <strong>{new Date().toLocaleString("zh-CN", { year: "numeric", month: "2-digit", day: "2-digit", hour: "2-digit", minute: "2-digit" }).replace(/\//g, "-")}</strong>
                </div>
              </div>
              <div className="result-driver-tip">
                <Info size={16} className="result-driver-tip-icon" />
                <span>
                  模型运行如需匹配设备驱动，请
                  <button className="result-driver-link" onClick={() => void openKylinOsManager()}>
                    前往麒麟管家
                  </button>
                  下载安装。如已安装驱动，可回到
                  <button className="result-driver-link" onClick={() => void openAiModuleManager()}>
                    设置-AI模块管理
                  </button>
                  导入模型包。
                </span>
              </div>
            </div>
          </PanelBody>
          <PanelFoot>
            <button className="btn btn-gray" onClick={handleOpenDirectory}>
              打开所在位置
            </button>
            <button className="btn btn-primary" onClick={resetAll}>
              继续打包模型
            </button>
          </PanelFoot>
        </Panel>
      )}

      {state.step === "result" && state.resultKind === "failure" && (
        <Panel title="打包结果" subtitle="模型包打包失败。" active={active} resultKind={state.resultKind} onRequestClose={routeCloseRequest}>
          <PanelBody colCenter>
            <div className="result-center result-center-failure">
              <div className="result-icon result-icon-failure">
                <X size={20} strokeWidth={3} />
              </div>
              <strong className="result-title">模型打包失败</strong>
              <span className="result-filename">{metadata.model_display_name}</span>
              <span className="result-failure-reason">失败原因：{formatFailureReason(state.packagingError ?? state.error ?? "未知错误")}</span>
            </div>
          </PanelBody>
          <PanelFoot>
            <button className="btn btn-gray" onClick={resetAll}>
              新建打包
            </button>
            <button className="btn btn-gray" onClick={() => dispatch({ type: "go", step: "confirm" })}>
              重试
            </button>
          </PanelFoot>
        </Panel>
      )}

      {showCancelPackagingDialog && (
        <ConfirmDialog
          title="确定要终止打包吗？"
          message="当前打包进度将全部丢失，已生成的临时文件会被清理。"
          confirmText="确定"
          cancelText="继续打包"
          onConfirm={handleCancelPackaging}
          onCancel={() => setShowCancelPackagingDialog(false)}
        />
      )}

      {showExitDialog && (
        <ConfirmDialog
          title="确定要终止打包吗？"
          message="当前打包进度将全部丢失，已生成的临时文件会被清理。"
          confirmText="确定"
          cancelText="继续打包"
          onConfirm={() => void handleConfirmExit()}
          onCancel={() => setShowExitDialog(false)}
        />
      )}

      {confirmError && (
        <ErrorDialog
          message={confirmError}
          onClose={() => setConfirmError(null)}
        />
      )}
    </main>
  );
}

function windowMinimize() {
  if (isTauriRuntime) {
    void getCurrentWindow().minimize().catch((error) => console.error("最小化失败", error));
  }
}

function windowToggleMaximize() {
  if (isTauriRuntime) {
    void getCurrentWindow().toggleMaximize().catch((error) => console.error("最大化失败", error));
  }
}

function destroyWindow() {
  if (!isTauriRuntime) return;
  // v2 用 destroy 绕过 close-requested 拦截（capabilities 已加 core:window:allow-destroy）；
  // v1 API 没有 destroy，其 close() 本身就是直接销毁窗口、不经过拦截，行为等价。
  const win = getCurrentWindow();
  const closing = typeof win.destroy === "function" ? win.destroy() : win.close();
  void closing.catch((error) => console.error("关闭失败", error));
}

function Panel({ title, subtitle, active, resultKind, onRequestClose, children }: { title: string; subtitle?: string; active: number; resultKind?: ResultKind | null; onRequestClose: () => void; children: React.ReactNode }) {
  return (
    <section className="panel">
      <div className="panel-head">
        <div className="panel-titlebar" data-tauri-drag-region>
          <div className="panel-brand" data-tauri-drag-region>
            <img src="/icon.svg" className="panel-brand-icon" alt="模型管理工具" draggable={false} />
            <span className="panel-brand-name">模型管理工具</span>
          </div>
          <div className="win-ctrls">
            <button className="win-ctrl" onClick={windowMinimize} aria-label="最小化">
              <Minus size={14} />
            </button>
            <button className="win-ctrl" onClick={windowToggleMaximize} aria-label="最大化">
              <Square size={12} />
            </button>
            <button className="win-ctrl win-ctrl-close" onClick={onRequestClose} aria-label="关闭">
              <X size={14} />
            </button>
          </div>
        </div>
        <div className="panel-stepper-row">
          <Stepper active={active} resultKind={resultKind} />
        </div>
        <div className="panel-title-row">
          <span className="panel-title">{title}</span>
          {subtitle && <span className="panel-subtitle">{subtitle}</span>}
        </div>
      </div>
      {children}
    </section>
  );
}

function PanelBody({ colCenter, children }: { colCenter?: boolean; children: React.ReactNode }) {
  return <div className={`panel-body${colCenter ? " col-center" : ""}`}>{children}</div>;
}

function PanelFoot({ children }: { children: React.ReactNode }) {
  return <div className="panel-foot">{children}</div>;
}

function Stepper({ active, resultKind }: { active: number; resultKind?: ResultKind | null }) {
  return (
    <div className="stepper">
      {stepLabels.map((label, index) => {
        const isCompleted = index < active;
        const isCurrent = index === active;
        const isResultStep = index === stepLabels.length - 1;
        let dotContent: React.ReactNode = index + 1;
        let dotClass = `step-dot`;
        if (isResultStep && resultKind === "success") {
          dotContent = <Check size={12} />;
          dotClass = `step-dot step-dot-success`;
        } else if (isResultStep && resultKind === "failure") {
          dotContent = <X size={12} />;
          dotClass = `step-dot step-dot-failure`;
        } else if (isCompleted) {
          dotClass = `step-dot active`;
        } else if (isCurrent) {
          dotClass = `step-dot active step-dot-current`;
        }
        return (
          <React.Fragment key={label}>
            <div className="step-item">
              <span className={dotClass}>{dotContent}</span>
              <span className={`step-label${isCompleted || isCurrent ? " active" : ""}`}>{label}</span>
            </div>
            {index < stepLabels.length - 1 && (
              <span className={`step-line${index < active ? " active" : ""}`} />
            )}
          </React.Fragment>
        );
      })}
    </div>
  );
}

function TextField(props: { label: string; value: string; onChange?: (value: string) => void; disabled?: boolean }) {
  return (
    <label className="field">
      <span>{props.label}</span>
      <input value={props.value} onChange={(event) => props.onChange?.(event.target.value)} disabled={props.disabled} />
    </label>
  );
}

function NumberField(props: { label: string; value: number; onChange: (value: number) => void; disabled?: boolean }) {
  return (
    <label className="field">
      <span>{props.label}</span>
      <input
        type="number"
        min={1}
        value={props.value}
        onChange={(event) => props.onChange(Number(event.target.value))}
        disabled={props.disabled}
      />
    </label>
  );
}

function BoolSelect(props: { label: string; value: boolean; onChange: (value: boolean) => void; disabled?: boolean }) {
  return (
    <label className="field">
      <span>{props.label}</span>
      <select
        value={props.value ? "是" : "否"}
        onChange={(event) => props.onChange(event.target.value === "是")}
        disabled={props.disabled}
      >
        <option value="是">是</option>
        <option value="否">否</option>
      </select>
    </label>
  );
}

function Toggle(props: { label: string; checked: boolean; onChange: (value: boolean) => void; disabled?: boolean }) {
  return (
    <label className="toggle">
      <input
        type="checkbox"
        checked={props.checked}
        onChange={(event) => props.onChange(event.target.checked)}
        disabled={props.disabled}
      />
      <span>{props.label}</span>
    </label>
  );
}

function TerminalLog({ lines }: { lines: CloudLogLine[] }) {
  const preRef = useRef<HTMLPreElement>(null);
  useEffect(() => {
    if (preRef.current) {
      preRef.current.scrollTop = preRef.current.scrollHeight;
    }
  }, [lines]);
  return (
    <pre ref={preRef} className="terminal-log">
      {lines.map((line, index) => (
        <div key={index} className="terminal-line">
          <span className="terminal-time">[{line.time}]</span> {line.message}
        </div>
      ))}
      {lines.length === 0 && <span className="terminal-empty">等待输出...</span>}
    </pre>
  );
}

function ConfirmDialog(props: {
  title: string;
  message: string;
  confirmText: string;
  cancelText: string;
  onConfirm: () => void;
  onCancel: () => void;
}) {
  return (
    <div className="modal-overlay">
      <div className="modal-dialog">
        <strong className="modal-title">{props.title}</strong>
        <span className="modal-message">{props.message}</span>
        <div className="modal-actions">
          <button className="btn btn-gray modal-btn" onClick={props.onCancel}>
            {props.cancelText}
          </button>
          <button className="btn btn-primary modal-btn" onClick={props.onConfirm}>
            {props.confirmText}
          </button>
        </div>
      </div>
    </div>
  );
}

function ErrorDialog(props: {
  message: string;
  onClose: () => void;
}) {
  return (
    <div className="modal-overlay">
      <div className="modal-dialog modal-dialog-error">
        <button className="modal-error-close" onClick={props.onClose} aria-label="关闭">
          <X size={16} />
        </button>
        <div className="modal-error-body">
          <Info size={18} className="modal-error-icon" fill="#3790fa" color="#fff" />
          <span className="modal-error-message">{props.message}</span>
        </div>
        <div className="modal-actions">
          <button className="btn btn-primary modal-btn" onClick={props.onClose}>
            确定
          </button>
        </div>
      </div>
    </div>
  );
}

function CapabilitySelect(props: {
  value: string[];
  onChange: (value: string[]) => void;
  disabled?: boolean;
}) {
  const [open, setOpen] = useState(false);
  const containerRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!open) return;
    function handleClickOutside(event: MouseEvent) {
      if (containerRef.current && !containerRef.current.contains(event.target as Node)) {
        setOpen(false);
      }
    }
    document.addEventListener("mousedown", handleClickOutside);
    return () => document.removeEventListener("mousedown", handleClickOutside);
  }, [open]);

  function toggle(cap: string) {
    if (props.value.includes(cap)) {
      props.onChange(props.value.filter((c) => c !== cap));
    } else {
      props.onChange([...props.value, cap]);
    }
  }

  return (
    <div className="field capability-select" ref={containerRef}>
      <div className={`cap-tags-display${props.disabled ? " cap-tags-locked" : ""}`} onClick={() => !props.disabled && setOpen(!open)}>
        {props.value.length === 0 && <span className="card-sub">{props.disabled ? "—" : "点击选择..."}</span>}
        {props.value.map((cap) => (
          <span key={cap} className="cap-tag" onClick={(e) => { e.stopPropagation(); if (!props.disabled) toggle(cap); }}>
            {capabilityLabel(cap)}
            {!props.disabled && <span className="cap-tag-remove">×</span>}
          </span>
        ))}
      </div>
      {open && !props.disabled && (
        <div className="cap-dropdown">
          {CAPABILITY_OPTIONS.map((option) => {
            const isSelected = props.value.includes(option.value);
            return (
              <div
                key={option.value}
                className={`cap-option${isSelected ? " cap-option-selected" : ""}`}
                onClick={() => toggle(option.value)}
              >
                {isSelected && <Check size={14} className="cap-option-check" />}
                <span>{option.label}</span>
              </div>
            );
          })}
        </div>
      )}
    </div>
  );
}

function formatBytes(bytes: number): string {
  if (bytes < 1024) return `${bytes} B`;
  const units = ["KB", "MB", "GB", "TB"];
  let value = bytes / 1024;
  let unitIndex = 0;
  while (value >= 1024 && unitIndex < units.length - 1) {
    value /= 1024;
    unitIndex += 1;
  }
  return `${value.toFixed(1)} ${units[unitIndex]}`;
}

function formatSpeed(bytesPerSecond: number): string {
  return formatBytes(bytesPerSecond) + "/s";
}

createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);
