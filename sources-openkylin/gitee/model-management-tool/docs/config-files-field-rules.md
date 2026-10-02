# config.pbtxt 与 model_description.yaml 字段填充规则

本文档整理打包工具在生成模型包时，对 `config.pbtxt` 和 `model_description.yaml` 两个文件的字段填充规则。

- 生成入口：`src-tauri/src/packager.rs`
- `config.pbtxt` 由 `render_config_pbtxt(draft, base_model)`（packager.rs:760，按 `draft.model_kind` 分派 llm/语音模板）或 `render_config_from_original(original, draft, base_model)`（packager.rs:1006）生成
- `model_description.yaml` 由 `render_model_description(draft)`（packager.rs:1282，语音走样例分支）或 `render_description_from_original(original, draft)`（packager.rs:1121）生成
- 模型类型 `ModelKind`（llm/asr/tts）与识别方式 `AsrMode`（streaming/once）由确认页经 `PackageDraft.model_kind` / `asr_mode` 传入；类型判定规则见 `docs/cloud-download-rules.md` 第六节
- 语音模型字段规则以最新仓库样例为准：`/home/v11/pkgtool/model-repository/asr_Qwen3-ASR-0.6b-1_1.0`、`asr_Qwen3-ASR-1.7b-1_1.0`（各含 `config.pbtxt` + `model_description.yaml`）

## 原始配置文件优先策略

打包时优先使用**原有配置文件**作为基础，仅更新必要的字段并对缺少的信息按模板规则补充：

| 场景 | 行为 |
| --- | --- |
| 云端下载 | 使用暂存目录里下载的 `config.pbtxt` / `model_description.yaml`，将用户确认信息写入其中后再打包 |
| 本地已有配置文件 | 通过 `discover_config_files`（`packager.rs`）在模型目录（或其父目录）发现 `config.pbtxt` / `model_description.yaml`，作为生成基础 |
| 无配置文件 | 回退到模板生成（`render_config_pbtxt` / `render_model_description`） |

- `generate_package` 中 `draft.config_file` / `draft.description_file` 指向原始文件时，先读取原文，再调用 `render_config_from_original` / `render_description_from_original` 处理。
- 处理逻辑：字段/参数分三类——**受管**（始终以草稿/表单值覆盖，缺失追加）、**补充**（原文有非空值保留，缺失或值为空补默认值）、**非受管**（原样保留），详见 1.4 与 2.5。

数据来源有三类：

| 来源 | 说明 |
| --- | --- |
| 固定值 | 模板中写死的常量 |
| 用户编辑元数据 `EditableMetadata` | 确认信息页的表单字段 |
| 模型包草稿 `PackageDraft` | 模型主目录名、品牌、已发现的模型文件（base/lora/mmproj） |

---

## 一、config.pbtxt

### 1.1 固定不变部分

| 字段 | 值 | 说明 |
| --- | --- | --- |
| `backend` | `"llamacpp"` | 后端引擎，固定 |
| `model_transaction_policy.decoupled` | `True` | 解耦模式，固定 |
| `instance_group.count` | `1` | 实例数，固定 |
| `instance_group.kind` | `KIND_CPU` | CPU 实例，固定 |
| `input` | 13 个固定输入 | `request_type`、`request_id`、`text_input`、`system_input`、`temperature_input`、`top_k_input`、`top_p_input`、`n_keep_input`、`n_predict_input`、`cache_prompt_input`、`stop_input`、`stream_input`、`lora_scale_input` |
| `output` | 4 个固定输出 | `text_output`、`token_output`、`lora_scale_output`、`metrics_output` |

各输入输出的 `data_type`、`dims`、`optional` 固定，模板不随用户元数据变化：

- 必填输入：`request_type`(INT32)、`text_input`(STRING)、`stop_input`(STRING)、`stream_input`(BOOL)
- 可选输入（`optional: true`）：`request_id`、`system_input`、`temperature_input`、`top_k_input`、`top_p_input`、`n_keep_input`、`n_predict_input`、`cache_prompt_input`、`lora_scale_input`

