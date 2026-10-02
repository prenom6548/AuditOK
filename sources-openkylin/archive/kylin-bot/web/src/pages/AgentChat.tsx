import { useState, useEffect, useRef, useCallback } from 'react';
import { Send, Bot, User, AlertCircle, Copy, Check, ChevronDown, ChevronRight, Paperclip } from 'lucide-react';
import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import { useOutletContext } from 'react-router-dom';
import type { WsMessage } from '@/types/api';
import { WebSocketClient, getOrCreateSessionId } from '@/lib/ws';
import { generateUUID } from '@/lib/uuid';
import { useDraft } from '@/hooks/useDraft';
import { useTheme } from '@/hooks/useTheme';
import { t } from '@/lib/i18n';
import { getSessionMessages } from '@/lib/api';
import ToolCallCard from '@/components/ToolCallCard';
import type { ToolCallInfo } from '@/components/ToolCallCard';
import {
  loadChatHistory,
  mapServerMessagesToPersisted,
  persistedToUiMessages,
  saveChatHistory,
  uiMessagesToPersisted,
} from '@/lib/chatHistoryStorage';
import type { AgentPageOutletContext } from '@/components/layout/Layout';
import { assetBasePath } from '@/lib/basePath';
import { useAgent } from '@/contexts/AgentContext';

interface ChatMessage {
  id: string;
  role: 'user' | 'agent';
  kind?: 'text' | 'event';
  content?: string;
  thinking?: string;
  markdown?: boolean;
  toolCall?: ToolCallInfo;
  eventTitle?: string;
  eventSummary?: string;
  eventPayload?: WsMessage;
  timestamp: Date;
}

interface PendingApproval {
  requestId: string;
  toolName: string;
  argumentsSummary: string;
  expiresAt: number | null;
  choices: Array<'yes' | 'no' | 'always'>;
  submitting: boolean;
}

const DRAFT_KEY = 'agent-chat';
const CHAT_STORAGE_PREFIX = 'zeroclaw_agent_chat_';
const MAX_VISIBLE_MESSAGES = 100;
const TIME_DIVIDER_GAP_MS = 5 * 60 * 1000;

const LIGHT_SUGGESTION_IDS = ['agent.light_suggestion_one', 'agent.light_suggestion_two', 'agent.light_suggestion_three'] as const;

function lightThemeIconPath(fileName: string): string {
  return `${assetBasePath}/icons/${encodeURIComponent(fileName)}`;
}

function LightThemeAssetIcon({
  fileName,
  alt,
  className,
}: {
  fileName: string;
  alt: string;
  className: string;
}) {
  return <img src={lightThemeIconPath(fileName)} alt={alt} className={className} />;
}

function SuggestedPromptGlyph() {
  return (
    <span className="relative inline-block h-[15px] w-[15px]" aria-hidden="true">
      <span className="absolute left-[1px] top-[1px] h-[5px] w-[5px] rounded-[2px] border" style={{ borderColor: '#717784' }} />
      <span className="absolute right-[1px] top-[1px] h-[5px] w-[5px] rounded-full border" style={{ borderColor: '#717784' }} />
      <span className="absolute bottom-[1px] left-[4px] h-[5px] w-[7px] rotate-[-18deg] rounded-[2px] border" style={{ borderColor: '#717784' }} />
    </span>
  );
}

function chatStorageKey(): string {
  return `${CHAT_STORAGE_PREFIX}${getOrCreateSessionId()}`;
}

function shouldShowTimeDivider(current: Date, previous?: Date): boolean {
  if (!previous) {
    return true;
  }
  return current.getTime() - previous.getTime() >= TIME_DIVIDER_GAP_MS;
}

function formatConversationTime(timestamp: Date, now = new Date()): string {
  const sameYear = timestamp.getFullYear() === now.getFullYear();
  const todayStart = new Date(now.getFullYear(), now.getMonth(), now.getDate());
  const yesterdayStart = new Date(todayStart);
  yesterdayStart.setDate(todayStart.getDate() - 1);
  const weekStart = new Date(todayStart);
  weekStart.setDate(todayStart.getDate() - 6);

  const hhmm = timestamp.toLocaleTimeString('zh-CN', {
    hour: '2-digit',
    minute: '2-digit',
    hour12: false,
  });

  if (timestamp >= todayStart) {
    return hhmm;
  }

  if (timestamp >= yesterdayStart) {
    return `昨天 ${hhmm}`;
  }

  if (timestamp >= weekStart) {
    const weekday = ['周日', '周一', '周二', '周三', '周四', '周五', '周六'][timestamp.getDay()];
    return `${weekday} ${hhmm}`;
  }

  const year = sameYear ? `${timestamp.getMonth() + 1}月${timestamp.getDate()}日` : `${timestamp.getFullYear()}年${timestamp.getMonth() + 1}月${timestamp.getDate()}日`;
  return `${year} ${hhmm}`;
}

function capMessages(messages: ChatMessage[]): ChatMessage[] {
  if (messages.length <= MAX_VISIBLE_MESSAGES) {
    return messages;
  }
  return messages.slice(messages.length - MAX_VISIBLE_MESSAGES);
}

function loadPersistedMessages(): ChatMessage[] {
  try {
    const raw = sessionStorage.getItem(chatStorageKey());
    if (!raw) {
      return [];
    }
    const parsed = JSON.parse(raw) as Array<
      Omit<ChatMessage, 'timestamp'> & { timestamp: string }
    >;
    return capMessages(
      parsed.map((message) => ({
        ...message,
        kind: message.kind ?? 'text',
        timestamp: new Date(message.timestamp),
      })),
    );
  } catch {
    return [];
  }
}

function persistMessages(messages: ChatMessage[]): void {
  try {
    sessionStorage.setItem(chatStorageKey(), JSON.stringify(capMessages(messages)));
  } catch {
    // Ignore storage failures and keep the in-memory chat usable.
  }
}

