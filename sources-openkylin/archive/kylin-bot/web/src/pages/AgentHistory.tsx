import { useMemo } from 'react';
import { useNavigate } from 'react-router-dom';
import { Clock3, MessageSquare } from 'lucide-react';
import { listChatSessions } from '@/lib/chatHistoryStorage';
import { setSessionId } from '@/lib/ws';

function formatUpdatedAt(iso: string): string {
  const date = new Date(iso);
  if (Number.isNaN(date.getTime())) return '';
  return date.toLocaleString('zh-CN', {
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    hour12: false,
  });
}

export default function AgentHistory() {
  const navigate = useNavigate();
  const sessions = useMemo(() => listChatSessions(), []);

  const openSession = (sessionId: string) => {
    setSessionId(sessionId);
    navigate('/agent');
  };

  return (
    <div className="h-[calc(100vh-3.5rem)] overflow-y-auto px-6 py-5 md:px-10">
      <div className="mx-auto w-full max-w-4xl">
        <div className="mb-5">
          <h2 className="text-lg font-semibold" style={{ color: 'var(--pc-text-primary)' }}>
            历史对话
          </h2>
          <p className="mt-1 text-sm" style={{ color: 'var(--pc-text-muted)' }}>
            选择一个会话以恢复聊天上下文
          </p>
        </div>

        {sessions.length === 0 ? (
          <div
            className="rounded-2xl border px-5 py-8 text-sm"
            style={{
              background: 'var(--pc-bg-elevated)',
              borderColor: 'var(--pc-border)',
              color: 'var(--pc-text-muted)',
            }}
          >
            还没有可用的历史对话
          </div>
        ) : (
          <div className="space-y-2">
            {sessions.map((session) => (
              <button
                key={session.sessionId}
                type="button"
                onClick={() => openSession(session.sessionId)}
                className="w-full rounded-xl px-4 py-3 text-left transition-colors"
                style={{
                  background: '#F6F6F6',
                }}
                onMouseEnter={(e) => {
                  e.currentTarget.style.background = '#EBEBEB';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.background = '#F6F6F6';
                }}
              >
                <div className="flex items-start justify-between gap-3">
                  <div className="min-w-0">
                    <p className="truncate text-sm font-semibold" style={{ color: 'var(--pc-text-primary)' }}>
                      {session.title}
                    </p>
                    <p className="mt-1 truncate text-xs" style={{ color: 'var(--pc-text-muted)' }}>
                      {session.preview || '（无文本内容）'}
                    </p>
                  </div>
                  <div className="flex shrink-0 items-center gap-3 text-xs" style={{ color: 'var(--pc-text-faint)' }}>
                    <span className="inline-flex items-center gap-1">
                      <MessageSquare className="h-3.5 w-3.5" />
                      {session.messageCount}
                    </span>
                    <span className="inline-flex items-center gap-1">
                      <Clock3 className="h-3.5 w-3.5" />
                      {formatUpdatedAt(session.updatedAt)}
                    </span>
                  </div>
                </div>
              </button>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