### 1.2 parameters 固定值（LLM 模板默认值）

下表为 LLM 模板参数取值。**有原始配置时这些参数不再强制覆盖**：原文有非空值一律保留，仅在 key 缺失或 `string_value` 为空时按下表默认值补充（见 1.4）；从零生成时全部按下表输出。

| 参数 key | 值 | 说明 |
| --- | --- | --- |
| `n_threads` | `"1"` | 线程数，模板默认 |
| `n_predict` | `"40960"` | 最大生成 token 数，模板默认 |
| `n_ctx` | `"128000"` | 上下文窗口大小，模板默认（**不随用户填写的 `config_max_token` 变化**） |
| `n_batch` | `"512"` | 批大小，模板默认 |
| `n_parallel` | `"1"` | 并行数，模板默认 |
| `n_threads_http` | `"32"` | HTTP 线程数，模板默认（LLM 专有） |
| `cont_batching` | `"true"` | 连续批处理，模板默认 |
| `metrics_period_ms` | `"200"` | 指标上报周期，模板默认 |
| `lora_init_without_apply` | `"false"` | 模板默认 |
| `worker_save_path` | `""` | 默认为空 |
| `use_jinja` | `"true"` | 模板默认 |
| `chat_template_file` | jinja 安装路径 / `""` | 包内存在 `chat_template.jinja` 时为 `{INSTALL_ROOT}/{model_name}/1/{jinja 文件名}`，否则为空（发现规则见 1.3.1） |
| `mmproj_use_gpu` | `"true"` | 模板默认 |
| `llamacpp_verbose` | `"false"` | 模板默认 |

### 1.3 parameters 动态值

以下参数根据模型包草稿动态填充（`template_parameters`，`packager.rs:691`）。使用原始配置文件时，这 4 个动态 key 的 `string_value` 会被**强制覆盖**为当前草稿值（LLM/语音一致）；其余固定参数（1.2 / 1.5）保留原文值，仅缺失或为空时按默认补充；原始配置中缺失的模板参数在文件末尾按默认值追加：

| 参数 key | 填充规则 |
| --- | --- |
| `model` | 基座模型安装路径：`/opt/appdata/kylin-ai/model-repository/{model_name}/1/{base_model.file_name}` |
| `lora` | 所有 LoRA 模型安装路径，用 `;` 连接：`/opt/appdata/kylin-ai/model-repository/{model_name}/1/{lora.file_name}`；无 LoRA 时为空字符串 |
| `mmproj` | 多模态投射器安装路径：`/opt/appdata/kylin-ai/model-repository/{model_name}/1/{mmproj.file_name}`；无 mmproj 时为空字符串 |
| `model_decrypt_type` | 基座模型文件名以 `.gguf` 结尾时为 `"0"`，否则为 `"1"` |

安装根目录常量：`INSTALL_ROOT = /opt/appdata/kylin-ai/model-repository`（`packager.rs:11`）。

#### 1.3.1 chat_template_file 取值

`chat_template_file` 属于固定参数（原文非空保留、缺失/为空补默认，见 1.4），但其模板默认值不再是空串，而是按 `chat_template_install_path`（`packager.rs`）计算的 jinja 安装路径：

1. 优先取 `draft.chat_template_file`（本地：5.4 发现；云端：5.2 暂存路径），文件存在（`is_file`）时用其文件名拼安装路径 `{INSTALL_ROOT}/{model_name}/1/{文件名}`；
2. 未提供或文件不存在时，回退检查 `draft.source_dir` 下的 `chat_template.jinja` 与 `1/chat_template.jinja`（仓库式目录布局下 jinja 位于 `1/`，`discover_config_files` 以目录为入参时不会发现它，但 `copy_source_tree` 会将其复制进包），命中则用 `chat_template.jinja` 拼安装路径；
3. 均未命中输出空串（后端使用 GGUF 内嵌模板）。

