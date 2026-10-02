# 云端下载规则

本文档整理云端模型下载的文件来源、范围与进度计算规则。

- 实现位置：`src-tauri/src/cloud.rs`
- 入口命令：`download_cloud_model`（`cloud.rs:668`）

---

## 一、文件清单来源

模型可下载哪些文件，来自云端仓库分支清单：

`https://archive2.kylinos.cn/LLM/KYLIN/branches/default.json`（`cloud.rs:7`）

清单结构（`BranchManifest` / `RemoteModel`，`cloud.rs:73`）：

```
models:
  <模型名>:
    subPath: <子路径>
    descrip: <描述>
    files:
      <文件名>: { size, sha256, time? }
```

每个模型的 `files` 即为该模型可下载的完整文件清单。

## 二、下载范围：全部文件

下载时对清单 `files` 不做任何扩展名过滤，`request.files` 中的全部文件都会被下载（`cloud.rs:576`）。包括：

- 权重文件：`.gguf` / `.bin` / `.safetensors`
- 配置文件：`config.pbtxt`
- 描述文件：`model_description.yaml`
- 其他清单中列出的文件

文件不足时（清单为空）返回错误“未发现可下载的文件”（`cloud.rs:580`）。

## 三、下载地址

每个文件按模型子路径拼接：

`https://archive2.kylinos.cn/LLM/KYLIN/models/<subPath>/<文件名>`（`cloud.rs:667`）

## 四、断点续传与校验

- 暂存文件为 `<file>.part`，续传时以 `Range: bytes=<已有字节>-` 请求并追加写入（`download_file`）。
- 续传前校验：
  - 已有字节超过预期 `size` 时，说明远端文件变更或残留脏数据，删除 `.part` 后从头下载。
  - 服务器返回 `206` 时校验 `Content-Range` 的起始偏移必须与请求一致；不一致（服务器忽略 Range 或偏移不符）则删除 `.part` 从头重下，最多重下一次，避免死循环（`content_range_start`）。
  - 服务器返回 `200`（忽略 Range）时按全量下载处理。
- 下载完成后按清单 `sha256` + `size` 校验，通过后 `.part` 改名为最终文件（`finalize_file`）。
- 校验失败说明 `.part` 内容损坏：删除 `.part` 后自动从头重下一次（`download_one_file`），仍失败才中止。
- 已存在的最终文件会先校验，通过则跳过下载；不通过则删除最终文件与残留 `.part`。
- 整体进度以所有 `.part` 的已有字节数为初始值，续传时进度不会归零。

## 五、下载进度计算

整体进度按**所有待下载文件**的大小合计计算（`cloud.rs:584`）：

- `overall_size` = 全部文件 `size` 之和
- `overall_downloaded` = 已下载字节累计
- `overall_progress` = `overall_downloaded / overall_size × 100`

单文件进度 = 当前文件已下载字节 / 当前文件 `size` × 100（`emit_progress`，`cloud.rs:748`）。

## 六、模型列表与类型判定（语音模型）

仓库中的 asr/tts 模型与 nlp 模型同页展示，列表/下载/进度 UI 完全一致，仅行描述与确认页按类型区分。

- **capability 获取**：`list_cloud_models`（`cloud.rs:191`）对清单中每个模型并发拉取 `model_description.yaml`，解析 `model_capability` 填入 `CloudModel.model_capability`；同一份文本复用于后摩过滤（见第十节），拉取失败的模型不在列表中展示。
- **类型判定规则**（前端 `detectModelKind`，`workflow.ts`）：capability 任一项含 `asr`（`once_asr`/`streaming_asr`/`asr` 均算）→ 语音识别；否则任一项含 `tts` → 语音合成；其余 → 大语言模型。识别方式：含 `streaming_asr` → 流式，否则（`once_asr`/`asr`）→ 非流式。
- **列表行描述**：语音识别 → "语音识别模型 版本 1.0 约 <大小>"；语音合成 → "语音合成模型 …"；其余 → "通用语言大模型 …"。
- **描述解析**：`CloudModelDescription` 新增 `model_capability` 字段（`parse_model_description_yaml` / 前端 `parseModelDescriptionYaml` 同步解析），供确认页决定表单形态与预填。

## 七、模型信息使用规则

云端模型信息（`model_description.yaml`）完整可靠，使用规则：

- **确认页展示**：下载完成后前端读取暂存目录 `outputDir/.cloud-staging/<subPath>/model_description.yaml`（`read_model_description_file`），将其中信息预填到确认信息页表单。仓库模型描述文件信息完整，确认页只允许修改 `model_display_name`（模型名字），其余字段一律只读展示（`main.tsx` 中 `metaLocked = state.source === "cloud"`，禁用各表单控件；`CapabilitySelect` 禁用时隐藏删除叉、显示灰底）。语音模型（capability 含 asr/tts）确认页展示语音表单（模型名称/任务类型/识别方式），任务类型与识别方式为固定展示值"语音识别/非流式"（不可修改，打包固定按 asr + once 取值）。确认页字段为最终打包取值。
- **字段取值来源**：`capabilities`、`model_supports_thinking`、`model_supports_thinking_toggle`、`model_max_token`、`config_max_token`、`supports_tool_choice`、`parameter_count` 直接取下载的 `model_description.yaml`，用户不可修改；`model_display_name` 以确认信息页用户填写的值为准。
- **能力缺失兜底**：仓库模型若描述文件缺少 `capabilities`（描述文件下载异常），确认信息时提示"模型描述文件缺少模型能力，请重新下载模型"，需重新下载而非在页面上补填。
- **其他字段**：`model_name`、`brand` 等，直接使用下载文件/云端描述自带的值。

