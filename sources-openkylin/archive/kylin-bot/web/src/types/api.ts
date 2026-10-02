export interface StatusResponse {
  provider: string | null;
  model: string;
  temperature: number;
  uptime_seconds: number;
  gateway_port: number;
  version?: string | null;
  locale: string;
  memory_backend: string;
  paired: boolean;
  channels: Record<string, boolean>;
  health: HealthSnapshot;
}

export interface HealthSnapshot {
  pid: number;
  updated_at: string;
  uptime_seconds: number;
  components: Record<string, ComponentHealth>;
}

export interface ComponentHealth {
  status: string;
  updated_at: string;
  last_ok: string | null;
  last_error: string | null;
  restart_count: number;
}

export interface ToolSpec {
  name: string;
  description: string;
  parameters: any;
}

export interface CronJob {
  id: string;
  name: string | null;
  expression: string;
  command: string;
  prompt: string | null;
  job_type: string;
  schedule: {
    kind: 'cron' | 'at' | 'every';
    expr?: string;
    tz?: string | null;
    at?: string;
    every_ms?: number;
  };
  enabled: boolean;
  delivery: {
    mode: string;
    channel?: string | null;
    to?: string | null;
    best_effort?: boolean;
  };
  session_target?: 'isolated' | 'main';
  model?: string | null;
  allowed_tools?: string[] | null;
  delete_after_run: boolean;
  created_at: string;
  next_run: string;
  last_run: string | null;
  last_status: string | null;
  last_output: string | null;
}

export interface CronRun {
  id: number;
  job_id: string;
  started_at: string;
  finished_at: string;
  status: string;
  output: string | null;
  duration_ms: number | null;
}

export interface Integration {
  name: string;
  description: string;
  category: string;
  status: 'Available' | 'Active';
}

export interface DiagResult {
  severity: 'ok' | 'warn' | 'error';
  category: string;
  message: string;
}

export interface MemoryEntry {
  id: string;
  key: string;
  content: string;
  category: string;
  timestamp: string;
  session_id: string | null;
  score: number | null;
}

export interface CostSummary {
  session_cost_usd: number;
  daily_cost_usd: number;
  monthly_cost_usd: number;
  total_tokens: number;
  request_count: number;
  by_model: Record<string, ModelStats>;
}

export interface ModelStats {
  model: string;
  cost_usd: number;
  total_tokens: number;
  request_count: number;
}

export interface CliTool {
  name: string;
  path: string;
  version: string | null;
  category: string;
}

export interface Session {
  id?: string;
  session_id?: string;
  channel?: string;
  status?: string;
  started_at?: string;
  created_at: string;
  last_activity: string;
  message_count: number;
  name?: string;
}

export interface ChannelDetail {
  name: string;
  type: string;
  enabled: boolean;
  status: 'active' | 'inactive' | 'error';
  message_count: number;
  last_message_at: string | null;
  health: 'healthy' | 'degraded' | 'down';
}

export interface AgentRuntimeInfo {
  status: 'idle' | 'running' | 'error';
  stage: string;
  active_tasks: number;
  current_task: string | null;
  started_at: string | null;
  last_started_at: string | null;
  last_finished_at: string | null;
  last_error: string | null;
}

export interface AgentProfile {
  name: string;
  provider: string;
  model: string;
  system_prompt?: string | null;
  temperature?: number | null;
  max_depth: number;
  agentic: boolean;
  allowed_tools: string[];
  max_iterations: number;
  skills_directory?: string | null;
  runtime: AgentRuntimeInfo;
}

export interface SSEEvent {
  type: string;
  timestamp?: string;
  [key: string]: any;
}

export interface WsMessage {
  event_type?: string;
  type:
    | 'message'
    | 'chunk'
    | 'chunk_reset'
    | 'thinking'
    | 'tool_call'
    | 'tool_result'
    | 'done'
    | 'error'
    | 'session_start'
    | 'connected'
    | 'cron_result'
    | 'approval_request'
    | 'approval_restored'
    | 'approval_resolved'
    | 'approval_expired'
    | 'message_cancelled';
  content?: string;
  full_response?: string;
  name?: string;
  tool?: string;
  args?: any;
  output?: string;
  payload?: unknown;
  message?: string;
  code?: string;
  session_id?: string;
  resumed?: boolean;
  message_count?: number;
  timestamp?: string;
  job_id?: string;
  success?: boolean;
  message_id?: string;
  request_id?: string;
  tool_name?: string;
  arguments_summary?: string;
  expires_at?: number | null;
  choices?: Array<'yes' | 'no' | 'always'>;
  decision?: string;
}

export interface WsOutboundAttachment {
  file_name: string;
  data_base64: string;
  mime_type?: string;
}

/** Row from GET /api/sessions/{id}/messages */
export interface SessionMessageRow {
  role: string;
  content: string;
}

export interface SessionMessagesResponse {
  session_id: string;
  messages: SessionMessageRow[];
  session_persistence: boolean;
}

export interface Skill {
  name: string;
  description: string;
  version: string;
  author: string | null;
  tags: string[];
  enabled: boolean;
}