LLM 与语音模板同一规则。

### 1.4 原始 config.pbtxt 的处理

`render_config_from_original(original, draft, base_model)`（packager.rs:1006，LLM/语音统一规则）：

- 逐块扫描 `parameters { ... }` 块并按 key 匹配；替换时**在块内定位含 `string_value:` 的行**，兼容单行 `value: { string_value: ... }` 与多行块（`string_value` 独立成行，最新仓库样例为多行格式）两种写法。
- **动态 key**（`model`/`lora`/`mmproj`/`model_decrypt_type`）：始终覆盖为草稿计算值（新安装路径/解密类型）。
- **固定参数**（1.2 LLM / 1.5 语音参数表）：原文有非空值**保留原文**；`string_value` 为空按缺失处理，覆盖为模板默认值。
- 模板参数表中原文缺失的 key，在文件末尾以标准块格式追加（**参数表按模型类型分派**：语音用 1.5 的语音参数表，不追加 `n_threads_http` 等 LLM 专有参数）。
- 原文中非受管参数（如厂商自定义参数）与其他内容（backend、input/output、instance_group、`model_transaction_policy`、注释行等）原样保留。

### 1.5 语音模型 config.pbtxt

最新仓库样例：`model-repository/asr_Qwen3-ASR-0.6b-1_1.0`、`asr_Qwen3-ASR-1.7b-1_1.0`（parameters 为多行块格式、含中文注释）。

- **无原始配置**：`render_config_pbtxt_voice`（packager.rs:770）按样例生成：
  - 无 `model_transaction_policy`（非流式识别一请求一响应，无需 decoupled；模板不输出、缺失不追加，原始配置中已有时原样保留）；
  - `input` 为 12 个张量：同 LLM 但**无 `system_input`**，且 **`stop_input` 为 `optional: true`**（LLM 模板中 `stop_input` 必填）；
  - `output` 4 个、`instance_group` 与 LLM 模板相同；
   - 固定参数补充默认值：`n_threads=4`、`n_predict=1024`、`n_ctx=2048`、`n_batch=512`、`n_parallel=1`、`cont_batching=true`、`metrics_period_ms=200`、`lora_init_without_apply=false`、`worker_save_path=""`、`use_jinja=true`、`chat_template_file`（按 1.3.1 规则计算，无 jinja 时为空）、`mmproj_use_gpu=true`、`llamacpp_verbose=false`；**不含 `n_threads_http` 等 LLM 专有参数**；
  - 动态 key 同 1.3；其中 `mmproj` 为语音（音频）encoder 路径，Qwen3-ASR 系列必需。
- **固定参数取值随模型不同**：样例中 `n_predict` 0.6b 为 `512`、1.7b 为 `1024`（`n_ctx` 均为 2048，长音频需业务侧分段）。模板值 `1024` 仅作为无原始配置、或原文缺失/值为空时的补充默认值。
- **有原始配置**：走 1.4 统一规则——仅覆盖动态 key，固定参数与注释保留原文，缺失参数按上述默认补充。

---

## 二、model_description.yaml

### 2.1 固定值字段

| 字段 | 值 | 说明 |
| --- | --- | --- |
| `model_capability` | `["llm"]` | 仅 LLM 从零生成模板输出；有原文时原样保留（语音见 2.4） |
| `model_is3rdModel` | `"true"` | 受管字段：LLM/语音均强制覆盖为 `"true"`，缺失追加 |
| `model_internal` | `"false"` | LLM 从零生成模板输出；语音为补充字段（原文保留，缺失/空补 `"false"`，见 2.4） |
| `model_is_private` | `"true"` | **仅 LLM 从零生成输出**；最新语音样例已移除该字段——语音不输出、不追加，原文有则原样保留 |
| `brand` | `"qwen"` | **补充字段**：原文有非空值保留，缺失/值为空补 `"qwen"`；**不再取 `draft.brand`**（LLM/语音一致） |

