import { useState, useRef, useEffect } from 'react';
import { Loader2, X } from 'lucide-react';
import { t } from '@/lib/i18n';
import { useSkills } from '@/hooks/useApi';
import { toggleSkill as toggleSkillApi, createSkill, deleteSkill, importSkill, openSkillFolder } from '@/lib/api';
import { uninstallSkill } from '@/lib/skillhubApi';
import { assetBasePath } from '@/lib/basePath';
import type { Skill } from '@/types/api';

function ImportMenu({
  onImportResult,
  onAddClick,
}: {
  onImportResult: (success: boolean) => void;
  onAddClick: () => void;
}) {
  const [menuOpen, setMenuOpen] = useState(false);
  const [dropUp, setDropUp] = useState(false);
  const [overwriteName, setOverwriteName] = useState<string | null>(null);
  const [overwriteFile, setOverwriteFile] = useState<File | null>(null);
  const [overwriting, setOverwriting] = useState(false);
  const fileInputRef = useRef<HTMLInputElement>(null);
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!menuOpen) return;
    const handler = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) setMenuOpen(false);
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [menuOpen]);

  const handleToggleMenu = () => {
    setMenuOpen((v) => {
      if (!v && ref.current) {
        const rect = ref.current.getBoundingClientRect();
        setDropUp(rect.bottom + 170 > window.innerHeight);
      }
      return !v;
    });
  };

  const handleUploadFile = () => {
    fileInputRef.current?.click();
  };

  const handleFileChange = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    e.target.value = '';
    setMenuOpen(false);
    await doImport(file);
  };

  const doImport = async (file: File) => {
    try {
      await importSkill(file);
      onImportResult(true);
    } catch (err: unknown) {
      const e = err as Error & { duplicate?: boolean; skillName?: string };
      if (e.duplicate && e.skillName) {
        setOverwriteName(e.skillName);
        setOverwriteFile(file);
        return;
      }
      onImportResult(false);
    }
  };

  const handleOverwrite = async () => {
    if (!overwriteFile || !overwriteName) return;
    setOverwriting(true);
    try {
      await deleteSkill(overwriteName);
      await doImport(overwriteFile);
      setOverwriteName(null);
      setOverwriteFile(null);
    } catch {
      onImportResult(false);
    } finally {
      setOverwriting(false);
    }
  };

  return (
    <div ref={ref} style={{ position: 'relative', display: 'inline-block' }}>
      <button
        className="skill-toolbar-btn"
        onClick={handleToggleMenu}
      >
        <span style={{ fontSize: 18, lineHeight: 1 }}>+</span>
        <span>添加</span>
      </button>
      {menuOpen && (
        <div
          onMouseDown={(e) => e.stopPropagation()}
          style={{
            position: 'absolute',
            right: 0,
            ...(dropUp ? { bottom: '100%' } : { top: '100%' }),
            zIndex: 100,
            background: '#fff',
            border: '1px solid #e8e8e8',
            borderRadius: 8,
            boxShadow: '0 4px 16px rgba(0,0,0,0.12)',
            width: 180,
            padding: 8,
            boxSizing: 'border-box',
            display: 'flex',
            flexDirection: 'column',
            gap: 2,
          }}
        >
          <button
            className="skill-uninstall-btn"
            style={{ width: 164, height: 36 }}
            onClick={() => { setMenuOpen(false); onAddClick(); }}
          >
            自定义
          </button>
          <button
            className="skill-uninstall-btn"
            style={{ width: 164, height: 36 }}
            onClick={handleUploadFile}
          >
            上传文件
          </button>
        </div>
      )}
      <input ref={fileInputRef} type="file" accept=".md,.zip" style={{ display: 'none' }} onChange={handleFileChange} />
      <OverwriteConfirmDialog
        open={!!overwriteName}
        onCancel={() => { setOverwriteName(null); setOverwriteFile(null); }}
        onConfirm={handleOverwrite}
        overwriting={overwriting}
        skillName={overwriteName || ''}
      />
    </div>
  );
}

