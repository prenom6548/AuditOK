# SkillHub 前端 UI 集成方案

> 日期：2026-05-11
> 状态：已确认，待实施
> 范围：搜索（Search）、展示（Showcase）、安装（Install）三大功能的前端 UI 集成

---

## 1. 概述

为 `zeroclaw-skillhub` crate 的搜索、展示、安装功能开发前端 UI 集成方案。采用 **Gateway 代理模式**：在 `kylinbot-gateway` 中新增 REST API 端点，透传 `zeroclaw-skillhub` crate 的现有函数返回值；前端新增独立 `/skillhub` 页面，负责全部 UI 渲染和交互逻辑。

**核心约束**：不修改 `zeroclaw-skillhub` crate 内部实现逻辑。

---

## 2. 架构

```
┌─────────────────────────────────────────────────────────┐
│  web_kbot (React SPA)                                   │
│  /skillhub 页面                                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│  │ 搜索视图  │  │ 展示视图  │  │ 安装流程  │              │
│  └─────┬────┘  └─────┬────┘  └─────┬────┘              │
│        │             │             │                     │
│  ┌─────▼─────────────▼─────────────▼────┐               │
│  │  skillhubApi (lib/skillhubApi.ts)    │               │
│  └─────┬─────────────┬─────────────┬────┘               │
└────────┼─────────────┼─────────────┼────────────────────┘
         │             │             │
    GET /api/skillhub/search    GET /api/skillhub/showcase    POST /api/skillhub/install
         │             │             │
┌────────▼─────────────▼─────────────▼────────────────────┐
│  kylinbot-gateway (Axum)                                │
│  api_skillhub.rs (新增)                                  │
│  ┌─────────────────────────────────────────────┐        │
│  │  调用 zeroclaw_skillhub crate 现有函数        │        │
│  │  search::run() / showcase::run() /           │        │
│  │  install::run()                              │        │
│  └─────────────────────────────────────────────┘        │
└─────────────────────────────────────────────────────────┘
```

---

## 3. Gateway API 端点

新增文件：`crates/kylinbot-gateway/src/api_skillhub.rs`

### 3.1 端点清单

| 方法 | 路径 | 说明 | 对应 crate 函数 |
|------|------|------|-----------------|
| `GET` | `/api/skillhub/search?q={query}` | 搜索技能 | `zeroclaw_skillhub::commands::search::run()` |
| `GET` | `/api/skillhub/showcase?cat={category}` | 展示列表 | `zeroclaw_skillhub::commands::showcase::run()` |
| `POST` | `/api/skillhub/install` | 安装技能 | `zeroclaw_skillhub::commands::install::run()` |
| `GET` | `/api/skillhub/list` | 列出已安装 | `zeroclaw_skillhub::commands::list::run()` |
| `POST` | `/api/skillhub/upgrade` | 升级技能 | `zeroclaw_skillhub::commands::upgrade::run()` |

### 3.2 请求/响应格式

#### 搜索

```
GET /api/skillhub/search?q=web%20scraper
```

```json
{
  "results": [
    {
      "name": "Web Scraper",
      "slug": "web-scraper",
      "description": "...",
      "version": "1.2.0",
      "author": "clawhub",
      "tags": ["web", "scraping"],
      "downloads": 1500,
      "stars": 42,
      "profile_url": "https://clawhub.ai/skills/web-scraper",
      "zip_url": "https://clawhub.ai/api/v1/skills/web-scraper/download",
      "sha256": "abc123..."
    }
  ]
}
```

#### 展示

```
GET /api/skillhub/showcase?cat=hot
```

```json
{
  "section": "hot",
  "skills": [
    {
      "slug": "my-skill",
      "name": "My Skill",
      "description": "...",
      "description_zh": "...",
      "downloads": 3200,
      "stars": 88,
      "category": "automation",
      "created_at": "2025-03-15T10:00:00Z",
      "homepage": "https://...",
      "version": "2.1.0",
      "icon_url": "https://..."
    }
  ]
}
```

#### 安装

```
POST /api/skillhub/install
Content-Type: application/json

{
  "source": "clawhub:my-skill",
  "force": false,
  "timeout": 30
}
```

成功响应 (200)：

```json
{
  "success": true,
  "slug": "my-skill",
  "name": "My Skill",
  "version": "1.0.0",
  "message": "Skill installed successfully"
}
```

已存在响应 (409)：