### 2.2 动态字段（来自 PackageDraft）

| 字段 | 填充规则 | 来源 |
| --- | --- | --- |
| `model_name` | 模型主目录名（如 `llm_demo_0.1`），需符合 `<llm/asr/tts>_<name>_<version-or-quant>` | `draft.model_name` |

> `draft.brand` **不再写入** model_description.yaml（brand 改为补充字段，默认 `"qwen"`，见 2.1），仅用于确认页展示与云端预填。

### 2.3 动态字段（来自 EditableMetadata）

| 字段 | 填充规则 | 对应确认页表单 |
| --- | --- | --- |
| `model_deb_name` | 模型管理名称，需转义反斜杠与双引号 | 打包时固定取 `model_name`（`main.tsx:544`） |
| `model_display_name` | 模型显示名字，需转义 | “模型名字” |
| `model_supports_thinking` | 是否支持深度思考 | “是否支持深度思考” |
| `model_supports_thinking_toggle` | 深度思考是否支持关闭 | “深度思考是否支持关闭” |
| `others_info.modelMaxToken` | 模型最大上下文长度 | “模型最大上下文长度” |
| `others_info.configMaxToken` | 配置最大 Token 数 | 表单同 `model_max_token` 默认 4096 |
| `others_info.supportsToolChoice` | 是否支持工具调用（JSON 布尔 `true`/`false`，不加引号） | “是否支持工具调用” |
| `others_info.parameterCount` | 参数量 | “参数量” |
| `others_info.capabilities` | 模型能力数组（如 `["text","tool"]`） | “模型能力”多选 |
| `others_info.lora` | **仅当存在 LoRA 模型时**追加 `"lora":["GET_LABEL","INTENT"]`；无 LoRA 时省略 | 由 `draft.lora_models` 非空决定 |

### 2.4 语音模型分支

最新仓库样例（`asr_Qwen3-ASR-0.6b-1_1.0` / `asr_Qwen3-ASR-1.7b-1_1.0` 的 `model_description.yaml`）字段依次为：`model_deb_name`、`model_name`、`model_display_name`、`model_capability`、`model_is3rdModel`、`model_internal`、`brand`、`model_supports_thinking`、`model_supports_thinking_toggle`、`others_info`（均带行尾中文注释）。

- `model_capability`：**补充字段**——原文有非空值保留能力名（样例为 `["once_asr"]`，兼容 `asr`/`streaming_asr` 等变体，含行尾注释）；缺失或值为空时按确认页识别方式生成：asr → `["once_asr"]`/`["streaming_asr"]`（识别方式固定非流式，即 `once_asr`），tts → `["tts"]`。
- `others_info`：**补充字段**——原文有非空值保留；缺失或值为空（如旧样例的 `''`）时补 `"{\"isBuiltin\":false}"`（**isBuiltin 已移入 others_info JSON 内**，不再是顶层字段）。不用表单值覆盖。
- `model_internal`：**补充字段**——原文保留，缺失/值为空补 `"false"`。
- `brand`：同 2.1——原文保留，缺失/值为空补 `"qwen"`。
- `model_is3rdModel`：受管字段，强制 `"true"`（与最新样例一致）。
- `model_supports_thinking` / `model_supports_thinking_toggle`：受管字段，按确认页取值（语音确认页无这两项，取默认/预填值）；语音输出 YAML 布尔（不带引号，与样例一致），LLM 保持带引号字符串。
- `model_deb_name`：仍固定取 `model_name`（样例的 `ai-kylin-*` 命名与 model_name 不同，打包时会被改写为目录名——既有规则，暂不改）。
- 顶层 `isBuiltin`、`model_is_private`：**最新样例已移除**——被动处理：模板不输出、缺失不追加，原始配置中存在时按非受管字段原样保留。
- 无原始配置时 `render_model_description` 语音分支按样例结构生成（`model_is3rdModel: "true"`、`model_internal: "false"`、`brand: "qwen"`、思考类字段为裸布尔 `false`、`others_info: "{\"isBuiltin\":false}"`）。

