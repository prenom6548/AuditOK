import { describe, expect, it } from "vitest";
import { cloudRowModelDesc, deriveModelDirectoryName, deriveVoiceModelDirectoryName, detectAsrMode, detectModelKind, formatCloudDownloadError, initialState, isNetworkErrorMessage, isValidModelDirectoryName, parentDirectory, pickBaseModel, pickLoraModels, reducer, splitPathInput } from "../lib/workflow";
import type { CloudModel, ModelFile } from "../types";

const files: ModelFile[] = [
  {
    path: "/tmp/base_model_0.1.gguf",
    file_name: "base_model_0.1.gguf",
    role: "base"
  },
  {
    path: "/tmp/lora_modelA_0.1.gguf",
    file_name: "lora_modelA_0.1.gguf",
    role: "lora"
  }
];

const cloudModel: CloudModel = {
  id: "Qwen3.6-35B-A3B",
  name: "Qwen3.6-35B-A3B",
  vendor: "Kylin 云端仓库",
  parameter_count: 0,
  capabilities: ["text"],
  model_capability: ["llm"],
  sub_path: "Qwen3.6-35B-A3B",
  description: "Qwen3.6-35B-A3B",
  files: [
    { name: "Qwen3.6-35B-A3B-Q4_K_M.gguf", size: 21166757728, sha256: "bbb" },
    { name: "config.pbtxt", size: 4027, sha256: "aaa" }
  ]
};

