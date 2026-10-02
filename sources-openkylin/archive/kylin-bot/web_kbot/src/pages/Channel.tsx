import { useState, useEffect, useCallback, useMemo, useRef } from 'react';
import { X, Check } from 'lucide-react';
import { assetBasePath } from '@/lib/basePath';
import { getStatus, getChannelIdentifier, disconnectChannel } from '@/lib/api';
import type { StatusResponse } from '@/types/api';
import type { ChannelIdentifier } from '@/lib/api';
import { useLocale } from '@/lib/i18n';
import WeChatBind from './WeChatBind';
import WeComBind from './WeComBind';
import DingTalkBind from './DingTalkBind';
import FeishuBind from './FeishuBind';
import LansengerBind from './LansengerBind';
import {
  getChannelBindContentPaddingTop,
  getChannelBindModalHeight,
  getChannelBindModalStyle,
  getChannelBindOverlayStyle,
} from './channel/channelBindLayout';

type ModalChannel = 'wechat' | 'wecom' | 'dingtalk' | 'feishu' | 'lansenger';

const CHANNEL_CONFIG_BUTTON_BG = '#E5E5E5';
const CHANNEL_CONFIG_BUTTON_HOVER_BG = '#D9D9D9';
const CHANNEL_CONFIG_BUTTON_ACTIVE_BG = '#CCCCCC';
const CHANNEL_DISCONNECT_BUTTON_BG = '#F53F3F';
const CHANNEL_DISCONNECT_BUTTON_HOVER_BG = '#D93030';
const CHANNEL_DISCONNECT_BUTTON_ACTIVE_BG = '#B82020';
const CHANNEL_CONNECTED_MODAL_HEIGHT = 298;
const CHANNEL_CLOSE_TOOLTIP_DELAY_MS = 1000;
const CHANNEL_CLOSE_TOOLTIP_OFFSET_X = 12;
const CHANNEL_CLOSE_TOOLTIP_OFFSET_Y = 18;

interface ChannelCloseTooltip {
  label: string;
  left: number;
  top: number;
}

interface ChannelCloseTooltipHandlers {
  show: (label: string, clientX: number, clientY: number) => void;
  hide: () => void;
}

interface ChannelItem {
  name: string;
  icon: string;
  key: ModalChannel;
  status: 'connected' | 'disconnected' | 'unconfigured';
  description: string;
}

function ChannelCloseTooltipLayer({ tooltip }: { tooltip: ChannelCloseTooltip | null }) {
  if (!tooltip) return null;

  return (
    <span
      aria-hidden="true"
      style={{
        position: 'absolute',
        left: tooltip.left,
        top: tooltip.top,
        padding: '4px 10px',
        background: '#ffffff',
        border: '1px solid #e8e8e8',
        borderRadius: 4,
        fontFamily: "'Noto Sans CJK SC', sans-serif",
        fontSize: 12,
        fontWeight: 400,
        lineHeight: '18px',
        color: '#374151',
        whiteSpace: 'nowrap',
        boxShadow: '0 2px 8px rgba(0, 0, 0, 0.1)',
        pointerEvents: 'none',
        zIndex: 100,
      }}
    >
      {tooltip.label}
    </span>
  );
}

