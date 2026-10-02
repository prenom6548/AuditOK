# ThorVG 1.1.0 openKylin 打包改动说明

## 1. 对照范围

本文档以 Debian 的 `thorvg 1.0.6+dfsg-1` 打包目录为基线：

- 参考目录：<https://sources.debian.org/src/thorvg/1.0.6%2Bdfsg-1/debian/>
- 当前上游版本：`1.1.0`
- 当前 openKylin 包版本：`1.1.0-ok1`
- 目标发行版：`huanghe`
- 打包仓库：<https://gitee.com/openkylin/thorvg>
- 打包分支：`openkylin/huanghe`

这里的“原版”均指上述 Debian `1.0.6+dfsg-1` 的 `debian/`，不是 ThorVG
上游源码本身。

## 2. 总体变化

| 状态 | 文件 | 变化目的 |
| --- | --- | --- |
| 修改 | `changelog` | 建立 openKylin `huanghe` 的首个 1.1.0 版本记录 |
| 修改 | `control` | 切换维护者、发行仓库及 openKylin 兼容元数据 |
| 修改 | `copyright` | 适配 1.1.0 已经更新的测试字体及版权信息 |
| 修改 | `rules` | 删除旧字体注入逻辑，补齐 openKylin 构建和加固参数 |
| 修改 | `libthorvg1.symbols` | 与 1.1.0 实际导出 ABI 对齐 |
| 修改 | `libthorvg1.lintian-overrides` | 保留 SONAME 例外，调整说明用语 |
| 修改 | `watch` | 改用 huanghe 工具链支持的 watch v4，并取消 `+dfsg` 重打包 |
| 删除 | `README.source` | 旧 DFSG 重打包说明已经不适用 |
| 删除 | `gbp.conf` | Debian Salsa/gbp 分支配置不适用于 openKylin 仓库 |
| 删除 | `salsa-ci.yml` | Salsa CI 配置不适用于 Gitee/openKylin 流程 |
| 删除 | `patches/*` | 1.1.0 上游已经完成测试字体替换 |
| 删除 | `source/include-binaries` | `debian/` 中不再携带二进制字体 |
| 删除 | `test/PublicSans-Regular.ttf` | 1.1.0 上游源码已经自带该自由字体 |
| 不变 | 三个 `*.install` 文件 | 1.1.0 的安装路径和二进制包拆分方式未变 |
| 不变 | `thorvg-tools.manpages` 和两个 manpage | 工具名称和命令行用法仍然匹配 1.1.0 |
| 不变 | `source/format` | 继续采用 `3.0 (quilt)` 源码包格式 |
| 不变 | `upstream/metadata` | 上游项目、问题跟踪和发布记录仍位于 GitHub |
| 新增 | `README.openkylin.md` | 记录本文所述的移植依据和维护注意事项 |

## 3. 修改文件详解

### 3.1 `debian/changelog`

旧文件中的 Debian 历史记录全部删除，仅保留新的 openKylin 条目：

```text
thorvg (1.1.0-ok1) huanghe; urgency=medium
```

具体变化及原因：

- 上游版本从 `1.0.6` 更新到 `1.1.0`。
- Debian 修订号 `-1` 改为 openKylin 修订号 `-ok1`。
- 发行目标从 `unstable` 改为 `huanghe`。
- 不再使用 `+dfsg`，因为 1.1.0 上游已经删除旧的 `Arial.ttf` 和
  `NanumGothicCoding.ttf`，无需再次裁剪上游源码包。
- 删除旧 changelog 是本次打包的明确要求；该包从 openKylin 的首个 1.1.0
  条目开始维护。
- 签名改为 `iaom <zhangpengfei@kylinos.cn>`。

### 3.2 `debian/control`

源码包字段的变化如下：

- 新增 `Priority: optional`，显式声明包优先级。
- `Maintainer` 从 Debian 维护者改为
  `iaom <zhangpengfei@kylinos.cn>`，表示当前 openKylin 包的直接维护责任。
- 删除原 `Uploaders`。当前由一个明确的 openKylin 维护者负责，不再把旧 Debian
  协作上传者列为当前上传者。
- 不设置 `XSBC-Original-Maintainer`。当前包直接使用 ThorVG GitHub 上游源码，
  并在 openKylin 仓库中建立自己的 `debian/`，不属于从 Debian 源码包修改而来的
  衍生版本。对 Debian 打包工作的参考和署名保留在 `debian/copyright` 及本文档中。
- `Standards-Version` 从 `4.7.4` 调整为 `4.6.2`，与 huanghe 当前提供的
  packaging-policy/Lintian 版本保持兼容，避免将本地工具链尚未识别的规范版本写入包。