```json
{
  "success": false,
  "slug": "my-skill",
  "message": "Already installed. Use force=true to overwrite."
}
```

失败响应 (500)：

```json
{
  "success": false,
  "slug": "my-skill",
  "message": "SHA256 verification failed"
}
```

#### 列出已安装

```
GET /api/skillhub/list
```

```json
{
  "skills": [
    {
      "name": "My Skill",
      "slug": "my-skill",
      "version": "1.0.0",
      "source": "clawhub:my-skill",
      "installed_at": "2025-05-11T08:30:00Z"
    }
  ]
}
```

#### 升级

```
POST /api/skillhub/upgrade
Content-Type: application/json

{
  "slug": "my-skill",
  "check_only": false,
  "timeout": 30
}
```

```json
{
  "upgraded": [
    { "slug": "my-skill", "old_version": "1.0.0", "new_version": "2.0.0" }
  ],
  "checked": [
    { "slug": "other-skill", "current": "1.5.0", "latest": "1.5.0", "up_to_date": true }
  ]
}
```

### 3.3 Gateway 实现要点

1. **认证**：复用现有 `require_auth()` — 所有 `/api/skillhub/*` 端点需 Bearer Token
2. **超时**：安装端点使用长任务超时（复用 `gateway_long_running_request_timeout_secs()`，默认 10 分钟），搜索/展示使用默认 30s 超时
3. **错误处理**：捕获 `zeroclaw_skillhub` 的 `anyhow::Error`，转换为结构化 JSON 错误响应
4. **路由注册**：在 `lib.rs` 路由构建块中添加 `api_skillhub` 模块的 5 条路由
5. **依赖**：`Cargo.toml` 添加 `zeroclaw-skillhub` 依赖

---

## 4. 前端页面设计

### 4.1 路由与导航

- 新增路由 `/skillhub`，注册于 `App.tsx`
- 在 `Header.tsx` 导航中添加 "SkillHub" 入口（图标：🛒 或商店图标）

### 4.2 页面结构

```
/skillhub
├── SkillHubPage (主页面容器)
│   ├── SkillHubHeader (标题 + 描述)
│   ├── SearchBar (搜索输入框)
│   ├── TabView (展示 / 已安装 切换)
│   │   ├── ShowcaseTab
│   │   │   ├── CategoryPills (hot / featured / newest)
│   │   │   └── SkillCardGrid
│   │   │       └── SkillCard (技能卡片)
│   │   └── InstalledTab
│   │       └── InstalledSkillList
│   │           └── InstalledSkillRow
│   └── InstallModal (安装确认对话框)
```

### 4.3 组件详细设计

#### SearchBar

| 属性 | 说明 |
|------|------|
| 防抖 | 300ms debounce |
| 占位符 | "搜索技能..." |
| 清除按钮 | 右侧 × 图标 |
| 搜索状态 | 输入时显示加载 spinner |

交互流程：
- 输入 → 300ms 防抖 → `GET /api/skillhub/search?q=` → 渲染搜索结果
- 空输入 → 恢复展示视图
- 无结果 → 空状态插图 + "未找到匹配的技能"

#### SkillCard（技能卡片）

```
┌──────────────────────────────────┐
│  [icon]  My Skill         ★ 42  │
│          by clawhub              │
│  技能描述文本，最多两行...        │
│                                  │
│  📦 1.5K  🏷 web, scraping  v1.2│
│                                  │
│         [ 安装 ]                 │
└──────────────────────────────────┘
```

| 字段 | 来源 | 说明 |
|------|------|------|
| 图标 | `icon_url` / 默认首字母 | ShowcaseSkill 有 icon_url |
| 名称 | `name` | 主标题 |
| 作者 | `author` | 副标题 |
| 描述 | `description` / `description_zh` | 最多 2 行，溢出省略 |
| 星标 | `stars` | 右上角 |
| 下载量 | `downloads` | 格式化（1.5K, 2.3M） |
| 标签 | `tags` | 最多 3 个 |
| 版本 | `version` | 右下角 |
| 安装按钮 | — | 已安装 → "已安装 ✓"（绿色），未安装 → "安装" |

#### ShowcaseTab

- 顶部：CategoryPills（`hot` 🔥 / `featured` ⭐ / `newest` 🆕）
- 内容：SkillCardGrid 响应式网格
- 默认加载 `hot` 分类

#### InstalledTab