function ChannelModalCloseButton({
  label,
  onClose,
  tooltipHandlers,
}: {
  label: string;
  onClose: () => void;
  tooltipHandlers: ChannelCloseTooltipHandlers;
}) {
  const [isCloseHovered, setIsCloseHovered] = useState(false);
  const [isClosePressed, setIsClosePressed] = useState(false);
  const tooltipTimerRef = useRef<number | null>(null);
  const tooltipClientPositionRef = useRef({ clientX: 0, clientY: 0 });
  const tooltipVisibleRef = useRef(false);

  const clearTooltipTimer = useCallback(() => {
    if (tooltipTimerRef.current === null) return;
    window.clearTimeout(tooltipTimerRef.current);
    tooltipTimerRef.current = null;
  }, []);

  const updateTooltipClientPosition = (clientX: number, clientY: number) => {
    tooltipClientPositionRef.current = { clientX, clientY };
  };

  const hideTooltip = useCallback(() => {
    clearTooltipTimer();
    tooltipVisibleRef.current = false;
    tooltipHandlers.hide();
  }, [clearTooltipTimer, tooltipHandlers]);

  const startTooltipTimer = useCallback(() => {
    clearTooltipTimer();
    tooltipVisibleRef.current = false;
    tooltipHandlers.hide();
    tooltipTimerRef.current = window.setTimeout(() => {
      tooltipTimerRef.current = null;
      tooltipVisibleRef.current = true;
      tooltipHandlers.show(
        label,
        tooltipClientPositionRef.current.clientX,
        tooltipClientPositionRef.current.clientY,
      );
    }, CHANNEL_CLOSE_TOOLTIP_DELAY_MS);
  }, [clearTooltipTimer, label, tooltipHandlers]);

  useEffect(() => hideTooltip, [hideTooltip]);

  return (
    <button
      type="button"
      onClick={() => {
        hideTooltip();
        onClose();
      }}
      aria-label={label}
      className="absolute z-10 flex items-center justify-center transition-colors"
      style={{
        position: 'absolute',
        zIndex: 10,
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        width: 30,
        height: 30,
        top: 4,
        right: 4,
        borderRadius: 6,
        background: isClosePressed ? '#D93030' : isCloseHovered ? '#F53F3F' : 'transparent',
        border: 'none',
        padding: 0,
        cursor: 'pointer',
        color: isCloseHovered || isClosePressed ? '#FFFFFF' : '#000000',
        transition: 'background 0.15s ease, color 0.15s ease',
      }}
      onMouseEnter={(e) => {
        setIsCloseHovered(true);
        updateTooltipClientPosition(e.clientX, e.clientY);
        startTooltipTimer();
      }}
      onMouseMove={(e) => {
        if (!tooltipVisibleRef.current && tooltipTimerRef.current !== null) {
          updateTooltipClientPosition(e.clientX, e.clientY);
        }
      }}
      onMouseLeave={() => {
        setIsCloseHovered(false);
        setIsClosePressed(false);
        hideTooltip();
      }}
      onBlur={() => {
        setIsCloseHovered(false);
        setIsClosePressed(false);
        hideTooltip();
      }}
      onMouseDown={(e) => {
        if (e.button === 0) {
          setIsClosePressed(true);
        }
      }}
      onMouseUp={(e) => {
        if (e.button === 0) {
          setIsClosePressed(false);
        }
      }}
    >
      <X
        className="h-[14px] w-[14px]"
        aria-hidden="true"
        style={{ width: 14, height: 14, color: 'currentColor', transition: 'color 0.15s ease' }}
      />
    </button>
  );
}

function BindModal({
  channel,
  onClose,
  onBindSuccess,
  closeTooltipHandlers,
}: {
  channel: ModalChannel;
  onClose: () => void;
  onBindSuccess: () => void;
  closeTooltipHandlers: ChannelCloseTooltipHandlers;
}) {
  const { t } = useLocale();
  const closeTooltip = t('common.close');
  const modalBaseHeight = channel === 'lansenger' ? 446 : undefined;
  const [modalHeight, setModalHeight] = useState(() =>
    getChannelBindModalHeight(typeof window === 'undefined' ? 0 : window.innerHeight, modalBaseHeight),
  );
  const [contentPaddingTop, setContentPaddingTop] = useState(() =>
    getChannelBindContentPaddingTop(typeof window === 'undefined' ? 0 : window.innerHeight),
  );

  useEffect(() => {
    const updateLayout = () => {
      setModalHeight(getChannelBindModalHeight(window.innerHeight, modalBaseHeight));
      setContentPaddingTop(getChannelBindContentPaddingTop(window.innerHeight));
    };

    updateLayout();
    window.addEventListener('resize', updateLayout);
    return () => window.removeEventListener('resize', updateLayout);
  }, [modalBaseHeight]);

  return (
    <div
      className="animate-fade-in"
      style={getChannelBindOverlayStyle()}
    >
      <div
        className="animate-fade-in-scale"
        style={getChannelBindModalStyle(modalHeight)}
      >
        <ChannelModalCloseButton label={closeTooltip} onClose={onClose} tooltipHandlers={closeTooltipHandlers} />

        {channel === 'wechat' && <WeChatBind onBindSuccess={onBindSuccess} contentPaddingTop={contentPaddingTop} />}
        {channel === 'wecom' && <WeComBind onBindSuccess={onBindSuccess} contentPaddingTop={contentPaddingTop} />}
        {channel === 'dingtalk' && <DingTalkBind onBindSuccess={onBindSuccess} contentPaddingTop={contentPaddingTop} />}
        {channel === 'feishu' && <FeishuBind onBindSuccess={onBindSuccess} contentPaddingTop={contentPaddingTop} />}
        {channel === 'lansenger' && <LansengerBind onBindSuccess={onBindSuccess} contentPaddingTop={contentPaddingTop} />}
      </div>
    </div>
  );
}