function AddSkillDialog({
  open,
  onCancel,
  onCreated,
  existingSlugs,
}: {
  open: boolean;
  onCancel: () => void;
  onCreated: () => void;
  existingSlugs: string[];
}) {
  const [name, setName] = useState('');
  const [description, setDescription] = useState('');
  const [capability, setCapability] = useState('');
  const [creating, setCreating] = useState(false);
  const [nameOverflow, setNameOverflow] = useState(false);
  const [descOverflow, setDescOverflow] = useState(false);
  const [nameInvalidChar, setNameInvalidChar] = useState(false);
  const [overwriteName, setOverwriteName] = useState<string | null>(null);
  const [overwriting, setOverwriting] = useState(false);
  const nameRef = useRef<HTMLInputElement>(null);

  const byteLen = (s: string) => new TextEncoder().encode(s).length;
  const INVALID_CHARS = ['\\', '/', ':', '*', '?', '"', '<', '>', '|'];
  const hasInvalidChar = (s: string) => INVALID_CHARS.some((c) => s.includes(c));
  const findMaxLen = (s: string, maxBytes: number): number => {
    let total = 0;
    for (let i = 0; i < s.length; i++) {
      total += new TextEncoder().encode(s[i]).length;
      if (total > maxBytes) return i;
    }
    return s.length;
  };

  useEffect(() => {
    if (open) {
      setName('');
      setDescription('');
      setCapability('');
      setCreating(false);
      setNameOverflow(false);
      setDescOverflow(false);
      setNameInvalidChar(false);
      setOverwriteName(null);
      setOverwriting(false);
      requestAnimationFrame(() => nameRef.current?.focus());
    }
  }, [open]);

  const handleCreate = async () => {
    const trimmed = name.trim();
    if (!trimmed) return;
    if (existingSlugs.some((s) => s.toLowerCase() === trimmed.toLowerCase())) {
      setOverwriteName(trimmed);
      return;
    }
    await doCreate(trimmed);
  };

  const doCreate = async (skillName: string) => {
    setCreating(true);
    try {
      const body: { description?: string; prompts?: string[] } = {};
      if (description.trim()) body.description = description.trim();
      if (capability.trim()) {
        body.prompts = [capability.trim()];
      }
      await createSkill(skillName, body);
      onCreated();
    } catch (err) {
      console.error('Failed to create skill:', err);
    } finally {
      setCreating(false);
    }
  };

  const handleOverwrite = async () => {
    if (!overwriteName) return;
    setOverwriting(true);
    try {
      await deleteSkill(overwriteName);
      await doCreate(overwriteName);
      setOverwriteName(null);
    } catch (err) {
      console.error('Failed to overwrite skill:', err);
    } finally {
      setOverwriting(false);
    }
  };

  const inputStyle: React.CSSProperties = {
    width: '100%',
    height: 36,
    padding: '0 10px',
    fontSize: 14,
    color: 'var(--pc-text-primary)',
    border: '1px solid var(--pc-border)',
    borderRadius: 4,
    outline: 'none',
    boxSizing: 'border-box',
  };

  const labelStyle: React.CSSProperties = {
    display: 'block',
    fontSize: 14,
    color: '#000',
    marginBottom: 4,
  };

  const errorStyle: React.CSSProperties = {
    fontSize: 12,
    color: '#F53F3F',
    minHeight: 18,
    lineHeight: '18px',
  };

  const canSubmit = name.trim() && description.trim();

  if (!open && !overwriteName) return null;

  return (
    <>
      <div
        style={{
          position: 'fixed',
          top: 0,
          left: 0,
          width: '100vw',
          height: '100vh',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          zIndex: 200,
          transform: 'translate(-28px, -40px)',
        }}
      >
        <div
          style={{
            width: 424,
            height: 520,
            background: '#fff',
            borderRadius: 8,
            boxSizing: 'border-box',
            display: 'flex',
            flexDirection: 'column',
            overflow: 'hidden',
            boxShadow: '0 6px 30px rgba(0,0,0,0.12)',
          }}
        >
          <div
            style={{
              height: 40,
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
              paddingLeft: 12,
              paddingRight: 8,
              flexShrink: 0,
            }}
          >
            <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
              <img
                src={`${assetBasePath}/kylin-bot.svg`}
                alt=""
                width={20}
                height={20}
                style={{ objectFit: 'contain', flexShrink: 0 }}
                onError={(e) => { (e.target as HTMLImageElement).style.display = 'none'; }}
              />
              <span style={{ fontSize: 14, fontWeight: 500, color: 'var(--pc-text-primary)' }}>
                添加技能
              </span>
            </div>
            <CloseTooltipButton onClick={onCancel} hoverEffect />
          </div>

          <div style={{ paddingLeft: 24, paddingRight: 24, paddingTop: 8, paddingBottom: 24, display: 'flex', flexDirection: 'column', flex: 1, overflowY: 'auto' }}>
            <div>
              <label style={labelStyle}>技能名称</label>
              <input
                ref={nameRef}
                value={name}
                onChange={(e) => {
                  const v = e.target.value;
                  if (hasInvalidChar(v)) {
                    setNameInvalidChar(true);
                    return;
                  }
                  setNameInvalidChar(false);
                  if (byteLen(v) > 255) {
                    setNameOverflow(true);
                    setName(v.slice(0, findMaxLen(v, 255)));
                  } else {
                    setNameOverflow(false);
                    setName(v);
                  }
                }}
                placeholder="必填"
                className="placeholder-[#A6A6A6]"
                style={inputStyle}
              />
              <div style={errorStyle}>
                {nameInvalidChar ? '文件名中不能包含以下字符：\\/:*?"<>|' : nameOverflow ? '长度不得大于255字符' : '\u00A0'}
              </div>
            </div>

            <div>
              <label style={labelStyle}>技能简介</label>
              <input
                value={description}
                onChange={(e) => {
                  const v = e.target.value;
                  if (byteLen(v) > 255) {
                    setDescOverflow(true);
                    setDescription(v.slice(0, findMaxLen(v, 255)));
                  } else {
                    setDescOverflow(false);
                    setDescription(v);
                  }
                }}
                placeholder="必填"
                className="placeholder-[#A6A6A6]"
                style={inputStyle}
              />
              <div style={errorStyle}>
                {descOverflow ? '长度不得大于255字符' : '\u00A0'}
              </div>
            </div>

            <div>
              <label style={labelStyle}>技能能力</label>
              <textarea
                value={capability}
                onChange={(e) => setCapability(e.target.value)}
                placeholder="选填。例如：文件分类、重复文件识别、自动归档、通知提醒"
                className="placeholder-[#A6A6A6]"
                rows={9}
                style={{
                  width: '100%',
                  padding: '8px 10px',
                  fontSize: 14,
                  color: 'var(--pc-text-primary)',
                  border: '1px solid var(--pc-border)',
                  borderRadius: 4,
                  outline: 'none',
                  boxSizing: 'border-box',
                  resize: 'none',
                  lineHeight: '20px',
                }}
              />
            </div>

            <div style={{ display: 'flex', justifyContent: 'flex-end', gap: 8, marginTop: 'auto' }}>
              <button className="skill-dialog-cancel-btn" onClick={onCancel} style={{ width: 64, height: 36 }}>
                取消
              </button>
              <button
                className="skill-dialog-uninstall-btn"
                onClick={handleCreate}
                disabled={creating || !canSubmit}
                style={{ opacity: !canSubmit ? 0.5 : undefined, width: 64, height: 36, cursor: !canSubmit ? 'default' : 'pointer' }}
              >
                {creating ? <Loader2 className="h-4 w-4 animate-spin" style={{ color: '#fff' }} /> : '添加'}
              </button>
            </div>
          </div>
        </div>
      </div>
      <OverwriteConfirmDialog
        open={!!overwriteName}
        onCancel={() => setOverwriteName(null)}
        onConfirm={handleOverwrite}
        overwriting={overwriting}
        skillName={overwriteName || ''}
      />
    </>
  );
}