### 2.5 原始 model_description.yaml 的处理

`render_description_from_original(original, draft)`（packager.rs:1121）将字段分三类处理：

| 类别 | 规则 | LLM 字段 | 语音字段 |
| --- | --- | --- | --- |
| 受管字段 | 始终以草稿/表单值覆盖，缺失时末尾追加 | `model_deb_name`、`model_name`、`model_display_name`、`model_is3rdModel`、`model_supports_thinking`、`model_supports_thinking_toggle`、`others_info` | `model_deb_name`、`model_name`、`model_display_name`、`model_is3rdModel`、`model_supports_thinking`、`model_supports_thinking_toggle` |
| 补充字段 | 原文有非空值保留（含行尾注释）；缺失**或值为空**时按默认值补充/覆盖 | `brand`（`"qwen"`） | `model_capability`（映射值）、`model_internal`（`"false"`）、`brand`（`"qwen"`）、`others_info`（`"{\"isBuiltin\":false}"`） |
| 非受管字段 | 原样保留 | `model_capability`、`model_internal`、`model_is_private` 及自定义字段 | 顶层 `isBuiltin`、`model_is_private` 及自定义字段 |

- `model_is3rdModel` 强制固定为 `"true"`：本地补充模型信息与云端下载的模型打包时，原文中该字段的值都会被覆盖（LLM/语音一致）。
- 受管字段覆盖为整行重写，原文行尾注释会丢失；补充字段原文有值时整行保留，注释不丢。

### 2.6 others_info JSON 结构

`others_info` 是单引号包裹的 JSON 字符串，完整结构（有 LoRA 时）：

```
"{\"modelMaxToken\":<model_max_token>,\"configMaxToken\":<config_max_token>,
  \"supportsToolChoice\":true|false,\"parameterCount\":<parameter_count>,
  \"capabilities\":[\"cap1\",\"cap2\",...],\"lora\":[\"GET_LABEL\",\"INTENT\"]}"
```

无 LoRA 时 `\"lora\":[...]` 整个省略。

> 以上结构仅 LLM 使用（受管字段，按表单值覆盖）。语音模型 `others_info` 为补充字段：原文有非空值保留，缺失/值为空补 `"{\"isBuiltin\":false}"`（见 2.4）。

### 2.7 字符串转义

- `model_deb_name`、`model_name`、`model_display_name`：使用 `yaml_escape`，将 `\` 替换为 `\\`，`"` 替换为 `\"`（packager.rs:1343）。`brand` 已改为固定默认值 `"qwen"`，不再经转义写入。
- `capabilities` 数组元素：同样经 `yaml_escape` 后以 `\"...\"` 形式拼进 JSON。

---

## 三、元数据校验规则

`EditableMetadata::validate(kind)`（packager.rs:126）在打包前校验，任一失败则打包中止；**语音模型（asr/tts）仅校验模型名字与管理名称**，跳过能力/上下文/参数量校验（确认页无这些字段）：

| 字段 | 校验规则 | 错误信息 |
| --- | --- | --- |
| `model_display_name` | 去空格后非空 | “模型名字不能为空” |
| `model_deb_name` | 去空格后非空 | “模型管理名称不能为空” |
| `capabilities` | 非空且每个元素去空格后非空 | “模型能力不能为空” |
| `model_max_token` | 必须为正整数 | “模型最大上下文长度必须为正整数” |
| `config_max_token` | 必须为正整数 | “配置最大Token数必须为正整数” |
| `parameter_count` | 必须为正整数 | “参数量必须为正整数” |