function ConnectedModal({
  channel,
  onClose,
  onDisconnected,
  closeTooltipHandlers,
}: {
  channel: ChannelItem;
  onClose: () => void;
  onDisconnected: () => void;
  closeTooltipHandlers: ChannelCloseTooltipHandlers;
}) {
  const [identifier, setIdentifier] = useState<ChannelIdentifier | null>(null);
  const [disconnecting, setDisconnecting] = useState(false);
  const [isConfirmHovered, setIsConfirmHovered] = useState(false);
  const [isConfirmPressed, setIsConfirmPressed] = useState(false);
  const [isDisconnectHovered, setIsDisconnectHovered] = useState(false);
  const [isDisconnectPressed, setIsDisconnectPressed] = useState(false);
  const { t } = useLocale();
  const closeTooltip = t('common.close');
  const confirmBackground = isConfirmPressed
    ? CHANNEL_CONFIG_BUTTON_ACTIVE_BG
    : isConfirmHovered
      ? CHANNEL_CONFIG_BUTTON_HOVER_BG
      : CHANNEL_CONFIG_BUTTON_BG;
  const disconnectBackground = disconnecting
    ? 'rgba(245, 63, 63, 0.5)'
    : isDisconnectPressed
      ? CHANNEL_DISCONNECT_BUTTON_ACTIVE_BG
      : isDisconnectHovered
        ? CHANNEL_DISCONNECT_BUTTON_HOVER_BG
        : CHANNEL_DISCONNECT_BUTTON_BG;

  useEffect(() => {
    getChannelIdentifier(channel.key)
      .then(setIdentifier)
      .catch(() => { });
  }, [channel.key]);

  const handleDisconnect = async () => {
    if (disconnecting) return;
    setDisconnecting(true);
    try {
      await disconnectChannel(channel.key);
      onDisconnected();
    } catch {
      setDisconnecting(false);
    }
  };

  return (
    <div
      className="animate-fade-in"
      style={getChannelBindOverlayStyle()}
    >
      <div
        className="animate-fade-in-scale"
        style={{
          ...getChannelBindModalStyle(CHANNEL_CONNECTED_MODAL_HEIGHT),
          display: 'flex',
          flexDirection: 'column',
          overflowY: 'hidden',
        }}
      >
        <ChannelModalCloseButton label={closeTooltip} onClose={onClose} tooltipHandlers={closeTooltipHandlers} />

        <div
          className="flex flex-col items-center"
          style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            paddingTop: 32,
          }}
        >
          <img src={`${assetBasePath}/icons/${channel.icon}`} alt={channel.name} width="64" height="64" style={{ borderRadius: 16 }} />

          <h2 style={{
            fontFamily: "'Noto Sans CJK SC', sans-serif",
            fontSize: 20,
            fontWeight: 500,
            color: '#000000',
            margin: 0,
            marginTop: 16,
          }}>
            {channel.name} {t('channel.connected')}
          </h2>

          <div
            className="flex flex-col items-center"
            style={{
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              marginTop: 20,
            }}
          >
            <span style={{
              fontFamily: "'Noto Sans CJK SC', sans-serif",
              fontSize: 14,
              fontWeight: 400,
              color: 'rgba(0, 0, 0, 0.45)',
              marginBottom: 4,
            }}>
              {channel.name} {identifier?.label ?? t('channel.bot_id')}：
            </span>
            <span style={{
              fontFamily: "'Noto Sans CJK SC', sans-serif",
              fontSize: 14,
              fontWeight: 400,
              color: 'rgba(0, 0, 0, 0.85)',
            }}>
              {identifier?.value ?? '—'}
            </span>
          </div>
        </div>

        <div
          className="flex items-center justify-end"
          style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'flex-end',
            marginTop: 'auto',
            paddingBottom: 24,
          }}
        >
          <button
            onClick={onClose}
            style={{
              fontFamily: "'Noto Sans CJK SC', sans-serif",
              fontSize: 14,
              fontWeight: 400,
              minWidth: 88,
              padding: '8px 16px',
              borderRadius: 6,
              background: confirmBackground,
              color: 'rgba(0, 0, 0, 0.85)',
              border: 'none',
              cursor: 'pointer',
              whiteSpace: 'nowrap',
              marginRight: 12,
              transition: 'background 0.15s ease',
            }}
            onMouseEnter={() => setIsConfirmHovered(true)}
            onMouseLeave={() => {
              setIsConfirmHovered(false);
              setIsConfirmPressed(false);
            }}
            onMouseDown={(e) => {
              if (e.button === 0) {
                setIsConfirmPressed(true);
              }
            }}
            onMouseUp={(e) => {
              if (e.button === 0) {
                setIsConfirmPressed(false);
              }
            }}
            onBlur={() => {
              setIsConfirmHovered(false);
              setIsConfirmPressed(false);
            }}
          >
            {t('channel.confirm')}
          </button>
          <button
            onClick={handleDisconnect}
            disabled={disconnecting}
            style={{
              fontFamily: "'Noto Sans CJK SC', sans-serif",
              fontSize: 14,
              fontWeight: 400,
              minWidth: 88,
              padding: '8px 16px',
              borderRadius: 6,
              background: disconnectBackground,
              color: '#FFFFFF',
              border: 'none',
              cursor: disconnecting ? 'not-allowed' : 'pointer',
              whiteSpace: 'nowrap',
              transition: 'background 0.15s ease',
            }}
            onMouseEnter={() => {
              if (!disconnecting) setIsDisconnectHovered(true);
            }}
            onMouseLeave={() => {
              setIsDisconnectHovered(false);
              setIsDisconnectPressed(false);
            }}
            onMouseDown={(e) => {
              if (!disconnecting && e.button === 0) {
                setIsDisconnectPressed(true);
              }
            }}
            onMouseUp={(e) => {
              if (!disconnecting && e.button === 0) {
                setIsDisconnectPressed(false);
              }
            }}
            onBlur={() => {
              setIsDisconnectHovered(false);
              setIsDisconnectPressed(false);
            }}
          >
            {disconnecting ? t('channel.disconnecting') : t('channel.disconnect')}
          </button>
        </div>
      </div>
    </div>
  );
}

