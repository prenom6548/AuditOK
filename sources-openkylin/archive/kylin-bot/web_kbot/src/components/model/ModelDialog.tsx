import { useEffect, useMemo, useState, useCallback, useRef, useId } from 'react';
import { ChevronDown, ChevronUp } from 'lucide-react';
import { ModelCloseButton } from '@/components/model/modelTooltips';
import { SKILL_EMBEDDED_DIALOG_SHELL } from '@/components/model/modelEmbeddedShellStyles';
import {
  ADD_MODEL_DEFAULT_PROVIDER_KEY,
  findPresetByKey,
  resolveProviderMeta,
  type PresetProviderKey,
} from './providerMeta';
import { ModelDropdownOption } from './ModelDropdownOption';
import { ProviderPicker } from './ProviderPicker';
import type { ModelInfo } from '@/lib/models';
import { getCatalogModels, getCatalogProviderDefaults, getProp, listProps } from '@/lib/api';
import { assetBasePath } from '@/lib/basePath';

export type ModelDialogMode =
  | { kind: 'add' }
  | { kind: 'edit'; original: ModelInfo };

/** Per-million-token pricing in CNY (元); stored as-is in config `pricing`. */
export interface ModelDialogPricing {
  input: number;
  output: number;
}

export interface ModelDialogValue {
  provider: PresetProviderKey;
  storageProvider: string;
  modelName: string;
  apiKey: string;
  baseUrl: string;
  label: string;
  /** `null` = clear pricing; `undefined` omitted on add with no fields filled */
  pricing?: ModelDialogPricing | null;
}

interface ModelDialogProps {
  open: boolean;
  mode: ModelDialogMode | null;
  onCancel: () => void;
  onSubmit: (value: ModelDialogValue) => Promise<boolean | void> | boolean | void;
  keyboardShortcuts?: boolean;
  /** 上层确认层打开时禁用下层点击，避免误触下层「取消」 */
  interactionLocked?: boolean;
}

function isAllWhitespace(s: string) {
  return s.length > 0 && s.trim().length === 0;
}

/** Offline / unreachable API — matches today’s `OllamaProvider::BASE_URL`. */
const OLLAMA_BASE_URL_FALLBACK = 'http://localhost:11434';

/** Placeholder / 空态提示（与 design 一致：灰 35% 黑） */
const FIELD_HINT_COLOR = 'rgba(0,0,0,0.35)';

/** Design body font */
const FONT =
  "'Noto Sans CJK SC', 'Source Han Sans SC', 'PingFang SC', 'Microsoft YaHei', sans-serif";

const K_ALPHA_NORMAL = 'rgba(0, 0, 0, 0.1)';
const K_ALPHA_HOVER = 'rgba(0, 0, 0, 0.15)';
const K_ALPHA_PRESS = 'rgba(0, 0, 0, 0.2)';

const PRIMARY_BTN = '#3790FA';
const PRIMARY_HOVER = '#3489ee';
const PRIMARY_PRESS = '#2c73c8';

const PRICING_UNIT = '元/百万tokens';

const TEXT_MAIN = 'rgba(0, 0, 0, 0.85)';
/** 主弹窗 overlay 滚动条与右边缘留白（px） */
const SCROLLBAR_RIGHT_GUTTER = 6;

/** 输入时仅保留非负小数字面量（数字 + 至多一个小数点），非法字符静默丢弃。 */
function sanitizePricingInput(raw: string): string {
  const filtered = raw.replace(/[^\d.]/g, '');
  const dotIndex = filtered.indexOf('.');
  if (dotIndex === -1) return filtered;
  const before = filtered.slice(0, dotIndex + 1);
  const after = filtered.slice(dotIndex + 1).replace(/\./g, '');
  return before + after;
}

function parsePricingField(raw: string): number | null {
  const t = raw.trim();
  if (t.length === 0) return null;
  const n = Number(t);
  if (!Number.isFinite(n) || n < 0) return null;
  return n;
}

function parsePricingFromForm(
  inputRaw: string,
  outputRaw: string,
): { ok: true; value: ModelDialogPricing | null } | { ok: false; message: string } {
  const inputTrim = inputRaw.trim();
  const outputTrim = outputRaw.trim();
  if (inputTrim.length === 0 && outputTrim.length === 0) {
    return { ok: true, value: null };
  }
  const input = parsePricingField(inputRaw);
  const output = parsePricingField(outputRaw);
  if (inputTrim.length > 0 && input === null) {
    return { ok: false, message: '模型输入 Tokens 单价需为非负数字' };
  }
  if (outputTrim.length > 0 && output === null) {
    return { ok: false, message: '模型输出 Tokens 单价需为非负数字' };
  }
  return {
    ok: true,
    value: { input: input ?? 0, output: output ?? 0 },
  };
}

