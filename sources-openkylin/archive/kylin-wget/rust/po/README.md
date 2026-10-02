# Kylin Wget 翻译文件

与 GNU wget 相同，采用 **gettext + `.po` 文件** 维护译文；源码里只写英文 **msgid**。

## 目录

| 文件 | 说明 |
|------|------|
| `kylin-wget.pot` | 翻译模板（msgstr 为空，供 `msginit` / `msgmerge`） |
| `zh_CN.po` | 简体中文译文 |

编译时 `build.rs` 会读取 `po/*.po`，生成 `OUT_DIR/i18n_catalogs.rs` 并嵌入二进制，**无需** 系统安装 `libintl`。

## 工作流程

1. 在 Rust 源码中新增或修改英文字符串（作为 `i18n::gettext("...")` 的 msgid）。
2. 更新模板并合并到各语言：

   ```bash
   cd rust/po
   # 若已安装 gettext 工具：
   msgmerge -U zh_CN.po kylin-wget.pot
   ```

3. 编辑 `zh_CN.po` 中对应 `msgstr`。
4. 重新编译：

   ```bash
   cd rust && cargo build --release
   ```

5. 验证：

   ```bash
   LANG=C ./target/release/wget -h | head
   LANG=zh_CN.UTF-8 ./target/release/wget -h | head
   ```

## 语言选择

运行时读取 `LC_ALL`、`LC_MESSAGES`、`LANG`（按此顺序）：

| 环境 | catalog | 行为 |
|------|---------|------|
| 未设置 / `C` / `POSIX` | `C` | 英文（msgid） |
| `en_US.UTF-8` 等 `en*` | `C` | 英文 |
| `zh_CN.UTF-8`、`zh_TW` 等 `zh*` | `zh_CN`（或规范化后的 `zh_TW` 等，有对应 po 时） | 查表译文 |
| `fr_FR.UTF-8` 等其它语言 | `fr_FR` 等 | **无 po 时回退英文** |

新增语言：在 `po/` 下增加 `de_DE.po` 等，文件名与 `LANG` 规范化结果一致即可（`build.rs` 自动编入）。

## 已国际化范围

- `-h` / `--help` 全部选项说明
- 无 URL、参数校验错误
- 下载状态行（长度、保存路径、重试、304 等）
- 后台模式提示、进度条相关警告
- `--header` / `--user-agent` / `--referer` 校验错误

## 刻意保持英文（非翻译项）

- `-V` / `--version` 固定品牌与版权信息
- HTTP 响应行（`HTTP/1.1 200 OK`）、`-S` 头字段名
- `-o` 日志文件机器格式（时间戳 + `URL:...`）
- 进度条符号、`B`/`K`/`M`/`MB/s` 等单位
- 底层库/网络栈返回的原始错误文本（`wget: {e}` 中的 `{e}`）
