# 海光平台安全能力定义

## 文件说明

- `hygon_capability.c` - 能力库插件实现
- `hygon_capabilities.json` - 能力定义 JSON 文件
- `CMakeLists.txt` - 构建配置

## 能力列表

### 1. TCM (可信密码模块)
- **ID**: `tcm_support`
- **描述**: 中国国家可信密码模块支持
- **CPU 要求**: 海光 CPU (`vendor_id == 'HygonGenuine'`)
- **固件要求**: UEFI 固件（可选）
- **OS 要求**: 内核配置启用 `CONFIG_TCG_TPM`
- **依赖**: 无

### 2. CSV (中国安全虚拟化)
- **ID**: `csv_virtualization`
- **描述**: 面向中国平台的安全虚拟化技术
- **CPU 要求**: 支持 SVM (`flags contains 'svm'`)
- **固件要求**: 无
- **OS 要求**: 加载 `kvm_amd` 模块
- **依赖**: `tcm_support`

### 3. IOMMU 保护
- **ID**: `iommu_protection`
- **描述**: 输入输出内存管理单元，用于 DMA 保护
- **CPU 要求**: 支持 SVM (`flags contains 'svm'`)
- **固件要求**: 无
- **OS 要求**: 内核配置启用 `CONFIG_IOMMU`
- **依赖**: 无

## 自定义能力

您可以通过编辑 `hygon_capabilities.json` 文件来自定义能力定义，无需重新编译代码。

JSON 格式示例：
```json
{
    "id": "能力唯一标识符",
    "name": "能力名称",
    "description": "能力描述",
    "cpu_rule": {
        "expression": "CPU 条件表达式",
        "required": true/false
    },
    "firmware_rule": {
        "expression": "固件条件表达式",
        "required": true/false
    },
    "os_rule": {
        "expression": "OS 条件表达式",
        "required": true/false
    },
    "dependencies": ["依赖的能力 ID 列表"]
}
```

## 安装位置

安装后，JSON 文件将位于：`/usr/local/share/plat-sec-patrol/capabilities/hygon_capabilities.json`

插件会自动从标准路径加载此文件。

## 外部工具依赖声明（external_tools）

能力库 JSON 根级可通过 `external_tools` 数组声明能力判定所依赖的外部工具。核心层在 `check` 时检查这些工具是否存在（在 PATH 中查找且具有可执行权限），缺失时在检测结果下方输出警告与安装指导；JSON 输出格式下警告走 stderr，保证 stdout 为纯净 JSON。

字段定义：

| 字段              | 必填 | 说明                                       |
| ----------------- | ---- | ------------------------------------------ |
| `name`            | 是   | 工具标识，如 `hag`                         |
| `description`     | 否   | 用途说明（仅输出到日志，供排查使用）       |
| `check_command`   | 否   | 存在性检查命令，缺省回退为 `name`          |
| `install_guide`   | 否   | 缺失时向用户展示的安装指导                 |
| `verify_command`  | 否   | 安装后的验证命令                           |

当前声明：

```json
"external_tools": [
    {
        "name": "hag",
        "description": "Hygon HAG (Hygon Authentication Gateway), provides security firmware version and TKM/TDM status",
        "check_command": "hag",
        "install_guide": "sudo wget -c https://download.sourcefind.cn:65024/directlink/8/c86-devkit/previous-release/bin/hag -O /usr/local/bin/hag && sudo chmod +x /usr/local/bin/hag",
        "verify_command": "sudo hag general check"
    }
]
```

HAG 提供 `firmware_version`、`tkm_status`、`tdm_status` 三项环境信息，影响 csv1、csv2、csv3、tcm2.0、tkm、tdm 等能力的判定准确性。新增其他外部工具依赖时，只需在此数组中追加声明，核心层无需修改代码。