const CHANNEL_KEY_MAP: Record<string, ModalChannel> = {
  WeChat: 'wechat',
  WeCom: 'wecom',
  DingTalk: 'dingtalk',
  Feishu: 'feishu',
  Lansenger: 'lansenger',
};

function getChannelCardBorderRadius(index: number, total: number, radius = 8): string {
  const columns = 2;
  const lastIndex = total - 1;
  const topLeft = index === 0;
  const topRight = index === Math.min(columns - 1, lastIndex);
  const lastRowStart = Math.floor(lastIndex / columns) * columns;
  const bottomLeft = index === lastRowStart;
  const bottomRight = index === lastIndex;

  return [
    topLeft ? radius : 0,
    topRight ? radius : 0,
    bottomRight ? radius : 0,
    bottomLeft ? radius : 0,
  ].join('px ') + 'px';
}

type ZoomKeyEvent = Pick<KeyboardEvent, 'ctrlKey' | 'metaKey' | 'key' | 'code'>;
type ZoomWheelEvent = Pick<WheelEvent, 'ctrlKey' | 'metaKey'>;
type ZoomTouchEvent = Pick<TouchEvent, 'touches'>;

export function isModifiedZoomWheel(event: ZoomWheelEvent): boolean {
  return event.ctrlKey || event.metaKey;
}

export function isZoomShortcut(event: ZoomKeyEvent): boolean {
  if (!event.ctrlKey && !event.metaKey) return false;

  return ['+', '=', '-', '_', '0'].includes(event.key) ||
    ['NumpadAdd', 'NumpadSubtract', 'Numpad0'].includes(event.code);
}

export function isMultiTouchZoom(event: ZoomTouchEvent): boolean {
  return event.touches.length > 1;
}