前端在选中云端模型时先通过 `fetch_model_description` 拉取远端 `model_description.yaml` 预填表单（`main.tsx`），下载完成后再次读取本地下载的 `model_description.yaml` 覆盖预填。

## 八、下载产物与打包衔接

全部文件下载到 `outputDir/.cloud-staging/<subPath>/` 后，调用 `discover_model_files`（`cloud.rs`）对暂存目录分类，仅将权重文件（`.gguf/.bin/.safetensors`）识别为 base/lora/mmproj 并构造 `PackageDraft`；`config.pbtxt`、`model_description.yaml` 等不会进入模型文件列表。

打包时前端将暂存目录中的 `config.pbtxt`、`model_description.yaml`、`chat_template.jinja` 路径通过 `draft.config_file` / `draft.description_file` / `draft.chat_template_file` 传入，`generate_package` 以前两份**下载的配置文件为原始基础**（`render_config_from_original` / `render_description_from_original`）：把用户确认的信息写入其中，并按当前规则补充缺失字段，然后输出到模型包。`chat_template.jinja` 若存在于暂存目录，则原样复制到模型包 `1/` 目录（与权重文件放在一起），且渲染 config.pbtxt 时其安装路径会写入 `chat_template_file` 参数（原文缺失/为空时补充，规则见 `docs/config-files-field-rules.md` 1.3.1）；不存在则跳过。用户可直接在暂存目录查看下载下来的原始配置文件。

## 九、暂存目录清理规则

暂存目录 `outputDir/.cloud-staging/<subPath>/` 由后端 `clean_staging_dir`（`cloud.rs`）删除，前端通过 `clean_cloud_staging` 命令（`main.rs`）调用，封装为 `cleanCloudStaging`（`lib/api.ts`）。安全约束：

- 仅允许删除路径中含 `.cloud-staging` 的目录，防止误删；目录不存在视为成功（幂等）。
- 命令内部先置下载取消标志，并等待进行中的下载任务退出（`CloudDownloadState.active`，最长 10s）后再删除，避免边删边写。

**需要清理的场景**（前端 `main.tsx`，暂存目录记录在 `stagingRef`）：

| 场景 | 触发点 |
|------|--------|
| 打包成功（文件已复制进模型包，暂存冗余） | `packageLocalModel` 成功分支 |
| 下载中点 "x" 停止下载（不保留续传） | `handleStopDownload` |
| 回到首页（各页"取消"、失败页"新建打包"、成功页"继续打包模型"） | `resetAll` |
| 关闭窗口（下载中/下载完成后/打包中确认退出），先清理再销毁 | `routeCloseRequest` / `destroyWindowWithCleanup` |
| 模型列表切换选中其他模型（清理旧模型暂存） | `handleSelectCloudModel` |
| 强杀/断电/崩溃残留（兜底） | 应用启动 `setup` 中 `clean_staging_root` 清理下载目录下的 `.cloud-staging` |

**不清理的场景**：暂停下载（保留 `.part` 供续传）、取消打包回到模型信息确认页、确认页点"上一步"、打包失败页"重试"、确认页修改输出位置。

## 十、模型列表硬件过滤（后摩卡）

仓库模型列表（`list_cloud_models`）按本机是否装有后摩卡过滤：

- **后摩卡检测**（`device.rs`）：内置 kyml 设备白名单（`KYML_WHITELIST`，内容取自 `/usr/share/kyml/kyml_whitelists.txt`，不依赖 kyml 包安装），仅取后摩设备项（`kyml_plugin_name` 或 `vendor` 含 `houmo`，如 `0x1f6b|0x0c00|...|houmo|houmo|HOUMO.AI|LQ50`），再扫描 `/sys/bus/pci/devices` 下各设备的 `vendor`/`device` 文件匹配 vendor_id + device_id（与 kyml `pci_sysfs_find_by_vendor_device` 一致）；任一匹配即判定有后摩卡。目录不可读或无匹配视为无后摩卡。
- **模型类型判断**：`model_description.yaml` 的 `model_name` 字段格式固定为 `llm_<核心名>_<版本>`（如 `llm_Qwen-3.6-35ba3b-1_1.0`）。去掉前缀 `llm_` 与版本后缀后，核心名以 `-1` 结尾为后摩模型，`-0` 结尾为普通模型。
- **过滤规则**（`should_show_model`）：`list_cloud_models` 并发拉取每个模型的 `model_description.yaml`，后摩机器只展示后摩模型，非后摩机器只展示普通模型；描述拉取失败（无法判定类型）的模型一律不展示。
- **测试开关**：环境变量 `MMT_FORCE_HOUMO_CARD=1`（强制视为有后摩卡，只展示后摩模型）或 `=0`（强制视为无后摩卡，只展示普通模型），设置后跳过 PCI 扫描；未设置或取值非 0/1 时按实际硬件检测。
