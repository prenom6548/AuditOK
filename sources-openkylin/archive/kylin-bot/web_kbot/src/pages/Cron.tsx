import React, { useState, useEffect, useLayoutEffect, useRef, useMemo, useCallback } from 'react';
import {
  AlarmClock,
  Plus,
  X,
  MoreHorizontal,
  ChevronDown,
  ChevronUp,
  Check,
  ChevronLeft,
  ChevronRight,
  ChevronsLeft,
  ChevronsRight,
} from 'lucide-react';
import type { CronJob } from '@/types/api';
import {
  getCronJobs,
  addCronJob,
  deleteCronJob,
  patchCronJob,
  runCronJobNow,
  pauseCronJob,
  resumeCronJob,
} from '@/lib/api';
import { assetBasePath } from '@/lib/basePath';
import { t } from '@/lib/i18n';

function formatDate(iso: string | null): string {
  if (!iso) return '-';
  const d = new Date(iso);
  return `${d.getFullYear()}/${d.getMonth() + 1}/${d.getDate()} ${String(d.getHours()).padStart(2, '0')}:${String(d.getMinutes()).padStart(2, '0')}:${String(d.getSeconds()).padStart(2, '0')}`;
}

// ---------- Toggle Switch ----------
function Toggle({ checked, onChange, disabled }: { checked: boolean; onChange: () => void; disabled?: boolean }) {
  return (
    <button
      onClick={onChange}
      disabled={disabled}
      style={{
        position: 'relative',
        display: 'inline-flex',
        alignItems: 'center',
        width: 40,
        height: 22,
        borderRadius: 11,
        background: checked ? '#1890ff' : '#d9d9d9',
        border: 'none',
        cursor: disabled ? 'not-allowed' : 'pointer',
        transition: 'background 0.25s',
        flexShrink: 0,
        opacity: disabled ? 0.5 : 1,
        padding: 0,
      }}
    >
      <span style={{
        position: 'absolute',
        left: checked ? 20 : 2,
        width: 18,
        height: 18,
        borderRadius: '50%',
        background: '#fff',
        boxShadow: '0 1px 4px rgba(0,0,0,0.2)',
        transition: 'left 0.25s',
      }} />
    </button>
  );
}

// ---------- Status Badge ----------
function StatusBadge({ status }: { status: string | null }) {
  if (!status) return <span style={{ color: '#bbb', fontSize: 13 }}>-</span>;
  const s = status.toLowerCase();
  const ok = s === 'ok' || s === 'success';
  const color = ok ? '#00cc66' : '#ff4d4f';
  const bgColor = ok ? 'rgba(0, 204, 102, 0.1)' : 'rgba(255, 77, 79, 0.1)';
  const label = ok ? t('cron.status_success') : t('cron.status_failed');

  return (
    <span style={{
      display: 'inline-flex',
      alignItems: 'center',
      gap: 4,
      color,
      background: bgColor,
      padding: '3px 8px',
      borderRadius: 12,
      fontSize: 12,
      fontWeight: 500
    }}>
      <svg width="14" height="14" viewBox="0 0 14 14" fill="none" xmlns="http://www.w3.org/2000/svg" style={{ flexShrink: 0 }}>
        <circle cx="7" cy="7" r="7" fill={color} />
        {ok ? (
          <path d="M4.5 7.5L6 9L9.5 5" stroke="#fff" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" />
        ) : (
          <path d="M4.5 4.5L9.5 9.5M9.5 4.5L4.5 9.5" stroke="#fff" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" />
        )}
      </svg>
      {label}
    </span>
  );
}