describe("workflow", () => {
  it("switches from start to local import", () => {
    const state = reducer(initialState, { type: "choose_source", source: "local" });
    expect(state.step).toBe("local");
    expect(state.source).toBe("local");
  });

  it("selects a source without leaving the start step", () => {
    const cloud = reducer(initialState, { type: "select_source", source: "cloud" });
    expect(cloud.step).toBe("start");
    expect(cloud.source).toBe("cloud");

    const local = reducer(cloud, { type: "select_source", source: "local" });
    expect(local.step).toBe("start");
    expect(local.source).toBe("local");
  });

  it("stays on local step after local files are read", () => {
    const state = reducer(initialState, { type: "set_local_files", files });
    expect(state.step).toBe("local");
    expect(pickBaseModel(state.localFiles)?.file_name).toBe("base_model_0.1.gguf");
    expect(pickLoraModels(state.localFiles)).toHaveLength(1);
  });

  it("reset_local_selection clears local files and directory", () => {
    const selected = reducer(initialState, { type: "set_local_files", files });
    const withDir = reducer(selected, { type: "set_model_directory", directory: "/tmp" });
    const reset = reducer(withDir, { type: "reset_local_selection" });
    expect(reset.localFiles).toHaveLength(0);
    expect(reset.modelDirectory).toBe(null);
  });

  it("selects a cloud model and resets progress", () => {
    const state = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    expect(state.source).toBe("cloud");
    expect(state.cloudStatus).toBe("selected");
    expect(state.selectedCloudModel?.sub_path).toBe("Qwen3.6-35B-A3B");
    expect(state.cloudProgress).toBe(0);
  });

  it("tracks real cloud download progress", () => {
    const selected = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    const downloading = reducer(selected, { type: "start_cloud_download" });
    expect(downloading.cloudStatus).toBe("downloading");
    expect(downloading.isBusy).toBe(true);

    const progressing = reducer(downloading, {
      type: "set_cloud_file_progress",
      progress: {
        file_name: "Qwen3.6-35B-A3B-Q4_K_M.gguf",
        downloaded_bytes: 1000,
        file_size: 21166757728,
        file_progress: 1,
        overall_downloaded: 1000,
        overall_size: 21166757728,
        overall_progress: 1
      },
      speed: 0
    });
    expect(progressing.cloudCurrentFile).toBe("Qwen3.6-35B-A3B-Q4_K_M.gguf");
    expect(progressing.cloudOverallProgress).toBe(1);
  });

  it("marks cloud download completed and waits for user to proceed", () => {
    const selected = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    const downloading = reducer(selected, { type: "start_cloud_download" });
    const completed = reducer(downloading, { type: "download_cloud_completed", files });

    expect(completed.source).toBe("cloud");
    expect(completed.localFiles).toBe(files);
    expect(completed.cloudStatus).toBe("completed");
    expect(completed.cloudProgress).toBe(100);
    expect(completed.isBusy).toBe(false);

    const confirmed = reducer(completed, { type: "go", step: "confirm" });
    expect(confirmed.step).toBe("confirm");
  });

  it("pauses cloud download and keeps progress for resume", () => {
    const selected = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    const downloading = reducer(selected, { type: "start_cloud_download" });
    const progressing = reducer(downloading, {
      type: "set_cloud_file_progress",
      progress: {
        file_name: "model.gguf",
        downloaded_bytes: 500,
        file_size: 1000,
        file_progress: 50,
        overall_downloaded: 500,
        overall_size: 1000,
        overall_progress: 50
      },
      speed: 1024
    });

    const paused = reducer(progressing, { type: "pause_cloud_download" });
    expect(paused.cloudStatus).toBe("paused");
    expect(paused.isBusy).toBe(false);
    expect(paused.cloudDownloadSpeed).toBe(0);
    expect(paused.cloudOverallProgress).toBe(50);

    const resumed = reducer(paused, { type: "start_cloud_download" });
    expect(resumed.cloudStatus).toBe("downloading");
    expect(resumed.isBusy).toBe(true);
    expect(resumed.cloudOverallProgress).toBe(50);
  });

  it("stops cloud download and returns to the pre-download selected state", () => {
    const selected = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    const downloading = reducer(selected, { type: "start_cloud_download" });
    const progressing = reducer(downloading, {
      type: "set_cloud_file_progress",
      progress: {
        file_name: "model.gguf",
        downloaded_bytes: 500,
        file_size: 1000,
        file_progress: 50,
        overall_downloaded: 500,
        overall_size: 1000,
        overall_progress: 50
      },
      speed: 1024
    });

    const paused = reducer(progressing, { type: "pause_cloud_download" });
    const stopped = reducer(paused, { type: "stop_cloud_download" });
    expect(stopped.cloudStatus).toBe("selected");
    expect(stopped.isBusy).toBe(false);
    expect(stopped.cloudOverallProgress).toBe(0);
    expect(stopped.cloudLogLines).toHaveLength(0);
    expect(stopped.selectedCloudModel?.sub_path).toBe("Qwen3.6-35B-A3B");
    expect(stopped.error).toBe(null);
  });

  it("marks cloud download errors", () => {
    const selected = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    const downloading = reducer(selected, { type: "start_cloud_download" });
    const failed = reducer(downloading, { type: "cloud_unavailable", error: "校验失败" });

    expect(failed.cloudStatus).toBe("error");
    expect(failed.isBusy).toBe(false);
    expect(failed.error).toBe("校验失败");
  });

  it("maps network error messages to friendly Chinese text", () => {
    expect(isNetworkErrorMessage("error sending request for url (https://x/): dns error")).toBe(true);
    expect(isNetworkErrorMessage("error trying to connect: tcp connect error: Connection refused (os error 111)")).toBe(true);
    expect(isNetworkErrorMessage("SHA-256 校验失败: a != b")).toBe(false);

    expect(formatCloudDownloadError("error sending request for url (https://x/): operation timed out")).toBe("网络连接异常");
    expect(formatCloudDownloadError("下载未完成: 100 / 200")).toBe("网络连接异常");
    expect(formatCloudDownloadError("No space left on device (os error 28)")).toBe("磁盘空间不足");
    expect(formatCloudDownloadError("磁盘可用空间不足: 需要 1 字节，可用 0 字节")).toBe("磁盘空间不足");
    expect(formatCloudDownloadError("下载失败: HTTP 404")).toBe("服务器响应异常（HTTP 404）");
    expect(formatCloudDownloadError("服务器返回 206 但缺少 Content-Range 头")).toBe("服务器响应异常");
    expect(formatCloudDownloadError("SHA-256 校验失败: a != b")).toBe("文件校验失败");
    expect(formatCloudDownloadError("未发现可下载的文件")).toBe("未发现可下载的文件");
    expect(formatCloudDownloadError("some unknown english error")).toBe("下载异常，请稍后重试");
  });

  it("splits pasted or dragged path input", () => {
    expect(splitPathInput("/tmp/a.gguf\n\n/tmp/b.gguf ")).toEqual(["/tmp/a.gguf", "/tmp/b.gguf"]);
  });

  it("sets cloud list status to no-network", () => {
    const state = reducer(initialState, { type: "set_cloud_list_status", status: "no-network" });
    expect(state.cloudListStatus).toBe("no-network");
  });

  it("sets cloud list status to ready", () => {
    const noNet = reducer(initialState, { type: "set_cloud_list_status", status: "no-network" });
    const ready = reducer(noNet, { type: "set_cloud_list_status", status: "ready" });
    expect(ready.cloudListStatus).toBe("ready");
  });

  it("appends cloud log lines and truncates beyond 500", () => {
    let state = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    for (let i = 0; i < 501; i++) {
      state = reducer(state, {
        type: "append_cloud_log",
        line: { time: `00:00:${String(i).padStart(2, "0")}`, message: `line ${i}` }
      });
    }
    expect(state.cloudLogLines).toHaveLength(500);
    expect(state.cloudLogLines[0].message).toBe("line 1");
    expect(state.cloudLogLines[499].message).toBe("line 500");
  });

  it("set_cloud_file_progress appends a log line", () => {
    const selected = reducer(initialState, { type: "select_cloud_model", model: cloudModel });
    const downloading = reducer(selected, { type: "start_cloud_download" });
    expect(downloading.cloudLogLines).toHaveLength(0);

    const progressing = reducer(downloading, {
      type: "set_cloud_file_progress",
      progress: {
        file_name: "model.gguf",
        downloaded_bytes: 500,
        file_size: 1000,
        file_progress: 50,
        overall_downloaded: 500,
        overall_size: 1000,
        overall_progress: 50
      },
      speed: 1024
    });
    expect(progressing.cloudLogLines).toHaveLength(1);
    expect(progressing.cloudDownloadSpeed).toBe(1024);
  });

  it("handles package_cancelled state flow", () => {
    const packaging = reducer(initialState, { type: "start_packaging" });
    const state = reducer(packaging, { type: "package_cancelled" });
    expect(state.step).toBe("confirm");
    expect(state.resultKind).toBe(null);
    expect(state.packagingCancelled).toBe(true);
    expect(state.isBusy).toBe(false);
    expect(state.packagingProgress).toBe(0);
  });

  it("handles package_failed state flow", () => {
    const state = reducer(initialState, {
      type: "package_failed",
      error: "磁盘空间不足"
    });
    expect(state.step).toBe("result");
    expect(state.resultKind).toBe("failure");
    expect(state.packagingError).toBe("磁盘空间不足");
    expect(state.isBusy).toBe(false);
  });

  it("package_success sets resultKind to success", () => {
    const result = { package_root: "/tmp/out/llm_demo_0.1", generated_files: ["/tmp/out/llm_demo_0.1/config.pbtxt"] };
    const state = reducer(initialState, { type: "package_success", result });
    expect(state.step).toBe("result");
    expect(state.resultKind).toBe("success");
    expect(state.result).toBe(result);
  });

  it("start_packaging resets packaging state", () => {
    let state = reducer(initialState, { type: "start_packaging" });
    state = reducer(state, {
      type: "set_packaging_progress",
      progress: { step: "copy", message: "复制中...", progress: 30 }
    });
    expect(state.packagingProgress).toBe(30);
    expect(state.packagingLogLines).toHaveLength(1);

    const restarted = reducer(state, { type: "start_packaging" });
    expect(restarted.packagingProgress).toBe(0);
    expect(restarted.packagingLogLines).toHaveLength(0);
    expect(restarted.packagingCancelled).toBe(false);
    expect(restarted.resultKind).toBe(null);
  });

  it("set_packaging_progress updates progress and appends log", () => {
    let state = reducer(initialState, { type: "start_packaging" });
    state = reducer(state, {
      type: "set_packaging_progress",
      progress: { step: "config", message: "生成 config.pbtxt...", progress: 60 }
    });
    expect(state.packagingProgress).toBe(60);
    expect(state.packagingLogLines).toHaveLength(1);
    expect(state.packagingLogLines[0].message).toContain("config.pbtxt");
  });

  it("parentDirectory returns the directory containing the package", () => {
    expect(parentDirectory("/tmp/out/llm_demo_0.1")).toBe("/tmp/out");
    expect(parentDirectory("/tmp/out/llm_demo_0.1/")).toBe("/tmp/out");
    expect(parentDirectory("/llm_demo_0.1")).toBe("/");
    expect(parentDirectory("C:\\models\\llm_demo_0.1")).toBe("C:\\models");
    expect(parentDirectory("C:\\llm_demo_0.1")).toBe("C:\\");
  });

  it("derives a valid main directory name from gguf stems with underscores", () => {
    expect(deriveModelDirectoryName("qilin0.6.0_qwen3_4b_Q4-1")).toBe("llm_qilin0.6.0-qwen3-4b-Q4-1_1.0");
    expect(deriveModelDirectoryName("Qwen3.6-35B-A3B-Q4_K_M")).toBe("llm_Qwen3.6-35B-A3B-Q4-K-M_1.0");
    expect(deriveModelDirectoryName("base_model_0.1")).toBe("llm_base-model_0.1");
    expect(deriveModelDirectoryName("model")).toBe("llm_model_1.0");
    expect(deriveModelDirectoryName("")).toBe("llm_demo_0.1");
  });

  it("derives voice main directory names with asr/tts prefix", () => {
    expect(deriveVoiceModelDirectoryName("HiModel_xh2_qwen3-asr_0.6b_b1", "asr")).toBe("asr_HiModel-xh2-qwen3-asr-0.6b-b1_1.0");
    expect(deriveVoiceModelDirectoryName("qilin0.6.0_qwen3_4b_Q4-1", "asr")).toBe("asr_qilin0.6.0-qwen3-4b-Q4-1_1.0");
    expect(deriveVoiceModelDirectoryName("llm_qwen3_4b_1.0", "asr")).toBe("asr_qwen3-4b_1.0");
    expect(deriveVoiceModelDirectoryName("model", "tts")).toBe("tts_model_1.0");
    expect(deriveVoiceModelDirectoryName("asr_Qwen3-ASR-0.6b-1_1.0", "asr")).toBe("asr_Qwen3-ASR-0.6b-1_1.0");
    expect(deriveVoiceModelDirectoryName("tts_demo_2.0", "tts")).toBe("tts_demo_2.0");
    expect(deriveVoiceModelDirectoryName("asr_foo_1.0", "tts")).toBe("tts_foo_1.0");
    expect(deriveVoiceModelDirectoryName("", "asr")).toBe("asr_demo_1.0");
  });

  it("keeps already valid main directory names untouched", () => {
    expect(deriveModelDirectoryName("llm_demo_0.1")).toBe("llm_demo_0.1");
    expect(deriveModelDirectoryName("llm_Qwen-3.6-35ba3b-1_1.0")).toBe("llm_Qwen-3.6-35ba3b-1_1.0");
    expect(deriveModelDirectoryName("llm_qwen3_4b")).toBe("llm_qwen3_4b");
  });

  it("normalizes llm-prefixed stems with extra segments", () => {
    expect(deriveModelDirectoryName("llm_qwen3_4b_1.0")).toBe("llm_qwen3-4b_1.0");
    expect(deriveModelDirectoryName("llm_qwen3_4b_Q4-1")).toBe("llm_qwen3-4b-Q4-1_1.0");
  });

  it("normalizes voice-prefixed stems to llm directory names", () => {
    expect(deriveModelDirectoryName("asr_Qwen3-ASR-0.6b-1_1.0")).toBe("llm_Qwen3-ASR-0.6b-1_1.0");
    expect(deriveModelDirectoryName("tts_demo_2.0")).toBe("llm_demo_2.0");
  });

  it("detects model kind from model_capability", () => {
    expect(detectModelKind(["once_asr"])).toBe("asr");
    expect(detectModelKind(["streaming_asr"])).toBe("asr");
    expect(detectModelKind(["asr"])).toBe("asr");
    expect(detectModelKind(["kylin_tts_v2"])).toBe("tts");
    expect(detectModelKind(["llm"])).toBe("llm");
    expect(detectModelKind(["text", "tool"])).toBe("llm");
    expect(detectModelKind([])).toBe("llm");
    expect(detectModelKind(["asr", "tts"])).toBe("asr");
  });

  it("maps model kind to cloud row description", () => {
    expect(cloudRowModelDesc("asr")).toBe("语音识别模型");
    expect(cloudRowModelDesc("tts")).toBe("语音合成模型");
    expect(cloudRowModelDesc("llm")).toBe("通用语言大模型");
  });

  it("detects asr mode from model_capability", () => {
    expect(detectAsrMode(["streaming_asr"])).toBe("streaming");
    expect(detectAsrMode(["once_asr"])).toBe("once");
    expect(detectAsrMode(["asr"])).toBe("once");
    expect(detectAsrMode([])).toBe("once");
  });

  it("every derived name passes the llm_<name>_<version> shape check", () => {
    const stems = [
      "qilin0.6.0_qwen3_4b_Q4-1",
      "Qwen3.6-35B-A3B-Q4_K_M",
      "base_model_0.1",
      "llm_qwen3_4b_1.0",
      "model",
      "a__b"
    ];
    for (const stem of stems) {
      const name = deriveModelDirectoryName(stem);
      expect(isValidModelDirectoryName(name)).toBe(true);
      expect(name).toBe(deriveModelDirectoryName(name));
    }
  });
});