function useChannelInteractionGuards() {
  useEffect(() => {
    // Qt Quick WebEngine: document-level pinch/gesture + non-passive wheel handlers have been flaky on
    // aarch64/Qt 5.12; polyfills.ts adds `kybot-embedded-qt-webengine` for this host.
    try {
      if (
        typeof document !== 'undefined' &&
        document.documentElement.classList.contains('kybot-embedded-qt-webengine')
      ) {
        return;
      }
    } catch {
      /* proceed */
    }

    const viewportMeta = document.querySelector<HTMLMetaElement>('meta[name="viewport"]');
    const previousViewportContent = viewportMeta?.getAttribute('content') ?? null;

    viewportMeta?.setAttribute(
      'content',
      'width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no',
    );

    const preventDefault = (event: Event) => {
      event.preventDefault();
    };

    const handleWheel = (event: WheelEvent) => {
      if (isModifiedZoomWheel(event)) {
        event.preventDefault();
      }
    };

    const handleKeyDown = (event: KeyboardEvent) => {
      if (isZoomShortcut(event)) {
        event.preventDefault();
      }
    };

    const handleTouchMove = (event: TouchEvent) => {
      if (isMultiTouchZoom(event)) {
        event.preventDefault();
      }
    };

    document.addEventListener('contextmenu', preventDefault);
    document.addEventListener('wheel', handleWheel, { passive: false });
    document.addEventListener('keydown', handleKeyDown);
    document.addEventListener('touchmove', handleTouchMove, { passive: false });
    document.addEventListener('gesturestart', preventDefault);
    document.addEventListener('gesturechange', preventDefault);
    document.addEventListener('gestureend', preventDefault);

    return () => {
      document.removeEventListener('contextmenu', preventDefault);
      document.removeEventListener('wheel', handleWheel);
      document.removeEventListener('keydown', handleKeyDown);
      document.removeEventListener('touchmove', handleTouchMove);
      document.removeEventListener('gesturestart', preventDefault);
      document.removeEventListener('gesturechange', preventDefault);
      document.removeEventListener('gestureend', preventDefault);

      if (viewportMeta) {
        if (previousViewportContent === null) {
          viewportMeta.removeAttribute('content');
        } else {
          viewportMeta.setAttribute('content', previousViewportContent);
        }
      }
    };
  }, []);
}

