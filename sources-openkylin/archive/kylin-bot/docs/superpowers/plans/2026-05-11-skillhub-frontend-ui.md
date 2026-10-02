# SkillHub Frontend UI Integration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add SkillHub marketplace search, showcase, and install functionality to the web dashboard via Gateway REST API proxy and a new `/skillhub` React page.

**Architecture:** Gateway adds `api_skillhub.rs` with 5 REST endpoints that call existing `zeroclaw-skillhub` crate functions. Frontend adds a new `/skillhub` page with search, showcase, and install UI. No changes to `zeroclaw-skillhub` crate internals.

**Tech Stack:** Rust/Axum 0.8 (gateway), React 19 + TypeScript + Tailwind CSS 4 (frontend), `zeroclaw-skillhub` crate (unchanged)

---

## File Structure

### Gateway (Rust)

| File | Operation | Responsibility |
|------|-----------|---------------|
| `crates/kylinbot-gateway/src/api_skillhub.rs` | Create | 5 handler functions for search/showcase/install/list/upgrade |
| `crates/kylinbot-gateway/src/lib.rs` | Modify | Add `mod api_skillhub` + 5 route registrations |
| `crates/kylinbot-gateway/Cargo.toml` | Modify | Add `zeroclaw-skillhub` dependency |

### Frontend (TypeScript/React)

| File | Operation | Responsibility |
|------|-----------|---------------|
| `web_kbot/src/pages/skillhub/types.ts` | Create | TypeScript type definitions |
| `web_kbot/src/lib/skillhubApi.ts` | Create | API client for `/api/skillhub/*` |
| `web_kbot/src/pages/skillhub/SkillCard.tsx` | Create | Skill card component |
| `web_kbot/src/pages/skillhub/SkillCardGrid.tsx` | Create | Responsive grid layout |
| `web_kbot/src/pages/skillhub/SearchBar.tsx` | Create | Search input with debounce |
| `web_kbot/src/pages/skillhub/CategoryPills.tsx` | Create | Showcase category switcher |
| `web_kbot/src/pages/skillhub/ShowcaseTab.tsx` | Create | Showcase view |
| `web_kbot/src/pages/skillhub/InstalledTab.tsx` | Create | Installed skills list |
| `web_kbot/src/pages/skillhub/InstallModal.tsx` | Create | Install confirmation dialog |
| `web_kbot/src/pages/skillhub/SkillHubPage.tsx` | Create | Main page container |
| `web_kbot/src/App.tsx` | Modify | Add `/skillhub` route |
| `web_kbot/src/components/layout/navModes.ts` | Modify | Add `/skillhub` to nav items |

---

### Task 1: Add `zeroclaw-skillhub` dependency to Gateway Cargo.toml

**Files:**
- Modify: `crates/kylinbot-gateway/Cargo.toml`

- [ ] **Step 1: Add dependency**

Add `zeroclaw-skillhub.workspace = true` to the `[dependencies]` section, after the `kylinbot-hardware` line:

```toml
kylinbot-hardware.workspace = true
zeroclaw-skillhub.workspace = true
```

- [ ] **Step 2: Verify it resolves**