模型主目录名校验 `validate_model_name(name, kind)`（`packager.rs:250`）：以下划线分割恰好 3 段、各段非空，**首段前缀按模型类型决定**：llm → `llm_`、asr → `asr_`、tts → `tts_`（如 `asr_Qwen3-ASR-0.6b-1_1.0`）。

---

## 四、前端字段来源与默认值

| 字段 | 默认值 | 来源 |
| --- | --- | --- |
| `model_name` | `llm_demo_0.1` | 本地：由基座文件去扩展名推断（`llm_xxx_x.y` 或 `llm_<文件名>_1.0`），若附近存在 `model_description.yaml` 则优先取其 `model_name`；云端：下载后读取暂存目录中的 `model_description.yaml` 取 `model_name` |
| `brand` | `Kylin` | 本地固定；云端：模型厂商 `vendor`，随后可被云端描述 `brand` 覆盖。**仅用于确认页展示/预填，不再写入 model_description.yaml**（brand 按补充规则处理，默认 `"qwen"`，见 2.1） |
| `model_display_name` | `Kylin Demo Model` | 本地：由基座文件名推断；云端：云端描述 `model_display_name` |
| `capabilities` | `["text"]` | 云端：`others_info.capabilities` 优先，否则 `model_capability` |
| `model_supports_thinking` | `false` | 云端描述 |
| `model_supports_thinking_toggle` | `false` | 云端描述 |
| `model_max_token` | `4096` | 云端 `others_info.modelMaxToken`，缺省保留默认 |
| `config_max_token` | `4096` | 云端 `others_info.configMaxToken`，缺省保留默认 |
| `supports_tool_choice` | `false` | 云端 `others_info.supportsToolChoice`（读取时兼容 JSON 布尔与字符串 `"true"`/`"false"`） |
| `parameter_count` | `1000000000` | 云端 `others_info.parameterCount`，缺省保留默认 |

---

## 五、模型文件处理流程（下载产物与本地导入）

本节整理云端下载与本地导入之后，模型权重文件的**发现、分类、暂存与原始配置文件衔接**流程。该流程产出的 `ModelFile` 列表与配置文件路径最终填入 `PackageDraft`，作为前述字段渲染与打包复制的基础。

- 发现/分类入口：`discover_model_files` / `discover_path` / `classify_model_file`（`packager.rs:261` / `:306` / `:324`）
- 云端下载入口：`download_cloud_model`（`cloud.rs:578`）
- 本地导入入口：`loadLocalPaths`（`main.tsx:272`）
- 命令注册：`src-tauri/src/main.rs`

### 5.1 模型文件发现与分类

`discover_model_files(paths)` 对传入路径**递归遍历**（`discover_path`，`packager.rs:306`）：路径为文件时直接分类，为目录时遍历其下所有条目继续递归。每个文件经 `classify_model_file`（`packager.rs:324`）判定：

| 判定步骤 | 规则 |
| --- | --- |
| 扩展名过滤 | 仅 `.gguf` / `.bin` / `.safetensors`（小写比较）被识别；其他扩展名直接返回 `None`，**不会进入模型文件列表**（`packager.rs:326`） |
| 角色分类 | 按文件名（小写形式）前缀判定（`packager.rs:332`）：<br>• `mmproj*` → `Mmproj`<br>• `lora_*` → `Lora`（注意：此处用**原始大小写**文件名匹配 `lora_` 前缀）<br>• 其余 → `Base` |
| 文件信息 | `ModelFile { path, file_name, role, size }`（packager.rs:89），`size` 取 `std::fs::metadata` 的文件长度 |
| 排序 | 全部结果按 `file_name` 字典序排序（`packager.rs:266`） |

> `config.pbtxt`、`model_description.yaml` 及任意非权重文件因扩展名不在白名单内，**不会被分类为模型文件**，因此不进入 base/lora/mmproj 列表；它们通过 5.4 的配置文件发现流程单独处理。