- 列表视图，每行：名称、版本、安装时间、操作按钮（升级/卸载）
- 空状态："尚未安装任何技能，去市场逛逛 →"

#### InstallModal

```
┌─────────────────────────────┐
│  安装技能                     │
│                              │
│  确定要安装 "My Skill" v1.2?  │
│                              │
│  ☐ 强制覆盖已安装版本         │
│                              │
│  [ 取消 ]    [ 确认安装 ]     │
└─────────────────────────────┘
```

状态机：
- 待确认 → 安装中（按钮 spinner + "安装中..."）→ 成功（绿色 ✓ + 2s 自动关闭）/ 失败（红色 ✗ + 错误信息，保持打开）

---

## 5. 前端 API 客户端与 Hooks

### 5.1 API 客户端

新增文件：`web_kbot/src/lib/skillhubApi.ts`

```typescript
const BASE = '/api/skillhub';

export async function searchSkills(query: string): Promise<SkillSearchResult[]> { ... }
export async function getShowcase(category: string): Promise<ShowcaseResponse> { ... }
export async function installSkill(source: string, force?: boolean, timeout?: number): Promise<InstallResult> { ... }
export async function listInstalled(): Promise<InstalledSkill[]> { ... }
export async function upgradeSkill(slug?: string, checkOnly?: boolean, timeout?: number): Promise<UpgradeResult> { ... }
```

风格与现有 `api.ts` 保持一致（Bearer Token 认证、错误处理、JSON 序列化）。

### 5.2 React Hooks

新增文件：`web_kbot/src/hooks/useSkillHub.ts`

```typescript
export function useSkillHubSearch() { ... }    // 搜索状态 + 300ms 防抖
export function useSkillHubShowcase() { ... }   // 展示分类切换 + 数据加载
export function useSkillHubInstall() { ... }    // 安装流程 + 状态机 (idle → installing → success/error)
export function useSkillHubInstalled() { ... }  // 已安装列表 + 刷新
```

### 5.3 TypeScript 类型

新增文件：`web_kbot/src/pages/skillhub/types.ts`

与 `zeroclaw-skillhub` crate 的 `models.rs` 对应：

```typescript
export interface SkillSearchResult {
  name: string;
  slug: string;
  description: string;
  version: string;
  author: string;
  tags: string[];
  downloads: number;
  stars: number;
  created_at: string;
  updated_at: string;
  profile_url: string;
  zip_url: string;
  sha256: string;
}

export interface ShowcaseSkill {
  slug: string;
  name: string;
  description: string;
  description_zh: string;
  downloads: number;
  stars: number;
  category: string;
  created_at: string;
  homepage: string;
  version: string;
  icon_url: string;
}

export interface ShowcaseResponse {
  section: string;
  skills: ShowcaseSkill[];
}

export interface InstalledSkill {
  name: string;
  slug: string;
  version: string;
  source: string;
  zip_url: string;
  update_url: string;
  sha256: string;
  installed_at: string;
}

export interface InstallResult {
  success: boolean;
  slug: string;
  name?: string;
  version?: string;
  message: string;
}

export interface UpgradeResult {
  upgraded: Array<{ slug: string; old_version: string; new_version: string }>;
  checked: Array<{ slug: string; current: string; latest: string; up_to_date: boolean }>;
}
```

---

## 6. 交互反馈

### 6.1 加载状态

| 场景 | 表现 |
|------|------|
| 搜索中 | SearchBar 内 spinner + 结果区域骨架屏 |
| 展示加载 | 卡片骨架屏（3×4 网格） |
| 安装中 | InstallModal 按钮变为 spinner + "安装中..." |
| 升级中 | 行内 spinner 替换升级按钮 |

### 6.2 成功/失败提示

| 场景 | 表现 |
|------|------|
| 安装成功 | 卡片按钮变为 "已安装 ✓"（绿色）+ 顶部 toast "✓ My Skill 安装成功" |
| 安装失败 | InstallModal 内显示红色错误信息 + toast "✗ 安装失败: {reason}" |
| 升级成功 | 行内版本号更新 + toast "✓ My Skill 已升级到 v2.0.0" |
| 升级失败 | 行内错误提示 + toast |
| 网络错误 | 全局错误边界捕获 + 重试按钮 |

### 6.3 Toast 通知

- 成功：绿色背景，3 秒自动消失
- 失败：红色背景，需手动关闭
- 位置：页面右上角

---

## 7. 响应式设计

### 7.1 断点