function CloseTooltipButton({ onClick, hoverEffect }: { onClick: () => void; hoverEffect?: boolean }) {
  const [tooltipVisible, setTooltipVisible] = useState(false);
  const [tooltipPos, setTooltipPos] = useState({ x: 0, y: 0 });
  const tooltipTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  return (
    <div style={{ position: 'relative', display: 'inline-flex' }}>
      <button
        className="skill-dialog-close-btn"
        style={{ cursor: 'default' }}
        onClick={onClick}
        onMouseEnter={(e) => {
          setTooltipPos({ x: e.clientX, y: e.clientY });
          tooltipTimerRef.current = setTimeout(() => setTooltipVisible(true), 500);
          if (hoverEffect) {
            e.currentTarget.style.background = '#F53F3F';
            e.currentTarget.style.color = '#ffffff';
          }
        }}
        onMouseLeave={(e) => {
          if (tooltipTimerRef.current) clearTimeout(tooltipTimerRef.current);
          tooltipTimerRef.current = null;
          setTooltipVisible(false);
          if (hoverEffect) {
            e.currentTarget.style.background = 'transparent';
            e.currentTarget.style.color = '';
          }
        }}
      >
        <X size={16} />
      </button>
      {tooltipVisible && (
        <div
          style={{
            position: 'fixed',
            left: tooltipPos.x + 8,
            top: tooltipPos.y + 8,
            zIndex: 300,
            background: '#fff',
            border: '1px solid #e8e8e8',
            borderRadius: 8,
            boxShadow: '0 4px 16px rgba(0,0,0,0.12)',
            padding: '8px 12px',
            boxSizing: 'border-box',
            fontSize: 14,
            color: 'var(--pc-text-primary)',
            whiteSpace: 'nowrap',
            pointerEvents: 'none',
          }}
        >
          关闭
        </div>
      )}
    </div>
  );
}

