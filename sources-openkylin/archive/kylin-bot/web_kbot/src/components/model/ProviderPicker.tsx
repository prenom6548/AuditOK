import { useEffect, useRef, useState } from 'react';
import { PRESET_PROVIDERS, type PresetProviderKey, findPresetByKey } from './providerMeta';
import { ModelDropdownOption } from './ModelDropdownOption';
import { ProviderIcon } from './ProviderIcon';
import { assetBasePath } from '@/lib/basePath';

interface ProviderPickerProps {
  value: PresetProviderKey;
  onChange: (key: PresetProviderKey) => void;
  disabled?: boolean;
}

const FONT =
  "'Noto Sans CJK SC', 'Source Han Sans SC', 'PingFang SC', 'Microsoft YaHei', sans-serif";

export function ProviderPicker({ value, onChange, disabled }: ProviderPickerProps) {
  const [open, setOpen] = useState(false);
  const wrapperRef = useRef<HTMLDivElement | null>(null);
  const current = findPresetByKey(value);

  useEffect(() => {
    if (disabled) setOpen(false);
  }, [disabled]);

  useEffect(() => {
    if (!open) return;
    const handler = (e: MouseEvent) => {
      const target = e.target as Node | null;
      if (wrapperRef.current && !wrapperRef.current.contains(target)) {
        setOpen(false);
      }
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [open]);

  return (
    <div ref={wrapperRef} className="relative w-full" style={{ fontFamily: FONT }}>
      <button
        type="button"
        disabled={disabled}
        onClick={() => {
          if (disabled) return;
          setOpen((v) => !v);
        }}
        aria-expanded={open}
        aria-haspopup="listbox"
        className="w-full box-border cursor-default border-none outline-none transition-colors"
        style={{
          height: 36,
          padding: 8,
          borderRadius: 6,
          background: disabled ? '#E6E6E6' : '#E6E6E6',
          color: 'rgba(0, 0, 0, 0.85)',
          opacity: disabled ? 0.6 : 1,
          display: 'flex',
          flexDirection: 'row',
          flexWrap: 'nowrap',
          alignItems: 'center',
          justifyContent: 'space-between',
          columnGap: 8,
          boxSizing: 'border-box',
        }}
      >
        <span
          className="min-w-0 flex-1"
          style={{
            display: 'flex',
            flexDirection: 'row',
            flexWrap: 'nowrap',
            alignItems: 'center',
            marginRight: 'auto',
          }}
        >
          <ProviderIcon meta={current} bundledIcon={current.iconAsset} size={16} />
          <span
            className="min-w-0 truncate whitespace-nowrap"
            style={{
              fontFamily: FONT,
              fontSize: 14,
              fontWeight: 400,
              marginLeft: 8,
              flexShrink: 1,
              minWidth: 0,
            }}
          >
            {current.label}
          </span>
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
            src={`${assetBasePath}/icons/${open ? 'ukui-up-symbolic.svg' : 'ukui-down-symbolic.svg'}`}
            alt=""
            width={16}
            height={16}
            className="object-contain"
            draggable={false}
            style={{
              opacity: disabled ? 0.35 : 1,
              display: 'block',
            }}
          />
        </span>
      </button>

      {open && !disabled && (
        <div
          className="model-dialog-dropdown model-dialog-provider-dropdown absolute left-0 z-[120] box-border flex flex-col overflow-hidden border border-solid"
          style={{
            top: 'calc(100% - 36px)',
            width: 328,
            height: 274,
            padding: '8px 6px 8px 8px',
            borderRadius: 8,
            borderColor: '#E5E7EB',
            background: '#FFFFFF',
          }}
        >
          <div
            className="model-dialog-scrollbar min-h-0 flex-1 overscroll-contain"
            role="listbox"
            onMouseDown={(e) => e.stopPropagation()}
          >
            {PRESET_PROVIDERS.map((p) => (
              <ModelDropdownOption
                key={p.key}
                selected={p.key === value}
                reserveCheckmarkSlot
                onPick={() => {
                  onChange(p.key);
                  setOpen(false);
                }}
              >
                <span
                  style={{
                    display: 'flex',
                    flexShrink: 0,
                    alignItems: 'center',
                  }}
                >
                  <ProviderIcon meta={p} bundledIcon={p.iconAsset} size={16} />
                </span>
                <span
                  className="min-w-0 flex-1 truncate whitespace-nowrap text-left"
                  style={{
                    marginLeft: 8,
                    fontFamily: FONT,
                    fontSize: 14,
                    fontWeight: 400,
                    color: 'rgba(0,0,0,0.85)',
                  }}
                >
                  {p.label}
                </span>
              </ModelDropdownOption>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