// ---------- Row Dropdown Menu ----------
function RowMenu({ onEdit, onDelete, onRunNow }: {
  onEdit: () => void;
  onDelete: () => void;
  onRunNow: () => void;
}) {
  const [open, setOpen] = useState(false);
  const btnRef = useRef<HTMLButtonElement>(null);
  const menuRef = useRef<HTMLDivElement>(null);
  const posRef = useRef({ x: 0, y: 0 });
  const MENU_W = 180;
  const MENU_H = 128;

  useEffect(() => {
    if (!open) return;
    const handler = (e: MouseEvent) => {
      if (
        btnRef.current && !btnRef.current.contains(e.target as Node) &&
        menuRef.current && !menuRef.current.contains(e.target as Node)
      ) {
        setOpen(false);
      }
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [open]);

  const handleClick = (e: React.MouseEvent) => {
    const x = e.clientX;
    const y = e.clientY;
    const vw = window.innerWidth;
    const vh = window.innerHeight;

    let left = x;
    if (left + MENU_W > vw) {
      left = vw - MENU_W - 8;
    }

    let top = y + 8;
    if (top + MENU_H > vh) {
      top = y - MENU_H - 8;
    }

    posRef.current = { x: left, y: top };
    setOpen(v => !v);
  };

  const item = (label: string, action: () => void) => (
    <button
      onClick={() => { action(); setOpen(false); }}
      style={{
        display: 'flex', alignItems: 'center',
        width: 164, height: 36, padding: '0 12px', margin: '0 8px',
        background: 'none', border: 'none', cursor: 'pointer',
        fontSize: 13, color: 'rgba(0, 0, 0, 0.85)',
        textAlign: 'left', borderRadius: 6,
        transition: 'background 0.15s',
      }}
      onMouseEnter={e => (e.currentTarget.style.background = '#DCDCDC')}
      onMouseLeave={e => (e.currentTarget.style.background = 'none')}
    >
      {label}
    </button>
  );

  return (
    <>
      <button
        ref={btnRef}
        onClick={handleClick}
        style={{
          background: 'none', border: 'none', cursor: 'pointer',
          padding: '4px 6px', borderRadius: 6, color: '#999',
          display: 'inline-flex', alignItems: 'center',
        }}
      >
        <MoreHorizontal size={18} />
      </button>
      {open && (
        <div
          ref={menuRef}
          style={{
            position: 'fixed',
            left: posRef.current.x,
            top: posRef.current.y,
            zIndex: 100,
            background: '#fff', border: '1px solid #e8e8e8',
            borderRadius: 8, boxShadow: '0 4px 16px rgba(0,0,0,0.12)',
            width: MENU_W, height: MENU_H, padding: '10px 0',
            boxSizing: 'border-box',
          }}
        >
          {item(t('cron.run_now'), onRunNow)}
          {item(t('cron.edit'), onEdit)}
          {item(t('cron.delete'), onDelete)}
        </div>
      )}
    </>
  );
}

// ---------- Schedule helpers ----------
type CronScheduleInput = Parameters<typeof addCronJob>[0]['schedule'];

function getCronCurrentTimeHHMM(): string {
  const now = new Date();
  return `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}`;
}

function formatLocalYmd(d: Date): string {
  const y = d.getFullYear();
  const m = String(d.getMonth() + 1).padStart(2, '0');
  const day = String(d.getDate()).padStart(2, '0');
  return `${y}-${m}-${day}`;
}


/** True when expr matches the fixed-shape cron produced by the daily/custom time UI (`m h * * dow`). */
function isSimpleDailyCronExpr(expr: string): boolean {
  const parts = (expr ?? '').trim().split(/\s+/);
  if (parts.length !== 5) return false;
  const [min, hr, dom, mon, dow] = parts;
  if (min === undefined || hr === undefined || dom === undefined || mon === undefined || dow === undefined)
    return false;
  if (dom !== '*' || mon !== '*') return false;
  if (!/^\d{1,2}$/.test(min) || !/^\d{1,2}$/.test(hr)) return false;
  if (dow !== '*' && !/^[\d,\s]+$/.test(dow)) return false;
  return true;
}

function parseCronExprToUI(expr: string): { time: string; weekdays: number[] } {
  const parts = (expr ?? '').trim().split(/\s+/);
  if (parts.length >= 5) {
    const minutePart = parts[0] ?? '0';
    const hourPart = parts[1] ?? '0';
    const wdStr = parts[4] ?? '*';
    const time = `${hourPart.padStart(2, '0')}:${minutePart.padStart(2, '0')}`;
    const weekdays =
      wdStr === '*'
        ? [1, 2, 3, 4, 5, 6, 7]
        : wdStr
          .split(',')
          .map(n => {
            const d = parseInt(n, 10);
            return d === 0 ? 7 : d;
          })
          .filter(d => d >= 1 && d <= 7);
    return { time, weekdays };
  }
  return { time: getCronCurrentTimeHHMM(), weekdays: [1, 2, 3, 4, 5, 6, 7] };
}

/** True when expr was produced by the interval UI (minute or hour step within an hour range). */
function isIntervalCronExpr(expr: string): boolean {
  const parts = (expr ?? '').trim().split(/\s+/);
  if (parts.length !== 5) return false;
  const [min, hr] = parts;
  if (min?.startsWith('*/')) return true;
  if (min === '0' && hr?.includes('-')) return true;
  return false;
}

function parseIntervalCronExpr(
  expr: string,
): {
  value: string;
  unit: 'minute' | 'hour';
  weekdays: number[];
  windowStart: string;
  windowEnd: string;
} | null {
  const parts = (expr ?? '').trim().split(/\s+/);
  if (parts.length !== 5) return null;
  const [min, hr, dom, mon, dow] = parts;
  if (dom !== '*' || mon !== '*') return null;

  const weekdays =
    dow === '*'
      ? [1, 2, 3, 4, 5, 6, 7]
      : dow!
          .split(',')
          .map(n => {
            const d = parseInt(n, 10);
            return d === 0 ? 7 : d;
          })
          .filter(d => d >= 1 && d <= 7);

  if (min!.startsWith('*/')) {
    const val = min!.slice(2);
    const m = hr!.match(/^(\d{1,2})-(\d{1,2})$/);
    if (!m) return null;
    const sh = parseInt(m[1]!, 10);
    const eh = parseInt(m[2]!, 10);
    return {
      value: val,
      unit: 'minute',
      weekdays,
      windowStart: `${String(sh).padStart(2, '0')}:00`,
      windowEnd: `${String(eh).padStart(2, '0')}:59`,
    };
  }

  if (min === '0' && hr!.includes('-')) {
    const m = hr!.match(/^(\d{1,2})-(\d{1,2})(?:\/(\d{1,2}))?$/);
    if (!m) return null;
    const sh = parseInt(m[1]!, 10);
    const eh = parseInt(m[2]!, 10);
    const val = m[3] ?? '1';
    return {
      value: val,
      unit: 'hour',
      weekdays,
      windowStart: `${String(sh).padStart(2, '0')}:00`,
      windowEnd: `${String(eh).padStart(2, '0')}:59`,
    };
  }

  return null;
}

function hhmmToMinutes(hhmm: string): number | null {
  const m = hhmm.trim().match(/^(\d{1,2}):(\d{2})$/);
  if (!m) return null;
  const h = parseInt(m[1] ?? '', 10);
  const mi = parseInt(m[2] ?? '', 10);
  if (
    !Number.isFinite(h) ||
    !Number.isFinite(mi) ||
    h < 0 ||
    h > 23 ||
    mi < 0 ||
    mi > 59
  )
    return null;
  return h * 60 + mi;
}

/** Interval UI → API schedule. Uses `every` only for all days + full-day window; else cron (weekdays + hour span). */
function buildIntervalSchedule(
  val: number,
  unit: 'minute' | 'hour',
  weekdays: number[],
  winStart: string,
  winEnd: string,
): CronScheduleInput | null {
  const max = unit === 'hour' ? 23 : 59;
  if (!Number.isFinite(val) || val < 1 || val > max) return null;
  if (weekdays.length === 0) return null;
  const startMin = hhmmToMinutes(winStart);
  const endMin = hhmmToMinutes(winEnd);
  if (startMin === null || endMin === null) return null;
  if (endMin < startMin) return null;

  const fullDay = winStart === '00:00' && winEnd === '23:59';
  const fullWeek = weekdays.length === 7;
  if (fullDay && fullWeek) {
    const unitMs = unit === 'minute' ? 60000 : 3600000;
    return { kind: 'every', every_ms: val * unitMs };
  }

  const sh = Math.floor(startMin / 60);
  const eh = Math.floor(endMin / 60);
  if (eh < sh) return null;
  const dow = weekdays
    .map(d => (d === 7 ? 0 : d))
    .sort((a, b) => a - b)
    .join(',');

  if (unit === 'minute') {
    const expr = `*/${val} ${sh}-${eh} * * ${dow}`;
    return { kind: 'cron', expr };
  }
  const hourField = val === 1 ? `${sh}-${eh}` : `${sh}-${eh}/${val}`;
  const expr = `0 ${hourField} * * ${dow}`;
  return { kind: 'cron', expr };
}

// ---------- Weekday Chips ----------
function WeekdayChips({
  weekdays,
  onChange,
}: {
  weekdays: number[];
  onChange: (next: number[]) => void;
}) {
  const days = [1, 2, 3, 4, 5, 6, 7];
  const keys: Record<number, string> = {
    1: 'cron.weekday_1',
    2: 'cron.weekday_2',
    3: 'cron.weekday_3',
    4: 'cron.weekday_4',
    5: 'cron.weekday_5',
    6: 'cron.weekday_6',
    7: 'cron.weekday_7',
  };
  return (
    <div style={{ display: 'flex', flexWrap: 'wrap', marginTop: 8 }}>
      {days.map((day, idx) => {
        const selected = weekdays.includes(day);
        const dayKey = keys[day] ?? 'cron.weekday_1';
        const bgDefault = selected ? 'rgba(55, 144, 250, 0.1)' : '#e6e6e6';
        const bgHover = selected ? 'rgba(55, 144, 250, 0.15)' : '#dcdcdc';
        const bgActive = selected ? 'rgba(55, 144, 250, 0.2)' : '#b9b9b9';
        const txtColor = selected ? '#3790fa' : 'rgba(0, 0, 0, 0.85)';
        return (
          <React.Fragment key={day}>
            <button
              type="button"
              onClick={() => {
                const next = selected
                  ? weekdays.filter(d => d !== day)
                  : [...weekdays, day].sort((a, b) => a - b);
                onChange(next);
              }}
              onMouseEnter={e => (e.currentTarget.style.background = bgHover)}
              onMouseLeave={e => (e.currentTarget.style.background = bgDefault)}
              onMouseDown={e => (e.currentTarget.style.background = bgActive)}
              onMouseUp={e => (e.currentTarget.style.background = bgHover)}
              style={{
                boxSizing: 'border-box',
                display: 'inline-flex',
                alignItems: 'center',
                justifyContent: 'center',
                height: 36,
                padding: '0 8px',
                width: 'auto',
                borderRadius: 6,
                border: 'none',
                background: bgDefault,
                color: txtColor,
                fontSize: 14,
                fontWeight: 400,
                cursor: 'pointer',
                transition: 'background 0.15s',
              }}
            >
              {t(dayKey)}
            </button>
            {idx < 6 && <span style={{ width: '8px', flexShrink: 0, display: 'inline-block' }} aria-hidden />}
          </React.Fragment>
        );
      })}
    </div>
  );
}

// ---------- Execution-time schedule kind (custom dropdown per design spec) ----------
const SCHEDULE_KIND_TEXT = 'rgba(0, 0, 0, 0.85)';
const SCHEDULE_KIND_ROW_SELECTED_BG = 'rgba(0, 0, 0, 0.15)';

/** String list picker: same visuals as execution-time schedule kind dropdown. */
function CronModalListSelect({
  value,
  onChange,
  options,
  triggerStyle,
  containerStyle,
}: {
  value: string;
  onChange: (next: string) => void;
  options: ReadonlyArray<{ value: string; label: string }>;
  triggerStyle: React.CSSProperties;
  containerStyle?: React.CSSProperties;
}) {
  const [open, setOpen] = useState(false);
  const wrapRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!open) return;
    const handler = (e: MouseEvent) => {
      if (wrapRef.current && !wrapRef.current.contains(e.target as Node)) setOpen(false);
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [open]);

  const activeLabel = options.find(o => o.value === value)?.label ?? (value !== '' ? value : '');

  const triggerMerged: React.CSSProperties = {
    ...triggerStyle,
    display: 'flex',
    alignItems: 'center',
    gap: 8,
    width: '100%',
    justifyContent: 'space-between',
    textAlign: 'left',
    cursor: 'pointer',
    fontFamily: 'inherit',
  };

  const wrapOuter: React.CSSProperties = {
    position: 'relative',
    width: '100%',
    ...containerStyle,
  };

  return (
    <div ref={wrapRef} style={wrapOuter}>
      <button
        type="button"
        aria-expanded={open}
        aria-haspopup="listbox"
        onClick={() => setOpen(o => !o)}
        style={triggerMerged}
      >
        <span
          style={{
            overflow: 'hidden',
            textOverflow: 'ellipsis',
            whiteSpace: 'nowrap',
            flex: 1,
            minWidth: 0,
            color: SCHEDULE_KIND_TEXT,
          }}
        >
          {activeLabel}
        </span>
        <ChevronDown size={16} style={{ flexShrink: 0, color: SCHEDULE_KIND_TEXT }} aria-hidden />
      </button>

      {open && (
        <div
          role="listbox"
          style={{
            position: 'absolute',
            left: 0,
            right: 0,
            top: '100%',
            marginTop: 4,
            background: '#fff',
            borderRadius: 8,
            border: '1px solid rgba(0, 0, 0, 0.06)',
            boxShadow: '0 4px 16px rgba(0, 0, 0, 0.12)',
            zIndex: 400,
            padding: '6px 8px',
          }}
        >
          {options.map((opt, i) => {
            const selected = value === opt.value;
            return (
              <button
                key={`${opt.value}:${i}:${opt.label}`}
                role="option"
                type="button"
                aria-selected={selected}
                onClick={() => {
                  onChange(opt.value);
                  setOpen(false);
                }}
                style={{
                  width: '100%',
                  height: 36,
                  padding: '0 8px',
                  border: 'none',
                  cursor: 'pointer',
                  fontSize: 14,
                  fontWeight: 400,
                  fontFamily: 'inherit',
                  color: SCHEDULE_KIND_TEXT,
                  background: selected ? SCHEDULE_KIND_ROW_SELECTED_BG : 'transparent',
                  display: 'flex',
                  alignItems: 'center',
                  gap: 4,
                  boxSizing: 'border-box',
                  borderRadius: 6,
                  marginBottom: i < options.length - 1 ? 2 : 0,
                }}
              >
                <span
                  style={{
                    width: 20,
                    flexShrink: 0,
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                  }}
                >
                  {selected ? (
                    <Check
                      size={14}
                      strokeWidth={2.5}
                      color={SCHEDULE_KIND_TEXT}
                      aria-hidden
                    />
                  ) : null}
                </span>
                <span style={{ flex: 1, textAlign: 'left' }}>{opt.label}</span>
              </button>
            );
          })}
        </div>
      )}
    </div>
  );
}

function ScheduleKindSelect({
  value,
  onChange,
  triggerStyle,
}: {
  value: 'daily' | 'interval' | 'once' | 'custom';
  onChange: (next: 'daily' | 'interval' | 'once' | 'custom') => void;
  triggerStyle: React.CSSProperties;
}) {
  return (
    <CronModalListSelect
      value={value}
      onChange={next =>
        next === 'daily' || next === 'interval' || next === 'once' || next === 'custom'
          ? onChange(next)
          : undefined
      }
      triggerStyle={triggerStyle}
      options={[
        { value: 'daily', label: t('cron.schedule_daily_label') },
        { value: 'interval', label: t('cron.schedule_interval_label') },
        { value: 'once', label: t('cron.schedule_once_label') },
        { value: 'custom', label: t('cron.schedule_custom_label') },
      ]}
    />
  );
}

/** Minute / hour picker for interval mode — execution-time dropdown visuals */
function IntervalUnitSelect({
  value,
  onChange,
  triggerStyle,
}: {
  value: 'minute' | 'hour';
  onChange: (next: 'minute' | 'hour') => void;
  triggerStyle: React.CSSProperties;
}) {
  return (
    <CronModalListSelect
      value={value}
      containerStyle={{ flex: 1, minWidth: 0 }}
      onChange={next => (next === 'minute' || next === 'hour' ? onChange(next) : undefined)}
      triggerStyle={triggerStyle}
      options={[
        { value: 'minute', label: t('cron.interval_unit_minute') },
        { value: 'hour', label: t('cron.interval_unit_hour') },
      ]}
    />
  );
}

// ---------- Time Spinner (QTimeEdit style, HH:MM only) ----------
function TimeSpinner({
  value,
  onChange,
  style,
}: {
  value: string;
  onChange: (v: string) => void;
  style?: React.CSSProperties;
}) {
  const [focused, setFocused] = useState(false);
  const [segment, setSegment] = useState<'h' | 'm'>('h');
  const [pendingDigit, setPendingDigit] = useState<string | null>(null);
  const containerRef = useRef<HTMLDivElement>(null);

  const parts = value.split(':');
  const hVal = Math.min(23, Math.max(0, parseInt(parts[0] ?? '0', 10) || 0));
  const mVal = Math.min(59, Math.max(0, parseInt(parts[1] ?? '0', 10) || 0));
  const hh = String(hVal).padStart(2, '0');
  const mm = String(mVal).padStart(2, '0');

  const emit = (h: number, m: number) =>
    onChange(
      `${String(((h % 24) + 24) % 24).padStart(2, '0')}:${String(((m % 60) + 60) % 60).padStart(2, '0')}`,
    );

  const bump = (delta: number) => {
    if (segment === 'h') emit(hVal + delta, mVal);
    else emit(hVal, mVal + delta);
  };

  const handleDigit = (digit: string) => {
    const d = parseInt(digit, 10);
    if (segment === 'h') {
      if (pendingDigit === null) {
        emit(d, mVal);
        if (d >= 3) { setPendingDigit(null); setSegment('m'); }
        else setPendingDigit(digit);
      } else {
        const combined = parseInt(pendingDigit, 10) * 10 + d;
        emit(combined <= 23 ? combined : d, mVal);
        setPendingDigit(null);
        setSegment('m');
      }
    } else {
      if (pendingDigit === null) {
        emit(hVal, d);
        if (d >= 6) setPendingDigit(null);
        else setPendingDigit(digit);
      } else {
        const combined = parseInt(pendingDigit, 10) * 10 + d;
        emit(hVal, combined <= 59 ? combined : d);
        setPendingDigit(null);
      }
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    switch (e.key) {
      case 'ArrowUp': e.preventDefault(); bump(1); break;
      case 'ArrowDown': e.preventDefault(); bump(-1); break;
      case 'ArrowLeft': e.preventDefault(); setSegment('h'); setPendingDigit(null); break;
      case 'ArrowRight': e.preventDefault(); setSegment('m'); setPendingDigit(null); break;
      default:
        if (/^\d$/.test(e.key)) { e.preventDefault(); handleDigit(e.key); }
    }
  };

  const segStyle = (active: boolean): React.CSSProperties => ({
    padding: '0 3px',
    borderRadius: 2,
    background: focused && active ? '#3790FA' : 'transparent',
    color: focused && active ? '#fff' : 'rgba(0,0,0,0.85)',
    fontSize: 13,
    minWidth: 22,
    textAlign: 'center' as const,
    display: 'inline-block',
    lineHeight: '22px',
    cursor: 'default',
  });

  return (
    <div
      ref={containerRef}
      tabIndex={0}
      onFocus={() => setFocused(true)}
      onBlur={e => {
        if (!containerRef.current?.contains(e.relatedTarget as Node)) {
          setFocused(false);
          setPendingDigit(null);
        }
      }}
      onKeyDown={handleKeyDown}
      style={{
        ...style,
        display: 'flex',
        alignItems: 'center',
        paddingLeft: 12,
        paddingRight: 36,
        position: 'relative',
        cursor: 'default',
        outline: 'none',
        border: focused ? '1px solid #3790FA' : '1px solid transparent',
        borderRadius: 6,
        background: '#E6E6E6',
        height: 36,
        boxSizing: 'border-box',
        userSelect: 'none',
      }}
    >
      <span style={segStyle(segment === 'h')} onClick={() => { setSegment('h'); containerRef.current?.focus(); }}>
        {hh}
      </span>
      <span style={{ fontSize: 13, color: 'rgba(0,0,0,0.85)', padding: '0 1px' }}>:</span>
      <span style={segStyle(segment === 'm')} onClick={() => { setSegment('m'); containerRef.current?.focus(); }}>
        {mm}
      </span>
      <div
        style={{
          position: 'absolute',
          right: 8,
          top: 0,
          bottom: 0,
          width: 16,
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
        }}
      >
        <button
          type="button"
          tabIndex={-1}
          onClick={e => { e.preventDefault(); bump(1); containerRef.current?.focus(); }}
          aria-label={t('cron.daily_time_step_up')}
          style={{ padding: 0, margin: 0, border: 'none', background: 'transparent', cursor: 'pointer', height: 18, width: 16, display: 'flex', alignItems: 'center', justifyContent: 'center', flexShrink: 0, color: SCHEDULE_KIND_TEXT }}
        >
          <ChevronUp size={16} strokeWidth={2} aria-hidden />
        </button>
        <button
          type="button"
          tabIndex={-1}
          onClick={e => { e.preventDefault(); bump(-1); containerRef.current?.focus(); }}
          aria-label={t('cron.daily_time_step_down')}
          style={{ padding: 0, margin: 0, border: 'none', background: 'transparent', cursor: 'pointer', height: 18, width: 16, display: 'flex', alignItems: 'center', justifyContent: 'center', flexShrink: 0, color: SCHEDULE_KIND_TEXT }}
        >
          <ChevronDown size={16} strokeWidth={2} aria-hidden />
        </button>
      </div>
    </div>
  );
}

/** Single-run date field: `YYYY/MM/DD` display, click opens month grid (Mon-first). Value remains `YYYY-MM-DD`. */
function OnceDatePicker({
  value,
  onChange,
  style,
}: {
  value: string;
  onChange: (v: string) => void;
  style?: React.CSSProperties;
}) {
  const mt = /^(\d{4})-(\d{2})-(\d{2})$/.exec(value.trim());
  const selY = mt ? parseInt(mt[1]!, 10) : new Date().getFullYear();
  const selMo = mt ? parseInt(mt[2]!, 10) : new Date().getMonth() + 1;
  const selD = mt ? parseInt(mt[3]!, 10) : new Date().getDate();

  const wrapRef = useRef<HTMLDivElement>(null);
  const [open, setOpen] = useState(false);
  const [popoverAnchor, setPopoverAnchor] = useState<{ right: number; bottom: number } | null>(
    null,
  );
  const [viewY, setViewY] = useState(selY);
  const [viewM, setViewM] = useState(selMo);

  const display = `${String(selY).padStart(4, '0')}/${String(selMo).padStart(2, '0')}/${String(selD).padStart(2, '0')}`;

  useEffect(() => {
    if (open) {
      setViewY(selY);
      setViewM(selMo);
    }
  }, [open, selY, selMo]);

  const updatePopoverAnchor = useCallback(() => {
    const r = wrapRef.current?.getBoundingClientRect();
    if (!r) return;
    const gap = 4;
    setPopoverAnchor({
      right: window.innerWidth - r.right,
      bottom: window.innerHeight - r.top + gap,
    });
  }, []);

  useLayoutEffect(() => {
    if (!open) {
      setPopoverAnchor(null);
      return;
    }
    updatePopoverAnchor();
  }, [open, updatePopoverAnchor]);

  useEffect(() => {
    if (!open) return;
    window.addEventListener('resize', updatePopoverAnchor);
    window.addEventListener('scroll', updatePopoverAnchor, true);
    return () => {
      window.removeEventListener('resize', updatePopoverAnchor);
      window.removeEventListener('scroll', updatePopoverAnchor, true);
    };
  }, [open, updatePopoverAnchor]);

  useEffect(() => {
    if (!open) return;
    const onDoc = (e: MouseEvent) => {
      if (wrapRef.current?.contains(e.target as Node)) return;
      setOpen(false);
    };
    document.addEventListener('mousedown', onDoc);
    return () => document.removeEventListener('mousedown', onDoc);
  }, [open]);

  const cells = useMemo(() => {
    const first = new Date(viewY, viewM - 1, 1);
    const startPad = (first.getDay() + 6) % 7;
    const daysInMonth = new Date(viewY, viewM, 0).getDate();
    const out: { date: Date; inMonth: boolean }[] = [];
    for (let i = startPad; i > 0; i--) {
      out.push({ date: new Date(viewY, viewM - 1, 1 - i), inMonth: false });
    }
    for (let d = 1; d <= daysInMonth; d++) {
      out.push({ date: new Date(viewY, viewM - 1, d), inMonth: true });
    }
    let cur = new Date(viewY, viewM - 1, daysInMonth);
    while (out.length % 7 !== 0) {
      cur.setDate(cur.getDate() + 1);
      out.push({ date: new Date(cur), inMonth: false });
    }
    while (out.length < 42) {
      cur.setDate(cur.getDate() + 1);
      out.push({ date: new Date(cur), inMonth: false });
    }
    return out;
  }, [viewY, viewM]);

  const shiftMonth = (delta: number) => {
    const d = new Date(viewY, viewM - 1 + delta, 1);
    setViewY(d.getFullYear());
    setViewM(d.getMonth() + 1);
  };

  const shiftYear = (delta: number) => {
    setViewY(y => y + delta);
  };

  const headerCenter = `${selY}/${selMo}/${selD}`;

  const isSameDay = (a: Date, y: number, m: number, day: number) =>
    a.getFullYear() === y && a.getMonth() + 1 === m && a.getDate() === day;

  const navBtn: React.CSSProperties = {
    padding: 4,
    border: 'none',
    background: 'transparent',
    cursor: 'pointer',
    borderRadius: 4,
    color: SCHEDULE_KIND_TEXT,
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
  };

  return (
    <div ref={wrapRef} style={{ ...style, position: 'relative', minWidth: 0 }}>
      <button
        type="button"
        onClick={() => setOpen(o => !o)}
        aria-label={t('cron.once_open_calendar')}
        aria-expanded={open}
        style={{
          width: '100%',
          height: 36,
          boxSizing: 'border-box',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          paddingLeft: 12,
          paddingRight: 10,
          border: open ? '1px solid #3790FA' : '1px solid transparent',
          borderRadius: 6,
          background: '#E6E6E6',
          cursor: 'pointer',
          fontSize: 13,
          color: 'rgba(0,0,0,0.85)',
          outline: 'none',
          textAlign: 'left',
          fontFamily: 'inherit',
        }}
      >
        <span>{display}</span>
        <img
          src={`${assetBasePath}/icons/ukui-datetime-symbolic.svg`}
          width={18}
          height={18}
          alt=""
          aria-hidden
          style={{ flexShrink: 0, display: 'block', opacity: 0.45, objectFit: 'contain' }}
        />
      </button>

      {open && popoverAnchor && (
        <div
          style={{
            position: 'fixed',
            right: popoverAnchor.right,
            bottom: popoverAnchor.bottom,
            width: 'min(300px, calc(100vw - 32px))',
            zIndex: 10000,
            background: '#fff',
            borderRadius: 12,
            boxShadow: '0 8px 28px rgba(0,0,0,0.14)',
            padding: '12px 12px 14px',
            boxSizing: 'border-box',
          }}
        >
          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
              marginBottom: 10,
              gap: 4,
            }}
          >
            <button type="button" style={navBtn} aria-label={t('cron.cal_prev_year')} onClick={() => shiftYear(-1)}>
              <ChevronsLeft size={18} strokeWidth={2} />
            </button>
            <button type="button" style={navBtn} aria-label={t('cron.cal_prev_month')} onClick={() => shiftMonth(-1)}>
              <ChevronLeft size={18} strokeWidth={2} />
            </button>
            <span style={{ fontSize: 14, color: 'rgba(0,0,0,0.85)', fontWeight: 500, flex: 1, textAlign: 'center' }}>
              {headerCenter}
            </span>
            <button type="button" style={navBtn} aria-label={t('cron.cal_next_month')} onClick={() => shiftMonth(1)}>
              <ChevronRight size={18} strokeWidth={2} />
            </button>
            <button type="button" style={navBtn} aria-label={t('cron.cal_next_year')} onClick={() => shiftYear(1)}>
              <ChevronsRight size={18} strokeWidth={2} />
            </button>
          </div>

          <div
            style={{
              display: 'grid',
              gridTemplateColumns: 'repeat(7, 1fr)',
              gap: 2,
              textAlign: 'center',
              fontSize: 12,
              color: 'rgba(0,0,0,0.45)',
              marginBottom: 6,
            }}
          >
            {([1, 2, 3, 4, 5, 6, 7] as const).map(d => (
              <span key={d}>{t(`cron.cal_head_${d}` as 'cron.cal_head_1')}</span>
            ))}
          </div>

          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(7, 1fr)', gap: 4 }}>
            {cells.map(({ date, inMonth }, idx) => {
              const d = date.getDate();
              const selected = isSameDay(date, selY, selMo, selD);
              return (
                <button
                  key={`${idx}-${date.getTime()}`}
                  type="button"
                  onClick={() => {
                    onChange(formatLocalYmd(date));
                    setOpen(false);
                  }}
                  style={{
                    height: 32,
                    border: 'none',
                    borderRadius: 6,
                    cursor: 'pointer',
                    fontSize: 13,
                    fontWeight: selected ? 600 : 400,
                    background: selected ? '#3790FA' : 'transparent',
                    color: selected ? '#fff' : inMonth ? 'rgba(0,0,0,0.85)' : 'rgba(0,0,0,0.28)',
                  }}
                >
                  {d}
                </button>
              );
            })}
          </div>
        </div>
      )}
    </div>
  );
}