function parsePricingNumberField(value: unknown): number | null {
  if (typeof value === 'number' && Number.isFinite(value)) return value >= 0 ? value : null;
  if (typeof value === 'string') {
    const t = value.trim();
    if (t.length === 0) return null;
    const n = Number(t);
    if (!Number.isFinite(n) || n < 0) return null;
    return n;
  }
  return null;
}

function pricingFromTomlDisplay(raw: string): ModelDialogPricing | null {
  const inputMatch = raw.match(/(?:^|[\s,{])input\s*=\s*([0-9]+(?:\.[0-9]+)?)/);
  const outputMatch = raw.match(/(?:^|[\s,{])output\s*=\s*([0-9]+(?:\.[0-9]+)?)/);
  if (!inputMatch && !outputMatch) return null;
  const input = inputMatch ? Number(inputMatch[1]) : 0;
  const output = outputMatch ? Number(outputMatch[1]) : 0;
  if (input === 0 && output === 0) return null;
  return { input, output };
}

function pricingFromPropValue(value: unknown): ModelDialogPricing | null {
  if (value == null) return null;

  let record: Record<string, unknown> | null = null;
  if (typeof value === 'object' && !Array.isArray(value)) {
    record = value as Record<string, unknown>;
  } else if (typeof value === 'string') {
    const trimmed = value.trim();
    if (trimmed.length === 0 || trimmed === '<unset>') return null;
    try {
      const parsed: unknown = JSON.parse(trimmed);
      if (parsed && typeof parsed === 'object' && !Array.isArray(parsed)) {
        record = parsed as Record<string, unknown>;
      }
    } catch {
      return pricingFromTomlDisplay(trimmed);
    }
  }

  if (!record) return null;
  const input = parsePricingNumberField(record.input) ?? 0;
  const output = parsePricingNumberField(record.output) ?? 0;
  if (input === 0 && output === 0) return null;
  return { input, output };
}

export function ModelDialog({
  open,
  mode,
  onCancel,
  onSubmit,
  keyboardShortcuts = true,
  interactionLocked = false,
}: ModelDialogProps) {
  const isEdit = mode?.kind === 'edit';
  const dialogModeKind = mode?.kind ?? null;
  const editTargetKey =
    mode?.kind === 'edit'
      ? `${mode.original.provider}\0${mode.original.value}`
      : null;

  const [provider, setProvider] = useState<PresetProviderKey>(
    ADD_MODEL_DEFAULT_PROVIDER_KEY,
  );
  const [modelName, setModelName] = useState('');
  const [apiKey, setApiKey] = useState('');
  const [baseUrl, setBaseUrl] = useState('');
  const [showApiKey, setShowApiKey] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [submitting, setSubmitting] = useState(false);
  const [advancedOpen, setAdvancedOpen] = useState(false);
  const [pricingInput, setPricingInput] = useState('');
  const [pricingOutput, setPricingOutput] = useState('');
  /** Edit mode baseline — used to disable「确定」until a field changes. */
  const [editBaseline, setEditBaseline] = useState<{
    modelName: string;
    apiKey: string;
    baseUrl: string;
    pricingInput: string;
    pricingOutput: string;
  } | null>(null);

  const [ollamaModels, setOllamaModels] = useState<string[] | null>(null);
  const [ollamaLoading, setOllamaLoading] = useState(false);
  const [ollamaFailed, setOllamaFailed] = useState(false);
  const [ollamaModelPickerOpen, setOllamaModelPickerOpen] = useState(false);
  const ollamaModelPickerRef = useRef<HTMLDivElement | null>(null);
  const panelRef = useRef<HTMLDivElement | null>(null);
  /** Avoid stale async committing Ollama default after switching to custom / presets */
  const providerRef = useRef(provider);
  /** Stale guards for async Ollama URL hydration */
  const dialogOpenRef = useRef(open);
  const dialogModeKindRef = useRef<ModelDialogMode['kind'] | null>(
    mode?.kind ?? null,
  );
  /** Add-dialog only: fetch default URL once per Ollama "session"; never overwrite after user edits */
  const ollamaAddBaseUrlHydratedRef = useRef(false);
  const editTargetKeyRef = useRef(editTargetKey);

  const fetchOllamaModels = useCallback(async () => {
    setOllamaLoading(true);
    setOllamaFailed(false);
    setOllamaModels(null);
    try {
      const response = await getCatalogModels('ollama');
      if (response.live && response.models.length > 0) {
        setOllamaModels(response.models);
      } else {
        setOllamaFailed(true);
      }
    } catch {
      setOllamaFailed(true);
    } finally {
      setOllamaLoading(false);
    }
  }, []);

  useEffect(() => {
    if (!open || !mode) return;
    ollamaAddBaseUrlHydratedRef.current = false;
    if (mode.kind === 'add') {
      setProvider(ADD_MODEL_DEFAULT_PROVIDER_KEY);
      setModelName('');
      setApiKey('');
      setBaseUrl('');
      setPricingInput('');
      setPricingOutput('');
      setEditBaseline(null);
      setAdvancedOpen(false);
      setOllamaModels(null);
      setOllamaFailed(false);
    } else {
      const { meta, baseUrl: existingBaseUrl } = resolveProviderMeta(
        mode.original.provider,
      );
      const initialModelName = mode.original.value;
      const initialApiKey = mode.original.apiKey ?? '';
      const initialBaseUrl = existingBaseUrl ?? mode.original.baseUrl ?? '';
      setProvider(meta.key);
      setModelName(initialModelName);
      setApiKey(initialApiKey);
      setBaseUrl(initialBaseUrl);
      setPricingInput('');
      setPricingOutput('');
      setEditBaseline({
        modelName: initialModelName.trim(),
        apiKey: initialApiKey.trim(),
        baseUrl: initialBaseUrl.trim(),
        pricingInput: '',
        pricingOutput: '',
      });
      setAdvancedOpen(false);
      if (meta.key === 'ollama') {
        void fetchOllamaModels();
      }
      const storageKey = mode.original.provider;
      const expectedEditKey = editTargetKey;
      void (async () => {
        try {
          const r = await getProp(`providers.models.${storageKey}.pricing`);
          if (expectedEditKey !== editTargetKeyRef.current) return;
          const parsed = pricingFromPropValue(r.value);
          if (parsed) {
            const input = String(parsed.input);
            const output = String(parsed.output);
            setPricingInput(input);
            setPricingOutput(output);
            setEditBaseline((prev) =>
              prev
                ? { ...prev, pricingInput: input, pricingOutput: output }
                : prev,
            );
          }
        } catch {
          /* pricing optional */
        }
      })();
    }
    setShowApiKey(false);
    setError(null);
    setSubmitting(false);
    setOllamaModelPickerOpen(false);
  }, [open, dialogModeKind, editTargetKey, fetchOllamaModels]);

  useEffect(() => {
    providerRef.current = provider;
    dialogOpenRef.current = open;
    dialogModeKindRef.current = mode?.kind ?? null;
    editTargetKeyRef.current = editTargetKey;
  }, [provider, open, mode, editTargetKey]);

  useEffect(() => {
    if (provider !== 'ollama' || !open || !mode) return;
    if (!ollamaModels && !ollamaFailed && !ollamaLoading) {
      void fetchOllamaModels();
    }
  }, [
    provider,
    open,
    mode,
    ollamaModels,
    ollamaFailed,
    ollamaLoading,
    fetchOllamaModels,
  ]);

  /**
   * 添加模型 + Ollama：仅在本弹窗内回填一次默认请求地址；不把 `baseUrl` 放进 deps，
   * 用户改过之后不会因为 effect 重新跑而被刷掉。
   */
  useEffect(() => {
    if (!open || mode?.kind !== 'add') return;
    if (provider !== 'ollama') return;
    if (ollamaAddBaseUrlHydratedRef.current) return;

    let cancelled = false;
    void (async () => {
      let suggestion = '';
      try {
        const { entries } = await listProps('providers.models.ollama');
        const row = entries.find((e) => e.path === 'providers.models.ollama.base-url');
        const raw = row?.value;
        if (
          row?.populated &&
          typeof raw === 'string' &&
          raw.trim().length > 0 &&
          raw !== '<unset>'
        ) {
          suggestion = raw.trim();
        }
      } catch {
        /* ignore (401 etc.) */
      }
      if (!suggestion && !cancelled) {
        try {
          const d = await getCatalogProviderDefaults('ollama');
          if (typeof d.base_url === 'string' && d.base_url.trim().length > 0) {
            suggestion = d.base_url.trim();
          }
        } catch {
          /* ignore */
        }
      }
      if (!suggestion && !cancelled) suggestion = OLLAMA_BASE_URL_FALLBACK;

      if (
        cancelled ||
        providerRef.current !== 'ollama' ||
        !dialogOpenRef.current ||
        dialogModeKindRef.current !== 'add'
      ) {
        return;
      }
      ollamaAddBaseUrlHydratedRef.current = true;
      setBaseUrl((prev) => (prev.trim().length > 0 ? prev : suggestion));
    })();

    return () => {
      cancelled = true;
    };
  }, [provider, open, mode]);

  useEffect(() => {
    if (!ollamaModelPickerOpen) return;
    const handler = (e: MouseEvent) => {
      const target = e.target as Node | null;
      if (ollamaModelPickerRef.current && !ollamaModelPickerRef.current.contains(target)) {
        setOllamaModelPickerOpen(false);
      }
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [ollamaModelPickerOpen]);

  useEffect(() => {
    if (!open || !mode) return;
    const prev = document.body.style.overflow;
    document.body.style.overflow = 'hidden';
    return () => {
      document.body.style.overflow = prev;
    };
  }, [open, mode]);

  const meta = findPresetByKey(provider);
  const isCustom = meta.isCustom;
  const isOllama = provider === 'ollama';
  const needsBaseUrl = isCustom || isOllama;

  const trimmedModel = modelName.trim();
  const trimmedBaseUrl = baseUrl.trim();
  const modelNameValid = trimmedModel.length > 0 && !isAllWhitespace(modelName);
  const baseUrlValid = !needsBaseUrl || trimmedBaseUrl.length > 0;
  const baseUrlSchemeOk =
    !needsBaseUrl ||
    trimmedBaseUrl.length === 0 ||
    /^https?:\/\//i.test(trimmedBaseUrl);
  const showBaseUrlSchemeError = needsBaseUrl && trimmedBaseUrl.length > 0 && !baseUrlSchemeOk;
  const ollamaDisabled = isOllama && ollamaFailed;
  const hasEditChanges = useMemo(() => {
    if (!isEdit || !editBaseline) return true;
    if (trimmedModel !== editBaseline.modelName) return true;
    if (apiKey.trim() !== editBaseline.apiKey) return true;
    if (trimmedBaseUrl !== editBaseline.baseUrl) return true;
    if (!isOllama) {
      if (pricingInput.trim() !== editBaseline.pricingInput.trim()) return true;
      if (pricingOutput.trim() !== editBaseline.pricingOutput.trim()) return true;
    }
    return false;
  }, [
    isEdit,
    editBaseline,
    trimmedModel,
    apiKey,
    trimmedBaseUrl,
    pricingInput,
    pricingOutput,
    isOllama,
  ]);
  const canSubmit =
    modelNameValid &&
    baseUrlValid &&
    baseUrlSchemeOk &&
    !submitting &&
    !ollamaDisabled &&
    hasEditChanges;

  const dialogWidth = 376;

  const apiKeyPlaceholder = useMemo(() => {
    if (isOllama) return '选填';
    return '选填（云端服务必填）';
  }, [isOllama]);

  const baseUrlLabel = useMemo(() => {
    if (isOllama) return '请求地址';
    return '请求地址（OpenAI 兼容）';
  }, [isOllama]);

  /** Only custom rows use the MiniMax示例；Ollama 由服务端默认预填正文，占位留空以免误导 */
  const baseUrlPlaceholder = useMemo(
    () => (isCustom ? '必填（https://api.minimaxi.com/v1）' : '必填（http://localhost:11434）'),
    [isCustom],
  );

  const handleSubmit = async () => {
    if (!canSubmit || !mode) return;
    setError(null);
    if (needsBaseUrl && trimmedBaseUrl.length > 0 && !/^https?:\/\//i.test(trimmedBaseUrl)) {
      return;
    }
    const pricingResult = parsePricingFromForm(pricingInput, pricingOutput);
    if (!pricingResult.ok) {
      setError(pricingResult.message);
      return;
    }
    const targetStorageProvider = isCustom
      ? `custom:${trimmedBaseUrl}`
      : meta.storageProvider;
    setSubmitting(true);
    try {
      const result = await onSubmit({
        provider,
        storageProvider: targetStorageProvider,
        modelName: trimmedModel,
        apiKey: apiKey.trim(),
        baseUrl: trimmedBaseUrl,
        label: trimmedModel,
        pricing:
          pricingResult.value !== null
            ? pricingResult.value
            : isEdit
              ? null
              : undefined,
      });
      if (result === false) setSubmitting(false);
    } catch (e) {
      setError(e instanceof Error ? e.message : String(e));
      setSubmitting(false);
    }
  };

  const submitButtonLabel = useMemo(() => (isEdit ? '确定' : '添加'), [isEdit]);
  const title = isEdit ? '编辑模型' : '添加模型';

  useEffect(() => {
    if (!open || !mode || !keyboardShortcuts) return;
    const handler = (e: KeyboardEvent) => {
      if (e.key === 'Escape') {
        e.stopPropagation();
        onCancel();
        return;
      }
      if (e.key === 'Enter' && canSubmit) {
        const target = e.target as HTMLElement | null;
        if (target?.tagName === 'TEXTAREA') return;
        e.preventDefault();
        void handleSubmit();
      }
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, [open, mode, keyboardShortcuts, onCancel, canSubmit, handleSubmit]);

  useEffect(() => {
    if (!open || !mode) return;
    let cancelled = false;
    const id = window.setTimeout(() => {
      if (cancelled) return;
      const root = panelRef.current;
      if (!root?.isConnected) return;
      const field = root.querySelector<HTMLElement>(
        'input:not([type="hidden"]):not([disabled]),textarea:not([disabled]),select:not([disabled])',
      );
      const focusable =
        field ?? root.querySelector<HTMLElement>('button:not([disabled])');
      focusable?.focus();
    }, 30);
    return () => {
      cancelled = true;
      window.clearTimeout(id);
    };
  }, [open, mode]);

  const handleProviderChange = (p: PresetProviderKey) => {
    if (isEdit) return;
    if (p !== provider) {
      setProvider(p);
      setError(null);
      setModelName('');
      setApiKey('');
      setBaseUrl('');
      setPricingInput('');
      setPricingOutput('');
      setAdvancedOpen(false);
      setShowApiKey(false);
      setOllamaModelPickerOpen(false);
      if (provider === 'ollama' && p !== 'ollama') {
        ollamaAddBaseUrlHydratedRef.current = false;
      }
    }
  };

  const selectOllamaModel = (m: string) => {
    setModelName(m);
    setOllamaModelPickerOpen(false);
  };

  if (!mode || !open) return null;

  return (
    <div
      role="dialog"
      aria-modal="true"
      aria-label={title}
      style={{
        ...SKILL_EMBEDDED_DIALOG_SHELL,
        pointerEvents: interactionLocked ? 'none' : undefined,
      }}
    >
      <div
        ref={panelRef}
        style={{
          width: dialogWidth,
          maxHeight: '85vh',
          background: '#fff',
          borderRadius: 8,
          boxSizing: 'border-box',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
          boxShadow: '0 6px 30px rgba(0, 0, 0, 0.12)',
          fontFamily: FONT,
          fontSize: 14,
          fontWeight: 400 as const,
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <div
          style={{
            height: 40,
            flexShrink: 0,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            paddingLeft: 12,
            paddingRight: 8,
            boxSizing: 'border-box',
          }}
        >
          <div
            style={{
              display: 'flex',
              flexDirection: 'row',
              alignItems: 'center',
              alignSelf: 'stretch',
              flex: '1 1 auto',
              minWidth: 0,
              lineHeight: 0,
            }}
          >
            <span
              style={{
                display: 'inline-flex',
                flexShrink: 0,
                alignItems: 'center',
                justifyContent: 'center',
                alignSelf: 'center',
              }}
            >
              <img
                src={`${assetBasePath}/icons/logo.png`}
                alt=""
                width={24}
                height={24}
                style={{ width: 24, height: 24, objectFit: 'contain', display: 'block' }}
                onError={(e) => {
                  e.currentTarget.style.display = 'none';
                }}
              />
            </span>
            <span
              style={{
                display: 'inline-flex',
                alignItems: 'center',
                alignSelf: 'center',
                marginLeft: 8,
                lineHeight: '20px',
                fontSize: 14,
                color: TEXT_MAIN,
              }}
            >
              {title}
            </span>
          </div>
          <div style={{ display: 'inline-flex' }}>
            <ModelCloseButton onClick={onCancel} />
          </div>
        </div>

        <div
          className="model-dialog-scrollbar"
          style={{
            flex: 1,
            minHeight: 0,
            maxHeight: advancedOpen && !isOllama ? 520 : undefined,
            overflowY: advancedOpen && !isOllama ? 'overlay' : 'visible',
            marginRight: SCROLLBAR_RIGHT_GUTTER,
            paddingLeft: 24,
            paddingRight: 24,
            boxSizing: 'border-box',
          }}
        >
          <div className="flex flex-col [&_input::placeholder]:text-[rgb(0_0_0_/_0.35)]">
        <div style={{ marginTop: 8, marginBottom: 4 }}>
          <Field label="模型提供商">
            <ProviderPicker
              value={provider}
              onChange={handleProviderChange}
              disabled={isEdit}
            />
          </Field>
        </div>

        <div style={{ marginTop: 18, marginBottom: 4 }}>
          <Field label="模型名称">
            {isOllama && ollamaModels && ollamaModels.length > 0 ? (
              <div ref={ollamaModelPickerRef} className="relative w-full" style={{ fontFamily: FONT }}>
                <button
                  type="button"
                  onClick={() => setOllamaModelPickerOpen((v) => !v)}
                  className="box-border w-full border-none outline-none"
                  style={{
                    height: 36,
                    padding: 8,
                    borderRadius: 6,
                    background: '#E6E6E6',
                    display: 'flex',
                    flexDirection: 'row',
                    flexWrap: 'nowrap',
                    alignItems: 'center',
                    justifyContent: 'space-between',
                    columnGap: 8,
                    cursor: 'default',
                    color: modelName.trim() ? TEXT_MAIN : FIELD_HINT_COLOR,
                  }}
                >
                  <span
                    className="min-w-0 flex-1 truncate text-left"
                    style={{ fontSize: 14, fontWeight: 400 }}
                  >
                    {modelName.trim() ? modelName : '请选择本地模型'}
                  </span>
                  <span
                    style={{
                      display: 'flex',
                      flexShrink: 0,
                      alignItems: 'center',
                      justifyContent: 'center',
                      height: 20,
                      lineHeight: 0,
                    }}
                  >
                    <img
                      src={`${assetBasePath}/icons/${ollamaModelPickerOpen ? 'ukui-up-symbolic.svg' : 'ukui-down-symbolic.svg'}`}
                      alt=""
                      width={16}
                      height={16}
                      className="object-contain"
                      draggable={false}
                      style={{ display: 'block' }}
                    />
                  </span>
                </button>
                {ollamaModelPickerOpen && (
                  <div
                    className="model-dialog-dropdown absolute left-0 z-[120] box-border flex flex-col overflow-hidden"
                    style={{
                      top: 'calc(100% - 36px)',
                      width: 328,
                      maxHeight: 274,
                      padding: '8px 6px 8px 8px',
                      borderRadius: 8,
                      border: '1px solid #E5E7EB',
                      background: '#FFFFFF',
                    }}
                  >
                    <div
                      role="listbox"
                      className="model-dialog-scrollbar overscroll-contain"
                      style={{ maxHeight: 258 }}
                      onMouseDown={(e) => e.stopPropagation()}
                    >
                    {ollamaModels.map((m) => (
                      <ModelDropdownOption
                        key={m}
                        selected={m === modelName}
                        onPick={() => selectOllamaModel(m)}
                      >
                        <span className="min-w-0 flex-1 truncate text-left">{m}</span>
                      </ModelDropdownOption>
                    ))}
                    </div>
                  </div>
                )}
              </div>
            ) : isOllama && ollamaLoading ? (
              <div
                style={{
                  height: 36,
                  paddingLeft: 8,
                  paddingRight: 8,
                  borderRadius: 6,
                  background: '#E6E6E6',
                  color: FIELD_HINT_COLOR,
                  display: 'flex',
                  alignItems: 'center',
                  fontSize: 14,
                  fontWeight: 400,
                  boxSizing: 'border-box',
                }}
              >
                加载中…
              </div>
            ) : isOllama && ollamaFailed ? (
              <div
                style={{
                  height: 36,
                  paddingLeft: 8,
                  paddingRight: 8,
                  borderRadius: 6,
                  background: '#E6E6E6',
                  color: FIELD_HINT_COLOR,
                  display: 'flex',
                  alignItems: 'center',
                  fontSize: 14,
                  fontWeight: 400,
                  boxSizing: 'border-box',
                  pointerEvents: 'none',
                  userSelect: 'none',
                }}
              >
                无可用本地模型
              </div>
            ) : (
              <input
                type="text"
                value={modelName}
                onChange={(e) => setModelName(e.target.value)}
                placeholder="必填（例如：deepseek-v3.2）"
                style={{
                  width: '100%',
                  height: 36,
                  paddingLeft: 8,
                  paddingRight: 8,
                  borderRadius: 6,
                  background: '#E6E6E6',
                  border: 'none',
                  boxSizing: 'border-box',
                  fontFamily: FONT,
                  fontSize: 14,
                  fontWeight: 400,
                  color: TEXT_MAIN,
                }}
              />
            )}
          </Field>
        </div>

        <div style={{ marginTop: 18, marginBottom: 4 }}>
          <Field label="APIKey">
            <div style={{ position: 'relative', height: 36 }}>
              <input
                type={showApiKey ? 'text' : 'password'}
                value={apiKey}
                onChange={(e) => setApiKey(e.target.value)}
                placeholder={apiKeyPlaceholder}
                style={{
                  width: '100%',
                  height: 36,
                  paddingLeft: 8,
                  paddingRight: 40,
                  borderRadius: 6,
                  background: '#E6E6E6',
                  border: 'none',
                  boxSizing: 'border-box',
                  fontFamily: FONT,
                  fontSize: 14,
                  fontWeight: 400,
                  color: showApiKey ? TEXT_MAIN : '#000000',
                }}
                autoComplete="off"
              />
              <button
                type="button"
                onClick={() => setShowApiKey((v) => !v)}
                aria-label={showApiKey ? '隐藏 API Key' : '显示 API Key'}
                style={{
                  position: 'absolute',
                  right: 8,
                  top: 0,
                  bottom: 0,
                  width: 24,
                  background: 'transparent',
                  border: 'none',
                  padding: 0,
                  margin: 0,
                  cursor: 'pointer',
                  display: 'inline-flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                }}
              >
                <img
                  src={`${assetBasePath}/icons/${
                    showApiKey ? 'ukui-eye-display-symbolic.svg' : 'ukui-eye-hidden-symbolic.svg'
                  }`}
                  alt=""
                  width={16}
                  height={16}
                  draggable={false}
                  style={{ display: 'block' }}
                />
              </button>
            </div>
          </Field>
        </div>

        {needsBaseUrl && (
          <div style={{ marginTop: 18, marginBottom: 4 }}>
            <Field label={baseUrlLabel}>
              <input
                type="text"
                value={baseUrl}
                onChange={(e) => setBaseUrl(e.target.value)}
                placeholder={baseUrlPlaceholder}
                style={{
                  width: '100%',
                  height: 36,
                  paddingLeft: 8,
                  paddingRight: 8,
                  borderRadius: 6,
                  background: '#E6E6E6',
                  border: 'none',
                  boxSizing: 'border-box',
                  fontFamily: FONT,
                  fontSize: 14,
                  fontWeight: 400,
                  color: TEXT_MAIN,
                }}
              />
            </Field>
            {showBaseUrlSchemeError ? (
              <div
                style={{
                  marginTop: 4,
                  fontSize: 12,
                  fontWeight: 400,
                  color: '#F53F3F',
                  fontFamily: FONT,
                  lineHeight: '16px',
                }}
              >
                请求地址需以 http:// 或 https:// 开头
              </div>
            ) : null}
          </div>
        )}

        {!isOllama && (
          <div style={{ marginTop: 18 }}>
            <button
              type="button"
              className="model-dialog-advanced-toggle border-none bg-transparent p-0 outline-none"
              style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'flex-start',
                gap: 8,
                cursor: 'pointer',
                fontFamily: FONT,
                fontSize: 14,
                fontWeight: 400,
              }}
              onClick={() => setAdvancedOpen((v) => !v)}
            >
              <span className="text-left">高级设置（可配置模型用量统计）</span>
              <span style={{ display: 'inline-flex', flexShrink: 0, lineHeight: 0 }}>
                {advancedOpen ? (
                  <ChevronUp size={16} strokeWidth={2} aria-hidden />
                ) : (
                  <ChevronDown size={16} strokeWidth={2} aria-hidden />
                )}
              </span>
            </button>
            {advancedOpen ? (
              <div style={{ marginTop: 16, display: 'flex', flexDirection: 'column', gap: 16 }}>
                <PricingField
                  label="模型输入 Tokens 单价"
                  value={pricingInput}
                  onChange={setPricingInput}
                />
                <PricingField
                  label="模型输出 Tokens 单价"
                  value={pricingOutput}
                  onChange={setPricingOutput}
                  labelMarginTop={4}
                />
              </div>
            ) : null}
          </div>
        )}

        {error && (
          <div style={{ color: '#F53F3F', marginTop: 8, fontSize: 12, fontFamily: FONT }}>{error}</div>
        )}
          </div>
        </div>

        <div
          style={{
            flexShrink: 0,
            marginRight: SCROLLBAR_RIGHT_GUTTER,
            paddingLeft: 24,
            paddingRight: 24,
            paddingBottom: 24,
            paddingTop: 32,
            boxSizing: 'border-box',
          }}
        >
          <div
            style={{
              display: 'flex',
              flexDirection: 'row',
              flexWrap: 'nowrap',
              justifyContent: 'flex-end',
              alignItems: 'center',
              boxSizing: 'border-box',
            }}
          >
            <button
              type="button"
              onClick={onCancel}
              style={{
                height: 36,
                paddingLeft: 16,
                paddingRight: 16,
                borderRadius: 6,
                background: K_ALPHA_NORMAL,
                color: TEXT_MAIN,
                border: 'none',
                cursor: 'pointer',
                fontFamily: FONT,
                fontSize: 14,
                fontWeight: 400,
                boxSizing: 'border-box',
              }}
              onMouseEnter={(e) => {
                e.currentTarget.style.background = K_ALPHA_HOVER;
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.background = K_ALPHA_NORMAL;
              }}
              onMouseDown={(e) => {
                e.currentTarget.style.background = K_ALPHA_PRESS;
              }}
              onMouseUp={(e) => {
                e.currentTarget.style.background = K_ALPHA_HOVER;
              }}
            >
              取消
            </button>
            <button
              type="button"
              disabled={!canSubmit}
              onClick={() => void handleSubmit()}
              style={{
                marginLeft: 8,
                height: 36,
                paddingLeft: 16,
                paddingRight: 16,
                borderRadius: 6,
                background: canSubmit ? PRIMARY_BTN : 'rgba(55, 144, 250, 0.45)',
                color: '#FFFFFF',
                border: 'none',
                cursor: canSubmit ? 'pointer' : 'not-allowed',
                fontFamily: FONT,
                fontSize: 14,
                fontWeight: 400,
                boxSizing: 'border-box',
              }}
              onMouseEnter={(e) => {
                if (!canSubmit) return;
                e.currentTarget.style.background = PRIMARY_HOVER;
              }}
              onMouseLeave={(e) => {
                if (!canSubmit) return;
                e.currentTarget.style.background = PRIMARY_BTN;
              }}
              onMouseDown={(e) => {
                if (!canSubmit) return;
                e.currentTarget.style.background = PRIMARY_PRESS;
              }}
              onMouseUp={(e) => {
                if (!canSubmit) return;
                e.currentTarget.style.background = PRIMARY_HOVER;
              }}
            >
              {submitButtonLabel}
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}

function Field({
  label,
  children,
}: {
  label: string;
  children: React.ReactNode;
}) {
  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 0, fontFamily: FONT }}>
      <label
        style={{
          fontSize: 14,
          fontWeight: 400,
          color: TEXT_MAIN,
          marginBottom: 4,
        }}
      >
        {label}
      </label>
      {children}
    </div>
  );
}

function PricingField({
  label,
  value,
  onChange,
  labelMarginTop,
}: {
  label: string;
  value: string;
  onChange: (v: string) => void;
  /** 用于「模型输出 Tokens 单价」等与上一块之间的额外上边距（px） */
  labelMarginTop?: number;
}) {
  const inputId = useId();
  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 0, fontFamily: FONT }}>
      <label
        htmlFor={inputId}
        style={{
          fontSize: 14,
          fontWeight: 400,
          color: TEXT_MAIN,
          marginBottom: 4,
          ...(labelMarginTop != null ? { marginTop: labelMarginTop } : {}),
        }}
      >
        {label}
      </label>
      <div style={{ display: 'flex', flexDirection: 'row', alignItems: 'center' }}>
        <input
          id={inputId}
          type="text"
          inputMode="decimal"
          placeholder="选填"
          value={value}
          onChange={(e) => onChange(sanitizePricingInput(e.target.value))}
          style={{
            width: 204,
            maxWidth: '100%',
            height: 36,
            flexShrink: 0,
            paddingLeft: 8,
            paddingRight: 8,
            borderRadius: 6,
            background: '#E6E6E6',
            border: 'none',
            boxSizing: 'border-box',
            fontFamily: FONT,
            fontSize: 14,
            fontWeight: 400,
            color: TEXT_MAIN,
          }}
        />
        <span
          style={{
            flexShrink: 0,
            marginLeft: 24,
            whiteSpace: 'nowrap',
            fontSize: 14,
            fontWeight: 400,
            color: TEXT_MAIN,
          }}
        >
          {PRICING_UNIT}
        </span>
      </div>
    </div>
  );
}