- 新增 `Rules-Requires-Root: no`，声明整个构建过程不需要真实 root 权限。
- `Vcs-Browser` 从 Debian Salsa 改为当前 openKylin 分支网页：
  `https://gitee.com/openkylin/thorvg/tree/openkylin/huanghe`。
- `Vcs-Git` 改为
  `https://gitee.com/openkylin/thorvg.git -b openkylin/huanghe`。这里指向的是
  发行版打包仓库及其打包分支，不是 ThorVG 上游 GitHub 仓库。
- `Build-Depends`、三个二进制包名、架构、Multi-Arch、依赖变量和包描述均保持不变；
  已证明这些定义仍适用于 1.1.0。

### 3.3 `debian/copyright`

保留 Debian 原文件中的完整许可证正文和第三方代码归属，仅修改与 1.1.0
源码树有关的部分：

- 删除 `Files-Excluded` 中的 `Arial.ttf` 和 `NanumGothicCoding.ttf`。这两个文件
  已经不在 1.1.0 上游源码中，因此当前源码包不再是 DFSG 重打包版本。
- 在 `Files: debian/*` 中加入当前打包者
  `iaom <zhangpengfei@kylinos.cn>`。
- Public Sans 的路径从 Debian 自带的
  `debian/test/PublicSans-Regular.ttf` 改为上游自带的
  `test/resources/PublicSans-Regular.ttf`。
- 新增 `test/resources/DMSans.otf` 的版权和 `SIL-1.1` 许可证记录。

### 3.4 `debian/rules`

保留了 `debhelper`、Meson 以及全部 loader、saver、CPU/GL engine、C API、工具和
测试的构建方式。改动如下：

- 删除创建 `test/resources` 以及从 `debian/test` 复制 Public Sans 的命令。
  1.1.0 已经在上游测试资源目录内提供所需字体。
- 检测到 huanghe 当前的 `dpkg-buildflags` 没有提供默认 C++/CPP 编译参数，因此在
  这些字段为空时补充：
  - 正常构建使用 `-O3 -g`；
  - `DEB_BUILD_OPTIONS=noopt` 时使用 `-O0 -g`；
  - 增加 Fortify、stack protector、格式字符串检查、RELRO 和 immediate binding
    等加固参数。
- i386 的 `-msse2 -mfpmath=sse` 从覆盖赋值改为追加赋值，避免清除前面设置的
  优化和加固参数。
- 新增 `-Db_pie=true`，使命令行工具以 PIE 方式构建。
- 新增 `-Dstrip=false`，由 `dh_strip` 统一完成剥离并生成标准 dbgsym 包，避免
  Meson 提前剥离调试信息。
- 清理阶段改为删除测试生成的 `test/resources/test.gif`。该文件由 GIF saver
  测试产生，必须移除才能保证源码树在构建后可再次生成源码包。

### 3.5 `debian/libthorvg1.symbols`

该文件依据 1.1.0 优化构建后的真实动态符号重新核对：

- 新增 10 个 1.1.0 导出符号，最低版本均标记为 `1.1.0`，主要涉及 Lottie
  tween、区域相交查询和带 context 的 WebGPU target API。
- 删除两个 1.0.x 中存在、但 1.1.0 已不再导出的 C++ 符号；对应接口在 1.1.0
  中改成了带附加参数的新签名。
- 将符号文件元字段从错误的复数形式
  `Build-Depends-Packages` 修正为 Lintian 识别的
  `Build-Depends-Package`。
- 32/64 位条件符号仅按生成结果重新排序，最低版本没有改变。

需要注意：上游 1.1.0 在 SONAME 仍为 `libthorvg-1.so.1` 的情况下移除了两个旧
C++ 导出符号。这属于潜在 ABI 兼容风险，因此在正式进入仓库前，建议对依赖
`libthorvg1` 的现有包进行一次反向依赖重构建测试。

### 3.6 `debian/libthorvg1.lintian-overrides`

继续保留 `package-name-doesnt-match-sonames` override。上游把主版本同时放进库名和
SONAME，实际 SONAME 为 `libthorvg-1.so.1`；如果机械生成包名会成为重复主版本的
`libthorvg-1-1`，因此继续使用较清晰的 `libthorvg1`。

唯一文本变化是把注释中的 “Debian package” 改为 “binary package”，使说明对
Debian 和 openKylin 都成立；override 行本身没有改变。

### 3.7 `debian/watch`

- 从 Debian 新工具链使用的 watch v5 GitHub 模板，改为 huanghe 当前
  `devscripts` 支持的 watch v4 显式匹配规则。
- 仍然监视 ThorVG 上游 GitHub tags，因为 `watch` 负责发现上游新版本；它不应
  指向 openKylin 的发行版打包仓库。