export default function Channel() {
  const { t } = useLocale();
  useChannelInteractionGuards();
  const channelPageRef = useRef<HTMLDivElement>(null);
  const [activeModal, setActiveModal] = useState<ModalChannel | null>(null);
  const [successToast, setSuccessToast] = useState<string | null>(null);
  const [closeTooltip, setCloseTooltip] = useState<ChannelCloseTooltip | null>(null);

  const channelBase: Omit<ChannelItem, 'status' | 'statusLabel'>[] = [
    {
      name: t('channel.wechat'),
      icon: 'wechat.svg',
      key: 'wechat',
      description: t('channel.wechat.desc'),
    },
    {
      name: t('channel.wecom'),
      icon: 'wecom.svg',
      key: 'wecom',
      description: t('channel.wecom.desc'),
    },
    {
      name: t('channel.dingtalk'),
      icon: 'dingtalk.svg',
      key: 'dingtalk',
      description: t('channel.dingtalk.desc'),
    },
    {
      name: t('channel.feishu'),
      icon: 'feishu.svg',
      key: 'feishu',
      description: t('channel.feishu.desc'),
    },
    {
      name: t('channel.lansenger'),
      icon: 'lansenger.svg',
      key: 'lansenger',
      description: t('channel.lansenger.desc'),
    },
  ];

  const resolveStatus = (configured: boolean): Pick<ChannelItem, 'status'> => {
    return { status: configured ? 'connected' : 'unconfigured' };
  };

  const statusStyle = (status: ChannelItem['status']) => {
    switch (status) {
      case 'connected':
        return { color: '#00e68a', label: t('channel.connected') };
      case 'disconnected':
        return { color: '#ffaa00', label: t('channel.not_connected') };
      case 'unconfigured':
        return { color: 'var(--pc-text-faint)', label: t('channel.not_configured') };
    }
  };

  const CHANNEL_SUCCESS_TEXT: Record<ModalChannel, string> = {
    wechat: t('channel.wechat.connected'),
    wecom: t('channel.wecom.connected'),
    dingtalk: t('channel.dingtalk.connected'),
    feishu: t('channel.feishu.connected'),
    lansenger: t('channel.lansenger.connected'),
  };

  const CHANNEL_DISCONNECTED_TEXT: Record<ModalChannel, string> = {
    wechat: t('channel.wechat.disconnected'),
    wecom: t('channel.wecom.disconnected'),
    dingtalk: t('channel.dingtalk.disconnected'),
    feishu: t('channel.feishu.disconnected'),
    lansenger: t('channel.lansenger.disconnected'),
  };

  const [channels, setChannels] = useState<ChannelItem[]>(channelBase.map(c => ({
    ...c,
    status: 'unconfigured' as const,
  })));

  const showCloseTooltip = useCallback((label: string, clientX: number, clientY: number) => {
    const rect = channelPageRef.current?.getBoundingClientRect();
    setCloseTooltip({
      label,
      left: clientX - (rect?.left ?? 0) + CHANNEL_CLOSE_TOOLTIP_OFFSET_X,
      top: clientY - (rect?.top ?? 0) + CHANNEL_CLOSE_TOOLTIP_OFFSET_Y,
    });
  }, []);

  const hideCloseTooltip = useCallback(() => {
    setCloseTooltip(null);
  }, []);

  const closeTooltipHandlers = useMemo<ChannelCloseTooltipHandlers>(() => ({
    show: showCloseTooltip,
    hide: hideCloseTooltip,
  }), [hideCloseTooltip, showCloseTooltip]);

  const refreshStatus = useCallback(() => {
    getStatus()
      .then((data: StatusResponse) => {
        const statusMap: Record<ModalChannel, boolean> = {
          wechat: false,
          wecom: false,
          dingtalk: false,
          feishu: false,
          lansenger: false,
        };
        for (const [backendKey, configured] of Object.entries(data.channels)) {
          const mapped = CHANNEL_KEY_MAP[backendKey];
          if (mapped) statusMap[mapped] = configured;
        }
        setChannels(prev =>
          prev.map(ch => ({
            ...ch,
            ...resolveStatus(statusMap[ch.key]),
          }))
        );
      })
      .catch(() => { });
  }, [CHANNEL_KEY_MAP]);

  useEffect(() => {
    refreshStatus();
  }, [refreshStatus]);

  return (
    <div
      ref={channelPageRef}
      className="pr-6 pb-6 pl-6 animate-fade-in"
      style={{
        position: 'relative',
        paddingTop: 10,
        paddingRight: 24,
        paddingBottom: 24,
        paddingLeft: 24,
        background: '#FFFFFF',
        minHeight: '100vh',
      }}
    >
      <h2 className="mb-2" style={{
        margin: 0,
        fontSize: 14,
        fontWeight: 400,
        color: 'rgba(0, 0, 0, 0.85)',
        marginLeft: 8,
        marginBottom: 8,
      }}>{t('channel.title')}</h2>

      <div className="grid grid-cols-2 gap-x-2 gap-y-2 stagger-children">
        {channels.map((channel, idx) => {
          const s = statusStyle(channel.status);
          const cornerStyle: React.CSSProperties = {
            background: '#F6F6F6',
            border: '1px solid var(--pc-border)',
            borderRadius: getChannelCardBorderRadius(idx, channels.length),
            paddingLeft: 20,
            paddingRight: 20,
            boxSizing: 'border-box',
          };
          return (
            <div
              key={channel.key}
              className="animate-slide-in-up flex items-center transition-all"
              style={{ ...cornerStyle, height: 89, display: 'flex', alignItems: 'center' }}
            >
              <img
                src={`${assetBasePath}/icons/${channel.icon}`}
                alt={channel.name}
                className="h-8 w-8 shrink-0"
                style={{ width: 32, height: 32, flexShrink: 0, marginRight: 16 }}
              />

              <div
                className="flex-1 min-w-0"
                style={{ flex: '1 1 0%', minWidth: 0, marginRight: 16 }}
              >
                <div
                  className="flex items-center mb-0.5"
                  style={{ display: 'flex', alignItems: 'center', marginBottom: 2 }}
                >
                  <span className="text-sm font-semibold" style={{ color: 'var(--pc-text-primary)' }}>
                    {channel.name}
                  </span>
                  <span
                    className="inline-flex items-center justify-center"
                    style={{
                      minWidth: 44,
                      height: 20,
                      padding: '0 8px',
                      borderRadius: 10,
                      background: channel.status === 'connected' ? 'rgba(0, 180, 42, 0.05)' : 'rgba(0, 0, 0, 0.05)',
                      fontSize: 12,
                      fontWeight: 400,
                      color: channel.status === 'connected' ? '#00B42A' : 'rgba(0, 0, 0, 0.65)',
                      whiteSpace: 'nowrap',
                      marginLeft: 8,
                    }}
                  >
                    {s.label}
                  </span>
                </div>
                <p
                  className="text-sm leading-relaxed line-clamp-2"
                  style={{ margin: 0, color: 'var(--pc-text-muted)' }}
                >
                  {channel.description}
                </p>
              </div>

              <button
                onClick={() => setActiveModal(channel.key)}
                className="shrink-0 rounded-lg text-sm font-medium transition-all flex items-center justify-center"
                style={{
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  minWidth: 64,
                  height: 36,
                  padding: '0 14px',
                  background: CHANNEL_CONFIG_BUTTON_BG,
                  border: '1px solid var(--pc-border)',
                  color: 'rgba(0, 0, 0, 0.85)',
                  whiteSpace: 'nowrap',
                }}
                onMouseEnter={(e) => {
                  e.currentTarget.style.background = CHANNEL_CONFIG_BUTTON_HOVER_BG;
                  e.currentTarget.style.borderColor = 'var(--pc-border-strong)';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.background = CHANNEL_CONFIG_BUTTON_BG;
                  e.currentTarget.style.borderColor = 'var(--pc-border)';
                }}
                onMouseDown={(e) => {
                  if (e.button === 0) {
                    e.currentTarget.style.background = CHANNEL_CONFIG_BUTTON_ACTIVE_BG;
                  }
                }}
                onMouseUp={(e) => {
                  if (e.button === 0) {
                    e.currentTarget.style.background = CHANNEL_CONFIG_BUTTON_HOVER_BG;
                  }
                }}
              >
                {t('channel.configure')}
              </button>
            </div>
          );
        })}
      </div>

      {activeModal && (() => {
        const ch = channels.find(c => c.key === activeModal);
        if (!ch) return null;
        if (ch.status === 'connected') {
          return (
            <ConnectedModal
              channel={ch}
              closeTooltipHandlers={closeTooltipHandlers}
              onClose={() => {
                setActiveModal(null);
                refreshStatus();
              }}
              onDisconnected={() => {
                setActiveModal(null);
                setSuccessToast(CHANNEL_DISCONNECTED_TEXT[ch.key]);
                refreshStatus();
                setTimeout(() => setSuccessToast(null), 3000);
              }}
            />
          );
        }
        return <BindModal channel={activeModal} onClose={() => {
          setActiveModal(null);
          refreshStatus();
        }} onBindSuccess={() => {
          setActiveModal(null);
          setSuccessToast(CHANNEL_SUCCESS_TEXT[activeModal]);
          refreshStatus();
          setTimeout(() => setSuccessToast(null), 3000);
        }} closeTooltipHandlers={closeTooltipHandlers} />;
      })()}

      <ChannelCloseTooltipLayer tooltip={closeTooltip} />

      {successToast && (
        <div
          className="flex items-center rounded-lg shadow-lg"
          style={{
            position: 'fixed',
            display: 'flex',
            alignItems: 'center',
            left: '50%',
            top: '50%',
            transform: 'translate(-50%, -50%)',
            zIndex: 100,
            padding: '18px 16px',
            borderRadius: 8,
            background: '#FFFFFF',
            border: '1px solid rgba(0,0,0,0.08)',
            fontSize: 14,
            color: 'rgba(0,0,0,0.85)',
            fontWeight: 500,
            animation: 'channelToastFadeInScale 0.3s ease-out both',
          }}
        >
          <div
            className="flex items-center justify-center shrink-0 rounded-full"
            style={{
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              flexShrink: 0,
              width: 16,
              height: 16,
              borderRadius: '50%',
              background: '#10b981',
              marginRight: 8,
            }}
          >
            <Check
              className="w-[10px] h-[10px]"
              style={{ width: 10, height: 10, color: '#FFFFFF', strokeWidth: 3 }}
            />
          </div>
          <span>{successToast}</span>
        </div>
      )}
    </div>
  );
}