function SkillSwitch({
  enabled,
  loading,
  onToggle,
}: {
  enabled: boolean;
  loading: boolean;
  onToggle: () => void;
}) {
  return (
    <button
      onClick={onToggle}
      disabled={loading}
      className="relative rounded-full transition-colors disabled:opacity-50 flex-shrink-0"
      style={{
        width: 44,
        height: 22,
        background: enabled ? '#3790FA' : 'var(--pc-border)',
      }}
      aria-pressed={enabled}
      aria-label={enabled ? t('skills.enabled') : t('skills.disabled')}
    >
      <span
        className="absolute top-1 left-1 w-[14px] h-[14px] rounded-full bg-white transition-transform"
        style={{
          transform: enabled ? 'translateX(22px)' : 'translateX(0)',
        }}
      />
    </button>
  );
}

function OverwriteConfirmDialog({
  open,
  onCancel,
  onConfirm,
  overwriting,
  skillName,
}: {
  open: boolean;
  onCancel: () => void;
  onConfirm: () => void;
  overwriting: boolean;
  skillName: string;
}) {
  if (!open) return null;

  return (
    <div
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        width: '100vw',
        height: '100vh',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 200,
        transform: 'translate(-28px, -40px)',
      }}
    >
      <div
        style={{
          width: 376,
          height: 212,
          background: '#fff',
          borderRadius: 8,
          boxSizing: 'border-box',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
          boxShadow: '0 6px 30px rgba(0,0,0,0.12)',
        }}
      >
        <div
          style={{
            width: 376,
            height: 40,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'flex-end',
            paddingRight: 8,
            boxSizing: 'border-box',
            flexShrink: 0,
          }}
        >
          <CloseTooltipButton onClick={onCancel} />
        </div>

        <div
          style={{
            paddingLeft: 24,
            paddingRight: 24,
            paddingBottom: 24,
            flex: 1,
            display: 'flex',
            flexDirection: 'column',
          }}
        >
          <div style={{ display: 'flex', alignItems: 'flex-start', gap: 8, marginBottom: 4 }}>
            <img
              src={`${assetBasePath}/icons/dialog-question.svg`}
              alt="question"
              width={16}
              height={16}
              style={{ flexShrink: 0, marginTop: 2 }}
            />
            <span style={{ fontSize: 14, color: 'var(--pc-text-primary)', lineHeight: '20px' }}>
              确认覆盖现有技能？
            </span>
          </div>

          <span
            style={{
              fontSize: 14,
              color: 'var(--pc-text-muted)',
              lineHeight: '20px',
              marginLeft: 24,
            }}
          >
            当前技能列表中已存在"{skillName}"，是否进行覆盖？
          </span>

          <div style={{ flex: 1 }} />

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: 8 }}>
            <button
              className="skill-dialog-cancel-btn"
              onClick={onCancel}
            >
              取消
            </button>
            <button
              className="skill-dialog-uninstall-btn"
              onClick={onConfirm}
              disabled={overwriting}
            >
              {overwriting ? (
                <Loader2 className="h-4 w-4 animate-spin" style={{ color: '#fff' }} />
              ) : (
                '覆盖'
              )}
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}

