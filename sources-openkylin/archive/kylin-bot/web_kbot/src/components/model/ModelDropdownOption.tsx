import type { CSSProperties, ReactNode } from 'react';
import { assetBasePath } from '@/lib/basePath';

export const MODEL_DROPDOWN_OPTION_WIDTH = 304;

const FONT =
  "'Noto Sans CJK SC', 'Source Han Sans SC', 'PingFang SC', 'Microsoft YaHei', sans-serif";

interface ModelDropdownOptionProps {
  selected: boolean;
  onPick: () => void;
  children: ReactNode;
  /** Reserve checkmark column even when unselected (provider list alignment). */
  reserveCheckmarkSlot?: boolean;
}

export function ModelDropdownOption({
  selected,
  onPick,
  children,
  reserveCheckmarkSlot = false,
}: ModelDropdownOptionProps) {
  return (
    <button
      type="button"
      role="option"
      aria-selected={selected}
      onClick={onPick}
      onMouseDown={(e) => e.preventDefault()}
      className="model-dialog-dropdown-option box-border flex shrink-0 cursor-default flex-row flex-nowrap items-center overflow-hidden border-none outline-none"
      style={{
        width: MODEL_DROPDOWN_OPTION_WIDTH,
        height: 36,
        paddingLeft: 8,
        paddingRight: 8,
        columnGap: 8,
        borderRadius: 4,
        color: 'rgba(0, 0, 0, 0.85)',
        fontFamily: FONT,
        fontSize: 14,
        fontWeight: 400,
      } as CSSProperties}
    >
      <span
        className="inline-flex shrink-0 items-center justify-center"
        style={{ width: 16, height: 16 }}
      >
        {selected ? (
          <img
            src={`${assetBasePath}/icons/object-select-symbolic.svg`}
            alt=""
            width={16}
            height={16}
            className="block object-contain"
            draggable={false}
          />
        ) : reserveCheckmarkSlot ? (
          <span className="block w-4" aria-hidden />
        ) : null}
      </span>
      {children}
    </button>
  );
}
