import {
  useEffect,
  useLayoutEffect,
  useRef,
  useState,
  type CSSProperties,
  type RefObject,
} from 'react';
import { createPortal } from 'react-dom';
import { X } from 'lucide-react';

const ANCHOR_TOOLTIP_OFFSET = 8;
const TOOLTIP_DELAY_MS = 500;

const TOOLTIP_SURFACE: CSSProperties = {
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
};

function computeAnchorTooltipPos(anchor: HTMLElement) {
  const rect = anchor.getBoundingClientRect();
  return {
    top: rect.bottom + ANCHOR_TOOLTIP_OFFSET,
    left: rect.right + ANCHOR_TOOLTIP_OFFSET,
  };
}

function computeTextTooltipPos(anchor: HTMLElement) {
  const rect = anchor.getBoundingClientRect();
  const above = rect.bottom + 80 > window.innerHeight;
  return {
    top: above ? rect.top - ANCHOR_TOOLTIP_OFFSET : rect.bottom + ANCHOR_TOOLTIP_OFFSET,
    left: rect.left,
    above,
  };
}

function isTextTruncated(el: HTMLElement): boolean {
  return el.scrollWidth > el.clientWidth;
}

/** 固定在锚点元素右下方；用 fixed + portal，避免弹窗 overflow:hidden 裁切。 */
function FixedAnchorTooltip({
  anchorRef,
  visible,
  label,
  zIndex = 300,
}: {
  anchorRef: RefObject<HTMLElement | null>;
  visible: boolean;
  label: string;
  zIndex?: number;
}) {
  const [pos, setPos] = useState({ top: 0, left: 0 });

  useLayoutEffect(() => {
    if (!visible || !anchorRef.current) return;
    setPos(computeAnchorTooltipPos(anchorRef.current));
  }, [visible, anchorRef]);

  if (!visible || typeof document === 'undefined') return null;

  return createPortal(
    <div
      style={{
        position: 'fixed',
        top: pos.top,
        left: pos.left,
        zIndex,
        ...TOOLTIP_SURFACE,
      }}
    >
      {label}
    </div>,
    document.body,
  );
}

/** 截断文本悬停提示：fixed + portal，避免列表行 overflow:hidden 裁切。 */
function FixedTextTooltip({
  anchorRef,
  visible,
  text,
  zIndex = 150,
}: {
  anchorRef: RefObject<HTMLElement | null>;
  visible: boolean;
  text: string;
  zIndex?: number;
}) {
  const [pos, setPos] = useState({ top: 0, left: 0, above: false });

  useLayoutEffect(() => {
    if (!visible || !anchorRef.current) return;
    setPos(computeTextTooltipPos(anchorRef.current));
  }, [visible, anchorRef, text]);

  if (!visible || typeof document === 'undefined') return null;

  return createPortal(
    <div
      style={{
        position: 'fixed',
        top: pos.top,
        left: pos.left,
        transform: pos.above ? 'translateY(-100%)' : undefined,
        zIndex,
        ...TOOLTIP_SURFACE,
        fontSize: 12,
        whiteSpace: 'pre-wrap',
        maxWidth: 300,
        wordBreak: 'break-word',
      }}
    >
      {text}
    </div>,
    document.body,
  );
}

function useDelayedAnchorTooltip(enabled = true) {
  const [tooltipVisible, setTooltipVisible] = useState(false);
  const timerRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  const onMouseEnter = () => {
    if (!enabled) return;
    timerRef.current = setTimeout(() => setTooltipVisible(true), TOOLTIP_DELAY_MS);
  };

  const onMouseLeave = () => {
    if (timerRef.current) clearTimeout(timerRef.current);
    timerRef.current = null;
    setTooltipVisible(false);
  };

  useEffect(() => {
    return () => {
      if (timerRef.current) clearTimeout(timerRef.current);
    };
  }, []);

  return { tooltipVisible, onMouseEnter, onMouseLeave };
}

/** 弹窗关闭按钮：默认黑色无框，悬停红底白字并提示「关闭」，点击红色加深（CSS :active）。 */
export function ModelCloseButton({ onClick }: { onClick: () => void }) {
  const anchorRef = useRef<HTMLDivElement>(null);
  const { tooltipVisible, onMouseEnter, onMouseLeave } = useDelayedAnchorTooltip();

  return (
    <>
      <div ref={anchorRef} style={{ position: 'relative', display: 'inline-flex' }}>
        <button
          type="button"
          className="skill-dialog-close-btn"
          onClick={onClick}
          aria-label="关闭"
          style={{ cursor: 'default' }}
          onMouseEnter={onMouseEnter}
          onMouseLeave={onMouseLeave}
        >
          <X size={16} />
        </button>
      </div>
      <FixedAnchorTooltip anchorRef={anchorRef} visible={tooltipVisible} label="关闭" />
    </>
  );
}

/** 列表「更多」按钮悬停提示（菜单展开时不显示，固定在按钮右下方）。 */
export function useModelMoreTooltip(menuOpen: boolean) {
  const anchorRef = useRef<HTMLDivElement>(null);
  const { tooltipVisible, onMouseEnter, onMouseLeave } = useDelayedAnchorTooltip(!menuOpen);

  const tooltip =
    tooltipVisible && !menuOpen ? (
      <FixedAnchorTooltip anchorRef={anchorRef} visible label="更多" zIndex={100} />
    ) : null;

  return { anchorRef, onMouseEnter, onMouseLeave, tooltip };
}

/** 文本截断时在悬停处显示完整内容（与 Skills 卡片名称提示一致）。 */
export function TruncatedTextTooltip({
  text,
  className,
  style,
}: {
  text: string;
  className?: string;
  style?: CSSProperties;
}) {
  const anchorRef = useRef<HTMLSpanElement>(null);
  const [showTooltip, setShowTooltip] = useState(false);

  return (
    <>
      <span
        ref={anchorRef}
        className={`block min-w-0 truncate whitespace-nowrap ${className ?? ''}`}
        style={style}
        onMouseEnter={() => {
          const el = anchorRef.current;
          if (!el || !isTextTruncated(el)) return;
          setShowTooltip(true);
        }}
        onMouseLeave={() => setShowTooltip(false)}
      >
        {text}
      </span>
      <FixedTextTooltip anchorRef={anchorRef} visible={showTooltip} text={text} />
    </>
  );
}