// ---------- Add/Edit Modal ----------
function CronModal({
  job,
  onClose,
  onSaved,
}: {
  job: CronJob | 'add';
  onClose: () => void;
  onSaved: (updated: CronJob) => void;
}) {
  const isEdit = job !== 'add';
  const existing = isEdit ? (job as CronJob) : null;

  // Derive initial values from existing job
  const initKind = (): 'daily' | 'interval' | 'once' | 'custom' => {
    if (!existing) return 'daily';
    if (existing.schedule.kind === 'cron') {
      const ex = existing.schedule.expr ?? '';
      if (isIntervalCronExpr(ex)) return 'interval';
      const hasTz = Boolean(existing.schedule.tz && String(existing.schedule.tz).trim());
      if (!isSimpleDailyCronExpr(ex) || hasTz) return 'custom';
      const parts = ex.trim().split(/\s+/);
      const dow = parts[4] ?? '*';
      return dow === '*' ? 'daily' : 'custom';
    }
    if (existing.schedule.kind === 'every') return 'interval';
    return 'once';
  };

  // Daily: time only (`m h * * *`). Custom: time + weekdays (same as pre-change「每天」).
  const cronParsed =
    existing?.schedule.kind === 'cron' && existing.schedule.expr
      ? parseCronExprToUI(existing.schedule.expr)
      : null;

  const intervalParsed =
    existing?.schedule.kind === 'cron' && existing.schedule.expr && isIntervalCronExpr(existing.schedule.expr)
      ? parseIntervalCronExpr(existing.schedule.expr)
      : null;

  const initIntervalValue = (): string => {
    if (existing?.schedule.kind === 'every' && existing.schedule.every_ms) {
      const ms = existing.schedule.every_ms;
      if (ms % 3600000 === 0) return String(ms / 3600000);
      if (ms % 60000 === 0) return String(ms / 60000);
      return String(Math.max(1, Math.round(ms / 60000)));
    }
    if (intervalParsed) return intervalParsed.value;
    return '5';
  };

  const initIntervalUnit = (): 'minute' | 'hour' => {
    if (existing?.schedule.kind === 'every' && existing.schedule.every_ms) {
      const ms = existing.schedule.every_ms;
      if (ms % 3600000 === 0) return 'hour';
      return 'minute';
    }
    if (intervalParsed) return intervalParsed.unit;
    return 'minute';
  };

  // Form state
  const [formName, setFormName] = useState(existing?.name ?? '');
  const [formPrompt, setFormPrompt] = useState(existing?.prompt ?? existing?.command ?? '');
  const [formScheduleKind, setFormScheduleKind] = useState<
    'daily' | 'interval' | 'once' | 'custom'
  >(initKind());

  const [formDailyTime, setFormDailyTime] = useState(
    cronParsed?.time ?? getCronCurrentTimeHHMM(),
  );
  const [formCustomTime, setFormCustomTime] = useState(
    cronParsed?.time ?? getCronCurrentTimeHHMM(),
  );
  const [formCustomWeekdays, setFormCustomWeekdays] = useState<number[]>(
    cronParsed?.weekdays ?? [1, 2, 3, 4, 5, 6, 7],
  );
  // Interval
  const [formIntervalValue, setFormIntervalValue] = useState(initIntervalValue());
  const [formIntervalUnit, setFormIntervalUnit] = useState<'minute' | 'hour'>(initIntervalUnit());
  const [formIntervalWeekdays, setFormIntervalWeekdays] = useState<number[]>(
    intervalParsed?.weekdays ?? [1, 2, 3, 4, 5, 6, 7],
  );
  const [formIntervalWindowStart, setFormIntervalWindowStart] = useState(
    intervalParsed?.windowStart ?? '00:00',
  );
  const [formIntervalWindowEnd, setFormIntervalWindowEnd] = useState(
    intervalParsed?.windowEnd ?? '23:59',
  );
  const initOnceTime = (): string => {
    if (existing?.schedule.kind === 'at' && existing.schedule.at) {
      const d = new Date(existing.schedule.at);
      return `${String(d.getHours()).padStart(2, '0')}:${String(d.getMinutes()).padStart(2, '0')}`;
    }
    return getCronCurrentTimeHHMM();
  };

  const initOnceDate = (): string => {
    if (existing?.schedule.kind === 'at' && existing.schedule.at) {
      return formatLocalYmd(new Date(existing.schedule.at));
    }
    return formatLocalYmd(new Date());
  };

  // Once — separate time + date (defaults: current local time / date)
  const [formOnceTime, setFormOnceTime] = useState(initOnceTime());
  const [formOnceDate, setFormOnceDate] = useState(initOnceDate());
  const [formError, setFormError] = useState<string | null>(null);
  const [submitting, setSubmitting] = useState(false);

  // Dirty detection: capture initial snapshot for edit mode
  const initialSnapshot = useRef({
    name: existing?.name ?? '',
    prompt: existing?.prompt ?? existing?.command ?? '',
    scheduleKind: initKind(),
    dailyTime: cronParsed?.time ?? getCronCurrentTimeHHMM(),
    customTime: cronParsed?.time ?? getCronCurrentTimeHHMM(),
    customWeekdays: [...(cronParsed?.weekdays ?? [1, 2, 3, 4, 5, 6, 7])],
    intervalValue: initIntervalValue(),
    intervalUnit: initIntervalUnit(),
    intervalWeekdays: intervalParsed?.weekdays ?? [1, 2, 3, 4, 5, 6, 7],
    intervalWindowStart: intervalParsed?.windowStart ?? '00:00',
    intervalWindowEnd: intervalParsed?.windowEnd ?? '23:59',
    onceTime: initOnceTime(),
    onceDate: initOnceDate(),
  });

  const canSubmit = useMemo(() => {
    if (!isEdit) {
      // Add mode: enabled only when name and prompt are both non-empty
      return formName.trim().length > 0 && formPrompt.trim().length > 0;
    }
    const s = initialSnapshot.current;
    return (
      formName !== s.name ||
      formPrompt !== s.prompt ||
      formScheduleKind !== s.scheduleKind ||
      formDailyTime !== s.dailyTime ||
      formCustomTime !== s.customTime ||
      formCustomWeekdays.length !== s.customWeekdays.length ||
      formCustomWeekdays.some((v, i) => v !== s.customWeekdays[i]) ||
      formIntervalValue !== s.intervalValue ||
      formIntervalUnit !== s.intervalUnit ||
      formIntervalWeekdays.length !== s.intervalWeekdays.length ||
      formIntervalWeekdays.some((v, i) => v !== s.intervalWeekdays[i]) ||
      formIntervalWindowStart !== s.intervalWindowStart ||
      formIntervalWindowEnd !== s.intervalWindowEnd ||
      formOnceTime !== s.onceTime ||
      formOnceDate !== s.onceDate
    );
  }, [
    isEdit,
    formName,
    formPrompt,
    formScheduleKind,
    formDailyTime,
    formCustomTime,
    formCustomWeekdays,
    formIntervalValue,
    formIntervalUnit,
    formIntervalWeekdays,
    formIntervalWindowStart,
    formIntervalWindowEnd,
    formOnceTime,
    formOnceDate,
  ]);


  const handleSubmit = async () => {
    if (!formPrompt.trim()) {
      setFormError(t('cron.validation_error'));
      return;
    }
    let schedule: CronScheduleInput;
    if (formScheduleKind === 'daily') {
      const [hStr = '0', mStr = '0'] = formDailyTime.split(':');
      const h = parseInt(hStr, 10);
      const m = parseInt(mStr, 10);
      schedule = { kind: 'cron', expr: `${m} ${h} * * *` };
    } else if (formScheduleKind === 'custom') {
      const [hStr = '0', mStr = '0'] = formCustomTime.split(':');
      const h = parseInt(hStr, 10);
      const m = parseInt(mStr, 10);
      if (formCustomWeekdays.length === 0) {
        setFormError(t('cron.validation_error'));
        return;
      }
      const days =
        formCustomWeekdays.length === 7
          ? '*'
          : formCustomWeekdays.map(d => (d === 7 ? 0 : d)).join(',');
      schedule = { kind: 'cron', expr: `${m} ${h} * * ${days}` };
    } else if (formScheduleKind === 'interval') {
      const val = parseInt(formIntervalValue, 10);
      const built = buildIntervalSchedule(
        val,
        formIntervalUnit,
        formIntervalWeekdays,
        formIntervalWindowStart,
        formIntervalWindowEnd,
      );
      if (!built) {
        setFormError(t('cron.validation_error'));
        return;
      }
      schedule = built;
    } else {
      const ymd = formOnceDate.trim();
      const hhmm = formOnceTime.trim();
      if (!/^\d{4}-\d{2}-\d{2}$/.test(ymd) || !/^\d{2}:\d{2}$/.test(hhmm)) {
        setFormError(t('cron.validation_error'));
        return;
      }
      const atLocal = new Date(`${ymd}T${hhmm}:00`);
      if (Number.isNaN(atLocal.getTime())) {
        setFormError(t('cron.validation_error'));
        return;
      }
      schedule = { kind: 'at', at: atLocal.toISOString() };
    }

    setSubmitting(true);
    setFormError(null);
    try {
      const payload = {
        name: formName.trim() || undefined,
        schedule,
        prompt: formPrompt.trim(),
        job_type: 'agent' as const,
        session_target: 'main' as const,
        enabled: true,
      };
      let result: CronJob;
      if (isEdit) {
        result = await patchCronJob((existing as CronJob).id, payload);
      } else {
        result = await addCronJob(payload);
      }
      onSaved(result);
    } catch (err) {
      setFormError(
        err instanceof Error ? err.message : t(isEdit ? 'cron.edit_error' : 'cron.add_error'),
      );
    } finally {
      setSubmitting(false);
    }
  };

  // Shared styles
  const fieldLabel: React.CSSProperties = {
    display: 'block',
    fontSize: 14,
    color: 'rgba(0,0,0,0.85)',
    marginBottom: 6,
    fontWeight: 400,
  };
  const inputBase: React.CSSProperties = {
    width: '100%',
    boxSizing: 'border-box',
    height: 36,
    padding: '0 12px',
    border: 'none',
    borderRadius: 6,
    fontSize: 13,
    color: 'rgba(0,0,0,0.85)',
    background: '#E6E6E6',
    outline: 'none',
  };
  const selStyle: React.CSSProperties = {
    ...inputBase,
    padding: '0 8px 0 12px',
    cursor: 'pointer',
    appearance: 'none',
  };

  const intervalValueMax = formIntervalUnit === 'hour' ? 23 : 59;
  const bumpIntervalValue = (delta: number) => {
    const raw = parseInt(formIntervalValue, 10);
    const base = Number.isFinite(raw) && raw >= 1 ? raw : 1;
    setFormIntervalValue(
      String(Math.min(intervalValueMax, Math.max(1, base + delta))),
    );
  };

  return (
    <div
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        width: '100vw',
        height: '100vh',
        background: 'transparent',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 200,
      }}
    >
      <style>{`
        .cron-add-task-modal input::-webkit-input-placeholder,
        .cron-add-task-modal textarea::-webkit-input-placeholder {
          color: rgba(0, 0, 0, 0.35) !important;
          -webkit-text-fill-color: rgba(0, 0, 0, 0.35) !important;
          opacity: 1 !important;
        }
        .cron-add-task-modal input::-moz-placeholder,
        .cron-add-task-modal textarea::-moz-placeholder {
          color: rgba(0, 0, 0, 0.35) !important;
          opacity: 1 !important;
        }
        .cron-add-task-modal input:-ms-input-placeholder,
        .cron-add-task-modal textarea:-ms-input-placeholder {
          color: rgba(0, 0, 0, 0.35) !important;
        }
        .cron-add-task-modal input::placeholder,
        .cron-add-task-modal textarea::placeholder {
          color: rgba(0, 0, 0, 0.35) !important;
          opacity: 1 !important;
        }
        .cron-add-task-modal input.cron-interval-number-input::-webkit-outer-spin-button,
        .cron-add-task-modal input.cron-interval-number-input::-webkit-inner-spin-button {
          -webkit-appearance: none;
          margin: 0;
        }
        .cron-add-task-modal input.cron-interval-number-input {
          appearance: none;
          -moz-appearance: textfield;
        }
      `}</style>
      <div
        className="cron-add-task-modal"
        style={{
          background: '#fff',
          borderRadius: 14,
          width: 424,
          maxHeight: 520,
          display: 'flex',
          flexDirection: 'column',
          boxShadow: '0 8px 32px rgba(0,0,0,0.18)',
          transform: 'translate(-28px, -40px)',
        }}
      >
        {/* Header */}
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            padding: '16px 20px 12px 8px',
          }}
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
            <img
              src={`${assetBasePath}/kylin-bot.svg`}
              width={24}
              height={24}
              style={{ borderRadius: 6, flexShrink: 0 }}
              alt=""
            />
            <span style={{ fontSize: 14, fontWeight: 600, color: 'rgba(0,0,0,0.85)' }}>
              {isEdit ? t('cron.edit_modal_title') : t('cron.add_modal_title')}
            </span>
          </div>
          <button
            onClick={onClose}
            style={{
              background: 'none',
              border: 'none',
              cursor: 'pointer',
              color: 'rgba(0, 0, 0, 0.85)',
              padding: 4,
              display: 'flex',
              alignItems: 'center',
            }}
          >
            <X size={18} />
          </button>
        </div>

        {/* Body */}
        <div style={{ padding: '0 24px 4px', overflowY: 'auto', flex: 1 }}>
          {formError && (
            <div
              style={{
                background: '#fff2f0',
                border: '1px solid #ffa39e',
                borderRadius: 6,
                padding: '8px 12px',
                marginBottom: 12,
                fontSize: 13,
                color: '#cf1322',
              }}
            >
              {formError}
            </div>
          )}
          <div style={{ display: 'flex', flexDirection: 'column', gap: 16, paddingBottom: 8 }}>
            {/* 任务名称 */}
            <div>
              <label style={fieldLabel}>{t('cron.task_name')}</label>
              <input
                type="text"
                value={formName}
                onChange={e => setFormName(e.target.value)}
                placeholder={t('cron.task_name_placeholder')}
                maxLength={32}
                style={inputBase}
              />
            </div>

            {/* 任务指令 */}
            <div>
              <label style={fieldLabel}>{t('cron.task_prompt_label')}</label>
              <textarea
                value={formPrompt}
                onChange={e => setFormPrompt(e.target.value)}
                placeholder={t('cron.task_prompt_placeholder')}
                maxLength={1000}
                style={{
                  ...inputBase,
                  height: 120,
                  padding: '8px 12px',
                  resize: 'none',
                  lineHeight: '1.5',
                }}
              />
            </div>

            {/* 执行时间 */}
            <div>
              <label style={fieldLabel}>{t('cron.exec_time')}</label>
              <ScheduleKindSelect value={formScheduleKind} onChange={setFormScheduleKind} triggerStyle={selStyle} />

              {/* Daily: time of day, every calendar day */}
              {formScheduleKind === 'daily' && (
                <div style={{ marginTop: 8 }}>
                  <TimeSpinner value={formDailyTime} onChange={setFormDailyTime} style={{ width: '100%' }} />
                </div>
              )}

              {/* Custom: time + weekdays (legacy「每天」controls) */}
              {formScheduleKind === 'custom' && (
                <div style={{ marginTop: 8 }}>
                  <TimeSpinner value={formCustomTime} onChange={setFormCustomTime} style={{ width: '100%' }} />
                  <WeekdayChips weekdays={formCustomWeekdays} onChange={setFormCustomWeekdays} />
                </div>
              )}

              {/* Interval: 每间隔 + stepped number + unit dropdown + weekdays */}
              {formScheduleKind === 'interval' && (
                <div style={{ marginTop: 8, display: 'flex', flexDirection: 'column' }}>
                  <div
                    style={{
                      display: 'flex',
                      alignItems: 'center',
                      width: '100%',
                      flexWrap: 'nowrap',
                    }}
                  >
                    <span
                      style={{
                        fontSize: 14,
                        color: 'rgba(0,0,0,0.85)',
                        whiteSpace: 'nowrap',
                        flexShrink: 0,
                      }}
                    >
                      {t('cron.interval_row_label')}
                    </span>
                    <span style={{ width: '8px', flexShrink: 0, display: 'inline-block' }} aria-hidden />
                    <div style={{ flex: 1, minWidth: 0, position: 'relative' }}>
                      <input
                        type="number"
                        inputMode="numeric"
                        min={1}
                        max={intervalValueMax}
                        className="cron-interval-number-input"
                        value={formIntervalValue}
                        onChange={e => {
                          const v = e.target.value;
                          if (v === '') {
                            setFormIntervalValue('');
                            return;
                          }
                          const n = parseInt(v, 10);
                          if (!Number.isFinite(n)) return;
                          setFormIntervalValue(
                            String(Math.min(intervalValueMax, Math.max(1, n))),
                          );
                        }}
                        style={{
                          ...inputBase,
                          width: '100%',
                          paddingLeft: 12,
                          paddingRight: 36,
                          textAlign: 'left',
                        }}
                      />
                      <div
                        style={{
                          position: 'absolute',
                          right: 8,
                          top: 0,
                          bottom: 0,
                          width: 16,
                          display: 'flex',
                          flexDirection: 'column',
                          alignItems: 'center',
                          justifyContent: 'center',
                          gap: 0,
                        }}
                      >
                        <button
                          type="button"
                          onClick={e => {
                            e.preventDefault();
                            bumpIntervalValue(1);
                          }}
                          aria-label={t('cron.interval_step_up')}
                          style={{
                            padding: 0,
                            margin: 0,
                            border: 'none',
                            background: 'transparent',
                            cursor: 'pointer',
                            height: 18,
                            width: 16,
                            display: 'flex',
                            alignItems: 'center',
                            justifyContent: 'center',
                            flexShrink: 0,
                            color: SCHEDULE_KIND_TEXT,
                          }}
                        >
                          <ChevronUp size={16} strokeWidth={2} aria-hidden />
                        </button>
                        <button
                          type="button"
                          onClick={e => {
                            e.preventDefault();
                            bumpIntervalValue(-1);
                          }}
                          aria-label={t('cron.interval_step_down')}
                          style={{
                            padding: 0,
                            margin: 0,
                            border: 'none',
                            background: 'transparent',
                            cursor: 'pointer',
                            height: 18,
                            width: 16,
                            display: 'flex',
                            alignItems: 'center',
                            justifyContent: 'center',
                            flexShrink: 0,
                            color: SCHEDULE_KIND_TEXT,
                          }}
                        >
                          <ChevronDown size={16} strokeWidth={2} aria-hidden />
                        </button>
                      </div>
                    </div>
                    <span style={{ width: '8px', flexShrink: 0, display: 'inline-block' }} aria-hidden />
                    <IntervalUnitSelect
                      value={formIntervalUnit}
                      onChange={next => {
                        setFormIntervalUnit(next);
                        const max = next === 'hour' ? 23 : 59;
                        setFormIntervalValue(prev => {
                          if (prev === '') return '';
                          const n = parseInt(prev, 10);
                          if (!Number.isFinite(n))
                            return String(Math.min(max, Math.max(1, 5)));
                          return String(Math.min(max, Math.max(1, n)));
                        });
                      }}
                      triggerStyle={selStyle}
                    />
                  </div>
                  <div style={{ marginBottom: '8px' }}>
                    <WeekdayChips
                      weekdays={formIntervalWeekdays}
                      onChange={setFormIntervalWeekdays}
                    />
                  </div>
                  <div
                    style={{
                      marginTop: 8,
                      display: 'flex',
                      alignItems: 'center',
                      width: '100%',
                    }}
                  >
                    <span
                      style={{
                        fontSize: 14,
                        color: 'rgba(0,0,0,0.85)',
                        whiteSpace: 'nowrap',
                        flexShrink: 0,
                        fontWeight: 400,
                      }}
                    >
                      {t('cron.interval_effective_time')}
                    </span>
                    <span style={{ width: '8px', flexShrink: 0, display: 'inline-block' }} aria-hidden />
                    <TimeSpinner
                      value={formIntervalWindowStart}
                      onChange={setFormIntervalWindowStart}
                      style={{ flex: 1, minWidth: 0 }}
                    />
                    <span style={{ width: '8px', flexShrink: 0, display: 'inline-block' }} aria-hidden />
                    <TimeSpinner
                      value={formIntervalWindowEnd}
                      onChange={setFormIntervalWindowEnd}
                      style={{ flex: 1, minWidth: 0 }}
                    />
                  </div>
                </div>
              )}

               {/* Once: time + date row (defaults: current local time / date) */}
               {formScheduleKind === 'once' && (
                 <div
                   style={{
                     marginTop: 8,
                     display: 'flex',
                     alignItems: 'stretch',
                     width: '100%',
                   }}
                 >
                   <TimeSpinner value={formOnceTime} onChange={setFormOnceTime} style={{ flex: 1, minWidth: 0 }} />
                   <span style={{ width: '8px', flexShrink: 0, display: 'inline-block' }} aria-hidden />
                   <OnceDatePicker value={formOnceDate} onChange={setFormOnceDate} style={{ flex: 1, minWidth: 0 }} />
                 </div>
               )}

            </div>

          </div>
        </div>

        {/* Footer */}
        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            alignItems: 'center',
            gap: 8,
            padding: '16px 24px 24px',
          }}
        >
          <button
            type="button"
            onClick={onClose}
            style={{
              boxSizing: 'border-box',
              height: 36,
              padding: '0 16px',
              borderRadius: 6,
              border: 'none',
              background: 'rgba(0, 0, 0, 0.1)',
              color: 'rgba(0, 0, 0, 0.85)',
              fontSize: 14,
              fontWeight: 400,
              cursor: 'pointer',
              display: 'inline-flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            {t('cron.cancel')}
          </button>
          <button
            type="button"
            onClick={handleSubmit}
            disabled={submitting || !canSubmit}
            style={{
              boxSizing: 'border-box',
              height: 36,
              padding: '0 16px',
              borderRadius: 6,
              border: 'none',
              background: submitting || !canSubmit
                ? 'rgba(55, 144, 250, 0.45)'
                : '#3790FA',
              color: submitting || !canSubmit
                ? 'rgba(255, 255, 255, 0.75)'
                : '#FFFFFF',
              fontSize: 14,
              fontWeight: 400,
              cursor: submitting || !canSubmit ? 'not-allowed' : 'pointer',
              display: 'inline-flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            {submitting
              ? t(isEdit ? 'cron.saving' : 'cron.adding')
              : t(isEdit ? 'cron.confirm' : 'cron.modal_add')}
          </button>
        </div>
      </div>
    </div>
  );
}

// ---------- Confirm Delete Dialog ----------
function ConfirmDeleteModal({ onConfirm, onCancel }: { onConfirm: () => void; onCancel: () => void }) {
  const cancelDefault = 'rgba(0, 0, 0, 0.1)';
  const cancelHover = 'rgba(0, 0, 0, 0.15)';
  const cancelActive = 'rgba(0, 0, 0, 0.2)';
  const deleteDefault = '#3790FA';
  const deleteHover = '#2a7ee0';
  const deleteActive = '#1e6bc7';

  return (
    <div
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        width: '100vw',
        height: '100vh',
        background: 'transparent',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 300,
      }}
    >
      <div style={{ background: '#fff', borderRadius: 10, width: 376, height: 192, boxShadow: '0 8px 32px rgba(0,0,0,0.18)', display: 'flex', flexDirection: 'column', padding: '48px 24px 24px', position: 'relative', transform: 'translate(-28px, -40px)' }}>
        {/* Close button */}
        <button
          onClick={onCancel}
          style={{
            position: 'absolute',
            top: 12,
            right: 12,
            background: 'none',
            border: 'none',
            cursor: 'pointer',
            color: 'rgba(0, 0, 0, 0.45)',
            padding: 4,
            display: 'flex',
            alignItems: 'center',
          }}
        >
          <X size={16} />
        </button>
        {/* Title row: icon + text */}
        <div style={{ display: 'flex', alignItems: 'center', gap: 8, marginBottom: 8 }}>
          <img
            src={`${assetBasePath}/icons/dialog-question.svg`}
            alt=""
            width={16}
            height={16}
            style={{ flexShrink: 0, display: 'block', objectFit: 'contain' }}
          />
          <span style={{ fontSize: 14, fontWeight: 500, color: 'rgba(0, 0, 0, 0.85)' }}>
            {t('cron.confirm_delete')}
          </span>
        </div>
        {/* Description */}
        <p style={{ margin: 0, marginLeft: 24, fontSize: 14, color: 'rgba(0, 0, 0, 0.6)', lineHeight: 1.5, flex: 1 }}>
          {t('cron.delete_desc')}
        </p>
        {/* Buttons */}
        <div style={{ display: 'flex', justifyContent: 'flex-end', gap: 10, flexShrink: 0 }}>
          <button
            onClick={onCancel}
            onMouseEnter={e => (e.currentTarget.style.background = cancelHover)}
            onMouseLeave={e => (e.currentTarget.style.background = cancelDefault)}
            onMouseDown={e => (e.currentTarget.style.background = cancelActive)}
            onMouseUp={e => (e.currentTarget.style.background = cancelHover)}
            style={{
              width: 88,
              height: 36,
              borderRadius: 6,
              border: 'none',
              background: cancelDefault,
              color: 'rgba(0, 0, 0, 0.85)',
              fontSize: 14,
              fontWeight: 400,
              cursor: 'pointer',
              display: 'inline-flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            {t('cron.no')}
          </button>
          <button
            onClick={onConfirm}
            onMouseEnter={e => (e.currentTarget.style.background = deleteHover)}
            onMouseLeave={e => (e.currentTarget.style.background = deleteDefault)}
            onMouseDown={e => (e.currentTarget.style.background = deleteActive)}
            onMouseUp={e => (e.currentTarget.style.background = deleteHover)}
            style={{
              width: 88,
              height: 36,
              borderRadius: 6,
              border: 'none',
              background: deleteDefault,
              color: '#FFFFFF',
              fontSize: 14,
              fontWeight: 400,
              cursor: 'pointer',
              display: 'inline-flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            {t('cron.yes')}
          </button>
        </div>
      </div>
    </div>
  );
}

// ---------- Main Page ----------
export default function Cron() {
  const [jobs, setJobs] = useState<CronJob[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [modalJob, setModalJob] = useState<CronJob | 'add' | null>(null);
  const [confirmDeleteId, setConfirmDeleteId] = useState<string | null>(null);
  const [togglingId, setTogglingId] = useState<string | null>(null);
  const [runningId, setRunningId] = useState<string | null>(null);

  const fetchJobs = () => {
    setLoading(true);
    getCronJobs()
      .then(setJobs)
      .catch(err => setError(err.message))
      .finally(() => setLoading(false));
  };

  useEffect(() => { fetchJobs(); }, []);

  const handleToggle = async (job: CronJob) => {
    setTogglingId(job.id);
    try {
      const updated = job.enabled ? await pauseCronJob(job.id) : await resumeCronJob(job.id);
      setJobs(prev => prev.map(j => j.id === updated.id ? updated : j));
    } catch (err) {
      setError(err instanceof Error ? err.message : t('cron.toggle_error'));
    } finally {
      setTogglingId(null);
    }
  };

  const handleDelete = async (id: string) => {
    try {
      await deleteCronJob(id);
      setJobs(prev => prev.filter(j => j.id !== id));
    } catch (err) {
      setError(err instanceof Error ? err.message : t('cron.delete_error'));
    } finally {
      setConfirmDeleteId(null);
    }
  };

  const handleRunNow = async (job: CronJob) => {
    setRunningId(job.id);
    try {
      await runCronJobNow(job.id);
      fetchJobs();
    } catch (err) {
      setError(err instanceof Error ? err.message : t('cron.run_error'));
    } finally {
      setRunningId(null);
    }
  };

  const handleSaved = (updated: CronJob) => {
    setJobs(prev => {
      const idx = prev.findIndex(j => j.id === updated.id);
      if (idx >= 0) {
        const next = [...prev];
        next[idx] = updated;
        return next;
      }
      return [...prev, updated];
    });
    setModalJob(null);
  };

  // Shared page wrapper style
  const page: React.CSSProperties = {
    padding: '16px 20px',
    fontFamily: 'var(--pc-font-ui)',
    color: '#333',
    height: '100%',
    boxSizing: 'border-box',
    display: 'flex',
    flexDirection: 'column',
    gap: 8,
  };

  const thStyle: React.CSSProperties = {
    padding: '0 14px',
    height: 36,
    boxSizing: 'border-box',
    textAlign: 'left',
    fontSize: 13,
    fontWeight: 500,
    color: 'rgba(0, 0, 0, 0.6)',
    whiteSpace: 'nowrap',
    background: '#f5f5f5',
    borderBottom: '2px solid #fff',
  };

  const tdStyle: React.CSSProperties = {
    padding: '0 14px',
    height: 60,
    boxSizing: 'border-box',
    fontSize: 13,
    color: '#333',
    verticalAlign: 'middle',
    background: '#f5f5f5',
    borderBottom: '2px solid #fff',
  };

  if (error) {
    return (
      <div style={page}>
        <div style={{ background: '#fff2f0', border: '1px solid #ffa39e', borderRadius: 8, padding: '12px 16px', color: '#cf1322', fontSize: 13 }}>
          {t('cron.load_error')}: {error}
        </div>
      </div>
    );
  }

  if (loading) {
    return (
      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', height: 200 }}>
        <div style={{ width: 28, height: 28, border: '2px solid #e0e0e0', borderTopColor: '#1890ff', borderRadius: '50%', animation: 'spin 0.8s linear infinite' }} />
        <style>{`@keyframes spin { to { transform: rotate(360deg); } }`}</style>
      </div>
    );
  }

  const addBtnDefaultBg = '#F6F6F6';
  const addBtnHoverBg = '#F2F2F2';
  const addBtnActiveBg = '#EEEEEE';
  const addBtnColor = 'rgba(0, 0, 0, 0.85)';

  return (
    <div style={page}>
      {/* Header */}
      <div style={{ padding: '0 14px' }}>
        <span style={{ fontSize: 14, color: 'rgba(0, 0, 0, 0.85)', fontWeight: 500 }}>
          {t('cron.task_list')}（{jobs.length}）
        </span>
      </div>

      {/* Content */}
      {jobs.length === 0 ? (
        /* Empty state */
        <div
          style={{
            border: '1px solid #f0f0f0', borderRadius: 8,
            background: addBtnDefaultBg,
          }}
          onMouseEnter={e => (e.currentTarget.style.background = addBtnHoverBg)}
          onMouseLeave={e => (e.currentTarget.style.background = addBtnDefaultBg)}
          onMouseDown={e => (e.currentTarget.style.background = addBtnActiveBg)}
          onMouseUp={e => (e.currentTarget.style.background = addBtnHoverBg)}
        >
          <button
            onClick={() => setModalJob('add')}
            style={{
              display: 'flex', alignItems: 'center', justifyContent: 'center',
              gap: 6, width: '100%', padding: '16px',
              background: 'transparent', border: 'none', cursor: 'pointer',
              fontSize: 14, color: addBtnColor,
            }}
          >
            <Plus size={16} />{t('cron.add_job')}
          </button>
        </div>
      ) : (
        /* Table */
        <div style={{ borderRadius: 8, background: '#fff', overflow: 'auto', flex: 1, paddingRight: 8 }}>
          <table style={{ width: '100%', borderCollapse: 'separate', borderSpacing: 0 }}>
            <thead>
              <tr>
                <th style={{ ...thStyle, borderRadius: '8px 0 0 0' }}>{t('cron.name')}</th>
                <th style={{ ...thStyle, width: '38%' }}>{t('cron.prompt_col')}</th>
                <th style={{ ...thStyle, whiteSpace: 'nowrap' }}>{t('cron.next_run')}</th>
                <th style={{ ...thStyle, whiteSpace: 'nowrap' }}>{t('cron.last_status')}</th>
                <th style={{ ...thStyle, whiteSpace: 'nowrap', borderRadius: '0 8px 0 0' }}>{t('cron.actions')}</th>
              </tr>
            </thead>
            <tbody>
              {jobs.map(job => (
                <tr key={job.id}>
                  {/* Name */}
                  <td style={tdStyle}>
                    <span style={{ display: 'inline-flex', alignItems: 'center', gap: 6, color: 'rgba(0, 0, 0, 0.85)', fontWeight: 400 }}>
                      <AlarmClock size={14} style={{ color: '#262626', flexShrink: 0 }} />
                      {job.name ?? job.id.slice(0, 8)}
                    </span>
                  </td>
                  {/* Prompt / command */}
                  <td style={{ ...tdStyle, color: 'rgba(0, 0, 0, 0.6)', maxWidth: 0 }}>
                    <span style={{ display: 'block', overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
                      {job.prompt ?? job.command}
                    </span>
                  </td>
                  {/* Next run */}
                  <td style={{ ...tdStyle, color: 'rgba(0, 0, 0, 0.6)', whiteSpace: 'nowrap' }}>
                    {formatDate(job.next_run)}
                  </td>
                  {/* Last status */}
                  <td style={tdStyle}>
                    <StatusBadge status={job.last_status} />
                  </td>
                  {/* Actions */}
                  <td style={tdStyle}>
                    <div style={{ display: 'inline-flex', alignItems: 'center', gap: 8 }}>
                      <RowMenu
                        onEdit={() => setModalJob(job)}
                        onDelete={() => setConfirmDeleteId(job.id)}
                        onRunNow={() => handleRunNow(job)}
                      />
                      <Toggle
                        checked={job.enabled}
                        onChange={() => handleToggle(job)}
                        disabled={togglingId === job.id || runningId === job.id}
                      />
                    </div>
                  </td>
                </tr>
              ))}
              {/* Add task row */}
              <tr>
                <td
                  colSpan={5}
                  onClick={() => setModalJob('add')}
                  onMouseEnter={e => (e.currentTarget.style.background = addBtnHoverBg)}
                  onMouseLeave={e => (e.currentTarget.style.background = addBtnDefaultBg)}
                  onMouseDown={e => (e.currentTarget.style.background = addBtnActiveBg)}
                  onMouseUp={e => (e.currentTarget.style.background = addBtnHoverBg)}
                  style={{
                    ...tdStyle,
                    textAlign: 'center',
                    borderBottom: 'none',
                    borderRadius: '0 0 6px 6px',
                    cursor: 'pointer',
                    background: addBtnDefaultBg,
                    color: addBtnColor,
                  }}
                >
                  <span style={{
                    display: 'inline-flex',
                    alignItems: 'center',
                    gap: 6,
                    fontSize: 14,
                    color: addBtnColor,
                  }}>
                    <Plus size={14} />{t('cron.add_job')}
                  </span>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      )}

      {/* Add / Edit Modal */}
      {modalJob !== null && (
        <CronModal
          job={modalJob}
          onClose={() => setModalJob(null)}
          onSaved={handleSaved}
        />
      )}

      {/* Confirm Delete */}
      {confirmDeleteId !== null && (
        <ConfirmDeleteModal
          onConfirm={() => handleDelete(confirmDeleteId)}
          onCancel={() => setConfirmDeleteId(null)}
        />
      )}
    </div>
  );
}