function UninstallConfirmDialog({
  open,
  onCancel,
  onConfirm,
  uninstalling,
}: {
  open: boolean;
  onCancel: () => void;
  onConfirm: () => void;
  uninstalling: boolean;
}) {
  if (!open) return null;

  return (
    <div
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        width: '100vw',
        height: '100vh',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 200,
        transform: 'translate(-28px, -40px)',
      }}
    >
      <div
        style={{
          width: 376,
          height: 212,
          background: '#fff',
          borderRadius: 8,
          boxSizing: 'border-box',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
          boxShadow: '0 6px 30px rgba(0,0,0,0.12)',
        }}
      >
        <div
          style={{
            width: 376,
            height: 40,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'flex-end',
            paddingRight: 8,
            boxSizing: 'border-box',
            flexShrink: 0,
          }}
        >
          <CloseTooltipButton onClick={onCancel} />
        </div>

        <div
          style={{
            paddingLeft: 24,
            paddingRight: 24,
            paddingBottom: 24,
            flex: 1,
            display: 'flex',
            flexDirection: 'column',
          }}
        >
          <div style={{ display: 'flex', alignItems: 'flex-start', gap: 8, marginBottom: 4 }}>
            <img
              src={`${assetBasePath}/icons/dialog-question.svg`}
              alt="question"
              width={16}
              height={16}
              style={{ flexShrink: 0, marginTop: 2 }}
            />
            <span style={{ fontSize: 14, color: 'var(--pc-text-primary)', lineHeight: '20px' }}>
              确认卸载该技能？
            </span>
          </div>

          <span
            style={{
              fontSize: 14,
              color: 'var(--pc-text-muted)',
              lineHeight: '20px',
              marginLeft: 24,
            }}
          >
            卸载后，该技能的本地文件与配置数据将被删除，且无法恢复。
          </span>

          <div style={{ flex: 1 }} />

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: 8 }}>
            <button
              className="skill-dialog-cancel-btn"
              onClick={onCancel}
            >
              取消
            </button>
            <button
              className="skill-dialog-uninstall-btn"
              onClick={onConfirm}
              disabled={uninstalling}
            >
              {uninstalling ? (
                <Loader2 className="h-4 w-4 animate-spin" style={{ color: '#fff' }} />
              ) : (
                '卸载'
              )}
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}

function SkillOperateMenu({
  skillSlug,
  onUninstalled,
}: {
  skillSlug: string;
  onUninstalled: () => void;
}) {
  const [menuOpen, setMenuOpen] = useState(false);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [uninstalling, setUninstalling] = useState(false);
  const [dropUp, setDropUp] = useState(false);
  const [tooltipVisible, setTooltipVisible] = useState(false);
  const [tooltipPos, setTooltipPos] = useState({ x: 0, y: 0 });
  const tooltipTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!menuOpen) return;
    const handler = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) setMenuOpen(false);
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [menuOpen]);

  const handleToggleMenu = () => {
    setMenuOpen((v) => {
      if (!v && ref.current) {
        const rect = ref.current.getBoundingClientRect();
        setDropUp(rect.bottom + 80 > window.innerHeight);
      }
      return !v;
    });
  };

  const handleUninstall = async () => {
    setUninstalling(true);
    try {
      await Promise.allSettled([
        uninstallSkill(skillSlug),
        deleteSkill(skillSlug),
      ]);
      setDialogOpen(false);
      setMenuOpen(false);
      onUninstalled();
    } catch (err) {
      console.error('Failed to uninstall skill:', err);
    } finally {
      setUninstalling(false);
    }
  };

  return (
    <>
      <UninstallConfirmDialog
        open={dialogOpen}
        onCancel={() => setDialogOpen(false)}
        onConfirm={handleUninstall}
        uninstalling={uninstalling}
      />
      <div ref={ref} style={{ position: 'relative', display: 'inline-block' }}>
        <button
          className="skill-operate-btn"
          style={{ cursor: 'default' }}
          onClick={handleToggleMenu}
          onMouseEnter={(e) => {
            setTooltipPos({ x: e.clientX, y: e.clientY });
            tooltipTimerRef.current = setTimeout(() => setTooltipVisible(true), 500);
          }}
          onMouseLeave={() => {
            if (tooltipTimerRef.current) clearTimeout(tooltipTimerRef.current);
            tooltipTimerRef.current = null;
            setTooltipVisible(false);
          }}
        >
          <img
            src={`${assetBasePath}/icons/operate-omit.svg`}
            alt="更多"
            width={16}
            height={16}
          />
        </button>
        {tooltipVisible && !menuOpen && (
          <div
            style={{
              position: 'fixed',
              left: tooltipPos.x + 8,
              top: tooltipPos.y + 8,
              zIndex: 100,
              background: '#fff',
              border: '1px solid #e8e8e8',
              borderRadius: 8,
              boxShadow: '0 4px 16px rgba(0,0,0,0.12)',
              padding: '8px 12px',
              boxSizing: 'border-box',
              fontSize: 14,
              color: 'var(--pc-text-primary)',
              whiteSpace: 'nowrap',
              pointerEvents: 'none',
            }}
          >
            更多
          </div>
        )}
        {menuOpen && (
          <div
            style={{
              position: 'absolute',
              right: 0,
              ...(dropUp ? { bottom: '100%' } : { top: '100%' }),
              zIndex: 100,
              background: '#fff',
              border: '1px solid #e8e8e8',
              borderRadius: 8,
              boxShadow: '0 4px 16px rgba(0,0,0,0.12)',
              width: 180,
              padding: 8,
              boxSizing: 'border-box',
              display: 'flex',
              flexDirection: 'column',
              gap: 2,
            }}
          >
            <button
              className="skill-uninstall-btn"
              style={{ height: 32 }}
              onClick={async () => {
                setMenuOpen(false);
                try { await openSkillFolder(skillSlug); } catch (e) { console.error('Failed to open folder:', e); }
              }}
            >
              文件所在位置
            </button>
            <button
              className="skill-uninstall-btn"
              style={{ height: 32 }}
              onClick={() => { setMenuOpen(false); setDialogOpen(true); }}
            >
              卸载
            </button>
          </div>
        )}
      </div>
    </>
  );
}