- 删除 `Repacksuffix: +dfsg` 和对应版本转换逻辑，因为 1.1.0 不再需要删除旧字体。
- 下载文件名规范化为 `thorvg-<version>.tar.gz`。

## 4. 删除文件详解

### 4.1 `debian/README.source`

旧文件只说明如何从上游 tarball 删除 `Arial.ttf` 和
`NanumGothicCoding.ttf`。1.1.0 已不包含它们，保留该说明反而会误导后续维护者。

### 4.2 `debian/gbp.conf`

旧配置固定使用 `debian/unstable`、`upstream/1.0.x+dfsg` 和 `pristine-tar`，是
Debian Salsa 的 gbp 工作流。当前代码在 Gitee 的 `openkylin/huanghe` 分支维护，
且不再有 `+dfsg` 上游分支，因此删除而不复用错误配置。

### 4.3 `debian/salsa-ci.yml`

该文件只包含 Debian Salsa CI 模板。openKylin 仓库不运行 Salsa CI，所以删除；
在没有确认 openKylin CI 接口前，也没有擅自添加另一套流水线配置。

### 4.4 `debian/patches/replace-test-font-files.patch`

该补丁把测试从 Arial/Nanum Gothic 改为 Public Sans。对应修改已经进入 1.1.0
上游源码；继续应用会重复修改或导致补丁失败，因此删除。

### 4.5 `debian/patches/series`

旧文件只登记上述字体补丁。补丁删除后不再需要空的 series 文件，整个
`debian/patches/` 目录随之移除。

### 4.6 `debian/test/PublicSans-Regular.ttf`

旧包因 DFSG 重打包移除了上游字体，再由 `debian/test` 补入自由字体。1.1.0 上游
已经直接包含同一 Public Sans 测试字体，保留第二份会造成无意义的二进制重复。

### 4.7 `debian/source/include-binaries`

该文件原本允许 `debian/test/PublicSans-Regular.ttf` 出现在 Debian tarball 中。
删除 `debian/test` 下的二进制字体后，不再有需要额外声明的二进制文件。

## 5. 内容保持不变的文件

以下文件与 Debian 1.0.6+dfsg-1 参考目录逐字节相同：

- `debian/libthorvg1.install`：仍安装 `libthorvg-1.so.1*` 运行库。
- `debian/libthorvg-dev.install`：仍安装头文件、无版本 `.so` 链接和 pkg-config 文件。
- `debian/thorvg-tools.install`：仍安装 `usr/bin` 下的两个工具。
- `debian/thorvg-tools.manpages`：仍登记两个 section 1 manpage。
- `debian/manpages/tvg-svg2png.1`：1.1.0 的 `tvg-svg2png` CLI 与说明一致。
- `debian/manpages/tvg-lottie2gif.1`：1.1.0 的 `tvg-lottie2gif` CLI 与说明一致。
- `debian/source/format`：继续使用 `3.0 (quilt)`。
- `debian/upstream/metadata`：继续指向 ThorVG 上游 GitHub 仓库、issue 和 tags。

其中 `upstream/metadata` 与 `control` 中的 `Vcs-*` 角色不同：前者描述 ThorVG
上游项目，后者描述 openKylin 维护这份发行版打包代码的位置。

## 6. 构建验证结果

本次目录已在 openKylin amd64 环境完成以下验证：

- `dpkg-checkbuilddeps` 通过；
- `dpkg-buildpackage --no-sign -b -j2` 成功；
- Meson 单元测试 `1/1` 通过；
- 成功生成 `libthorvg1`、`libthorvg-dev` 和 `thorvg-tools`，并生成对应 dbgsym 包；
- `dpkg-buildpackage --no-sign -S` 成功生成 `3.0 (quilt)` 源码包；
- 二进制和源码 Lintian 均无 error 或 warning。

剩余提示均为非阻塞的信息或 pedantic 项：两个 manpage 保留了原 Debian 手册作者
声明；工具中没有可由 Fortify 替换的 libc 调用。

## 7. 后续维护注意事项

- 更新 ThorVG 版本时必须重新核对 `libthorvg1.symbols`，并特别关注 SONAME 未变但
  导出符号消失的情况。
- 若上游测试字体再次变化，需要同步检查 `copyright`，但不要默认恢复 `+dfsg`；只有
  确实需要裁剪上游 tarball 时才使用重打包版本。
- `Vcs-*` 应继续指向 openKylin 打包仓库；`watch` 和 `upstream/metadata` 应继续指向
  真正的 ThorVG 上游。
- 发布新的 openKylin 修订时，在现有 changelog 顶部增加 `-ok2`、`-ok3` 等新条目，
  不要改写已经发布的条目。