function persistableMessages(
  messages: ChatMessage[],
): Array<{
  id: string;
  role: 'user' | 'agent';
  content: string;
  thinking?: string;
  markdown?: boolean;
  timestamp: Date;
}> {
  return messages.filter(
    (
      message,
    ): message is {
      id: string;
      role: 'user' | 'agent';
      content: string;
      thinking?: string;
      markdown?: boolean;
      timestamp: Date;
    } => message.kind !== 'event' && typeof message.content === 'string',
  );
}

function extractToolName(msg: WsMessage): string {
  if (msg.name && msg.name.trim()) {
    return msg.name.trim();
  }

  const content = msg.content?.trim();
  if (!content) {
    return 'tool';
  }

  const quotedName = content.match(/`([^`]+)`/);
  if (quotedName?.[1]) {
    return quotedName[1];
  }

  const plainName = content.match(/^Running\s+([A-Za-z0-9_.-]+)/i);
  if (plainName?.[1]) {
    return plainName[1];
  }

  return 'tool';
}

function asRecord(value: unknown): Record<string, unknown> | undefined {
  return value && typeof value === 'object' && !Array.isArray(value)
    ? (value as Record<string, unknown>)
    : undefined;
}

function asString(value: unknown): string | undefined {
  return typeof value === 'string' && value.trim() ? value : undefined;
}

function normalizeIncomingMessage(msg: WsMessage): WsMessage {
  const eventType = msg.event_type?.trim().toLowerCase();
  const payload = asRecord(msg.payload);

  if (eventType === 'tool_call_start') {
    return {
      ...msg,
      type: 'tool_call',
      name: msg.name ?? msg.tool ?? asString(payload?.tool) ?? asString(payload?.name),
      args: msg.args ?? payload?.arguments ?? payload?.args,
    };
  }

  if (eventType === 'tool_call_result') {
    return {
      ...msg,
      type: 'tool_result',
      name: msg.name ?? msg.tool ?? asString(payload?.tool) ?? asString(payload?.name),
      output: msg.output ?? asString(payload?.output) ?? asString(payload?.content),
    };
  }

  return msg;
}

function titleCaseToken(token: string): string {
  if (token.toLowerCase() === 'llm') {
    return 'LLM';
  }
  return token.charAt(0).toUpperCase() + token.slice(1).toLowerCase();
}

function beautifyEventTitle(raw: string): string {
  const normalized = raw.trim().toLowerCase();
  if (!normalized) {
    return 'Event';
  }

  if (normalized === 'tool_call' || normalized === 'tool_call_start' || normalized === 'tool_result' || normalized === 'tool_call_result') {
    return 'Tool';
  }

  if (normalized === 'llm_request' || normalized === 'llm_response') {
    return 'LLM';
  }

  const simplified = normalized
    .replace(/^(web_|ws_)/, '')
    .replace(/_(start|end|request|response|call|result|event)$/, '');

  return simplified
    .split(/[_\-.]+/)
    .filter(Boolean)
    .map(titleCaseToken)
    .join(' ');
}

function eventTitleForMessage(msg: WsMessage): string {
  const source = msg.event_type ?? msg.type ?? msg.name ?? extractToolName(msg);
  return beautifyEventTitle(source);
}

function stringifyEventPayload(payload: WsMessage): string {
  return JSON.stringify(payload, null, 2);
}

function eventSummaryForMessage(msg: WsMessage): string | undefined {
  switch (msg.type) {
    case 'tool_call':
    case 'tool_result':
      return msg.name ?? extractToolName(msg);
    case 'approval_request':
    case 'approval_restored':
      return msg.tool_name ?? 'tool';
    case 'approval_resolved':
      return msg.decision ? `decision: ${msg.decision}` : undefined;
    case 'approval_expired':
      return msg.request_id ?? msg.message_id ?? undefined;
    default:
      return undefined;
  }
}

function shouldRenderEventMessage(msg: WsMessage): boolean {
  switch (msg.type) {
    case 'approval_request':
    case 'approval_restored':
    case 'approval_resolved':
    case 'approval_expired':
      return true;
    default:
      return false;
  }
}

function shouldKeepEventVisibleWhenSystemMessagesHidden(msg: ChatMessage): boolean {
  const eventType = msg.eventPayload?.type;
  return eventType === 'approval_request'
    || eventType === 'approval_restored'
    || eventType === 'approval_resolved'
    || eventType === 'approval_expired';
}

export default function AgentChat() {
  const sessionIdRef = useRef(getOrCreateSessionId());
  const outletContext = useOutletContext<AgentPageOutletContext | undefined>();
  const { draft, saveDraft, clearDraft } = useDraft(DRAFT_KEY);
  const { resolvedTheme } = useTheme();
  const isLight = resolvedTheme === 'light';
  const [messages, setMessages] = useState<ChatMessage[]>(() => loadPersistedMessages());
  const [historyReady, setHistoryReady] = useState(false);
  const [input, setInput] = useState(draft);
  const [typing, setTyping] = useState(false);
  const [connected, setConnected] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [pendingApproval, setPendingApproval] = useState<PendingApproval | null>(null);
  const [expandedEventIds, setExpandedEventIds] = useState<Set<string>>(new Set());
  const suggestionPrompts = LIGHT_SUGGESTION_IDS.map((key) => t(key));

  const {
    currentModel,
    availableModels,
    switchModel,
    modelLoading,
  } = useAgent();

  const [wsEpoch, setWsEpoch] = useState(0);
  const [showModelDropdown, setShowModelDropdown] = useState(false);
  const modelDropdownRef = useRef<HTMLDivElement>(null);

  const wsRef = useRef<WebSocketClient | null>(null);
  const messagesEndRef = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLTextAreaElement>(null);
  const [copiedId, setCopiedId] = useState<string | null>(null);
  const pendingContentRef = useRef('');
  const pendingThinkingRef = useRef('');
  // Snapshot of thinking captured at chunk_reset, so it survives the reset.
  const capturedThinkingRef = useRef('');
  const [streamingContent, setStreamingContent] = useState('');
  const [streamingThinking, setStreamingThinking] = useState('');
  const showAgentSystemMessages = outletContext?.showAgentSystemMessages ?? true;
  const visibleMessages = showAgentSystemMessages
    ? messages
    : messages.filter((message) =>
      message.kind === 'event'
        ? shouldKeepEventVisibleWhenSystemMessagesHidden(message)
        : Boolean(message.content?.trim()),
    );

  // Persist draft to in-memory store so it survives route changes
  useEffect(() => {
    saveDraft(input);
  }, [input, saveDraft]);

  // Hydrate chat from server (preferred) or localStorage fallback
  useEffect(() => {
    const sid = sessionIdRef.current;
    let cancelled = false;

    (async () => {
      try {
        const res = await getSessionMessages(sid);
        if (cancelled) return;
        if (res.session_persistence && res.messages.length > 0) {
          setMessages((prev) =>
            prev.length > 0 ? prev : persistedToUiMessages(mapServerMessagesToPersisted(res.messages)),
          );
        } else if (!res.session_persistence) {
          setMessages((prev) => {
            if (prev.length > 0) return prev;
            const ls = loadChatHistory(sid);
            return ls.length ? persistedToUiMessages(ls) : prev;
          });
        }
      } catch {
        if (!cancelled) {
          setMessages((prev) => {
            if (prev.length > 0) return prev;
            const ls = loadChatHistory(sid);
            return ls.length ? persistedToUiMessages(ls) : prev;
          });
        }
      } finally {
        if (!cancelled) setHistoryReady(true);
      }
    })();

    return () => {
      cancelled = true;
    };
  }, []);

  // Mirror transcript to localStorage (bounded); server remains source of truth when persistence is on
  useEffect(() => {
    if (!historyReady) return;
    saveChatHistory(sessionIdRef.current, uiMessagesToPersisted(persistableMessages(messages)));
  }, [messages, historyReady]);

  useEffect(() => {
    const ws = new WebSocketClient();

    const appendEventMessage = (msg: WsMessage) => {
      if (!shouldRenderEventMessage(msg)) {
        return;
      }
      setMessages((prev) =>
        capMessages([
          ...prev,
          {
            id: generateUUID(),
            role: 'agent',
            kind: 'event',
            eventTitle: eventTitleForMessage(msg),
            eventSummary: eventSummaryForMessage(msg),
            eventPayload: msg,
            timestamp: new Date(),
          },
        ]),
      );
    };

    ws.onOpen = () => {
      setConnected(true);
      setError(null);
    };

    ws.onClose = (ev: CloseEvent) => {
      setConnected(false);
      if (ev.code !== 1000 && ev.code !== 1001) {
        setError(`Connection closed unexpectedly (code: ${ev.code}). Please check your configuration.`);
      }
    };

    ws.onError = () => {
      setError(t('agent.connection_error'));
    };

    ws.onMessage = (msg: WsMessage) => {
      const normalizedMsg = normalizeIncomingMessage(msg);
      switch (normalizedMsg.type) {
        case 'connected':
          break;

        case 'thinking':
          setTyping(true);
          pendingThinkingRef.current += normalizedMsg.content ?? '';
          setStreamingThinking(pendingThinkingRef.current);
          break;

        case 'chunk':
          setTyping(true);
          pendingContentRef.current += normalizedMsg.content ?? '';
          setStreamingContent(pendingContentRef.current);
          break;

        case 'chunk_reset':
          // Server signals that the authoritative done message follows.
          // Snapshot thinking before clearing display state.
          capturedThinkingRef.current = pendingThinkingRef.current;
          pendingContentRef.current = '';
          pendingThinkingRef.current = '';
          setStreamingContent('');
          setStreamingThinking('');
          break;

        case 'message':
        case 'done': {
          const content = normalizedMsg.full_response ?? normalizedMsg.content ?? pendingContentRef.current;
          const thinking = capturedThinkingRef.current || pendingThinkingRef.current || undefined;
          if (content) {
            setMessages((prev) =>
              capMessages([
                ...prev,
                {
                  id: generateUUID(),
                  role: 'agent',
                  content,
                  thinking,
                  markdown: true,
                  timestamp: new Date(),
                },
              ]),
            );
          }
          pendingContentRef.current = '';
          pendingThinkingRef.current = '';
          capturedThinkingRef.current = '';
          setStreamingContent('');
          setStreamingThinking('');
          setTyping(false);
          break;
        }

        case 'tool_call': {
          const toolName = normalizedMsg.name ?? 'unknown';
          const toolArgs = normalizedMsg.args;
          setMessages((prev) => {
            // Dedup: backend streaming may re-send tool_call events before execution.
            // Skip if an unresolved card with the same name+args already exists.
            const argsKey = JSON.stringify(toolArgs ?? {});
            const isDuplicate = prev.some(
              (m) => m.toolCall
                && m.toolCall.output === undefined
                && m.toolCall.name === toolName
                && JSON.stringify(m.toolCall.args ?? {}) === argsKey,
            );
            if (isDuplicate) return prev;

            return [
              ...prev,
              {
                id: generateUUID(),
                role: 'agent' as const,
                content: `${t('agent.tool_call_prefix')} ${toolName}(${argsKey})`,
                toolCall: { name: toolName, args: toolArgs },
                timestamp: new Date(),
              },
            ];
          });
          break;
        }

        case 'approval_request':
        case 'approval_restored':
          appendEventMessage(normalizedMsg);
          setPendingApproval({
            requestId: normalizedMsg.request_id ?? normalizedMsg.message_id ?? generateUUID(),
            toolName: normalizedMsg.tool_name ?? 'tool',
            argumentsSummary: normalizedMsg.arguments_summary ?? '',
            expiresAt: typeof normalizedMsg.expires_at === 'number' ? normalizedMsg.expires_at : null,
            choices: normalizedMsg.choices ?? ['yes', 'no', 'always'],
            submitting: false,
          });
          setTyping(false);
          break;

        case 'approval_resolved':
          appendEventMessage(normalizedMsg);
          setPendingApproval(null);
          break;

        case 'approval_expired':
          appendEventMessage(normalizedMsg);
          setPendingApproval(null);
          break;

        case 'message_cancelled':
          setTyping(false);
          pendingContentRef.current = '';
          pendingThinkingRef.current = '';
          setStreamingContent('');
          setStreamingThinking('');
          break;

        case 'tool_result': {
          setMessages((prev) => {
            // Forward scan: find the FIRST unresolved toolCall (order-guaranteed by backend)
            const idx = prev.findIndex((m) => m.toolCall && m.toolCall.output === undefined);
            if (idx !== -1) {
              const updated = [...prev];
              const existing = prev[idx]!;
              updated[idx] = {
                ...existing,
                toolCall: { ...existing.toolCall!, output: normalizedMsg.output ?? '' },
              };
              return updated;
            }
            // Fallback: no unresolved call found — append standalone card
            return [
              ...prev,
              {
                id: generateUUID(),
                role: 'agent' as const,
                content: `${t('agent.tool_result_prefix')} ${normalizedMsg.output ?? ''}`,
                toolCall: { name: normalizedMsg.name ?? 'unknown', output: normalizedMsg.output ?? '' },
                timestamp: new Date(),
              },
            ];
          });
          break;
        }

        case 'error':
          setMessages((prev) =>
            capMessages([
              ...prev,
              {
                id: generateUUID(),
                role: 'agent',
                content: `${t('agent.error_prefix')} ${normalizedMsg.message ?? t('agent.unknown_error')}`,
                timestamp: new Date(),
              },
            ]),
          );
          if (normalizedMsg.code === 'AGENT_INIT_FAILED' || normalizedMsg.code === 'AUTH_ERROR' || normalizedMsg.code === 'PROVIDER_ERROR') {
            setError(`Configuration error: ${normalizedMsg.message}. Please check your provider settings (API key, model, etc.).`);
          } else if (normalizedMsg.code === 'INVALID_JSON' || normalizedMsg.code === 'UNKNOWN_MESSAGE_TYPE' || normalizedMsg.code === 'EMPTY_CONTENT') {
            setError(`Message error: ${normalizedMsg.message}`);
          }
          setTyping(false);
          pendingContentRef.current = '';
          pendingThinkingRef.current = '';
          setStreamingContent('');
          setStreamingThinking('');
          break;
      }
    };

    ws.connect();
    wsRef.current = ws;

    return () => {
      ws.disconnect();
    };
  }, [wsEpoch]);

  useEffect(() => {
    persistMessages(messages);
  }, [messages]);

  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [messages, typing, streamingContent]);

  useEffect(() => {
    function handleClickOutside(e: MouseEvent) {
      if (modelDropdownRef.current && !modelDropdownRef.current.contains(e.target as Node)) {
        setShowModelDropdown(false);
      }
    }
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSend = () => {
    const trimmed = input.trim();
    if (!trimmed || !wsRef.current?.connected || pendingApproval) return;

    setMessages((prev) =>
      capMessages([
        ...prev,
        {
          id: generateUUID(),
          role: 'user',
          content: trimmed,
          timestamp: new Date(),
        },
      ]),
    );

    try {
      wsRef.current.sendMessage(trimmed);
      setTyping(true);
      pendingContentRef.current = '';
      pendingThinkingRef.current = '';
    } catch {
      setError(t('agent.send_error'));
    }

    setInput('');
    clearDraft();
    if (inputRef.current) {
      inputRef.current.style.height = 'auto';
      inputRef.current.focus();
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key !== 'Enter' || e.nativeEvent.isComposing) return;
    if (e.shiftKey || e.ctrlKey || e.metaKey) return;
    e.preventDefault();
    handleSend();
  };

  const handleTextareaChange = (e: React.ChangeEvent<HTMLTextAreaElement>) => {
    setInput(e.target.value);
    e.target.style.height = 'auto';
    e.target.style.height = `${Math.min(e.target.scrollHeight, 200)}px`;
  };

  const applySuggestedPrompt = (prompt: string) => {
    setInput(prompt);
    saveDraft(prompt);
    requestAnimationFrame(() => inputRef.current?.focus());
  };

  const handleModelSwitch = async (model: string) => {
    setShowModelDropdown(false);
    if (model === currentModel) return;
    try {
      await switchModel(model);
      setWsEpoch((e) => e + 1);
    } catch {
      // Error surfaced by switchModel via AgentContext
    }
  };

  const handleApproval = (decision: 'yes' | 'no' | 'always') => {
    if (!pendingApproval || !wsRef.current?.connected) return;
    setPendingApproval((current) =>
      current ? { ...current, submitting: true } : current,
    );
    try {
      wsRef.current.sendApprovalResponse(pendingApproval.requestId, decision);
    } catch {
      setPendingApproval((current) =>
        current ? { ...current, submitting: false } : current,
      );
      setError(t('agent.send_error'));
    }
  };

  const handleCopy = useCallback((msgId: string, content: string) => {
    const onSuccess = () => {
      setCopiedId(msgId);
      setTimeout(() => setCopiedId((prev) => (prev === msgId ? null : prev)), 2000);
    };

    if (navigator.clipboard?.writeText) {
      navigator.clipboard.writeText(content).then(onSuccess).catch(() => {
        // Fallback for insecure contexts (HTTP)
        fallbackCopy(content) && onSuccess();
      });
    } else {
      fallbackCopy(content) && onSuccess();
    }
  }, []);

  const toggleEventExpanded = useCallback((msgId: string) => {
    setExpandedEventIds((prev) => {
      const next = new Set(prev);
      if (next.has(msgId)) {
        next.delete(msgId);
      } else {
        next.add(msgId);
      }
      return next;
    });
  }, []);

  /**
   * Fallback copy using a temporary textarea for HTTP contexts
   * where navigator.clipboard is unavailable.
   */
  function fallbackCopy(text: string): boolean {
    const textarea = document.createElement('textarea');
    textarea.value = text;
    textarea.style.position = 'fixed';
    textarea.style.opacity = '0';
    document.body.appendChild(textarea);
    textarea.select();
    try {
      document.execCommand('copy');
      return true;
    } catch {
      return false;
    } finally {
      document.body.removeChild(textarea);
    }
  }

  return (
    <div className="flex flex-col h-[calc(100vh-3.5rem)]">
      {/* Connection status bar */}
      {error && (
        <div className="px-4 py-2 border-b flex items-center gap-2 text-sm animate-fade-in" style={{ background: 'rgba(239, 68, 68, 0.08)', borderColor: 'rgba(239, 68, 68, 0.2)', color: '#f87171', }}>
          <AlertCircle className="h-4 w-4 shrink-0" />
          {error}
        </div>
      )}

      {/* Messages area */}
      <div className={`flex-1 overflow-y-auto space-y-4 ${isLight ? 'px-8 pb-8 pt-10 md:px-12' : 'p-4'}`}>
        {visibleMessages.length === 0 && (
          isLight ? (
            <div
              data-testid="agent-light-welcome"
              className="mx-auto flex h-full w-full max-w-5xl flex-col items-center text-center animate-fade-in"
              style={{ color: 'var(--pc-text-muted)' }}
            >
              <div className="mt-4 h-[52px] w-[52px] overflow-hidden rounded-[17px] shadow-[0_10px_24px_rgba(74,140,255,0.16)]">
                <img
                  src={`${assetBasePath}/kylin-bot.svg`}
                  alt="ZeroClaw"
                  className="h-full w-full object-cover"
                  onError={(e) => {
                    e.currentTarget.style.display = 'none';
                  }}
                />
              </div>
              <p className="mt-6 text-[22px] font-semibold tracking-[-0.02em]" style={{ color: 'var(--pc-text-primary)' }}>
                {t('agent.light_greeting_title')}
              </p>
              <p className="mt-3 max-w-3xl text-[13px] leading-[1.95]" style={{ color: 'var(--pc-text-muted)' }}>
                {t('agent.light_greeting_subtitle')}
              </p>

              <div className="mt-12 flex w-full items-center justify-between px-1 text-[13px]" style={{ color: 'var(--pc-text-secondary)' }}>
                <span>{t('agent.light_try_label')}</span>
                <button
                  type="button"
                  className="text-[12px] font-medium"
                  style={{ color: 'var(--pc-accent)' }}
                  onClick={() => applySuggestedPrompt(suggestionPrompts[0] ?? '')}
                >
                  {t('agent.light_refresh_label')}
                </button>
              </div>

              <div data-testid="agent-light-suggestions" className="mt-3 grid w-full gap-3.5">
                <button
                  type="button"
                  onClick={() => applySuggestedPrompt(suggestionPrompts[0] ?? '')}
                  className="rounded-[15px] border px-5 py-5 text-left transition-transform duration-200 hover:-translate-y-0.5"
                  style={{
                    background: '#f4f5ff',
                    borderColor: '#ececf8',
                    boxShadow: '0 8px 20px rgba(122, 135, 181, 0.06)',
                  }}
                >
                  <div className="flex items-center gap-2 text-[14px] font-semibold" style={{ color: 'var(--pc-text-primary)' }}>
                    <SuggestedPromptGlyph />
                    <span>{t('agent.light_card_recommended')}</span>
                  </div>
                  <ul className="mt-4 space-y-2.5 pl-4 text-[13px] leading-6" style={{ color: 'var(--pc-text-secondary)' }}>
                    {suggestionPrompts.map((prompt) => (
                      <li key={prompt}>{prompt}</li>
                    ))}
                  </ul>
                </button>
              </div>
            </div>
          ) : (
            <div className="flex flex-col items-center justify-center h-full text-center animate-fade-in" style={{ color: 'var(--pc-text-muted)' }}>
              <div className="h-16 w-16 rounded-3xl flex items-center justify-center mb-4 animate-float" style={{ background: 'var(--pc-accent-glow)' }}>
                <Bot className="h-8 w-8" style={{ color: 'var(--pc-accent)' }} />
              </div>
              <p className="text-lg font-semibold mb-1" style={{ color: 'var(--pc-text-primary)' }}>ZeroClaw Agent</p>
              <p className="text-sm" style={{ color: 'var(--pc-text-muted)' }}>{t('agent.start_conversation')}</p>
            </div>
          )
        )}

        {visibleMessages.map((msg, idx) => {
          const previousTimestamp = idx > 0 ? visibleMessages[idx - 1]?.timestamp : undefined;

          return (
            <div key={msg.id}>
              {shouldShowTimeDivider(msg.timestamp, previousTimestamp) && (
                <div
                  className="flex justify-center py-2"
                  style={{ color: isLight ? '#b0b4bc' : 'var(--pc-text-faint)' }}
                >
                  <span className={`${isLight ? 'text-[12px]' : 'text-[11px]'}`}>
                    {formatConversationTime(msg.timestamp)}
                  </span>
                </div>
              )}

              <div
                className={`group flex items-start gap-3 ${msg.role === 'user' ? 'flex-row-reverse animate-slide-in-right' : 'animate-slide-in-left'
                  }`}
                style={{ animationDelay: `${Math.min(idx * 30, 200)}ms` }}
              >
                <div
                  className={`flex-shrink-0 flex items-center justify-center border ${isLight ? 'w-8 h-8 rounded-[12px]' : 'w-9 h-9 rounded-2xl'}`}
                  style={{
                    background: msg.role === 'user'
                      ? (isLight ? '#ffffff' : 'var(--pc-accent)')
                      : 'var(--pc-bg-elevated)',
                    borderColor: msg.role === 'user'
                      ? (isLight ? '#d9dde5' : 'var(--pc-accent)')
                      : 'var(--pc-border)',
                  }}
                >
                  {msg.role === 'user' ? (
                    <User className="h-4 w-4" style={{ color: isLight ? '#9097a3' : '#ffffff' }} />
                  ) : (
                    <Bot className="h-4 w-4" style={{ color: 'var(--pc-accent)' }} />
                  )}
                </div>
                <div className={`relative ${isLight ? 'max-w-[78%]' : 'max-w-[75%]'}`}>
                  <div
                    className={`${isLight ? 'rounded-[8px] px-[14px] py-[11px]' : 'rounded-2xl px-4 py-3'} border`}
                    style={
                      msg.role === 'user'
                        ? {
                          background: isLight ? '#cfeeff' : 'var(--pc-accent-glow)',
                          borderColor: isLight ? '#c7e7fb' : 'var(--pc-accent-dim)',
                          color: isLight ? '#1f2d3d' : 'var(--pc-text-primary)',
                        }
                        : {
                          background: isLight ? '#ffffff' : 'var(--pc-bg-elevated)',
                          borderColor: isLight ? '#ececf1' : 'var(--pc-border)',
                          color: 'var(--pc-text-primary)',
                          boxShadow: isLight ? '0 1px 2px rgba(39, 47, 61, 0.05)' : 'none',
                        }
                    }
                  >
                    {msg.role === 'agent' && msg.kind !== 'event' && msg.thinking && (
                      <details className="mb-2">
                        <summary className="text-xs cursor-pointer select-none" style={{ color: 'var(--pc-text-muted)' }}>Thinking</summary>
                        <pre className="text-xs mt-1 whitespace-pre-wrap break-words leading-relaxed overflow-auto max-h-60 p-2 rounded-lg" style={{ color: 'var(--pc-text-muted)', background: 'var(--pc-bg-surface)' }}>{msg.thinking}</pre>
                      </details>
                    )}
                    {msg.toolCall ? (
                      <ToolCallCard toolCall={msg.toolCall} />
                    ) : msg.role === 'agent' && msg.kind === 'event' && msg.eventPayload ? (
                      <div className="text-sm">
                        <div className="flex flex-wrap items-center gap-2">
                          <button
                            type="button"
                            onClick={() => toggleEventExpanded(msg.id)}
                            aria-label={expandedEventIds.has(msg.id) ? t('agent.hide_json') : t('agent.show_json')}
                            className="inline-flex h-6 w-6 flex-shrink-0 items-center justify-center rounded-lg border transition-colors"
                            style={{ borderColor: 'var(--pc-border)', background: 'var(--pc-bg)', color: 'var(--pc-text-muted)' }}
                            onMouseEnter={(e) => { e.currentTarget.style.borderColor = 'var(--pc-accent-dim)'; e.currentTarget.style.color = 'var(--pc-text-primary)'; }}
                            onMouseLeave={(e) => { e.currentTarget.style.borderColor = 'var(--pc-border)'; e.currentTarget.style.color = 'var(--pc-text-muted)'; }}
                          >
                            {expandedEventIds.has(msg.id) ? (
                              <ChevronDown className="h-3.5 w-3.5" />
                            ) : (
                              <ChevronRight className="h-3.5 w-3.5" />
                            )}
                          </button>
                          <p className="break-words text-[13px] font-semibold" style={{ color: 'var(--pc-text-primary)' }}>
                            {msg.eventTitle ?? 'Event'}
                          </p>
                          {msg.eventSummary && (
                            <p
                              className="break-words rounded-md border px-2 py-0.5 text-[11px]"
                              style={{
                                color: 'var(--pc-text-muted)',
                                borderColor: 'var(--pc-border)',
                                background: 'var(--pc-bg)',
                              }}
                            >
                              {msg.eventSummary}
                            </p>
                          )}
                        </div>
                        {expandedEventIds.has(msg.id) && (
                          <pre
                            className="mt-3 overflow-x-auto rounded-xl border p-3 text-[12px] leading-6"
                            style={{ borderColor: 'var(--pc-border)', background: 'var(--pc-bg)', color: 'var(--pc-text-secondary)' }}
                          >
                            {stringifyEventPayload(msg.eventPayload)}
                          </pre>
                        )}
                      </div>
                    ) : msg.role === 'agent' && msg.markdown ? (
                      <div className={`${isLight ? 'text-[14px] leading-[1.45]' : 'text-sm leading-relaxed'} break-words chat-markdown`}>
                        <ReactMarkdown remarkPlugins={[remarkGfm]}>{msg.content ?? ''}</ReactMarkdown>
                      </div>
                    ) : msg.role === 'agent' ? (
                      <p className={`${isLight ? 'text-[14px] leading-[1.45]' : 'text-sm leading-relaxed'} whitespace-pre-wrap break-words`}>{msg.content ?? ''}</p>
                    ) : (
                      <p className={`${isLight ? 'text-[14px] leading-[1.45]' : 'text-sm leading-relaxed'} whitespace-pre-wrap break-words`}>{msg.content ?? ''}</p>
                    )}
                  </div>
                  <button
                    onClick={() =>
                      handleCopy(
                        msg.id,
                        msg.kind === 'event' && msg.eventPayload
                          ? stringifyEventPayload(msg.eventPayload)
                          : (msg.content ?? ''),
                      )
                    }
                    aria-label={t('agent.copy_message')}
                    className="absolute top-1 right-1 opacity-0 group-hover:opacity-100 transition-all p-1.5 rounded-xl"
                    style={{ background: 'var(--pc-bg-elevated)', border: '1px solid var(--pc-border)', color: 'var(--pc-text-muted)', }}
                    onMouseEnter={(e) => { e.currentTarget.style.color = 'var(--pc-text-primary)'; e.currentTarget.style.borderColor = 'var(--pc-accent-dim)'; }}
                    onMouseLeave={(e) => { e.currentTarget.style.color = 'var(--pc-text-muted)'; e.currentTarget.style.borderColor = 'var(--pc-border)'; }}
                  >
                    {copiedId === msg.id ? (
                      <Check className="h-3 w-3" style={{ color: '#34d399' }} />
                    ) : (
                      <Copy className="h-3 w-3" />
                    )}
                  </button>
                </div>
              </div>
            </div>
          );
        })}

        {typing && (
          <div className="flex items-start gap-3 animate-fade-in">
            <div className="flex-shrink-0 w-9 h-9 rounded-2xl flex items-center justify-center border" style={{ background: 'var(--pc-bg-elevated)', borderColor: 'var(--pc-border)' }}>
              <Bot className="h-4 w-4" style={{ color: 'var(--pc-accent)' }} />
            </div>
            {streamingContent || streamingThinking ? (
              <div className="rounded-2xl px-4 py-3 border max-w-[75%]" style={{ background: 'var(--pc-bg-elevated)', borderColor: 'var(--pc-border)', color: 'var(--pc-text-primary)' }}>
                {streamingThinking && (
                  <details className="mb-2" open={!streamingContent}>
                    <summary className="text-xs cursor-pointer select-none" style={{ color: 'var(--pc-text-muted)' }}>Thinking{!streamingContent && '...'}</summary>
                    <pre className="text-xs mt-1 whitespace-pre-wrap break-words leading-relaxed overflow-auto max-h-60 p-2 rounded-lg" style={{ color: 'var(--pc-text-muted)', background: 'var(--pc-bg-surface)' }}>{streamingThinking}</pre>
                  </details>
                )}
                {streamingContent && <p className="text-sm whitespace-pre-wrap break-words leading-relaxed">{streamingContent}</p>}
              </div>
            ) : (
              <div className="rounded-2xl px-4 py-3 border flex items-center gap-1.5" style={{ background: 'var(--pc-bg-elevated)', borderColor: 'var(--pc-border)' }}>
                <span className="bounce-dot w-1.5 h-1.5 rounded-full" style={{ background: 'var(--pc-accent)' }} />
                <span className="bounce-dot w-1.5 h-1.5 rounded-full" style={{ background: 'var(--pc-accent)' }} />
                <span className="bounce-dot w-1.5 h-1.5 rounded-full" style={{ background: 'var(--pc-accent)' }} />
              </div>
            )}
          </div>
        )}

        {pendingApproval && (
          <div className="rounded-2xl border p-4 max-w-3xl mx-auto animate-fade-in" style={{ background: 'var(--pc-bg-elevated)', borderColor: 'var(--pc-accent-dim)' }}>
            <div className="flex items-start justify-between gap-4">
              <div>
                <p className="text-xs uppercase tracking-[0.18em]" style={{ color: 'var(--pc-accent)' }}>
                  Approval Required
                </p>
                <p className="mt-1 text-sm font-semibold" style={{ color: 'var(--pc-text-primary)' }}>
                  {pendingApproval.toolName}
                </p>
                {pendingApproval.argumentsSummary && (
                  <p className="mt-2 text-sm leading-relaxed" style={{ color: 'var(--pc-text-muted)' }}>
                    {pendingApproval.argumentsSummary}
                  </p>
                )}
                {pendingApproval.expiresAt && (
                  <p className="mt-2 text-[11px]" style={{ color: 'var(--pc-text-faint)' }}>
                    Expires at {new Date(pendingApproval.expiresAt * 1000).toLocaleTimeString()}
                  </p>
                )}
              </div>
              <div className="flex flex-col gap-2 min-w-[150px]">
                {pendingApproval.choices.includes('yes') && (
                  <button
                    type="button"
                    onClick={() => handleApproval('yes')}
                    disabled={pendingApproval.submitting}
                    className="btn-electric rounded-xl px-3 py-2 text-sm"
                    style={{ color: 'white' }}
                  >
                    Allow Once
                  </button>
                )}
                {pendingApproval.choices.includes('always') && (
                  <button
                    type="button"
                    onClick={() => handleApproval('always')}
                    disabled={pendingApproval.submitting}
                    className="rounded-xl px-3 py-2 text-sm border"
                    style={{ borderColor: 'var(--pc-accent-dim)', color: 'var(--pc-text-primary)', background: 'var(--pc-bg)' }}
                  >
                    Always This Session
                  </button>
                )}
                {pendingApproval.choices.includes('no') && (
                  <button
                    type="button"
                    onClick={() => handleApproval('no')}
                    disabled={pendingApproval.submitting}
                    className="rounded-xl px-3 py-2 text-sm border"
                    style={{ borderColor: 'rgba(239, 68, 68, 0.3)', color: '#fca5a5', background: 'rgba(239, 68, 68, 0.08)' }}
                  >
                    Deny
                  </button>
                )}
              </div>
            </div>
          </div>
        )}

        <div ref={messagesEndRef} />
      </div>

      {/* Input area */}
      <div
        className={isLight ? 'px-8 pb-1 pt-2 md:px-12' : 'border-t p-4'}
        style={isLight ? undefined : { borderColor: 'var(--pc-border)', background: 'var(--pc-bg-surface)' }}
      >
        <div
          data-testid={isLight ? 'agent-light-composer-shell' : undefined}
          className={`mx-auto ${isLight ? 'max-w-5xl rounded-[14px] border px-[10px] pb-[8px] pt-[10px]' : 'max-w-4xl'}`}
          style={isLight ? {
            background: '#ffffff',
            borderColor: '#3f8cff',
            boxShadow: '0 0 0 1px rgba(224, 99, 255, 0.48), 0 0 0 2px rgba(63, 140, 255, 0.35), 0 14px 32px rgba(62, 83, 134, 0.06)',
          } : undefined}
        >
          <div className={isLight ? 'flex flex-col gap-2' : 'flex items-center gap-3'}>
            {isLight ? (
              <div className="relative">
                <textarea
                  ref={inputRef}
                  rows={1}
                  value={input}
                  onChange={handleTextareaChange}
                  onKeyDown={handleKeyDown}
                  placeholder={connected ? t('agent.light_type_message') : t('agent.connecting')}
                  disabled={!connected || !!pendingApproval}
                  className="w-full resize-none border-0 bg-transparent px-1 pr-10 text-[14px] outline-none disabled:opacity-40"
                  style={{
                    minHeight: '44px',
                    maxHeight: '200px',
                    paddingTop: '1px',
                    paddingBottom: '0',
                    color: 'var(--pc-text-primary)',
                    lineHeight: '1.55',
                  }}
                />
              </div>
            ) : (
              <textarea
                ref={inputRef}
                rows={1}
                value={input}
                onChange={handleTextareaChange}
                onKeyDown={handleKeyDown}
                placeholder={connected ? t('agent.type_message') : t('agent.connecting')}
                disabled={!connected || !!pendingApproval}
                className="input-electric flex-1 px-4 text-sm resize-none disabled:opacity-40"
                style={{ minHeight: '44px', maxHeight: '200px', paddingTop: '10px', paddingBottom: '10px' }}
              />
            )}

            {isLight ? (
              <div className="flex items-center justify-between gap-4 border-t pt-[9px]" style={{ borderColor: '#eceff4' }}>
                <div className="flex flex-wrap items-center gap-[14px] text-[12px]" style={{ color: '#4d5057' }}>
                  <div className="relative" ref={modelDropdownRef}>
                    <button
                      type="button"
                      onClick={() => setShowModelDropdown((v) => !v)}
                      disabled={
                        modelLoading
                        || typing
                        || (availableModels.length === 0 && currentModel === null)
                      }
                      className="flex max-w-[200px] items-center gap-1.5 rounded-lg border px-2.5 py-1 text-[12px] font-medium transition-colors disabled:opacity-50"
                      style={{
                        borderColor: '#dfe3ea',
                        color: '#4d5057',
                        background: '#fafbfc',
                      }}
                      title={modelLoading ? undefined : currentModel ?? t('agent.select_model')}
                    >
                      <span className="truncate">
                        {modelLoading
                          ? t('agent.model_switching')
                          : (currentModel ?? (availableModels.length === 0 ? t('agent.model_loading') : t('agent.select_model')))}
                      </span>
                      <ChevronDown className="h-3 w-3 shrink-0 opacity-75" aria-hidden />
                    </button>

                    {showModelDropdown && availableModels.length > 0 && (
                      <div
                        className="absolute left-0 z-50 mb-1 max-h-52 min-w-[200px] max-w-[min(280px,calc(100vw-120px))] overflow-y-auto rounded-xl border py-1 shadow-lg"
                        style={{
                          bottom: '100%',
                          background: '#ffffff',
                          borderColor: '#dfe3ea',
                          boxShadow: '0 -8px 24px rgba(62, 83, 134, 0.12)',
                        }}
                      >
                        {availableModels.map((model) => (
                          <button
                            key={model}
                            type="button"
                            onClick={() => void handleModelSwitch(model)}
                            className="w-full px-3 py-2 text-left text-[12px] transition-colors"
                            style={{
                              color: model === currentModel ? '#3790FA' : '#222222',
                              background: model === currentModel ? 'rgba(55, 144, 250, 0.08)' : 'transparent',
                            }}
                            onMouseEnter={(e) => {
                              if (model !== currentModel) {
                                e.currentTarget.style.background = '#f6f8fa';
                              }
                            }}
                            onMouseLeave={(e) => {
                              if (model !== currentModel) {
                                e.currentTarget.style.background = 'transparent';
                              }
                            }}
                          >
                            {model}
                          </button>
                        ))}
                      </div>
                    )}
                  </div>
                </div>
                <div className="flex items-center gap-[10px]">
                  <button
                    type="button"
                    className="flex h-[20px] w-[20px] items-center justify-center"
                    style={{ color: '#5c6067' }}
                    aria-label="Attach file"
                  >
                    <Paperclip className="h-[14px] w-[14px]" strokeWidth={1.9} />
                  </button>
                  <button
                    type="button"
                    className="flex h-[20px] w-[20px] items-center justify-center"
                    style={{ color: '#5c6067' }}
                    aria-label="Reference file"
                  >
                    <LightThemeAssetIcon
                      fileName="reference-files.svg"
                      alt="Reference file"
                      className="h-[14px] w-[14px] opacity-[0.84]"
                    />
                  </button>
                  <button
                    type="button"
                    onClick={input.trim() && connected && !pendingApproval ? handleSend : undefined}
                    disabled={!connected || !!pendingApproval}
                    className="flex h-[24px] w-[24px] items-center justify-center rounded-full transition-colors"
                    style={{
                      background: '#e7e8ec',
                      color: '#5f646d',
                    }}
                    aria-label={input.trim() ? 'Send message' : 'Voice input'}
                  >
                    <LightThemeAssetIcon
                      fileName="ukui-microphone-on-symbolic.svg"
                      alt={input.trim() ? 'Send message' : 'Voice input'}
                      className="h-[13px] w-[13px] opacity-[0.84]"
                    />
                  </button>
                </div>
              </div>
            ) : (
              <>
                <button
                  type='button'
                  onClick={handleSend}
                  disabled={!connected || !input.trim() || !!pendingApproval}
                  className="btn-electric flex-shrink-0 rounded-2xl flex items-center justify-center"
                  style={{ color: 'white', width: '40px', height: '40px' }}
                >
                  <Send className="h-5 w-5" />
                </button>
              </>
            )}
          </div>

          {!isLight && (
            <div className="flex items-center justify-center mt-2 gap-2">
              <span
                className="status-dot"
                style={connected
                  ? { background: 'var(--color-status-success)', boxShadow: '0 0 6px var(--color-status-success)' }
                  : { background: 'var(--color-status-error)', boxShadow: '0 0 6px var(--color-status-error)' }
                }
              />
              <span className="text-[10px]" style={{ color: 'var(--pc-text-faint)' }}>
                {connected ? t('agent.connected_status') : t('agent.disconnected_status')}
              </span>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