function SkillCard({
  skill,
  isDefault,
  isToggling,
  onToggle,
  onUninstalled,
}: {
  skill: Skill;
  isDefault: boolean;
  isToggling: boolean;
  onToggle: () => void;
  onUninstalled: () => void;
}) {
  const descRef = useRef<HTMLParagraphElement>(null);
  const nameRef = useRef<HTMLDivElement>(null);
  const [isTruncated, setIsTruncated] = useState(false);
  const [showTooltip, setShowTooltip] = useState(false);
  const [tooltipAbove, setTooltipAbove] = useState(false);
  const [nameTruncated, setNameTruncated] = useState(false);
  const [showNameTooltip, setShowNameTooltip] = useState(false);
  const [nameTooltipAbove, setNameTooltipAbove] = useState(false);

  useEffect(() => {
    requestAnimationFrame(() => {
      if (descRef.current) {
        setIsTruncated(descRef.current.scrollHeight > descRef.current.clientHeight);
      }
      if (nameRef.current) {
        setNameTruncated(nameRef.current.scrollWidth > nameRef.current.clientWidth);
      }
    });
  }, [skill.description, skill.name]);

  return (
    <div
      style={{
        width: 326,
        height: 140,
        padding: 16,
        background: '#F6F6F6',
        borderRadius: 6,
        boxSizing: 'border-box',
        position: 'relative',
      }}
    >
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          height: 20,
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: 8, minWidth: 0, flex: 1 }}>
          <div
            style={{
              width: 32,
              height: 32,
              background: '#E1EEFE',
              borderRadius: 4,
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              flexShrink: 0,
            }}
          >
            <span style={{ fontSize: 16, color: '#3790FA', fontWeight: 600, lineHeight: 1 }}>
              {skill.name.charAt(0).toUpperCase()}
            </span>
          </div>
          <div
            className="relative"
            onMouseEnter={() => {
              if (!nameTruncated) return;
              if (nameRef.current) {
                const rect = nameRef.current.getBoundingClientRect();
                setNameTooltipAbove(rect.bottom + 100 > window.innerHeight);
              }
              setShowNameTooltip(true);
            }}
            onMouseLeave={() => setShowNameTooltip(false)}
            style={{ flex: 1, minWidth: 0 }}
          >
            <div
              ref={nameRef}
              style={{
                fontSize: 14,
                color: 'var(--pc-text-primary)',
                fontWeight: 600,
                overflow: 'hidden',
                textOverflow: 'ellipsis',
                whiteSpace: 'nowrap',
              }}
            >
              {skill.name}
            </div>
            {nameTruncated && showNameTooltip && (
              <div
                className={`absolute z-50 left-0 p-2 rounded-md text-xs whitespace-pre-wrap max-w-[300px] shadow-lg border ${nameTooltipAbove ? 'bottom-full mb-1' : 'top-full mt-1'}`}
                style={{
                  background: '#ffffff',
                  color: 'var(--pc-text-primary)',
                  borderColor: 'var(--pc-border)',
                }}
              >
                {skill.name}
              </div>
            )}
          </div>
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: 4, flexShrink: 0 }}>
          {!isDefault && (
            <SkillOperateMenu
              skillSlug={skill.slug}
              onUninstalled={onUninstalled}
            />
          )}
          <SkillSwitch
            enabled={skill.enabled}
            loading={isToggling}
            onToggle={onToggle}
          />
        </div>
      </div>

      <div
        className="relative"
        onMouseEnter={() => {
          if (!isTruncated) return;
          if (descRef.current) {
            const rect = descRef.current.getBoundingClientRect();
            setTooltipAbove(rect.bottom + 100 > window.innerHeight);
          }
          setShowTooltip(true);
        }}
        onMouseLeave={() => setShowTooltip(false)}
      >
        <p
          ref={descRef}
          style={{
            width: 294,
            height: 40,
            fontSize: 14,
            color: 'var(--pc-text-muted)',
            margin: 0,
            marginTop: 8,
            lineHeight: '20px',
            overflow: 'hidden',
            display: '-webkit-box',
            WebkitLineClamp: 2,
            WebkitBoxOrient: 'vertical',
          }}
        >
          {skill.description}
        </p>
        {isTruncated && showTooltip && (
          <div
            className={`absolute z-50 left-0 p-2 rounded-md text-xs whitespace-pre-wrap max-w-[300px] shadow-lg border ${tooltipAbove ? 'bottom-full mb-1' : 'top-full mt-1'}`}
            style={{
              background: '#ffffff',
              color: 'var(--pc-text-primary)',
              borderColor: 'var(--pc-border)',
            }}
          >
            {skill.description}
          </div>
        )}
      </div>

      <div style={{ position: 'absolute', bottom: 16, left: 16 }}>
        {isDefault ? (
          <span
            style={{
              display: 'inline-flex',
              alignItems: 'center',
              height: 24,
              paddingLeft: 8,
              paddingRight: 8,
              fontSize: 12,
              color: '#3790FA',
              background: '#E1EEFE',
              borderRadius: 10,
              boxSizing: 'border-box',
            }}
          >
            麒麟认证
          </span>
        ) : (
          <span
            style={{
              display: 'inline-flex',
              alignItems: 'center',
              height: 24,
              paddingLeft: 8,
              paddingRight: 8,
              fontSize: 12,
              color: 'var(--pc-text-secondary)',
              background: '#E6E6E6',
              borderRadius: 10,
              boxSizing: 'border-box',
            }}
          >
            未认证
          </span>
        )}
      </div>
    </div>
  );
}