Run: `cargo check -p kylinbot-gateway 2>&1 | head -20`
Expected: Compiles without errors (may have unused import warnings, that's fine)

---

### Task 2: Create `api_skillhub.rs` Gateway handlers

**Files:**
- Create: `crates/kylinbot-gateway/src/api_skillhub.rs`

- [ ] **Step 1: Create the handler file**

Create `crates/kylinbot-gateway/src/api_skillhub.rs` with the following content:

```rust
use axum::{
    extract::{Query, State},
    http::{HeaderMap, StatusCode},
    response::{IntoResponse, Json},
};
use serde::{Deserialize, Serialize};

use super::AppState;

fn require_auth(
    state: &AppState,
    headers: &HeaderMap,
) -> Result<(), (StatusCode, Json<serde_json::Value>)> {
    super::api::require_auth(state, headers)
}

#[derive(Deserialize)]
pub struct SearchQuery {
    pub q: String,
}

#[derive(Deserialize)]
pub struct ShowcaseQuery {
    pub cat: String,
}

#[derive(Deserialize)]
pub struct InstallBody {
    pub source: String,
    #[serde(default)]
    pub force: bool,
    #[serde(default = "default_timeout")]
    pub timeout: u64,
}

fn default_timeout() -> u64 {
    30
}

#[derive(Deserialize)]
pub struct UpgradeBody {
    pub slug: Option<String>,
    #[serde(default)]
    pub check_only: bool,
    #[serde(default = "default_timeout")]
    pub timeout: u64,
}

#[derive(Serialize)]
struct SkillhubError {
    success: bool,
    message: String,
}

fn err_response(status: StatusCode, msg: impl Into<String>) -> axum::response::Response {
    (
        status,
        Json(SkillhubError {
            success: false,
            message: msg.into(),
        }),
    )
        .into_response()
}

pub async fn handle_skillhub_search(
    State(state): State<AppState>,
    headers: HeaderMap,
    Query(params): Query<SearchQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let query = params.q.trim().to_string();
    if query.is_empty() {
        return err_response(StatusCode::BAD_REQUEST, "query parameter 'q' is required");
    }

    let api = zeroclaw_skillhub::ApiClient::new();
    match api.search(&query).await {
        Ok(results) => Json(serde_json::json!({ "results": results })).into_response(),
        Err(e) => err_response(StatusCode::INTERNAL_SERVER_ERROR, format!("search failed: {e}")),
    }
}

pub async fn handle_skillhub_showcase(
    State(state): State<AppState>,
    headers: HeaderMap,
    Query(params): Query<ShowcaseQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let category = params.cat.trim().to_string();
    if category.is_empty() {
        return err_response(StatusCode::BAD_REQUEST, "query parameter 'cat' is required");
    }

    let api = zeroclaw_skillhub::ApiClient::new();
    match api.showcase(&category).await {
        Ok(resp) => Json(resp).into_response(),
        Err(e) => {
            err_response(StatusCode::INTERNAL_SERVER_ERROR, format!("showcase failed: {e}"))
        }
    }
}

pub async fn handle_skillhub_install(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<InstallBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let source = body.source.trim().to_string();
    if source.is_empty() {
        return err_response(StatusCode::BAD_REQUEST, "field 'source' is required");
    }

    let workspace_dir = &state.config.lock().workspace_dir;
    let api = zeroclaw_skillhub::ApiClient::new();

    match zeroclaw_skillhub::commands::install::run(&api, workspace_dir, &source, body.force, body.timeout).await {
        Ok(()) => {
            let slug = source
                .strip_prefix("clawhub:")
                .unwrap_or(&source)
                .trim()
                .to_string();
            Json(serde_json::json!({
                "success": true,
                "slug": slug,
                "message": "Skill installed successfully"
            }))
            .into_response()
        }
        Err(e) => {
            let msg = format!("{e}");
            let status = if msg.contains("already exists") || msg.contains("Already installed") {
                StatusCode::CONFLICT
            } else {
                StatusCode::INTERNAL_SERVER_ERROR
            };
            let slug = source
                .strip_prefix("clawhub:")
                .unwrap_or(&source)
                .trim()
                .to_string();
            (
                status,
                Json(SkillhubError {
                    success: false,
                    message: msg,
                }),
            )
                .into_response()
        }
    }
}

pub async fn handle_skillhub_list(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let workspace_dir = &state.config.lock().workspace_dir;
    let skills_dir = zeroclaw_skillhub::config::skills_install_dir(workspace_dir);
    let lock = zeroclaw_skillhub::lockfile::load_lockfile(&skills_dir);

    let mut skills: Vec<_> = lock.skills.into_values().collect();
    skills.sort_by(|a, b| a.slug.cmp(&b.slug));

    Json(serde_json::json!({ "skills": skills })).into_response()
}

pub async fn handle_skillhub_upgrade(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<UpgradeBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let workspace_dir = &state.config.lock().workspace_dir;
    let api = zeroclaw_skillhub::ApiClient::new();

    match zeroclaw_skillhub::commands::upgrade::run(
        &api,
        workspace_dir,
        body.slug.as_deref(),
        body.check_only,
        body.timeout,
    )
    .await
    {
        Ok(()) => Json(serde_json::json!({
            "success": true,
            "message": if body.check_only { "Check complete" } else { "Upgrade complete" }
        }))
        .into_response(),
        Err(e) => err_response(StatusCode::INTERNAL_SERVER_ERROR, format!("upgrade failed: {e}")),
    }
}
```

- [ ] **Step 2: Verify compilation**

Run: `cargo check -p kylinbot-gateway 2>&1 | head -30`
Expected: Compiles without errors

---

### Task 3: Register `api_skillhub` module and routes in Gateway

**Files:**
- Modify: `crates/kylinbot-gateway/src/lib.rs`

- [ ] **Step 1: Add module declaration**

In `lib.rs`, after the `pub mod api_personality;` line (~line 15), add:

```rust
pub mod api_skillhub;
```

- [ ] **Step 2: Add route registrations**

In the route building block, after the line `.route("/api/skills", get(api::handle_api_skills))` (~line 1139), add:

```rust
    // ── SkillHub marketplace API routes ──
    .route("/api/skillhub/search", get(api_skillhub::handle_skillhub_search))
    .route("/api/skillhub/showcase", get(api_skillhub::handle_skillhub_showcase))
    .route("/api/skillhub/install", post(api_skillhub::handle_skillhub_install))
    .route("/api/skillhub/list", get(api_skillhub::handle_skillhub_list))
    .route("/api/skillhub/upgrade", post(api_skillhub::handle_skillhub_upgrade))
```

- [ ] **Step 3: Verify compilation**

Run: `cargo check -p kylinbot-gateway 2>&1 | head -30`
Expected: Compiles without errors

---

### Task 4: Create frontend TypeScript types

**Files:**
- Create: `web_kbot/src/pages/skillhub/types.ts`

- [ ] **Step 1: Create types file**

Create `web_kbot/src/pages/skillhub/types.ts`:

```typescript
export interface SkillSearchResult {
  name: string;
  slug: string;
  description: string;
  version: string;
  author: string;
  tags: string[];
  downloads: number | null;
  stars: number | null;
  created_at: string | null;
  updated_at: string | null;
  profile_url: string | null;
  zip_url: string | null;
  sha256: string | null;
}

export interface ShowcaseSkill {
  slug: string;
  name: string;
  description: string;
  description_zh: string;
  downloads: number;
  stars: number;
  category: string;
  created_at: number;
  homepage: string;
  version: string;
  icon_url: string | null;
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
  installed_at: number;
}

export interface InstallResult {
  success: boolean;
  slug: string;
  message: string;
}

export interface UpgradeResult {
  success: boolean;
  message: string;
}

export type SkillHubSkill = SkillSearchResult | ShowcaseSkill;

export function isShowcaseSkill(skill: SkillHubSkill): skill is ShowcaseSkill {
  return 'category' in skill;
}
```

- [ ] **Step 2: Verify no type errors**

Run: `cd /home/cong/project/gitee/kybot/web_kbot && npx tsc --noEmit --pretty 2>&1 | head -20`
Expected: No errors related to this file

---

### Task 5: Create frontend API client

**Files:**
- Create: `web_kbot/src/lib/skillhubApi.ts`

- [ ] **Step 1: Create API client**

Create `web_kbot/src/lib/skillhubApi.ts`:

```typescript
import { apiFetch } from './api';
import type {
  SkillSearchResult,
  ShowcaseResponse,
  InstalledSkill,
  InstallResult,
  UpgradeResult,
} from '../pages/skillhub/types';

const BASE = '/api/skillhub';

export function searchSkills(query: string): Promise<{ results: SkillSearchResult[] }> {
  const params = new URLSearchParams({ q: query });
  return apiFetch<{ results: SkillSearchResult[] }>(`${BASE}/search?${params}`);
}

export function getShowcase(category: string): Promise<ShowcaseResponse> {
  const params = new URLSearchParams({ cat: category });
  return apiFetch<ShowcaseResponse>(`${BASE}/showcase?${params}`);
}

export function installSkill(
  source: string,
  force: boolean = false,
  timeout: number = 30,
): Promise<InstallResult> {
  return apiFetch<InstallResult>(`${BASE}/install`, {
    method: 'POST',
    body: JSON.stringify({ source, force, timeout }),
  });
}

export function listInstalled(): Promise<{ skills: InstalledSkill[] }> {
  return apiFetch<{ skills: InstalledSkill[] }>(`${BASE}/list`);
}

export function upgradeSkill(
  slug?: string,
  checkOnly: boolean = false,
  timeout: number = 30,
): Promise<UpgradeResult> {
  return apiFetch<UpgradeResult>(`${BASE}/upgrade`, {
    method: 'POST',
    body: JSON.stringify({ slug: slug ?? null, check_only: checkOnly, timeout }),
  });
}
```

- [ ] **Step 2: Verify no type errors**

Run: `cd /home/cong/project/gitee/kybot/web_kbot && npx tsc --noEmit --pretty 2>&1 | head -20`
Expected: No errors related to this file

---

### Task 6: Create SkillCard component

**Files:**
- Create: `web_kbot/src/pages/skillhub/SkillCard.tsx`

- [ ] **Step 1: Create SkillCard**

Create `web_kbot/src/pages/skillhub/SkillCard.tsx`:

```tsx
import { Download, Star, Check, Loader2 } from 'lucide-react';
import type { SkillHubSkill } from './types';
import { isShowcaseSkill } from './types';

function formatDownloads(n: number): string {
  if (n >= 1_000_000) return `${(n / 1_000_000).toFixed(1)}M`;
  if (n >= 1_000) return `${(n / 1_000).toFixed(1)}K`;
  return String(n);
}

interface SkillCardProps {
  skill: SkillHubSkill;
  isInstalled: boolean;
  isInstalling: boolean;
  onInstall: () => void;
}

export default function SkillCard({ skill, isInstalled, isInstalling, onInstall }: SkillCardProps) {
  const showcase = isShowcaseSkill(skill) ? skill : null;
  const displayName = skill.name || skill.slug;
  const description = showcase?.description_zh || skill.description;
  const downloads = showcase?.downloads ?? (skill as { downloads?: number | null }).downloads ?? 0;
  const stars = showcase?.stars ?? (skill as { stars?: number | null }).stars ?? 0;
  const version = skill.version;
  const tags = 'tags' in skill ? skill.tags : [];
  const iconUrl = showcase?.icon_url ?? null;

  return (
    <div className="card card-radius-6 p-5 flex flex-col gap-3 animate-slide-in-up">
      <div className="flex items-start gap-3">
        <div
          className="h-10 w-10 rounded-lg flex items-center justify-center text-sm font-bold shrink-0"
          style={{
            background: 'var(--pc-accent-glow)',
            color: 'var(--pc-accent)',
            border: '1px solid var(--pc-accent-dim)',
          }}
        >
          {iconUrl ? (
            <img src={iconUrl} alt="" className="h-8 w-8 rounded object-cover" onError={(e) => { e.currentTarget.style.display = 'none'; }} />
          ) : (
            displayName.charAt(0).toUpperCase()
          )}
        </div>
        <div className="min-w-0 flex-1">
          <h4 className="text-sm font-semibold truncate" style={{ color: 'var(--pc-text-primary)' }}>
            {displayName}
          </h4>
          {skill.author && (
            <p className="text-xs" style={{ color: 'var(--pc-text-faint)' }}>
              by {skill.author}
            </p>
          )}
        </div>
        {stars > 0 && (
          <span className="flex items-center gap-1 text-xs shrink-0" style={{ color: 'var(--pc-text-muted)' }}>
            <Star className="h-3 w-3" />
            {stars}
          </span>
        )}
      </div>

      {description && (
        <p className="text-xs line-clamp-2" style={{ color: 'var(--pc-text-muted)' }}>
          {description}
        </p>
      )}

      <div className="flex items-center gap-3 text-xs" style={{ color: 'var(--pc-text-faint)' }}>
        {downloads > 0 && (
          <span className="flex items-center gap-1">
            <Download className="h-3 w-3" />
            {formatDownloads(downloads)}
          </span>
        )}
        {tags.length > 0 && (
          <span className="flex items-center gap-1 overflow-hidden">
            {tags.slice(0, 3).map((tag) => (
              <span
                key={tag}
                className="px-1.5 py-0.5 rounded text-[10px]"
                style={{ background: 'var(--pc-hover)', color: 'var(--pc-text-muted)' }}
              >
                {tag}
              </span>
            ))}
          </span>
        )}
        {version && (
          <span className="ml-auto shrink-0">v{version}</span>
        )}
      </div>

      <button
        onClick={onInstall}
        disabled={isInstalling || isInstalled}
        className="w-full py-2 rounded-lg text-xs font-semibold transition-all"
        style={
          isInstalled
            ? { background: 'rgba(0, 230, 138, 0.1)', color: 'var(--color-status-success)', border: '1px solid rgba(0, 230, 138, 0.2)', cursor: 'default' }
            : isInstalling
            ? { background: 'var(--pc-accent)', color: 'white', opacity: 0.7, cursor: 'wait' }
            : { background: 'var(--pc-accent)', color: 'white', border: 'none', cursor: 'pointer' }
        }
      >
        {isInstalling ? (
          <span className="flex items-center justify-center gap-1.5">
            <Loader2 className="h-3.5 w-3.5 animate-spin" />
            Installing...
          </span>
        ) : isInstalled ? (
          <span className="flex items-center justify-center gap-1.5">
            <Check className="h-3.5 w-3.5" />
            Installed
          </span>
        ) : (
          'Install'
        )}
      </button>
    </div>
  );
}
```

---

### Task 7: Create SkillCardGrid component

**Files:**
- Create: `web_kbot/src/pages/skillhub/SkillCardGrid.tsx`

- [ ] **Step 1: Create SkillCardGrid**

Create `web_kbot/src/pages/skillhub/SkillCardGrid.tsx`:

```tsx
import SkillCard from './SkillCard';
import type { SkillHubSkill, InstalledSkill } from './types';

interface SkillCardGridProps {
  skills: SkillHubSkill[];
  installedSlugs: Set<string>;
  installingSlug: string | null;
  onInstall: (slug: string) => void;
}

export default function SkillCardGrid({ skills, installedSlugs, installingSlug, onInstall }: SkillCardGridProps) {
  if (skills.length === 0) {
    return (
      <div className="card card-radius-6 p-8 text-center">
        <p style={{ color: 'var(--pc-text-muted)' }}>No skills found</p>
      </div>
    );
  }

  return (
    <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4 stagger-children">
      {skills.map((skill) => (
        <SkillCard
          key={skill.slug}
          skill={skill}
          isInstalled={installedSlugs.has(skill.slug)}
          isInstalling={installingSlug === skill.slug}
          onInstall={() => onInstall(skill.slug)}
        />
      ))}
    </div>
  );
}
```

---

### Task 8: Create SearchBar component

**Files:**
- Create: `web_kbot/src/pages/skillhub/SearchBar.tsx`

- [ ] **Step 1: Create SearchBar**

Create `web_kbot/src/pages/skillhub/SearchBar.tsx`:

```tsx
import { useState, useEffect, useRef } from 'react';
import { Search, X, Loader2 } from 'lucide-react';

interface SearchBarProps {
  onSearch: (query: string) => void;
  isLoading: boolean;
}

export default function SearchBar({ onSearch, isLoading }: SearchBarProps) {
  const [query, setQuery] = useState('');
  const debounceRef = useRef<ReturnType<typeof setTimeout>>();

  useEffect(() => {
    if (debounceRef.current) {
      clearTimeout(debounceRef.current);
    }

    debounceRef.current = setTimeout(() => {
      onSearch(query.trim());
    }, 300);

    return () => {
      if (debounceRef.current) {
        clearTimeout(debounceRef.current);
      }
    };
  }, [query, onSearch]);

  return (
    <div className="relative">
      <Search
        className="absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4"
        style={{ color: 'var(--pc-text-muted)' }}
      />
      <input
        type="text"
        value={query}
        onChange={(e) => setQuery(e.target.value)}
        placeholder="Search skills..."
        className="input-electric w-full pl-10 pr-10 py-2.5 text-sm rounded-xl"
      />
      {isLoading && (
        <Loader2
          className="absolute right-10 top-1/2 -translate-y-1/2 h-4 w-4 animate-spin"
          style={{ color: 'var(--pc-text-muted)' }}
        />
      )}
      {query && (
        <button
          onClick={() => setQuery('')}
          className="absolute right-3 top-1/2 -translate-y-1/2 p-0.5 rounded transition-colors"
          style={{ color: 'var(--pc-text-muted)' }}
          onMouseEnter={(e) => { e.currentTarget.style.color = 'var(--pc-text-primary)'; }}
          onMouseLeave={(e) => { e.currentTarget.style.color = 'var(--pc-text-muted)'; }}
        >
          <X className="h-4 w-4" />
        </button>
      )}
    </div>
  );
}
```

---

### Task 9: Create CategoryPills component

**Files:**
- Create: `web_kbot/src/pages/skillhub/CategoryPills.tsx`

- [ ] **Step 1: Create CategoryPills**

Create `web_kbot/src/pages/skillhub/CategoryPills.tsx`:

```tsx
interface CategoryPillsProps {
  active: string;
  onChange: (category: string) => void;
}

const categories = [
  { key: 'hot', label: '🔥 Hot' },
  { key: 'featured', label: '⭐ Featured' },
  { key: 'newest', label: '🆕 Newest' },
];

export default function CategoryPills({ active, onChange }: CategoryPillsProps) {
  return (
    <div className="flex gap-2 overflow-x-auto pb-1">
      {categories.map((cat) => (
        <button
          key={cat.key}
          onClick={() => onChange(cat.key)}
          className="px-3.5 py-1.5 rounded-[6px] text-xs font-semibold transition-all whitespace-nowrap"
          style={
            active === cat.key
              ? { background: 'var(--pc-accent)', color: 'white' }
              : { color: 'var(--pc-text-muted)', border: '1px solid var(--pc-border)', background: 'transparent' }
          }
        >
          {cat.label}
        </button>
      ))}
    </div>
  );
}
```

---

### Task 10: Create ShowcaseTab component

**Files:**
- Create: `web_kbot/src/pages/skillhub/ShowcaseTab.tsx`

- [ ] **Step 1: Create ShowcaseTab**

Create `web_kbot/src/pages/skillhub/ShowcaseTab.tsx`:

```tsx
import { useState, useEffect, useCallback } from 'react';
import { Loader2 } from 'lucide-react';
import { getShowcase } from '@/lib/skillhubApi';
import CategoryPills from './CategoryPills';
import SkillCardGrid from './SkillCardGrid';
import type { ShowcaseSkill, InstalledSkill } from './types';

interface ShowcaseTabProps {
  installedSlugs: Set<string>;
  installingSlug: string | null;
  onInstall: (slug: string) => void;
}

export default function ShowcaseTab({ installedSlugs, installingSlug, onInstall }: ShowcaseTabProps) {
  const [category, setCategory] = useState('hot');
  const [skills, setSkills] = useState<ShowcaseSkill[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchShowcase = useCallback(async (cat: string) => {
    setLoading(true);
    setError(null);
    try {
      const resp = await getShowcase(cat);
      setSkills(resp.skills);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load showcase');
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchShowcase(category);
  }, [category, fetchShowcase]);

  return (
    <div className="space-y-4">
      <CategoryPills active={category} onChange={setCategory} />
      {loading ? (
        <div className="flex items-center justify-center py-12">
          <Loader2 className="h-8 w-8 animate-spin" style={{ color: 'var(--pc-accent)' }} />
        </div>
      ) : error ? (
        <div className="card card-radius-6 p-8 text-center">
          <p style={{ color: 'var(--color-status-error)' }}>{error}</p>
          <button
            onClick={() => fetchShowcase(category)}
            className="btn-electric mt-4 px-4 py-2 text-sm font-medium"
          >
            Retry
          </button>
        </div>
      ) : (
        <SkillCardGrid
          skills={skills}
          installedSlugs={installedSlugs}
          installingSlug={installingSlug}
          onInstall={onInstall}
        />
      )}
    </div>
  );
}
```

---

### Task 11: Create InstalledTab component

**Files:**
- Create: `web_kbot/src/pages/skillhub/InstalledTab.tsx`

- [ ] **Step 1: Create InstalledTab**

Create `web_kbot/src/pages/skillhub/InstalledTab.tsx`:

```tsx
import { useEffect, useCallback, useState } from 'react';
import { Loader2, ArrowUpCircle, ExternalLink } from 'lucide-react';
import { listInstalled, upgradeSkill } from '@/lib/skillhubApi';
import type { InstalledSkill } from './types';

export default function InstalledTab() {
  const [skills, setSkills] = useState<InstalledSkill[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [upgradingSlug, setUpgradingSlug] = useState<string | null>(null);
  const [upgradeResult, setUpgradeResult] = useState<string | null>(null);

  const fetchInstalled = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const resp = await listInstalled();
      setSkills(resp.skills);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load installed skills');
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchInstalled();
  }, [fetchInstalled]);

  const handleUpgrade = async (slug: string) => {
    setUpgradingSlug(slug);
    setUpgradeResult(null);
    try {
      const result = await upgradeSkill(slug, false, 60);
      setUpgradeResult(result.message);
      await fetchInstalled();
    } catch (err) {
      setUpgradeResult(err instanceof Error ? err.message : 'Upgrade failed');
    } finally {
      setUpgradingSlug(null);
    }
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center py-12">
        <Loader2 className="h-8 w-8 animate-spin" style={{ color: 'var(--pc-accent)' }} />
      </div>
    );
  }

  if (error) {
    return (
      <div className="card card-radius-6 p-8 text-center">
        <p style={{ color: 'var(--color-status-error)' }}>{error}</p>
        <button onClick={fetchInstalled} className="btn-electric mt-4 px-4 py-2 text-sm font-medium">
          Retry
        </button>
      </div>
    );
  }

  if (skills.length === 0) {
    return (
      <div className="card card-radius-6 p-8 text-center">
        <p style={{ color: 'var(--pc-text-muted)' }}>No skills installed yet. Browse the marketplace to find skills.</p>
      </div>
    );
  }

  return (
    <div className="space-y-2">
      {upgradeResult && (
        <div
          className="p-3 rounded-lg text-xs"
          style={{
            background: upgradeResult.includes('failed') ? 'rgba(239, 68, 68, 0.1)' : 'rgba(0, 230, 138, 0.1)',
            color: upgradeResult.includes('failed') ? 'var(--color-status-error)' : 'var(--color-status-success)',
          }}
        >
          {upgradeResult}
        </div>
      )}
      {skills.map((skill) => (
        <div
          key={skill.slug}
          className="card card-radius-6 p-4 flex items-center gap-4"
        >
          <div className="min-w-0 flex-1">
            <h4 className="text-sm font-semibold truncate" style={{ color: 'var(--pc-text-primary)' }}>
              {skill.name || skill.slug}
            </h4>
            <div className="flex items-center gap-3 text-xs mt-1" style={{ color: 'var(--pc-text-faint)' }}>
              <span>{skill.slug}</span>
              {skill.version && <span>v{skill.version}</span>}
              {skill.installed_at > 0 && (
                <span>{new Date(skill.installed_at * 1000).toLocaleDateString()}</span>
              )}
            </div>
          </div>
          <button
            onClick={() => handleUpgrade(skill.slug)}
            disabled={upgradingSlug === skill.slug}
            className="flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-medium transition-all"
            style={{
              color: 'var(--pc-accent)',
              border: '1px solid var(--pc-accent-dim)',
              background: 'var(--pc-accent-glow)',
              cursor: upgradingSlug === skill.slug ? 'wait' : 'pointer',
              opacity: upgradingSlug === skill.slug ? 0.7 : 1,
            }}
          >
            {upgradingSlug === skill.slug ? (
              <Loader2 className="h-3.5 w-3.5 animate-spin" />
            ) : (
              <ArrowUpCircle className="h-3.5 w-3.5" />
            )}
            Upgrade
          </button>
        </div>
      ))}
    </div>
  );
}
```

---

### Task 12: Create InstallModal component

**Files:**
- Create: `web_kbot/src/pages/skillhub/InstallModal.tsx`

- [ ] **Step 1: Create InstallModal**

Create `web_kbot/src/pages/skillhub/InstallModal.tsx`:

```tsx
import { useState } from 'react';
import { Loader2, Check, X } from 'lucide-react';
import { installSkill } from '@/lib/skillhubApi';

interface InstallModalProps {
  slug: string;
  name: string;
  isOpen: boolean;
  onClose: () => void;
  onSuccess: () => void;
}

type InstallState = 'confirm' | 'installing' | 'success' | 'error';

export default function InstallModal({ slug, name, isOpen, onClose, onSuccess }: InstallModalProps) {
  const [state, setState] = useState<InstallState>('confirm');
  const [force, setForce] = useState(false);
  const [errorMsg, setErrorMsg] = useState('');

  if (!isOpen) return null;

  const handleInstall = async () => {
    setState('installing');
    setErrorMsg('');
    try {
      const source = slug.includes(':') ? slug : `clawhub:${slug}`;
      const result = await installSkill(source, force, 60);
      if (result.success) {
        setState('success');
        onSuccess();
        setTimeout(() => {
          onClose();
          setState('confirm');
        }, 2000);
      } else {
        setState('error');
        setErrorMsg(result.message);
      }
    } catch (err) {
      setState('error');
      setErrorMsg(err instanceof Error ? err.message : 'Installation failed');
    }
  };

  const handleClose = () => {
    if (state !== 'installing') {
      onClose();
      setState('confirm');
      setErrorMsg('');
    }
  };

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center px-4"
      onClick={handleClose}
    >
      <div className="absolute inset-0" style={{ background: 'rgba(0,0,0,0.5)' }} />
      <div
        className="relative surface-panel p-6 w-full max-w-md rounded-2xl animate-fade-in-scale"
        onClick={(e) => e.stopPropagation()}
      >
        <button
          onClick={handleClose}
          disabled={state === 'installing'}
          className="absolute top-4 right-4 p-1 rounded-lg transition-colors"
          style={{ color: 'var(--pc-text-muted)' }}
          onMouseEnter={(e) => { e.currentTarget.style.color = 'var(--pc-text-primary)'; }}
          onMouseLeave={(e) => { e.currentTarget.style.color = 'var(--pc-text-muted)'; }}
        >
          <X className="h-4 w-4" />
        </button>

        <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--pc-text-primary)' }}>
          Install Skill
        </h3>

        {state === 'confirm' && (
          <>
            <p className="text-sm mb-4" style={{ color: 'var(--pc-text-muted)' }}>
              Install <strong style={{ color: 'var(--pc-text-primary)' }}>{name || slug}</strong> from SkillHub?
            </p>
            <label className="flex items-center gap-2 mb-6 cursor-pointer">
              <input
                type="checkbox"
                checked={force}
                onChange={(e) => setForce(e.target.checked)}
                className="rounded"
              />
              <span className="text-xs" style={{ color: 'var(--pc-text-muted)' }}>
                Force overwrite if already installed
              </span>
            </label>
            <div className="flex justify-end gap-3">
              <button
                onClick={handleClose}
                className="px-4 py-2 rounded-lg text-sm font-medium transition-colors"
                style={{ color: 'var(--pc-text-muted)', border: '1px solid var(--pc-border)' }}
              >
                Cancel
              </button>
              <button
                onClick={handleInstall}
                className="btn-electric px-4 py-2 text-sm font-medium"
              >
                Install
              </button>
            </div>
          </>
        )}

        {state === 'installing' && (
          <div className="flex flex-col items-center gap-3 py-4">
            <Loader2 className="h-8 w-8 animate-spin" style={{ color: 'var(--pc-accent)' }} />
            <p className="text-sm" style={{ color: 'var(--pc-text-muted)' }}>
              Installing {name || slug}...
            </p>
          </div>
        )}

        {state === 'success' && (
          <div className="flex flex-col items-center gap-3 py-4">
            <Check className="h-8 w-8" style={{ color: 'var(--color-status-success)' }} />
            <p className="text-sm" style={{ color: 'var(--color-status-success)' }}>
              {name || slug} installed successfully!
            </p>
          </div>
        )}

        {state === 'error' && (
          <>
            <div className="flex flex-col items-center gap-3 py-4">
              <X className="h-8 w-8" style={{ color: 'var(--color-status-error)' }} />
              <p className="text-sm text-center" style={{ color: 'var(--color-status-error)' }}>
                Installation failed
              </p>
              <p className="text-xs text-center max-w-full break-words" style={{ color: 'var(--pc-text-muted)' }}>
                {errorMsg}
              </p>
            </div>
            <div className="flex justify-end gap-3">
              <button
                onClick={handleClose}
                className="px-4 py-2 rounded-lg text-sm font-medium transition-colors"
                style={{ color: 'var(--pc-text-muted)', border: '1px solid var(--pc-border)' }}
              >
                Close
              </button>
              <button
                onClick={handleInstall}
                className="btn-electric px-4 py-2 text-sm font-medium"
              >
                Retry
              </button>
            </div>
          </>
        )}
      </div>
    </div>
  );
}
```

---

### Task 13: Create SkillHubPage main container

**Files:**
- Create: `web_kbot/src/pages/skillhub/SkillHubPage.tsx`

- [ ] **Step 1: Create SkillHubPage**

Create `web_kbot/src/pages/skillhub/SkillHubPage.tsx`:

```tsx
import { useState, useCallback, useEffect } from 'react';
import { Store, Loader2 } from 'lucide-react';
import { searchSkills, listInstalled } from '@/lib/skillhubApi';
import SearchBar from './SearchBar';
import ShowcaseTab from './ShowcaseTab';
import InstalledTab from './InstalledTab';
import SkillCardGrid from './SkillCardGrid';
import InstallModal from './InstallModal';
import type { SkillSearchResult, InstalledSkill } from './types';

type TabKey = 'showcase' | 'installed';

export default function SkillHubPage() {
  const [activeTab, setActiveTab] = useState<TabKey>('showcase');
  const [searchQuery, setSearchQuery] = useState('');
  const [searchResults, setSearchResults] = useState<SkillSearchResult[]>([]);
  const [searchLoading, setSearchLoading] = useState(false);
  const [installedSkills, setInstalledSkills] = useState<InstalledSkill[]>([]);
  const [installingSlug, setInstallingSlug] = useState<string | null>(null);
  const [modalSlug, setModalSlug] = useState<string | null>(null);
  const [modalName, setModalName] = useState('');

  const installedSlugs = new Set(installedSkills.map((s) => s.slug));

  const fetchInstalled = useCallback(async () => {
    try {
      const resp = await listInstalled();
      setInstalledSkills(resp.skills);
    } catch {
      // silently ignore — installed list is supplementary
    }
  }, []);

  useEffect(() => {
    fetchInstalled();
  }, [fetchInstalled]);

  const handleSearch = useCallback(async (query: string) => {
    setSearchQuery(query);
    if (!query) {
      setSearchResults([]);
      return;
    }
    setSearchLoading(true);
    try {
      const resp = await searchSkills(query);
      setSearchResults(resp.results);
    } catch {
      setSearchResults([]);
    } finally {
      setSearchLoading(false);
    }
  }, []);

  const handleInstall = (slug: string) => {
    const skill = searchResults.find((s) => s.slug === slug);
    setModalSlug(slug);
    setModalName(skill?.name || slug);
  };

  const handleInstallSuccess = () => {
    fetchInstalled();
  };

  return (
    <div className="p-6 space-y-6 animate-fade-in">
      <div className="flex items-center gap-2">
        <Store className="h-5 w-5" style={{ color: 'var(--pc-accent)' }} />
        <h2 className="text-sm font-semibold uppercase tracking-wider" style={{ color: 'var(--pc-text-primary)' }}>
          SkillHub Marketplace
        </h2>
      </div>

      <SearchBar onSearch={handleSearch} isLoading={searchLoading} />

      {!searchQuery ? (
        <>
          <div className="flex gap-2">
            {(['showcase', 'installed'] as TabKey[]).map((tab) => (
              <button
                key={tab}
                onClick={() => setActiveTab(tab)}
                className="px-3.5 py-1.5 rounded-[6px] text-xs font-semibold transition-all capitalize"
                style={
                  activeTab === tab
                    ? { background: 'var(--pc-accent)', color: 'white' }
                    : { color: 'var(--pc-text-muted)', border: '1px solid var(--pc-border)', background: 'transparent' }
                }
              >
                {tab === 'showcase' ? 'Marketplace' : `Installed (${installedSkills.length})`}
              </button>
            ))}
          </div>

          {activeTab === 'showcase' && (
            <ShowcaseTab
              installedSlugs={installedSlugs}
              installingSlug={installingSlug}
              onInstall={handleInstall}
            />
          )}
          {activeTab === 'installed' && <InstalledTab />}
        </>
      ) : (
        <div className="space-y-4">
          <h3 className="text-xs font-semibold uppercase tracking-wider" style={{ color: 'var(--pc-text-faint)' }}>
            Search results for "{searchQuery}"
          </h3>
          {searchLoading ? (
            <div className="flex items-center justify-center py-12">
              <Loader2 className="h-8 w-8 animate-spin" style={{ color: 'var(--pc-accent)' }} />
            </div>
          ) : (
            <SkillCardGrid
              skills={searchResults}
              installedSlugs={installedSlugs}
              installingSlug={installingSlug}
              onInstall={handleInstall}
            />
          )}
        </div>
      )}

      {modalSlug && (
        <InstallModal
          slug={modalSlug}
          name={modalName}
          isOpen={true}
          onClose={() => setModalSlug(null)}
          onSuccess={handleInstallSuccess}
        />
      )}
    </div>
  );
}
```

---

### Task 14: Register `/skillhub` route in App.tsx

**Files:**
- Modify: `web_kbot/src/App.tsx`

- [ ] **Step 1: Add import**

Add the import for SkillHubPage after the Channel import (~line 21):

```tsx
import SkillHubPage from './pages/skillhub/SkillHubPage';
```

- [ ] **Step 2: Add route**

Add the route inside the `<Route element={<Layout />}>` block, after the Channel route (~line 257):

```tsx
            <Route path="/skillhub" element={<SkillHubPage />} />
```

---

### Task 15: Add `/skillhub` to navigation

**Files:**
- Modify: `web_kbot/src/components/layout/navModes.ts`

- [ ] **Step 1: Add NavItemPath type**

Add `'/skillhub'` to the `NavItemPath` union type:

```typescript
export type NavItemPath =
  | '/agent'
  | '/agent/history'
  | '/general'
  | '/model-switch'
  | '/skills'
  | '/skillhub'
  | '/cost'
  | '/memory'
  | '/cron';
```

- [ ] **Step 2: Add to configNavItems array**

Add `'/skillhub'` to the `configNavItems` array, after `'/skills'`:

```typescript
const configNavItems: NavItemPath[] = [
  '/agent',
  '/general',
  '/model-switch',
  '/skills',
  '/skillhub',
  '/cost',
  '/memory',
  '/cron',
];
```

- [ ] **Step 3: Add to visibleNavItems in config mode**

Update the `visibleNavItems` in the config nav mode to include `'/skillhub'` after `'/skills'`:

```typescript
    visibleNavItems: ['/general', '/model-switch', '/skills', '/skillhub', '/cost', '/memory', '/cron'],
```

---

### Task 16: Verify full build

- [ ] **Step 1: Check Rust compilation**

Run: `cargo check -p kylinbot-gateway 2>&1 | tail -5`
Expected: `Finished` without errors

- [ ] **Step 2: Check frontend TypeScript**

Run: `cd /home/cong/project/gitee/kybot/web_kbot && npx tsc --noEmit --pretty 2>&1 | head -30`
Expected: No errors

- [ ] **Step 3: Check frontend build**

Run: `cd /home/cong/project/gitee/kybot/web_kbot && npx vite build 2>&1 | tail -10`
Expected: Build succeeds

---

### Task 17: Run clippy and fmt

- [ ] **Step 1: Run cargo fmt check**

Run: `cargo fmt --all -- --check 2>&1 | head -20`
Expected: No formatting issues

- [ ] **Step 2: Run cargo clippy**

Run: `cargo clippy -p kylinbot-gateway -- -D warnings 2>&1 | tail -20`
Expected: No warnings