### 5.2 云端下载产物与暂存目录

云端模型下载到**暂存目录**而非直接进入模型包：

| 项目 | 规则 |
| --- | --- |
| 暂存目录 | `outputDir/.cloud-staging/<subPath>/`，由前端拼接（`main.tsx:363`），后端 `download_cloud_model` 中 `create_dir_all`（`cloud.rs:584`） |
| 下载范围 | 清单中的全部文件（权重 + `config.pbtxt` + `model_description.yaml` 等），均写入暂存目录（详见 `docs/cloud-download-rules.md`） |
| 下载后分类 | 下载完成后对暂存目录调用 `discover_model_files(&[staging_dir])`（`cloud.rs:735`），**仅识别权重文件**并返回 `Vec<ModelFile>` |
| 前端落地 | 返回的模型文件经 `download_cloud_completed` 写入 `state.localFiles`（`main.tsx:369`、`workflow.ts:157`） |
| 配置文件路径 | 前端直接将暂存目录下的配置文件路径写死设置（`main.tsx:371`）：<br>• `configFilePath = staging_dir/config.pbtxt`<br>• `descriptionFilePath = staging_dir/model_description.yaml`<br>• `chatTemplateFilePath = staging_dir/chat_template.jinja`（文件可能不存在，打包时后端按 `is_file` 判断） |

因此云端路径下，`PackageDraft.config_file` / `description_file` 指向暂存目录中**下载下来的原始配置文件**，打包时走 `render_config_from_original` / `render_description_from_original`（见“原始配置文件优先策略”一节）。

### 5.3 本地导入流程（仅目录）

本地导入**只支持选择目录**，入口两种，汇聚到 `loadLocalPaths(paths)`（`main.tsx`）：

| 入口 | 实现 | 说明 |
| --- | --- | --- |
| 选择模型目录按钮 | `select_local_model_directory_dialog`（`main.rs:57`） | 原生目录选择器，返回单个目录路径 |
| 拖拽 | Tauri `onDragDropEvent` + HTML `drop`（`main.tsx`） | 同时监听原生拖拽（含路径）与浏览器拖拽（回退用 `file.path`） |

页面顶部新增必填单选"*选择模型类型"（大语言模型/语音模型），**无默认选中，未选时"下一步"禁用**；单选值仅作为无 `model_description.yaml` 时确认页表单形态的来源，确认页预填以 description 的 `model_capability` 优先，最终取值以确认页为准。

`loadLocalPaths` 处理顺序（**单目录语义：新选择/拖入直接替换旧结果**）：

1. 清空旧选择（`reset_local_selection`）与错误态；
2. `paths.length > 1` → 红色可关闭错误条"一次仅支持上传一个文件目录"，不加载任何路径；
3. `inspectLocalPath(paths[0])` → 命令 `inspect_local_path`（`main.rs:31`）判断目录；为文件 → 红色可关闭错误条"仅支持上传文件目录，单个文件无法上传"；
4. 目录 → `discoverLocalFiles([dir])` 递归分类；结果为空 → "目录名称"警告卡"目录内未检测到.gguf 模型文件，请重新上传"，右栏模型文件=未发现（橙）、下一步禁用；不含 `Base` 角色 → 同警告卡"未发现基座模型文件，请重新上传"；
5. 成功 → "目录名称"成功卡（路径 + 总大小 + 绿✓），右栏模型文件=已发现；`dispatch set_local_files` 保存分类结果；
6. `findModelConfigFiles([dir])` → `discover_config_files` 发现配置文件，写入 `configFilePath` / `descriptionFilePath`。

> 目录选择会触发递归遍历，目录下任意层级的权重文件都会被发现；非权重文件在读取阶段被忽略，但打包阶段会随源目录全量保留（见 5.6）。

### 5.4 配置文件发现

`discover_config_files(paths)`（`packager.rs:270`）在模型文件所在目录附近查找 `config.pbtxt`、`model_description.yaml` 与 `chat_template.jinja`：