export default function Skills() {
  const { data: skills, loading, error, refetch } = useSkills();
  const [toggling, setToggling] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [isSearchFocused, setIsSearchFocused] = useState(false);
  const searchInputRef = useRef<HTMLInputElement>(null);
  const [addDialogOpen, setAddDialogOpen] = useState(false);
  const [tip, setTip] = useState<{ success: boolean; message: string } | null>(null);

  useEffect(() => {
    const handler = (e: MouseEvent) => e.preventDefault();
    document.addEventListener('contextmenu', handler);
    return () => document.removeEventListener('contextmenu', handler);
  }, []);

  const handleToggle = async (skillName: string, currentEnabled: boolean) => {
    setToggling(skillName);
    try {
      await toggleSkillApi(skillName, !currentEnabled);
      refetch();
    } catch (err) {
      console.error('Failed to toggle skill:', err);
      setToggling(null);
    }
  };

  useEffect(() => {
    if (toggling) {
      setToggling(null);
    }
  }, [skills, error]);

  const filtered = (skills || []).filter((skill) => {
    if (!searchQuery) return true;
    const q = searchQuery.toLowerCase();
    return (
      skill.name.toLowerCase().includes(q) ||
      skill.description.toLowerCase().includes(q)
    );
  });

  const sorted = filtered;

  if (loading && !skills) {
    return (
      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', minHeight: 200 }}>
        <Loader2 className="h-8 w-8 animate-spin" style={{ color: 'var(--pc-accent)' }} />
      </div>
    );
  }

  if (error) {
    return (
      <div style={{ textAlign: 'center', padding: 24 }}>
        <p style={{ color: 'var(--pc-text-muted)' }}>{t('skills.empty')}</p>
      </div>
    );
  }

  return (
    <div
      style={{
        maxWidth: 1040,
        width: '100%',
        padding: '0 0 0 24px',
        boxSizing: 'border-box',
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      <div
        style={{
          height: 36,
          display: 'flex',
          alignItems: 'center',
          flexShrink: 0,
          paddingRight: 24,
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: 4 }}>
          <span style={{ fontSize: 14, color: 'var(--pc-text-primary)', fontWeight: 500 }}>
            已安装（{skills?.length || 0}）
          </span>
        </div>

        <div style={{ flex: 1 }} />

        <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
          <div
            onClick={() => searchInputRef.current?.focus()}
            style={{
              width: 236,
              height: 36,
              background: isSearchFocused ? '#FFFFFF' : '#E6E6E6',
              borderRadius: 4,
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              paddingLeft: 10,
              paddingRight: 10,
              gap: 6,
              boxSizing: 'border-box',
              border: isSearchFocused ? '2px solid #3790FA' : '2px solid transparent',
              transition: 'background 0.2s ease, border-color 0.2s ease',
              cursor: 'text',
            }}
          >
            <div
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: 6,
                width: '100%',
                justifyContent: isSearchFocused || searchQuery ? 'flex-start' : 'center',
              }}
            >
              <img
                src={`${assetBasePath}/icons/search.svg`}
                alt="search"
                width={14}
                height={14}
                style={{ flexShrink: 0 }}
              />
              <input
                ref={searchInputRef}
                type="text"
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                onFocus={() => setIsSearchFocused(true)}
                onBlur={() => setIsSearchFocused(false)}
                placeholder="搜索"
                className="placeholder-[#A6A6A6]"
                style={{
                  border: 'none',
                  outline: 'none',
                  background: 'transparent',
                  fontSize: 14,
                  color: 'var(--pc-text-primary)',
                  width: isSearchFocused || searchQuery ? '100%' : 40,
                  flex: isSearchFocused || searchQuery ? 1 : 'none',
                  padding: 0,
                  textAlign: 'left',
                }}
              />
              {searchQuery && (
                <button
                  onClick={() => setSearchQuery('')}
                  style={{
                    border: 'none',
                    background: 'transparent',
                    cursor: 'pointer',
                    padding: 0,
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                    flexShrink: 0,
                    color: 'var(--pc-text-muted)',
                  }}
                >
                  <X size={14} />
                </button>
              )}
            </div>
          </div>

          <ImportMenu
            onImportResult={(success) => {
              setTip({ success, message: success ? '技能添加成功' : '技能添加失败' });
              if (success) refetch();
              setTimeout(() => setTip(null), 3000);
            }}
            onAddClick={() => setAddDialogOpen(true)}
          />

          {/* 技能中心按钮已隐藏 */}
        </div>
      </div>

      <div
        className="skill-list-scroll scrollbar-auto-hide"
        style={{
          flex: 1,
          minHeight: 0,
          overflowY: 'overlay',
          overflowX: 'hidden',
          marginTop: 8,
          paddingRight: 24,
        }}
      >
        <div
          style={{
            width: 992,
            display: 'flex',
            flexWrap: 'wrap',
            gap: 7,
          }}
        >
          {sorted.length === 0 ? (
            <div
              style={{
                width: 992,
                textAlign: 'center',
                padding: '48px 0',
                color: 'var(--pc-text-muted)',
                fontSize: 14,
              }}
            >
              {t('skills.empty')}
            </div>
          ) : (
            sorted.map((skill) => (
              <SkillCard
                key={skill.name}
                skill={skill}
                isDefault={skill.is_default}
                isToggling={toggling === skill.slug}
                onToggle={() => handleToggle(skill.slug, skill.enabled)}
                onUninstalled={() => refetch()}
              />
            ))
          )}
        </div>
      </div>
      <AddSkillDialog
        open={addDialogOpen}
        onCancel={() => setAddDialogOpen(false)}
        onCreated={() => { setAddDialogOpen(false); refetch(); setTip({ success: true, message: '技能添加成功' }); setTimeout(() => setTip(null), 3000); }}
        existingSlugs={(skills ?? []).map((s: Skill) => s.slug)}
      />
      {tip && (
        <div
          style={{
            position: 'fixed',
            top: 0,
            left: 0,
            width: '100vw',
            height: '100vh',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            zIndex: 300,
          }}
          onClick={() => setTip(null)}
        >
          <div
            style={{
              padding: '16px 24px',
              borderRadius: 8,
              fontSize: 14,
              color: 'var(--pc-text-primary)',
              background: '#ffffff',
              border: '1px solid var(--pc-border)',
              boxShadow: '0 6px 30px rgba(0,0,0,0.12)',
              display: 'flex',
              alignItems: 'center',
              gap: 8,
            }}
            onClick={(e) => e.stopPropagation()}
          >
            <img
              src={`${assetBasePath}/icons/${tip.success ? 'dialog-success.svg' : 'dialog-error.svg'}`}
              alt={tip.success ? 'success' : 'error'}
              width={16}
              height={16}
              style={{ flexShrink: 0 }}
            />
            <span>{tip.message}</span>
          </div>
        </div>
      )}
    </div>
  );
}