| 断点 | 宽度 | 卡片网格 | 布局调整 |
|------|------|----------|----------|
| `sm` | ≥640px | 2 列 | SearchBar 全宽 |
| `md` | ≥768px | 3 列 | 侧边 padding 增加 |
| `lg` | ≥1024px | 4 列 | 完整布局 |
| `xl` | ≥1280px | 4 列 | 最大宽度约束 |

### 7.2 移动端适配

- SearchBar：全宽，圆角，搜索图标在左
- SkillCard：单列时拉伸为全宽列表项
- CategoryPills：水平滚动，不换行
- InstallModal：全屏底部抽屉（mobile sheet）
- InstalledTab：紧凑列表，操作按钮折叠到 ⋯ 菜单

### 7.3 浏览器兼容性

与 Vite 6 默认目标一致：`last 2 versions and not dead, > 0.3%, Firefox ESR`

---

## 8. 文件改动清单

### 8.1 Gateway（Rust）

| 文件 | 操作 | 说明 |
|------|------|------|
| `crates/kylinbot-gateway/src/api_skillhub.rs` | **新增** | 5 个 handler 函数 |
| `crates/kylinbot-gateway/src/lib.rs` | 修改 | 添加 `mod api_skillhub` + 5 条路由注册 |
| `crates/kylinbot-gateway/Cargo.toml` | 修改 | 添加 `zeroclaw-skillhub` 依赖 |

### 8.2 前端（TypeScript/React）

| 文件 | 操作 | 说明 |
|------|------|------|
| `web_kbot/src/pages/skillhub/SkillHubPage.tsx` | **新增** | 主页面容器 |
| `web_kbot/src/pages/skillhub/SearchBar.tsx` | **新增** | 搜索输入框 |
| `web_kbot/src/pages/skillhub/SkillCard.tsx` | **新增** | 技能卡片 |
| `web_kbot/src/pages/skillhub/SkillCardGrid.tsx` | **新增** | 响应式网格 |
| `web_kbot/src/pages/skillhub/ShowcaseTab.tsx` | **新增** | 展示视图 |
| `web_kbot/src/pages/skillhub/InstalledTab.tsx` | **新增** | 已安装视图 |
| `web_kbot/src/pages/skillhub/InstallModal.tsx` | **新增** | 安装确认对话框 |
| `web_kbot/src/pages/skillhub/CategoryPills.tsx` | **新增** | 分类切换 |
| `web_kbot/src/pages/skillhub/types.ts` | **新增** | TypeScript 类型定义 |
| `web_kbot/src/lib/skillhubApi.ts` | **新增** | API 客户端 |
| `web_kbot/src/hooks/useSkillHub.ts` | **新增** | React hooks |
| `web_kbot/src/App.tsx` | 修改 | 添加 `/skillhub` 路由 |
| `web_kbot/src/components/layout/Header.tsx` | 修改 | 添加导航入口 |

### 8.3 不改动

| 模块 | 原因 |
|------|------|
| `crates/zeroclaw-skillhub/` | 按约束不修改内部实现 |

---

## 9. 数据流

### 搜索

```
用户输入 → 300ms 防抖 → GET /api/skillhub/search?q=xxx
    → gateway handler 调用 ApiClient::search()
    → 返回 Vec<SkillSearchResult>
    → gateway 序列化为 JSON
    → 前端渲染 SkillCardGrid
```

### 展示

```
页面加载 / 分类切换 → GET /api/skillhub/showcase?cat=hot
    → gateway handler 调用 showcase::run()
    → 返回 ShowcaseResponse
    → 前端渲染 SkillCardGrid
```

### 安装

```
用户点击"安装" → InstallModal 弹出 → 确认
    → POST /api/skillhub/install { source, force, timeout }
    → gateway handler 调用 install::run()
    → 返回 { success, slug, name, version, message }
    → 前端更新卡片状态 + toast
```

---

## 10. 安全考量

1. **认证**：所有 `/api/skillhub/*` 端点通过 `require_auth()` 守护，需 Bearer Token
2. **输入校验**：Gateway handler 校验 `q` 参数非空、`source` 格式合法、`timeout` 在合理范围
3. **速率限制**：复用现有 `auth_rate_limit` 机制
4. **CORS**：由 Gateway 统一处理，前端无需直连外部 API
5. **SHA256 校验**：由 `zeroclaw-skillhub` crate 内部处理，Gateway 不介入