| 路径类型 | 候选目录 |
| --- | --- |
| 目录 | 该目录本身 |
| 文件 | 文件所在父目录 + 祖父目录（两级） |

- 对每个候选目录依次检查 `config.pbtxt`、`model_description.yaml`、`chat_template.jinja` 是否存在；**首次命中即记录**，不再覆盖。
- 三个文件均命中时立即停止遍历。
- 返回 `ModelConfigFiles { config_file, description_file, chat_template_file }`，任一可能为 `None`。
- **云端场景不走该函数**：5.2 中暂存目录下的配置文件路径由前端直接拼接设置。

### 5.5 与 PackageDraft 的衔接

确认信息页确认后，`packageLocalModel`（`main.tsx:402`）构造 `PackageDraft`：

| 草稿字段 | 来源 |
| --- | --- |
| `base_model` | `pickBaseModel(localFiles)` —— `role == "base"` 的第一项（`workflow.ts:260`） |
| `lora_models` | `pickLoraModels(localFiles)` —— 所有 `role == "lora"`（`workflow.ts:264`） |
| `mmproj_model` | `pickMmprojModel(localFiles)` —— `role == "mmproj"` 的第一项（`workflow.ts:268`） |
| `metadata.model_deb_name` | 打包时**强制取 `model_name`**（`main.tsx`），忽略表单其他输入 |
| `model_kind` / `asr_mode` | llm / asr（语音模型固定为 asr，确认页任务类型/识别方式固定显示"语音识别/非流式"）；asr 时 `asr_mode` 固定 once，llm 为 null |
| `config_file` | `configFilePath`（本地：5.4 发现；云端：5.2 暂存路径） |
| `description_file` | `descriptionFilePath`（同上） |
| `chat_template_file` | `chatTemplateFilePath`（同上） |
| `source_dir` | 打包源目录：本地=用户选择目录；云端=暂存目录（见 5.6） |
| `output_dir` | `outputDir`（默认下载目录，可在确认页“输出位置”重选） |

打包前 `validate_draft`（`packager.rs:516`）额外校验：`model_name` 前缀符合 `model_kind`（见第三节）、`base_model` 必须存在（否则报“缺少基座模型文件”；**mmproj 不做强制校验**）。`generate_package`（`packager.rs:350`）按 5.6 的复制规则将源目录文件复制到 `<output_dir>/<model_name>/1/`，最后按“原始配置文件优先策略”渲染 `config.pbtxt` / `model_description.yaml` 到模型包根目录。

### 5.6 源目录全量保留复制规则

`draft.source_dir` 存在且为目录时（本地选择目录、云端暂存目录均适用），`copy_source_tree`（`packager.rs:532`）递归复制其中**全部文件**：

| 源内相对路径 | 包内位置 |
| --- | --- |
| 以 `1/` 开头（仓库式目录，权重在 `1/` 下） | 原样映射：`<model_name>/<相对路径>`（即包 `1/` 下） |
| 其余（平铺目录、子目录如 `docs/x.txt`） | 自动前缀 `1/`：`<model_name>/1/<相对路径>` |

- **仅排除** `draft.config_file` / `draft.description_file` 引用的两份原始配置（按 canonical 路径比对）——它们渲染后输出到包根目录，不再原样复制；子目录中的同名文件不被排除。
- `chat_template.jinja` 不再特判，随全量复制自然落入 `1/`；渲染 config.pbtxt 时按 1.3.1 规则将其安装路径写入 `chat_template_file` 参数（原文缺失/为空时补充）。
- 两种目录布局产物一致：`<model_name>/1/<权重及其余文件>` + 根目录两份渲染配置。
- `source_dir` 缺失时回退旧逻辑（仅复制 base/lora/mmproj + chat_template）。
- 复制过程支持取消与逐文件进度；目标已存在且大小相同则跳过（重试场景）。
