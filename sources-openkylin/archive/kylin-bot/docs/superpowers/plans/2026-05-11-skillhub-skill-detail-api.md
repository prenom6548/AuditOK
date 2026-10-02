# SkillHub Skill 详细信息 API 调研

## 概述

SkillHub（`https://www.skillhub.cn`）的 skill 详细信息通过其 API 服务 `https://api.skillhub.cn/api/v1` 获取。以下为实际探测到的 API 端点及响应结构。

---

## 1. Skill 详情接口

### 请求

```
GET https://api.skillhub.cn/api/v1/skills/{slug}
```

- **方法**: GET
- **认证**: 无需认证（公开接口）
- **参数**: `slug` 为 skill 的唯一标识符（路径参数）

### 示例

```bash
curl -s "https://api.skillhub.cn/api/v1/skills/weather" | python3 -m json.tool
```

### 响应结构 (HTTP 200)

```json
{
  "latestVersion": {
    "changelog": "Synced by skillhub pipeline",
    "createdAt": 1774662201379,
    "version": "1.0.0"
  },
  "owner": {
    "displayName": "steipete",
    "handle": "steipete",
    "image": null
  },
  "securityReports": {
    "keen": {
      "reportUrl": "https://tix.qq.com/search/skill?keyword=f3ab253b941f894f36368ba36cee0fa4",
      "status": "benign",
      "statusText": "安全，无风险"
    },
    "sanbu": {
      "reportUrl": "https://static.cloudsec.tencent.com/html-reports/2026/04/23/...",
      "status": "benign",
      "statusText": "安全，无风险"
    }
  },
  "skill": {
    "category": "productivity",
    "createdAt": 1772739778242,
    "displayName": "Weather",
    "iconUrl": null,
    "labels": {
      "requires_api_key": "false"
    },
    "slug": "weather",
    "source": "clawhub",
    "stats": {
      "comments": 0,
      "downloads": 198526,
      "installs": 21530,
      "stars": 392,
      "versions": 1
    },
    "summary": "Get current weather and forecasts (no API key required).",
    "summary_zh": "获取当前天气和预报（无需API密钥）",
    "tags": {
      "latest": "1.0.0"
    },
    "updatedAt": 1778489117393
  }
}
```

### 错误响应 (HTTP 404)

```json
{"error": "Skill not found"}
```

### 响应字段说明

| 字段 | 类型 | 说明 |
|------|------|------|
| `latestVersion.changelog` | string | 版本变更日志 |
| `latestVersion.createdAt` | number | 版本创建时间（毫秒时间戳） |
| `latestVersion.version` | string | 最新版本号 |
| `owner.displayName` | string | 作者显示名 |
| `owner.handle` | string | 作者用户名 |
| `owner.image` | string\|null | 作者头像 URL |
| `securityReports.{lab}.status` | string | 安全检测状态（`benign`=安全） |
| `securityReports.{lab}.statusText` | string | 安全检测状态中文描述 |
| `securityReports.{lab}.reportUrl` | string | 安全检测报告链接 |
| `skill.category` | string | 分类（如 `productivity`、`ai-intelligence`） |
| `skill.createdAt` | number | 创建时间（毫秒时间戳） |
| `skill.displayName` | string | 技能显示名称 |
| `skill.iconUrl` | string\|null | 图标 URL |
| `skill.labels` | object | 标签键值对（如 `requires_api_key`） |
| `skill.slug` | string | 唯一标识符 |
| `skill.source` | string | 来源（如 `clawhub`） |
| `skill.stats.downloads` | number | 下载量 |
| `skill.stats.installs` | number | 安装量 |
| `skill.stats.stars` | number | 收藏数 |
| `skill.stats.comments` | number | 评论数 |
| `skill.stats.versions` | number | 版本数量 |
| `skill.summary` | string | 英文摘要 |
| `skill.summary_zh` | string | 中文摘要 |
| `skill.tags` | object | 标签键值对（如 `latest`=>版本号） |
| `skill.updatedAt` | number | 更新时间（毫秒时间戳） |

---

## 2. Skill 版本列表接口

### 请求

```
GET https://api.skillhub.cn/api/v1/skills/{slug}/versions
```

- **方法**: GET
- **认证**: 无需认证（公开接口）

### 示例

```bash
curl -s "https://api.skillhub.cn/api/v1/skills/weather/versions" | python3 -m json.tool
```

### 响应结构 (HTTP 200)

```json
{
  "slug": "weather",
  "source": "clawhub",
  "versions": [
    {
      "changelog": "Synced by skillhub pipeline",
      "createdAt": 1774662201379,
      "securityReports": {
        "keen": {
          "status": "benign",
          "statusText": "安全，无风险",
          "reportUrl": "https://tix.qq.com/search/skill?keyword=..."
        },
        "sanbu": {
          "status": "benign",
          "statusText": "安全，无风险",
          "reportUrl": "https://static.cloudsec.tencent.com/html-reports/..."
        }
      },
      "version": "1.0.0",
      "versionId": 11
    }
  ]
}
```

### 响应字段说明

