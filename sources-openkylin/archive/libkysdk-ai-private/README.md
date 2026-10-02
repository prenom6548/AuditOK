# libkyai-config

#### 介绍
本仓库提供 Kylin AI 相关的配置与助手能力库，包括：
- `kyai-config`：模型配置与配置服务代理相关能力
- `kyai-assistant`：助手服务代理与 OS Assistant 相关能力

#### 软件架构
- `ai-config/`：配置库实现与安装文件
- `ai-assistant/`：助手库实现与安装文件
- `debian/`：打包相关配置

#### 依赖
- CMake >= 3.5
- C++17 编译器
- `jsoncpp`
- `glib` / `gio-unix-2.0`
- `KylinAiProto`

#### 编译安装
1. 生成构建目录
2. 编译
3. 安装

```bash
cmake -S . -B build
cmake --build build
cmake --install build
```

#### 构建选项
- `BUILD_AI_CONFIG`：是否构建 `kyai-config`（默认 `ON`）
- `BUILD_AI_ASSISTANT`：是否构建 `kyai-assistant`（默认 `ON`）
- `ENABLE_TEST`：是否启用测试（默认 `OFF`）

示例：只构建 `kyai-config`
```bash
cmake -S . -B build -DBUILD_AI_ASSISTANT=OFF
cmake --build build
```

#### 使用说明
- 头文件安装在 `include/kylin-ai/` 下
- 库文件安装在系统库目录（例如 `lib`）

#### 参与贡献
1. Fork 本仓库
2. 新建 `Feat_xxx` 分支
3. 提交代码
4. 新建 Pull Request