| 字段 | 类型 | 说明 |
|------|------|------|
| `slug` | string | 技能唯一标识符 |
| `source` | string | 来源 |
| `versions[]` | array | 版本列表 |
| `versions[].version` | string | 版本号 |
| `versions[].versionId` | number | 版本 ID |
| `versions[].changelog` | string | 变更日志 |
| `versions[].createdAt` | number | 创建时间（毫秒时间戳） |
| `versions[].securityReports` | object | 各版本独立的安全检测报告 |

---

## 3. 已有项目中已集成的 API

当前 `zeroclaw-skillhub` crate（[client.rs](../../crates/zeroclaw-skillhub/src/client.rs)）已实现以下 API 调用：

| 方法 | API 端点 | 用途 |
|------|----------|------|
| `search(query)` | `GET /search?q={query}` | 搜索技能 |
| `showcase(scene)` | `GET /showcase/{scene}` | 获取展示列表 |
| `index()` | `GET /skills` | 获取技能索引 |
| `download(slug)` | `GET /download?slug={slug}` | 下载技能 ZIP 包 |
| `fetch_json(url, timeout)` | `GET {url}` | 通用 JSON 获取 |

**缺失**: 尚未实现 **skill 详情** (`GET /skills/{slug}`) 和 **版本列表** (`GET /skills/{slug}/versions`) 接口的调用。

---

## 4. 前端 SkillHub 模块现状

### 前端 API 层 ([skillhubApi.ts](../../web_kbot/src/lib/skillhubApi.ts))

已有 5 个 API 函数：

| 函数 | 端点 | 方法 |
|------|------|------|
| `searchSkills(query)` | `/api/skillhub/search` | GET |
| `getShowcase(category)` | `/api/skillhub/showcase` | GET |
| `installSkill(source, force, timeout)` | `/api/skillhub/install` | POST |
| `listInstalled()` | `/api/skillhub/list` | GET |
| `upgradeSkill(slug, checkOnly, timeout)` | `/api/skillhub/upgrade` | POST |

**缺失**: 尚未实现 **skill 详情** 获取函数。

### 前端类型定义 ([types.ts](../../web_kbot/src/pages/skillhub/types.ts))

已有类型：`SkillSearchResult`、`ShowcaseSkill`、`InstalledSkill`、`InstallResult`、`UpgradeResult`。

**缺失**: 尚未定义 **SkillDetail** 详情类型（对应 API 返回的 `owner`、`securityReports`、`latestVersion` 等字段）。

### 后端 Gateway 层 ([api_skillhub.rs](../../crates/kylinbot-gateway/src/api_skillhub.rs))

已注册 5 条路由到 `/api/skillhub/*`。

**缺失**: 尚未注册 **skill 详情** 和 **版本列表** 的 HTTP handler 及路由。

---

## 5. 补充 Skill 详情功能的实现建议

### 5.1 后端 (Rust)

1. **`zeroclaw-skillhub/src/client.rs`** — 新增 `detail(slug)` 和 `versions(slug)` 方法：

```rust
pub async fn detail(&self, slug: &str) -> Result<SkillDetail> {
    let url = format!("{}/skills/{}", self.base_url, urlencoding::encode(slug));
    let resp = self.client.get(&url).send().await
        .context("detail request failed")?
        .error_for_status()
        .context("detail API error")?;
    let data: SkillDetail = resp.json().await
        .context("parse detail response failed")?;
    Ok(data)
}

pub async fn versions(&self, slug: &str) -> Result<SkillVersionsResponse> {
    let url = format!("{}/skills/{}/versions", self.base_url, urlencoding::encode(slug));
    let resp = self.client.get(&url).send().await
        .context("versions request failed")?
        .error_for_status()
        .context("versions API error")?;
    let data: SkillVersionsResponse = resp.json().await
        .context("parse versions response failed")?;
    Ok(data)
}
```

2. **`zeroclaw-skillhub/src/models.rs`** — 新增 `SkillDetail`、`SkillVersionsResponse` 等类型定义

3. **`kylinbot-gateway/src/api_skillhub.rs`** — 新增 `handle_skillhub_detail` handler

4. **`kylinbot-gateway/src/lib.rs`** — 注册路由 `GET /api/skillhub/detail?slug={slug}`

### 5.2 前端 (React)

1. **`web_kbot/src/pages/skillhub/types.ts`** — 新增 `SkillDetail` 类型

2. **`web_kbot/src/lib/skillhubApi.ts`** — 新增 `getSkillDetail(slug)` 函数

3. **`web_kbot/src/pages/skillhub/`** — 新增 SkillDetail 组件或弹窗，展示：
   - 基本信息（名称、描述、分类、作者）
   - 统计数据（下载量、安装量、收藏数）
   - 版本信息（最新版本、changelog）
   - 安全检测报告（状态、报告链接）
   - 标签信息

---

## 6. API 基础信息汇总

| 项目 | 值 |
|------|-----|
| API Base URL | `https://api.skillhub.cn/api/v1` |
| Skill 详情 | `GET /skills/{slug}` |
| Skill 版本列表 | `GET /skills/{slug}/versions` |
| 搜索 | `GET /search?q={query}` |
| 展示 | `GET /showcase/{scene}` |
| 技能索引 | `GET /skills` |
| 下载 | `GET /download?slug={slug}` |
| 认证方式 | 无需认证 |
| 数据来源 | ClawHub（OpenClaw 官方技能市场） |
| 安全检测 | 科恩实验室(keen)、云鼎实验室(sanbu) |
